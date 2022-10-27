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

#include "net_policy_service.h"

#include <algorithm>

#include "system_ability_definition.h"

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_constants.h"
#include "net_policy_core.h"
#include "net_policy_event_handler.h"
#include "net_policy_file.h"
#include "net_policy_firewall.h"
#include "net_policy_inner_define.h"
#include "net_policy_rule.h"
#include "net_policy_traffic.h"
#include "net_quota_policy.h"
#include "net_settings.h"
#include "netmanager_base_permission.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<bool>
    g_RegisterToService(SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetPolicyService>::GetInstance().get()));

NetPolicyService::NetPolicyService()
    : SystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, true), state_(STATE_STOPPED)
{
}

NetPolicyService::~NetPolicyService() = default;

void NetPolicyService::OnStart()
{
    NETMGR_LOG_I("NetPolicyService OnStart");
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_W("NetPolicyService already start.");
        return;
    }

    if (!g_RegisterToService) {
        g_RegisterToService =
            SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetPolicyService>::GetInstance().get());
        if (!g_RegisterToService) {
            NETMGR_LOG_E("Register to local sa manager failed again, give up.");
            return;
        }
    }
    if (!Publish(DelayedSingleton<NetPolicyService>::GetInstance().get())) {
        NETMGR_LOG_E("Register to sa manager failed");
        return;
    }

    state_ = STATE_RUNNING;
    Init();
}

void NetPolicyService::OnStop()
{
    runner_->Stop();
    handler_.reset();
    runner_.reset();
    netPolicyCore_.reset();
    netPolicyCallback_.reset();
    netPolicyTraffic_.reset();
    netPolicyFirewall_.reset();
    netPolicyRule_.reset();
    state_ = STATE_STOPPED;
    g_RegisterToService = false;
}

int32_t NetPolicyService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_LOG_D("Start policy Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR)
                   : static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

void NetPolicyService::Init()
{
    NETMGR_LOG_I("NetPolicyService Init");
    handler_->PostTask(
        [this]() {
            serviceComm_ = (std::make_unique<NetPolicyServiceCommon>()).release();
            NetManagerCenter::GetInstance().RegisterPolicyService(serviceComm_);
            netPolicyCore_ = DelayedSingleton<NetPolicyCore>::GetInstance();
            netPolicyCallback_ = DelayedSingleton<NetPolicyCallback>::GetInstance();
            netPolicyTraffic_ = netPolicyCore_->CreateCore<NetPolicyTraffic>();
            netPolicyFirewall_ = netPolicyCore_->CreateCore<NetPolicyFirewall>();
            netPolicyRule_ = netPolicyCore_->CreateCore<NetPolicyRule>();
        },
        AppExecFwk::EventQueue::Priority::HIGH);
}

int32_t NetPolicyService::SetPolicyByUid(uint32_t uid, uint32_t policy)
{
    if (!CheckPermission(Permission::SET_NETWORK_POLICY, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }

    NETMGR_LOG_D("SetPolicyByUid uid[%{public}d] policy[%{public}d]", uid, policy);
    netPolicyRule_->TransPolicyToRule(uid, policy);
    return NetPolicyResultCode::ERR_NONE;
}

uint32_t NetPolicyService::GetPolicyByUid(uint32_t uid)
{
    if (!CheckPermission(Permission::GET_NETWORK_POLICY, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }

    NETMGR_LOG_D("GetPolicyByUid uid[%{public}d]", uid);
    return netPolicyRule_->GetPolicyByUid(uid);
}

std::vector<uint32_t> NetPolicyService::GetUidsByPolicy(uint32_t policy)
{
    if (!CheckPermission(Permission::GET_NETWORK_POLICY, __func__)) {
        return std::vector<uint32_t>(0);
    }

    NETMGR_LOG_D("GetUidsByPolicy policy[%{public}d]", policy);
    return netPolicyRule_->GetUidsByPolicy(policy);
}

bool NetPolicyService::IsUidNetAllowed(uint32_t uid, bool metered)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return false;
    }

    NETMGR_LOG_D("IsUidNetAllowed uid[%{public}d metered[%{public}d]", uid, metered);
    if (NetSettings::GetInstance().IsSystem(uid)) {
        return true;
    }
    return netPolicyRule_->IsUidNetAllowed(uid, metered);
}

bool NetPolicyService::IsUidNetAllowed(uint32_t uid, const std::string &ifaceName)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return false;
    }

    NETMGR_LOG_D("IsUidNetAllowed uid[%{public}d ifaceName[%{public}s]", uid, ifaceName.c_str());
    const auto &vec = netPolicyTraffic_->GetMeteredIfaces();
    if (std::find(vec.begin(), vec.end(), ifaceName) != vec.end()) {
        return IsUidNetAllowed(uid, true);
    }
    return IsUidNetAllowed(uid, false);
}

int32_t NetPolicyService::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    NETMGR_LOG_D("RegisterNetPolicyCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("RegisterNetPolicyCallback parameter callback is null");
        return NetPolicyResultCode::ERR_INVALID_PARAM;
    }

    netPolicyCallback_->RegisterNetPolicyCallback(callback);

    return NetPolicyResultCode::ERR_NONE;
}

int32_t NetPolicyService::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    NETMGR_LOG_D("UnregisterNetPolicyCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("UnregisterNetPolicyCallback parameter callback is null");
        return NetPolicyResultCode::ERR_INVALID_PARAM;
    }

    netPolicyCallback_->UnregisterNetPolicyCallback(callback);

    return NetPolicyResultCode::ERR_NONE;
}

int32_t NetPolicyService::SetNetQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    if (!CheckPermission(Permission::SET_NETWORK_POLICY, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }

    NETMGR_LOG_D("SetNetQuotaPolicies quotaPolicySize[%{public}zd]", quotaPolicies.size());
    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("SetNetQuotaPolicies size is empty");
        return ERR_INVALID_QUOTA_POLICY;
    }
    return netPolicyTraffic_->UpdateQuotaPolicies(quotaPolicies);
}

int32_t NetPolicyService::GetNetQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    if (!CheckPermission(Permission::GET_NETWORK_POLICY, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    NETMGR_LOG_D("GetNetQuotaPolicies begin");
    return netPolicyTraffic_->GetNetQuotaPolicies(quotaPolicies);
}

int32_t NetPolicyService::ResetPolicies(const std::string &iccid)
{
    if (!CheckPermission(Permission::SET_NETWORK_POLICY, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    NETMGR_LOG_D("ResetPolicies begin");
    netPolicyRule_->ResetPolicies();
    netPolicyFirewall_->ResetPolicies();
    netPolicyTraffic_->ResetPolicies(iccid);
    return ERR_NONE;
}

int32_t NetPolicyService::SetBackgroundPolicy(bool allow)
{
    if (!CheckPermission(Permission::SET_NETWORK_POLICY, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    NETMGR_LOG_D("SetBackgroundPolicy allow[%{public}d]", allow);
    netPolicyRule_->SetBackgroundPolicy(allow);
    return ERR_NONE;
}

bool NetPolicyService::GetBackgroundPolicy()
{
    if (!CheckPermission(Permission::GET_NETWORK_POLICY, __func__)) {
        return false;
    }
    NETMGR_LOG_D("GetBackgroundPolicy begin");
    return netPolicyRule_->GetBackgroundPolicy();
}

uint32_t NetPolicyService::GetBackgroundPolicyByUid(uint32_t uid)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    NETMGR_LOG_D("GetBackgroundPolicyByUid uid[%{public}d]", uid);
    return netPolicyRule_->GetBackgroundPolicyByUid(uid);
}

int32_t NetPolicyService::UpdateRemindPolicy(int32_t netType, const std::string &iccid, uint32_t remindType)
{
    if (!CheckPermission(Permission::SET_NETWORK_POLICY, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    NETMGR_LOG_D("UpdateRemindPolicy start");
    return netPolicyTraffic_->UpdateRemindPolicy(netType, iccid, remindType);
}

int32_t NetPolicyService::SetDeviceIdleAllowedList(uint32_t uid, bool isAllowed)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }

    NETMGR_LOG_D("SetDeviceIdleAllowedList info: uid[%{public}d] isAllowed[%{public}d]", uid, isAllowed);
    netPolicyFirewall_->SetDeviceIdleAllowedList(uid, isAllowed);
    return NetPolicyResultCode::ERR_NONE;
}

int32_t NetPolicyService::GetDeviceIdleAllowedList(std::vector<uint32_t> &uids)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }
    uids = netPolicyFirewall_->GetDeviceIdleAllowedList();
    NETMGR_LOG_D("GetDeviceIdleAllowedList uids size[%{public}zu]", uids.size());
    return NetPolicyResultCode::ERR_NONE;
}

int32_t NetPolicyService::SetDeviceIdlePolicy(bool enable)
{
    if (!CheckPermission(Permission::CONNECTIVITY_INTERNAL, __func__)) {
        return NetPolicyResultCode::ERR_PERMISSION_DENIED;
    }

    NETMGR_LOG_D("SetDeviceIdlePolicy enable[%{public}d]", enable);
    netPolicyFirewall_->UpdateDeviceIdlePolicy(enable);
    return NetPolicyResultCode::ERR_NONE;
}

int32_t NetPolicyService::GetDumpMessage(std::string &message)
{
    netPolicyRule_->GetDumpMessage(message);
    netPolicyTraffic_->GetDumpMessage(message);
    return NetPolicyResultCode::ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
