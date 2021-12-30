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
#include "net_supplier.h"

#include <atomic>
#include <cinttypes>

#include "net_activate.h"
#include "net_mgr_log_wrapper.h"
#include "broadcast_manager.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<uint32_t> g_nextNetSupplierId = 0x03EB;

NetSupplier::NetSupplier(NetworkType netSupplierType, const std::string &netSupplierIdent, uint64_t netCapabilities)
    : netSupplierType_ (netSupplierType),
      netSupplierIdent_(netSupplierIdent),
      netCapabilities_(netCapabilities),
      supplierId_(g_nextNetSupplierId++) {}

NetSupplier::~NetSupplier() {}

void NetSupplier::RegisterSupplierCallback(const sptr<INetSupplierCallback> &callback)
{
    netController_ = callback;
}

bool NetSupplier::operator==(const NetSupplier &netSupplier) const
{
    return supplierId_ == netSupplier.supplierId_ && netSupplierType_ == netSupplier.netSupplierType_ &&
        netSupplierIdent_ == netSupplier.netSupplierIdent_ && netCapabilities_ == netSupplier.netCapabilities_;
}

void NetSupplier::UpdateNetSupplierInfo(const NetSupplierInfo &netSupplierInfo)
{
    NETMGR_LOG_D("Update net supplier info: netSupplierInfo[%{public}s]", netSupplierInfo_.ToString("").c_str());
    bool oldAvailable = netSupplierInfo_.isAvailable_;
    netSupplierInfo_ = netSupplierInfo;
    if (oldAvailable == netSupplierInfo_.isAvailable_) {
        return;
    }
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr!");
        return;
    }
    network_->UpdateBasicNetwork(netSupplierInfo_.isAvailable_);
    if (!netSupplierInfo_.isAvailable_) {
        UpdateServiceState(SERVICE_STATE_DISCONNECTED);
    }
    return;
}

void NetSupplier::UpdateNetCapabilities(uint64_t netCapabilities)
{
    netCapabilities_ = netCapabilities;
}

int32_t NetSupplier::UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Update netlink info: netLinkInfo[%{public}s]", netLinkInfo.ToStringBase("").c_str());
    NETMGR_LOG_D("[%{public}s]", netLinkInfo.ToStringAddr("").c_str());
    NETMGR_LOG_D("[%{public}s]", netLinkInfo.ToStringDns("").c_str());
    NETMGR_LOG_D("[%{public}s]", netLinkInfo.ToStringRoute("").c_str());
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr!");
        return ERR_NO_NETWORK;
    }

    if (!network_->UpdateNetLinkInfo(netLinkInfo)) {
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    netLinkInfo_ = netLinkInfo;
    UpdateServiceState(SERVICE_STATE_CONNECTED);
    return ERR_SERVICE_UPDATE_NET_LINK_INFO_SUCCES;
}

NetworkType NetSupplier::GetNetSupplierType() const
{
    return netSupplierType_;
}

std::string NetSupplier::GetNetSupplierIdent() const
{
    return netSupplierIdent_;
}

uint64_t NetSupplier::GetNetCapabilities() const
{
    return netCapabilities_;
}

NetLinkInfo NetSupplier::GetNetLinkInfo() const
{
    return netLinkInfo_;
}

void NetSupplier::SetNetwork(const sptr<Network> &network)
{
    network_ = network;
}

sptr<Network> NetSupplier::GetNetwork() const
{
    return network_;
}

int32_t NetSupplier::GetNetId() const
{
    if (network_ == nullptr) {
        return INVALID_NET_ID;
    }
    return network_->GetNetId();
}

uint32_t NetSupplier::GetSupplierId() const
{
    return supplierId_;
}

bool NetSupplier::GetRoaming() const
{
    return netSupplierInfo_.isRoaming_;
}

int8_t NetSupplier::GetStrength() const
{
    return netSupplierInfo_.strength_;
}

uint16_t NetSupplier::GetFrequency() const
{
    return netSupplierInfo_.frequency_;
}

int32_t NetSupplier::GetSupplierUid() const
{
    return netSupplierInfo_.uid_;
}

bool NetSupplier::SupplierConnection(uint64_t netCapabilities)
{
    NETMGR_LOG_D("param ident[%{public}s] netCapabilities[%{public}" PRId64 "]", netSupplierIdent_.c_str(),
        netCapabilities);
    if (IsConnecting()) {
        NETMGR_LOG_D("this service is connecting");
        return true;
    }
    if (IsConnected()) {
        NETMGR_LOG_D("this service is already connected");
        return true;
    }
    UpdateServiceState(SERVICE_STATE_IDLE);

    if (netController_ == nullptr) {
        NETMGR_LOG_E("netController_ is nullptr");
        return false;
    }
    NETMGR_LOG_D("execute RequestNetwork");
    int32_t errCode = netController_->RequestNetwork(netSupplierIdent_, netCapabilities);
    NETMGR_LOG_D("RequestNetwork errCode[%{public}d]", errCode);
    if (errCode != REG_OK) {
        NETMGR_LOG_E("RequestNetwork fail");
        return false;
    }
    UpdateServiceState(SERVICE_STATE_READY);
    return true;
}

bool NetSupplier::SupplierDisconnection(uint64_t netCapabilities)
{
    NETMGR_LOG_D("param ident_[%{public}s] netCapabilities[%{public}" PRId64 "]", netSupplierIdent_.c_str(),
        netCapabilities);
    if ((!IsConnecting()) && (IsConnected())) {
        NETMGR_LOG_D("no need to disconnect");
        return true;
    }
    if (netController_ == nullptr) {
        NETMGR_LOG_E("netController_ is nullptr");
        return false;
    }
    NETMGR_LOG_D("execute ReleaseNetwork");
    int32_t errCode = netController_->ReleaseNetwork(netSupplierIdent_, netCapabilities);
    NETMGR_LOG_D("ReleaseNetwork errCode[%{public}d]", errCode);
    if (errCode != REG_OK) {
        NETMGR_LOG_E("ReleaseNetwork fail");
        return false;
    }
    UpdateServiceState(SERVICE_STATE_DISCONNECTING);
    return true;
}

void NetSupplier::UpdateServiceState(ServiceState serviceState)
{
    switch (serviceState) {
        case SERVICE_STATE_IDLE:
        case SERVICE_STATE_CONNECTING:
        case SERVICE_STATE_READY:
        case SERVICE_STATE_CONNECTED:
        case SERVICE_STATE_DISCONNECTING:
        case SERVICE_STATE_DISCONNECTED:
        case SERVICE_STATE_FAILURE:
            state_ = serviceState;
            break;
        case SERVICE_STATE_UNKNOWN:
        default:
            state_ = SERVICE_STATE_FAILURE;
            break;
    }

    BroadcastInfo info;
    // EventFwk::CommonEventSupport::COMMON_EVENT_NETMANAGER_CONNECTION_STATE_CHANGED
    info.action = "usual.event.netmanager.NETMANAGER_CONNECTION_STATE_CHANGED";
    info.data = "Net Manager Connection State Changed";
    info.code = static_cast<int32_t>(serviceState);
    info.ordered = true;
    std::string netTypeName = std::to_string(static_cast<int32_t>(netSupplierType_));
    std::map<std::string, std::string> param = {{"NetType", netTypeName}};
    DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);

    sptr<NetConnCallbackInfo> netConnCallback = (std::make_unique<NetConnCallbackInfo>()).release();
    if (netConnCallback == nullptr) {
        NETMGR_LOG_E("make_unique<NetConnCallbackInfo>() failed");
        return;
    }
    netConnCallback->netState_ = static_cast<int32_t>(serviceState);
    netConnCallback->netType_ = static_cast<int32_t>(netSupplierType_);
    NotifyNetConnStateChanged(netConnCallback);

    NETMGR_LOG_D("serviceState is [%{public}d]", state_);
}

ServiceState NetSupplier::GetServiceState() const
{
    return state_;
}

bool NetSupplier::IsConnecting() const
{
    bool isConnecting = false;

    switch (state_) {
        case SERVICE_STATE_UNKNOWN:
        case SERVICE_STATE_FAILURE:
        case SERVICE_STATE_IDLE:
            break;
        case SERVICE_STATE_READY:
        case SERVICE_STATE_CONNECTING:
            isConnecting = true;
            break;
        case SERVICE_STATE_CONNECTED:
        case SERVICE_STATE_DISCONNECTING:
        case SERVICE_STATE_DISCONNECTED:
        default:
            break;
    }

    NETMGR_LOG_D("isConnecting is [%{public}d]", isConnecting);
    return isConnecting;
}

bool NetSupplier::IsConnected() const
{
    bool isConnected = false;
    switch (state_) {
        case SERVICE_STATE_UNKNOWN:
        case SERVICE_STATE_FAILURE:
        case SERVICE_STATE_IDLE:
        case SERVICE_STATE_CONNECTING:
        case SERVICE_STATE_READY:
        case SERVICE_STATE_DISCONNECTING:
        case SERVICE_STATE_DISCONNECTED:
            break;
        case SERVICE_STATE_CONNECTED:
            isConnected = true;
            break;
        default:
            break;
    }
    NETMGR_LOG_D("isConnected is [%{public}d]", isConnected);
    return isConnected;
}

void NetSupplier::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return;
    }

    if (std::find(netConnCallback_.begin(), netConnCallback_.end(), callback) != netConnCallback_.end()) {
        NETMGR_LOG_D("netConnCallback_ had this callback");
        return;
    }
    netConnCallback_.emplace_back(callback);
}

void NetSupplier::RegisterNetConnCallback(const std::vector<sptr<INetConnCallback>> &callback)
{
    for (const auto &callbackEx : callback) {
        netConnCallback_.emplace_back(callbackEx);
    }
}

void NetSupplier::AddRequsetIdToList(uint32_t requestId)
{
    NETMGR_LOG_D("AddRequsetIdToList reqId = [%{public}u]", requestId);
    requestList_.insert(requestId);
    return;
}

void NetSupplier::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return;
    }

    for (auto iter = netConnCallback_.begin(); iter != netConnCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            netConnCallback_.erase(iter);
            break;
        }
    }
}

int32_t NetSupplier::NotifyNetConnStateChanged(const sptr<NetConnCallbackInfo> &info)
{
    for (const auto &callback : netConnCallback_) {
        callback->NetConnStateChanged(info);
    }

    return ERR_NONE;
}

void NetSupplier::UpdateNetStateForTest(int32_t netState)
{
    NETMGR_LOG_I("Test NetSupplier::UpdateNetStateForTest(), begin");
    sptr<NetConnCallbackInfo> callbackInfo = (std::make_unique<NetConnCallbackInfo>()).release();
    if (callbackInfo == nullptr) {
        NETMGR_LOG_E("make_unique<callbackInfo>() failed");
        return;
    }
    NETMGR_LOG_I("Test NetService::UpdateNetStateForTest(), netState[%{public}d], networkType_[%{public}d]",
        netState, netSupplierType_);
    callbackInfo->netState_ = netState;
    callbackInfo->netType_ = static_cast<uint32_t>(netSupplierType_);
    NotifyNetConnStateChanged(callbackInfo);
}

bool NetSupplier::RequestToConnect(uint32_t reqId)
{
    requestList_.insert(reqId);
    return SupplierConnection(netCapabilities_);
}

int32_t NetSupplier::SelectAsBestNetwork(uint32_t reqId)
{
    NETMGR_LOG_D("NetSupplier::SelectAsBestNetwork");
    requestList_.insert(reqId);
    bestReqList_.insert(reqId);
    return ERR_NONE;
}

void NetSupplier::ReceiveBestScore(uint32_t reqId, int32_t bestScore)
{
    std::set<uint32_t>::iterator iter = requestList_.find(reqId);
    if (iter == requestList_.end()) {
        return;
    }
    if (netScore_ < bestScore) {
        requestList_.erase(reqId);
        if (requestList_.empty()) {
            SupplierDisconnection(netCapabilities_);
        }
        bestReqList_.erase(reqId);
    }
    return;
}


int32_t NetSupplier::CancleRequest(uint32_t reqId)
{
    std::set<uint32_t>::iterator iter = requestList_.find(reqId);
    if (iter == requestList_.end()) {
        return ERR_SERVICE_NO_REQUEST;
    }
    requestList_.erase(reqId);
    if (requestList_.empty()) {
        SupplierDisconnection(netCapabilities_);
    }
    bestReqList_.erase(reqId);
    return ERR_NONE;
}

void NetSupplier::RemoveBestRequest(uint32_t reqId)
{
    NETMGR_LOG_D("RemoveBestRequest Enter");
    auto iter = bestReqList_.find(reqId);
    if (iter == bestReqList_.end()) {
        return;
    }
    bestReqList_.erase(reqId);
    return;
}

std::set<uint32_t>& NetSupplier::GetBestRequestList()
{
    return bestReqList_;
}

void NetSupplier::SetNetValid(bool ifValid)
{
    ifNetValid_ = ifValid;
}
bool NetSupplier::IfNetValid()
{
    return ifNetValid_;
}
void NetSupplier::SetNetScore(int32_t score)
{
    netScore_ = score;
    NETMGR_LOG_D("netScore_ = %{public}d", netScore_);
}

int32_t NetSupplier::GetNetScore() const
{
    return netScore_;
}

void NetSupplier::SetRealScore(int32_t score)
{
    netRealScore_ = score;
    NETMGR_LOG_D("netRealScore_ = %{public}d", netRealScore_);
}

int32_t NetSupplier::GetRealScore()
{
    return netRealScore_;
}

void NetSupplier::SetDefault()
{
    if (network_) {
        network_->SetDefaultNetWork();
    }
}

void NetSupplier::ClearDefault()
{
    if (network_) {
        network_->ClearDefaultNetWorkNetId();
    }
}
} // namespace NetManagerStandard
} // namespace OHOS