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

#ifndef NET_POLICY_SERVICE_PROXY_H
#define NET_POLICY_SERVICE_PROXY_H

#include "iremote_proxy.h"

#include "i_net_policy_service.h"
#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "net_policy_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyServiceProxy : public IRemoteProxy<INetPolicyService> {
public:
    explicit NetPolicyServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetPolicyServiceProxy();
    NetPolicyResultCode SetPolicyByUid(uint32_t uid, NetUidPolicy policy) override;
    NetUidPolicy GetPolicyByUid(uint32_t uid) override;
    std::vector<uint32_t> GetUidsByPolicy(NetUidPolicy policy) override;
    bool IsUidNetAccess(uint32_t uid, bool metered) override;
    bool IsUidNetAccess(uint32_t uid, const std::string &ifaceName) override;
    int32_t RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) override;
    int32_t UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) override;
    NetPolicyResultCode SetNetQuotaPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies) override;
    NetPolicyResultCode GetNetQuotaPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies) override;
    NetPolicyResultCode SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies) override;
    NetPolicyResultCode GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies) override;
    NetPolicyResultCode SetFactoryPolicy(const std::string &slotId) override;
    NetPolicyResultCode SetBackgroundPolicy(bool backgroundPolicy) override;
    bool GetBackgroundPolicy() override;
    bool GetBackgroundPolicyByUid(uint32_t uid) override;
    NetBackgroundPolicy GetCurrentBackgroundPolicy() override;
    NetPolicyResultCode SetSnoozePolicy(int8_t netType, int32_t slotId) override;
    NetPolicyResultCode SetIdleTrustlist(uint32_t uid, bool isTrustlist) override;
    NetPolicyResultCode GetIdleTrustlist(std::vector<uint32_t> &uids) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<NetPolicyServiceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_SERVICE_PROXY_H
