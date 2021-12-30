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

NetPolicyResultCode NetPolicyClient::SetUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetUidPolicy(uid, policy);
}

NetUidPolicy NetPolicyClient::GetUidPolicy(uint32_t uid)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetUidPolicy::NET_POLICY_NONE;
    }

    return proxy->GetUidPolicy(uid);
}

std::vector<uint32_t> NetPolicyClient::GetUids(NetUidPolicy policy)
{
    std::vector<uint32_t> uids;
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return uids;
    }

    uids = proxy->GetUids(policy);
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

NetPolicyResultCode NetPolicyClient::SetNetPolicys(const std::vector<NetPolicyQuotaPolicy> &quotaPolicys)
{
    if (quotaPolicys.empty()) {
        NETMGR_LOG_E("quotaPolicys is empty");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetNetPolicys(quotaPolicys);
}

NetPolicyResultCode NetPolicyClient::GetNetPolicys(std::vector<NetPolicyQuotaPolicy> &quotaPolicys)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetNetPolicys(quotaPolicys);
}

NetPolicyResultCode NetPolicyClient::SetCellularPolicys(const std::vector<NetPolicyCellularPolicy> &cellularPolicys)
{
    if (cellularPolicys.empty()) {
        NETMGR_LOG_E("cellularPolicys is empty");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetCellularPolicys(cellularPolicys);
}

NetPolicyResultCode NetPolicyClient::GetCellularPolicys(std::vector<NetPolicyCellularPolicy> &cellularPolicys)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetCellularPolicys(cellularPolicys);
}

NetPolicyResultCode NetPolicyClient::ResetFactory(const std::string &subscriberId)
{
    if (subscriberId.empty()) {
        NETMGR_LOG_E("subscriberId is empty");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->ResetFactory(subscriberId);
}

NetPolicyResultCode NetPolicyClient::SetBackgroundPolicy(bool backgroundPolicy)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetBackgroundPolicy(backgroundPolicy);
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

bool NetPolicyClient::GetCurrentBackgroundPolicy()
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return false;
    }

    return proxy->GetCurrentBackgroundPolicy();
}

NetPolicyResultCode NetPolicyClient::SnoozePolicy(const NetPolicyQuotaPolicy &quotaPolicy)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SnoozePolicy(quotaPolicy);
}

NetPolicyResultCode NetPolicyClient::SetIdleWhitelist(uint32_t uid, bool isWhiteList)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->SetIdleWhitelist(uid, isWhiteList);
}

NetPolicyResultCode NetPolicyClient::GetIdleWhitelist(std::vector<uint32_t> &uids)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return proxy->GetIdleWhitelist(uids);
}
} // namespace NetManagerStandard
} // namespace OHOS
