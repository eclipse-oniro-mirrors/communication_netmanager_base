/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef NETLINK_EVENT_H__
#define NETLINK_EVENT_H__

#include <string>

#include <netinet/icmp6.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "netlink_define.h"

namespace OHOS {
namespace nmd {
using namespace NetlinkDefine;
class NetlinkMessageDecoder {
public:
    enum class Action {
        UNKNOW = 0,
        ADD = 1,
        REMOVE = 2,
        CHANGE = 3,
        LINKUP = 4,
        LINKDOWN = 5,
        ADDRESSUPDATE = 6,
        ADDRESSREMOVED = 7,
        RDNSS = 8,
        ROUTEUPDATED = 9,
        ROUTEREMOVED = 10,
    };

    NetlinkMessageDecoder();
    virtual ~NetlinkMessageDecoder();

    bool Decode(char *buffer, int32_t size, int32_t format = NETLINK_FORMAT_ASCII);
    const std::string FindParam(const char *paramName);

    void Dump();

    inline const std::string &GetSubsystem()
    {
        return subSystem_;
    }

    inline const Action &GetAction()
    {
        return action_;
    }

private:
    static constexpr int32_t NL_PARAMS_MAX = 32;
    static constexpr int32_t SPLIT_SIZE = 2;
    int64_t seq_ = 0;
    std::string path_;
    Action action_;
    std::string subSystem_;
    std::vector<std::string> params_ = std::vector<std::string>(NL_PARAMS_MAX);

    bool ParseBinaryNetlinkMessage(const char *buffer, int32_t size);
    bool ParseAsciiNetlinkMessage(char *buffer, int32_t size);
    bool ParseIfInfoMessage(const nlmsghdr *nh);
    bool ParseIfAddrMessage(const nlmsghdr *nh);
    bool ParseUlogPacketMessage(const nlmsghdr *nh);
    bool ParseNfPacketMessage(nlmsghdr *nh);
    bool ParseRtMessage(const nlmsghdr *nh);
    bool ParseNdUserOptMessage(const nlmsghdr *nh);
    bool ParseNdOptRnss(const nd_opt_hdr *opthdr, const char *ifname);
    nlattr *FindNlAttr(const nlmsghdr *nl, size_t hdrlen, uint16_t attr);
    bool ProcessIFAddress(ifaddrmsg *ifaddr,
                          char *addrstr,
                          socklen_t len,
                          const char *msgtype,
                          char *ifname,
                          rtattr *rta);
    void AddParam(const char *addrstr,
                  const ifaddrmsg *ifaddr,
                  uint32_t flags,
                  const ifa_cacheinfo *cacheinfo,
                  const char *ifname);
    rtmsg* CheckRtParam(const nlmsghdr *nh, uint8_t type);
    bool AddRtParam(char *dst,
                    size_t dstLen,
                    const char *gw,
                    const char *dev,
                    int32_t prefixLength,
                    int32_t family);
};
} // namespace nmd
} // namespace OHOS

#endif // NETLINK_EVENT_H__
