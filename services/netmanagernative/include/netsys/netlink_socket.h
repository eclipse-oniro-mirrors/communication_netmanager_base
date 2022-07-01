/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
class NetlinkSocket {
public:
    int socketFd;

    virtual ~NetlinkSocket();

    void SetPid(int pid)
    {
        this->pid = pid;
    }

    int Create(int protocol);
    int Create(int type, int protocol);
    int SendNetlinkMsgToKernel(nlmsghdr *msg);
    int Shutdown();
private:
    int pid = 0;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_SOCKET_H__
