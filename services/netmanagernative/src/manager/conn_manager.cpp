/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "conn_manager.h"

#include "bpf_def.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "local_network.h"
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "physical_network.h"
#include "virtual_network.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;
namespace {
constexpr int32_t INTERFACE_UNSET = -1;
constexpr int32_t LOCAL_NET_ID = 99;
} // namespace

ConnManager::ConnManager()
{
    networks_.EnsureInsert(LOCAL_NET_ID, std::make_shared<LocalNetwork>(LOCAL_NET_ID));
    defaultNetId_ = 0;
    needReinitRouteFlag_ = false;
}

ConnManager::~ConnManager()
{
    networks_.Clear();
}

int32_t ConnManager::SetInternetPermission(uint32_t uid, uint8_t allow, uint8_t isBroker)
{
    // 0 means root
    if (uid == 0) {
        return NETMANAGER_ERROR;
    }
    if (isBroker) {
        BpfMapper<sock_permission_key, sock_permission_value> permissionMap(BROKER_SOCKET_PERMISSION_MAP_PATH,
                                                                            BPF_F_WRONLY);
        if (!permissionMap.IsValid()) {
            return NETMANAGER_ERROR;
        }
        // 0 means no permission
        if (permissionMap.Write(uid, allow, 0) != 0) {
            return NETMANAGER_ERROR;
        }

        return NETMANAGER_SUCCESS;
    }
    BpfMapper<sock_permission_key, sock_permission_value> permissionMap(OH_SOCKET_PERMISSION_MAP_PATH, BPF_F_WRONLY);
    if (!permissionMap.IsValid()) {
        return NETMANAGER_ERROR;
    }
    // 0 means no permission
    if (permissionMap.Write(uid, allow, 0) != 0) {
        return NETMANAGER_ERROR;
    }

    return NETMANAGER_SUCCESS;
}

int32_t ConnManager::CreatePhysicalNetwork(uint16_t netId, NetworkPermission permission)
{
    if (needReinitRouteFlag_) {
        std::set<int32_t> netIds;
        networks_.Iterate([&netIds](int32_t id, std::shared_ptr<NetsysNetwork> &NetsysNetworkPtr) {
            if (id == LOCAL_NET_ID || NetsysNetworkPtr == nullptr) {
                return;
            }
            netIds.insert(NetsysNetworkPtr->GetNetId());
        });

        for (auto netId : netIds) {
            std::string interfaceName;
            {
                std::lock_guard<std::mutex> lock(interfaceNameMutex_);
                interfaceName = physicalInterfaceName_[netId];
            }
            RemoveInterfaceFromNetwork(netId, interfaceName);
            DestroyNetwork(netId);
        }
        needReinitRouteFlag_ = false;
    }
    std::shared_ptr<NetsysNetwork> network = std::make_shared<PhysicalNetwork>(netId, permission);
    networks_.EnsureInsert(netId, network);
    return NETMANAGER_SUCCESS;
}

int32_t ConnManager::CreateVirtualNetwork(uint16_t netId, bool hasDns)
{
    networks_.EnsureInsert(netId, std::make_shared<VirtualNetwork>(netId, hasDns));
    return NETMANAGER_SUCCESS;
}

int32_t ConnManager::DestroyNetwork(int32_t netId)
{
    if (netId == LOCAL_NET_ID) {
        NETNATIVE_LOGE("Cannot destroy local network");
        return NETMANAGER_ERROR;
    }
    const auto &net = FindNetworkById(netId);
    if (std::get<0>(net)) {
        std::shared_ptr<NetsysNetwork> nw = std::get<1>(net);
        if (defaultNetId_ == netId) {
            if (nw->IsPhysical()) {
                static_cast<PhysicalNetwork *>(nw.get())->RemoveDefault();
            }
            defaultNetId_ = 0;
        }
        nw->ClearInterfaces();
    }
    networks_.Erase(netId);
    return NETMANAGER_SUCCESS;
}

int32_t ConnManager::SetDefaultNetwork(int32_t netId)
{
    if (defaultNetId_ == netId) {
        return NETMANAGER_SUCCESS;
    }

    // check if this network exists
    const auto &net = FindNetworkById(netId);
    if (std::get<0>(net)) {
        std::shared_ptr<NetsysNetwork> nw = std::get<1>(net);
        if (!nw->IsPhysical()) {
            NETNATIVE_LOGE("SetDefaultNetwork fail, network :%{public}d is not physical ", netId);
            return NETMANAGER_ERROR;
        }
        static_cast<PhysicalNetwork *>(nw.get())->AddDefault();
    }

    if (defaultNetId_ != 0) {
        const auto &defaultNet = FindNetworkById(defaultNetId_);
        if (std::get<0>(defaultNet)) {
            std::shared_ptr<NetsysNetwork> nw = std::get<1>(defaultNet);
            if (!nw->IsPhysical()) {
                NETNATIVE_LOGE("SetDefaultNetwork fail, defaultNetId_ :%{public}d is not physical", defaultNetId_);
                return NETMANAGER_ERROR;
            }
            static_cast<PhysicalNetwork *>(nw.get())->RemoveDefault();
        }
    }
    defaultNetId_ = netId;
    return NETMANAGER_SUCCESS;
}

int32_t ConnManager::ClearDefaultNetwork()
{
    if (defaultNetId_ != 0) {
        const auto &net = FindNetworkById(defaultNetId_);
        if (std::get<0>(net)) {
            std::shared_ptr<NetsysNetwork> nw = std::get<1>(net);
            if (!nw->IsPhysical()) {
                NETNATIVE_LOGE("ClearDefaultNetwork fail, defaultNetId_ :%{public}d is not physical", defaultNetId_);
                return NETMANAGER_ERROR;
            }
            static_cast<PhysicalNetwork *>(nw.get())->RemoveDefault();
        }
    }
    defaultNetId_ = 0;
    return NETMANAGER_SUCCESS;
}

std::tuple<bool, std::shared_ptr<NetsysNetwork>> ConnManager::FindNetworkById(int32_t netId)
{
    NETNATIVE_LOG_D("Entry ConnManager::FindNetworkById netId:%{public}d", netId);
    std::shared_ptr<NetsysNetwork> netsysNetworkPtr;
    bool ret = networks_.Find(netId, netsysNetworkPtr);
    if (ret) {
        return std::make_tuple(true, netsysNetworkPtr);
    }
    return std::make_tuple<bool, std::shared_ptr<NetsysNetwork>>(false, nullptr);
}

int32_t ConnManager::GetDefaultNetwork() const
{
    return defaultNetId_;
}

int32_t ConnManager::GetNetworkForInterface(std::string &interfaceName)
{
    NETNATIVE_LOG_D("Entry ConnManager::GetNetworkForInterface interfaceName:%{public}s", interfaceName.c_str());
    std::map<int32_t, std::shared_ptr<NetsysNetwork>>::iterator it;
    int32_t InterfaceId = INTERFACE_UNSET;
    networks_.Iterate([&InterfaceId, &interfaceName](int32_t id, std::shared_ptr<NetsysNetwork> &NetsysNetworkPtr) {
        if (InterfaceId != INTERFACE_UNSET) {
            return;
        }
        if (NetsysNetworkPtr != nullptr) {
            if (NetsysNetworkPtr->ExistInterface(interfaceName)) {
                InterfaceId = id;
            }
        }
    });
    return InterfaceId;
}

int32_t ConnManager::AddInterfaceToNetwork(int32_t netId, std::string &interfaceName)
{
    NETNATIVE_LOG_D("Entry ConnManager::AddInterfaceToNetwork netId:%{public}d, interfaceName:%{public}s", netId,
                    interfaceName.c_str());
    int32_t alreadySetNetId = GetNetworkForInterface(interfaceName);
    if ((alreadySetNetId != netId) && (alreadySetNetId != INTERFACE_UNSET)) {
        NETNATIVE_LOGE("AddInterfaceToNetwork failed alreadySetNetId:%{public}d", alreadySetNetId);
        return NETMANAGER_ERROR;
    }

    const auto &net = FindNetworkById(netId);
    if (std::get<0>(net)) {
        std::shared_ptr<NetsysNetwork> nw = std::get<1>(net);
        if (nw->IsPhysical()) {
            std::lock_guard<std::mutex> lock(interfaceNameMutex_);
            physicalInterfaceName_[netId] = interfaceName;
        }
        return nw->AddInterface(interfaceName);
    }
    return NETMANAGER_ERROR;
}

int32_t ConnManager::RemoveInterfaceFromNetwork(int32_t netId, std::string &interfaceName)
{
    int32_t alreadySetNetId = GetNetworkForInterface(interfaceName);
    if ((alreadySetNetId != netId) || (alreadySetNetId == INTERFACE_UNSET)) {
        return NETMANAGER_SUCCESS;
    } else if (alreadySetNetId == netId) {
        const auto &net = FindNetworkById(netId);
        if (std::get<0>(net)) {
            std::shared_ptr<NetsysNetwork> nw = std::get<1>(net);
            int32_t ret = nw->RemoveInterface(interfaceName);
            if (nw->IsPhysical()) {
                std::lock_guard<std::mutex> lock(interfaceNameMutex_);
                physicalInterfaceName_.erase(netId);
            }
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t ConnManager::ReinitRoute()
{
    NETNATIVE_LOG_D("ConnManager::ReInitRoute");
    needReinitRouteFlag_ = true;
    return NETMANAGER_SUCCESS;
}

int32_t ConnManager::AddRoute(int32_t netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return RouteManager::AddRoute(GetTableType(netId), interfaceName, destination, nextHop);
}

int32_t ConnManager::RemoveRoute(int32_t netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return RouteManager::RemoveRoute(GetTableType(netId), interfaceName, destination, nextHop);
}

int32_t ConnManager::UpdateRoute(int32_t netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return RouteManager::UpdateRoute(GetTableType(netId), interfaceName, destination, nextHop);
}

RouteManager::TableType ConnManager::GetTableType(int32_t netId)
{
    if (netId == LOCAL_NET_ID) {
        return RouteManager::LOCAL_NETWORK;
    } else if (FindVirtualNetwork(netId) != nullptr) {
        return RouteManager::VPN_NETWORK;
    } else {
        return RouteManager::INTERFACE;
    }
}

int32_t ConnManager::GetFwmarkForNetwork(int32_t netId)
{
    return NETMANAGER_ERROR;
}

int32_t ConnManager::SetPermissionForNetwork(int32_t netId, NetworkPermission permission)
{
    return NETMANAGER_ERROR;
}

std::shared_ptr<NetsysNetwork> ConnManager::FindVirtualNetwork(int32_t netId)
{
    if (netId == LOCAL_NET_ID) {
        return nullptr;
    }
    std::shared_ptr<NetsysNetwork> netsysNetworkPtr = nullptr;
    auto ret = networks_.Find(netId, netsysNetworkPtr);
    if (!ret || netsysNetworkPtr == nullptr) {
        NETNATIVE_LOGE("invalid netId:%{public}d or nw is null.", netId);
        return nullptr;
    }
    if (netsysNetworkPtr->IsPhysical()) {
        return nullptr;
    }
    return netsysNetworkPtr;
}

int32_t ConnManager::AddUidsToNetwork(int32_t netId, const std::vector<NetManagerStandard::UidRange> &uidRanges)
{
    auto netsysNetwork = FindVirtualNetwork(netId);
    if (netsysNetwork == nullptr) {
        NETNATIVE_LOGE("cannot add uids to non-virtual network with netId:%{public}d", netId);
        return NETMANAGER_ERROR;
    }
    return static_cast<VirtualNetwork *>(netsysNetwork.get())->AddUids(uidRanges);
}

int32_t ConnManager::RemoveUidsFromNetwork(int32_t netId, const std::vector<NetManagerStandard::UidRange> &uidRanges)
{
    auto netsysNetwork = FindVirtualNetwork(netId);
    if (netsysNetwork == nullptr) {
        NETNATIVE_LOGE("cannot remove uids from non-virtual network with netId:%{public}d", netId);
        return NETMANAGER_ERROR;
    }
    return static_cast<VirtualNetwork *>(netsysNetwork.get())->RemoveUids(uidRanges);
}

void ConnManager::GetDumpInfos(std::string &infos)
{
    static const std::string TAB = "  ";
    infos.append("Netsys connect manager :\n");
    infos.append(TAB + "default NetId: " + std::to_string(defaultNetId_) + "\n");
    networks_.Iterate([&infos](int32_t id, std::shared_ptr<NetsysNetwork> &NetsysNetworkPtr) {
        infos.append(TAB + "NetId:" + std::to_string(id));
        std::string interfaces = TAB + "interfaces: {";
        for (const auto &interface : NetsysNetworkPtr->GetAllInterface()) {
            interfaces.append(interface + ", ");
        }
        infos.append(interfaces + "}\n");
    });
}
} // namespace nmd
} // namespace OHOS
