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

#ifndef __INCLUDE_FWMARK_SERVER_H__
#define __INCLUDE_FWMARK_SERVER_H__

#include <memory>
#include "job.h"
#include "server_template.h"
namespace OHOS {
namespace nmd {
const char *const FWMARK_SERVER_SOCK_NAME = "fwmarkd.sock";
const char *const FWMARK_SERVER_NAME = "FWMarkServer";

class fwmark_job : public job {
public:
    fwmark_job(const int fd, const uint8_t *msg, const size_t msgLen,
        const std::shared_ptr<common::server_socket> &serverSocket)
        : job(fd, msg, msgLen, serverSocket)
    {}
    ~fwmark_job() = default;

    virtual void run() override;

private:
    void responseOk();
    void responseFailed();
};

class fwmark_server : public common::server_template {
public:
    fwmark_server() : common::server_template(FWMARK_SERVER_SOCK_NAME, FWMARK_SERVER_NAME) {}

    virtual ~fwmark_server() = default;

private:
    virtual void initJob(const int socketFd, const uint8_t *msg, const size_t msgLen) override;
};
} // namespace nmd
} // namespace OHOS
#endif // !__INCLUDE_FWMARK_SERVER_H__