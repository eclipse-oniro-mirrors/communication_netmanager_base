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

#include "net_stats_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsClient::NetStatsClient() : netStatsService_(nullptr), deathRecipient_(nullptr) {}

NetStatsClient::~NetStatsClient() {}

int32_t NetStatsClient::RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR);
    }

    return proxy->RegisterNetStatsCallback(callback);
}

int32_t NetStatsClient::UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR);
    }

    return proxy->UnregisterNetStatsCallback(callback);
}

NetStatsResultCode NetStatsClient::GetIfaceStatsDetail(const std::string &iface, uint32_t start, uint32_t end,
    NetStatsInfo &statsInfo)
{
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return proxy->GetIfaceStatsDetail(iface, start, end, statsInfo);
}

NetStatsResultCode NetStatsClient::GetUidStatsDetail(const std::string &iface, uint32_t uid,
    uint32_t start, uint32_t end, NetStatsInfo &statsInfo)
{
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return  proxy->GetUidStatsDetail(iface, uid, start, end, statsInfo);
}

NetStatsResultCode NetStatsClient::UpdateIfacesStats(const std::string &iface,
    uint32_t start, uint32_t end, const NetStatsInfo &stats)
{
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return proxy->UpdateIfacesStats(iface, start, end, stats);
}

NetStatsResultCode NetStatsClient::UpdateStatsData()
{
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return proxy->UpdateStatsData();
}

NetStatsResultCode NetStatsClient::ResetFactory()
{
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return proxy->ResetFactory();
}

sptr<INetStatsService> NetStatsClient::GetProxy()
{
    std::lock_guard lock(mutex_);

    if (netStatsService_ != nullptr) {
        NETMGR_LOG_D("get proxy is ok");
        return netStatsService_;
    }

    NETMGR_LOG_D("execute GetSystemAbilityManager");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOG_E("NetPolicyManager::GetProxy(), get SystemAbilityManager failed");
        return nullptr;
    }

    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_NET_STATS_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOG_E("get Remote service failed");
        return nullptr;
    }

    deathRecipient_ = (std::make_unique<NetStatsDeathRecipient>(*this)).release();
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_LOG_E("add death recipient failed");
        return nullptr;
    }

    netStatsService_ = iface_cast<INetStatsService>(remote);
    if (netStatsService_ == nullptr) {
        NETMGR_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return netStatsService_;
}

void NetStatsClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_LOG_D("on remote died");
    if (remote == nullptr) {
        NETMGR_LOG_E("remote object is nullptr");
        return;
    }

    std::lock_guard lock(mutex_);
    if (netStatsService_ == nullptr) {
        NETMGR_LOG_E("NetConnService_ is nullptr");
        return;
    }

    sptr<IRemoteObject> local = netStatsService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_LOG_E("proxy and stub is not same remote object");
        return;
    }

    local->RemoveDeathRecipient(deathRecipient_);
    netStatsService_ = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS