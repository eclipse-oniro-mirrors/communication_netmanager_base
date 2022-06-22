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

#include "net_manager_native.h"

#include <net/if.h>

#include "conn_manager.h"
#include "interface_manager.h"
#include "netnative_log_wrapper.h"
#include "network_permission.h"
#include "route_manager.h"
#include "traffic_manager.h"

std::vector<unsigned int> OHOS::nmd::NetManagerNative::interfaceIndex;

namespace OHOS {
namespace nmd {
NetManagerNative::NetManagerNative()
    : routeManager(std::make_shared<RouteManager>()), interfaceManager(std::make_shared<InterfaceManager>()),
      sharingManager(std::make_shared<SharingManager>()), connManager(std::make_shared<ConnManager>())
{
}

void NetManagerNative::GetOriginInterfaceIndex()
{
    std::vector<std::string> ifNameList = InterfaceManager::GetInterfaceNames();
    NetManagerNative::interfaceIndex.clear();
    for (auto iter = ifNameList.begin(); iter != ifNameList.end(); ++iter) {
        unsigned int infIndex = if_nametoindex((*iter).c_str());
        NetManagerNative::interfaceIndex.push_back(infIndex);
    }
}

void NetManagerNative::UpdateInterfaceIndex(unsigned int infIndex)
{
    NetManagerNative::interfaceIndex.push_back(infIndex);
}

std::vector<unsigned int> NetManagerNative::GetCurrentInterfaceIndex()
{
    return NetManagerNative::interfaceIndex;
}

void NetManagerNative::Init()
{
    GetOriginInterfaceIndex();
}

int NetManagerNative::NetworkCreatePhysical(int netId, int permission)
{
    return connManager->CreatePhysicalNetwork(
        static_cast<uint16_t>(netId), static_cast<NetworkPermission>(permission));
}

int NetManagerNative::NetworkDestroy(int netId)
{
    return connManager->DestroyNetwork(netId);
}

int NetManagerNative::NetworkAddInterface(int netId, std::string interfaceName)
{
    NETNATIVE_LOGI("Entry NetManagerNative::NetworkAddInterface");
    return connManager->AddInterfaceToNetwork(netId, interfaceName);
}

int NetManagerNative::NetworkRemoveInterface(int netId, std::string interfaceName)
{
    return connManager->RemoveInterfaceFromNetwork(netId, interfaceName);
}

int NetManagerNative::InterfaceAddAddress(std::string ifName, std::string addrString, int prefixLength)
{
    NETNATIVE_LOGI("NetManagerNative::InterfaceAddAddress, ifName:%{public}s, addrString:%{public}s,"
        "prefixLength:%{public}d", ifName.c_str(), addrString.c_str(), prefixLength);

    return interfaceManager->AddAddress(ifName.c_str(), addrString.c_str(), prefixLength);
}

int NetManagerNative::InterfaceDelAddress(std::string ifName, std::string addrString, int prefixLength)
{
    NETNATIVE_LOGI("NetManagerNative::InterfaceAddAddress, ifName:%{public}s, addrString:%{public}s,"                                                                                                                                                "prefixLength:%{public}d", ifName.c_str(), addrString.c_str(), prefixLength);

    return interfaceManager->DelAddress(ifName.c_str(), addrString.c_str(), prefixLength);
}

int NetManagerNative::NetworkAddRoute(
    int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return connManager->AddRoute(netId, interfaceName, destination, nextHop);
}

int NetManagerNative::NetworkRemoveRoute(
    int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return connManager->RemoveRoute(netId, interfaceName, destination, nextHop);
}

int NetManagerNative::NetworkGetDefault()
{
    return connManager->GetDefaultNetwork();
}

int NetManagerNative::NetworkSetDefault(int netId)
{
    return connManager->SetDefaultNetwork(netId);
}

int NetManagerNative::NetworkClearDefault()
{
    return connManager->ClearDefaultNetwork();
}

int NetManagerNative::NetworkSetPermissionForNetwork(int netId, NetworkPermission permission)
{
    return connManager->SetPermissionForNetwork(netId, permission);
}

std::vector<std::string> NetManagerNative::InterfaceGetList()
{
    return InterfaceManager::GetInterfaceNames();
}

nmd::InterfaceConfigurationParcel NetManagerNative::InterfaceGetConfig(std::string interfaceName)
{
    return InterfaceManager::GetIfaceConfig(interfaceName.c_str());
}

void NetManagerNative::InterfaceSetConfig(nmd::InterfaceConfigurationParcel parcel)
{
    InterfaceManager::SetIfaceConfig(parcel);
}

void NetManagerNative::InterfaceClearAddrs(const std::string ifName)
{
}

int NetManagerNative::InterfaceGetMtu(std::string ifName)
{
    return InterfaceManager::GetMtu(ifName.c_str());
}

int NetManagerNative::InterfaceSetMtu(std::string ifName, int mtuValue)
{
    std::string mtu = std::to_string(mtuValue);
    return InterfaceManager::SetMtu(ifName.c_str(), mtu.c_str());
}

nmd::MarkMaskParcel NetManagerNative::GetFwmarkForNetwork(int netId)
{
    nmd::MarkMaskParcel mark;
    mark.mark = connManager->GetFwmarkForNetwork(netId);
    mark.mask = 0XFFFF;
    return mark;
}

int NetManagerNative::NetworkAddRouteParcel(int netId, RouteInfoParcel parcel)
{
    return connManager->AddRoute(netId, parcel.ifName, parcel.destination, parcel.nextHop);
}

int NetManagerNative::NetworkRemoveRouteParcel(int netId, RouteInfoParcel parcel)
{
    return connManager->RemoveRoute(netId, parcel.ifName, parcel.destination, parcel.nextHop);
}

int NetManagerNative::SetProcSysNet(int32_t ipversion, int32_t which, const std::string ifname,
    const std::string parameter, const std::string value)
{
    return 0;
}

int NetManagerNative::GetProcSysNet(
    int32_t ipversion, int32_t which, const std::string ifname, const std::string parameter, std::string *value)
{
    return 0;
}

long NetManagerNative::GetCellularRxBytes()
{
    return 0;
}

long NetManagerNative::GetCellularTxBytes()
{
    return 0;
}

long NetManagerNative::GetAllRxBytes()
{
    return nmd::TrafficManager::GetAllRxTraffic();
}

long NetManagerNative::GetAllTxBytes()
{
    return nmd::TrafficManager::GetAllTxTraffic();
}

long NetManagerNative::GetUidTxBytes(int uid)
{
    return 0;
}

long NetManagerNative::GetUidRxBytes(int uid)
{
    return 0;
}

long NetManagerNative::GetIfaceRxBytes(std::string interfaceName)
{
    nmd::TrafficStatsParcel interfaceTraffic = nmd::TrafficManager::GetInterfaceTraffic(interfaceName);
    return interfaceTraffic.rxBytes;
}

long NetManagerNative::GetIfaceTxBytes(std::string interfaceName)
{
    nmd::TrafficStatsParcel interfaceTraffic = nmd::TrafficManager::GetInterfaceTraffic(interfaceName);
    return interfaceTraffic.txBytes;
}

long NetManagerNative::GetTetherRxBytes()
{
    return 0;
}

long NetManagerNative::GetTetherTxBytes()
{
    return 0;
}

int32_t NetManagerNative::IpEnableForwarding(const std::string &requester)
{
    return this->sharingManager->IpEnableForwarding(requester);
}

int32_t NetManagerNative::IpDisableForwarding(const std::string &requester)
{
    return this->sharingManager->IpDisableForwarding(requester);
}

int32_t NetManagerNative::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    return this->sharingManager->EnableNat(downstreamIface, upstreamIface);
}

int32_t NetManagerNative::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    return this->sharingManager->DisableNat(downstreamIface, upstreamIface);
}

int32_t NetManagerNative::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    return this->sharingManager->IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetManagerNative::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    return this->sharingManager->IpfwdRemoveInterfaceForward(fromIface, toIface);
}
} // namespace nmd
} // namespace OHOS
