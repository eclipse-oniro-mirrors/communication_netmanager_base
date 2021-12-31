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

#ifndef NETD_SOCKET_BASE_H__
#define NETD_SOCKET_BASE_H__

#include <functional>
#include <memory>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
namespace common {
typedef struct end_point {
    int port;
} end_point;
class socket_base {
public:
    socket_base();
    virtual ~socket_base();

    int createInet();
    int createUnix();
    int listenSocket();
    int acceptSocket();
    int connectSocket(struct sockaddr_in serverAddr);
    ssize_t sendSocket(int socketFd, const char *buffer);
    ssize_t sendSocket(const char *buffer);
    virtual ssize_t sendMsg(const int socketFd, const msghdr &msg);
    char *receiveSocket(char *buffer);

    template<typename R, typename... Params>
    void setRecevedHandler(R (*)(Params...))
    {}

    template<typename R, typename C, typename... Params>
    void setRecevedHandler(R (C::*func)(Params...), C *instance)
    {
        this->handler_ =
            std::bind(func, instance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    int getSocketFileDescriptor()
    {
        return this->socketFd_;
    }

protected:
    int socketFd_;
    int epollFd_ = 0;
    int eventCnt_ = 0;
    struct epoll_event *epollEvents_;
    struct epoll_event event_ {};
    std::function<void(const int, const uint8_t *, const size_t)> handler_;

private:
    int create(int domain, int protocol);
};
} // namespace common
} // namespace nmd
} // namespace OHOS
#endif // !NETD_SOCKET_BASE_H__
