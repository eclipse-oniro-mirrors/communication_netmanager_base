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

#ifndef INCLUDE_NETLINK_MSG_H__
#define INCLUDE_NETLINK_MSG_H__

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/fib_rules.h>
#include <linux/inet_diag.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace OHOS {
namespace nmd {
static const uint32_t NETLINK_MAX_LEN = 1024;

class NetlinkMsg {
public:
    NetlinkMsg(uint16_t flags, size_t maxBufLen, int pid);
    ~NetlinkMsg();

    void AddRoute(unsigned short action, struct rtmsg msg);
    void AddRule(unsigned short action, struct fib_rule_hdr msg);
    void AddAddress(unsigned short action, struct ifaddrmsg msg);
    int AddAttr(unsigned int rtaType, void *buf, size_t bufLen);
    int AddAttr16(unsigned int rtaType, uint16_t data);
    int AddAttr32(unsigned int rtaType, uint32_t data);

    struct nlmsghdr *GetNetLinkMessage();
private:
    struct nlmsghdr *netlinkMessage;
    size_t maxBufLen;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_MSG_H__
