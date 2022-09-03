/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_NET_MANAGER_NATIVE_H__
#define INCLUDE_NET_MANAGER_NATIVE_H__

#include <memory>
#include <string>
#include <vector>
#include "bandwidth_manager.h"
#include "conn_manager.h"
#include "dns_manager.h"
#include "firewall_manager.h"
#include "interface_manager.h"
#include "interface_type.h"
#include "route_manager.h"
#include "route_type.h"
#include "sharing_manager.h"

namespace OHOS {
namespace nmd {
class NetManagerNative {
public:
    NetManagerNative();
    ~NetManagerNative() = default;

    static void GetOriginInterfaceIndex();
    static std::vector<unsigned int> GetCurrentInterfaceIndex();
    static void UpdateInterfaceIndex(unsigned int infIndex);

    void Init();

    int NetworkCreatePhysical(int netId, int permission);
    int NetworkDestroy(int netId);
    int NetworkAddInterface(int netId, std::string iface);
    int NetworkRemoveInterface(int netId, std::string iface);

    MarkMaskParcel GetFwmarkForNetwork(int netId);
    int NetworkAddRoute(int netId, std::string ifName, std::string destination, std::string nextHop);
    int NetworkRemoveRoute(int netId, std::string ifName, std::string destination, std::string nextHop);
    int NetworkGetDefault();
    int NetworkSetDefault(int netId);
    int NetworkClearDefault();
    int NetworkSetPermissionForNetwork(int netId, NetworkPermission permission);
    std::vector<std::string> InterfaceGetList();

    int SetProcSysNet(int32_t ipversion, int32_t which, const std::string ifname, const std::string parameter,
                      const std::string value);
    int GetProcSysNet(int32_t ipversion, int32_t which, const std::string ifname, const std::string parameter,
                      std::string *value);

    nmd::InterfaceConfigurationParcel InterfaceGetConfig(std::string ifName);
    void InterfaceSetConfig(InterfaceConfigurationParcel cfg);
    void InterfaceClearAddrs(const std::string ifName);
    int InterfaceGetMtu(std::string ifName);
    int InterfaceSetMtu(std::string ifName, int mtuValue);
    int InterfaceAddAddress(std::string ifName, std::string addrString, int prefixLength);
    int InterfaceDelAddress(std::string ifName, std::string addrString, int prefixLength);

    int NetworkAddRouteParcel(int netId, RouteInfoParcel routeInfo);
    int NetworkRemoveRouteParcel(int netId, RouteInfoParcel routeInfo);

    long GetCellularRxBytes();
    long GetCellularTxBytes();
    long GetAllRxBytes();
    long GetAllTxBytes();
    long GetUidTxBytes(int uid);
    long GetUidRxBytes(int uid);
    long GetIfaceRxBytes(std::string interfaceName);
    long GetIfaceTxBytes(std::string interfaceName);
    long GetTetherRxBytes();
    long GetTetherTxBytes();
    int32_t IpEnableForwarding(const std::string &requester);
    int32_t IpDisableForwarding(const std::string &requester);
    int32_t EnableNat(const std::string &downstreamIface, const std::string &upstreamIface);
    int32_t DisableNat(const std::string &downstreamIface, const std::string &upsteramIface);
    int32_t IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface);
    int32_t IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface);

    int32_t DnsSetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                 const std::vector<std::string> &servers, const std::vector<std::string> &domains);
    int32_t DnsGetResolverConfig(uint16_t netId, std::vector<std::string> &servers, std::vector<std::string> &domains,
                                 uint16_t &baseTimeoutMsec, uint8_t &retryCount);
    int32_t DnsCreateNetworkCache(uint16_t netid);
    int32_t BandwidthEnableDataSaver(bool enable);
    int32_t BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes);
    int32_t BandwidthRemoveIfaceQuota(const std::string &ifName);
    int32_t BandwidthAddDeniedList(uint32_t uid);
    int32_t BandwidthRemoveDeniedList(uint32_t uid);
    int32_t BandwidthAddAllowedList(uint32_t uid);
    int32_t BandwidthRemoveAllowedList(uint32_t uid);

    int32_t FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids);
    int32_t FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids);
    int32_t FirewallEnableChain(uint32_t chain, bool enable);
    int32_t FirewallSetUidRule(uint32_t chain, uint32_t uid, uint32_t firewallRule);
    void ShareDnsSet(uint16_t netId);
    void StartDnsProxyListen();
    void StopDnsProxyListen();
    void GetDumpInfo(std::string &infos);

private:
    std::shared_ptr<BandwidthManager> bandwidthManager_;
    std::shared_ptr<ConnManager> connManager_;
    std::shared_ptr<FirewallManager> firewallManager_;
    std::shared_ptr<RouteManager> routeManager_;
    std::shared_ptr<InterfaceManager> interfaceManager_;
    std::shared_ptr<SharingManager> sharingManager_ = nullptr;
    std::shared_ptr<DnsManager> dnsManager_;
    static std::vector<unsigned int> interfaceIdex;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NET_MANAGER_NATIVE_H__
