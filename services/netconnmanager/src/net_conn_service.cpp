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
#include "net_conn_service.h"

#include <sys/time.h>

#include "system_ability_definition.h"
#include "common_event_support.h"

#include "broadcast_manager.h"
#include "net_conn_types.h"
#include "net_supplier.h"
#include "net_activate.h"
#include "netd_controller.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetConnService>::GetInstance().get());

NetConnService::NetConnService()
    : SystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    CreateDefaultRequest();
}

NetConnService::~NetConnService() {}

void NetConnService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    NETMGR_LOG_D("NetConnService::OnStart begin");
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
    NETMGR_LOG_D("NetConnService::OnStart end");
}

void NetConnService::CreateDefaultRequest()
{
    if (!defaultNetActivate_) {
        defaultNetSpecifier_ = (std::make_unique<NetSpecifier>()).release();
        defaultNetSpecifier_->SetCapability(NET_CAPABILITY_INTERNET);
        defaultNetActivate_ = std::make_unique<NetActivate>(defaultNetSpecifier_, nullptr,
            std::bind(&NetConnService::DeactivateNetwork, this, std::placeholders::_1), 0).release();
        defaultNetActivate_->SetRequestId(DEFAULT_REQUEST_ID);
        netActivates_.insert(std::pair<uint32_t, sptr<NetActivate>>(DEFAULT_REQUEST_ID, defaultNetActivate_));
    }
    return;
}

void NetConnService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool NetConnService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOG_E("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetConnService>::GetInstance().get())) {
            NETMGR_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    serviceIface_ = std::make_unique<NetConnServiceIface>().release();
    NetManagerCenter::GetInstance().RegisterConnService(serviceIface_);
    netScore_ = std::make_unique<NetScore>();
    if (netScore_ == nullptr) {
        NETMGR_LOG_E("Make NetScore failed");
        return false;
    }
    return true;
}

int32_t NetConnService::SystemReady()
{
    NETMGR_LOG_D("System ready.");
    return 0;
}

int32_t NetConnService::RegisterNetSupplier(
    NetBearType bearerType, const std::string &ident, const std::set<NetCap> &netCaps, uint32_t &supplierId)
{
    NETMGR_LOG_D("register supplier, netType[%{public}u] ident[%{public}s]",
        static_cast<uint32_t>(bearerType), ident.c_str());

    // According to netType, ident, get the supplier from the list and save the supplierId in the list
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return ERR_INVALID_NETORK_TYPE;
    }

    sptr<NetSupplier> supplier = GetNetSupplierFromList(bearerType, ident, netCaps);
    if (supplier != nullptr) {
        NETMGR_LOG_D("supplier already exists.");
        supplierId = supplier->GetSupplierId();
        return ERR_NONE;
    }

    // If there is no supplier in the list, create a supplier
    supplier = (std::make_unique<NetSupplier>(bearerType, ident, netCaps)).release();
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }
    supplierId = supplier->GetSupplierId();
    if (!netScore_->GetServiceScore(supplier)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }

    // create network
    int32_t netId = GenerateNetId();
    NETMGR_LOG_D("GenerateNetId is: [%{public}d]", netId);
    if (netId == INVALID_NET_ID) {
        NETMGR_LOG_E("GenerateNetId fail");
        return ERR_NO_NETWORK;
    }
    using namespace std::placeholders;
    sptr<Network> network = (std::make_unique<Network>(netId, supplierId,
        std::bind(&NetConnService::HandleDetectionResult, this, _1, _2))).release();
    if (network == nullptr) {
        NETMGR_LOG_E("network is nullptr");
        return ERR_NO_NETWORK;
    }
    NETMGR_LOG_D("netId is: [%{public}d]", network->GetNetId());
    supplier->SetNetwork(network);

    // save supplier
    netSuppliers_.insert(std::pair<uint32_t, sptr<NetSupplier>>(supplierId, supplier));
    networks_.insert(std::pair<uint32_t, sptr<Network>>(netId, network));

    NETMGR_LOG_D("netSuppliers_ size[%{public}zd]", netSuppliers_.size());
    return ERR_NONE;
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

int32_t NetConnService::UnregisterNetSupplier(uint32_t supplierId)
{
    NETMGR_LOG_D("UnregisterNetSupplier supplierId[%{public}d]", supplierId);
    // Remove supplier from the list based on supplierId
    NET_SUPPLIER_MAP::iterator iterSupplier = netSuppliers_.find(supplierId);
    if (iterSupplier == netSuppliers_.end()) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return ERR_NO_SUPPLIER;
    }

    int32_t netId = iterSupplier->second->GetNetId();
    NET_NETWORK_MAP::iterator iterNetwork = networks_.find(netId);
    if (iterNetwork != networks_.end()) {
        networks_.erase(iterNetwork);
    }
    netSuppliers_.erase(iterSupplier);
    NETMGR_LOG_D("Destory supplier network.");
    FindBestNetworkForAllRequest();

    return ERR_NONE;
}

int32_t NetConnService::RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return ERR_SERVICE_NULL_PTR;
    }
    std::map<uint32_t, sptr<NetSupplier>>::iterator iterSupplier = netSuppliers_.find(supplierId);
    if (iterSupplier == netSuppliers_.end()) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return ERR_NO_SUPPLIER;
    }
    iterSupplier->second->RegisterSupplierCallback(callback);
    SendAllRequestToNetwork(iterSupplier->second);
    return ERR_NONE;
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return ERR_SERVICE_NULL_PTR;
    }
    return RegisterNetConnCallback(defaultNetSpecifier_, callback, 0);
}

int32_t NetConnService::RegisterNetConnCallback(
    const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS)
{
    if (netActivates_.size() >= MAX_REQUEST_NUM) {
        NETMGR_LOG_E("Over the max request number");
        return ERR_NET_OVER_MAX_REQUEST_NUM;
    }
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return ERR_SERVICE_NULL_PTR;
    }
    uint32_t reqId = 0;
    if (FindSameCallback(callback, reqId)) {
        return ERR_REGISTER_THE_SAME_CALLBACK;
    }
    return ActivateNetwork(netSpecifier, callback, timeoutMS);
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    NETMGR_LOG_D("UnregisterNetConnCallback Enter");
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null");
        return ERR_SERVICE_NULL_PTR;
    }
    uint32_t reqId = 0;
    if (!FindSameCallback(callback, reqId)) {
        return ERR_UNREGISTER_CALLBACK_NOT_FOUND;
    }
    deleteNetActivates_.clear();
    return DeactivateNetwork(reqId);
}

bool NetConnService::FindSameCallback(const sptr<INetConnCallback> &callback, uint32_t &reqId)
{
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
            return true;
        }
    }
    return false;
}

int32_t NetConnService::UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    NETMGR_LOG_I("Test NetConnService::UpdateNetStateForTest(), begin");
    if (netSpecifier == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return ERR_SERVICE_NULL_PTR;
    }
    return ERR_NONE;
}

int32_t NetConnService::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    NETMGR_LOG_D("Update supplier info: supplierId[%{public}d]", supplierId);
    if (netSupplierInfo == nullptr) {
        NETMGR_LOG_E("netSupplierInfo is nullptr");
        return ERR_INVALID_PARAMS;
    }

    NETMGR_LOG_D("Update supplier info: netSupplierInfo[%{public}s]", netSupplierInfo->ToString("").c_str());

    // According to supplierId, get the supplier from the list
    NET_SUPPLIER_MAP::iterator iterSupplier = netSuppliers_.find(supplierId);
    if ((iterSupplier == netSuppliers_.end()) || (iterSupplier->second == nullptr)) {
        NETMGR_LOG_E("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }

    iterSupplier->second->UpdateNetSupplierInfo(*netSupplierInfo);

    if (!netSupplierInfo->isAvailable_) {
        CallbackForSupplier(iterSupplier->second, CALL_TYPE_LOST);
    } else {
        CallbackForSupplier(iterSupplier->second, CALL_TYPE_UPDATE_CAP);
    }
    if (!netScore_->GetServiceScore(iterSupplier->second)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }
    FindBestNetworkForAllRequest();
    return ERR_NONE;
}

int32_t NetConnService::RestrictBackgroundChanged(bool restrictBackground)
{
    NETMGR_LOG_D("NetConnService::RestrictBackgroundChanged restrictBackground = %{public}d", restrictBackground);
    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        if (it->second->GetRestrictBackground() == restrictBackground) {
            NETMGR_LOG_D("it->second->GetRestrictBackground() == restrictBackground");
            return ERR_NET_NO_RESTRICT_BACKGROUND;
        }

        if (it->second->GetNetSupplierType() == BEARER_VPN) {
            CallbackForSupplier(it->second, CALL_TYPE_BLOCK_STATUS);
        }
        it->second->SetRestrictBackground(restrictBackground);
    }
    return ERR_NONE;
}

int32_t NetConnService::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    NETMGR_LOG_D("supplierId[%{public}d]", supplierId);
    if (netLinkInfo == nullptr) {
        NETMGR_LOG_E("netLinkInfo is nullptr");
        return ERR_INVALID_PARAMS;
    }

    NET_SUPPLIER_MAP::iterator iterSupplier = netSuppliers_.find(supplierId);
    if ((iterSupplier == netSuppliers_.end()) || (iterSupplier->second == nullptr)) {
        NETMGR_LOG_E("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }
    // According to supplier id, get network from the list
    if (iterSupplier->second->UpdateNetLinkInfo(*netLinkInfo) != ERR_SERVICE_UPDATE_NET_LINK_INFO_SUCCES) {
        NETMGR_LOG_E("UpdateNetLinkInfo fail");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    CallbackForSupplier(iterSupplier->second, CALL_TYPE_UPDATE_LINK);
    if (!netScore_->GetServiceScore(iterSupplier->second)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }
    FindBestNetworkForAllRequest();
    return ERR_NONE;
}

int32_t NetConnService::RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter NetConnService::RegisterNetDetectionCallback");
    return RegUnRegNetDetectionCallback(netId, callback, true);
}

int32_t NetConnService::UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter NetConnService::UnRegisterNetDetectionCallback");
    return RegUnRegNetDetectionCallback(netId, callback, false);
}

int32_t NetConnService::NetDetection(int32_t netId)
{
    NETMGR_LOG_D("Enter NetConnService::NetDetection");
    sptr<Network> dectionNetwork = nullptr;
    NET_NETWORK_MAP::iterator iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network.");
    } else {
        dectionNetwork = iterNetwork->second;
    }

    if (dectionNetwork == nullptr) {
        NETMGR_LOG_E("Network is not find, need register!");
        return ERR_NET_NOT_FIND_NETID;
    }
    dectionNetwork->SetExternDetection();
    dectionNetwork->StartNetDetection();
    return ERR_NONE;
}

int32_t NetConnService::RegUnRegNetDetectionCallback(
    int32_t netId, const sptr<INetDetectionCallback> &callback, bool isReg)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    sptr<Network> dectionNetwork = nullptr;
    NET_NETWORK_MAP::iterator iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network.");
    } else {
        dectionNetwork = iterNetwork->second;
    }

    if (dectionNetwork == nullptr) {
        NETMGR_LOG_E("Network is not find, need register!");
        return ERR_NET_NOT_FIND_NETID;
    }
    if (isReg) {
        dectionNetwork->RegisterNetDetectionCallback(callback);
        return ERR_NONE;
    } else {
        return dectionNetwork->UnRegisterNetDetectionCallback(callback);
    }
}

sptr<NetSupplier> NetConnService::GetNetSupplierFromList(NetBearType bearerType, const std::string &ident)
{
    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        if ((bearerType == it->second->GetNetSupplierType()) && (ident == it->second->GetNetSupplierIdent())) {
            return it->second;
        }
    }

    NETMGR_LOG_E("net supplier is nullptr");
    return nullptr;
}

sptr<NetSupplier> NetConnService::GetNetSupplierFromList(
    NetBearType bearerType, const std::string &ident, const std::set<NetCap> &netCaps)
{
    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        if ((bearerType == it->second->GetNetSupplierType()) && (ident == it->second->GetNetSupplierIdent()) &&
            (netCaps == it->second->GetNetCaps())) {
            return it->second;
        }
    }

    return nullptr;
}

int32_t NetConnService::ActivateNetwork(const sptr<NetSpecifier>& netSpecifier,
    const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS)
{
    NETMGR_LOG_D("ActivateNetwork Enter");
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return ERR_INVALID_PARAMS;
    }
    sptr<NetActivate> request =(std::make_unique<NetActivate>(netSpecifier, callback,
        std::bind(&NetConnService::DeactivateNetwork, this, std::placeholders::_1), timeoutMS)).release();
    uint32_t reqId = request->GetRequestId();
    NETMGR_LOG_D("ActivateNetwork  reqId is [%{public}d]", reqId);
    netActivates_.insert(std::pair<uint32_t, sptr<NetActivate>>(reqId, request));
    sptr<NetSupplier> bestNet = nullptr;
    int bestscore = FindBestNetworkForRequest(bestNet, request);
    if (bestscore != 0 && bestNet != nullptr) {
        NETMGR_LOG_D("The bestscore is: [%{public}d]", bestscore);
        bestNet->SelectAsBestNetwork(reqId);
        request->SetServiceSupply(bestNet);
        CallbackForAvailable(bestNet, callback);
        return ERR_NONE;
    }

    NETMGR_LOG_D("not found the bestnet");
    SendRequestToAllNetwork(request);
    deleteNetActivates_.clear();
    return ERR_NONE;
}

int32_t NetConnService::DeactivateNetwork(uint32_t reqId)
{
    NETMGR_LOG_D("DeactivateNetwork Enter, reqId is [%{public}d]", reqId);
    NET_ACTIVATE_MAP::iterator iterActivate = netActivates_.find(reqId);
    if (iterActivate == netActivates_.end()) {
        NETMGR_LOG_E("not found the reqId: [%{public}d]", reqId);
        return ERR_NET_NOT_FIND_REQUEST_ID;
    }
    sptr<NetActivate> pNetActivate = iterActivate->second;
    if (pNetActivate) {
        sptr<NetSupplier> pNetService = pNetActivate->GetServiceSupply();
        if (pNetService) {
            pNetService->CancelRequest(reqId);
        }
    }

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        iterSupplier->second->CancelRequest(reqId);
    }
    deleteNetActivates_.insert(std::pair<uint32_t, sptr<NetActivate>>(reqId, pNetActivate));
    netActivates_.erase(reqId);
    return ERR_NONE;
}

int32_t NetConnService::GetDefaultNet(int32_t& netId)
{
    NETMGR_LOG_D("GetDefaultNet Enter");
    if (!defaultNetSupplier_) {
        NETMGR_LOG_E("not found the netId");
        return ERR_NET_NOT_FIND_NETID;
    }

    netId = defaultNetSupplier_->GetNetId();
    NETMGR_LOG_D("found the netId: [%{public}d]", netId);
    return ERR_NONE;
}

int32_t NetConnService::HasDefaultNet(bool& flag)
{
    NETMGR_LOG_D("HasDefaultNet Enter");
    if (!defaultNetSupplier_) {
        flag = false;
        return ERR_NET_DEFAULTNET_NOT_EXIST;
    }
    flag = true;
    return ERR_NONE;
}

void NetConnService::MakeDefaultNetWork(sptr<NetSupplier>& oldSupplier, sptr<NetSupplier>& newSupplier)
{
    NETMGR_LOG_D("MakeDefaultNetWork Enter");
    if (oldSupplier == newSupplier) {
        return;
    }
    if (oldSupplier && !newSupplier) {
        oldSupplier->ClearDefault();
        oldSupplier = newSupplier;
        return;
    }
    oldSupplier = newSupplier;
    newSupplier->SetDefault();
    return;
}

int32_t NetConnService::GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList)
{
    NETMGR_LOG_D("Enter GetAddressesByName");
    return NetManagerCenter::GetInstance().GetAddressesByName(host, static_cast<uint16_t>(netId), addrList);
}

int32_t NetConnService::GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr)
{
    NETMGR_LOG_D("Enter GetAddressByName");
    std::vector<INetAddr> addrList;
    int ret = GetAddressesByName(host, netId, addrList);
    if (ret == ERR_NONE) {
        if (!addrList.empty()) {
            addr = addrList[0];
            return ret;
        }
        return ERR_NO_ADDRESS;
    }
    return ret;
}

int32_t NetConnService::GetSpecificNet(NetBearType bearerType, std::list<int32_t> &netIdList)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return ERR_INVALID_NETORK_TYPE;
    }

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        auto supplierType = iterSupplier->second->GetNetSupplierType();
        if (bearerType == supplierType) {
            netIdList.push_back(iterSupplier->second->GetNetId());
        }
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}zd] networks_ size[%{public}zd]", netSuppliers_.size(), networks_.size());
    return ERR_NONE;
}

int32_t NetConnService::GetAllNets(std::list<int32_t> &netIdList)
{
    for (auto p = networks_.begin(); p != networks_.end(); ++p) {
        netIdList.push_back(p->second->GetNetId());
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}zd] networks_ size[%{public}zd]", netSuppliers_.size(), networks_.size());
    return ERR_NONE;
}

int32_t NetConnService::GetSpecificUidNet(int32_t uid, int32_t &netId)
{
    NETMGR_LOG_D("Enter GetSpecificUidNet.");
    NETMGR_LOG_D("uid is [%{public}d].", uid);
    netId = INVALID_NET_ID;
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        NETMGR_LOG_D("uid is [%{public}d].", iterSupplier->second->GetSupplierUid());
        NETMGR_LOG_D("type is [%{public}d].", static_cast<int32_t>(iterSupplier->second->GetNetSupplierType()));
        if ((uid == iterSupplier->second->GetSupplierUid())
            && (iterSupplier->second->GetNetSupplierType() == BEARER_VPN)) {
            netId = iterSupplier->second->GetNetId();
            return ERR_NONE;
        }
    }
    NETMGR_LOG_D("No vpn, run GetDefaultNet.");
    return GetDefaultNet(netId);
}

int32_t NetConnService::GetConnectionProperties(int32_t netId, NetLinkInfo &info)
{
    NET_NETWORK_MAP::iterator iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        return ERR_NO_NETWORK;
    }

    info = iterNetwork->second->GetNetLinkInfo();
    return ERR_NONE;
}

int32_t NetConnService::GetNetCapabilities(int32_t netId, NetAllCapabilities &netAllCap)
{
    NETMGR_LOG_D("Enter GetNetCapabilities.");
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if (netId == iterSupplier->second->GetNetId()) {
            netAllCap = iterSupplier->second->GetNetCapabilities();
            return ERR_NONE;
        }
    }
    NETMGR_LOG_E("no network.");
    return ERR_NO_NETWORK;
}

int32_t NetConnService::BindSocket(int32_t socket_fd, int32_t netId)
{
    NETMGR_LOG_D("Enter BindSocket.");
    return NetdController::GetInstance().BindSocket(socket_fd, netId);
}

void NetConnService::NotFindBestSupplier(uint32_t reqId, const sptr<NetActivate> &active,
    const sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback)
{
    if (supplier != nullptr) {
        supplier->RemoveBestRequest(reqId);
        if (callback != nullptr) {
            sptr<NetHandle> netHandle = supplier->GetNetHandle();
            callback->NetLost(netHandle);
        }
    }
    active->SetServiceSupply(nullptr);
    SendRequestToAllNetwork(active);
}

void NetConnService::FindBestNetworkForAllRequest()
{
    NETMGR_LOG_D("FindBestNetworkForAllRequest Enter");
    NET_ACTIVATE_MAP::iterator iterActive;
    sptr<NetSupplier> bestSupplier = nullptr;
    for (iterActive = netActivates_.begin(); iterActive != netActivates_.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        int score = FindBestNetworkForRequest(bestSupplier, iterActive->second);
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

        if (bestSupplier == oldSupplier) {
            continue;
        }
        if (oldSupplier) {
            oldSupplier->RemoveBestRequest(iterActive->first);
        }
        iterActive->second->SetServiceSupply(bestSupplier);
        CallbackForAvailable(bestSupplier, callback);
        bestSupplier->SelectAsBestNetwork(iterActive->first);
        SendBestScoreAllNetwork(iterActive->first, score);
    }
}

uint32_t NetConnService::FindBestNetworkForRequest(sptr<NetSupplier> &supplier, sptr<NetActivate> &netActivateNetwork)
{
    NETMGR_LOG_E("FindBestNetworkForRequest Enter");
    int bestScore = 0;
    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if ((!netActivateNetwork->MatchRequestAndNetwork(iter->second)) ||
            (!iter->second->IsConnected())) {
            NETMGR_LOG_D("supplier is not connected");
            continue;
        }
        int score = iter->second->GetRealScore();
        NETMGR_LOG_D("score : [%{public}d]", score);
        if (score > bestScore) {
            bestScore = score;
            supplier = iter->second;
        }
    }
    NETMGR_LOG_E("the bestscore is: [%{public}d]", bestScore);
    return bestScore;
}

void NetConnService::SendAllRequestToNetwork(sptr<NetSupplier> supplier)
{
    NETMGR_LOG_E("SendAllRequestToNetwork Enter");
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is null");
        return;
    }
    NET_ACTIVATE_MAP::iterator iter;
    NETMGR_LOG_D("now is not have request. size = %{public}zd", netActivates_.size());
    for (iter = netActivates_.begin(); iter != netActivates_.end(); ++iter) {
        if (!iter->second->MatchRequestAndNetwork(supplier)) {
            continue;
        }
        bool result = supplier->RequestToConnect(iter->first);
        if (!result) {
            NETMGR_LOG_E("connect supplier failed, result: %{public}d", result);
        }
    }
    return;
}

void NetConnService::SendRequestToAllNetwork(sptr<NetActivate> request)
{
    NETMGR_LOG_D("SendRequestToAllNetwork Enter");
    if (request == nullptr) {
        NETMGR_LOG_E("request is null");
        return;
    }

    uint32_t reqId = request->GetRequestId();
    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (!request->MatchRequestAndNetwork(iter->second)) {
            continue;
        }
        bool result = iter->second->RequestToConnect(reqId);
        if (!result) {
            NETMGR_LOG_E("connect service failed, result %{public}d", result);
        }
    }
    return;
}

void NetConnService::SendBestScoreAllNetwork(uint32_t reqId, int32_t bestScore)
{
    NETMGR_LOG_E("SendBestScoreAllNetwork Enter");
    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        iter->second->ReceiveBestScore(reqId, bestScore);
    }
}

void NetConnService::CallbackForSupplier(sptr<NetSupplier>& supplier, CallbackType type)
{
    NETMGR_LOG_D("NetConnService::CallbackForSupplier Enter");
    if (supplier == nullptr) {
        return;
    }
    std::set<uint32_t>& bestReqList = supplier->GetBestRequestList();
    NETMGR_LOG_D("bestReqList size = %{public}zd", bestReqList.size());
    for (auto it = bestReqList.begin(); it != bestReqList.end(); it++) {
        auto reqIt = netActivates_.find(*it);
        if ((reqIt == netActivates_.end()) || (!reqIt->second)) {
            NETMGR_LOG_D("netActivates_ not find reqId : %{public}d", *it);
            continue;
        }
        sptr<INetConnCallback> callback = reqIt->second->GetNetCallback();
        if (!callback) {
            NETMGR_LOG_D("netActivite->callback is nullptr");
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
                *pInfo = supplier->GetNetLinkInfo();
                callback->NetConnectionPropertiesChange(netHandle, pInfo);
                break;
            }
            case CALL_TYPE_BLOCK_STATUS: {
                std::set<NetCap> netCaps = supplier->GetNetCaps();
                bool Metered = (netCaps.find(NET_CAPABILITY_NOT_METERED) != netCaps.end());
                bool newBlocked = NetManagerCenter::GetInstance().IsUidNetAccess(supplier->GetSupplierUid(), Metered);
                callback->NetBlockStatusChange(netHandle, newBlocked);
                break;
            }
            default:
                break;
        }
    }
    return;
}

void NetConnService::CallbackForAvailable(sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback)
{
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
    *pInfo = supplier->GetNetLinkInfo();
    callback->NetConnectionPropertiesChange(netHandle, pInfo);
    return;
}

int32_t NetConnService::GetIfaceNameByType(NetBearType bearerType, const std::string &ident, std::string &ifaceName)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return ERR_INVALID_NETORK_TYPE;
    }

    sptr<NetSupplier> supplier = GetNetSupplierFromList(bearerType, ident);
    if (supplier == nullptr) {
        NETMGR_LOG_D("supplier is nullptr.");
        return ERR_NO_SUPPLIER;
    }

    sptr<Network> network = supplier->GetNetwork();
    if (network == nullptr) {
        NETMGR_LOG_E("network is nullptr");
        return ERR_NO_NETWORK;
    }

    ifaceName = network->GetNetLinkInfo().ifaceName_;

    return ERR_NONE;
}

void NetConnService::HandleDetectionResult(uint32_t supplierId, bool ifValid)
{
    NETMGR_LOG_D("Enter HandleDetectionResult");
    NET_SUPPLIER_MAP::iterator iterSupplier = netSuppliers_.find(supplierId);
    if ((iterSupplier == netSuppliers_.end()) || (iterSupplier->second == nullptr)) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return;
    }
    iterSupplier->second->SetNetValid(ifValid);
    CallbackForSupplier(iterSupplier->second, CALL_TYPE_UPDATE_CAP);
    if (!netScore_->GetServiceScore(iterSupplier->second)) {
        NETMGR_LOG_E("GetServiceScore fail.");
        return;
    }
    FindBestNetworkForAllRequest();
    return;
}

int32_t NetConnService::SetAirplaneMode(bool state)
{
    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED;
    info.data = "Net Manager Airplane Mode Changed";
    info.code = static_cast<int32_t>(state);
    info.ordered = true;
    std::map<std::string, int32_t> param;
    DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);
    return 0;
}

int32_t NetConnService::RestoreFactoryData()
{
    NetManagerCenter::GetInstance().ResetEthernetFactory();
    NetManagerCenter::GetInstance().ResetPolicyFactory();
    NetManagerCenter::GetInstance().ResetStatsFactory();
    defaultNetSupplier_ = nullptr;
    netActivates_.clear();
    NETMGR_LOG_D("Reset NetConnService, clear network request complete.");
    netSuppliers_.clear();
    networks_.clear();
    NETMGR_LOG_D("Reset NetConnService, clear registered network complete.");
    defaultNetSpecifier_ = nullptr;
    defaultNetActivate_ = nullptr;
    CreateDefaultRequest();
    NETMGR_LOG_D("Reset NetConnService, default network complete.");
    SetAirplaneMode(false);
    NETMGR_LOG_D("Reset NetConnService, turn off airplane mode.");
    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
