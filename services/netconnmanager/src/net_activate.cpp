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
#include "net_conn_service.h"
#include "net_mgr_log_wrapper.h"
#include "app_state_aware.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<uint32_t> g_nextRequestId = MIN_REQUEST_ID;
static std::string IDENT_WIFI = "wifi";
using TimeOutCallback = std::function<void()>;

NetActivate::NetActivate(const sptr<NetSpecifier> &specifier, const sptr<INetConnCallback> &callback,
                         std::weak_ptr<INetActivateCallback> timeoutCallback, const uint32_t &timeoutMS,
                         const std::shared_ptr<AppExecFwk::EventHandler> &netActEventHandler,
                         uint32_t uid, const int32_t registerType)
    : netSpecifier_(specifier),
      netConnCallback_(callback),
      timeoutMS_(timeoutMS),
      timeoutCallback_(timeoutCallback),
      netActEventHandler_(netActEventHandler),
      uid_(uid),
      registerType_(registerType)
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

bool NetActivate::MatchRequestAndNetwork(sptr<NetSupplier> supplier, bool skipCheckIdent)
{
    NETMGR_LOG_D("supplier[%{public}d, %{public}s], request[%{public}d]",
                 (supplier ? supplier->GetSupplierId() : 0),
                 (supplier ? supplier->GetNetSupplierIdent().c_str() : "nullptr"), requestId_);
    if (supplier == nullptr) {
        NETMGR_LOG_E("Supplier is null");
        return false;
    }
    if (!CompareByNetworkCapabilities(supplier->GetNetCaps())) {
        NETMGR_LOG_D("Supplier[%{public}d], request[%{public}d], capability is not matched", supplier->GetSupplierId(),
                     requestId_);
        return false;
    }
    if (!CompareByNetworkNetType((supplier->GetNetSupplierType()))) {
        NETMGR_LOG_D("Supplier[%{public}d], request[%{public}d], Supplier net type not matched",
                     supplier->GetSupplierId(), requestId_);
        return false;
    }
    if (!CompareByNetworkIdent(supplier->GetNetSupplierIdent(), supplier->GetNetSupplierType(),
        skipCheckIdent)) {
        NETMGR_LOG_W("Supplier[%{public}d], request[%{public}d], Supplier ident is not matched",
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

bool NetActivate::CompareByNetworkIdent(const std::string &ident, NetBearType bearerType, bool skipCheckIdent)
{
    if (ident.empty() || netSpecifier_->ident_.empty()) {
        return true;
    }
    if (IDENT_WIFI == netSpecifier_->ident_) {
        return true;
    }
    if (ident == netSpecifier_->ident_) {
        return true;
    }
    if (skipCheckIdent && BEARER_WIFI == bearerType) {
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

std::set<NetBearType> NetActivate::GetBearType() const
{
    return netSpecifier_->netCapabilities_.bearerTypes_;
}

int32_t NetActivate::GetRegisterType() const
{
    return registerType_;
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

uint32_t NetActivate::GetUid() const
{
    return uid_;
}

bool NetActivate::IsAppFrozened() const
{
    bool isAppFrozened = isAppFrozened_.load();
    return isAppFrozened;
}

void NetActivate::SetIsAppFrozened(bool isFrozened)
{
    isAppFrozened_ = isFrozened;
}

CallbackType NetActivate::GetLastCallbackType() const
{
    int32_t lastCallbackType = lastCallbackType_;
    return static_cast<CallbackType>(lastCallbackType);
}

void NetActivate::SetLastCallbackType(CallbackType callbackType)
{
    if ((callbackType == CALL_TYPE_UPDATE_CAP || callbackType == CALL_TYPE_UPDATE_LINK)
        && (lastCallbackType_.load() == CALL_TYPE_AVAILABLE)) {
        return;
    }
    lastCallbackType_ = callbackType;
}


int32_t NetActivate::GetLastNetid()
{
    return lastNetId_;
}

void NetActivate::SetLastNetid(const int32_t netid)
{
    lastNetId_ = netid;
}

bool NetActivate::IsAllowCallback(CallbackType callbackType)
{
    bool isAppFrozened = isAppFrozened_.load();
    bool isForegroundApp = AppStateAwareManager::GetInstance().IsForegroundApp(uid_);
    if (NetConnService::GetInstance()->IsAppFrozenedCallbackLimitation() && isAppFrozened && !isForegroundApp) {
        if (lastCallbackType_ != CALL_TYPE_LOST && callbackType == CALL_TYPE_LOST
            && lastNetId_ == 0 && netServiceSupplied_ != nullptr
            && netServiceSupplied_->GetNetHandle() != nullptr) {
                lastNetId_ = netServiceSupplied_->GetNetHandle()->GetNetId();
        }
        SetLastCallbackType(callbackType);
        NETMGR_LOG_I("UID[%{public}d] is AppFrozened, not Allow send callbackType[%{public}d]",
            uid_, callbackType);
        return false;
    }
    std::unique_lock<std::recursive_mutex> lock(notifyLostMutex_);
    if (isNotifyLostDelay_ && callbackType == CALL_TYPE_LOST) {
        NETMGR_LOG_I("UID[%{public}d] is delay, not Allow send callbackType[%{public}d]",
            uid_, callbackType);
        return false;
    }
    isNotifyLostDelay_ = false;
    notifyLostNetId_ = 0;
    return true;
}

void NetActivate::SetNotifyLostDelay(bool isNotifyLostDelay)
{
    std::unique_lock<std::recursive_mutex> lock(notifyLostMutex_);
    isNotifyLostDelay_ = isNotifyLostDelay;
}

void NetActivate::SetFrozenedNotifyLostDelay(bool isNotifyLostDelay)
{
    isFrozenedNotifyLostDelay_ = isNotifyLostDelay;
}

void NetActivate::SetNotifyLostNetId(int32_t notifyLostNetId)
{
    std::unique_lock<std::recursive_mutex> lock(notifyLostMutex_);
    notifyLostNetId_ = notifyLostNetId;
}

int32_t NetActivate::GetNotifyLostNetId()
{
    std::unique_lock<std::recursive_mutex> lock(notifyLostMutex_);
    return notifyLostNetId_;
}

bool NetActivate::GetNotifyLostDelay()
{
    std::unique_lock<std::recursive_mutex> lock(notifyLostMutex_);
    return isNotifyLostDelay_;
}

bool NetActivate::GetFrozenedNotifyLostDelay()
{
    return isFrozenedNotifyLostDelay_;
}
} // namespace NetManagerStandard
} // namespace OHOS
