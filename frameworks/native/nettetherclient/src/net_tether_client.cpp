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
#include "net_tether_client.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherClient::NetTetherClient() : netTetherService_(nullptr), deathRecipient_(nullptr) {}

NetTetherClient::~NetTetherClient() {}

int32_t NetTetherClient::TetherByIface(const std::string &iface)
{
    sptr<INetTetherService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    return proxy->TetherByIface(iface);
}

int32_t NetTetherClient::UntetherByIface(const std::string &iface)
{
    sptr<INetTetherService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    return proxy->UntetherByIface(iface);
}

int32_t NetTetherClient::TetherByType(TetheringType type)
{
    sptr<INetTetherService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    return proxy->TetherByType(type);
}

int32_t NetTetherClient::UntetherByType(TetheringType type)
{
    sptr<INetTetherService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    return proxy->UntetherByType(type);
}

int32_t NetTetherClient::RegisterTetheringEventCallback(const sptr<INetTetherCallback> &callback)
{
    sptr<INetTetherService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    return proxy->RegisterTetheringEventCallback(callback);
}

sptr<INetTetherService> NetTetherClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (netTetherService_ != nullptr) {
        NETMGR_LOG_D("get proxy is ok");
        return netTetherService_;
    }
    NETMGR_LOG_D("execute GetSystemAbilityManager");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOG_E("NetTetherClient::GetProxy(), get SystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOG_E("get Remote service failed");
        return nullptr;
    }
    deathRecipient_ = (std::make_unique<NetTetherDeathRecipient>(*this)).release();
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_LOG_E("add death recipient failed");
        return nullptr;
    }
    netTetherService_ = iface_cast<INetTetherService>(remote);
    if (netTetherService_ == nullptr) {
        NETMGR_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return netTetherService_;
}

void NetTetherClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_LOG_D("on remote died");
    if (remote == nullptr) {
        NETMGR_LOG_E("remote object is nullptr");
        return;
    }
    std::lock_guard lock(mutex_);
    if (netTetherService_ == nullptr) {
        NETMGR_LOG_E("netTetherService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = netTetherService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_LOG_E("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    netTetherService_ = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS