/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_MOCK_NET_POLICY_SERVICE_H
#define COMMON_MOCK_NET_POLICY_SERVICE_H

#include "i_net_policy_service.h"
#include "net_all_capabilities.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace NetManagerStandard {

class MockINetPolicyService : public INetPolicyService {
public:
    MockINetPolicyService() = default;
    ~MockINetPolicyService() override = default;

    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
    MOCK_METHOD(int32_t, SetPolicyByUid, (uint32_t uid, uint32_t policy), (override));
    MOCK_METHOD(int32_t, GetPolicyByUid, (uint32_t uid, uint32_t &policy), (override));
    MOCK_METHOD(int32_t, GetUidsByPolicy, (uint32_t policy, std::vector<uint32_t> &uids), (override));
    MOCK_METHOD(int32_t, IsUidNetAllowed, (uint32_t uid, bool metered, bool &isAllowed), (override));
    MOCK_METHOD(int32_t, IsUidNetAllowed, (uint32_t uid, const std::string &ifaceName, bool &isAllowed), (override));
    MOCK_METHOD(int32_t, RegisterNetPolicyCallback, (const sptr<INetPolicyCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnregisterNetPolicyCallback, (const sptr<INetPolicyCallback> &callback), (override));
    MOCK_METHOD(int32_t, SetNetQuotaPolicies, (const std::vector<NetQuotaPolicy> &quotaPolicies), (override));
    MOCK_METHOD(int32_t, GetNetQuotaPolicies, (std::vector<NetQuotaPolicy> & quotaPolicies), (override));
    MOCK_METHOD(int32_t, UpdateRemindPolicy, (int32_t netType, const std::string &simId, uint32_t remindType),
                (override));
    MOCK_METHOD(int32_t, SetDeviceIdleTrustlist, (const std::vector<uint32_t> &uids, bool isAllowed), (override));
    MOCK_METHOD(int32_t, GetDeviceIdleTrustlist, (std::vector<uint32_t> & uids), (override));
    MOCK_METHOD(int32_t, SetDeviceIdlePolicy, (bool enable), (override));
    MOCK_METHOD(int32_t, ResetPolicies, (const std::string &simId), (override));
    MOCK_METHOD(int32_t, SetBackgroundPolicy, (bool isAllowed), (override));
    MOCK_METHOD(int32_t, GetBackgroundPolicy, (bool &backgroundPolicy), (override));
    MOCK_METHOD(int32_t, GetBackgroundPolicyByUid, (uint32_t uid, uint32_t &backgroundPolicyOfUid), (override));
    MOCK_METHOD(int32_t, GetPowerSaveTrustlist, (std::vector<uint32_t> & uids), (override));
    MOCK_METHOD(int32_t, SetPowerSaveTrustlist, (const std::vector<uint32_t> &uids, bool isAllowed), (override));
    MOCK_METHOD(int32_t, SetPowerSavePolicy, (bool enable), (override));
    MOCK_METHOD(int32_t, CheckPermission, (), (override));
    MOCK_METHOD(int32_t, FactoryResetPolicies, (), (override));
    MOCK_METHOD(int32_t, SetNetworkAccessPolicy, (uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag),
                (override));
    MOCK_METHOD(int32_t, GetNetworkAccessPolicy, (AccessPolicyParameter parameter, AccessPolicySave &policy),
                (override));
    MOCK_METHOD(int32_t, GetSelfNetworkAccessPolicy, (NetAccessPolicy & policy), (override));
    MOCK_METHOD(int32_t, NotifyNetAccessPolicyDiag, (uint32_t uid), (override));
    MOCK_METHOD(int32_t, SetNicTrafficAllowed, (const std::vector<std::string> &ifaceNames, bool status), (override));
    MOCK_METHOD(int32_t, SetInternetAccessByIpForWifiShare,
                (const std::string &ipAddr, uint8_t family, bool accessInternet, const std::string &clientNetIfName),
                (override));
    MOCK_METHOD(int32_t, SetIdleDenyPolicy, (bool enable), (override));
    MOCK_METHOD(int32_t, SetUidsDeniedListChain, (const std::vector<uint32_t> &uids, bool isAdd), (override));
    MOCK_METHOD(int32_t, AddNetworkAccessPolicy, (const std::vector<std::string> &bundleNames), (override));
    MOCK_METHOD(int32_t, RemoveNetworkAccessPolicy, (const std::vector<std::string> &bundleNames), (override));
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // COMMON_MOCK_NET_POLICY_SERVICE_H
