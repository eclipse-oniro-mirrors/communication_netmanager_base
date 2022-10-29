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

#include "net_stats_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsClient::NetStatsClient() : netStatsService_(nullptr), deathRecipient_(nullptr) {}

NetStatsClient::~NetStatsClient() = default;

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

int64_t NetStatsClient::GetIfaceRxBytes(const std::string &interfaceName)
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetIfaceRxBytes(interfaceName);
}

int64_t NetStatsClient::GetIfaceTxBytes(const std::string &interfaceName)
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetIfaceTxBytes(interfaceName);
}

int64_t NetStatsClient::GetCellularRxBytes()
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetCellularRxBytes();
}

int64_t NetStatsClient::GetCellularTxBytes()
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetCellularTxBytes();
}

int64_t NetStatsClient::GetAllRxBytes()
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetAllRxBytes();
}

int64_t NetStatsClient::GetAllTxBytes()
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetAllTxBytes();
}

int64_t NetStatsClient::GetUidRxBytes(uint32_t uid)
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetUidRxBytes(uid);
}

int64_t NetStatsClient::GetUidTxBytes(uint32_t uid)
{
    int64_t err = -1;
    sptr<INetStatsService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return err;
    }
    return proxy->GetUidTxBytes(uid);
}
} // namespace NetManagerStandard
} // namespace OHOS