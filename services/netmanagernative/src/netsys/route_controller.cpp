/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "route_controller.h"
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <netlink_socket.h>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <linux/fib_rules.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "securec.h"
#include "bitcast.h"
#include "netlink_manager.h"
#include "netlink_msg.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
namespace {
    constexpr uint32_t OUTPUT_MAX = 128;
    constexpr uint32_t BIT_32_LEN = 32;
    constexpr uint32_t BIT_MAX_LEN = 255;
    constexpr uint32_t DECIMAL_DIGITAL = 10;
    constexpr uint32_t BYTE_ALIGNMENT = 8;
    constexpr uint32_t THOUSAND_LEN = 1000;

    constexpr uint32_t RULE_LOCAL_NETWORK_PRI = 18000;
    constexpr uint32_t RULE_DEFAULT_NETWORK_PRI = 19000;

    constexpr uint32_t ROUTE_LOCAL_NETWORK_TABLE = 100;
}
std::map<std::string, uint32_t> RouteController::interfaceToTable;

RouteController::RouteController()
{
    int status = ModifyRule(RTM_NEWRULE, ROUTE_LOCAL_NETWORK_TABLE, FR_ACT_TO_TBL, RULE_LOCAL_NETWORK_PRI);
    if (status < 0) {
        NETNATIVE_LOGE("RouteController::RouteController, add rule error");
    }
}

RouteController::~RouteController() {}

int RouteController::ModifyRule(uint32_t type, uint32_t table, uint8_t action, uint32_t priority)
{
    nmd::NetlinkSocket netLinker;
    netLinker.Create(NETLINK_ROUTE);
    nmd::NetlinkMsg nlmsg(NLM_F_CREATE | NLM_F_EXCL, nmd::NETLINK_MAX_LEN, NetlinkManager::GetPid());

    struct fib_rule_hdr msg = {0};

    msg.action = action;
    msg.family = AF_INET;
    msg.table = RT_TABLE_UNSPEC;

    nlmsg.AddRule(type, msg);
    nlmsg.AddAttr32(FRA_PRIORITY, priority);
    nlmsg.AddAttr32(FRA_TABLE, table);

    return netLinker.SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
}

int RouteController::AddInterfaceToDefaultNetwork(const char *interfaceName, NetworkPermission permission)
{
    NETNATIVE_LOGI("Entry RouteController::AddInterfaceToDefaultNetwork, %{public}s", interfaceName);

    uint32_t table = GetRouteTableForInterface(interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        return -ESRCH;
    }

    return ModifyRule(RTM_NEWRULE, table, FR_ACT_TO_TBL, RULE_DEFAULT_NETWORK_PRI);
}

int RouteController::RemoveInterfaceFromDefaultNetwork(const char *interfaceName, NetworkPermission permission)
{
    NETNATIVE_LOGI("Entry RouteController::AddInterfaceToDefaultNetwork, %{public}s", interfaceName);

    uint32_t table = GetRouteTableForInterface(interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        return -ESRCH;
    }

    return ModifyRule(RTM_DELRULE, table, FR_ACT_TO_TBL, RULE_DEFAULT_NETWORK_PRI);
}

int nmd::RouteController::ReadAddrGw(const char *addr, InetAddr *res)
{
    std::string addressString(addr);
    if (strchr(addr, ':')) {
        res->family = AF_INET6;
        res->bitlen = OUTPUT_MAX;
    } else {
        res->family = AF_INET;
        res->bitlen = BIT_32_LEN;
    }

    return inet_pton(res->family, addressString.c_str(), res->data);
}

int nmd::RouteController::ReadAddr(const char *addr, InetAddr *res)
{
    const char *slashStr = strchr(addr, '/');
    if (slashStr == nullptr) {
        return -EINVAL;
    }

    const char *maskLenStr = slashStr + 1;
    if (*maskLenStr == 0) {
        return -EINVAL;
    }

    char *endptr = nullptr;
    unsigned templen = strtoul(maskLenStr, &endptr, DECIMAL_DIGITAL);
    if ((endptr == nullptr) || (templen > BIT_MAX_LEN)) {
        return -EINVAL;
    }
    res->prefixlen = templen;

    std::string addressString(addr, slashStr - addr);
    if (strchr(addr, ':')) {
        res->family = AF_INET6;
        res->bitlen = OUTPUT_MAX;
    } else {
        res->family = AF_INET;
        res->bitlen = BIT_32_LEN;
    }

    return inet_pton(res->family, addressString.c_str(), res->data);
}

int nmd::RouteController::AddRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    NETNATIVE_LOGE("Entry nmd::RouteController::AddRoute");

    nmd::NetlinkSocket netLinker;
    netLinker.Create(NETLINK_ROUTE);
    nmd::NetlinkMsg nlmsg(NLM_F_CREATE | NLM_F_EXCL, nmd::NETLINK_MAX_LEN, NetlinkManager::GetPid());

    struct rtmsg msg;
    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));

    msg.rtm_family = AF_INET;
    msg.rtm_dst_len = BIT_32_LEN;
    msg.rtm_protocol = RTPROT_STATIC;
    msg.rtm_scope = RT_SCOPE_UNIVERSE;
    msg.rtm_type = RTN_UNICAST;
    msg.rtm_table = RT_TABLE_UNSPEC;

    unsigned int table;
    if (netId == nmd::LOCAL_NETWORK_NETID) {
        table = ROUTE_LOCAL_NETWORK_TABLE;
    } else {
        table = GetRouteTableForInterface(interfaceName.c_str());
        if (table == RT_TABLE_UNSPEC) {
            return -1;
        }
    }

    InetAddr dst;
    int readAddrResult = ReadAddr(destination.c_str(), &dst);
    if (readAddrResult != 1) {
        NETNATIVE_LOGE("dest parse failed:%{public}d", readAddrResult);
        return -1;
    }
    msg.rtm_family = dst.family;
    msg.rtm_dst_len = dst.prefixlen;
    if (dst.family == AF_INET) {
        msg.rtm_scope = RT_SCOPE_LINK;
    } else if (dst.family == AF_INET6) {
        msg.rtm_scope = RT_SCOPE_UNIVERSE;
    }
    NETNATIVE_LOGI("msg.rtm_dst_len:%{public}d, table:%{public}u", dst.prefixlen, table);

    InetAddr gw;
    readAddrResult = ReadAddrGw(nextHop.c_str(), &gw);
    if (readAddrResult != 1) {
        NETNATIVE_LOGE("gw parse failed:%{public}d", readAddrResult);
        return -1;
    }
    if (gw.bitlen != 0) {
        msg.rtm_scope = RT_SCOPE_UNIVERSE;
        msg.rtm_family = gw.family;
    }

    unsigned int index = if_nametoindex(interfaceName.c_str());

    nlmsg.AddRoute(RTM_NEWROUTE, msg);
    nlmsg.AddAttr32(RTA_TABLE, table);
    nlmsg.AddAttr(RTA_DST, (void *)dst.data, dst.bitlen / BYTE_ALIGNMENT);
    nlmsg.AddAttr(RTA_GATEWAY, (void *)gw.data, gw.bitlen / BYTE_ALIGNMENT);
    nlmsg.AddAttr32(RTA_OIF, index);

    netLinker.SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
    NETNATIVE_LOGI("nmd::RouteController::AddRoute:%{public}d %{public}s %{public}s %{public}s",
        netId, interfaceName.c_str(), destination.c_str(), nextHop.c_str());

    return 1;
}

int RouteController::RemoveRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    nmd::NetlinkSocket netLinker;
    netLinker.Create(NETLINK_ROUTE);
    nmd::NetlinkMsg nlmsg(NLM_F_CREATE | NLM_F_EXCL, nmd::NETLINK_MAX_LEN, NetlinkManager::GetPid());

    struct rtmsg msg;
    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));

    msg.rtm_family = AF_INET;
    msg.rtm_dst_len = BIT_32_LEN;
    msg.rtm_scope = RT_SCOPE_UNIVERSE;
    msg.rtm_table = RT_TABLE_UNSPEC;

    unsigned int table;
    if (netId == nmd::LOCAL_NETWORK_NETID) {
        table = ROUTE_LOCAL_NETWORK_TABLE;
    } else {
        table = GetRouteTableForInterface(interfaceName.c_str());
        if (table == RT_TABLE_UNSPEC) {
            return -1;
        }
    }

    InetAddr dst;
    int readAddrResult = ReadAddr(destination.c_str(), &dst);
    if (readAddrResult != 1) {
        NETNATIVE_LOGE("dest parse failed:%{public}d", readAddrResult);
        return -1;
    }
    msg.rtm_family = dst.family;
    msg.rtm_dst_len = dst.prefixlen;
    if (dst.family == AF_INET) {
        msg.rtm_scope = RT_SCOPE_LINK;
    } else if (dst.family == AF_INET6) {
        msg.rtm_scope = RT_SCOPE_UNIVERSE;
    }

    InetAddr gw;
    readAddrResult = ReadAddrGw(nextHop.c_str(), &gw);
    if (readAddrResult != 1) {
        NETNATIVE_LOGE("gw parse failed:%{public}d", readAddrResult);
        return -1;
    }
    if (gw.bitlen != 0) {
        msg.rtm_scope = 0;
        msg.rtm_family = gw.family;
    }

    unsigned int index = if_nametoindex(interfaceName.c_str());

    nlmsg.AddRoute(RTM_DELROUTE, msg);
    nlmsg.AddAttr32(RTA_TABLE, table);
    nlmsg.AddAttr(RTA_DST, (void *)dst.data, dst.bitlen / BYTE_ALIGNMENT);
    nlmsg.AddAttr(RTA_GATEWAY, (void *)gw.data, gw.bitlen / BYTE_ALIGNMENT);
    nlmsg.AddAttr32(RTA_OIF, index);

    netLinker.SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
    NETNATIVE_LOGI("nmd::RouteController::RemoveRoute:%{public}d %{public}s %{public}s %{public}s",
        netId, interfaceName.c_str(), destination.c_str(), nextHop.c_str());

    return 1;
}

uint32_t RouteController::GetRouteTableForInterface(const char *interfaceName)
{
    auto iter = interfaceToTable.find(interfaceName);
    if (iter != interfaceToTable.end()) {
        return iter->second;
    }

    uint32_t table = if_nametoindex(interfaceName);
    if (table == 0) {
        NETNATIVE_LOGE(
            "[RouteController] cannot find interface %{public}s, error:%{public}d", interfaceName, errno);
        return RT_TABLE_UNSPEC;
    }
    table += THOUSAND_LEN;
    interfaceToTable[interfaceName] = table;
    return table;
}
} // namespace nmd
} // namespace OHOS