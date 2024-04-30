/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NET_ACCESS_POLICY_H
#define NET_ACCESS_POLICY_H

#include <stdint.h>
#include <map>
#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {

class NetworkAccessPolicy {
public:
    NetworkAccessPolicy() = default;
    bool wifiAllow = true;     // true means allow. false means deny.
    bool cellularAllow = true;  // true means allow. false means deny.
};

class AccessPolicyParameter {
public:
    AccessPolicyParameter() = default;
    bool flag = false;
    uint32_t uid = 0;
    uint32_t userId = 0;
};

class AccessPolicySave {
public:
    AccessPolicySave() = default;

    NetworkAccessPolicy policy;
    std::map<int32_t, NetworkAccessPolicy> uid_policies;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif /* NET_ACCESS_POLICY_H */
