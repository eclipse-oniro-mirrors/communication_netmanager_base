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
#include "net_policy_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyClient::NetPolicyClient() : netPolicyService_(nullptr), deathRecipient_(nullptr) {}

NetPolicyClient::~NetPolicyClient() {}

NetPolicyResultCode NetPolicyClient::SetPolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetPolicyByUid(uid, policy);
}

NetUidPolicy NetPolicyClient::GetPolicyByUid(uint32_t uid)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetUidPolicy::NET_POLICY_NONE;
    }

    return proxy->GetPolicyByUid(uid);
}

std::vector<uint32_t> NetPolicyClient::GetUidsByPolicy(NetUidPolicy policy)
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

bool NetPolicyClient::IsUidNetAccess(uint32_t uid, bool metered)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->IsUidNetAccess(uid, metered);
}

bool NetPolicyClient::IsUidNetAccess(uint32_t uid, const std::string &ifaceName)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->IsUidNetAccess(uid, ifaceName);
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

NetPolicyResultCode NetPolicyClient::SetNetQuotaPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("quotaPolicies is empty");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetNetQuotaPolicies(quotaPolicies);
}

NetPolicyResultCode NetPolicyClient::GetNetQuotaPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetNetQuotaPolicies(quotaPolicies);
}

NetPolicyResultCode NetPolicyClient::SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    if (cellularPolicies.empty()) {
        NETMGR_LOG_E("cellularPolicies is empty");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetCellularPolicies(cellularPolicies);
}

NetPolicyResultCode NetPolicyClient::GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetCellularPolicies(cellularPolicies);
}

NetPolicyResultCode NetPolicyClient::SetFactoryPolicy(const std::string &slotId)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetFactoryPolicy(slotId);
}

NetPolicyResultCode NetPolicyClient::SetBackgroundPolicy(bool isBackgroundPolicyAllow)
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

bool NetPolicyClient::GetBackgroundPolicyByUid(uint32_t uid)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->GetBackgroundPolicyByUid(uid);
}

NetBackgroundPolicy NetPolicyClient::GetCurrentBackgroundPolicy()
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetBackgroundPolicy::NET_BACKGROUND_POLICY_NONE;
    }

    return proxy->GetCurrentBackgroundPolicy();
}

NetPolicyResultCode NetPolicyClient::SetSnoozePolicy(int8_t netType, int32_t slotId)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetSnoozePolicy(netType, slotId);
}

NetPolicyResultCode NetPolicyClient::SetIdleTrustlist(uint32_t uid, bool isTrustlist)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetIdleTrustlist(uid, isTrustlist);
}

NetPolicyResultCode NetPolicyClient::GetIdleTrustlist(std::vector<uint32_t> &uids)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetIdleTrustlist(uids);
}
} // namespace NetManagerStandard
} // namespace OHOS
