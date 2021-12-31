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

#include "net_tether_request_network.h"
#include "net_mgr_log_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherRequestNetwork::TetherRequestNetworkCallback::TetherRequestNetworkCallback(
    NetTetherRequestNetwork &netTetherRequestNetwork) : netTetherRequestNetwork_(netTetherRequestNetwork) {}

NetTetherRequestNetwork::TetherRequestNetworkCallback::~TetherRequestNetworkCallback() {}

int32_t NetTetherRequestNetwork::TetherRequestNetworkCallback::NetConnStateChanged(
    const sptr<NetConnCallbackInfo> &info)
{
    return 0;
}

int32_t NetTetherRequestNetwork::TetherRequestNetworkCallback::NetAvailable(int32_t netId)
{
    NETMGR_LOG_D("TetherRequestNetworkCallback::NetAvailable, netId: [%{public}d]", netId);
    netTetherRequestNetwork_.NetAvailable(netId);
    return 0;
}

int32_t NetTetherRequestNetwork::TetherRequestNetworkCallback::NetCapabilitiesChange(int32_t netId,
    const uint64_t &netCap)
{
    NETMGR_LOG_D("TetherRequestNetworkCallback::NetCapabilitiesChange, netId: [%{public}d]", netId);
    return 0;
}

int32_t NetTetherRequestNetwork::TetherRequestNetworkCallback::NetConnectionPropertiesChange(int32_t netId,
    const sptr<NetLinkInfo> &info)
{
    NETMGR_LOG_D("TetherRequestNetworkCallback::NetConnectionPropertiesChange, netId: [%{public}d]", netId);
    netTetherRequestNetwork_.NetConnectionPropertiesChange(netId, *info);
    return 0;
}

int32_t NetTetherRequestNetwork::TetherRequestNetworkCallback::NetLost(int32_t netId)
{
    NETMGR_LOG_D("TetherRequestNetworkCallback::NetLost, netId: [%{public}d]", netId);
    netTetherRequestNetwork_.NetLost(netId);
    return 0;
}

NetTetherRequestNetwork::NetTetherRequestNetwork()
{
    netConncallback_ = (std::make_unique<TetherRequestNetworkCallback>(*this)).release();
    netConnService_ = GetProxy();
    sptr<NetSpecifier> netSpecifier = (std::make_unique<NetSpecifier>()).release();
    netSpecifier->ident_ = "ident";
    netSpecifier->netType_ = NET_TYPE_CELLULAR;
    netSpecifier->netCapabilities_ = NET_CAPABILITIES_INTERNET;
    if (netConnService_ != nullptr) {
        netConnService_->ActivateNetwork(netSpecifier, netConncallback_, reqId_);
    } else {
        NETMGR_LOG_D("netConnService_ == nullptr!");
    }
}

NetTetherRequestNetwork::~NetTetherRequestNetwork()
{
    if (netConnService_ != nullptr) {
        netConnService_->DeactivateNetwork(reqId_);
    }
}

void NetTetherRequestNetwork::RerequestNetwork()
{
    sptr<NetSpecifier> netSpecifier = (std::make_unique<NetSpecifier>()).release();
    netSpecifier->ident_ = "ident";
    netSpecifier->netType_ = NET_TYPE_ETHERNET;
    netSpecifier->netCapabilities_ = NET_CAPABILITIES_INTERNET;
    if (netConnService_ != nullptr) {
        netConnService_->DeactivateNetwork(reqId_);
        netConnService_->ActivateNetwork(netSpecifier, netConncallback_, reqId_);
    }
}

void NetTetherRequestNetwork::RegisterNetRequestCallback(const RequestNetworkCallback &callback)
{
    callback_ = callback;
}

int32_t NetTetherRequestNetwork::GetUpstreamNetId() const
{
    return netId_;
}

const NetLinkInfo &NetTetherRequestNetwork::GetUpstreamLinkInfo() const
{
    return info_;
}

int32_t NetTetherRequestNetwork::NetAvailable(int32_t netId)
{
    netId_ = netId;
    return netId;
}

int32_t NetTetherRequestNetwork::NetLost(int32_t netId)
{
    netId_ = -1;
    info_ = NetLinkInfo();
    if (callback_.NetLost != nullptr) {
        callback_.NetLost(netId);
    }
    return netId;
}

int32_t NetTetherRequestNetwork::NetConnectionPropertiesChange(int32_t netId, const NetLinkInfo &info)
{
    if (netId_ == netId) {
        info_ = info;
    } else {
        NETMGR_LOG_E("Find [%{public}d] netId failed when NetConnectionPropertiesChange called", netId);
    }
    return netId;
}

sptr<INetConnService> NetTetherRequestNetwork::GetProxy()
{
    NETMGR_LOG_D("NetConnService GetProxy ... ");
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        return nullptr;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remote) {
        sptr<INetConnService> netConnService = iface_cast<INetConnService>(remote);
        NETMGR_LOG_D("NetConnService Get COMM_NET_CONN_MANAGER_SYS_ABILITY_ID success ... ");
        return netConnService;
    } else {
        NETMGR_LOG_D("NetConnService Get COMM_NET_CONN_MANAGER_SYS_ABILITY_ID fail ... ");
        return nullptr;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS