/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "netlink_message_decoder.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_log.h>

#include "netlink_define.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace nmd {
using namespace NetlinkDefine;
using namespace OHOS::NetManagerStandard::CommonUtils;
namespace {
constexpr int16_t LOCAL_QLOG_NL_EVENT = 112;
constexpr int16_t LOCAL_NFLOG_PACKET = NFNL_SUBSYS_ULOG << 8 | NFULNL_MSG_PACKET;

constexpr int16_t ULOG_MAC_LEN = 80;
constexpr int16_t ULOG_PREFIX_LEN = 32;

constexpr int32_t ONE_BYTE = 8;
constexpr int32_t OPT_MAX = 3;
constexpr int32_t OPT_OFFSET = 0x1;
constexpr int32_t ADDR_OFFSET = 1;
constexpr int32_t ADDR_SITE = 2;
constexpr int32_t LEN_MAX = 256;
constexpr int32_t HEX_OFFSET_FOUR = 4;
constexpr int32_t HEX_OFFSET_FIVE = 5;
constexpr int32_t HEX_MULTIPLE = 2;
constexpr int32_t HEX_ALLOC_NUM = 1;

constexpr int32_t OPT_RDNSS = 25;
constexpr int32_t OPT_DNSSL = 31;
constexpr int32_t OPT_CAPTIVE_PORTAL = 37;
constexpr int32_t OPT_PREF64 = 38;
} // namespace
struct nd_opt_rdnss {
    u_int8_t nd_opt_rdnss_type;
    u_int8_t nd_opt_rdnss_len;
    u_int16_t nd_opt_rdnss_reserved;
    u_int32_t nd_opt_rdnss_lifetime;
};

struct ulog_packet_msg_t {
    uint64_t mark;
    int64_t timestamp_sec;
    int64_t timestamp_usec;
    uint32_t hook;
    char indev_name[IFNAMSIZ];
    char outdev_name[IFNAMSIZ];
    size_t data_len;
    char prefix[ULOG_PREFIX_LEN];
    uint8_t mac_len;
    uint8_t mac[ULOG_MAC_LEN];
    uint8_t payload[0];
};

NetlinkMessageDecoder::NetlinkMessageDecoder()
{
    action_ = Action::UNKNOW;
}

NetlinkMessageDecoder::~NetlinkMessageDecoder()
{
    params_.clear();
}

void NetlinkMessageDecoder::Dump()
{
    for (auto &param : params_) {
        NETNATIVE_LOG_D("NL param '%{public}s'\n", param.c_str());
    }
}

static const char *RtMessageName(int32_t type)
{
    switch (type) {
        case RTM_NEWLINK:
            return "RTM_NEWLINK";
        case RTM_DELLINK:
            return "RTM_DELLINK";
        case RTM_NEWADDR:
            return "RTM_NEWADDR";
        case RTM_DELADDR:
            return "RTM_DELADDR";
        case RTM_NEWROUTE:
            return "RTM_NEWROUTE";
        case RTM_DELROUTE:
            return "RTM_DELROUTE";
        case RTM_NEWNDUSEROPT:
            return "RTM_NEWNDUSEROPT";
        case LOCAL_QLOG_NL_EVENT:
            return "LOCAL_QLOG_NL_EVENT";
        case LOCAL_NFLOG_PACKET:
            return "LOCAL_NFLOG_PACKET";
        default:
            return nullptr;
    }
}

static bool CheckRtNetlinkLength(const nlmsghdr *nh, size_t size)
{
    if (nh->nlmsg_len < NLMSG_LENGTH(size)) {
        NETNATIVE_LOGE("Got a short %s message\n", RtMessageName(nh->nlmsg_type));
        return false;
    }
    return true;
}

static bool MaybeLogDuplicateAttribute(bool isDup, const char *attributeName, const char *messageName)
{
    if (isDup) {
        NETNATIVE_LOGE("Multiple %{public}s attributes in %{public}s, ignoring\n", attributeName, messageName);
        return true;
    }
    return false;
}

bool NetlinkMessageDecoder::ParseIfInfoMessage(const nlmsghdr *nh)
{
    ifinfomsg *ifi = reinterpret_cast<ifinfomsg *> (NLMSG_DATA(nh));
    if (!CheckRtNetlinkLength(nh, sizeof(*ifi))) {
        return false;
    }
    if ((ifi->ifi_flags & IFF_LOOPBACK) != 0) {
        return false;
    }
    int32_t len = IFLA_PAYLOAD(nh);
    rtattr *rta = nullptr;
    for (rta = IFLA_RTA(ifi); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if (rta == nullptr) {
            NETNATIVE_LOGE("Invalid ifinfomsg\n");
            return false;
        }
        switch (rta->rta_type) {
            case IFLA_IFNAME:
                params_[INDEX_ZERO] = "INTERFACE=" + std::string(reinterpret_cast<char *>RTA_DATA(rta));
                params_[INDEX_ONE] = "IFINDEX=" + std::to_string(ifi->ifi_index);
                action_ = (ifi->ifi_flags & IFF_LOWER_UP) ? Action::LINKUP : Action::LINKDOWN;
                subSystem_ = "net";
                return true;
            default:
                break;
        }
    }
    return false;
}

bool NetlinkMessageDecoder::ParseIfAddrMessage(const nlmsghdr *nh)
{
    ifaddrmsg *ifaddr = reinterpret_cast<ifaddrmsg *>(NLMSG_DATA(nh));
    ifa_cacheinfo *cacheinfo = nullptr;
    char addrstr[INET6_ADDRSTRLEN] = "";
    char ifname[IFNAMSIZ] = "";
    uint32_t flags;

    if (!CheckRtNetlinkLength(nh, sizeof(*ifaddr))) {
        return false;
    }
    int32_t type = nh->nlmsg_type;
    if (type != RTM_NEWADDR && type != RTM_DELADDR) {
        NETNATIVE_LOGE("parseIfAddrMessage on incorrect message type 0x%{public}x\n", type);
        return false;
    }
    const char *msgtype = RtMessageName(type);
    flags = ifaddr->ifa_flags;
    rtattr *rta = nullptr;
    int32_t len = IFA_PAYLOAD(nh);
    for (rta = IFA_RTA(ifaddr); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if (rta == nullptr) {
            NETNATIVE_LOGE("Invalid ifaddrmsg\n");
            return false;
        }
        if (rta->rta_type == IFA_ADDRESS) {
            if (!ProcessIFAddress(ifaddr, addrstr, sizeof(addrstr), msgtype, ifname, rta)) {
                continue;
            }
        } else if (rta->rta_type == IFA_CACHEINFO) {
            if (MaybeLogDuplicateAttribute(cacheinfo, "IFA_CACHEINFO", msgtype)) {
                continue;
            }
            if (RTA_PAYLOAD(rta) < sizeof(*cacheinfo)) {
                NETNATIVE_LOGE("Short IFA_CACHEINFO (%{public}zu vs. %zu bytes) in %{public}s", RTA_PAYLOAD(rta),
                               sizeof(*cacheinfo), msgtype);
                continue;
            }
            cacheinfo = reinterpret_cast<ifa_cacheinfo *>(RTA_DATA(rta));
        } else if (rta->rta_type == IFA_FLAGS) {
            flags = *(reinterpret_cast<uint32_t *>(RTA_DATA(rta)));
        }
    }
    if (addrstr[INDEX_ZERO] == '\0') {
        NETNATIVE_LOGE("No IFA_ADDRESS in %{public}s\n", msgtype);
        return false;
    }
    action_ = (type == RTM_NEWADDR) ? Action::ADDRESSUPDATE : Action::ADDRESSREMOVED;
    AddParam(addrstr, ifaddr, flags, cacheinfo, ifname);
    return true;
}

void NetlinkMessageDecoder::AddParam(const char *addrstr, const ifaddrmsg *ifaddr, uint32_t flags,
                                     const ifa_cacheinfo *cacheinfo, const char *ifname)
{
    subSystem_ = "net";
    params_[INDEX_ZERO] = "ADDRESS=" + std::string(addrstr) + "/" + std::to_string(ifaddr->ifa_prefixlen);
    params_[INDEX_ONE] = "INTERFACE=" + std::string(ifname);
    params_[INDEX_TWO] = "FLAGS=" + std::to_string(flags);
    params_[INDEX_THREE] = "SCOPE=" + std::to_string(ifaddr->ifa_scope);
    params_[INDEX_FOUR] = "IFINDEX=" + std::to_string(ifaddr->ifa_index);
    if (cacheinfo) {
        params_[INDEX_FIVE] = "PREFERRED=" + std::to_string(cacheinfo->ifa_prefered);
        params_[INDEX_SIX] = "VALID=" + std::to_string(cacheinfo->ifa_valid);
        params_[INDEX_SEVEN] = "CSTAMP=" + std::to_string(cacheinfo->cstamp);
        params_[INDEX_EIGHT] = "TSTAMP=" + std::to_string(cacheinfo->tstamp);
    }
}

bool NetlinkMessageDecoder::ProcessIFAddress(ifaddrmsg *ifaddr, char *addrstr, socklen_t len, const char *msgtype,
                                             char *ifname, rtattr *rta)
{
    if (MaybeLogDuplicateAttribute(*addrstr != '\0', "IFA_ADDRESS", msgtype)) {
        return false;
    }
    if (ifaddr->ifa_family == AF_INET) {
        in_addr *addr4 = reinterpret_cast<in_addr *>(RTA_DATA(rta));
        if (RTA_PAYLOAD(rta) < sizeof(*addr4)) {
            NETNATIVE_LOGE("Short IPv4 address (%{public}zu bytes) in %{public}s", RTA_PAYLOAD(rta), msgtype);
            return false;
        }
        inet_ntop(AF_INET, addr4, addrstr, len);
    } else if (ifaddr->ifa_family == AF_INET6) {
        in6_addr *addr6 = reinterpret_cast<in6_addr *>(RTA_DATA(rta));
        if (RTA_PAYLOAD(rta) < sizeof(*addr6)) {
            NETNATIVE_LOGE("Short IPv6 address (%{public}zu bytes) in %{public}s", RTA_PAYLOAD(rta), msgtype);
            return false;
        }
        inet_ntop(AF_INET6, addr6, addrstr, len);
    } else {
        NETNATIVE_LOGE("Unknown address family %{public}d\n", ifaddr->ifa_family);
        return false;
    }
    if (!if_indextoname(ifaddr->ifa_index, ifname)) {
        NETNATIVE_LOG_D("Unknown ifindex %d in %{public}s", ifaddr->ifa_index, msgtype);
    }
    return true;
}

bool NetlinkMessageDecoder::ParseUlogPacketMessage(const nlmsghdr *nh)
{
    std::string devname;
    ulog_packet_msg_t *pm = reinterpret_cast<ulog_packet_msg_t *>(NLMSG_DATA(nh));
    if (!CheckRtNetlinkLength(nh, sizeof(*pm))) {
        return false;
    }
    devname = pm->indev_name[INDEX_ZERO] ? pm->indev_name : pm->outdev_name;
    params_[INDEX_ZERO] = "ALERT_NAME=" + std::string(pm->prefix);
    params_[INDEX_ONE] = "INTERFACE=" + devname;
    subSystem_ = "qlog";
    action_ = Action::CHANGE;
    return true;
}

static size_t NlAttrLen(const nlattr *nla)
{
    return nla->nla_len - NLA_HDRLEN;
}

static const uint8_t *NlAttrData(const nlattr *nla)
{
    return reinterpret_cast<const uint8_t *>(nla) + NLA_HDRLEN;
}

static uint32_t NlAttrU32(const nlattr *nla)
{
    return *reinterpret_cast<const uint32_t *>(NlAttrData(nla));
}

bool NetlinkMessageDecoder::ParseNfPacketMessage(nlmsghdr *nh)
{
    int32_t uid = -1;
    int32_t len = 0;
    const char *raw = nullptr;

    nlattr *uid_attr = FindNlAttr(nh, sizeof(genlmsghdr), NFULA_UID);
    if (uid_attr) {
        uid = ntohl(NlAttrU32(uid_attr));
    }
    nlattr *payload = FindNlAttr(nh, sizeof(genlmsghdr), NFULA_PAYLOAD);
    if (payload) {
        len = NlAttrLen(payload);
        if (len > LEN_MAX) {
            len = LEN_MAX;
        }
        raw = reinterpret_cast<const char *>(NlAttrData(payload));
    }
    size_t hexSize = HEX_OFFSET_FIVE + (len * HEX_MULTIPLE);
    static const char *HEX_PREFIX = "HEX=";
    char *hex = reinterpret_cast<char *>(calloc(HEX_ALLOC_NUM, hexSize));
    if (hex == nullptr) {
        return false;
    }
    strcpy_s(hex, strlen(HEX_PREFIX), HEX_PREFIX);
    for (uint32_t i = 0; i < len; i++) {
        hex[HEX_OFFSET_FIVE + (i * HEX_MULTIPLE)] = "0123456789abcdef"[(raw[i] >> HEX_OFFSET_FOUR) & 0xf];
        hex[HEX_OFFSET_FIVE + (i * HEX_MULTIPLE)] = "0123456789abcdef"[raw[i] & 0xf];
    }
    params_[INDEX_ZERO] = "UID=" + std::to_string(uid);
    params_[INDEX_ONE] = hex;
    subSystem_ = "strict";
    action_ = Action::CHANGE;
    free(hex);
    return true;
}

bool NetlinkMessageDecoder::ParseRtMessage(const nlmsghdr *nh)
{
    uint8_t type = nh->nlmsg_type;
    const char *msgname = RtMessageName(type);
    rtmsg *rtm = nullptr;

    if ((rtm = CheckRtParam(nh, type)) == nullptr) {
        return false;
    }

    int32_t family = rtm->rtm_family;
    int32_t prefixLength = rtm->rtm_dst_len;
    char dst[INET6_ADDRSTRLEN] = "";
    char gw[INET6_ADDRSTRLEN] = "";
    char dev[IFNAMSIZ] = "";
    size_t len = RTM_PAYLOAD(nh);
    rtattr *rta = nullptr;
    for (rta = RTM_RTA(rtm); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        switch (rta->rta_type) {
            case RTA_DST:
                if (MaybeLogDuplicateAttribute(*dst, "RTA_DST", msgname)) {
                    continue;
                }
                if (!inet_ntop(family, RTA_DATA(rta), dst, sizeof(dst))) {
                    return false;
                }
                continue;
            case RTA_GATEWAY:
                if (MaybeLogDuplicateAttribute(*gw, "RTA_GATEWAY", msgname)) {
                    continue;
                }
                if (!inet_ntop(family, RTA_DATA(rta), gw, sizeof(gw))) {
                    return false;
                }
                continue;
            case RTA_OIF:
                if (MaybeLogDuplicateAttribute(*dev, "RTA_OIF", msgname)) {
                    continue;
                }
                if (!if_indextoname(*(reinterpret_cast<int32_t *>(RTA_DATA(rta))), dev)) {
                    return false;
                }
                continue;
            default:
                continue;
        }
    }
    if (!AddRtParam(dst, sizeof(dst), gw, dev, prefixLength, family)) {
        return false;
    }
    action_ = (type == RTM_NEWROUTE) ? Action::ROUTEUPDATED : Action::ROUTEREMOVED;
    return true;
}

bool NetlinkMessageDecoder::AddRtParam(char *dst, size_t dstLen, const char *gw, const char *dev, int32_t prefixLength,
                                       int32_t family)
{
    if (!*dst && !prefixLength) {
        if (family == AF_INET) {
            strncpy_s(dst, dstLen, "0.0.0.0", dstLen);
        } else if (family == AF_INET6) {
            strncpy_s(dst, dstLen, "::", dstLen);
        }
    }

    if (!*dst || (!*gw && !*dev)) {
        return false;
    }
    subSystem_ = "net";
    params_[INDEX_ZERO] = "ROUTE=" + std::string(dst) + "/" + std::to_string(prefixLength);
    params_[INDEX_ONE] = "GATEWAY=" + std::string((*gw) ? gw : "");
    params_[INDEX_TWO] = "INTERFACE=" + std::string((*dev) ? dev : "");
    return true;
}

rtmsg *NetlinkMessageDecoder::CheckRtParam(const nlmsghdr *nh, uint8_t type)
{
    if (type != RTM_NEWROUTE && type != RTM_DELROUTE) {
        NETNATIVE_LOGE("incorrect message type %{public}d\n", type);
        return nullptr;
    }

    rtmsg *rtm = reinterpret_cast<rtmsg *>(NLMSG_DATA(nh));
    if (!CheckRtNetlinkLength(nh, sizeof(*rtm))) {
        return nullptr;
    }

    if ((rtm->rtm_protocol != RTPROT_KERNEL && rtm->rtm_protocol != RTPROT_RA) ||
        (rtm->rtm_scope != RT_SCOPE_UNIVERSE) || (rtm->rtm_type != RTN_UNICAST) || (rtm->rtm_src_len != 0) ||
        (rtm->rtm_flags & RTM_F_CLONED)) {
        return nullptr;
    }
    return rtm;
}

/*
 * Parse a RTM_NEWNDUSEROPT message.
 */

bool NetlinkMessageDecoder::ParseNdOptRnss(nd_opt_hdr *opthdr, const char *ifname)
{
    uint16_t optlen = opthdr->nd_opt_len;
    if ((optlen < OPT_MAX) || !(optlen & OPT_OFFSET)) {
        NETNATIVE_LOGE("Invalid optlen %{public}d for RDNSS option\n", optlen);
        return false;
    }
    const int32_t numaddrs = (optlen - ADDR_OFFSET) / ADDR_SITE;
    // Find the lifetime.
    nd_opt_rdnss *rndss_opt = reinterpret_cast<nd_opt_rdnss *>(opthdr);
    const uint32_t lifetime = ntohl(rndss_opt->nd_opt_rdnss_lifetime);
    // Create a buffer to hold the message.
    static const size_t kMaxSingleAddressLength = INET6_ADDRSTRLEN + strlen("%") + IFNAMSIZ + strlen(",");
    const size_t bufsize = numaddrs * kMaxSingleAddressLength;
    auto buf = std::make_unique<char[]>(bufsize);
    memset_s(buf.get(), bufsize, 0, bufsize);

    in6_addr *addrs = reinterpret_cast<in6_addr *>(rndss_opt + ADDR_OFFSET);
    size_t pos = 0;
    for (int32_t i = 0; i < numaddrs; i++) {
        if (i > 0) {
            buf[pos++] = ',';
        }
        inet_ntop(AF_INET6, addrs + i, buf.get() + pos, bufsize - pos);
        pos += strlen(buf.get() + pos);
        if (IN6_IS_ADDR_LINKLOCAL(addrs + i)) {
            buf[pos++] = '%';
            pos += strlcpy(buf.get() + pos, ifname, bufsize - pos);
        }
    }
    buf[pos] = '\0';
    params_[INDEX_ZERO] = "INTERFACE=" + std::string(ifname);
    params_[INDEX_ONE] = "LIFETIME=" + std::to_string(lifetime);
    params_[INDEX_TWO] = "SERVERS=" + std::string(buf.get());
    return true;
}

bool NetlinkMessageDecoder::ParseNdUserOptMessage(const nlmsghdr *nh)
{
    nduseroptmsg *msg = reinterpret_cast<nduseroptmsg *>(NLMSG_DATA(nh));
    if (!CheckRtNetlinkLength(nh, sizeof(*msg))) {
        return false;
    }
    // Get the length of the options and check that it is not too long.
    int32_t len = NLMSG_PAYLOAD(nh, sizeof(*msg));
    if (msg->nduseropt_opts_len > len) {
        NETNATIVE_LOGE("RTM_NEWNDUSEROPT invalid length %{public}d > %{public}d\n", msg->nduseropt_opts_len, len);
        return false;
    }
    len = msg->nduseropt_opts_len;

    // Check that the length of the options is a multiple of 8.
    if (msg->nduseropt_family != AF_INET6) {
        NETNATIVE_LOGE("RTM_NEWNDUSEROPT message for unknown family %{public}d\n", msg->nduseropt_family);
        return false;
    }

    if (msg->nduseropt_icmp_type != ND_ROUTER_ADVERT || msg->nduseropt_icmp_code != 0) {
        NETNATIVE_LOGE("RTM_NEWNDUSEROPT message for unknown ICMPv6 type/code %{public}d/%{public}d\n",
                       msg->nduseropt_icmp_type, msg->nduseropt_icmp_code);
        return false;
    }

    // Find the interface is the interface index.
    char ifname[IFNAMSIZ];
    if (!if_indextoname(msg->nduseropt_ifindex, ifname)) {
        NETNATIVE_LOGE("RTM_NEWNDUSEROPT on unknown ifindex %{public}d\n", msg->nduseropt_ifindex);
        return false;
    }

    // Kernel will send the message with the following options: type, length,
    nd_opt_hdr *opthdr = reinterpret_cast<nd_opt_hdr *>(msg + 1);

    // The first option should be the source link-layer address option.
    uint16_t optlen = opthdr->nd_opt_len;
    if (optlen * ONE_BYTE > len) {
        NETNATIVE_LOGE("Invalid option length %{public}d > %{public}d for ND option %{public}d\n", optlen * ONE_BYTE,
                       len, opthdr->nd_opt_type);
        return false;
    }

    switch (opthdr->nd_opt_type) {
        case OPT_RDNSS: {
            // The RDNSS option is a list of DNS servers.
            ParseNdOptRnss(opthdr, ifname);
            action_ = Action::RDNSS;
            subSystem_ = "net";
            break;
        }
        case OPT_DNSSL:
            break;
        case OPT_CAPTIVE_PORTAL:
            break;
        case OPT_PREF64:
            break;
        default:
            NETNATIVE_LOG_D("Unknown ND option type %{public}d\n", opthdr->nd_opt_type);
            return false;
    }
    return true;
}

bool NetlinkMessageDecoder::ParseBinaryNetlinkMessage(char *buffer, int32_t size)
{
    nlmsghdr *nh = nullptr;
    bool result = false;
    for (nh = reinterpret_cast<nlmsghdr *>(buffer); NLMSG_OK(nh, (unsigned)size) && (nh->nlmsg_type != NLMSG_DONE);
         nh = NLMSG_NEXT(nh, size)) {
        if (!RtMessageName(nh->nlmsg_type)) {
            NETNATIVE_LOG_D("Unexpected netlink message type %{public}d\n", nh->nlmsg_type);
            continue;
        }
        switch (nh->nlmsg_type) {
            case RTM_NEWLINK:
                result = ParseIfInfoMessage(nh);
                break;
            case LOCAL_QLOG_NL_EVENT:
                result = ParseUlogPacketMessage(nh);
                break;
            case RTM_NEWADDR:
            case RTM_DELADDR:
                result = ParseIfAddrMessage(nh);
                break;
            case RTM_NEWROUTE:
            case RTM_DELROUTE:
                result = ParseRtMessage(nh);
                break;
            case RTM_NEWNDUSEROPT:
                result = ParseNdUserOptMessage(nh);
                break;
            case LOCAL_NFLOG_PACKET:
                result = (ParseNfPacketMessage(nh));
                break;
            default:
                result = false;
                NETNATIVE_LOG_D("Unknown netlink message type %{public}d\n", nh->nlmsg_type);
                break;
        }
        if (result) {
            return true;
        }
    }
    return false;
}

const std::string GetElement(const std::string &source, const std::string relax)
{
    auto str = Split(source, relax);
    return str[0];
}

bool NetlinkMessageDecoder::ParseAsciiNetlinkMessage(char *buffer, int32_t size)
{
    std::string buf = buffer;
    char *start = buffer;
    char *end = start + size;
    std::vector<std::string> recvmsg;
    if (size == 0) {
        return false;
    }
    auto msg = Split(buf, "@");
    path_ = msg[INDEX_ONE];
    if (path_.empty()) {
        return false;
    }
    auto action = msg[0];
    if (action.empty()) {
        return false;
    }

    // Skip the first line.
    start += strlen(start) + 1;
    while (start < end) {
        if (start != nullptr) {
            recvmsg.emplace_back(start);
        }
        // Skip to next line.
        start += strlen(start) + 1;
    }

    // Split the message and push them into params.
    for (auto &i : recvmsg) {
        if (i.find("ACTION=") != std::string::npos) {
            if (i.find("add") != std::string::npos) {
                action_ = Action::ADD;
            } else if (i.find("remove") != std::string::npos) {
                action_ = Action::REMOVE;
            } else if (i.find("change") != std::string::npos) {
                action_ = Action::CHANGE;
            }
        } else if (i.find("SEQNUM=") != std::string::npos) {
            auto seq = Split(i, "=");
            if (seq.size() == SPLIT_SIZE) {
                seq_ = std::strtol(seq[1].c_str(), nullptr, DECIMALISM);
            }
        } else if (i.find("SUBSYSTEM=") != std::string::npos) {
            auto subsys = Split(i, "=");
            if (subsys.size() == SPLIT_SIZE) {
                subSystem_ = subsys[1];
            }
        } else {
            params_.emplace_back(i);
        }
    }
    return true;
}

bool NetlinkMessageDecoder::Decode(char *buffer, int32_t size, int32_t format)
{
    if (format == NETLINK_FORMAT_BINARY || format == NETLINK_FORMAT_BINARY_UNICAST) {
        return ParseBinaryNetlinkMessage(buffer, size);
    } else {
        return ParseAsciiNetlinkMessage(buffer, size);
    }
}

const std::string NetlinkMessageDecoder::FindParam(const char *paramName)
{
    for (auto &i : params_) {
        size_t index = i.find(paramName);
        if (index != std::string::npos) {
            return i.substr(index + strlen(paramName) + 1);
        }
    }
    return "";
}

nlattr *NetlinkMessageDecoder::FindNlAttr(nlmsghdr *nh, size_t hdrlen, uint16_t attr)
{
    if (nh == nullptr || NLMSG_HDRLEN + NLMSG_ALIGN(hdrlen) > SSIZE_MAX) {
        return nullptr;
    }

    const ssize_t NLA_START = NLMSG_HDRLEN + NLMSG_ALIGN(hdrlen);
    ssize_t left = nh->nlmsg_len - NLA_START;
    uint8_t *hdr = (reinterpret_cast<uint8_t *>(nh)) + NLA_START;

    while (left >= NLA_HDRLEN) {
        nlattr *nla = reinterpret_cast<nlattr *>(hdr);
        if (nla->nla_type == attr) {
            return nla;
        }

        hdr += NLA_ALIGN(nla->nla_len);
        left -= NLA_ALIGN(nla->nla_len);
    }

    return nullptr;
}
} // namespace nmd
} // namespace OHOS
