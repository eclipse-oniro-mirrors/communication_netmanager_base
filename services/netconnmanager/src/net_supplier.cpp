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

#include "common_event_support.h"

#include "event_report.h"
#include "net_activate.h"
#include "net_mgr_log_wrapper.h"
#include "broadcast_manager.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<uint32_t> g_nextNetSupplierId = 0x03EB;
constexpr int32_t REG_OK = 0;

NetSupplier::NetSupplier(
    NetBearType bearerType, const std::string &netSupplierIdent, const std::set<NetCap> &netCaps)
    : netSupplierType_(bearerType), netSupplierIdent_(netSupplierIdent), netCaps_(netCaps),
    supplierId_(g_nextNetSupplierId++)
{
    netAllCapabilities_.netCaps_ = netCaps;
    netAllCapabilities_.bearerTypes_.insert(bearerType);
}

NetSupplier::~NetSupplier() {}

void NetSupplier::RegisterSupplierCallback(const sptr<INetSupplierCallback> &callback)
{
    netController_ = callback;
}

bool NetSupplier::operator==(const NetSupplier &netSupplier) const
{
    return supplierId_ == netSupplier.supplierId_ && netSupplierType_ == netSupplier.netSupplierType_ &&
        netSupplierIdent_ == netSupplier.netSupplierIdent_ && netCaps_ == netSupplier.netCaps_;
}

void NetSupplier::UpdateNetSupplierInfo(const NetSupplierInfo &netSupplierInfo)
{
    NETMGR_LOG_D("Update net supplier[%{public}d, %{public}s], netSupplierInfo[%{public}s]", supplierId_,
                 netSupplierIdent_.c_str(), netSupplierInfo_.ToString("").c_str());
    bool oldAvailable = netSupplierInfo_.isAvailable_;
    netSupplierInfo_ = netSupplierInfo;
    netAllCapabilities_.linkUpBandwidthKbps_ = netSupplierInfo_.linkUpBandwidthKbps_;
    netAllCapabilities_.linkDownBandwidthKbps_ = netSupplierInfo_.linkDownBandwidthKbps_;
    if (oldAvailable == netSupplierInfo_.isAvailable_) {
        return;
    }
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr!");
        return;
    }
    network_->UpdateBasicNetwork(netSupplierInfo_.isAvailable_);
    if (!netSupplierInfo_.isAvailable_) {
        UpdateNetConnState(NET_CONN_STATE_DISCONNECTED);
        netLinkInfo_.Initialize();
    }
    return;
}

int32_t NetSupplier::UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Update netlink info: netLinkInfo[%{public}s]", netLinkInfo.ToString(" ").c_str());
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr!");
        return ERR_NO_NETWORK;
    }

    if (!network_->UpdateNetLinkInfo(netLinkInfo)) {
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    netLinkInfo_ = netLinkInfo;
    UpdateNetConnState(NET_CONN_STATE_CONNECTED);
    return ERR_SERVICE_UPDATE_NET_LINK_INFO_SUCCES;
}

NetBearType NetSupplier::GetNetSupplierType() const
{
    return netSupplierType_;
}

std::string NetSupplier::GetNetSupplierIdent() const
{
    return netSupplierIdent_;
}

const std::set<NetCap> & NetSupplier::GetNetCaps() const
{
    return netCaps_;
}

std::set<NetCap> NetSupplier::GetNetCaps()
{
    return netCaps_;
}

NetAllCapabilities NetSupplier::GetNetCapabilities() const
{
    return netAllCapabilities_;
}

NetLinkInfo NetSupplier::GetNetLinkInfo() const
{
    return netLinkInfo_;
}

void NetSupplier::SetNetwork(const sptr<Network> &network)
{
    network_ = network;
    if (network_ != nullptr) {
        netHandle_ = std::make_unique<NetHandle>(network_->GetNetId()).release();
    }
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

sptr<NetHandle> NetSupplier::GetNetHandle() const
{
    return netHandle_;
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

bool NetSupplier::SupplierConnection(const std::set<NetCap> &netCaps)
{
    NETMGR_LOG_D("param ident[%{public}s]", netSupplierIdent_.c_str());
    if (IsConnecting()) {
        NETMGR_LOG_D("this service is connecting");
        return true;
    }
    if (IsConnected()) {
        NETMGR_LOG_D("this service is already connected");
        return true;
    }
    UpdateNetConnState(NET_CONN_STATE_IDLE);

    if (netController_ == nullptr) {
        NETMGR_LOG_E("netController_ is nullptr");
        return false;
    }
    NETMGR_LOG_D("execute RequestNetwork");
    int32_t errCode = netController_->RequestNetwork(netSupplierIdent_, netCaps);
    NETMGR_LOG_D("RequestNetwork errCode[%{public}d]", errCode);
    if (errCode != REG_OK) {
        NETMGR_LOG_E("RequestNetwork fail");
        return false;
    }
    UpdateNetConnState(NET_CONN_STATE_CONNECTING);
    return true;
}

void NetSupplier::SetRestrictBackground(bool restrictBackground)
{
    restrictBackground_ = restrictBackground;
}
bool NetSupplier::GetRestrictBackground() const
{
    return restrictBackground_;
}

bool NetSupplier::SupplierDisconnection(const std::set<NetCap> &netCaps)
{
    NETMGR_LOG_D("supplier[%{public}d, %{public}s]", supplierId_, netSupplierIdent_.c_str());
    if ((!IsConnecting()) && (!IsConnected())) {
        NETMGR_LOG_D("no need to disconnect");
        return true;
    }
    if (netController_ == nullptr) {
        NETMGR_LOG_E("netController_ is nullptr");
        return false;
    }
    NETMGR_LOG_D("execute ReleaseNetwork, supplierId[%{public}d]", supplierId_);
    int32_t errCode = netController_->ReleaseNetwork(netSupplierIdent_, netCaps);
    NETMGR_LOG_D("ReleaseNetwork retCode[%{public}d]", errCode);
    if (errCode != REG_OK) {
        NETMGR_LOG_E("ReleaseNetwork fail");
        return false;
    }
    return true;
}

void NetSupplier::UpdateNetConnState(NetConnState netConnState)
{
    switch (netConnState) {
        case NET_CONN_STATE_IDLE:
        case NET_CONN_STATE_CONNECTING:
        case NET_CONN_STATE_CONNECTED:
        case NET_CONN_STATE_DISCONNECTING:
        case NET_CONN_STATE_DISCONNECTED:
            state_ = netConnState;
            break;
        default:
            state_ = NET_CONN_STATE_UNKNOWN;
            break;
    }

    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE;
    info.data = "Net Manager Connection State Changed";
    info.code = static_cast<int32_t>(netConnState);
    info.ordered = true;
    std::map<std::string, int32_t> param = {{"NetType", static_cast<int32_t>(netSupplierType_)}};
    DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);
    NETMGR_LOG_D("supplier[%{public}d, %{public}s], serviceState[%{public}d]", supplierId_, netSupplierIdent_.c_str(),
                 state_);
}

NetConnState NetSupplier::GetNetConnState() const
{
    return state_;
}

bool NetSupplier::IsConnecting() const
{
    bool isConnecting = false;

    switch (state_) {
        case NET_CONN_STATE_UNKNOWN:
        case NET_CONN_STATE_IDLE:
            break;
        case NET_CONN_STATE_CONNECTING:
            isConnecting = true;
            break;
        case NET_CONN_STATE_CONNECTED:
        case NET_CONN_STATE_DISCONNECTING:
        case NET_CONN_STATE_DISCONNECTED:
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
        case NET_CONN_STATE_UNKNOWN:
        case NET_CONN_STATE_IDLE:
        case NET_CONN_STATE_CONNECTING:
        case NET_CONN_STATE_DISCONNECTING:
        case NET_CONN_STATE_DISCONNECTED:
            break;
        case NET_CONN_STATE_CONNECTED:
            isConnected = true;
            break;
        default:
            break;
    }
    NETMGR_LOG_D("isConnected is [%{public}d]", isConnected);
    return isConnected;
}

void NetSupplier::AddRequsetIdToList(uint32_t requestId)
{
    NETMGR_LOG_D("AddRequsetIdToList reqId = [%{public}u]", requestId);
    requestList_.insert(requestId);
    return;
}

void NetSupplier::UpdateNetStateForTest(int32_t netState)
{
    NETMGR_LOG_I("Test NetSupplier::UpdateNetStateForTest(), begin");
}

bool NetSupplier::RequestToConnect(uint32_t reqId)
{
    requestList_.insert(reqId);
    return SupplierConnection(netCaps_);
}

int32_t NetSupplier::SelectAsBestNetwork(uint32_t reqId)
{
    NETMGR_LOG_D("NetSupplier::SelectAsBestNetwork");
    requestList_.insert(reqId);
    bestReqList_.insert(reqId);
    return ERR_NONE;
}

void NetSupplier::ReceiveBestScore(uint32_t reqId, int32_t bestScore, uint32_t supplierId)
{
    NETMGR_LOG_D("NetSupplier::ReceiveBestScore, supplierId[%{public}d, %{public}s], bestSupplierId[%{public}d]",
                 supplierId_, netSupplierIdent_.c_str(), supplierId);
    if (requestList_.empty()) {
        SupplierDisconnection(netCaps_);
        return;
    }
    std::set<uint32_t>::iterator iter = requestList_.find(reqId);
    if (iter == requestList_.end()) {
        NETMGR_LOG_D("NetSupplier::ReceiveBestScore, supplierId[%{public}d], can not find request[%{public}d]",
                     supplierId_, reqId);
        return;
    }
    if (supplierId != supplierId_ && netScore_ < bestScore) {
        requestList_.erase(reqId);
        if (requestList_.empty()) {
            SupplierDisconnection(netCaps_);
        }
        bestReqList_.erase(reqId);
    }
}

int32_t NetSupplier::CancelRequest(uint32_t reqId)
{
    std::set<uint32_t>::iterator iter = requestList_.find(reqId);
    if (iter == requestList_.end()) {
        return ERR_SERVICE_NO_REQUEST;
    }
    requestList_.erase(reqId);
    if (requestList_.empty()) {
        SupplierDisconnection(netCaps_);
    }
    bestReqList_.erase(reqId);
    return ERR_NONE;
}

void NetSupplier::RemoveBestRequest(uint32_t reqId)
{
    NETMGR_LOG_D("Enter RemoveBestRequest");
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
    NETMGR_LOG_D("Enter SetNetValid. supplier[%{public}d, %{public}s], ifValid[%{public}d]", supplierId_,
                 netSupplierIdent_.c_str(), ifValid);
    ifNetValid_ = ifValid;
    if (netAllCapabilities_.netCaps_.find(NET_CAPABILITY_VALIDATED) != netAllCapabilities_.netCaps_.end()) {
        netAllCapabilities_.netCaps_.erase(NET_CAPABILITY_VALIDATED);
    }
    if (ifNetValid_) {
        netAllCapabilities_.netCaps_.insert(NET_CAPABILITY_VALIDATED);
    }
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
