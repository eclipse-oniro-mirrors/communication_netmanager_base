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
#include <cstdint>
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
    NET_ACTIVATE_MAP::iterator iterActive;
    for (iterActive = netActivates_.begin(); iterActive != netActivates_.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        if (ns->HasNetCap(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT)) {
            NETMGR_LOG_D("Supplier[%{public}d] is internal, skip.", ns->GetSupplierId());
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

    if (defaultNetSupplier_ != nullptr) {
        defaultNetSupplier_->ClearDefault();
        defaultNetSupplier_ = nullptr;
    }

    FindBestNetworkForAllRequest();
}

int32_t NetConnService::IsPreferCellularUrl(const std::string& url, bool& preferCellular)
{
    static std::vector<std::string> preferredUrlList = GetPreferredUrl();
    preferCellular = std::any_of(preferredUrlList.begin(), preferredUrlList.end(),
                                 [&url](const std::string &str) { return url.find(str) != std::string::npos; });
    return 0;
}

bool NetConnService::IsAddrInOtherNetwork(int32_t netId, const INetAddr &netAddr)
{
    for (const auto &network : networks_) {
        if (network.second->GetNetId() == netId) {
            continue;
        }
        if (network.second->GetNetLinkInfo().HasNetAddr(netAddr)) {
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
} // namespace NetManagerStandard
} // namespace OHOS
