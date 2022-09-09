/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "net_policy_constants.h"
#include "net_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyClient {
    DECLARE_DELAYED_SINGLETON(NetPolicyClient)

public:
    /**
     * Set the network policy for the specified UID.
     *
     * @param uid The specified UID of app.
     * @param policy The network policy for application.
     *      For details, see {@link NetUidPolicy}.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetPolicyByUid(uint32_t uid, uint32_t policy);

    /**
     * Get the network policy of the specified UID.
     *
     * @param uid The specified UID of app.
     * @return Returns the network policy of the specified UID application.
     *      For details, see {@link NetUidPolicy}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    uint32_t GetPolicyByUid(uint32_t uid);

    /**
     * Get the application UIDs of the specified policy.
     *
     * @param policy the network policy of the current UID of application.
     *      For details, see {@link NetUidPolicy}.
     * @return Returns the UIDs of the specified policy.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    std::vector<uint32_t> GetUidsByPolicy(uint32_t policy);

    /**
     * Get the status whether the specified uid app can access the metered network or non-metered network.
     *
     * @param uid The specified UID of application.
     * @param metered Indicates meterd network or non-metered network.
     * @return Returns it's allowed or not to access the network.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    bool IsUidNetAllowed(uint32_t uid, bool isMetered);

    /**
     * Get the status whether the specified uid app can access the specified iface network.
     *
     * @param uid The specified UID of application.
     * @param ifaceName Iface name.
     * @return Returns it's allowed or not to access the network.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    bool IsUidNetAllowed(uint32_t uid, const std::string &ifaceName);

    /**
     * @deprecated
     */
    bool IsUidNetAccess(uint32_t uid, bool isMetered);

    /**
     * @deprecated
     */
    bool IsUidNetAccess(uint32_t uid, const std::string &ifaceName);

    /**
     * Register network policy change callback.
     *
     * @param callback The callback of INetPolicyCallback interface.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback);

    /**
     * Unregister network policy change callback.
     *
     * @param callback The callback of INetPolicyCallback interface.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback);

    /**
     * SetCellularPolicies set policys by NetPolicyCellularPolicy
     *
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    NetPolicyResultCode SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies);

    /**
     * GetCellularPolicies get policys for NetPolicyCellularPolicy
     *
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    NetPolicyResultCode GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies);

    /**
     * SetNetQuotaPolicies set policys by NetPolicyQuotaPolicy
     *
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    NetPolicyResultCode SetNetQuotaPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies);

    /**
     * Set network policies.
     *
     * @param quotaPolicies The list of network quota policy, {@link NetQuotaPolicy}.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetNetQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies);

    /**
     * GetNetQuotaPolicies get policys for NetQuotaPolicy.
     *
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    NetPolicyResultCode GetNetQuotaPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies);

    /**
     * Get network policies.
     *
     * @param quotaPolicies The list of network quota policy, {@link NetQuotaPolicy}.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetNetQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies);

    /**
     * SetFactoryPolicy reset policys for iccid.
     *
     * @param iccid ID, get from telephone module
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    NetPolicyResultCode SetFactoryPolicy(const std::string &iccid);

    /**
     * Reset network policies\rules\quota policies\firewall rules.
     *
     * @param iccid Specify the matched iccid of quota policy.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t ResetPolicies(const std::string &iccid);

    /**
     * Control if apps can use data on background.
     *
     * @param isAllowed Allow apps to use data on background.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetBackgroundPolicy(bool isAllowed);

    /**
     * Get the status if apps can use data on background.
     *
     * @return It's allowed or not to use data on background.
     *      For details, see {@link BackgroundPolicy#BACKGROUND_POLICY_DISABLE}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    bool GetBackgroundPolicy();

    /**
     * Get the background network restriction policy for the specified uid.
     *
     * @param uid The specified UID of application.
     * @return {@link NetBackgroundPolicy}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    uint32_t GetBackgroundPolicyByUid(uint32_t uid);

    /**
     * GetCurrentBackgroundPolicy get background policy by current
     * @return Returns NetBackgroundPolicy
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    uint32_t GetCurrentBackgroundPolicy();

    /**
     * SetSnoozePolicy for Hibernate current policy
     *
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    NetPolicyResultCode SetSnoozePolicy(int8_t netType, const std::string &iccid);

    /**
     * Update the limit or warning remind time of quota policy.
     *
     * @param netType {@link NetBearType}.
     * @param iccid Specify the matched iccid of quota policy when netType is cellular.
     * @param remindType {@link RemindType}.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UpdateRemindPolicy(int32_t netType, const std::string &iccid, uint32_t remindType);

    /**
     * SetIdleTrustlist for add trust list for Idle status
     *
     * @param uid uid
     * @param isTrustlist true/false
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @deprecated
     */
    NetPolicyResultCode SetIdleTrustlist(uint32_t uid, bool isTrustlist);

    /**
     * Set the UID into device idle allow list.
     *
     * @param uid The specified UID of application.
     * @param isAllowed The UID is into allow list or not.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetDeviceIdleAllowedList(uint32_t uid, bool isAllowed);

    /**
     * GetIdleTrustlist for get trust list for Idle status
     *
     * @param uid uid
     * @param uids
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @deprecated
     */
    NetPolicyResultCode GetIdleTrustlist(std::vector<uint32_t> &uids);

    /**
     * Get the allow list of UID in device idle mode.
     *
     * @param uids The list of UIDs
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetDeviceIdleAllowedList(std::vector<uint32_t> &uids);

    /**
     * Process network policy in device idle mode.
     *
     * @param enable Device idle mode is open or not.
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetDeviceIdlePolicy(bool enable);

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
