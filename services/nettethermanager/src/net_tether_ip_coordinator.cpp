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

#include "net_tether_ip_coordinator.h"
#include "net_tether_define.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherIpCoordinator* NetTetherIpCoordinator::instance_ = nullptr;

NetTetherIpCoordinator* NetTetherIpCoordinator::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = new NetTetherIpCoordinator();
    }
    return instance_;
}

void NetTetherIpCoordinator::RequestIpv4Addr(NetTetherIpAddress &ipAddr)
{
    ipAddr = NetTetherIpAddress(DEFAULT_IFACE_ADDR, TETHER_PREFIX_LEN, true);
    return;
}
} // namespace NetManagerStandard
} // namespace OHOS