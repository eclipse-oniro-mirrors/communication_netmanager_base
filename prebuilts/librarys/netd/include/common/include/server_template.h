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

#ifndef INCLUDE_SERVER_TEMPLATE_H__
#define INCLUDE_SERVER_TEMPLATE_H__
#include <string>
#include <memory>
#include "server_socket.h"
#include "thread_pool.h"
namespace OHOS {
namespace nmd {
namespace common {
class server_template {
    const char *const SOCKET_FILE_PATH = "/dev/socket";

public:
    void start();
    void stop();
    void handler(int socketFd, const uint8_t *msg, const size_t msgLen);

public:
    explicit server_template(const char *socketName, const char *serverName)
        : socketName_(socketName), serverName_(serverName), server_(std::make_shared<nmd::common::server_socket>()),
          pool_(std::make_shared<nmd::thread_pool>(16, 256)), job_(nullptr)
    {}
    virtual ~server_template() = default;

protected:
    virtual void initJob(const int socketFd, const uint8_t *msg, const size_t msgLen) = 0;

protected:
    std::string socketName_;
    std::string serverName_;
    std::shared_ptr<nmd::common::server_socket> server_;
    std::shared_ptr<thread_pool> pool_;
    nmd::job *job_;
    bool mRunning = false;
};
} // namespace common
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_SERVER_TEMPLATE_H__