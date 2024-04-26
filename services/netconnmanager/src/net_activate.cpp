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

#include <atomic>
#include <functional>

#include "net_activate.h"
#include "net_caps.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<uint32_t> g_nextRequestId = MIN_REQUEST_ID;
using TimeOutCallback = std::function<void()>;

NetActivate::NetActivate(const sptr<NetSpecifier> &specifier, const sptr<INetConnCallback> &callback,
                         std::weak_ptr<INetActivateCallback> timeoutCallback, const uint32_t &timeoutMS,
                         const std::shared_ptr<AppExecFwk::EventHandler> &netActEventHandler)
    : netSpecifier_(specifier),
      netConnCallback_(callback),
      timeoutMS_(timeoutMS),
      timeoutCallback_(timeoutCallback),
      netActEventHandler_(netActEventHandler)
{
    requestId_ = g_nextRequestId++;
    if (g_nextRequestId > MAX_REQUEST_ID) {
        g_nextRequestId = MIN_REQUEST_ID;
    }
}

void NetActivate::StartTimeOutNetAvailable()
{
    activateName_ = "NetActivate" + std::to_string(requestId_);
    auto self = shared_from_this();
    if (netActEventHandler_ != nullptr && timeoutMS_ > 0) {
        netActEventHandler_->PostTask([self]() { self->TimeOutNetAvailable(); }, activateName_, timeoutMS_);
    }
}

NetActivate::~NetActivate()
{
    if (netActEventHandler_ != nullptr) {
        netActEventHandler_->RemoveTask(activateName_);
    }
}

void NetActivate::TimeOutNetAvailable()
{
    if (netServiceSupplied_) {
        return;
    }
    if (netConnCallback_) {
        netConnCallback_->NetUnavailable();
    }

    auto timeoutCb = timeoutCallback_.lock();
    if (timeoutCb) {
        timeoutCb->OnNetActivateTimeOut(requestId_);
    }
}

bool NetActivate::MatchRequestAndNetwork(sptr<NetSupplier> supplier)
{
    NETMGR_LOG_D("supplier[%{public}d, %{public}s], request[%{public}d]",
                 (supplier ? supplier->GetSupplierId() : 0),
                 (supplier ? supplier->GetNetSupplierIdent().c_str() : "nullptr"), requestId_);
    if (supplier == nullptr) {
        NETMGR_LOG_E("Supplier is null");
        return false;
    }
    if (!CompareByNetworkIdent(supplier->GetNetSupplierIdent())) {
        NETMGR_LOG_W("Supplier[%{public}d], request[%{public}d], Supplier ident is not matched",
                     supplier->GetSupplierId(), requestId_);
        return false;
    }
    if (!CompareByNetworkCapabilities(supplier->GetNetCaps())) {
        NETMGR_LOG_W("Supplier[%{public}d], request[%{public}d], capability is not matched", supplier->GetSupplierId(),
                     requestId_);
        return false;
    }
    if (!CompareByNetworkNetType((supplier->GetNetSupplierType()))) {
        NETMGR_LOG_W("Supplier[%{public}d], request[%{public}d], Supplier net type not matched",
                     supplier->GetSupplierId(), requestId_);
        return false;
    }
    NetAllCapabilities netAllCaps = supplier->GetNetCapabilities();
    if (!CompareByNetworkBand(netAllCaps.linkUpBandwidthKbps_, netAllCaps.linkDownBandwidthKbps_)) {
        NETMGR_LOG_W("Supplier[%{public}d], request[%{public}d], supplier net band not matched",
                     supplier->GetSupplierId(), requestId_);
        return false;
    }

    return true;
}

bool NetActivate::CompareByNetworkIdent(const std::string &ident)
{
    if (ident.empty() || netSpecifier_->ident_.empty()) {
        return true;
    }
    if (ident == netSpecifier_->ident_) {
        return true;
    }
    return false;
}

bool NetActivate::CompareByNetworkCapabilities(const NetCaps &netCaps)
{
    if (netSpecifier_ == nullptr) {
        return false;
    }
    std::set<NetCap> &reqCaps = netSpecifier_->netCapabilities_.netCaps_;
    if (reqCaps.empty()) {
        NETMGR_LOG_D("Use default Supplier for empty cap");
        return netCaps.HasNetCap(NET_CAPABILITY_INTERNET);
    }
    return netCaps.HasNetCaps(reqCaps);
}

bool NetActivate::CompareByNetworkNetType(NetBearType bearerType)
{
    if (netSpecifier_ == nullptr) {
        return false;
    }
    std::set<NetBearType> &reqTypes = netSpecifier_->netCapabilities_.bearerTypes_;
    if (reqTypes.empty()) {
        return true;
    }
    if (reqTypes.find(bearerType) == reqTypes.end()) {
        return false;
    }
    return true;
}

bool NetActivate::CompareByNetworkBand(uint32_t netLinkUpBand, uint32_t netLinkDownBand)
{
    uint32_t reqLinkUpBand = netSpecifier_->netCapabilities_.linkUpBandwidthKbps_;
    uint32_t reqLinkDownBand = netSpecifier_->netCapabilities_.linkDownBandwidthKbps_;
    if ((netLinkUpBand >= reqLinkUpBand) && (netLinkDownBand >= reqLinkDownBand)) {
        return true;
    }
    return false;
}

sptr<NetSpecifier> NetActivate::GetNetSpecifier()
{
    return netSpecifier_;
}

uint32_t NetActivate::GetRequestId() const
{
    return requestId_;
}

void NetActivate::SetRequestId(uint32_t reqId)
{
    requestId_ = reqId;
}

sptr<NetSupplier> NetActivate::GetServiceSupply() const
{
    return netServiceSupplied_;
}

void NetActivate::SetServiceSupply(sptr<NetSupplier> netServiceSupplied)
{
    netServiceSupplied_ = netServiceSupplied;
}

sptr<INetConnCallback> NetActivate::GetNetCallback()
{
    return netConnCallback_;
}

bool NetActivate::HaveCapability(NetCap netCap) const
{
    if (netSpecifier_ == nullptr) {
        return false;
    }
    auto &capsRef = netSpecifier_->netCapabilities_.netCaps_;
    if (capsRef.find(netCap) == capsRef.end()) {
        return false;
    }
    return true;
}

bool NetActivate::HaveTypes(const std::set<NetBearType> &bearerTypes) const
{
    if (netSpecifier_ == nullptr) {
        return false;
    }
    auto &typesRef = netSpecifier_->netCapabilities_.bearerTypes_;
    bool result = bearerTypes.size() > 0;
    for (auto type : bearerTypes) {
        if (typesRef.find(type) == typesRef.end()) {
            result = false;
            break;
        }
    }
    return result;
}
} // namespace NetManagerStandard
} // namespace OHOS
