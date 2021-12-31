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

#ifndef NET_TETHER_IP_COORDINATOR_H
#define NET_TETHER_IP_COORDINATOR_H

#include "net_tether_ip_address.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTetherIpCoordinator {
public:
    static NetTetherIpCoordinator* GetInstance();
    void RequestIpv4Addr(NetTetherIpAddress &ipAddr);

private:
    static NetTetherIpCoordinator* instance_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_IP_COORDINATOR_H