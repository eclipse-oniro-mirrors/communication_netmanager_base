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

#ifndef INCLUDE_JOB_H__
#define INCLUDE_JOB_H__

#include <vector>
#include "server_socket.h"
namespace OHOS {
namespace nmd {
class job {
public:
    job(const int fd, const uint8_t *msg, const size_t msgLen,
        const std::shared_ptr<common::socket_base> serverSocket)
        : fd_(fd), msg_(msg, msg + msgLen), serverSocket_(serverSocket)
    {}
    virtual ~job() = default;
    virtual void run() = 0;

protected:
    int fd_;
    std::vector<uint8_t> msg_;
    std::shared_ptr<common::socket_base> serverSocket_;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_JOB_H__