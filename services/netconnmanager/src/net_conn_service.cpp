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
#include <unistd.h>
#include <arpa/inet.h>

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
    NETMGR_LOG_D("NetConnService::OnStart begin timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
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
    NETMGR_LOG_D("NetConnService::OnStart end timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
}

void NetConnService::CreateDefaultRequest()
{
    if (!defaultNetActivate_) {
        sptr<NetSpecifier> specifier = (std::make_unique<NetSpecifier>(" ",
            static_cast<uint32_t>(NET_TYPE_UNKNOWN), static_cast<uint64_t>(NET_CAPABILITIES_INTERNET))).release();
        defaultNetActivate_ = std::make_unique<NetActivate>(specifier, nullptr).release();
        uint32_t reqId = 0;
        netActivates_.insert(std::pair<uint32_t, sptr<NetActivate>>(reqId, defaultNetActivate_));
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

int32_t NetConnService::RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities,
    uint32_t &supplierId)
{
    NETMGR_LOG_D("register supplier, netType[%{public}d] ident[%{public}s] netCapabilities[%{public}" PRId64 "]",
        netType, ident.c_str(), netCapabilities);

    // According to netType, ident, get the supplier from the list and save the supplierId in the list
    if (netType >= NET_TYPE_MAX || netType <= NET_TYPE_UNKNOWN) {
        NETMGR_LOG_E("netType parameter invalid");
        return ERR_INVALID_NETORK_TYPE;
    }

    sptr<NetSupplier> supplier = GetNetSupplierFromList(netType, ident, netCapabilities);
    if (supplier != nullptr) {
        NETMGR_LOG_D("supplier already exists.");
        supplierId = supplier->GetSupplierId();
        return ERR_NONE;
    }

    // If there is no supplier in the list, create a supplier
    supplier = (std::make_unique<NetSupplier>(static_cast<NetworkType>(netType), ident, netCapabilities)).release();
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
    sptr<Network> network = (std::make_unique<Network>(netId, supplierId, std::bind(&NetConnService::HandleDetectionResult,
        this, std::placeholders::_1, std::placeholders::_2))).release();
    if (network == nullptr) {
        NETMGR_LOG_E("network is nullptr");
        return ERR_NO_NETWORK;
    }
    NETMGR_LOG_D("netId is: [%{public}d]", network->GetNetId());
    supplier->SetNetwork(network);

    RegisterSupplierCallback(supplier, netType, netCapabilities);

    // save supplier
    netSuppliers_.insert(std::pair<uint32_t, sptr<NetSupplier>>(supplierId, supplier));
    networks_.insert(std::pair<uint32_t, sptr<Network>>(netId, network));

    NETMGR_LOG_D("netSuppliers_ size[%{public}d]", static_cast<int32_t>(netSuppliers_.size()));
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

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        iterSupplier->second->RegisterNetConnCallback(callback);
    }

    return ERR_NONE;
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    if (netSpecifier->netType_ <= NET_TYPE_UNKNOWN || netSpecifier->netType_ >= NET_TYPE_MAX) {
        NETMGR_LOG_E("The parameter of net type is invalid");
        return ERR_INVALID_PARAMS;
    }

    auto iter = netConnCallback_.find(*netSpecifier);
    if (iter == netConnCallback_.end()) {
        std::vector<sptr<INetConnCallback>> callbackVec;
        callbackVec.emplace_back(callback);
        netConnCallback_.insert(std::make_pair(*netSpecifier, callbackVec));
    } else {
        if (std::find(iter->second.begin(), iter->second.end(), callback) == iter->second.end()) {
            iter->second.emplace_back(callback);
        }
    }

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        uint64_t netCapabilit = iterSupplier->second->GetNetCapabilities();
        if (iterSupplier->second->GetNetSupplierType() == netSpecifier->netType_ &&
            (netCapabilit & netSpecifier->netCapabilities_) == netCapabilit) {
            iterSupplier->second->RegisterNetConnCallback(callback);
            break;
        }
    }

    return ERR_NET_TYPE_NOT_FOUND;
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        iterSupplier->second->UnregisterNetConnCallback(callback);
    }

    return ERR_NONE;
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    if (netSpecifier->netType_ <= NET_TYPE_UNKNOWN || netSpecifier->netType_ >= NET_TYPE_MAX) {
        NETMGR_LOG_E("The parameter of net type is invalid");
        return ERR_INVALID_PARAMS;
    }

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        uint64_t netCapabilit = iterSupplier->second->GetNetCapabilities();
        if (iterSupplier->second->GetNetSupplierType() == netSpecifier->netType_ &&
            (netCapabilit & netSpecifier->netCapabilities_) == netCapabilit) {
            iterSupplier->second->UnregisterNetConnCallback(callback);
            break;
        }
    }

    return ERR_NET_TYPE_NOT_FOUND;
}

int32_t NetConnService::UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    NETMGR_LOG_I("Test NetConnService::UpdateNetStateForTest(), begin");
    if (netSpecifier == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        uint64_t netCapabilit = it->second->GetNetCapabilities();
        if (it->second->GetNetSupplierType() == netSpecifier->netType_
            && (netCapabilit & netSpecifier->netCapabilities_) == netCapabilit) {
            NETMGR_LOG_I("Test NetConnService::UpdateNetStateForTest(), find net service");
            it->second->UpdateNetStateForTest(static_cast<ServiceState>(netState));
            break;
        }
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
    }
    if (!netScore_->GetServiceScore(iterSupplier->second)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }
    FindBestNetworkForAllRequest();
    return ERR_NONE;
}

int32_t NetConnService::UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities)
{
    NETMGR_LOG_D("supplierId[%{public}d] netCapabilities[%{public}" PRId64 "]", supplierId,
        netCapabilities);
    // According to supplierId, get the supplier from the list
    NET_SUPPLIER_MAP::iterator iterSupplier = netSuppliers_.find(supplierId);
    if ((iterSupplier == netSuppliers_.end()) || (iterSupplier->second == nullptr)) {
        NETMGR_LOG_E("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }
    iterSupplier->second->UpdateNetCapabilities(netCapabilities);
    CallbackForSupplier(iterSupplier->second, CALL_TYPE_UPDATE_CAP);
    if (!netScore_->GetServiceScore(iterSupplier->second)) {
        NETMGR_LOG_E("GetServiceScore fail.");
    }
    FindBestNetworkForAllRequest();

    NETMGR_LOG_D("netSuppliers_ size[%{public}d], networks_ size[%{public}d]",
        static_cast<int32_t>(netSuppliers_.size()), static_cast<int32_t>(networks_.size()));
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
    NETMGR_LOG_D("Enter NetConnService::RegisterNetDetectionCallback netId[%{public}d]", netId);
    return RegUnRegNetDetectionCallback(netId, callback, true);
}

int32_t NetConnService::UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter NetConnService::UnRegisterNetDetectionCallback netId[%{public}d]", netId);
    return RegUnRegNetDetectionCallback(netId, callback, false);
}

int32_t NetConnService::NetDetection(int32_t netId)
{
    NETMGR_LOG_D("Enter NetConnService::NetDetection netId[%{public}d]", netId);
    sptr<Network> dectionNetwork = nullptr;
    NET_NETWORK_MAP::iterator iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network,start Piling test!");
        if (isNeedTestNetDetection_) {
            // create network test netDetection Piling test
            dectionNetwork = FindExitNetworkTestNetDetection();
        }
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

sptr<Network> NetConnService::FindExitNetworkTestNetDetection()
{
    sptr<Network> tmpNetwork = nullptr;
    if (!networks_.empty()) {
        tmpNetwork = networks_.begin()->second;
    }
    if (tmpNetwork == nullptr) {
        NETMGR_LOG_E("tmpNetwork is nullptr, Network is not exist in list!");
        return nullptr;
    }

    return tmpNetwork;
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
        NETMGR_LOG_E("Could not find the corresponding network,start Piling test!");
        if (isNeedTestNetDetection_) {
            // create network test netDetection Piling test
            dectionNetwork = FindExitNetworkTestNetDetection();
        }
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

sptr<NetSupplier> NetConnService::GetNetSupplierFromList(
    uint32_t netType, const std::string &ident)
{
    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        if ((netType == it->second->GetNetSupplierType())
            && (ident == it->second->GetNetSupplierIdent())) {
            return it->second;
        }
    }

    NETMGR_LOG_E("net supplier is nullptr");
    return nullptr;
}

sptr<NetSupplier> NetConnService::GetNetSupplierFromList(
    uint32_t netType, const std::string &ident, uint64_t capabilities)
{
    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        if ((netType == it->second->GetNetSupplierType())
            && (ident == it->second->GetNetSupplierIdent())
            && (capabilities == it->second->GetNetCapabilities())) {
            return it->second;
        }
    }

    return nullptr;
}

void NetConnService::RegisterSupplierCallback(sptr<NetSupplier> supplier, uint32_t netType, uint64_t netCapabilities)
{
    NetSpecifier spec;
    spec.netType_ = netType;
    spec.netCapabilities_ = netCapabilities;
    auto iter = netConnCallback_.find(spec);
    if (iter != netConnCallback_.end()) {
        supplier->RegisterNetConnCallback(iter->second);
    }

    iter = netConnCallback_.find(specAll_);
    if (iter != netConnCallback_.end()) {
        supplier->RegisterNetConnCallback(iter->second);
    }
}

int32_t NetConnService::ActivateNetwork(const sptr<NetSpecifier>& netSpecifier,
    const sptr<INetConnCallback>& callback, uint32_t& reqId)
{
    NETMGR_LOG_D("ActivateNetwork Enter");
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return ERR_INVALID_PARAMS;
    }
    sptr<NetActivate> request =(std::make_unique<NetActivate>(netSpecifier, callback)).release();
    reqId = request->GetRequestId();
    netActivates_.insert(std::pair<uint32_t, sptr<NetActivate>>(reqId, request));
    sptr<NetSupplier> bestNet = nullptr;
    int bestscore = FindBestNetworkForRequest(bestNet, request);
    if (bestscore != 0 && bestNet != nullptr) {
        NETMGR_LOG_D("The bestscore is: [%{public}d]", bestscore);
        bestNet->SelectAsBestNetwork(reqId);
        request->SetServiceSupply(bestNet);
        callback->NetAvailable(bestNet->GetNetId());
        return ERR_NONE;
    }

    NETMGR_LOG_D("not found the bestnet");
    SendRequestToAllNetwork(request);
    return ERR_NONE;
}

int32_t NetConnService::DeactivateNetwork(uint32_t& reqId)
{
    NETMGR_LOG_D("DeactivateNetwork Enter");
    NET_ACTIVATE_MAP::iterator iterActivate = netActivates_.find(reqId);
    if (iterActivate == netActivates_.end()) {
        NETMGR_LOG_E("not found the reqId: [%{public}d]", reqId);
        return ERR_NET_NOT_FIND_REQUEST_ID;
    }
    sptr<NetActivate> pNetActivate = iterActivate->second;
    if (pNetActivate) {
        sptr<NetSupplier> pNetService = pNetActivate->GetServiceSupply();
        if (pNetService) {
            pNetService->CancleRequest(reqId);
        }
    }

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        iterSupplier->second->CancleRequest(reqId);
    }
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
        return;
    }
    oldSupplier = newSupplier;
    newSupplier->SetDefault();
    return;
}

static void FreeAddrInfo(struct addrinfo *aihead)
{
    struct addrinfo *ai = nullptr;
    struct addrinfo *ainext = nullptr;
    for (ai = aihead; ai != nullptr; ai = ainext) {
        if (ai->ai_addr != nullptr) {
            free(ai->ai_addr);
            ai->ai_addr = nullptr;
        }
        if (ai->ai_canonname != nullptr) {
            free(ai->ai_canonname);
            ai->ai_canonname = nullptr;
        }
        ainext = ai->ai_next ;
        free(ai);
        if (ainext == nullptr) {
            return;
        }
    }
}

int32_t NetConnService::GetAddressesByName(const std::string &host, int32_t netId, std::list<INetAddr> &addrList)
{
    struct addrinfo hints;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_socktype = SOCK_DGRAM;
    std::unique_ptr<addrinfo> res;
    std::string server;
    int32_t ret = NetdController::GetInstance().GetAddrInfo(host, server, hints, res, netId);
    if (ret < 0) {
        return ERR_NO_ADDRESS;
    }
    struct addrinfo *cur = nullptr;
    struct addrinfo *resAddr = res.release();
    for (cur = resAddr; cur != nullptr; cur = cur->ai_next) {
        if (cur->ai_family == AF_INET) {
            char ipbuf[0x10];
            bzero(ipbuf, sizeof(ipbuf));
            auto addr = reinterpret_cast<struct sockaddr_in*>(cur->ai_addr);
            inet_ntop(AF_INET, &addr->sin_addr, ipbuf, 0x10);
            INetAddr iNetAddr;
            iNetAddr.family_ = cur->ai_family;
            iNetAddr.address_ = std::string(ipbuf);
            addrList.push_back(iNetAddr);
        }
    }
    if (resAddr != nullptr) {
        FreeAddrInfo(resAddr);
    }
    return ERR_NONE;
}

int32_t NetConnService::GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr)
{
    std::list<INetAddr> addrList;
    int ret = GetAddressesByName(host, netId, addrList);
    if (ret == ERR_NONE) {
        if (addrList.size() != 0) {
            addr = addrList.front();
            return ret;
        }
        return ERR_NO_ADDRESS;
    }
    return ret;
}

int32_t NetConnService::GetSpecificNet(uint32_t type, std::list<int32_t> &netIdList)
{
    if (type >= NET_TYPE_MAX || type <= NET_TYPE_UNKNOWN) {
        NETMGR_LOG_E("netType parameter invalid");
        return ERR_INVALID_NETORK_TYPE;
    }

    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        NetworkType supplierType = iterSupplier->second->GetNetSupplierType();
        if (static_cast<NetworkType>(type) == supplierType) {
            netIdList.push_back(iterSupplier->second->GetNetId());
        }
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}d] networks_ size[%{public}d]",
        static_cast<int32_t>(netSuppliers_.size()), static_cast<int32_t>(networks_.size()));
    return ERR_NONE;
}

int32_t NetConnService::GetAllNets(std::list<int32_t> &netIdList)
{
    for (auto p = networks_.begin(); p != networks_.end(); ++p) {
        netIdList.push_back(p->second->GetNetId());
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}d] networks_ size[%{public}d]",
        static_cast<int32_t>(netSuppliers_.size()), static_cast<int32_t>(networks_.size()));
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
        NETMGR_LOG_D("type is [%{public}d].", iterSupplier->second->GetNetSupplierType());
        if ((uid == iterSupplier->second->GetSupplierUid())
            && (iterSupplier->second->GetNetSupplierType() == NET_TYPE_VPN)) {
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

int32_t NetConnService::GetNetCapabilities(int32_t netId, uint64_t &cap)
{
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if (netId == iterSupplier->second->GetNetId()) {
            cap = iterSupplier->second->GetNetCapabilities();
            return ERR_NONE;
        }
    }
    return ERR_NO_NETWORK;
}

int32_t NetConnService::BindSocket(int32_t socket_fd, int32_t netId)
{
    return NetdController::GetInstance().BindSocket(socket_fd, netId);
}

void NetConnService::NotFindBestSupplier(uint32_t reqId, const sptr<NetActivate> &active,
    const sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback)
{
    SendRequestToAllNetwork(active);
    if (supplier != nullptr) {
        supplier->RemoveBestRequest(reqId);
        if (callback != nullptr) {
            callback->NetLost(supplier->GetNetId());
        }
    }
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
        if (callback) {
            callback->NetAvailable(bestSupplier->GetNetId());
            callback->NetCapabilitiesChange(bestSupplier->GetNetId(), bestSupplier->GetNetCapabilities());
            NetLinkInfo info = bestSupplier->GetNetLinkInfo();
            sptr<NetLinkInfo> pInfo = std::make_unique<NetLinkInfo>().release();
            pInfo = &(info);
            callback->NetConnectionPropertiesChange(bestSupplier->GetNetId(), pInfo);
        }
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
    NETMGR_LOG_D("now is not have request. size = %{public}d", static_cast<int32_t>(netActivates_.size()));
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
    NETMGR_LOG_D("bestReqList size = %{public}d", static_cast<int32_t>(bestReqList.size()));
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

        int32_t netId = supplier->GetNetId();
        switch (type) {
            case CALL_TYPE_LOST: {
                callback->NetLost(netId);
                break;
            }
            case CALL_TYPE_UPDATE_CAP: {
                callback->NetCapabilitiesChange(netId, supplier->GetNetCapabilities());
                break;
            }
            case CALL_TYPE_UPDATE_LINK: {
                NetLinkInfo info = supplier->GetNetLinkInfo();
                sptr<NetLinkInfo> pInfo = std::make_unique<NetLinkInfo>().release();
                pInfo = &(info);
                callback->NetConnectionPropertiesChange(netId, pInfo);
                break;
            }
            default:
                break;
        }
    }
    return;
}

int32_t NetConnService::GetIfaceNameByType(uint32_t netType, const std::string &ident, std::string &ifaceName)
{
    if (netType >= NET_TYPE_MAX || netType <= NET_TYPE_UNKNOWN) {
        NETMGR_LOG_E("netType parameter invalid");
        return ERR_INVALID_NETORK_TYPE;
    }

    sptr<NetSupplier> supplier = GetNetSupplierFromList(netType, ident);
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
    NET_SUPPLIER_MAP::iterator iterSupplier = netSuppliers_.find(supplierId);
    if ((iterSupplier == netSuppliers_.end()) || (iterSupplier->second == nullptr)) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return;
    }
    iterSupplier->second->SetNetValid(ifValid);
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
    std::map<std::string, std::string> param;
    DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);
    return 0;
}

int32_t NetConnService::ResetNetConnService()
{
    defaultNetSupplier_ = nullptr;
    netActivates_.clear();
    netSuppliers_.clear();
    networks_.clear();
    netConnCallback_.clear();
    defaultNetActivate_ = nullptr;
    CreateDefaultRequest();
    SetAirplaneMode(false);
    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
