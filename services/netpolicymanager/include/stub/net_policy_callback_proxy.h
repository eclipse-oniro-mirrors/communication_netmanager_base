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

#ifndef NET_POLICY_CALLBACK_PROXY_H
#define NET_POLICY_CALLBACK_PROXY_H

#include "iremote_proxy.h"

#include "i_net_policy_callback.h"
#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyCallbackProxy : public IRemoteProxy<INetPolicyCallback> {
public:
    explicit NetPolicyCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetPolicyCallbackProxy();

public:
    int32_t NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy) override;
    int32_t NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicies) override;
    int32_t NetStrategySwitch(int32_t slotId, bool enable) override;
    int32_t NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<NetPolicyCallbackProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_CALLBACK_PROXY_H