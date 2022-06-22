/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_SOCKET_FACTORY_H
#define NET_CONN_SOCKET_FACTORY_H

#include <cstdint>

namespace OHOS {
namespace NetManagerStandard {
class SocketFactory {
public:
    virtual ~SocketFactory() {}

    virtual int32_t CreateSocket(int32_t domain, int32_t type, int32_t protocol) = 0;

    virtual void DestroySocket(int32_t sockFd) = 0;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif // NET_CONN_URL_H
