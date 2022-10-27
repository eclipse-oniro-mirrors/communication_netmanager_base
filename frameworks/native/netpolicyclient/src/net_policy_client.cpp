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

#include "net_policy_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyClient::NetPolicyClient() : netPolicyService_(nullptr), deathRecipient_(nullptr) {}

NetPolicyClient::~NetPolicyClient() = default;

int32_t NetPolicyClient::SetPolicyByUid(uint32_t uid, uint32_t policy)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetPolicyByUid(uid, policy);
}

uint32_t NetPolicyClient::GetPolicyByUid(uint32_t uid)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetUidPolicy::NET_POLICY_NONE;
    }

    return proxy->GetPolicyByUid(uid);
}

std::vector<uint32_t> NetPolicyClient::GetUidsByPolicy(uint32_t policy)
{
    std::vector<uint32_t> uids;
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return uids;
    }

    uids = proxy->GetUidsByPolicy(policy);
    return uids;
}

bool NetPolicyClient::IsUidNetAllowed(uint32_t uid, bool metered)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->IsUidNetAllowed(uid, metered);
}

bool NetPolicyClient::IsUidNetAllowed(uint32_t uid, const std::string &ifaceName)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->IsUidNetAllowed(uid, ifaceName);
}

bool NetPolicyClient::IsUidNetAccess(uint32_t uid, bool isMetered)
{
    return IsUidNetAllowed(uid, isMetered);
}

bool NetPolicyClient::IsUidNetAccess(uint32_t uid, const std::string &ifaceName)
{
    return IsUidNetAllowed(uid, ifaceName);
}

sptr<INetPolicyService> NetPolicyClient::GetProxy()
{
    std::lock_guard lock(mutex_);

    if (netPolicyService_ != nullptr) {
        NETMGR_LOG_D("get proxy is ok");
        return netPolicyService_;
    }

    NETMGR_LOG_I("execute GetSystemAbilityManager");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOG_E("NetPolicyManager::GetProxy(), get SystemAbilityManager failed");
        return nullptr;
    }

    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOG_E("get Remote service failed");
        return nullptr;
    }

    deathRecipient_ = (std::make_unique<NetPolicyDeathRecipient>(*this)).release();
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_LOG_E("add death recipient failed");
        return nullptr;
    }

    netPolicyService_ = iface_cast<INetPolicyService>(remote);
    if (netPolicyService_ == nullptr) {
        NETMGR_LOG_E("get Remote service proxy failed");
        return nullptr;
    }

    return netPolicyService_;
}

void NetPolicyClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_LOG_D("on remote died");
    if (remote == nullptr) {
        NETMGR_LOG_E("remote object is nullptr");
        return;
    }

    std::lock_guard lock(mutex_);
    if (netPolicyService_ == nullptr) {
        NETMGR_LOG_E("netPolicyService_ is nullptr");
        return;
    }

    sptr<IRemoteObject> local = netPolicyService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_LOG_E("proxy and stub is not same remote object");
        return;
    }

    local->RemoveDeathRecipient(deathRecipient_);
    netPolicyService_ = nullptr;
}

int32_t NetPolicyClient::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR);
    }

    return proxy->RegisterNetPolicyCallback(callback);
}

int32_t NetPolicyClient::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR);
    }

    return proxy->UnregisterNetPolicyCallback(callback);
}

int32_t NetPolicyClient::SetNetQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("quotaPolicies is empty");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    if (quotaPolicies.size() > QUOTA_POLICY_MAX_SIZE) {
        NETMGR_LOG_E("quotaPolicies's size is greater than the maximum, size is [%{public}zu]", quotaPolicies.size());
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetNetQuotaPolicies(quotaPolicies);
}

int32_t NetPolicyClient::GetNetQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetNetQuotaPolicies(quotaPolicies);
}

NetPolicyResultCode NetPolicyClient::SetFactoryPolicy(const std::string &iccid)
{
    return static_cast<NetPolicyResultCode>(ResetPolicies(iccid));
}

int32_t NetPolicyClient::ResetPolicies(const std::string &iccid)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->ResetPolicies(iccid);
}

int32_t NetPolicyClient::SetBackgroundPolicy(bool isBackgroundPolicyAllow)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetBackgroundPolicy(isBackgroundPolicyAllow);
}

bool NetPolicyClient::GetBackgroundPolicy()
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->GetBackgroundPolicy();
}

uint32_t NetPolicyClient::GetBackgroundPolicyByUid(uint32_t uid)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->GetBackgroundPolicyByUid(uid);
}

NetPolicyResultCode NetPolicyClient::SetSnoozePolicy(int8_t netType, const std::string &iccid)
{
    return static_cast<NetPolicyResultCode>(UpdateRemindPolicy(netType, iccid, RemindType::REMIND_TYPE_LIMIT));
}

int32_t NetPolicyClient::UpdateRemindPolicy(int32_t netType, const std::string &iccid, uint32_t remindType)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->UpdateRemindPolicy(netType, iccid, remindType);
}

NetPolicyResultCode NetPolicyClient::SetIdleTrustlist(uint32_t uid, bool isTrustlist)
{
    return static_cast<NetPolicyResultCode>(SetDeviceIdleAllowedList(uid, isTrustlist));
}

int32_t NetPolicyClient::SetDeviceIdleAllowedList(uint32_t uid, bool isAllowed)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetDeviceIdleAllowedList(uid, isAllowed);
}

NetPolicyResultCode NetPolicyClient::GetIdleTrustlist(std::vector<uint32_t> &uids)
{
    return static_cast<NetPolicyResultCode>(GetDeviceIdleAllowedList(uids));
}

int32_t NetPolicyClient::GetDeviceIdleAllowedList(std::vector<uint32_t> &uids)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetDeviceIdleAllowedList(uids);
}

int32_t NetPolicyClient::SetDeviceIdlePolicy(bool enable)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetDeviceIdlePolicy(enable);
}
} // namespace NetManagerStandard
} // namespace OHOS
