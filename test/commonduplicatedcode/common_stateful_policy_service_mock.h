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

#ifndef COMMON_STATEFUL_POLICY_SERVICE_MOCK_H
#define COMMON_STATEFUL_POLICY_SERVICE_MOCK_H

#include <map>
#include <memory>
#include <vector>

#include "common_mock_net_policy_service.h"
#include "net_manager_constants.h"
#include "net_policy_constants.h"
#include "netmanager_base_permission.h"

namespace OHOS {
namespace NetManagerStandard {

/**
 * @brief In-memory state emulating the observable behaviour of NetPolicyService that
 *        client-side tests rely on (uid policies, background policy flag, registered
 *        callback). Fixture scoped: create it in SetUpTestCase so sequential tests can
 *        observe each other's writes, mirroring the real service persistence.
 */
struct PolicyServiceMockState {
    std::map<uint32_t, uint32_t> uidPolicies;
    bool backgroundAllow = true;
    sptr<INetPolicyCallback> callback;
};

/**
 * @brief Builds a MockINetPolicyService whose default (ON_CALL) actions emulate the
 *        policy service behaviour, so NetPolicyClient tests verify the client path
 *        without any real IPC to the policy service (SA 1152).
 * @note  CheckPermission delegates to NetManagerPermission::CheckPermission, which is
 *        replaced by common_mock_netmanager_permission.cpp in test binaries, so the
 *        HasPermission / NoPermission cases stay meaningful (token based).
 */
inline void MakeStatefulPolicyServiceMock(sptr<MockINetPolicyService> &mock,
                                          std::shared_ptr<PolicyServiceMockState> &state)
{
    mock = sptr<MockINetPolicyService>::MakeSptr();
    state = std::make_shared<PolicyServiceMockState>();
    auto st = state;

    ON_CALL(*mock, SetPolicyByUid).WillByDefault([st](uint32_t uid, uint32_t policy) {
        st->uidPolicies[uid] = policy;
        if (st->callback != nullptr) {
            st->callback->NetUidPolicyChange(uid, policy);
            uint32_t rule = (policy & NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND)
                                ? NetUidRule::NET_RULE_REJECT_METERED
                                : NetUidRule::NET_RULE_ALLOW_METERED;
            st->callback->NetUidRuleChange(uid, rule);
        }
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, GetPolicyByUid).WillByDefault([st](uint32_t uid, uint32_t &policy) {
        auto it = st->uidPolicies.find(uid);
        policy = (it == st->uidPolicies.end()) ? NetUidPolicy::NET_POLICY_NONE : it->second;
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, GetUidsByPolicy).WillByDefault([st](uint32_t policy, std::vector<uint32_t> &uids) {
        uids.clear();
        for (const auto &item : st->uidPolicies) {
            if (item.second == policy) {
                uids.push_back(item.first);
            }
        }
        return NETMANAGER_SUCCESS;
    });
    // IsUidNetAllowed has two three-argument overloads; the universal matcher `_` is
    // viable for both, making the gmock_ overload set ambiguous. Disambiguate with
    // typed matchers on the second parameter.
    ON_CALL(*mock, IsUidNetAllowed(testing::_, testing::An<bool>(), testing::_))
        .WillByDefault([](uint32_t, bool, bool &isAllowed) {
            isAllowed = true;
            return NETMANAGER_SUCCESS;
        });
    ON_CALL(*mock, IsUidNetAllowed(testing::_, testing::An<const std::string &>(), testing::_))
        .WillByDefault([](uint32_t, const std::string &, bool &isAllowed) {
            isAllowed = true;
            return NETMANAGER_SUCCESS;
        });
    ON_CALL(*mock, RegisterNetPolicyCallback).WillByDefault([st](const sptr<INetPolicyCallback> &callback) {
        st->callback = callback;
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, UnregisterNetPolicyCallback).WillByDefault([st](const sptr<INetPolicyCallback> &callback) {
        if (st->callback == callback) {
            st->callback = nullptr;
        }
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, SetNetQuotaPolicies).WillByDefault([st](const std::vector<NetQuotaPolicy> &quotaPolicies) {
        if (st->callback != nullptr) {
            st->callback->NetQuotaPolicyChange(quotaPolicies);
        }
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, GetNetQuotaPolicies).WillByDefault([](std::vector<NetQuotaPolicy> &quotaPolicies) {
        quotaPolicies.clear();
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, UpdateRemindPolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetDeviceIdleTrustlist).WillByDefault([st](const std::vector<uint32_t> &uids, bool isAllowed) {
        if (st->callback != nullptr) {
            uint32_t rule = isAllowed ? NetUidRule::NET_RULE_ALLOW_ALL : NetUidRule::NET_RULE_REJECT_ALL;
            for (uint32_t uid : uids) {
                st->callback->NetUidRuleChange(uid, rule);
            }
        }
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, GetDeviceIdleTrustlist).WillByDefault([](std::vector<uint32_t> &uids) {
        uids.clear();
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, SetDeviceIdlePolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, ResetPolicies).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetBackgroundPolicy).WillByDefault([st](bool isAllowed) {
        // Same as NetPolicyRule::SetBackgroundPolicy: setting the same value is a
        // no-op and reports NETMANAGER_ERR_PARAMETER_ERROR; a change notifies
        // registered callbacks.
        if (st->backgroundAllow == isAllowed) {
            return NETMANAGER_ERR_PARAMETER_ERROR;
        }
        st->backgroundAllow = isAllowed;
        if (st->callback != nullptr) {
            st->callback->NetBackgroundPolicyChange(isAllowed);
        }
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, GetBackgroundPolicy).WillByDefault([st](bool &backgroundPolicy) {
        backgroundPolicy = st->backgroundAllow;
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, GetBackgroundPolicyByUid).WillByDefault([st](uint32_t, uint32_t &backgroundPolicyOfUid) {
        backgroundPolicyOfUid = st->backgroundAllow ? NetBackgroundPolicy::NET_BACKGROUND_POLICY_ENABLE
                                                    : NetBackgroundPolicy::NET_BACKGROUND_POLICY_DISABLE;
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, GetPowerSaveTrustlist).WillByDefault([](std::vector<uint32_t> &uids) {
        uids.clear();
        return NETMANAGER_SUCCESS;
    });
    ON_CALL(*mock, SetPowerSaveTrustlist).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetPowerSavePolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, CheckPermission).WillByDefault([]() {
        bool granted = NetManagerPermission::CheckPermission(Permission::MANAGE_NET_STRATEGY);
        return granted ? NETMANAGER_SUCCESS : NETMANAGER_ERR_PERMISSION_DENIED;
    });
    ON_CALL(*mock, FactoryResetPolicies).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetNetworkAccessPolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, GetNetworkAccessPolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, GetSelfNetworkAccessPolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, NotifyNetAccessPolicyDiag).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetNicTrafficAllowed).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetInternetAccessByIpForWifiShare).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetIdleDenyPolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, SetUidsDeniedListChain).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, AddNetworkAccessPolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
    ON_CALL(*mock, RemoveNetworkAccessPolicy).WillByDefault(testing::Return(NETMANAGER_SUCCESS));
}

/**
 * @brief Suite-scoped install / uninstall of the stateful policy service mock,
 *        replacing the SetUpTestCase/TearDownTestCase boilerplate that used to be
 *        duplicated verbatim per fixture.
 * @param mock / state / saved  the fixture's mock, mock-state and saved-service members
 * @param clientService         reference to the NetPolicyClient::netPolicyService_ member to
 *                              swap; the caller (test file) provides the private access,
 *                              so this header stays usable from any include context.
 */
inline void InstallStatefulPolicyServiceMock(sptr<MockINetPolicyService> &mock,
                                             std::shared_ptr<PolicyServiceMockState> &state,
                                             sptr<INetPolicyService> &saved, sptr<INetPolicyService> &clientService)
{
    MakeStatefulPolicyServiceMock(mock, state);
    saved = clientService;
    clientService = mock;
}

/**
 * @brief Restores the original service into the client and releases the fixture's
 *        mock / state / saved-service members; mirror of InstallStatefulPolicyServiceMock.
 */
inline void UninstallStatefulPolicyServiceMock(sptr<MockINetPolicyService> &mock,
                                               std::shared_ptr<PolicyServiceMockState> &state,
                                               sptr<INetPolicyService> &saved, sptr<INetPolicyService> &clientService)
{
    clientService = saved;
    state = nullptr;
    saved = nullptr;
    mock = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS

#endif // COMMON_STATEFUL_POLICY_SERVICE_MOCK_H
