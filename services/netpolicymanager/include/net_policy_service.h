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

#ifndef NET_POLICY_SERVICE_H
#define NET_POLICY_SERVICE_H

#include <mutex>

#include "singleton.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "net_policy_callback.h"
#include "net_policy_traffic.h"
#include "net_policy_firewall.h"

#include "net_policy_service_stub.h"
#include "net_policy_service_common.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyService : public SystemAbility,
    public NetPolicyServiceStub,
    public std::enable_shared_from_this<NetPolicyService> {
    DECLARE_DELAYED_SINGLETON(NetPolicyService)
    DECLARE_SYSTEM_ABILITY(NetPolicyService)

public:
    void OnStart() override;
    void OnStop() override;

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
    NetPolicyResultCode SetFactoryPolicy(const std::string &simId) override;
    NetPolicyResultCode SetBackgroundPolicy(bool backgroundPolicy) override;
    bool GetBackgroundPolicy() override;
    bool GetBackgroundPolicyByUid(uint32_t uid) override;
    NetBackgroundPolicy GetCurrentBackgroundPolicy() override;
    NetPolicyResultCode SetSnoozePolicy(int8_t netType, const std::string &simId) override;
    NetPolicyResultCode SetIdleTrustlist(uint32_t uid, bool isTrustlist) override;
    NetPolicyResultCode GetIdleTrustlist(std::vector<uint32_t> &uids) override;
    void CheckNetStatsOverLimit(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies);
    void CheckNetStatsOverLimit(const std::vector<NetPolicyCellularPolicy> &cellularPolicies);
    int32_t GetPeriodEndTime();
    int64_t GetCurrentTime();

private:
    bool Init();

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    sptr<NetPolicyTraffic> netPolicyTraffic_;
    sptr<NetPolicyFirewall> netPolicyFirewall_;
    sptr<NetPolicyFile> netPolicyFile_;
    bool registerToService_;
    ServiceRunningState state_;
    sptr<NetPolicyCallback> netPolicyCallback_;
    sptr<NetPolicyServiceCommon> serviceComm_ = nullptr;
    std::mutex mutex_;
    std::vector<uint16_t> monthDay_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_SERVICE_H
