/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <atomic>
#include <cinttypes>

#include "broadcast_manager.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_supplier.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t REG_OK = 0;
constexpr const char *SIMID_IDENT_PREFIX = "simId";
}
static std::atomic<uint32_t> g_nextNetSupplierId = 0x03EB;

NetSupplier::NetSupplier(NetBearType bearerType, const std::string &netSupplierIdent, const std::set<NetCap> &netCaps)
    : netSupplierType_(bearerType),
      netSupplierIdent_(netSupplierIdent),
      netCaps_(netCaps),
      supplierId_(g_nextNetSupplierId++)
{
    netAllCapabilities_.netCaps_ = netCaps;
    netAllCapabilities_.bearerTypes_.insert(bearerType);
    ResetNetSupplier();
    InitNetScore();
}

sptr<INetSupplierCallback> NetSupplier::GetSupplierCallback()
{
    return netController_;
}

void NetSupplier::RegisterSupplierCallback(const sptr<INetSupplierCallback> &callback)
{
    netController_ = callback;
}

void NetSupplier::InitNetScore()
{
    int32_t netScore = 0;
    auto iter = netTypeScore_.find(netSupplierType_);
    if (iter == netTypeScore_.end()) {
        NETMGR_LOG_E("Can not find net bearer type[%{public}d] for this net service", netSupplierType_);
        return;
    }
    NETMGR_LOG_D("Net type[%{public}d],default score[%{public}d]",
                 static_cast<int32_t>(iter->first), static_cast<int32_t>(iter->second));
    netScore = static_cast<int32_t>(iter->second);
    netScore_ = netScore;
    NETMGR_LOG_D("netScore_ = %{public}d", netScore_);
}

/**
 * Reset all attributes that may change in the supplier, such as detection progress and network quality.
 */
void NetSupplier::ResetNetSupplier()
{
    // Reset network quality.
    netQuality_ = QUALITY_NORMAL_STATE;
    // Reset network detection progress.
    isFirstTimeDetectionDone = false;
    // Reset network capabilities for checking connectivity finished flag.
    netCaps_.InsertNetCap(NET_CAPABILITY_CHECKING_CONNECTIVITY);
    netAllCapabilities_.netCaps_.insert(NET_CAPABILITY_CHECKING_CONNECTIVITY);
    NETMGR_LOG_I("Reset net supplier %{public}u", supplierId_);
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
    if (!netSupplierInfo_.ident_.empty()) {
        netSupplierIdent_ = netSupplierInfo_.ident_;
    }
    if (netSupplierInfo_.score_ != 0) {
        netScore_ = netSupplierInfo_.score_;
    }
    if (oldAvailable == netSupplierInfo_.isAvailable_) {
        NETMGR_LOG_W("Same supplier available status:[%{public}d]", oldAvailable);
        return;
    }
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr!");
        return;
    }
    network_->UpdateBasicNetwork(netSupplierInfo_.isAvailable_);
    if (!netSupplierInfo_.isAvailable_) {
        UpdateNetConnState(NET_CONN_STATE_DISCONNECTED);
    }
}

int32_t NetSupplier::UpdateNetLinkInfo(NetLinkInfo &netLinkInfo)
{
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr!");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    if (GetNetSupplierIdent().substr(0, strlen(SIMID_IDENT_PREFIX)) == SIMID_IDENT_PREFIX) {
        netLinkInfo.ident_ = GetNetSupplierIdent().substr(strlen(SIMID_IDENT_PREFIX));
    }
    NETMGR_LOG_D("Update netlink info: netLinkInfo[%{public}s]", netLinkInfo.ToString(" ").c_str());
    if (!network_->UpdateNetLinkInfo(netLinkInfo)) {
        return NET_CONN_ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    UpdateNetConnState(NET_CONN_STATE_CONNECTED);
    return NETMANAGER_SUCCESS;
}

NetBearType NetSupplier::GetNetSupplierType() const
{
    return netSupplierType_;
}

std::string NetSupplier::GetNetSupplierIdent() const
{
    return netSupplierIdent_;
}

bool NetSupplier::CompareNetCaps(const std::set<NetCap> caps) const
{
    if (caps.empty()) {
        return true;
    }
    return netCaps_.HasNetCaps(caps);
}

bool NetSupplier::HasNetCap(NetCap cap) const
{
    return netCaps_.HasNetCap(cap);
}

bool NetSupplier::HasNetCaps(const std::set<NetCap> &caps) const
{
    return netCaps_.HasNetCaps(caps);
}

const NetCaps &NetSupplier::GetNetCaps() const
{
    return netCaps_;
}

NetAllCapabilities NetSupplier::GetNetCapabilities() const
{
    return netAllCapabilities_;
}

void NetSupplier::SetNetwork(const std::shared_ptr<Network> &network)
{
    network_ = network;
    if (network_ != nullptr) {
        netHandle_ = std::make_unique<NetHandle>(network_->GetNetId()).release();
    }
}

std::shared_ptr<Network> NetSupplier::GetNetwork() const
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

void NetSupplier::GetHttpProxy(HttpProxy &httpProxy)
{
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr.");
        return;
    }
    httpProxy = network_->GetNetLinkInfo().httpProxy_;
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

bool NetSupplier::IsAvailable() const
{
    return netSupplierInfo_.isAvailable_;
}

bool NetSupplier::SupplierConnection(const std::set<NetCap> &netCaps, const NetRequest &netRequest)
{
    NETMGR_LOG_D("Supplier[%{public}d, %{public}s] request connect, available=%{public}d", supplierId_,
                 netSupplierIdent_.c_str(), netSupplierInfo_.isAvailable_);
    if (netSupplierInfo_.isAvailable_ && (netRequest.ident.empty())) {
        NETMGR_LOG_D("The supplier is currently available, there is no need to repeat the request for connection.");
        return true;
    }
    UpdateNetConnState(NET_CONN_STATE_IDLE);

    if (netController_ == nullptr) {
        NETMGR_LOG_E("netController_ is nullptr");
        return false;
    }
    NETMGR_LOG_D("execute RequestNetwork");
    int32_t errCode = netController_->RequestNetwork(netSupplierIdent_, netCaps, netRequest);
    NETMGR_LOG_D("RequestNetwork errCode[%{public}d]", errCode);
    if (errCode != REG_OK) {
        NETMGR_LOG_E("RequestNetwork fail");
        return false;
    }
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
    NETMGR_LOG_D("Supplier[%{public}d, %{public}s] request disconnect, available=%{public}d", supplierId_,
                 netSupplierIdent_.c_str(), netSupplierInfo_.isAvailable_);
    if (!netSupplierInfo_.isAvailable_) {
        NETMGR_LOG_D("The supplier is currently unavailable, there is no need to repeat the request to disconnect.");
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
    if (network_) {
        network_->UpdateNetConnState(netConnState);
    }
}

bool NetSupplier::IsConnecting() const
{
    if (network_) {
        return network_->IsConnecting();
    }
    return false;
}

bool NetSupplier::IsConnected() const
{
    if (network_) {
        return network_->IsConnected();
    }
    return false;
}

bool NetSupplier::RequestToConnect(const NetRequest &netrequest)
{
    if (requestList_.find(netrequest.requestId) == requestList_.end()) {
        requestList_.insert(netrequest.requestId);
    }
    AddRequest(netrequest);
    return SupplierConnection(netCaps_.ToSet(), netrequest);
}

int32_t NetSupplier::SelectAsBestNetwork(const NetRequest &netrequest)
{
    NETMGR_LOG_I("Request[%{public}d] select [%{public}d, %{public}s] as best network", netrequest.requestId,
                 supplierId_, netSupplierIdent_.c_str());
    if (requestList_.find(netrequest.requestId) == requestList_.end()) {
        requestList_.insert(netrequest.requestId);
    }
    if (bestReqList_.find(netrequest.requestId) == bestReqList_.end()) {
        bestReqList_.insert(netrequest.requestId);
    }
    AddRequest(netrequest);
    return NETMANAGER_SUCCESS;
}

void NetSupplier::ReceiveBestScore(int32_t bestScore, uint32_t supplierId, const NetRequest &netrequest)
{
    NETMGR_LOG_D("Supplier[%{public}d, %{public}s] receive best score, bestSupplierId[%{public}d]", supplierId_,
                 netSupplierIdent_.c_str(), supplierId);
    if (supplierId == supplierId_) {
        NETMGR_LOG_D("Same net supplier, no need to disconnect.");
        return;
    }
    if (requestList_.empty()) {
        SupplierDisconnection(netCaps_.ToSet());
        return;
    }
    if (requestList_.find(netrequest.requestId) == requestList_.end()) {
        NETMGR_LOG_W("Can not find request[%{public}d]", netrequest.requestId);
        return;
    }
    if (netScore_ >= bestScore) {
        NETMGR_LOG_D("High priority network, no need to disconnect");
        return;
    }
    requestList_.erase(netrequest.requestId);
    NETMGR_LOG_D("Supplier[%{public}d, %{public}s] remaining request list size[%{public}zd]", supplierId_,
                 netSupplierIdent_.c_str(), requestList_.size());
    if (requestList_.empty()) {
        SupplierDisconnection(netCaps_.ToSet());
    }
    RemoveRequest(netrequest);
}

int32_t NetSupplier::CancelRequest(const NetRequest &netrequest)
{
    auto iter = requestList_.find(netrequest.requestId);
    if (iter == requestList_.end()) {
        return NET_CONN_ERR_SERVICE_NO_REQUEST;
    }
    NETMGR_LOG_I("CancelRequest requestId:%{public}u", netrequest.requestId);
    requestList_.erase(netrequest.requestId);
    if (requestList_.empty()) {
        SupplierDisconnection(netCaps_.ToSet());
    }
    bestReqList_.erase(netrequest.requestId);
    RemoveRequest(netrequest);
    return NETMANAGER_SUCCESS;
}

void NetSupplier::AddRequest(const NetRequest &netRequest)
{
    if (netController_ == nullptr) {
        NETMGR_LOG_E("netController_ is nullptr");
        return;
    }
    NetRequest request;
    request.uid = netRequest.uid;
    request.ident = netSupplierIdent_;
    request.netCaps = netCaps_.ToSet();
    NETMGR_LOG_D("execute AddRequest");
    int32_t errCode = netController_->AddRequest(request);
    NETMGR_LOG_D("AddRequest errCode[%{public}d]", errCode);
    if (errCode != REG_OK) {
        NETMGR_LOG_E("AddRequest fail");
        return;
    }
    return;
}

void NetSupplier::RemoveRequest(const NetRequest &netrequest)
{
    if (netController_ == nullptr) {
        NETMGR_LOG_E("netController_ is nullptr");
        return;
    }
    NetRequest request;
    request.uid = netrequest.uid;
    request.ident = netSupplierIdent_;
    request.netCaps = netCaps_.ToSet();
    NETMGR_LOG_D("execute RemoveRequest");
    int32_t errCode = netController_->RemoveRequest(request);
    NETMGR_LOG_D("RemoveRequest errCode[%{public}d]", errCode);
    if (errCode != REG_OK) {
        NETMGR_LOG_E("RemoveRequest fail");
        return;
    }
    return;
}

void NetSupplier::RemoveBestRequest(uint32_t reqId)
{
    auto iter = bestReqList_.find(reqId);
    if (iter == bestReqList_.end()) {
        return;
    }
    bestReqList_.erase(reqId);
    NETMGR_LOG_I("RemoveBestRequest supplierId=[%{public}d], reqId=[%{public}u]", supplierId_, reqId);
}

std::set<uint32_t> &NetSupplier::GetBestRequestList()
{
    return bestReqList_;
}

void NetSupplier::SetNetValid(NetDetectionStatus netState)
{
    NETMGR_LOG_I("Enter SetNetValid. supplier[%{public}d, %{public}s], ifValid[%{public}d]", supplierId_,
                 netSupplierIdent_.c_str(), netState);
    if (netState == VERIFICATION_STATE) {
        if (!HasNetCap(NET_CAPABILITY_VALIDATED)) {
            netCaps_.InsertNetCap(NET_CAPABILITY_VALIDATED);
            netAllCapabilities_.netCaps_.insert(NET_CAPABILITY_VALIDATED);
            NETMGR_LOG_I("NetSupplier inserted cap:NET_CAPABILITY_VALIDATED");
        }
        if (HasNetCap(NET_CAPABILITY_PORTAL)) {
            netCaps_.RemoveNetCap(NET_CAPABILITY_PORTAL);
            netAllCapabilities_.netCaps_.erase(NET_CAPABILITY_PORTAL);
            NETMGR_LOG_I("NetSupplier remove cap:NET_CAPABILITY_PORTAL");
        }
    } else if (netState == CAPTIVE_PORTAL_STATE) {
        if (!HasNetCap(NET_CAPABILITY_PORTAL)) {
            netCaps_.InsertNetCap(NET_CAPABILITY_PORTAL);
            netAllCapabilities_.netCaps_.insert(NET_CAPABILITY_PORTAL);
            NETMGR_LOG_I("NetSupplier inserted cap:NET_CAPABILITY_PORTAL");
        }
        if (HasNetCap(NET_CAPABILITY_VALIDATED)) {
            netCaps_.RemoveNetCap(NET_CAPABILITY_VALIDATED);
            netAllCapabilities_.netCaps_.erase(NET_CAPABILITY_VALIDATED);
            NETMGR_LOG_I("NetSupplier remove cap:NET_CAPABILITY_VALIDATED");
        }
    } else if (netState == QUALITY_POOR_STATE) {
        netQuality_ = QUALITY_POOR_STATE;
    } else if (netState == QUALITY_GOOD_STATE) {
        netQuality_ = QUALITY_GOOD_STATE;
    } else {
        if (HasNetCap(NET_CAPABILITY_VALIDATED)) {
            netCaps_.RemoveNetCap(NET_CAPABILITY_VALIDATED);
            netAllCapabilities_.netCaps_.erase(NET_CAPABILITY_VALIDATED);
            NETMGR_LOG_I("NetSupplier remove cap:NET_CAPABILITY_VALIDATED");
        }
        if (HasNetCap(NET_CAPABILITY_PORTAL)) {
            netCaps_.RemoveNetCap(NET_CAPABILITY_PORTAL);
            netAllCapabilities_.netCaps_.erase(NET_CAPABILITY_PORTAL);
            NETMGR_LOG_I("NetSupplier remove cap:NET_CAPABILITY_PORTAL");
        }
    }
}

bool NetSupplier::IsNetValidated() const
{
    return HasNetCap(NET_CAPABILITY_VALIDATED);
}

/**
 * This method returns the score of the current network supplier.
 *
 * It is used to prioritize network suppliers so that higher priority producers can activate when lower
 * priority networks are available.
 *
 * @return the score of the current network supplier.
 */
int32_t NetSupplier::GetNetScore() const
{
    return netScore_;
}

/**
 * This method returns the real score of current network supplier.
 *
 * This method subtracts the score depending on different conditions, or returns netScore_ if the conditions are not
 * met.
 * It is used to compare the priorities of different networks.
 *
 * @return the real score of current network supplier.
 */
int32_t NetSupplier::GetRealScore()
{
    // Notice: the order is important here:
    // 1. If network detection is not complete in the first time, subtract NET_VALID_SCORE.
    if (IsInFirstTimeDetecting()) {
        return netScore_ - NET_VALID_SCORE;
    }

    // 2. If network is not validated, subtract NET_VALID_SCORE.
    if (!IsNetValidated()) {
        return netScore_ - NET_VALID_SCORE;
    }

    // 3. Deduct DIFF_SCORE_BETWEEN_GOOD_POOR for poor network quality (reported by the supplier).
    if (IsNetQualityPoor()) {
        return netScore_ - DIFF_SCORE_BETWEEN_GOOD_POOR;
    }
    return netScore_;
}

void NetSupplier::SetDefault()
{
    NETMGR_LOG_I("set default supplier[%{public}d].", supplierId_);
    if (network_) {
        network_->SetDefaultNetWork();
    }
}

void NetSupplier::ClearDefault()
{
    NETMGR_LOG_I("clear default supplier[%{public}d].", supplierId_);
    if (network_) {
        network_->ClearDefaultNetWorkNetId();
    }
}

void NetSupplier::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    NETMGR_LOG_I("supplierId[%{public}d] update global httpProxy.", supplierId_);
    if (network_) {
        network_->UpdateGlobalHttpProxy(httpProxy);
    }
}

std::string NetSupplier::TechToType(NetSlotTech techType)
{
    switch (techType) {
        case NetSlotTech::SLOT_TYPE_GSM:
            return "2G";
        case NetSlotTech::SLOT_TYPE_LTE:
        case NetSlotTech::SLOT_TYPE_LTE_CA:
            return "4G";
        default:
            return "3G";
    }
}

void NetSupplier::SetSupplierType(int32_t type)
{
    NETMGR_LOG_I("supplierId[%{public}d] update type[%{public}d].", supplierId_, type);
    type_ = TechToType(static_cast<NetSlotTech>(type));
}

std::string NetSupplier::GetSupplierType()
{
    return type_;
}

bool NetSupplier::ResumeNetworkInfo()
{
    if (network_ == nullptr) {
        NETMGR_LOG_E("network_ is nullptr!");
        return false;
    }

    return network_->ResumeNetworkInfo();
}

bool NetSupplier::IsNetQualityPoor()
{
    return netQuality_ == QUALITY_POOR_STATE;
}

void NetSupplier::SetDetectionDone()
{
    if (!isFirstTimeDetectionDone) {
        isFirstTimeDetectionDone = true;
    }
    if (HasNetCap(NET_CAPABILITY_CHECKING_CONNECTIVITY)) {
        netCaps_.RemoveNetCap(NET_CAPABILITY_CHECKING_CONNECTIVITY);
        netAllCapabilities_.netCaps_.erase(NET_CAPABILITY_CHECKING_CONNECTIVITY);
        NETMGR_LOG_I("supplier %{public}u detection done, remove NET_CAPABILITY_CHECKING_CONNECTIVITY", supplierId_);
    }
}

bool NetSupplier::IsInFirstTimeDetecting() const
{
    return !isFirstTimeDetectionDone;
}
} // namespace NetManagerStandard
} // namespace OHOS
