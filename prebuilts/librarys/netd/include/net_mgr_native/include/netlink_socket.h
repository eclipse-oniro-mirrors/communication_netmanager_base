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
class netlink_socket {
public:
    int socketFd_;
    int pid_ = 0;

    virtual ~netlink_socket();

    void setSock(int sock)
    {
        this->socketFd_ = sock;
    }

    int create(int protocol);
    int create(int type, int protocol);
    int binding();

    int acceptAndListen();

    int sendNetlinkMsgToKernel(struct nlmsghdr *msg);

    ssize_t receive(void *buf);

    int shutdown();

    void setOnDataReceiveHandler(const std::function<void(int, char *, ssize_t)> &handler);

    void setPid(int pid)
    {
        this->pid_ = pid;
    }

private:
    std::function<void(int, char *, ssize_t)> handler_;
    struct sockaddr addr_ {};
    /**
     * Link layer: RTM_NEWLINK, RTM_DELLINK, RTM_GETLINK, RTM_SETLINK
     * Address settings: RTM_NEWADDR, RTM_DELADDR, RTM_GETADDR
     * Routing tables: RTM_NEWROUTE, RTM_DELROUTE, RTM_GETROUTE
     * Neighbor cache: RTM_NEWNEIGH, RTM_DELNEIGH, RTM_GETNEIGH
     * Routing rules: RTM_NEWRULE, RTM_DELRULE, RTM_GETRULE
     * Queuing discipline settings: RTM_NEWQDISC, RTM_DELQDISC, RTM_GETQDISC
     * Traffic classes used with queues: RTM_NEWTCLASS, RTM_DELTCLASS, RTM_GETTCLASS
     * Traffic filters: RTM_NEWTFILTER, RTM_DELTFILTER, RTM_GETTFILTER
     * Others: RTM_NEWACTION, RTM_DELACTION, RTM_GETACTION, RTM_NEWPREFIX, RTM_GETPREFIX, RTM_GETMULTICAST,
     * RTM_GETANYCAST, RTM_NEWNEIGHTBL, RTM_GETNEIGHTBL, RTM_SETNEIGHTBL
     */
    int send(unsigned short action, char *buffer, size_t size, unsigned short rtaType, char *attrBuf,
        size_t attrBufLen);
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_SOCKET_H__