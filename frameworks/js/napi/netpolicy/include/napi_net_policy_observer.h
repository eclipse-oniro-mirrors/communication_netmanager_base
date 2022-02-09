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

#ifndef NAPI_NET_POLICY_OBSERVER_H
#define NAPI_NET_POLICY_OBSERVER_H

#include "net_policy_callback_stub.h"
#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "napi_net_policy.h"

namespace OHOS {
namespace NetManagerStandard {
class NapiNetPolicyObserver : public NetPolicyCallbackStub {
public:
    int32_t NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy) override;
    int32_t NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicys) override;
    int32_t NetStrategySwitch(const std::string &simId, bool enable) override;
    int32_t NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow) override;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NET_POLICY_OBSERVER_H