/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <arpa/inet.h>
#include <asm/types.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/fib_rules.h>
#include <linux/inet_diag.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace OHOS {
namespace nmd {
constexpr uint32_t NETLINK_MAX_LEN = 1024;
constexpr uint16_t NETLINK_REQUEST_FLAGS = NLM_F_REQUEST | NLM_F_ACK;
constexpr uint16_t NETLINK_ROUTE_CREATE_FLAGS = NETLINK_REQUEST_FLAGS | NLM_F_CREATE | NLM_F_EXCL;
constexpr uint16_t NETLINK_ROUTE_REPLACE_FLAGS = NETLINK_REQUEST_FLAGS | NLM_F_REPLACE;
constexpr uint16_t NETLINK_RULE_CREATE_FLAGS = NETLINK_REQUEST_FLAGS | NLM_F_CREATE;
class NetlinkMsg {
public:
    NetlinkMsg(uint16_t flags, size_t maxBufLen, int32_t pid);
    ~NetlinkMsg();
    /**
     * @brief Add route message to nlmsghdr
     *
     * @param action Action name
     * @param msg Added message
     */
    void AddRoute(uint16_t action, struct rtmsg msg);

    /**
     * @brief Add rule message to nlmsghdr
     *
     * @param action Action name
     * @param msg Added message
     */
    void AddRule(uint16_t action, struct fib_rule_hdr msg);

    /**
     * @brief Add address message to nlmsghdr
     *
     * @param action Action name
     * @param msg Added message
     */
    void AddAddress(uint16_t action, struct ifaddrmsg msg);

    /**
     * @brief Add rtattr to nlmsghdr
     *
     * @param rtaType Rta type
     * @param buf Rta data
     * @param bufLen Rta data length
     * @return Returns 0, add rtattr to nlmsghdr successfully, otherwise it will fail
     */
    int32_t AddAttr(uint16_t rtaType, void *data, size_t dataLen);

    /**
     * @brief Add 16 bit rtattr to nlmsghdr
     *
     * @param rtaType Rta type
     * @param data Rta data
     * @return Returns 0, add 16 bit rtattr to nlmsghdr successfully, otherwise it will fail
     */
    int32_t AddAttr16(uint16_t rtaType, uint16_t data);

    /**
     * @brief Add 32 bit rtattr to nlmsghdr for
     *
     * @param rtaType Rta type
     * @param data Rta data
     * @return Returns 0, add 32 bit rtattr to nlmsghdr successfully, otherwise it will fail
     */
    int32_t AddAttr32(uint16_t rtaType, uint32_t data);

    /**
     * @brief Get the netlink message
     *
     * @return Netlink message struct
     */
    struct nlmsghdr *GetNetLinkMessage();

private:
    struct nlmsghdr *netlinkMessage;
    size_t maxBufLen;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_MSG_H__