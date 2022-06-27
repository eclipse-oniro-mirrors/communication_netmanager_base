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
#include "common_event_support.h"
#include "net_mgr_log_wrapper.h"
#include "broadcast_manager.h"
#include "net_supplier.h"

namespace OHOS {
namespace NetManagerStandard {
static uint32_t g_nextNetSupplierId = 0x03EB;

static constexpr int32_t SCORE_ETHERNET = 70;
static constexpr int32_t SCORE_WIFI = 60;
static constexpr int32_t SCORE_CELLULAR = 50;
static constexpr int32_t SCORE_VALIDATED = 30;
static constexpr int32_t SCORE_MAX = 100;
static constexpr int32_t SCORE_MIN = 0;

NetSupplier::NetSupplier(NetBearType bearerType, const std::string &ident, const std::set<NetCap> &caps,
                         NetConnAsync &async)
    : id_(g_nextNetSupplierId++), bearerType_(bearerType), ident_(ident), caps_(caps), async_(async),
      allCaps_(new NetAllCapabilities), supplierInfo_(new NetSupplierInfo), linkInfo_(new NetLinkInfo),
      network_(new Network), netHandle_(new NetHandle(network_->GetId())),
      netMonitor_(new NetMonitor(network_->GetId(), *network_, async_))
{
    allCaps_->netCaps_ = caps;
    allCaps_->bearerTypes_.insert(bearerType_);
}

NetSupplier::~NetSupplier() {}

uint32_t NetSupplier::GetId() const
{
    return id_;
}

uint32_t NetSupplier::GetNetId() const
{
    return network_->GetId();
}

NetBearType NetSupplier::GetBearerType() const
{
    return bearerType_;
}

std::string NetSupplier::GetIdent() const
{
    return ident_;
}

std::set<NetCap> NetSupplier::GetCaps() const
{
    return caps_.ToSet();
}

sptr<Network> NetSupplier::GetNetwork() const
{
    return network_;
}

sptr<NetMonitor> NetSupplier::GetNetMonitor() const
{
    return netMonitor_;
}

sptr<NetHandle> NetSupplier::GetNetHandle() const
{
    return netHandle_;
}

sptr<NetSupplierInfo> NetSupplier::GetSupplierInfo() const
{
    return supplierInfo_;
}

sptr<NetLinkInfo> NetSupplier::GetNetLinkInfo() const
{
    return linkInfo_;
}

sptr<NetAllCapabilities> NetSupplier::GetNetAllCapabilities() const
{
    return allCaps_;
}

int32_t NetSupplier::GetCurrentScore() const
{
    int32_t score;
    if (supplierInfo_->score_ > 0) {
        score = supplierInfo_->score_;
    } else {
        switch (bearerType_) {
            case BEARER_CELLULAR:
                score = SCORE_CELLULAR;
                break;
            case BEARER_WIFI:
                score = SCORE_WIFI;
                break;
            case BEARER_ETHERNET:
                score = SCORE_ETHERNET;
                break;
            default:
                score = 0;
                break;
        }
    }

    score += HasNetCap(NET_CAPABILITY_VALIDATED) ? SCORE_VALIDATED : -SCORE_VALIDATED;
    if (score > SCORE_MAX) {
        score = SCORE_MAX;
    } else if (score < SCORE_MIN) {
        score = SCORE_MIN;
    }

    return score;
}

bool NetSupplier::IsAvailable() const
{
    return supplierInfo_->isAvailable_;
}

bool NetSupplier::IsRequested() const
{
    for (auto req : netReqs_) {
        if (req->GetNetSpecifier()->isRequested_) {
            return true;
        }
    }
    return false;
}

bool NetSupplier::HasNetCaps(const std::set<NetCap> &caps) const
{
    for (auto cap : caps) {
        if (!HasNetCap(cap)) {
            return false;
        }
    }
    return true;
}

bool NetSupplier::HasNetCap(NetCap cap) const
{
    return caps_.HasNetCap(cap);
}

void NetSupplier::InsertNetCap(NetCap cap)
{
    if (!HasNetCap(cap)) {
        caps_.InsertNetCap(cap);
        allCaps_->netCaps_.insert(cap);
        NETMGR_LOG_I("NetSupplier[%{public}s] inserted new cap:%{public}d", ident_.c_str(), cap);
        async_.CallbackOnNetCapabilitiesChanged(id_, *GetNetAllCapabilities());
        NotifyNetRequestCallbacks(INetConnCallback::NET_CAPABILITIES_CHANGE);
    }
}

void NetSupplier::RemoveNetCap(NetCap cap)
{
    if (HasNetCap(cap)) {
        caps_.RemoveNetCap(cap);
        allCaps_->netCaps_.erase(cap);
        NETMGR_LOG_I("NetSupplier[%{public}s] remove new cap:%{public}d", ident_.c_str(), cap);
        async_.CallbackOnNetCapabilitiesChanged(id_, *GetNetAllCapabilities());
        NotifyNetRequestCallbacks(INetConnCallback::NET_CAPABILITIES_CHANGE);
    }
}

void NetSupplier::UpdateNetSupplierInfo(sptr<NetSupplierInfo> supplierInfo)
{
    if (supplierInfo) {
        if (supplierInfo_->isAvailable_ != supplierInfo->isAvailable_) {
            NETMGR_LOG_I("NetSupplier[%{public}s] available changed:%{public}s", ident_.c_str(),
                         supplierInfo->isAvailable_ ? "true" : "false");
            if (supplierInfo->isAvailable_) {
                network_->CreatePhy();
                netMonitor_->Start();
                NotifyNetRequestCallbacks(INetConnCallback::NET_AVAILABLE);
            } else {
                network_->DestroyPhy();
                netMonitor_->Stop();
                NotifyNetRequestCallbacks(INetConnCallback::NET_LOST);
                SetNetConnState(NET_CONN_STATE_DISCONNECTED);
            }
            async_.CallbackOnNetAvailableChanged(id_, supplierInfo->isAvailable_);
        } else if (supplierInfo_->score_ != supplierInfo->score_) {
            NETMGR_LOG_I("NetSupplier[%{public}s] score changed:%{public}d", ident_.c_str(), supplierInfo->score_);
            async_.CallbackOnNetScoreChanged(id_, supplierInfo->score_);
        }

        allCaps_->linkUpBandwidthKbps_ = supplierInfo->linkUpBandwidthKbps_;
        allCaps_->linkDownBandwidthKbps_ = supplierInfo->linkDownBandwidthKbps_;
        *supplierInfo_ = *supplierInfo;
    }
}

void NetSupplier::UpdateNetLinkInfo(sptr<NetLinkInfo> netLinkInfo)
{
    if (netLinkInfo) {
        network_->SetIfaceName(netLinkInfo->ifaceName_);
        network_->SetDomain(netLinkInfo->domain_);
        network_->SetNetAddrList(netLinkInfo->netAddrList_);
        network_->SetDnsList(netLinkInfo->dnsList_);
        network_->SetRouteList(netLinkInfo->routeList_);
        network_->SetMtu(netLinkInfo->mtu_);
        network_->SetTcpBufferSizes(netLinkInfo->tcpBufferSizes_);
    }

    *linkInfo_ = *netLinkInfo;
    if (netConnState_ == NET_CONN_STATE_CONNECTING) {
        SetNetConnState(NET_CONN_STATE_CONNECTED);
    }
    NotifyNetRequestCallbacks(INetConnCallback::NET_CONNECTION_PROPERTIES_CHANGE);

    async_.CallbackOnNetLinkInfoChanged(id_, *linkInfo_);
}

void NetSupplier::SetSupplierCallback(sptr<INetSupplierCallback> supplierCb)
{
    if (netSupplierCb_ != supplierCb) {
        netSupplierCb_ = supplierCb;
        if (netSupplierCb_) {
            NETMGR_LOG_I("NetSupplier[%{public}s] callback has been set", ident_.c_str());
        }
        if (IsRequested() && netSupplierCb_) {
            RequestNetwork();
        }
    }
}

void NetSupplier::RegisterNetDetectionCallback(sptr<INetDetectionCallback> callback)
{
    netDetectionCbs_.push_back(callback);
}

void NetSupplier::UnregisterNetDetectionCallback(sptr<INetDetectionCallback> callback)
{
    netDetectionCbs_.remove_if(
        [&](sptr<INetDetectionCallback> cb) { return cb->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr(); });
}

bool NetSupplier::SatisfiyNetRequest(sptr<NetRequest> netRequest)
{
    bool identMatched = false;
    bool capsMatched = false;
    bool bearerTypesMatched = false;
    bool linkBandMatched = false;

    auto specifier = netRequest->GetNetSpecifier();
    if (!specifier) {
        return true;
    }

    if (specifier->ident_.empty() || ident_.empty() || specifier->ident_ == ident_) {
        identMatched = true;
    }

    const auto &reqCaps = specifier->netCapabilities_.netCaps_;
    if (reqCaps.empty() || HasNetCaps(reqCaps)) {
        capsMatched = true;
    }

    const auto &reqBearerTypes = specifier->netCapabilities_.bearerTypes_;
    if (reqBearerTypes.empty() || reqBearerTypes.find(bearerType_) != reqBearerTypes.end()) {
        bearerTypesMatched = true;
    }

    uint32_t reqLinkUpBand = specifier->netCapabilities_.linkUpBandwidthKbps_;
    uint32_t reqLinkDownBand = specifier->netCapabilities_.linkDownBandwidthKbps_;
    if ((supplierInfo_->linkUpBandwidthKbps_ >= reqLinkUpBand) &&
        (supplierInfo_->linkDownBandwidthKbps_ >= reqLinkDownBand)) {
        linkBandMatched = true;
    }

    return identMatched && capsMatched && bearerTypesMatched && linkBandMatched;
}

void NetSupplier::AddNetRequest(sptr<NetRequest> netRequest)
{
    NETMGR_LOG_I("NetSupplier[%{public}s] add request[%{public}d]", ident_.c_str(), netRequest->GetId());
    if (netReqs_.find(netRequest) == netReqs_.end()) {
        netReqs_.insert(netRequest);
        netRequest->SetNetSupplierId(id_);
        netRequest->CallbackForNetAvailable(netHandle_);
        netRequest->CallbackForNetCapabilitiesChanged(netHandle_, GetNetAllCapabilities());
        netRequest->CallbackForNetConnectionPropertiesChanged(netHandle_, linkInfo_);
        if (!IsRequested() && netRequest->GetNetSpecifier()->isRequested_) {
            RequestNetwork();
        }
    }
}

void NetSupplier::RemoveNetRequest(sptr<NetRequest> netRequest)
{
    NETMGR_LOG_I("NetSupplier[%{public}s] remove request[%{public}d]", ident_.c_str(), netRequest->GetId());
    bool isRequested = IsRequested();
    if (netReqs_.erase(netRequest) > 0 && isRequested != IsRequested()) {
        NETMGR_LOG_I("All request removed, network will be released");
        ReleaseNetwork();
        netRequest->SetNetSupplierId(0);
    }
}

void NetSupplier::RemoveAllNetRequests()
{
    for (auto req : netReqs_) {
        req->SetNetSupplierId(0);
    }
    netReqs_.clear();
    ReleaseNetwork();
}

void NetSupplier::NotifyNetDetectionResult(NetDetectionResultCode detectionResult, const std::string &urlRedirect)
{
    NETMGR_LOG_I("NetSupplier[%{public}s] notify detection result:[%{public}d, %{public}s]", ident_.c_str(),
                 detectionResult, urlRedirect.c_str());
    for (auto cb : netDetectionCbs_) {
        cb->OnNetDetectionResultChanged(detectionResult, urlRedirect);
    }
}

void NetSupplier::RequestNetwork()
{
    if ((netConnState_ == NET_CONN_STATE_CONNECTING) || (netConnState_ == NET_CONN_STATE_CONNECTED)) {
        return;
    }

    if (netSupplierCb_ == nullptr) {
        return;
    }

    SetNetConnState(NET_CONN_STATE_CONNECTING);

    NETMGR_LOG_I("NetSupplier[%{public}s] start request network", ident_.c_str());
    auto now = std::chrono::system_clock::now();
    reqRelAsync_.wait();
    reqRelAsync_ = std::async(std::launch::async, [&, now]() {
        int32_t err = netSupplierCb_->RequestNetwork(ident_, caps_.ToSet());
        NETMGR_LOG_I(
            "NetSupplier[%{public}s] request network finished, cost %{public}lld ms", ident_.c_str(),
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count());
        if (err) {
            NETMGR_LOG_W("NetSupplier[%{public}s] request network failed", ident_.c_str());
            async_.GetScheduler().Post([&]() { SetNetConnState(NET_CONN_STATE_IDLE); });
        }
    });
}

void NetSupplier::ReleaseNetwork()
{
    if ((netConnState_ != NET_CONN_STATE_CONNECTING) && (netConnState_ != NET_CONN_STATE_CONNECTED)) {
        return;
    }

    if (netSupplierCb_ == nullptr) {
        return;
    }

    SetNetConnState(NET_CONN_STATE_DISCONNECTING);
    NETMGR_LOG_I("NetSupplier[%{public}s] start release network", ident_.c_str());
    auto now = std::chrono::system_clock::now();
    reqRelAsync_.wait();
    reqRelAsync_ = std::async(std::launch::async, [&, now]() {
        netSupplierCb_->ReleaseNetwork(ident_, caps_.ToSet());
        NETMGR_LOG_I(
            "NetSupplier[%{public}s] release network finished, cost %{public}lld ms", ident_.c_str(),
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count());
    });
}

void NetSupplier::SetNetConnState(NetConnState netConnState)
{
    if (netConnState_ != netConnState) {
        netConnState_ = netConnState;

        NETMGR_LOG_I("NetSupplier[%{public}s] connect state changed to %{public}d", ident_.c_str(), netConnState_);

        BroadcastInfo info;
        info.action = EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE;
        info.data = "Net Manager Connection State Changed";
        info.code = static_cast<int32_t>(netConnState);
        info.ordered = true;
        std::map<std::string, int32_t> param = {{"NetType", static_cast<int32_t>(bearerType_)}};
        DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);
    }
}

void NetSupplier::NotifyNetRequestCallbacks(int32_t cmd)
{
    for (auto req : netReqs_) {
        switch (cmd) {
            case INetConnCallback::NET_AVAILABLE:
                NETMGR_LOG_I("NetSupplier[%{public}s] notify to requests: NET_AVAILABLE", ident_.c_str());
                req->CallbackForNetAvailable(netHandle_);
                break;
            case INetConnCallback::NET_CONNECTION_PROPERTIES_CHANGE:
                NETMGR_LOG_I("NetSupplier[%{public}s] notify to requests: NET_CONNECTION_PROPERTIES_CHANGE",
                             ident_.c_str());
                req->CallbackForNetConnectionPropertiesChanged(netHandle_, linkInfo_);
                break;
            case INetConnCallback::NET_LOST:
                NETMGR_LOG_I("NetSupplier[%{public}s] notify to requests: NET_LOST", ident_.c_str());
                req->CallbackForNetLost(netHandle_);
                break;
            case INetConnCallback::NET_CAPABILITIES_CHANGE:
                NETMGR_LOG_I("NetSupplier[%{public}s] notify to requests: NET_CAPABILITIES_CHANGE", ident_.c_str());
                req->CallbackForNetCapabilitiesChanged(netHandle_, allCaps_);
                break;
            default:
                return;
        }
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
