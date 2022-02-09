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

#ifndef NET_POLICY_CLIENT_H
#define NET_POLICY_CLIENT_H

#include "singleton.h"

#include "i_net_policy_service.h"
#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "net_policy_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyClient {
    DECLARE_DELAYED_SINGLETON(NetPolicyClient)

public:
    /**
     * @brief The interface is set uid policy
     *
     * @param uid uid
     * @param policy policy
     *
     * @return Returns 0 success, otherwise fail
     */
    NetPolicyResultCode SetPolicyByUid(uint32_t uid, NetUidPolicy policy);
    /**
     * @brief The interface is get uid policy
     *
     * @param uid uid
     * @return Returns NetUidPolicy, otherwise fail
     */
    NetUidPolicy GetPolicyByUid(uint32_t uid);
    /**
     * @brief The interface is get uids by policy
     *
     * @param policy policy
     * @return uids
     */
    std::vector<uint32_t> GetUidsByPolicy(NetUidPolicy policy);
    /**
     * @brief The interface determine whether you have access to the network
     *
     * @param uid uid
     * @param metered true/false
     * @return bool
     */
    bool IsUidNetAccess(uint32_t uid, bool metered);
    /**
     * @brief The interface determine whether you have access to the network
     *
     * @param uid uid
     * @param ifaceName , you can get ifacename by GetIfaceNameByType
     * @return bool
     */
    bool IsUidNetAccess(uint32_t uid, const std::string &ifaceName);
    /**
     * @brief Register net policy callback
     *
     * @param callback The callback of INetPolicyCallback interface
     *
     * @return Returns 0, successfully register net policy callback, otherwise it will failed
     */
    int32_t RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback);
    /**
     * @brief Unregister net policy callback
     *
     * @return Returns 0, successfully unregister net policy callback, otherwise it will fail
     */
    int32_t UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback);
    /**
     * @brief SetNetQuotaPolicies set policys by NetPolicyQuotaPolicy
     *
     * @return Returns 0, successfully
     */
    NetPolicyResultCode SetNetQuotaPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies);
    /**
     * @brief GetNetQuotaPolicies get policys for NetPolicyQuotaPolicy
     *
     * @return Returns 0, successfully
     */
    NetPolicyResultCode GetNetQuotaPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies);
    /**
     * @brief SetCellularPolicies set policys by NetPolicyCellularPolicy
     *
     * @return Returns 0, successfully
     */
    NetPolicyResultCode SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies);
    /**
     * @brief GetCellularPolicies get policys for NetPolicyCellularPolicy
     *
     * @return Returns 0, successfully
     */
    NetPolicyResultCode GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies);
    /**
     * @brief SetFactoryPolicy reset policys for simId
     *
     * @param simId ID, get from telephone module
     * @return Returns 0, successfully
     */
    NetPolicyResultCode SetFactoryPolicy(const std::string &simId);
    /**
     * @brief SetBackgroundPolicy reset backgroundpolicy for all app
     *
     * @param backgroundPolicy refuse app visit network
     * @return Returns 0, successfully
     */
    NetPolicyResultCode SetBackgroundPolicy(bool isBackgroundPolicyAllow);
    /**
     * @brief GetBackgroundPolicy get background policy
     *
     * @return bool
     */
    bool GetBackgroundPolicy();
    /**
     * @brief GetBackgroundPolicyByUid get background policy by uid
     *
     * @param uid uid
     * @return bool
     */
    bool GetBackgroundPolicyByUid(uint32_t uid);
    /**
     * @brief GetCurrentBackgroundPolicy get background policy by current
     *
     * @return Returns NetBackgroundPolicy
     */
    NetBackgroundPolicy GetCurrentBackgroundPolicy();
    /**
     * @brief SetSnoozePolicy for Hibernate current policy
     *
     * @return Returns 0, successfully
     */
    NetPolicyResultCode SetSnoozePolicy(int8_t netType, const std::string &simId);
    /**
     * @brief SetIdleTrustlist for add trust list for Idle status
     *
     * @param uid uid
     * @param isTrustlist true/false
     *
     * @return Returns 0, successfully
     */
    NetPolicyResultCode SetIdleTrustlist(uint32_t uid, bool isTrustlist);
    /**
     * @brief GetIdleTrustlist for get trust list for Idle status
     *
     * @param uid uid
     * @param uids
     *
     * @return Returns 0, successfully
     */
    NetPolicyResultCode GetIdleTrustlist(std::vector<uint32_t> &uids);

private:
    class NetPolicyDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetPolicyDeathRecipient(NetPolicyClient &client) : client_(client) {}
        ~NetPolicyDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetPolicyClient &client_;
    };

private:
    sptr<INetPolicyService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<INetPolicyService> netPolicyService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_CLIENT_H
