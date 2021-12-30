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

#include "net_conn_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"
#include "net_supplier_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnClient::NetConnClient() : NetConnService_(nullptr), deathRecipient_(nullptr) {}

NetConnClient::~NetConnClient() {}

int32_t NetConnClient::SystemReady()
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->SystemReady();
}

int32_t NetConnClient::RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    uint32_t supplierId = 0;
    int32_t result = proxy->RegisterNetSupplier(netType, ident, netCapabilities, supplierId);
    if (result != ERR_NONE) {
        return result;
    }
    return supplierId;
}

int32_t NetConnClient::RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities,
    uint32_t &supplierId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->RegisterNetSupplier(netType, ident, netCapabilities, supplierId);
}

int32_t NetConnClient::UnregisterNetSupplier(uint32_t supplierId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->UnregisterNetSupplier(supplierId);
}

int32_t NetConnClient::RegisterNetSupplierCallback(uint32_t supplierId, const sptr<NetSupplierCallbackBase> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    sptr<NetSupplierCallbackStub> ptr = std::make_unique<NetSupplierCallbackStub>().release();
    ptr->RegisterSupplierCallbackImpl(callback);
    netSupplierCallback_[supplierId] = ptr;
    return proxy->RegisterNetSupplierCallback(supplierId, ptr);
}

int32_t NetConnClient::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("The parameter of proxy is nullptr");
        return NET_CONN_ERR_INTERNAL_ERROR;
    }

    return proxy->RegisterNetConnCallback(callback);
}

int32_t NetConnClient::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("The parameter of proxy is nullptr");
        return NET_CONN_ERR_INTERNAL_ERROR;
    }

    return proxy->RegisterNetConnCallback(netSpecifier, callback);
}

int32_t NetConnClient::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NET_CONN_ERR_INTERNAL_ERROR;
    }

    return proxy->UnregisterNetConnCallback(callback);
}

int32_t NetConnClient::UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NET_CONN_ERR_INTERNAL_ERROR;
    }

    return proxy->UnregisterNetConnCallback(netSpecifier, callback);
}

int32_t NetConnClient::UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    NETMGR_LOG_I("====Test NetConnClient::UpdateNetStateForTest(), begin");
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NET_CONN_ERR_INTERNAL_ERROR;
    }

    return proxy->UpdateNetStateForTest(netSpecifier, netState);
}

int32_t NetConnClient::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
}

int32_t NetConnClient::UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->UpdateNetCapabilities(supplierId, netCapabilities);
}

int32_t NetConnClient::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->UpdateNetLinkInfo(supplierId, netLinkInfo);
}

int32_t NetConnClient::ActivateNetwork(const sptr<NetSpecifier>& netSpecifier,
    const sptr<INetConnCallback>& callback, uint32_t& reqId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->ActivateNetwork(netSpecifier, callback, reqId);
}

int32_t NetConnClient::DeactivateNetwork(uint32_t& reqId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->DeactivateNetwork(reqId);
}

int32_t NetConnClient::GetDefaultNet(int32_t& netId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetDefaultNet(netId);
}

int32_t NetConnClient::GetSpecificNet(uint32_t type, std::list<int32_t> &netIdList)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetSpecificNet(type, netIdList);
}

int32_t NetConnClient::GetAllNets(std::list<int32_t> &netIdList)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetAllNets(netIdList);
}

int32_t NetConnClient::GetSpecificUidNet(int32_t uid, int32_t &netId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetSpecificUidNet(uid, netId);
}

int32_t NetConnClient::GetConnectionProperties(int32_t netId, NetLinkInfo &info)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetConnectionProperties(netId, info);
}

int32_t NetConnClient::GetNetCapabilities(int32_t netId, uint64_t &cap)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetNetCapabilities(netId, cap);
}

int32_t NetConnClient::RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    return proxy->RegisterNetDetectionCallback(netId, callback);
}

int32_t NetConnClient::UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    return proxy->UnRegisterNetDetectionCallback(netId, callback);
}

int32_t NetConnClient::NetDetection(int32_t netId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    return proxy->NetDetection(netId);
}

int32_t NetConnClient::GetIfaceNameByType(uint32_t netType, const std::string &ident, std::string &ifaceName)
{
    if (ident.empty()) {
        NETMGR_LOG_E("ident is nullptr");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->GetIfaceNameByType(netType, ident, ifaceName);
}

sptr<INetConnService> NetConnClient::GetProxy()
{
    std::lock_guard lock(mutex_);

    if (NetConnService_) {
        NETMGR_LOG_D("get proxy is ok");
        return NetConnService_;
    }

    NETMGR_LOG_D("execute GetSystemAbilityManager");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOG_E("GetProxy(), get SystemAbilityManager failed");
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

int32_t NetConnClient::SetAirplaneMode(bool state)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }

    return proxy->SetAirplaneMode(state);
}

void NetConnClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
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
} // namespace NetManagerStandard
} // namespace OHOS
