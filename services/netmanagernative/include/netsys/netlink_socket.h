/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_NETLINK_SOCKET_H__
#define INCLUDE_NETLINK_SOCKET_H__

#include <functional>
#include <memory>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace OHOS {
namespace nmd {
constexpr uint32_t KNETLINK_DUMP_BUFFER_SIZE = 8192;
constexpr uint32_t LOCAL_PRIORITY = 32767;
using NetlinkDumpCallback = std::function<void(nlmsghdr *)>;
/**
 * Send netklink message to kernel
 *
 * @param msg nlmsghdr struct
 * @return Returns 0, send netklink message to kernel successfully, otherwise it will fail
 */
int32_t SendNetlinkMsgToKernel(nlmsghdr *msg);

/**
 * Flush route or rule configure
 *
 * @param getAction Decide to flush route or rule. Must be one of RTM_GETRULE/RTM_GETROUTE
 * @param deleteAction Decide to flush route or rule. Must be one of RTM_DELRULE/RTM_DELROUTE
 * @param what Decide to flush route or rule. Must be one of "rules"/"routes"
 * @param table If refresh route，this is table number, otherwise it will is 0
 * @return Returns 0, flush route or rule configure successfully, otherwise it will fail
 */
int32_t RtNetlinkFlush(uint16_t getAction, uint16_t deleteAction, const char *what, uint32_t table);
int32_t OpenNetlinkSocket(int32_t protocol);
int32_t RecvNetlinkAck(int32_t sock);
int32_t SendNetlinkRequest(uint16_t action, uint16_t flags, iovec *iov, int32_t iovlen,
    const NetlinkDumpCallback *callback);
int32_t ProcessNetlinkDump(int32_t sock, const NetlinkDumpCallback &callback);
uint32_t GetRtmU32Attribute(const nlmsghdr *nlh, int32_t attribute);
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_SOCKET_H__
