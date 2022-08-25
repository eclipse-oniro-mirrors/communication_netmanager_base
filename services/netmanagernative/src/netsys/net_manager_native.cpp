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

#include <net/if.h>

#include "interface_manager.h"
#include "netnative_log_wrapper.h"
#include "network_permission.h"
#include "net_manager_constants.h"
#include "route_manager.h"
#include "traffic_manager.h"

#include "net_manager_native.h"

std::vector<unsigned int> OHOS::nmd::NetManagerNative::interfaceIdex;

namespace OHOS {
namespace nmd {
NetManagerNative::NetManagerNative()
    : bandwidthManager_(std::make_shared<BandwidthManager>()),
      connManager_(std::make_shared<ConnManager>()),
      firewallManager_(std::make_shared<FirewallManager>()),
      routeManager_(std::make_shared<RouteManager>()),
      interfaceManager_(std::make_shared<InterfaceManager>()),
      sharingManager_(std::make_shared<SharingManager>()),
      dnsManager_(std::make_shared<DnsManager>())
{
}

void NetManagerNative::GetOriginInterfaceIndex()
{
    std::vector<std::string> ifNameList = InterfaceManager::GetInterfaceNames();
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
    GetOriginInterfaceIndex();
}

int NetManagerNative::NetworkCreatePhysical(int netId, int permission)
{
    return connManager_->CreatePhysicalNetwork(static_cast<uint16_t>(netId),
                                               static_cast<NetworkPermission>(permission));
}

int NetManagerNative::NetworkDestroy(int netId)
{
    return connManager_->DestroyNetwork(netId);
}

int NetManagerNative::NetworkAddInterface(int netId, std::string interfaceName)
{
    NETNATIVE_LOGI("Entry NetManagerNative::NetworkAddInterface");
    return connManager_->AddInterfaceToNetwork(netId, interfaceName);
}

int NetManagerNative::NetworkRemoveInterface(int netId, std::string interfaceName)
{
    return connManager_->RemoveInterfaceFromNetwork(netId, interfaceName);
}

int NetManagerNative::InterfaceAddAddress(std::string ifName, std::string addrString, int prefixLength)
{
    NETNATIVE_LOGI(
        "NetManagerNative::InterfaceAddAddress, ifName:%{public}s, addrString:%{public}s,"
        "prefixLength:%{public}d",
        ifName.c_str(), addrString.c_str(), prefixLength);

    return interfaceManager_->AddAddress(ifName.c_str(), addrString.c_str(), prefixLength);
}

int NetManagerNative::InterfaceDelAddress(std::string ifName, std::string addrString, int prefixLength)
{
    NETNATIVE_LOGI(
        "NetManagerNative::InterfaceAddAddress, ifName:%{public}s, addrString:%{public}s,"
        "prefixLength:%{public}d",
        ifName.c_str(), addrString.c_str(), prefixLength);

    return interfaceManager_->DelAddress(ifName.c_str(), addrString.c_str(), prefixLength);
}

int NetManagerNative::NetworkAddRoute(int netId, std::string interfaceName, std::string destination,
                                      std::string nextHop)
{
    return connManager_->AddRoute(netId, interfaceName, destination, nextHop);
}

int NetManagerNative::NetworkRemoveRoute(int netId, std::string interfaceName, std::string destination,
                                         std::string nextHop)
{
    return connManager_->RemoveRoute(netId, interfaceName, destination, nextHop);
}

int NetManagerNative::NetworkGetDefault()
{
    return connManager_->GetDefaultNetwork();
}

int NetManagerNative::NetworkSetDefault(int netId)
{
    dnsManager_->SetDefaultNetwork(netId); // set default netId to dns manager, do not delete this line!
    return connManager_->SetDefaultNetwork(netId);
}

int NetManagerNative::NetworkClearDefault()
{
    return connManager_->ClearDefaultNetwork();
}

int NetManagerNative::NetworkSetPermissionForNetwork(int netId, NetworkPermission permission)
{
    return connManager_->SetPermissionForNetwork(netId, permission);
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

void NetManagerNative::InterfaceClearAddrs(const std::string ifName) {}

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
    mark.mark = connManager_->GetFwmarkForNetwork(netId);
    mark.mask = 0XFFFF;
    return mark;
}

int NetManagerNative::NetworkAddRouteParcel(int netId, RouteInfoParcel parcel)
{
    return connManager_->AddRoute(netId, parcel.ifName, parcel.destination, parcel.nextHop);
}

int NetManagerNative::NetworkRemoveRouteParcel(int netId, RouteInfoParcel parcel)
{
    return connManager_->RemoveRoute(netId, parcel.ifName, parcel.destination, parcel.nextHop);
}

int NetManagerNative::SetProcSysNet(int32_t ipversion, int32_t which, const std::string ifname,
                                    const std::string parameter, const std::string value)
{
    return 0;
}

int NetManagerNative::GetProcSysNet(int32_t ipversion, int32_t which, const std::string ifname,
                                    const std::string parameter, std::string *value)
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
    return sharingManager_->IpEnableForwarding(requester);
}

int32_t NetManagerNative::IpDisableForwarding(const std::string &requester)
{
    return sharingManager_->IpDisableForwarding(requester);
}

int32_t NetManagerNative::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    return sharingManager_->EnableNat(downstreamIface, upstreamIface);
}

int32_t NetManagerNative::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    return sharingManager_->DisableNat(downstreamIface, upstreamIface);
}

int32_t NetManagerNative::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    return sharingManager_->IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetManagerNative::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    return sharingManager_->IpfwdRemoveInterfaceForward(fromIface, toIface);
}

int32_t NetManagerNative::DnsSetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                               const std::vector<std::string> &servers,
                                               const std::vector<std::string> &domains)
{
    return dnsManager_->SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
}

int32_t NetManagerNative::DnsGetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
                                               std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                               uint8_t &retryCount)
{
    return dnsManager_->GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
}

int32_t NetManagerNative::DnsCreateNetworkCache(uint16_t netid)
{
    return dnsManager_->CreateNetworkCache(netid);
}

int32_t NetManagerNative::BandwidthEnableDataSaver(bool enable)
{
    return bandwidthManager_->EnableDataSaver(enable);
}

int32_t NetManagerNative::BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes)
{
    return bandwidthManager_->SetIfaceQuota(ifName, bytes);
}

int32_t NetManagerNative::BandwidthRemoveIfaceQuota(const std::string &ifName)
{
    return bandwidthManager_->RemoveIfaceQuota(ifName);
}

int32_t NetManagerNative::BandwidthAddDeniedList(uint32_t uid)
{
    return bandwidthManager_->AddDeniedList(uid);
}

int32_t NetManagerNative::BandwidthRemoveDeniedList(uint32_t uid)
{
    return bandwidthManager_->RemoveDeniedList(uid);
}

int32_t NetManagerNative::BandwidthAddAllowedList(uint32_t uid)
{
    return bandwidthManager_->AddAllowedList(uid);
}

int32_t NetManagerNative::BandwidthRemoveAllowedList(uint32_t uid)
{
    return bandwidthManager_->RemoveAllowedList(uid);
}

int32_t NetManagerNative::FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    auto chainType = static_cast<NetManagerStandard::ChainType>(chain);
    return firewallManager_->SetUidsAllowedListChain(chainType, uids);
}

int32_t NetManagerNative::FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    auto chainType = static_cast<NetManagerStandard::ChainType>(chain);
    return firewallManager_->SetUidsDeniedListChain(chainType, uids);
}

int32_t NetManagerNative::FirewallEnableChain(uint32_t chain, bool enable)
{
    auto chainType = static_cast<NetManagerStandard::ChainType>(chain);
    return firewallManager_->EnableChain(chainType, enable);
}

int32_t NetManagerNative::FirewallSetUidRule(uint32_t chain, uint32_t uid, uint32_t firewallRule)
{
    auto chainType = static_cast<NetManagerStandard::ChainType>(chain);
    auto rule = static_cast<NetManagerStandard::FirewallRule>(firewallRule);
    return firewallManager_->SetUidRule(chainType, uid, rule);
}

void NetManagerNative::ShareDnsSet(uint16_t netId)
{
    dnsManager_->ShareDnsSet(netId);
}

void NetManagerNative::StartDnsProxyListen()
{
    dnsManager_->StartDnsProxyListen();
}

void NetManagerNative::StopDnsProxyListen()
{
    dnsManager_->StopDnsProxyListen();
}
} // namespace nmd
} // namespace OHOS
