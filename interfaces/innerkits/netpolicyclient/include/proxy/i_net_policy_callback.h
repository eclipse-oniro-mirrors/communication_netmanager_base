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

#ifndef I_NET_POLICY_CALLBACK_H
#define I_NET_POLICY_CALLBACK_H

#include <string>

#include "iremote_broker.h"
#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class INetPolicyCallback : public IRemoteBroker {
public:
    virtual ~INetPolicyCallback() = default;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetPolicyCallback");
    enum {
        NET_POLICY_UIDPOLICY_CHANGED = 0,
        NET_POLICY_CELLULARPOLICY_CHANGED = 1,
        NET_POLICY_STRATEGYSWITCH_CHANGED = 2,
        NET_POLICY_BACKGROUNDPOLICY_CHANGED = 3,
    };

public:
    virtual int32_t NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy) = 0;
    virtual int32_t NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicies) = 0;
    virtual int32_t NetStrategySwitch(const std::string &simId, bool enable) = 0;
    virtual int32_t NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_POLICY_CALLBACK_H