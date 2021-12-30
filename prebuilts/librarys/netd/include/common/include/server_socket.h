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

#ifndef NETD_COMMON_SERVER_SOCKET_H__
#define NETD_COMMON_SERVER_SOCKET_H__

#include "socket_base.h"
namespace OHOS {
namespace nmd {
namespace common {
class server_socket : public socket_base {
public:
    server_socket();
    ~server_socket();

    int bindPort(uint16_t port);
    int bindFile(const char *filePath, const char *name);

private:
    struct sockaddr addr_ {};
};
} // namespace common
} // namespace nmd
} // namespace OHOS
#endif  // !NETD_COMMON_SERVER_SOCKET_H__