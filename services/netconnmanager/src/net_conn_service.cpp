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
#include "net_conn_service.h"

#include "system_ability_definition.h"
#include "common_event_support.h"

#include "broadcast_manager.h"
#include "net_conn_types.h"
#include "netsys_controller.h"
#include "net_conn_service_iface.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_permission.h"
#include "scheduler.h"

static constexpr int NET_CONN_BLOCK_TIMEOUT_MS = 2000;

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetConnService>::GetInstance().get());
NetConnService::NetConnService()
    :SystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, true),
    asyncThread_([&]() {GetScheduler().Run();})
{
    CreateDefaultRequest();
}

NetConnService::~NetConnService()
{
    GetScheduler().Stop();
    if (asyncThread_.joinable()) {
        asyncThread_.join();
    }
}

void NetConnService::OnStart()
{
    sptr<NetConnServiceIface> iface = new NetConnServiceIface;
    NetManagerCenter::GetInstance().RegisterConnService(iface);
    if (!Publish(DelayedSingleton<NetConnService>::GetInstance().get())) {
        return;
    }
}

void NetConnService::OnStop()
{
}

int32_t NetConnService::SystemReady()
{
    return 0;
}

int32_t NetConnService::RegisterNetSupplier(NetBearType bearerType,
                                            const std::string &ident,
                                            const std::set<NetCap> &netCaps,
                                            uint32_t &supplierId)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
            return ERR_INVALID_NETORK_TYPE;
        }

        auto suppliers = FindNetSuppliersByInfo(bearerType, ident);
        if (!suppliers.empty()) {
            supplierId = suppliers.front()->GetId();
            return ERR_NONE;
        }
        auto supplier = CreateNetSupplier(bearerType, ident, netCaps);
        supplierId = supplier->GetId();

        return ERR_NONE;
    });
}

int32_t NetConnService::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (netSupplierInfo == nullptr) {
            return ERR_INVALID_PARAMS;
        }

        auto supplier = FindNetSupplier(supplierId);
        if (!supplier) {
            NETMGR_LOG_W("Supplier with id[%{public}d] not found", supplierId);
            return ERR_NO_SUPPLIER;
        } else {
            supplier->UpdateNetSupplierInfo(netSupplierInfo);
            return ERR_NONE;
        }
    });
}

int32_t NetConnService::RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (callback == nullptr) {
            return ERR_INVALID_PARAMS;
        }
        auto supplier = FindNetSupplier(supplierId);
        if (!supplier) {
            return ERR_NO_SUPPLIER;
        }
        supplier->SetSupplierCallback(callback);
        return ERR_NONE;
    });
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    return InvokeMethodSafety([&]() ->int32_t {
        sptr<NetSpecifier> netSpecifier = new NetSpecifier;
        netSpecifier->SetCapability(NET_CAPABILITY_INTERNET);
        return RegisterNetConnCallback(netSpecifier, callback, 0);
    });
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
                                                const sptr<INetConnCallback> &callback,
                                                const uint32_t &timeoutMS)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }

        if (netSpecifier == nullptr || callback == nullptr) {
            return ERR_INVALID_PARAMS;
        }

        if (FindNetRequestByCallback(callback)) {
            return ERR_REGISTER_THE_SAME_CALLBACK;
        }

        CreateNetRequest(netSpecifier, callback, timeoutMS);

        return ERR_NONE;
    });
}

int32_t NetConnService::UnregisterNetSupplier(uint32_t supplierId)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (RemoveNetSupplier(supplierId)) {
            return ERR_NONE;
        }
        return ERR_INVALID_PARAMS;
    });
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            return ERR_PERMISSION_CHECK_FAIL;
        }

        auto request = FindNetRequestByCallback(callback);
        if (!request) {
            return ERR_INVALID_PARAMS;
        }
        RemoveNetRequest(request);
        return ERR_NONE;
    });
}

int32_t NetConnService::UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    return ERR_NONE;
}

int32_t NetConnService::RestrictBackgroundChanged(bool restrictBackground)
{
    return ERR_NONE;
}

int32_t NetConnService::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (netLinkInfo == nullptr) {
            return ERR_INVALID_PARAMS;
        }

        auto supplier = FindNetSupplier(supplierId);
        if (!supplier) {
            return ERR_NO_SUPPLIER;
        }

        supplier->UpdateNetLinkInfo(netLinkInfo);

        return ERR_NONE;
    });
}

int32_t NetConnService::RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (callback == nullptr) {
            return ERR_INVALID_PARAMS;
        }
        auto supplier = FindNetSupplierByNetId(netId);
        if (supplier) {
            supplier->RegisterNetDetectionCallback(callback);
            return ERR_NONE;
        } else {
            return ERR_NO_NETWORK;
        }
    });
}

int32_t NetConnService::UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (callback == nullptr) {
            return ERR_INVALID_PARAMS;
        }
        auto supplier = FindNetSupplierByNetId(netId);
        if (supplier) {
            supplier->UnregisterNetDetectionCallback(callback);
            return ERR_NONE;
        } else {
            return ERR_NO_NETWORK;
        }
    });
}

int32_t NetConnService::NetDetection(int32_t netId)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO) ||
            !NetManagerPermission::CheckPermission(Permission::INTERNET)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }
        auto supplier = FindNetSupplierByNetId(netId);
        if (supplier == nullptr) {
            return ERR_NO_SUPPLIER;
        }
        supplier->GetNetMonitor()->Restart();
        return ERR_NONE;
    });
}

int32_t NetConnService::GetDefaultNet(int32_t &netId)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }

        if (!defaultNetSupplier_) {
            return ERR_NET_DEFAULTNET_NOT_EXIST;
        }

        netId = defaultNetSupplier_->GetNetId();
        return ERR_NONE;
    });
}

int32_t NetConnService::HasDefaultNet(bool &flag)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }
        if (defaultNetSupplier_) {
            flag = true;
        } else {
            flag = false;
        }
        return ERR_NONE;
    });
}

int32_t NetConnService::GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }
        return NetManagerCenter::GetInstance().GetAddressesByName(host, static_cast<uint16_t>(netId), addrList);
    });
}

int32_t NetConnService::GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }
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
    });
}

int32_t NetConnService::GetSpecificNet(NetBearType bearType, std::list<int32_t> &netIdList)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (bearType < BEARER_CELLULAR || bearType >= BEARER_DEFAULT) {
            return ERR_INVALID_NETORK_TYPE;
        }

        auto suppliers = FindNetSuppliersByInfo(bearType);
        for (const auto &supplier : suppliers) {
            netIdList.push_back(supplier->GetNetId());
        }

        return ERR_NONE;
    });
}

int32_t NetConnService::GetAllNets(std::list<int32_t> &netIdList)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }

        for (const auto &p : netSuppliers_) {
            netIdList.push_back(p.second->GetNetId());
        }

        return ERR_NONE;
    });
}

int32_t NetConnService::GetSpecificUidNet(int32_t uid, int32_t &netId)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (defaultNetSupplier_) {
            netId = defaultNetSupplier_->GetNetId();
            return ERR_NONE;
        } else {
            return ERR_NO_NETWORK;
        }
    });
}

int32_t NetConnService::GetConnectionProperties(int32_t netId, NetLinkInfo &info)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }

        auto netSupplier = FindNetSupplierByNetId(netId);
        if (netSupplier) {
            info = *netSupplier->GetNetLinkInfo();
            return ERR_NONE;
        } else {
            return ERR_NO_NETWORK;
        }
    });
}

int32_t NetConnService::GetNetCapabilities(int32_t netId, NetAllCapabilities &netAllCap)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            NETMGR_LOG_W("Permission check failed");
            return ERR_PERMISSION_CHECK_FAIL;
        }

        auto supplier = FindNetSupplierByNetId(netId);
        if (!supplier) {
            return ERR_NO_NETWORK;
        }

        netAllCap = *(supplier->GetNetAllCapabilities());
        return ERR_NONE;
    });
}

int32_t NetConnService::GetIfaceNames(NetBearType bearerType, std::list<std::string> &ifaceNames)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
            return ERR_INVALID_NETORK_TYPE;
        }

        auto suppliers = FindNetSuppliersByInfo(bearerType);
        for (auto supplier : suppliers) {
            std::string ifaceName = supplier->GetNetLinkInfo()->ifaceName_;
            if (!ifaceName.empty()) {
                ifaceNames.push_back(ifaceName);
            }
        }
        return ERR_NONE;
    });
}

int32_t NetConnService::GetIfaceNameByType(NetBearType bearerType, const std::string &ident, std::string &ifaceName)
{
    return InvokeMethodSafety([&]() ->int32_t {
        if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
            return ERR_INVALID_NETORK_TYPE;
        }

        auto suppliers = FindNetSuppliersByInfo(bearerType, ident);
        if (suppliers.empty()) {
            return ERR_NO_SUPPLIER;
        }
        auto supplier = suppliers.front();
        ifaceName = supplier->GetNetLinkInfo()->ifaceName_;

        return ERR_NONE;
    });
}

int32_t NetConnService::SetAirplaneMode(bool state)
{
    return InvokeMethodSafety([&]() ->int32_t {
        NETMGR_LOG_I("Broadcast air plane mode changed[%{public}s]", state ? "true" : "false");
        BroadcastInfo info;
        info.action = EventFwk::CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED;
        info.data = "Net Manager Airplane Mode Changed";
        info.code = static_cast<int32_t>(state);
        info.ordered = true;
        std::map<std::string, int32_t> param;
        DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);
        return ERR_NONE;
    });
}

int32_t NetConnService::RestoreFactoryData()
{
    return InvokeMethodSafety([&]() ->int32_t {
        NETMGR_LOG_I("Restore factory data begin");
        NetManagerCenter::GetInstance().ResetEthernetFactory();
        NetManagerCenter::GetInstance().ResetPolicyFactory();
        NetManagerCenter::GetInstance().ResetStatsFactory();
        defaultNetSupplier_ = nullptr;
        netSuppliers_.clear();
        netRequests_.clear();
        CreateDefaultRequest();
        SetAirplaneMode(false);
        NETMGR_LOG_I("Restore factory data end");
        return ERR_NONE;
    });
}

sptr<NetSupplier> NetConnService::CreateNetSupplier(NetBearType bearerType,
                                                    const std::string &ident,
                                                    const std::set<NetCap>& caps)
{
    std::string netCapsStr;
    for (auto cap : caps) {
        netCapsStr += std::to_string(cap) + ",";
    }
    NETMGR_LOG_I("bearerType[%{public}d], ident[%{public}s], netCaps[%{public}s]",
        bearerType, ident.c_str(), netCapsStr.c_str());

    sptr<NetSupplier> supplier = new NetSupplier(bearerType, ident, caps, *this);
    netSuppliers_[supplier->GetId()] = supplier;
    return supplier;
}

sptr<NetSupplier> NetConnService::FindNetSupplier(uint32_t supplierId)
{
    auto iter = netSuppliers_.find(supplierId);
    if (iter != netSuppliers_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::list<sptr<NetSupplier>> NetConnService::FindNetSuppliersByInfo(NetBearType bearerType, const std::string &ident)
{
    std::list<sptr<NetSupplier>> ret;
    for (auto p : netSuppliers_) {
        auto supplier = p.second;
        if (bearerType != BEARER_DEFAULT && supplier->GetBearerType() != bearerType) {
            continue;
        }
        if (!ident.empty() && supplier->GetIdent() != ident) {
            continue;
        }
        ret.push_back(supplier);
    }
    return ret;
}

sptr<NetSupplier> NetConnService::FindNetSupplierByNetId(uint32_t netId)
{
    for (auto p : netSuppliers_) {
        const auto &supplier = p.second;
        if (supplier->GetNetwork()->GetId() == netId) {
            return supplier;
        }
    }
    return nullptr;
}

sptr<NetRequest> NetConnService::CreateNetRequest(sptr<NetSpecifier> netSpecifier,
                                                  sptr<INetConnCallback> callback,
                                                  uint32_t timeoutMs)
{
    NETMGR_LOG_I("%{public}s", netSpecifier->ToString(" ").c_str());
    sptr<NetRequest> request = new NetRequest(netSpecifier, callback, timeoutMs, *this);

    auto sameRequests = FindNetRequestsBySameSpecifier(*netSpecifier);
    sptr<NetSupplier> supplier;
    if (!sameRequests.empty()) {
        supplier = FindNetSupplier(sameRequests.front()->GetNetSupplierId());
    } else {
        supplier = GetBestNetworkForRequest(request);
    }
    
    if (supplier) {
        supplier->AddNetRequest(request);
    }
    netRequests_[request->GetId()] = request;
    return request;
}

void NetConnService::CreateDefaultRequest()
{
    sptr<NetSpecifier> netSpecifier = new NetSpecifier;
    netSpecifier->SetCapability(NET_CAPABILITY_INTERNET);
    defaultNetRequest_ = CreateNetRequest(netSpecifier, nullptr, 0);
    NETMGR_LOG_I("Default net request created, id=%{public}d", defaultNetRequest_->GetId());
}

sptr<NetRequest> NetConnService::FindNetRequest(uint32_t reqId)
{
    auto iter = netRequests_.find(reqId);
    if (iter != netRequests_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::list<sptr<NetRequest>> NetConnService::FindNetRequestsBySameSpecifier(const NetSpecifier &netSpecifier)
{
    std::list<sptr<NetRequest>> ret;
    for (auto p : netRequests_) {
        const auto &request = p.second;
        if (request->GetNetSpecifier()->ident_ == netSpecifier.ident_ &&
            request->GetNetSpecifier()->netCapabilities_ == netSpecifier.netCapabilities_) {
            ret.push_back(request);
        }
    }
    return ret;
}

sptr<NetRequest> NetConnService::FindNetRequestByCallback(const sptr<INetConnCallback> &callback)
{
    for (auto p : netRequests_) {
        const auto &request = p.second;
        const auto &cb = request->GetNetConnCallback();
        if (cb && (callback->AsObject().GetRefPtr() == cb->AsObject().GetRefPtr())) {
            return request;
        }
    }
    return nullptr;
}

bool NetConnService::RemoveNetSupplier(int32_t supplierId)
{
    auto supplier = FindNetSupplier(supplierId);
    if (supplier) {
        NETMGR_LOG_I("Remove net supplier, id=%{public}d", supplier->GetId());
        supplier->RemoveAllNetRequests();
        netSuppliers_.erase(supplierId);
        return true;
    }
    return false;
}

bool NetConnService::RemoveNetRequest(const sptr<NetRequest> &request)
{
    if (request) {
        NETMGR_LOG_I("Remove net request, id=%{public}d", request->GetId());
        auto supplier = FindNetSupplier(request->GetNetSupplierId());
        if (supplier) {
            supplier->RemoveNetRequest(request);
        }
        netRequests_.erase(request->GetId());
        return true;
    }
    return false;
}

void NetConnService::RematchAllNetworks(RematchAllNetworksReason reason)
{
    std::string reasonStr;
    switch (reason) {
        case REASON_NET_AVAILABLE_CHANGED:
            reasonStr = "NET_AVAILABLE_CHANGED";
            break;
        case REASON_NET_CAPABILITIES_CHANGED:
            reasonStr = "NET_SUPPLIER_CAPABILITIES_CHANGED";
            break;
        case REASON_NET_LINK_INFO_CHANGED:
            reasonStr = "NET_LINK_INFO_CHANGED";
            break;
        case REASON_NET_SCORE_CHANGED:
            reasonStr = "NET_SCORE_CHANGED";
            break;
    }

    NETMGR_LOG_I("Start rematch all networks, reason=%{public}s", reasonStr.c_str());
    DumpSuppliersInfo();

    // rematch all request to best net supplier for themself
    sptr<NetSupplier> defaultNetSupplier = nullptr;
    for (auto p : netRequests_) {
        sptr<NetRequest> request = p.second;
        sptr<NetSupplier> oldSupplier = FindNetSupplier(request->GetNetSupplierId());
        sptr<NetSupplier> bestSupplier = GetBestNetworkForRequest(request);

        if (oldSupplier != bestSupplier) {
            if (oldSupplier) {
                oldSupplier->RemoveNetRequest(request);
            }
            if (bestSupplier) {
                bestSupplier->AddNetRequest(request);
            }
        }

        if (request == defaultNetRequest_) {
            defaultNetSupplier = bestSupplier;
        }
    }

    if (defaultNetSupplier) {
        defaultNetSupplier->GetNetwork()->SetDefault();
        NETMGR_LOG_I("Rematch networks finished, default network[%{public}s]", defaultNetSupplier->GetIdent().c_str());
    } else {
        NETMGR_LOG_I("Rematch networks finished, no default network");
    }
    defaultNetSupplier_ = defaultNetSupplier;
}

sptr<NetSupplier> NetConnService::GetBestNetworkForRequest(const sptr<NetRequest> &request)
{
    if (request == nullptr) {
        return nullptr;
    }
    sptr<NetSupplier> bestSupplier;
    for (auto iter : netSuppliers_) {
        auto supplier = iter.second;
        if (supplier->IsAvailable() && supplier->SatisfiyNetRequest(request)) {
            if (!bestSupplier) {
                bestSupplier = supplier;
            } else if (supplier->GetCurrentScore() >= bestSupplier->GetCurrentScore()) {
                bestSupplier = supplier;
            }
        }
    }
    return bestSupplier;
}

void NetConnService::OnNetAvailableChanged(uint32_t supplierId, bool available)
{
    RematchAllNetworks(REASON_NET_AVAILABLE_CHANGED);
}

void NetConnService::OnNetCapabilitiesChanged(uint32_t supplierId, const NetAllCapabilities &allCaps)
{
    RematchAllNetworks(REASON_NET_CAPABILITIES_CHANGED);
}

void NetConnService::OnNetLinkInfoChanged(uint32_t supplierId, const NetLinkInfo &linkInfo)
{
    RematchAllNetworks(REASON_NET_LINK_INFO_CHANGED);
}

void NetConnService::OnNetDetectionResultChanged(uint32_t netId,
                                                 NetDetectionResultCode detectionResult,
                                                 const std::string &urlRedirect)
{
    auto supplier = FindNetSupplierByNetId(netId);
    if (supplier) {
        if (detectionResult == NET_DETECTION_SUCCESS) {
            supplier->InsertNetCap(NET_CAPABILITY_VALIDATED);
        } else {
            supplier->RemoveNetCap(NET_CAPABILITY_VALIDATED);
        }
        supplier->NotifyNetDetectionResult(detectionResult, urlRedirect);
    }
}

void NetConnService::OnNetScoreChanged(uint32_t supplierId, uint32_t score)
{
    RematchAllNetworks(REASON_NET_SCORE_CHANGED);
}

int32_t NetConnService::InvokeMethodSafety(std::function<int32_t(void)> func)
{
    if (GetScheduler().InRunThread()) {
        return func();
    } else {
        int32_t err = ERR_NONE;
        auto task = GetScheduler().Post([&]() {
            err = func();
        });
        if (!task->WaitFor(NET_CONN_BLOCK_TIMEOUT_MS)) {
            NETMGR_LOG_W("Blocking call!!!");
            return ERR_METHOD_BLOCKING;
        }
        return err;
    }
}

void NetConnService::DumpSuppliersInfo()
{
    std::string str;
    for (auto iter : netSuppliers_) {
        str += "{";
        str += std::to_string(iter.second->GetId()) + ",";
        str += std::to_string(iter.second->IsAvailable()) + ",";
        str += std::to_string(iter.second->GetBearerType()) + ",";
        str += iter.second->GetIdent();
        str += "}";
    }
    NETMGR_LOG_I("AllSuppliers: [%{public}s]", str.c_str());
}
} // namespace NetManagerStandard
} // namespace OHOS
