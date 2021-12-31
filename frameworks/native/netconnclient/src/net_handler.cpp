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

#include "net_handler.h"

#include "net_conn_constants.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetHandler::NetHandler(): NetConnService_(nullptr), deathRecipient_(nullptr) {}

NetHandler::NetHandler(int32_t netId): NetConnService_(nullptr), deathRecipient_(nullptr), netId_(netId) {}

NetHandler::~NetHandler() {}

int32_t NetHandler::GetAddressesByName(const std::string& host, std::list<INetAddr>& addrList)
{
    if (host.empty()) {
        NETMGR_LOG_E("host is empty");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    sptr<INetConnService> proxy = getProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetAddressesByName(host, netId_, addrList);
}

int32_t NetHandler::GetAddressByName(const std::string &host, INetAddr &addr)
{
    if (host.empty()) {
        NETMGR_LOG_E("host is empty");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    sptr<INetConnService> proxy = getProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetAddressByName(host, netId_, addr);
}

int32_t NetHandler::BindSocket(int32_t socket_fd)
{
    if (socket_fd < 0) {
        NETMGR_LOG_E("socket_fd is invalid");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    sptr<INetConnService> proxy = getProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->BindSocket(socket_fd, netId_);
}

sptr<INetConnService> NetHandler::getProxy()
{
    std::lock_guard lock(mutex_);

    if (NetConnService_) {
        NETMGR_LOG_D("get proxy is ok");
        return NetConnService_;
    }

    NETMGR_LOG_D("execute GetSystemAbilityManager");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOG_E("NetConnManager::getProxy(), get SystemAbilityManager failed");
        return nullptr;
    }

    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOG_E("get Remote service failed");
        return nullptr;
    }

    deathRecipient_ = (std::make_unique<NetConnDeathRecipient>(*this)).release();
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_LOG_E("add death recipient failed");
        return nullptr;
    }

    NetConnService_ = iface_cast<INetConnService>(remote);
    if (NetConnService_ == nullptr) {
        NETMGR_LOG_E("get Remote service proxy failed");
        return nullptr;
    }

    return NetConnService_;
}

void NetHandler::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_LOG_D("on remote died");
    if (remote == nullptr) {
        NETMGR_LOG_E("remote object is nullptr");
        return;
    }

    std::lock_guard lock(mutex_);
    if (NetConnService_ == nullptr) {
        NETMGR_LOG_E("NetConnService_ is nullptr");
        return;
    }

    sptr<IRemoteObject> local = NetConnService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_LOG_E("proxy and stub is not same remote object");
        return;
    }

    local->RemoveDeathRecipient(deathRecipient_);
    NetConnService_ = nullptr;
}
} // namespace OHOS
} // namespace NetManagerStandard