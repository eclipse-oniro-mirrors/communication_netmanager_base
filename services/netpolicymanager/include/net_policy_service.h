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

    NetPolicyResultCode SetUidPolicy(uint32_t uid, NetUidPolicy policy) override;
    NetUidPolicy GetUidPolicy(uint32_t uid) override;
    std::vector<uint32_t> GetUids(NetUidPolicy policy) override;
    bool IsUidNetAccess(uint32_t uid, bool metered) override;
    bool IsUidNetAccess(uint32_t uid, const std::string &ifaceName) override;
    int32_t RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) override;
    int32_t UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) override;
    NetPolicyResultCode SetNetPolicys(const std::vector<NetPolicyQuotaPolicy> &quotaPolicys) override;
    NetPolicyResultCode GetNetPolicys(std::vector<NetPolicyQuotaPolicy> &quotaPolicys) override;
    NetPolicyResultCode SetCellularPolicys(const std::vector<NetPolicyCellularPolicy> &cellularPolicys) override;
    NetPolicyResultCode GetCellularPolicys(std::vector<NetPolicyCellularPolicy> &cellularPolicys) override;
    NetPolicyResultCode ResetFactory(const std::string &subscriberId) override;
    NetPolicyResultCode SetBackgroundPolicy(bool backgroundPolicy) override;
    bool GetBackgroundPolicy() override;
    bool GetBackgroundPolicyByUid(uint32_t uid) override;
    bool GetCurrentBackgroundPolicy() override;
    NetPolicyResultCode SnoozePolicy(const NetPolicyQuotaPolicy &quotaPolicy) override;
    NetPolicyResultCode SetIdleWhitelist(uint32_t uid, bool isWhiteList) override;
    NetPolicyResultCode GetIdleWhitelist(std::vector<uint32_t> &uids) override;

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
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_SERVICE_H
