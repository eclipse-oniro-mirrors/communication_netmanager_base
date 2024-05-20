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
#include <fstream>
#include <functional>
#include <memory>
#include <sys/time.h>
#include <utility>
#include <regex>

#include "common_event_support.h"
#include "network.h"
#include "system_ability_definition.h"

#include "broadcast_manager.h"
#include "event_report.h"
#include "net_activate.h"
#include "net_conn_service.h"
#include "net_conn_types.h"
#include "net_datashare_utils.h"
#include "net_http_proxy_tracker.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_supplier.h"
#include "netmanager_base_permission.h"
#include "netsys_controller.h"
#include "ipc_skeleton.h"
#include "parameter.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_ALLOW_UID_NUM = 2000;
constexpr uint32_t INVALID_SUPPLIER_ID = 0;
// hisysevent error messgae
constexpr const char *ERROR_MSG_NULL_SUPPLIER_INFO = "Net supplier info is nullptr";
constexpr const char *ERROR_MSG_NULL_NET_LINK_INFO = "Net link info is nullptr";
constexpr const char *ERROR_MSG_NULL_NET_SPECIFIER = "The parameter of netSpecifier or callback is null";
constexpr const char *ERROR_MSG_CAN_NOT_FIND_SUPPLIER = "Can not find supplier by id:";
constexpr const char *ERROR_MSG_UPDATE_NETLINK_INFO_FAILED = "Update net link info failed";
constexpr const char *NET_CONN_MANAGER_WORK_THREAD = "NET_CONN_MANAGER_WORK_THREAD";
constexpr const char *NET_ACTIVATE_WORK_THREAD = "NET_ACTIVATE_WORK_THREAD";
constexpr const char *NET_HTTP_PROBE_URL = "http://connectivitycheck.platform.hicloud.com/generate_204";
const uint32_t SYS_PARAMETER_SIZE = 256;
constexpr const char *CFG_NETWORK_PRE_AIRPLANE_MODE_WAIT_TIMES = "persist.network.pre_airplane_mode_wait_times";
constexpr const char *NO_DELAY_TIME_CONFIG = "100";
constexpr uint32_t INPUT_VALUE_LENGTH = 10;
constexpr uint32_t MAX_DELAY_TIME = 200;
constexpr uint16_t DEFAULT_MTU = 1500;
} // namespace

const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(NetConnService::GetInstance().get());

NetConnService::NetConnService()
    : SystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    netActEventRunner_ = AppExecFwk::EventRunner::Create(NET_ACTIVATE_WORK_THREAD);
    netActEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(netActEventRunner_);
    CreateDefaultRequest();
}

NetConnService::~NetConnService()
{
    RemoveALLClientDeathRecipient();
}

void NetConnService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    NETMGR_LOG_D("OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("init failed");
        return;
    }
    state_ = STATE_RUNNING;
    gettimeofday(&tv, nullptr);
    NETMGR_LOG_D("OnStart end");
}

void NetConnService::CreateDefaultRequest()
{
    if (!defaultNetActivate_) {
        defaultNetSpecifier_ = (std::make_unique<NetSpecifier>()).release();
        defaultNetSpecifier_->SetCapability(NET_CAPABILITY_INTERNET);
        std::weak_ptr<INetActivateCallback> timeoutCb;
        defaultNetActivate_ =
            std::make_shared<NetActivate>(defaultNetSpecifier_, nullptr, timeoutCb, 0, netActEventHandler_);
        defaultNetActivate_->StartTimeOutNetAvailable();
        defaultNetActivate_->SetRequestId(DEFAULT_REQUEST_ID);
        netActivates_[DEFAULT_REQUEST_ID] = defaultNetActivate_;
    }
}

void NetConnService::OnStop()
{
    NETMGR_LOG_D("OnStop begin");
    if (netConnEventRunner_) {
        netConnEventRunner_->Stop();
        netConnEventRunner_.reset();
    }
    if (netConnEventHandler_) {
        netConnEventHandler_.reset();
    }
    state_ = STATE_STOPPED;
    registerToService_ = false;
    NETMGR_LOG_D("OnStop end");
}

bool NetConnService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOG_E("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }

    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);

    netConnEventRunner_ = AppExecFwk::EventRunner::Create(NET_CONN_MANAGER_WORK_THREAD);
    if (netConnEventRunner_ == nullptr) {
        NETMGR_LOG_E("Create event runner failed.");
        return false;
    }
    netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnEventRunner_);

    if (!registerToService_) {
        if (!Publish(NetConnService::GetInstance().get())) {
            NETMGR_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    serviceIface_ = std::make_unique<NetConnServiceIface>().release();
    NetManagerCenter::GetInstance().RegisterConnService(serviceIface_);

    interfaceStateCallback_ = new (std::nothrow) NetInterfaceStateCallback();
    if (interfaceStateCallback_) {
        NetsysController::GetInstance().RegisterCallback(interfaceStateCallback_);
    }
    dnsResultCallback_ = std::make_unique<NetDnsResultCallback>().release();
    int32_t regDnsResult = NetsysController::GetInstance().RegisterDnsResultCallback(dnsResultCallback_, 0);
    NETMGR_LOG_I("Register Dns Result callback result: [%{public}d]", regDnsResult);

    netFactoryResetCallback_ = std::make_unique<NetFactoryResetCallback>().release();
    if (netFactoryResetCallback_ == nullptr) {
        NETMGR_LOG_E("netFactoryResetCallback_ is nullptr");
    }

    RecoverInfo();
    NETMGR_LOG_I("Init end");
    return true;
}

void NetConnService::RecoverInfo()
{
    // recover httpproxy
    LoadGlobalHttpProxy();
    if (!globalHttpProxy_.GetHost().empty()) {
        NETMGR_LOG_D("globalHttpProxy_ not empty, send broadcast");
        SendHttpProxyChangeBroadcast(globalHttpProxy_);
        UpdateGlobalHttpProxy(globalHttpProxy_);
    }
}

int32_t NetConnService::SystemReady()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("System ready.");
        return NETMANAGER_SUCCESS;
    } else {
        return NETMANAGER_ERROR;
    }
}

// Do not post into event handler, because this interface should have good performance
int32_t NetConnService::SetInternetPermission(uint32_t uid, uint8_t allow)
{
    return NetsysController::GetInstance().SetInternetPermission(uid, allow);
}

int32_t NetConnService::RegisterNetSupplier(NetBearType bearerType, const std::string &ident,
                                            const std::set<NetCap> &netCaps, uint32_t &supplierId)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, bearerType, &ident, &netCaps, &supplierId, &result]() {
            result = this->RegisterNetSupplierAsync(bearerType, ident, netCaps, supplierId);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, &callback, &result]() {
            result = this->RegisterNetSupplierCallbackAsync(supplierId, callback);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<INetConnCallback> callback)
{
    NETMGR_LOG_D("RegisterNetConnCallback service in.");
    return RegisterNetConnCallback(defaultNetSpecifier_, callback, 0);
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
                                                const sptr<INetConnCallback> callback, const uint32_t &timeoutMS)
{
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());

    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &netSpecifier, &callback, timeoutMS, callingUid, &result]() {
            result = this->RegisterNetConnCallbackAsync(netSpecifier, callback, timeoutMS, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::RequestNetConnection(const sptr<NetSpecifier> netSpecifier,
                                             const sptr<INetConnCallback> callback, const uint32_t timeoutMS)
{
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());

    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, netSpecifier, callback, timeoutMS, callingUid, &result]() {
            result = this->RequestNetConnectionAsync(netSpecifier, callback, timeoutMS, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter RegisterNetDetectionCallback");
    return RegUnRegNetDetectionCallback(netId, callback, true);
}

int32_t NetConnService::UnregisterNetSupplier(uint32_t supplierId)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask(
            [this, supplierId, &result]() { result = this->UnregisterNetSupplierAsync(supplierId); });
    }
    return result;
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask(
            [this, &callback, callingUid, &result]() {
                result = this->UnregisterNetConnCallbackAsync(callback, callingUid);
            });
    }
    return result;
}

int32_t NetConnService::UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter UnRegisterNetDetectionCallback");
    return RegUnRegNetDetectionCallback(netId, callback, false);
}

int32_t NetConnService::RegUnRegNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback,
                                                     bool isReg)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, netId, &callback, isReg, &result]() {
            result = this->RegUnRegNetDetectionCallbackAsync(netId, callback, isReg);
        });
    }
    return result;
}

int32_t NetConnService::UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &netSpecifier, netState, &result]() {
            result = this->UpdateNetStateForTestAsync(netSpecifier, netState);
        });
    }
    return result;
}

int32_t NetConnService::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, &netSupplierInfo, &result]() {
            result = this->UpdateNetSupplierInfoAsync(supplierId, netSupplierInfo);
        });
    }
    return result;
}

int32_t NetConnService::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, &netLinkInfo, &result]() {
            result = this->UpdateNetLinkInfoAsync(supplierId, netLinkInfo);
        });
    }
    return result;
}

int32_t NetConnService::NetDetection(int32_t netId)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    NETMGR_LOG_I("NetDetection, call uid [%{public}d]", callingUid);
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, netId, &result]() { result = this->NetDetectionAsync(netId); });
    }
    return result;
}

int32_t NetConnService::RestrictBackgroundChanged(bool restrictBackground)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, restrictBackground, &result]() {
            result = this->RestrictBackgroundChangedAsync(restrictBackground);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetSupplierAsync(NetBearType bearerType, const std::string &ident,
                                                 const std::set<NetCap> &netCaps, uint32_t &supplierId)
{
    NETMGR_LOG_I("RegisterNetSupplier service in, bearerType[%{public}u], ident[%{public}s]",
                 static_cast<uint32_t>(bearerType), ident.c_str());
    // If there is no supplier in the list, create a supplier
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }
    sptr<NetSupplier> supplier = GetNetSupplierFromList(bearerType, ident, netCaps);
    if (supplier != nullptr) {
        NETMGR_LOG_E("Supplier[%{public}d %{public}s] already exists.", supplier->GetSupplierId(), ident.c_str());
        supplierId = supplier->GetSupplierId();
        return NETMANAGER_SUCCESS;
    }
    // If there is no supplier in the list, create a supplier
    supplier = (std::make_unique<NetSupplier>(bearerType, ident, netCaps)).release();
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    supplierId = supplier->GetSupplierId();
    if (!NetScore::GetServiceScore(supplier)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }
    // create network
    bool isContainInternal = netCaps.find(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT) != netCaps.end();
    int32_t netId = isContainInternal ? GenerateInternalNetId() : GenerateNetId();
    NETMGR_LOG_D("GenerateNetId is: [%{public}d], bearerType: %{public}d, supplierId: %{public}d",
        netId, bearerType, supplierId);
    if (netId == INVALID_NET_ID) {
        NETMGR_LOG_E("GenerateNetId fail");
        return NET_CONN_ERR_INVALID_NETWORK;
    }
    std::shared_ptr<Network> network = std::make_shared<Network>(
        netId, supplierId,
        std::bind(&NetConnService::HandleDetectionResult, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2),
        bearerType, netConnEventHandler_);
    network->SetNetCaps(netCaps);
    supplier->SetNetwork(network);
    supplier->SetNetValid(VERIFICATION_STATE);
    // save supplier
    std::unique_lock<std::mutex> locker(netManagerMutex_);
    netSuppliers_[supplierId] = supplier;
    networks_[netId] = network;
    locker.unlock();
    struct EventInfo eventInfo = {.netId = netId, .bearerType = bearerType, .ident = ident, .supplierId = supplierId};
    EventReport::SendSupplierBehaviorEvent(eventInfo);
    NETMGR_LOG_I("RegisterNetSupplier service out, supplier[%{public}d %{public}s] netId[%{public}d]", supplierId,
                 ident.c_str(), netId);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RegisterNetSupplierCallbackAsync(uint32_t supplierId,
                                                         const sptr<INetSupplierCallback> &callback)
{
    NETMGR_LOG_I("RegisterNetSupplierCallback service in, supplierId[%{public}d]", supplierId);
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    supplier->RegisterSupplierCallback(callback);
    SendAllRequestToNetwork(supplier);
    NETMGR_LOG_I("RegisterNetSupplierCallback service out");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RegisterNetConnCallbackAsync(const sptr<NetSpecifier> &netSpecifier,
                                                     const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS,
                                                     const uint32_t callingUid)
{
    NETMGR_LOG_I("Register net connect callback async, call uid [%{public}u]", callingUid);
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        struct EventInfo eventInfo = {.errorType = static_cast<int32_t>(FAULT_INVALID_PARAMETER),
                                      .errorMsg = ERROR_MSG_NULL_NET_SPECIFIER};
        EventReport::SendRequestFaultEvent(eventInfo);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    uint32_t reqId = 0;
    if (FindSameCallback(callback, reqId)) {
        NETMGR_LOG_E("RegisterNetConnCallback find same callback");
        return NET_CONN_ERR_SAME_CALLBACK;
    }
    int32_t ret = IncreaseNetConnCallbackCntForUid(callingUid);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    AddClientDeathRecipient(callback);
    return ActivateNetwork(netSpecifier, callback, timeoutMS);
}

int32_t NetConnService::RequestNetConnectionAsync(const sptr<NetSpecifier> &netSpecifier,
                                                  const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS,
                                                  const uint32_t callingUid)
{
    NETMGR_LOG_I("Request net connect callback async, call uid [%{public}u]", callingUid);
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        struct EventInfo eventInfo = {.errorType = static_cast<int32_t>(FAULT_INVALID_PARAMETER),
                                      .errorMsg = ERROR_MSG_NULL_NET_SPECIFIER};
        EventReport::SendRequestFaultEvent(eventInfo);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    uint32_t reqId = 0;
    if (FindSameCallback(callback, reqId)) {
        NETMGR_LOG_E("RequestNetConnection found same callback");
        return NET_CONN_ERR_SAME_CALLBACK;
    }
    int32_t ret = IncreaseNetConnCallbackCntForUid(callingUid, REQUEST);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    return ActivateNetwork(netSpecifier, callback, timeoutMS);
}

int32_t NetConnService::UnregisterNetSupplierAsync(uint32_t supplierId)
{
    NETMGR_LOG_I("UnregisterNetSupplier service in, supplierId[%{public}d]", supplierId);
    // Remove supplier from the list based on supplierId
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    NETMGR_LOG_I("Unregister supplier[%{public}d, %{public}s], defaultNetSupplier[%{public}d], %{public}s]",
                 supplier->GetSupplierId(), supplier->GetNetSupplierIdent().c_str(),
                 defaultNetSupplier_ ? defaultNetSupplier_->GetSupplierId() : 0,
                 defaultNetSupplier_ ? defaultNetSupplier_->GetNetSupplierIdent().c_str() : "null");

    struct EventInfo eventInfo = {.bearerType = supplier->GetNetSupplierType(),
                                  .ident = supplier->GetNetSupplierIdent(),
                                  .supplierId = supplier->GetSupplierId()};
    EventReport::SendSupplierBehaviorEvent(eventInfo);

    int32_t netId = supplier->GetNetId();
    NET_NETWORK_MAP::iterator iterNetwork = networks_.find(netId);
    if (iterNetwork != networks_.end()) {
        NETMGR_LOG_I("the iterNetwork already exists.");
        std::unique_lock<std::mutex> locker(netManagerMutex_);
        networks_.erase(iterNetwork);
        locker.unlock();
    }
    if (defaultNetSupplier_ == supplier) {
        NETMGR_LOG_I("Set default net supplier to nullptr.");
        sptr<NetSupplier> newSupplier = nullptr;
        MakeDefaultNetWork(defaultNetSupplier_, newSupplier);
    }
    NetSupplierInfo info;
    supplier->UpdateNetSupplierInfo(info);
    std::unique_lock<std::mutex> locker(netManagerMutex_);
    netSuppliers_.erase(supplierId);
    locker.unlock();
    FindBestNetworkForAllRequest();
    NETMGR_LOG_I("UnregisterNetSupplier supplierId[%{public}d] out", supplierId);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::UnregisterNetConnCallbackAsync(const sptr<INetConnCallback> &callback,
                                                       const uint32_t callingUid)
{
    NETMGR_LOG_I("UnregisterNetConnCallback Enter, call uid [%{public}u]", callingUid);
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    RegisterType registerType = INVALIDTYPE;
    uint32_t reqId = 0;
    if (!FindSameCallback(callback, reqId, registerType) || registerType == INVALIDTYPE) {
        NETMGR_LOG_E("UnregisterNetConnCallback can not find same callback or callback is invalid.");
        return NET_CONN_ERR_CALLBACK_NOT_FOUND;
    }
    DecreaseNetConnCallbackCntForUid(callingUid, registerType);
    
    NET_ACTIVATE_MAP::iterator iterActive;
    for (iterActive = netActivates_.begin(); iterActive != netActivates_.end();) {
        if (!iterActive->second) {
            ++iterActive;
            continue;
        }
        sptr<INetConnCallback> saveCallback = iterActive->second->GetNetCallback();
        if (saveCallback == nullptr) {
            ++iterActive;
            continue;
        }
        if (callback->AsObject().GetRefPtr() != saveCallback->AsObject().GetRefPtr()) {
            ++iterActive;
            continue;
        }
        reqId = iterActive->first;
        auto netActivate = iterActive->second;
        if (netActivate) {
            sptr<NetSupplier> supplier = netActivate->GetServiceSupply();
            if (supplier) {
                supplier->CancelRequest(reqId);
            }
        }
        NET_SUPPLIER_MAP::iterator iterSupplier;
        for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
            if (iterSupplier->second != nullptr) {
                iterSupplier->second->CancelRequest(reqId);
            }
        }
        iterActive = netActivates_.erase(iterActive);
        RemoveClientDeathRecipient(callback);
    }
    NETMGR_LOG_I("UnregisterNetConnCallback End.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::IncreaseNetConnCallbackCntForUid(const uint32_t callingUid, const RegisterType registerType)
{
    auto &netUidRequest = registerType == REGISTER ?
        netUidRequest_ : internalDefaultUidRequest_;
    auto requestNetwork = netUidRequest.find(callingUid);
    if (requestNetwork == netUidRequest.end()) {
        netUidRequest.insert(std::make_pair(callingUid, 1));
    } else {
        if (requestNetwork->second >= MAX_ALLOW_UID_NUM) {
            NETMGR_LOG_E("return falied for UID [%{public}d] has registered over [%{public}d] callback",
                         callingUid, MAX_ALLOW_UID_NUM);
            return NET_CONN_ERR_NET_OVER_MAX_REQUEST_NUM;
        } else {
            requestNetwork->second++;
        }
    }
    return NETMANAGER_SUCCESS;
}

void NetConnService::DecreaseNetConnCallbackCntForUid(const uint32_t callingUid, const RegisterType registerType)
{
    auto &netUidRequest = registerType == REGISTER ?
        netUidRequest_ : internalDefaultUidRequest_;
    auto requestNetwork = netUidRequest.find(callingUid);
    if (requestNetwork == netUidRequest.end()) {
        NETMGR_LOG_E("Could not find the request calling uid");
    } else {
        if (requestNetwork->second >= 1) {
            requestNetwork->second--;
        }
        if (requestNetwork->second == 0) {
            netUidRequest.erase(requestNetwork);
        }
    }
}

int32_t NetConnService::RegUnRegNetDetectionCallbackAsync(int32_t netId, const sptr<INetDetectionCallback> &callback,
                                                          bool isReg)
{
    NETMGR_LOG_I("Enter Async");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network.");
        return NET_CONN_ERR_NETID_NOT_FOUND;
    }
    if (isReg) {
        iterNetwork->second->RegisterNetDetectionCallback(callback);
        return NETMANAGER_SUCCESS;
    }
    return iterNetwork->second->UnRegisterNetDetectionCallback(callback);
}

int32_t NetConnService::UpdateNetStateForTestAsync(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    NETMGR_LOG_D("Test NetConnService::UpdateNetStateForTest(), begin");
    if (netSpecifier == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::UpdateNetSupplierInfoAsync(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    NETMGR_LOG_I("UpdateNetSupplierInfo service in. supplierId[%{public}d]", supplierId);
    struct EventInfo eventInfo = {.updateSupplierId = supplierId};
    if (netSupplierInfo == nullptr) {
        NETMGR_LOG_E("netSupplierInfo is nullptr");
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_SUPPLIERINFO_INV_PARAM);
        eventInfo.errorMsg = ERROR_MSG_NULL_SUPPLIER_INFO;
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    eventInfo.supplierInfo = netSupplierInfo->ToString(" ");
    EventReport::SendSupplierBehaviorEvent(eventInfo);

    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("Can not find supplier for supplierId[%{public}d]", supplierId);
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_SUPPLIERINFO_INV_PARAM);
        eventInfo.errorMsg = std::string(ERROR_MSG_CAN_NOT_FIND_SUPPLIER).append(std::to_string(supplierId));
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    NETMGR_LOG_I("Update supplier[%{public}d, %{public}s], supplierInfo:[ %{public}s ]", supplierId,
                 supplier->GetNetSupplierIdent().c_str(), netSupplierInfo->ToString(" ").c_str());

    supplier->UpdateNetSupplierInfo(*netSupplierInfo);
    if (!netSupplierInfo->isAvailable_) {
        CallbackForSupplier(supplier, CALL_TYPE_LOST);
    } else {
        CallbackForSupplier(supplier, CALL_TYPE_UPDATE_CAP);
    }
    if (!NetScore::GetServiceScore(supplier)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }
    FindBestNetworkForAllRequest();
    NETMGR_LOG_I("UpdateNetSupplierInfo service out.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::UpdateNetLinkInfoAsync(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    NETMGR_LOG_I("UpdateNetLinkInfo service in. supplierId[%{public}d]", supplierId);
    struct EventInfo eventInfo = {.updateNetlinkId = supplierId};

    if (netLinkInfo == nullptr) {
        NETMGR_LOG_E("netLinkInfo is nullptr");
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_NETLINK_INFO_INV_PARAM);
        eventInfo.errorMsg = ERROR_MSG_NULL_NET_LINK_INFO;
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    eventInfo.netlinkInfo = netLinkInfo->ToString(" ");
    EventReport::SendSupplierBehaviorEvent(eventInfo);

    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_NETLINK_INFO_INV_PARAM);
        eventInfo.errorMsg = std::string(ERROR_MSG_CAN_NOT_FIND_SUPPLIER).append(std::to_string(supplierId));
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NET_CONN_ERR_NO_SUPPLIER;
    }

    HttpProxy oldHttpProxy;
    supplier->GetHttpProxy(oldHttpProxy);
    // According to supplier id, get network from the list
    std::unique_lock<std::mutex> locker(netManagerMutex_);
    if (supplier->UpdateNetLinkInfo(*netLinkInfo) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("UpdateNetLinkInfo fail");
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_NETLINK_INFO_FAILED);
        eventInfo.errorMsg = ERROR_MSG_UPDATE_NETLINK_INFO_FAILED;
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NET_CONN_ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    locker.unlock();
    if (oldHttpProxy != netLinkInfo->httpProxy_) {
        NETMGR_LOG_I("new httpProxy is %{public}s.", netLinkInfo->httpProxy_.ToString().c_str());
        SendHttpProxyChangeBroadcast(netLinkInfo->httpProxy_);
    }

    CallbackForSupplier(supplier, CALL_TYPE_UPDATE_LINK);
    if (!NetScore::GetServiceScore(supplier)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }
    FindBestNetworkForAllRequest();
    NETMGR_LOG_I("UpdateNetLinkInfo service out.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetDetectionAsync(int32_t netId)
{
    NETMGR_LOG_I("Enter NetDetection, netId=[%{public}d]", netId);
    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network.");
        return NET_CONN_ERR_NETID_NOT_FOUND;
    }
    iterNetwork->second->StartNetDetection(true);
    NETMGR_LOG_I("End NetDetection");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetDetectionForDnsHealthSync(int32_t netId, bool dnsHealthSuccess)
{
    NETMGR_LOG_D("Enter NetDetectionForDnsHealthSync");
    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network");
        return NET_CONN_ERR_NETID_NOT_FOUND;
    }
    iterNetwork->second->NetDetectionForDnsHealth(dnsHealthSuccess);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RestrictBackgroundChangedAsync(bool restrictBackground)
{
    NETMGR_LOG_I("Restrict background changed, background = %{public}d", restrictBackground);
    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        if (it->second == nullptr) {
            continue;
        }

        if (it->second->GetRestrictBackground() == restrictBackground) {
            NETMGR_LOG_D("it->second->GetRestrictBackground() == restrictBackground");
            return NET_CONN_ERR_NET_NO_RESTRICT_BACKGROUND;
        }

        if (it->second->GetNetSupplierType() == BEARER_VPN) {
            CallbackForSupplier(it->second, CALL_TYPE_BLOCK_STATUS);
        }
        it->second->SetRestrictBackground(restrictBackground);
    }
    NETMGR_LOG_I("End RestrictBackgroundChangedAsync");
    return NETMANAGER_SUCCESS;
}

void NetConnService::SendHttpProxyChangeBroadcast(const HttpProxy &httpProxy)
{
    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_HTTP_PROXY_CHANGE;
    info.data = "Global HttpProxy Changed";
    info.ordered = false;
    std::map<std::string, std::string> param = {{"HttpProxy", httpProxy.ToString()}};
    int32_t userId;
    int32_t ret = GetCallingUserId(userId);
    if (ret == NETMANAGER_SUCCESS) {
        param.emplace("UserId", std::to_string(userId));
    } else {
        NETMGR_LOG_E("SendHttpProxyChangeBroadcast get calling userId fail.");
    }
    BroadcastManager::GetInstance().SendBroadcast(info, param);
}

int32_t NetConnService::ActivateNetwork(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                                        const uint32_t &timeoutMS)
{
    NETMGR_LOG_D("ActivateNetwork Enter");
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    std::weak_ptr<INetActivateCallback> timeoutCb = shared_from_this();
    std::shared_ptr<NetActivate> request =
        std::make_shared<NetActivate>(netSpecifier, callback, timeoutCb, timeoutMS, netActEventHandler_);
    request->StartTimeOutNetAvailable();
    uint32_t reqId = request->GetRequestId();
    NETMGR_LOG_I("Make a new request, request id:[%{public}d]", reqId);
    netActivates_[reqId] = request;
    sptr<NetSupplier> bestNet = nullptr;
    int bestScore = static_cast<int>(FindBestNetworkForRequest(bestNet, request));
    if (bestScore != 0 && bestNet != nullptr) {
        NETMGR_LOG_I("Match to optimal supplier:[%{public}d %{public}s], netId[%{public}d], score:[%{public}d]",
                     bestNet->GetSupplierId(), bestNet->GetNetSupplierIdent().c_str(), bestNet->GetNetId(), bestScore);
        bestNet->SelectAsBestNetwork(reqId);
        request->SetServiceSupply(bestNet);
        CallbackForAvailable(bestNet, callback);
        if ((bestNet->GetNetSupplierType() == BEARER_CELLULAR) || (bestNet->GetNetSupplierType() == BEARER_WIFI)) {
            struct EventInfo eventInfo = {.capabilities = bestNet->GetNetCapabilities().ToString(" "),
                                          .supplierIdent = bestNet->GetNetSupplierIdent()};
            EventReport::SendRequestBehaviorEvent(eventInfo);
        }
        return NETMANAGER_SUCCESS;
    }
    if (timeoutMS == 0) {
        callback->NetUnavailable();
    }

    NETMGR_LOG_D("Not matched to the optimal network, send request to all networks.");
    SendRequestToAllNetwork(request);
    return NETMANAGER_SUCCESS;
}

void NetConnService::OnNetActivateTimeOut(uint32_t reqId)
{
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([reqId, this]() {
            NETMGR_LOG_I("DeactivateNetwork Enter, reqId is [%{public}d]", reqId);
            auto iterActivate = netActivates_.find(reqId);
            if (iterActivate == netActivates_.end()) {
                NETMGR_LOG_E("not found the reqId: [%{public}d]", reqId);
                return;
            }
            if (iterActivate->second != nullptr) {
                sptr<NetSupplier> pNetService = iterActivate->second->GetServiceSupply();
                if (pNetService) {
                    pNetService->CancelRequest(reqId);
                }
            }

            NET_SUPPLIER_MAP::iterator iterSupplier;
            for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
                if (iterSupplier->second == nullptr) {
                    continue;
                }
                iterSupplier->second->CancelRequest(reqId);
            }
        });
    }
}

sptr<NetSupplier> NetConnService::FindNetSupplier(uint32_t supplierId)
{
    auto iterSupplier = netSuppliers_.find(supplierId);
    if (iterSupplier != netSuppliers_.end()) {
        return iterSupplier->second;
    }
    return nullptr;
}

bool NetConnService::FindSameCallback(const sptr<INetConnCallback> &callback,
                                      uint32_t &reqId, RegisterType &registerType)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null");
        return false;
    }
    NET_ACTIVATE_MAP::iterator iterActive;
    for (iterActive = netActivates_.begin(); iterActive != netActivates_.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        sptr<INetConnCallback> saveCallback = iterActive->second->GetNetCallback();
        if (saveCallback == nullptr) {
            continue;
        }
        if (callback->AsObject().GetRefPtr() == saveCallback->AsObject().GetRefPtr()) {
            reqId = iterActive->first;
            if (iterActive->second) {
                auto specifier = iterActive->second->GetNetSpecifier();
                registerType = (specifier != nullptr &&
                    specifier->netCapabilities_.netCaps_.count(
                        NetManagerStandard::NET_CAPABILITY_INTERNAL_DEFAULT) > 0) ?
                        REQUEST : REGISTER;
            }
            return true;
        }
    }
    return false;
}

bool NetConnService::FindSameCallback(const sptr<INetConnCallback> &callback, uint32_t &reqId)
{
    RegisterType registerType = INVALIDTYPE;
    return FindSameCallback(callback, reqId, registerType);
}

void NetConnService::FindBestNetworkForAllRequest()
{
    NETMGR_LOG_I("FindBestNetworkForAllRequest Enter");
    NET_ACTIVATE_MAP::iterator iterActive;
    sptr<NetSupplier> bestSupplier = nullptr;
    for (iterActive = netActivates_.begin(); iterActive != netActivates_.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        int score = static_cast<int>(FindBestNetworkForRequest(bestSupplier, iterActive->second));
        NETMGR_LOG_D("Find best supplier[%{public}d, %{public}s]for request[%{public}d]",
                     bestSupplier ? bestSupplier->GetSupplierId() : 0,
                     bestSupplier ? bestSupplier->GetNetSupplierIdent().c_str() : "null",
                     iterActive->second->GetRequestId());
        if (iterActive->second == defaultNetActivate_) {
            MakeDefaultNetWork(defaultNetSupplier_, bestSupplier);
        }
        sptr<NetSupplier> oldSupplier = iterActive->second->GetServiceSupply();
        sptr<INetConnCallback> callback = iterActive->second->GetNetCallback();
        if (!bestSupplier) {
            // not found the bestNetwork
            NotFindBestSupplier(iterActive->first, iterActive->second, oldSupplier, callback);
            continue;
        }
        SendBestScoreAllNetwork(iterActive->first, score, bestSupplier->GetSupplierId());
        if (bestSupplier == oldSupplier) {
            NETMGR_LOG_D("bestSupplier is equal with oldSupplier.");
            continue;
        }
        if (oldSupplier) {
            oldSupplier->RemoveBestRequest(iterActive->first);
        }
        iterActive->second->SetServiceSupply(bestSupplier);
        CallbackForAvailable(bestSupplier, callback);
        bestSupplier->SelectAsBestNetwork(iterActive->first);
    }
    NETMGR_LOG_I("FindBestNetworkForAllRequest end");
}

uint32_t NetConnService::FindBestNetworkForRequest(sptr<NetSupplier> &supplier,
                                                   std::shared_ptr<NetActivate> &netActivateNetwork)
{
    int bestScore = 0;
    supplier = nullptr;
    if (netActivateNetwork == nullptr) {
        NETMGR_LOG_E("netActivateNetwork is null");
        return bestScore;
    }

    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        NETMGR_LOG_D("supplier info, supplier[%{public}d, %{public}s], realScore[%{public}d], isConnected[%{public}d]",
                     iter->second->GetSupplierId(), iter->second->GetNetSupplierIdent().c_str(),
                     iter->second->GetRealScore(), iter->second->IsConnected());
        if ((!iter->second->IsConnected()) || (!netActivateNetwork->MatchRequestAndNetwork(iter->second))) {
            NETMGR_LOG_D("Supplier[%{public}d] is not connected or not match request.", iter->second->GetSupplierId());
            continue;
        }
        int score = iter->second->GetRealScore();
        if (score > bestScore) {
            bestScore = score;
            supplier = iter->second;
        }
    }
    NETMGR_LOG_D(
        "bestScore[%{public}d], bestSupplier[%{public}d, %{public}s], "
        "request[%{public}d] is [%{public}s],",
        bestScore, supplier ? supplier->GetSupplierId() : 0,
        supplier ? supplier->GetNetSupplierIdent().c_str() : "null", netActivateNetwork->GetRequestId(),
        netActivateNetwork->GetNetSpecifier() ? netActivateNetwork->GetNetSpecifier()->ToString(" ").c_str() : "null");
    return bestScore;
}

void NetConnService::RequestAllNetworkExceptDefault()
{
    if ((defaultNetSupplier_ == nullptr) || (defaultNetSupplier_->IsNetValidated())) {
        NETMGR_LOG_E("defaultNetSupplier_ is  null or IsNetValidated");
        return;
    }
    NETMGR_LOG_I("Default supplier[%{public}d, %{public}s] is not valid,request to activate another network",
                 defaultNetSupplier_->GetSupplierId(), defaultNetSupplier_->GetNetSupplierIdent().c_str());
    if (defaultNetActivate_ == nullptr) {
        NETMGR_LOG_E("Default net request is null");
        return;
    }
    // Request activation of all networks except the default network
    uint32_t reqId = defaultNetActivate_->GetRequestId();
    for (const auto &netSupplier : netSuppliers_) {
        if (netSupplier.second == nullptr || netSupplier.second == defaultNetSupplier_) {
            NETMGR_LOG_E("netSupplier is null or is defaultNetSupplier_");
            continue;
        }
        if (netSupplier.second->GetNetScore() >= defaultNetSupplier_->GetNetScore()) {
            continue;
        }
        if (netSupplier.second->HasNetCap(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT)) {
            NETMGR_LOG_I("Supplier[%{public}d] is internal, skip.", netSupplier.second->GetSupplierId());
            continue;
        }
        if (!defaultNetActivate_->MatchRequestAndNetwork(netSupplier.second)) {
            continue;
        }
        if (!netSupplier.second->RequestToConnect(reqId)) {
            NETMGR_LOG_E("Request network for supplier[%{public}d, %{public}s] failed",
                         netSupplier.second->GetSupplierId(), netSupplier.second->GetNetSupplierIdent().c_str());
        }
    }
}

int32_t NetConnService::GenerateNetId()
{
    for (int32_t i = MIN_NET_ID; i <= MAX_NET_ID; ++i) {
        netIdLastValue_++;
        if (netIdLastValue_ > MAX_NET_ID) {
            netIdLastValue_ = MIN_NET_ID;
        }
        if (networks_.find(netIdLastValue_) == networks_.end()) {
            return netIdLastValue_;
        }
    }
    return INVALID_NET_ID;
}

int32_t NetConnService::GenerateInternalNetId()
{
    for (int32_t i = MIN_INTERNAL_NET_ID; i <= MAX_INTERNAL_NET_ID; ++i) {
        int32_t value = internalNetIdLastValue_++;
        if (value > MAX_INTERNAL_NET_ID) {
            internalNetIdLastValue_ = MIN_INTERNAL_NET_ID;
            value = MIN_INTERNAL_NET_ID;
        }
        if (networks_.find(value) == networks_.end()) {
            return value;
        }
    }
    return INVALID_NET_ID;
}

void NetConnService::NotFindBestSupplier(uint32_t reqId, const std::shared_ptr<NetActivate> &active,
                                         const sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback)
{
    NETMGR_LOG_I("Could not find best supplier for request:[%{public}d]", reqId);
    if (supplier != nullptr) {
        supplier->RemoveBestRequest(reqId);
        if (callback != nullptr) {
            sptr<NetHandle> netHandle = supplier->GetNetHandle();
            callback->NetLost(netHandle);
        }
    }
    if (active != nullptr) {
        active->SetServiceSupply(nullptr);
        SendRequestToAllNetwork(active);
    }
}

void NetConnService::SendAllRequestToNetwork(sptr<NetSupplier> supplier)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is null");
        return;
    }
    NETMGR_LOG_I("Send all request to supplier[%{public}d, %{public}s]", supplier->GetSupplierId(),
                 supplier->GetNetSupplierIdent().c_str());
    NET_ACTIVATE_MAP::iterator iter;
    for (iter = netActivates_.begin(); iter != netActivates_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        if (!iter->second->MatchRequestAndNetwork(supplier)) {
            continue;
        }
        bool result = supplier->RequestToConnect(iter->first);
        if (!result) {
            NETMGR_LOG_E("Request network for supplier[%{public}d, %{public}s] failed", supplier->GetSupplierId(),
                         supplier->GetNetSupplierIdent().c_str());
        }
    }
}

void NetConnService::SendRequestToAllNetwork(std::shared_ptr<NetActivate> request)
{
    if (request == nullptr) {
        NETMGR_LOG_E("request is null");
        return;
    }

    uint32_t reqId = request->GetRequestId();
    NETMGR_LOG_I("Send request[%{public}d] to all supplier", request->GetRequestId());
    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        if (!request->MatchRequestAndNetwork(iter->second)) {
            continue;
        }
        bool result = iter->second->RequestToConnect(reqId);
        if (!result) {
            NETMGR_LOG_E("Request network for supplier[%{public}d, %{public}s] failed", iter->second->GetSupplierId(),
                         iter->second->GetNetSupplierIdent().c_str());
        }
    }
}

void NetConnService::SendBestScoreAllNetwork(uint32_t reqId, int32_t bestScore, uint32_t supplierId)
{
    NETMGR_LOG_D("Send best supplier[%{public}d]-score[%{public}d] to all supplier", supplierId, bestScore);
    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        if (iter->second->HasNetCap(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT)) {
            continue;
        }
        iter->second->ReceiveBestScore(reqId, bestScore, supplierId);
    }
}

void NetConnService::CallbackForSupplier(sptr<NetSupplier> &supplier, CallbackType type)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return;
    }
    std::set<uint32_t> &bestReqList = supplier->GetBestRequestList();
    NETMGR_LOG_I("Callback type: %{public}d for supplier[%{public}d, %{public}s], best request size: %{public}zd",
                 static_cast<int32_t>(type), supplier->GetSupplierId(), supplier->GetNetSupplierIdent().c_str(),
                 bestReqList.size());
    for (auto it : bestReqList) {
        auto reqIt = netActivates_.find(it);
        if ((reqIt == netActivates_.end()) || (reqIt->second == nullptr)) {
            continue;
        }
        sptr<INetConnCallback> callback = reqIt->second->GetNetCallback();
        if (!callback) {
            continue;
        }
        sptr<NetHandle> netHandle = supplier->GetNetHandle();
        switch (type) {
            case CALL_TYPE_LOST: {
                callback->NetLost(netHandle);
                break;
            }
            case CALL_TYPE_UPDATE_CAP: {
                sptr<NetAllCapabilities> pNetAllCap = std::make_unique<NetAllCapabilities>().release();
                *pNetAllCap = supplier->GetNetCapabilities();
                callback->NetCapabilitiesChange(netHandle, pNetAllCap);
                break;
            }
            case CALL_TYPE_UPDATE_LINK: {
                sptr<NetLinkInfo> pInfo = std::make_unique<NetLinkInfo>().release();
                auto network = supplier->GetNetwork();
                if (network != nullptr && pInfo != nullptr) {
                    *pInfo = network->GetNetLinkInfo();
                }
                callback->NetConnectionPropertiesChange(netHandle, pInfo);
                break;
            }
            case CALL_TYPE_BLOCK_STATUS: {
                bool Metered = supplier->HasNetCap(NET_CAPABILITY_NOT_METERED);
                bool newBlocked = NetManagerCenter::GetInstance().IsUidNetAccess(supplier->GetSupplierUid(), Metered);
                callback->NetBlockStatusChange(netHandle, newBlocked);
                break;
            }
            default:
                break;
        }
    }
}

void NetConnService::CallbackForAvailable(sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback)
{
    NETMGR_LOG_I("Callback net available for supplier[%{public}d, %{public}s]",
                 supplier ? supplier->GetSupplierId() : 0,
                 supplier ? supplier->GetNetSupplierIdent().c_str() : "nullptr");
    if (supplier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("Input parameter is null.");
        return;
    }
    sptr<NetHandle> netHandle = supplier->GetNetHandle();
    callback->NetAvailable(netHandle);
    sptr<NetAllCapabilities> pNetAllCap = std::make_unique<NetAllCapabilities>().release();
    *pNetAllCap = supplier->GetNetCapabilities();
    callback->NetCapabilitiesChange(netHandle, pNetAllCap);
    sptr<NetLinkInfo> pInfo = std::make_unique<NetLinkInfo>().release();
    auto network = supplier->GetNetwork();
    if (network != nullptr && pInfo != nullptr) {
        *pInfo = network->GetNetLinkInfo();
    }
    callback->NetConnectionPropertiesChange(netHandle, pInfo);
    NetsysController::GetInstance().NotifyNetBearerTypeChange(pNetAllCap->bearerTypes_);
}

void NetConnService::MakeDefaultNetWork(sptr<NetSupplier> &oldSupplier, sptr<NetSupplier> &newSupplier)
{
    NETMGR_LOG_I(
        "oldSupplier[%{public}d, %{public}s], newSupplier[%{public}d, %{public}s], old equals "
        "new is [%{public}d]", oldSupplier ? oldSupplier->GetSupplierId() : 0,
        oldSupplier ? oldSupplier->GetNetSupplierIdent().c_str() : "null",
        newSupplier ? newSupplier->GetSupplierId() : 0,
        newSupplier ? newSupplier->GetNetSupplierIdent().c_str() : "null", oldSupplier == newSupplier);
    if (oldSupplier == newSupplier) {
        NETMGR_LOG_D("old supplier equal to new supplier.");
        return;
    }
    if (oldSupplier != nullptr) {
        oldSupplier->ClearDefault();
    }
    if (newSupplier != nullptr) {
        newSupplier->SetDefault();
    }
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    oldSupplier = newSupplier;
}

void NetConnService::HandleDetectionResult(uint32_t supplierId, NetDetectionStatus netState)
{
    NETMGR_LOG_I("Enter HandleDetectionResult, ifValid[%{public}d]", netState);
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return;
    }
    supplier->SetNetValid(netState);
    if (netState != QUALITY_POOR_STATE && netState != QUALITY_NORMAL_STATE && netState != QUALITY_GOOD_STATE) {
        CallbackForSupplier(supplier, CALL_TYPE_UPDATE_CAP);
    }
    if (!NetScore::GetServiceScore(supplier)) {
        NETMGR_LOG_E("GetServiceScore fail.");
        return;
    }
    FindBestNetworkForAllRequest();
    bool ifValid = netState == VERIFICATION_STATE;
    if (!ifValid && defaultNetSupplier_ && defaultNetSupplier_->GetSupplierId() == supplierId) {
        RequestAllNetworkExceptDefault();
    }
    NETMGR_LOG_I("Enter HandleDetectionResult end");
}

std::list<sptr<NetSupplier>> NetConnService::GetNetSupplierFromList(NetBearType bearerType, const std::string &ident)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    std::list<sptr<NetSupplier>> ret;
    for (const auto &netSupplier : netSuppliers_) {
        if (netSupplier.second == nullptr) {
            continue;
        }
        if ((bearerType != netSupplier.second->GetNetSupplierType())) {
            continue;
        }
        if (!ident.empty() && netSupplier.second->GetNetSupplierIdent() != ident) {
            continue;
        }
        ret.push_back(netSupplier.second);
    }
    return ret;
}

sptr<NetSupplier> NetConnService::GetNetSupplierFromList(NetBearType bearerType, const std::string &ident,
                                                         const std::set<NetCap> &netCaps)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    for (const auto &netSupplier : netSuppliers_) {
        if (netSupplier.second == nullptr) {
            continue;
        }
        if ((bearerType == netSupplier.second->GetNetSupplierType()) &&
            (ident == netSupplier.second->GetNetSupplierIdent()) && netSupplier.second->CompareNetCaps(netCaps)) {
            return netSupplier.second;
        }
    }
    return nullptr;
}

int32_t NetConnService::GetDefaultNet(int32_t &netId)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    if (!defaultNetSupplier_) {
        NETMGR_LOG_E("not found the netId");
        return NETMANAGER_SUCCESS;
    }

    netId = defaultNetSupplier_->GetNetId();
    NETMGR_LOG_D("GetDefaultNet found the netId: [%{public}d]", netId);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList)
{
    return NetManagerCenter::GetInstance().GetAddressesByName(host, static_cast<uint16_t>(netId), addrList);
}

int32_t NetConnService::GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr)
{
    std::vector<INetAddr> addrList;
    int ret = GetAddressesByName(host, netId, addrList);
    if (ret == NETMANAGER_SUCCESS) {
        if (!addrList.empty()) {
            addr = addrList[0];
            return ret;
        }
        return NET_CONN_ERR_NO_ADDRESS;
    }
    return ret;
}

int32_t NetConnService::GetSpecificNet(NetBearType bearerType, std::list<int32_t> &netIdList)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    std::lock_guard<std::mutex> locker(netManagerMutex_);
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if (iterSupplier->second == nullptr) {
            continue;
        }
        auto supplierType = iterSupplier->second->GetNetSupplierType();
        if (bearerType == supplierType) {
            netIdList.push_back(iterSupplier->second->GetNetId());
        }
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}zd] networks_ size[%{public}zd]", netSuppliers_.size(), networks_.size());
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetAllNets(std::list<int32_t> &netIdList)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    auto currentUid = IPCSkeleton::GetCallingUid();
    for (const auto &network : networks_) {
        if (network.second != nullptr && network.second->IsConnected()) {
            auto netId = network.second->GetNetId();
            sptr<NetSupplier> curSupplier = FindNetSupplier(network.second->GetSupplierId());
            // inner virtual interface and uid is not trusted, skip
            if (curSupplier != nullptr &&
                curSupplier->HasNetCap(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT) &&
                !IsInRequestNetUids(currentUid)) {
                NETMGR_LOG_D("Network [%{public}d] is internal, uid [%{public}d] skips.", netId, currentUid);
                continue;
            }
            netIdList.push_back(netId);
        }
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}zd] netIdList size[%{public}zd]", netSuppliers_.size(), netIdList.size());
    return NETMANAGER_SUCCESS;
}

bool NetConnService::IsInRequestNetUids(int32_t uid)
{
    return internalDefaultUidRequest_.count(uid) > 0;
}

int32_t NetConnService::GetSpecificUidNet(int32_t uid, int32_t &netId)
{
    NETMGR_LOG_D("Enter GetSpecificUidNet, uid is [%{public}d].", uid);
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    netId = INVALID_NET_ID;
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if ((iterSupplier->second != nullptr) && (uid == iterSupplier->second->GetSupplierUid()) &&
            (iterSupplier->second->GetNetSupplierType() == BEARER_VPN)) {
            netId = iterSupplier->second->GetNetId();
            return NETMANAGER_SUCCESS;
        }
    }
    if (defaultNetSupplier_ != nullptr) {
        netId = defaultNetSupplier_->GetNetId();
    }
    NETMGR_LOG_D("GetDefaultNet found the netId: [%{public}d]", netId);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetConnectionProperties(int32_t netId, NetLinkInfo &info)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    info = iterNetwork->second->GetNetLinkInfo();
    if (info.mtu_ == 0) {
        info.mtu_ = DEFAULT_MTU;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetNetCapabilities(int32_t netId, NetAllCapabilities &netAllCap)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if ((iterSupplier->second != nullptr) && (netId == iterSupplier->second->GetNetId())) {
            netAllCap = iterSupplier->second->GetNetCapabilities();
            return NETMANAGER_SUCCESS;
        }
    }
    return NET_CONN_ERR_INVALID_NETWORK;
}

int32_t NetConnService::GetIfaceNames(NetBearType bearerType, std::list<std::string> &ifaceNames)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    auto suppliers = GetNetSupplierFromList(bearerType);
    for (auto supplier : suppliers) {
        if (supplier == nullptr) {
            continue;
        }
        std::shared_ptr<Network> network = supplier->GetNetwork();
        if (network == nullptr) {
            continue;
        }
        std::string ifaceName = network->GetNetLinkInfo().ifaceName_;
        if (!ifaceName.empty()) {
            ifaceNames.push_back(ifaceName);
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetIfaceNameByType(NetBearType bearerType, const std::string &ident, std::string &ifaceName)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    auto suppliers = GetNetSupplierFromList(bearerType, ident);
    if (suppliers.empty()) {
        NETMGR_LOG_D("supplier is nullptr.");
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    auto supplier = suppliers.front();
    std::shared_ptr<Network> network = supplier->GetNetwork();
    if (network == nullptr) {
        NETMGR_LOG_E("network is nullptr");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    ifaceName = network->GetNetLinkInfo().ifaceName_;

    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetIfaceNameIdentMaps(NetBearType bearerType,
                                              std::unordered_map<std::string, std::string> &ifaceNameIdentMaps)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    auto suppliers = GetNetSupplierFromList(bearerType);
    for (auto supplier : suppliers) {
        if (supplier == nullptr) {
            continue;
        }
        std::shared_ptr<Network> network = supplier->GetNetwork();
        if (network == nullptr) {
            continue;
        }
        std::string ifaceName = network->GetNetLinkInfo().ifaceName_;
        if (ifaceName.empty()) {
            continue;
        }
        std::string ident = network->GetNetLinkInfo().ident_;
        ifaceNameIdentMaps.emplace(std::move(ifaceName), std::move(ident));
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetGlobalHttpProxy(HttpProxy &httpProxy)
{
    LoadGlobalHttpProxy();
    if (globalHttpProxy_.GetHost().empty()) {
        httpProxy.SetPort(0);
        NETMGR_LOG_E("The http proxy host is empty");
        return NETMANAGER_SUCCESS;
    }
    httpProxy = globalHttpProxy_;
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetDefaultHttpProxy(int32_t bindNetId, HttpProxy &httpProxy)
{
    LoadGlobalHttpProxy();
    if (!globalHttpProxy_.GetHost().empty()) {
        httpProxy = globalHttpProxy_;
        NETMGR_LOG_D("Return global http proxy as default.");
        return NETMANAGER_SUCCESS;
    }

    std::lock_guard<std::mutex> locker(netManagerMutex_);
    auto iter = networks_.find(bindNetId);
    if ((iter != networks_.end()) && (iter->second != nullptr)) {
        httpProxy = iter->second->GetNetLinkInfo().httpProxy_;
        NETMGR_LOG_D("Return bound network's http proxy as default.");
        return NETMANAGER_SUCCESS;
    }

    if (defaultNetSupplier_ != nullptr) {
        defaultNetSupplier_->GetHttpProxy(httpProxy);
        NETMGR_LOG_D("Return default network's http proxy as default.");
        return NETMANAGER_SUCCESS;
    }
    NETMGR_LOG_D("No default http proxy.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetNetIdByIdentifier(const std::string &ident, std::list<int32_t> &netIdList)
{
    if (ident.empty()) {
        NETMGR_LOG_E("The identifier in service is null");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    for (auto iterSupplier : netSuppliers_) {
        if (iterSupplier.second == nullptr) {
            continue;
        }
        if (iterSupplier.second->GetNetSupplierIdent() == ident) {
            int32_t netId = iterSupplier.second->GetNetId();
            netIdList.push_back(netId);
        }
    }
    return NETMANAGER_SUCCESS;
}

void NetConnService::GetDumpMessage(std::string &message)
{
    message.append("Net connect Info:\n");
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    if (defaultNetSupplier_) {
        message.append("\tSupplierId: " + std::to_string(defaultNetSupplier_->GetSupplierId()) + "\n");
        std::shared_ptr<Network> network = defaultNetSupplier_->GetNetwork();
        if (network) {
            message.append("\tNetId: " + std::to_string(network->GetNetId()) + "\n");
        } else {
            message.append("\tNetId: " + std::to_string(INVALID_NET_ID) + "\n");
        }
        message.append("\tConnStat: " + std::to_string(defaultNetSupplier_->IsConnected()) + "\n");
        message.append("\tIsAvailable: " + std::to_string(defaultNetSupplier_->IsNetValidated()) + "\n");
        message.append("\tIsRoaming: " + std::to_string(defaultNetSupplier_->GetRoaming()) + "\n");
        message.append("\tStrength: " + std::to_string(defaultNetSupplier_->GetStrength()) + "\n");
        message.append("\tFrequency: " + std::to_string(defaultNetSupplier_->GetFrequency()) + "\n");
        message.append("\tLinkUpBandwidthKbps: " +
                       std::to_string(defaultNetSupplier_->GetNetCapabilities().linkUpBandwidthKbps_) + "\n");
        message.append("\tLinkDownBandwidthKbps: " +
                       std::to_string(defaultNetSupplier_->GetNetCapabilities().linkDownBandwidthKbps_) + "\n");
        message.append("\tUid: " + std::to_string(defaultNetSupplier_->GetSupplierUid()) + "\n");
    } else {
        message.append("\tdefaultNetSupplier_ is nullptr\n");
        message.append("\tSupplierId: \n");
        message.append("\tNetId: 0\n");
        message.append("\tConnStat: 0\n");
        message.append("\tIsAvailable: \n");
        message.append("\tIsRoaming: 0\n");
        message.append("\tStrength: 0\n");
        message.append("\tFrequency: 0\n");
        message.append("\tLinkUpBandwidthKbps: 0\n");
        message.append("\tLinkDownBandwidthKbps: 0\n");
        message.append("\tUid: 0\n");
    }
    if (dnsResultCallback_ != nullptr) {
        dnsResultCallback_->GetDumpMessageForDnsResult(message);
    }
}

int32_t NetConnService::HasDefaultNet(bool &flag)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    if (!defaultNetSupplier_) {
        flag = false;
        return NETMANAGER_SUCCESS;
    }
    flag = true;
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::IsDefaultNetMetered(bool &isMetered)
{
    std::lock_guard<std::mutex> locker(netManagerMutex_);
    if (defaultNetSupplier_) {
        isMetered = !defaultNetSupplier_->HasNetCap(NET_CAPABILITY_NOT_METERED);
    } else {
        isMetered = true;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::BindSocket(int32_t socketFd, int32_t netId)
{
    NETMGR_LOG_D("Enter BindSocket.");
    return NetsysController::GetInstance().BindSocket(socketFd, netId);
}

int32_t NetConnService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_LOG_D("Start Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return (ret < 0) ? static_cast<int32_t>(NET_CONN_ERR_CREATE_DUMP_FAILED) : static_cast<int32_t>(NETMANAGER_SUCCESS);
}

bool NetConnService::IsValidDecValue(const std::string &inputValue)
{
    if (inputValue.length() > INPUT_VALUE_LENGTH) {
        NETMGR_LOG_E("The value entered is out of range, value:%{public}s", inputValue.c_str());
        return false;
    }
    bool isValueNumber = regex_match(inputValue, std::regex("(-[\\d+]+)|(\\d+)"));
    if (isValueNumber) {
        int64_t numberValue = std::stoll(inputValue);
        if ((numberValue >= INT32_MIN) && (numberValue <= INT32_MAX)) {
            return true;
        }
    }
    NETMGR_LOG_I("InputValue is not a decimal number");
    return false;
}

int32_t NetConnService::GetDelayNotifyTime()
{
    char param[SYS_PARAMETER_SIZE] = { 0 };
    int32_t delayTime = 0;
    int32_t code = GetParameter(CFG_NETWORK_PRE_AIRPLANE_MODE_WAIT_TIMES, NO_DELAY_TIME_CONFIG,
                                param, SYS_PARAMETER_SIZE);
    std::string time = param;
    if (code <= 0 || !IsValidDecValue(time)) {
        delayTime = std::stoi(NO_DELAY_TIME_CONFIG);
    } else {
        auto tmp = std::stoi(time);
        delayTime = tmp > MAX_DELAY_TIME ? std::stoi(NO_DELAY_TIME_CONFIG) : tmp;
    }
    NETMGR_LOG_D("delay time is %{public}d", delayTime);
    return delayTime;
}

int32_t NetConnService::RegisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback)
{
    int32_t callingUid = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    NETMGR_LOG_D("RegisterPreAirplaneCallback, calllinguid [%{public}d]", callingUid);
    std::lock_guard guard(preAirplaneCbsMutex_);
    preAirplaneCallbacks_[callingUid] = callback;
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::UnregisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback)
{
    int32_t callingUid = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    NETMGR_LOG_D("UnregisterPreAirplaneCallback, calllinguid [%{public}d]", callingUid);
    std::lock_guard guard(preAirplaneCbsMutex_);
    preAirplaneCallbacks_.erase(callingUid);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::SetAirplaneMode(bool state)
{
    NETMGR_LOG_I("Enter SetAirplaneMode, AirplaneMode is %{public}d", state);
    if (state) {
        std::lock_guard guard(preAirplaneCbsMutex_);
        for (const auto& mem : preAirplaneCallbacks_) {
            if (mem.second != nullptr) {
                int32_t ret = mem.second->PreAirplaneStart();
                NETMGR_LOG_D("PreAirplaneStart result %{public}d", ret);
            }
        }
    }
    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netConnEventHandler_->RemoveAsyncTask("delay airplane mode");
    auto delayTime = GetDelayNotifyTime();

    netConnEventHandler_->PostAsyncTask(
        [state]() {
            NETMGR_LOG_D("Enter delay");
            auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
            std::string airplaneMode = std::to_string(state);
            Uri uri(AIRPLANE_MODE_URI);
            int32_t ret = dataShareHelperUtils->Update(uri, KEY_AIRPLANE_MODE, airplaneMode);
            if (ret != NETMANAGER_SUCCESS) {
                NETMGR_LOG_E("Update airplane mode:%{public}d to datashare failed.", state);
                return;
            }
            BroadcastInfo info;
            info.action = EventFwk::CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED;
            info.data = "Net Manager Airplane Mode Changed";
            info.code = static_cast<int32_t>(state);
            info.ordered = false;
            std::map<std::string, int32_t> param;
            BroadcastManager::GetInstance().SendBroadcast(info, param);
        },
        "delay airplane mode", delayTime);

    return NETMANAGER_SUCCESS;
}

void NetConnService::ActiveHttpProxy()
{
    NETMGR_LOG_D("ActiveHttpProxy thread start");
    while (httpProxyThreadNeedRun_.load()) {
        NETMGR_LOG_D("Keep global http-proxy active every 2 minutes");
        CURL *curl = nullptr;
        HttpProxy tempProxy;
        {
            std::lock_guard guard(globalHttpProxyMutex_);
            auto userInfoHelp = NetProxyUserinfo::GetInstance();
            tempProxy = globalHttpProxy_;
            userInfoHelp.GetHttpProxyHostPass(tempProxy);
        }
        auto proxyType = (tempProxy.host_.find("https://") != std::string::npos) ? CURLPROXY_HTTPS : CURLPROXY_HTTP;
        if (!tempProxy.host_.empty() && !tempProxy.username_.empty()) {
            curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, NET_HTTP_PROBE_URL);
            curl_easy_setopt(curl, CURLOPT_PROXY, tempProxy.host_.c_str());
            curl_easy_setopt(curl, CURLOPT_PROXYPORT, tempProxy.port_);
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, proxyType);
            curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, tempProxy.username_.c_str());
            curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
            if (!tempProxy.password_.empty()) {
                curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, tempProxy.password_.c_str());
            }
        }
        if (curl) {
            auto ret = curl_easy_perform(curl);
            NETMGR_LOG_I("SetGlobalHttpProxy ActiveHttpProxy %{public}d", static_cast<int>(ret));
            curl_easy_cleanup(curl);
        }
        if (httpProxyThreadNeedRun_.load()) {
            std::unique_lock lock(httpProxyThreadMutex_);
            httpProxyThreadCv_.wait_for(lock, std::chrono::seconds(HTTP_PROXY_ACTIVE_PERIOD_S));
        }
    }
}

int32_t NetConnService::SetGlobalHttpProxy(const HttpProxy &httpProxy)
{
    NETMGR_LOG_I("Enter SetGlobalHttpProxy. httpproxy = %{public}s", httpProxy.GetHost().c_str());
    if (!httpProxyThreadNeedRun_ && !httpProxy.GetUsername().empty()) {
        NETMGR_LOG_I("ActiveHttpProxy  user.len[%{public}zu], pwd.len[%{public}zu]", httpProxy.username_.length(),
                     httpProxy.password_.length());
        httpProxyThreadNeedRun_ = true;
        std::thread t([this]() { ActiveHttpProxy(); });
        std::string threadName = "ActiveHttpProxy";
        pthread_setname_np(t.native_handle(), threadName.c_str());
        t.detach();
    } else if (httpProxyThreadNeedRun_ && httpProxy.GetHost().empty()) {
        httpProxyThreadNeedRun_ = false;
    }

    LoadGlobalHttpProxy();
    if (globalHttpProxy_ != httpProxy) {
        {
            std::lock_guard guard(globalHttpProxyMutex_);
            globalHttpProxy_ = httpProxy;
        }
        httpProxyThreadCv_.notify_all();
        int32_t userId;
        int32_t ret = GetCallingUserId(userId);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("GlobalHttpProxy get calling userId fail.");
            return ret;
        }
        NetHttpProxyTracker httpProxyTracker;
        if (IsPrimaryUserId(userId)) {
            if (!httpProxyTracker.WriteToSettingsData(globalHttpProxy_)) {
                NETMGR_LOG_E("GlobalHttpProxy write settingDate fail.");
                return NETMANAGER_ERR_INTERNAL;
            }
        }
        if (!httpProxyTracker.WriteToSettingsDataUser(globalHttpProxy_, userId)) {
            NETMGR_LOG_E("GlobalHttpProxy write settingDateUser fail. userId=%{public}d", userId);
            return NETMANAGER_ERR_INTERNAL;
        }
        SendHttpProxyChangeBroadcast(globalHttpProxy_);
        UpdateGlobalHttpProxy(globalHttpProxy_);
    }
    NETMGR_LOG_I("End SetGlobalHttpProxy.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetCallingUserId(int32_t &userId)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId) != ERR_OK) {
        NETMGR_LOG_E("GetOsAccountLocalIdFromUid error, uid: %{public}d.", uid);
        return NETMANAGER_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::SetAppNet(int32_t netId)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RegisterNetInterfaceCallback(const sptr<INetInterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    NETMGR_LOG_I("Enter RegisterNetInterfaceCallback.");
    if (interfaceStateCallback_ == nullptr) {
        NETMGR_LOG_E("interfaceStateCallback_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return interfaceStateCallback_->RegisterInterfaceCallback(callback);
}

int32_t NetConnService::GetNetInterfaceConfiguration(const std::string &iface, NetInterfaceConfiguration &config)
{
    using namespace OHOS::nmd;
    InterfaceConfigurationParcel configParcel;
    configParcel.ifName = iface;
    if (NetsysController::GetInstance().GetInterfaceConfig(configParcel) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERR_INTERNAL;
    }
    config.ifName_ = configParcel.ifName;
    config.hwAddr_ = configParcel.hwAddr;
    config.ipv4Addr_ = configParcel.ipv4Addr;
    config.prefixLength_ = configParcel.prefixLength;
    config.flags_.assign(configParcel.flags.begin(), configParcel.flags.end());
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetDetectionForDnsHealth(int32_t netId, bool dnsHealthSuccess)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([netId, dnsHealthSuccess, &result, this]() {
            result = this->NetDetectionForDnsHealthSync(netId, dnsHealthSuccess);
        });
    }
    return result;
}

void NetConnService::LoadGlobalHttpProxy()
{
    if (isGlobalProxyLoaded_.load()) {
        NETMGR_LOG_D("Global http proxy has been loaded from the SettingsData database.");
        return;
    }
    int32_t userId;
    int32_t ret = GetCallingUserId(userId);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("LoadGlobalHttpProxy get calling userId fail.");
        return;
    }
    NetHttpProxyTracker httpProxyTracker;
    if (IsPrimaryUserId(userId)) {
        httpProxyTracker.ReadFromSettingsData(globalHttpProxy_);
    } else {
        httpProxyTracker.ReadFromSettingsDataUser(globalHttpProxy_, userId);
    }
    isGlobalProxyLoaded_ = true;
}

void NetConnService::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return;
    }
    NETMGR_LOG_I("UpdateGlobalHttpProxy start");
    netConnEventHandler_->PostAsyncTask([this, httpProxy]() {
        for (const auto &supplier : netSuppliers_) {
            if (supplier.second == nullptr) {
                continue;
            }
            supplier.second->UpdateGlobalHttpProxy(httpProxy);
        }
        NETMGR_LOG_I("UpdateGlobalHttpProxy end");
    });
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceAddressUpdated(const std::string &addr,
                                                                             const std::string &ifName, int flags,
                                                                             int scope)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceAddressRemoved(const std::string &addr,
                                                                             const std::string &ifName, int flags,
                                                                             int scope)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceAdded(const std::string &iface)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceAdded(iface);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceRemoved(const std::string &iface)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceRemoved(iface);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceChanged(const std::string &iface, bool up)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceChanged(iface, up);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceLinkStateChanged(const std::string &iface, bool up)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceLinkStateChanged(iface, up);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnRouteChanged(bool updated, const std::string &route,
                                                                  const std::string &gateway, const std::string &ifName)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnBandwidthReachedLimit(const std::string &limitName,
                                                                           const std::string &iface)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::RegisterInterfaceCallback(
    const sptr<INetInterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &iter : ifaceStateCallbacks_) {
        if (!iter) {
            continue;
        }
        if (iter->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            NETMGR_LOG_E("RegisterInterfaceCallback find same callback");
            return NET_CONN_ERR_SAME_CALLBACK;
        }
    }
    ifaceStateCallbacks_.push_back(callback);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::AddNetworkRoute(int32_t netId, const std::string &ifName,
                                        const std::string &destination, const std::string &nextHop)
{
    return NetsysController::GetInstance().NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetConnService::RemoveNetworkRoute(int32_t netId, const std::string &ifName,
                                           const std::string &destination, const std::string &nextHop)
{
    return NetsysController::GetInstance().NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

int32_t NetConnService::AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                            int32_t prefixLength)
{
    return NetsysController::GetInstance().AddInterfaceAddress(ifName, ipAddr, prefixLength);
}

int32_t NetConnService::DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                            int32_t prefixLength)
{
    return NetsysController::GetInstance().DelInterfaceAddress(ifName, ipAddr, prefixLength);
}

int32_t NetConnService::AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                     const std::string &ifName)
{
    return NetsysController::GetInstance().AddStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetConnService::DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                     const std::string &ifName)
{
    return NetsysController::GetInstance().DelStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetConnService::RegisterSlotType(uint32_t supplierId, int32_t type)
{
    int32_t result = NETMANAGER_SUCCESS;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, type, &result]() {
            if (netSuppliers_.find(supplierId) == netSuppliers_.end()) {
                NETMGR_LOG_E("supplierId[%{public}d] is not exits", supplierId);
                result =  NETMANAGER_ERR_INVALID_PARAMETER;
            } else {
                NETMGR_LOG_I("supplierId[%{public}d] update type[%{public}d].", supplierId, type);
                sptr<NetSupplier> supplier = netSuppliers_[supplierId];
                supplier->SetSupplierType(type);
                result =  NETMANAGER_SUCCESS;
            }
        });
    }
    return result;
}

int32_t NetConnService::GetSlotType(std::string &type)
{
    int32_t result = NETMANAGER_SUCCESS;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &type, &result]() {
            if (defaultNetSupplier_ == nullptr) {
                NETMGR_LOG_E("supplier is nullptr");
                result =  NETMANAGER_ERR_LOCAL_PTR_NULL;
            } else {
                type = defaultNetSupplier_->GetSupplierType();
                result =  NETMANAGER_SUCCESS;
            }
        });
    }
    return result;
}

int32_t NetConnService::FactoryResetNetwork()
{
    NETMGR_LOG_I("Enter FactoryResetNetwork.");

    SetAirplaneMode(false);

    if (netFactoryResetCallback_ == nullptr) {
        NETMGR_LOG_E("netFactoryResetCallback_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netFactoryResetCallback_->NotifyNetFactoryResetAsync();

    NETMGR_LOG_I("End FactoryResetNetwork.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RegisterNetFactoryResetCallback(const sptr<INetFactoryResetCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    NETMGR_LOG_I("Enter RegisterNetFactoryResetCallback.");
    if (netFactoryResetCallback_ == nullptr) {
        NETMGR_LOG_E("netFactoryResetCallback_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netFactoryResetCallback_->RegisterNetFactoryResetCallbackAsync(callback);
}

void NetConnService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_LOG_I("OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSARemoved_) {
            OnNetSysRestart();
            hasSARemoved_ = false;
        }
    }
}

void NetConnService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_LOG_I("OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSARemoved_ = true;
    }
}

bool NetConnService::IsSupplierMatchRequestAndNetwork(sptr<NetSupplier> ns)
{
    if (ns == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return false;
    }
    NET_ACTIVATE_MAP::iterator iterActive;
    for (iterActive = netActivates_.begin(); iterActive != netActivates_.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        if (iterActive->second->MatchRequestAndNetwork(ns)) {
            return true;
        }
    }

    return false;
}

void NetConnService::OnNetSysRestart()
{
    NETMGR_LOG_I("OnNetSysRestart");

    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }

        NETMGR_LOG_D("supplier info, supplier[%{public}d, %{public}s], realScore[%{public}d], isConnected[%{public}d]",
            iter->second->GetSupplierId(), iter->second->GetNetSupplierIdent().c_str(),
            iter->second->GetRealScore(), iter->second->IsConnected());

        if ((!iter->second->IsConnected()) || (!IsSupplierMatchRequestAndNetwork(iter->second))) {
            NETMGR_LOG_D("Supplier[%{public}d] is not connected or not match request.", iter->second->GetSupplierId());
            continue;
        }

        iter->second->ResumeNetworkInfo();
    }
    std::unique_lock<std::mutex> locker(netManagerMutex_);
    if (defaultNetSupplier_ != nullptr) {
        defaultNetSupplier_->ClearDefault();
        defaultNetSupplier_ = nullptr;
    }
    locker.unlock();
    FindBestNetworkForAllRequest();
}

int32_t NetConnService::IsPreferCellularUrl(const std::string& url, bool& preferCellular)
{
    static std::vector<std::string> preferredUrlList = GetPreferredUrl();
    preferCellular = std::any_of(preferredUrlList.begin(), preferredUrlList.end(),
                                 [&url](const std::string &str) { return url.find(str) != std::string::npos; });
    return 0;
}

bool NetConnService::IsAddrInOtherNetwork(const std::string &ifaceName, int32_t netId, const INetAddr &netAddr)
{
    for (const auto &network : networks_) {
        if (network.second->GetNetId() == netId) {
            continue;
        }
        if (network.second->GetNetLinkInfo().ifaceName_ != ifaceName) {
            continue;
        }
        if (network.second->GetNetLinkInfo().HasNetAddr(netAddr)) {
            return true;
        }
    }
    return false;
}

bool NetConnService::IsIfaceNameInUse(const std::string &ifaceName, int32_t netId)
{
    for (const auto &network : networks_) {
        if (network.second->GetNetId() == netId) {
            continue;
        }
        if (network.second->GetNetLinkInfo().ifaceName_ == ifaceName) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> NetConnService::GetPreferredUrl()
{
    std::vector<std::string> preferCellularUrlList;
    const std::string preferCellularUrlPath = "/system/etc/prefer_cellular_url_list.txt";
    std::ifstream preferCellularFile(preferCellularUrlPath);
    if (preferCellularFile.is_open()) {
        std::string line;
        while (getline(preferCellularFile, line)) {
            preferCellularUrlList.push_back(line);
        }
        preferCellularFile.close();
    } else {
        NETMGR_LOG_E("open prefer cellular url file failure.");
    }
    return preferCellularUrlList;
}

void NetConnService::OnRemoteDied(const wptr<IRemoteObject> &remoteObject)
{
    sptr<IRemoteObject> diedRemoted = remoteObject.promote();
    if (diedRemoted == nullptr) {
        NETMGR_LOG_E("diedRemoted is null");
        return;
    }
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    NETMGR_LOG_I("OnRemoteDied, callingUid=%{public}u", callingUid);
    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(diedRemoted);
    UnregisterNetConnCallback(callback);
}

void NetConnService::RemoveClientDeathRecipient(const sptr<INetConnCallback> &callback)
{
    std::lock_guard<std::mutex> autoLock(remoteMutex_);
    auto iter =
        std::find_if(remoteCallback_.cbegin(), remoteCallback_.cend(), [&callback](const sptr<INetConnCallback> &item) {
            return item->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr();
        });
    if (iter == remoteCallback_.cend()) {
        return;
    }
    callback->AsObject()->RemoveDeathRecipient(deathRecipient_);
    remoteCallback_.erase(iter);
}

void NetConnService::AddClientDeathRecipient(const sptr<INetConnCallback> &callback)
{
    std::lock_guard<std::mutex> autoLock(remoteMutex_);
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) ConnCallbackDeathRecipient(*this);
    }
    if (deathRecipient_ == nullptr) {
        NETMGR_LOG_E("deathRecipient is null");
        return;
    }
    if (!callback->AsObject()->AddDeathRecipient(deathRecipient_)) {
        NETMGR_LOG_E("AddClientDeathRecipient failed");
        return;
    }
    auto iter =
        std::find_if(remoteCallback_.cbegin(), remoteCallback_.cend(), [&callback](const sptr<INetConnCallback> &item) {
            return item->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr();
        });
    if (iter == remoteCallback_.cend()) {
        remoteCallback_.emplace_back(callback);
    }
}

void NetConnService::RemoveALLClientDeathRecipient()
{
    std::lock_guard<std::mutex> autoLock(remoteMutex_);
    for (auto &item : remoteCallback_) {
        item->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    remoteCallback_.clear();
    deathRecipient_ = nullptr;
}

std::vector<sptr<NetSupplier>> NetConnService::FindSupplierWithInternetByBearerType(NetBearType bearerType)
{
    std::vector<sptr<NetSupplier>> result;
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if (iterSupplier->second == nullptr) {
            continue;
        }
        if (!iterSupplier->second->GetNetCaps().HasNetCap(NET_CAPABILITY_INTERNET)) {
            continue;
        }
        std::set<NetBearType>::iterator iter = iterSupplier->second->GetNetCapabilities().bearerTypes_.find(bearerType);
        if (iter != iterSupplier->second->GetNetCapabilities().bearerTypes_.end()) {
            NETMGR_LOG_I("found supplierId[%{public}d] by bearertype[%{public}d].", iterSupplier->first, bearerType);
            result.push_back(iterSupplier->second);
        }
    }
    return result;
}

int32_t NetConnService::UpdateSupplierScore(NetBearType bearerType, bool isBetter)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, bearerType, isBetter, &result]() {
            result = this->UpdateSupplierScoreAsync(bearerType, isBetter);
        });
    }
    return result;
}

int32_t NetConnService::UpdateSupplierScoreAsync(NetBearType bearerType, bool isBetter)
{
    NETMGR_LOG_I("update supplier score by bearertype[%{public}d], isBetter[%{public}d]", bearerType, isBetter);
    std::vector<sptr<NetSupplier>> suppliers = FindSupplierWithInternetByBearerType(bearerType);
    if (suppliers.empty()) {
        NETMGR_LOG_E(" not found supplierId by bearertype[%{public}d].", bearerType);
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    if (!defaultNetSupplier_) {
        NETMGR_LOG_E("default net supplier nullptr");
        return NETMANAGER_ERR_INTERNAL;
    }
    uint32_t supplierId = INVALID_SUPPLIER_ID;
    std::vector<sptr<NetSupplier>>::iterator iter;
    for (iter = suppliers.begin(); iter != suppliers.end(); ++iter) {
        if (defaultNetSupplier_->GetNetId() == (*iter)->GetNetId()) {
            supplierId = (*iter)->GetSupplierId();
            break;
        }
    }
    if (supplierId == INVALID_SUPPLIER_ID) {
        NETMGR_LOG_E("not found supplierId, default supplier id[%{public}d], netId:[%{public}d]",
            defaultNetSupplier_->GetSupplierId(), defaultNetSupplier_->GetNetId());
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    NetDetectionStatus state = isBetter ? QUALITY_GOOD_STATE : QUALITY_POOR_STATE;
    HandleDetectionResult(supplierId, state);
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
