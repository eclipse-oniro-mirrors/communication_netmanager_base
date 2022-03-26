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

#include "net_manager_native.h"
#include <net/if.h>
#include "interface_controller.h"
#include "netnative_log_wrapper.h"
#include "network_controller.h"
#include "route_controller.h"
#include "traffic_controller.h"

std::vector<unsigned int> OHOS::nmd::NetManagerNative::interfaceIdex;

namespace OHOS {
namespace nmd {
NetManagerNative::NetManagerNative()
    : networkController(std::make_shared<NetworkController>()),
      routeController(std::make_shared<RouteController>()),
      interfaceController(std::make_shared<InterfaceController>())
{}

NetManagerNative::~NetManagerNative() {}

void NetManagerNative::GetOriginInterfaceIndex()
{
    std::vector<std::string> ifNameList = InterfaceController::GetInterfaceNames();
    NetManagerNative::interfaceIdex.clear();
    for (auto iter = ifNameList.begin(); iter != ifNameList.end(); ++iter) {
        unsigned int infIndex = if_nametoindex((*iter).c_str());
        NetManagerNative::interfaceIdex.push_back(infIndex);
    }
}

void NetManagerNative::UpdateInterfaceIndex(unsigned int infIndex)
{
    NetManagerNative::interfaceIdex.push_back(infIndex);
}

std::vector<unsigned int> NetManagerNative::GetCurrentInterfaceIndex()
{
    return NetManagerNative::interfaceIdex;
}

void NetManagerNative::Init()
{
    this->GetOriginInterfaceIndex();
}

int NetManagerNative::NetworkCreatePhysical(int netId, int permission)
{
    return this->networkController->CreatePhysicalNetwork(
        static_cast<uint16_t>(netId), static_cast<NetworkPermission>(permission));
}

int NetManagerNative::NetworkDestroy(int netId)
{
    return this->networkController->DestroyNetwork(netId);
}

int NetManagerNative::NetworkAddInterface(int netId, std::string interfaceName)
{
    NETNATIVE_LOGI("Entry NetManagerNative::NetworkAddInterface");
    return this->networkController->AddInterfaceToNetwork(netId, interfaceName);
}

int NetManagerNative::NetworkRemoveInterface(int netId, std::string interfaceName)
{
    return this->networkController->RemoveInterfaceFromNetwork(netId, interfaceName);
}

int NetManagerNative::InterfaceAddAddress(std::string ifName, std::string addrString, int prefixLength)
{
    NETNATIVE_LOGI("NetManagerNative::InterfaceAddAddress, ifName:%{public}s, addrString:%{public}s,"
        "prefixLength:%{public}d", ifName.c_str(), addrString.c_str(), prefixLength);

    return this->interfaceController->AddAddress(ifName.c_str(), addrString.c_str(), prefixLength);
}

int NetManagerNative::InterfaceDelAddress(std::string ifName, std::string addrString, int prefixLength)
{
    NETNATIVE_LOGI("NetManagerNative::InterfaceAddAddress, ifName:%{public}s, addrString:%{public}s,"                                                                                                                                                "prefixLength:%{public}d", ifName.c_str(), addrString.c_str(), prefixLength);

    return this->interfaceController->DelAddress(ifName.c_str(), addrString.c_str(), prefixLength);
}

int NetManagerNative::NetworkAddRoute(
    int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return this->networkController->AddRoute(netId, interfaceName, destination, nextHop);
}

int NetManagerNative::NetworkRemoveRoute(
    int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return this->networkController->RemoveRoute(netId, interfaceName, destination, nextHop);
}

int NetManagerNative::NetworkGetDefault()
{
    return this->networkController->GetDefaultNetwork();
}

int NetManagerNative::NetworkSetDefault(int netId)
{
    return this->networkController->SetDefaultNetwork(netId);
}

int NetManagerNative::NetworkClearDefault()
{
    return this->networkController->ClearDefaultNetwork();
}

int NetManagerNative::NetworkSetPermissionForNetwork(int netId, NetworkPermission permission)
{
    return this->networkController->SetPermissionForNetwork(netId, permission);
}

std::vector<std::string> NetManagerNative::InterfaceGetList()
{
    return InterfaceController::GetInterfaceNames();
}

nmd::InterfaceConfigurationParcel NetManagerNative::InterfaceGetConfig(std::string interfaceName)
{
    return InterfaceController::GetIfaceConfig(interfaceName.c_str());
}

void NetManagerNative::InterfaceSetConfig(nmd::InterfaceConfigurationParcel parcel)
{
    InterfaceController::SetIfaceConfig(parcel);
}

void NetManagerNative::InterfaceClearAddrs(const std::string ifName)
{
}

int NetManagerNative::InterfaceGetMtu(std::string ifName)
{
    return InterfaceController::GetMtu(ifName.c_str());
}

int NetManagerNative::InterfaceSetMtu(std::string ifName, int mtuValue)
{
    std::string mtu = std::to_string(mtuValue);
    return InterfaceController::SetMtu(ifName.c_str(), mtu.c_str());
}

nmd::MarkMaskParcel NetManagerNative::GetFwmarkForNetwork(int netId)
{
    nmd::MarkMaskParcel mark;
    mark.mark = this->networkController->GetFwmarkForNetwork(netId);
    mark.mask = 0XFFFF;
    return mark;
}

int NetManagerNative::NetworkAddRouteParcel(int netId, RouteInfoParcel parcel)
{
    return this->networkController->AddRoute(netId, parcel.ifName, parcel.destination, parcel.nextHop);
}

int NetManagerNative::NetworkRemoveRouteParcel(int netId, RouteInfoParcel parcel)
{
    return this->networkController->RemoveRoute(netId, parcel.ifName, parcel.destination, parcel.nextHop);
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
    return nmd::TrafficController::GetAllRxTraffic();
}

long NetManagerNative::GetAllTxBytes()
{
    return nmd::TrafficController::GetAllTxTraffic();
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
    nmd::TrafficStatsParcel interfaceTraffic = nmd::TrafficController::GetInterfaceTraffic(interfaceName);
    return interfaceTraffic.rxBytes;
}

long NetManagerNative::GetIfaceTxBytes(std::string interfaceName)
{
    nmd::TrafficStatsParcel interfaceTraffic = nmd::TrafficController::GetInterfaceTraffic(interfaceName);
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
} // namespace nmd
} // namespace OHOS
