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

#include "netlink_socket.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "securec.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
NetlinkSocket::~NetlinkSocket()
{
    close(this->socketFd);
}

int NetlinkSocket::Create(int protocol)
{
    return this->Create(SOCK_RAW, protocol);
}

int NetlinkSocket::Create(int type, int protocol)
{
    this->socketFd = socket(AF_NETLINK, type, protocol);
    if (this->socketFd == -1) {
        NETNATIVE_LOGE("[NetlinkSocket] create socket failed: %{public}d", errno);
        return -1;
    }
    return this->socketFd;
}

int NetlinkSocket::SendNetlinkMsgToKernel(struct nlmsghdr *msg)
{
    if (msg == nullptr) {
        NETNATIVE_LOGE("[NetlinkSocket] msg can not be null ");
        return -1;
    }

    struct iovec ioVector;
    ioVector.iov_base = msg;
    ioVector.iov_len = msg->nlmsg_len;

    struct msghdr msgHeader;
    (void)memset_s(&msgHeader, sizeof(msgHeader), 0, sizeof(msgHeader));

    struct sockaddr_nl kernel;
    memset_s(&kernel, sizeof(kernel), 0, sizeof(kernel));
    kernel.nl_family = AF_NETLINK;
    kernel.nl_groups = 0;

    msgHeader.msg_name = &kernel;
    msgHeader.msg_namelen = sizeof(kernel);
    msgHeader.msg_iov = &ioVector;
    msgHeader.msg_iovlen = 1;

    ssize_t msgState = sendmsg(this->socketFd, &msgHeader, 0);
    if (msgState == -1) {
        NETNATIVE_LOGE("[NetlinkSocket] msg can not be null ");
        return -1;
    } else if (msgState == 0) {
        NETNATIVE_LOGE("[NetlinkSocket] 0 bytes send.");
        return -1;
    }
    return msgState;
}

int NetlinkSocket::Shutdown()
{
    return close(this->socketFd);
}
} // namespace nmd
} // namespace OHOS