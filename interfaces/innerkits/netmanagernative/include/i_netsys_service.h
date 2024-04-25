/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#ifndef I_NETSYS_SERVICE_H
#define I_NETSYS_SERVICE_H

#include <netdb.h>
#include <string>
#include <set>

#include "dns_config_client.h"
#include "i_net_diag_callback.h"
#include "i_notify_callback.h"
#include "i_net_dns_result_callback.h"
#include "i_net_dns_health_callback.h"
#include "interface_type.h"
#include "iremote_broker.h"
#include "net_stats_info.h"
#include "network_sharing.h"
#include "netsys_ipc_interface_code.h"
#include "route_type.h"
#include "uid_range.h"
#include "netsys_access_policy.h"
#include "net_all_capabilities.h"

namespace OHOS {
namespace NetsysNative {
using namespace nmd;
using namespace OHOS::NetManagerStandard;
class INetsysService : public IRemoteBroker {
public:
    virtual int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                      const std::vector<std::string> &servers,
                                      const std::vector<std::string> &domains) = 0;
    virtual int32_t GetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
                                      std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                      uint8_t &retryCount) = 0;
    virtual int32_t CreateNetworkCache(uint16_t netId) = 0;
    virtual int32_t DestroyNetworkCache(uint16_t netId) = 0;
    virtual int32_t GetAddrInfo(const std::string &hostName, const std::string &serverName, const AddrInfo &hints,
                                uint16_t netId, std::vector<AddrInfo> &res) = 0;
    virtual int32_t SetInterfaceMtu(const std::string &interfaceName, int mtu) = 0;
    virtual int32_t GetInterfaceMtu(const std::string &interfaceName) = 0;

    virtual int32_t SetTcpBufferSizes(const std::string &tcpBufferSizes) = 0;

    virtual int32_t RegisterNotifyCallback(sptr<INotifyCallback> &callback) = 0;
    virtual int32_t UnRegisterNotifyCallback(sptr<INotifyCallback> &callback) = 0;

    virtual int32_t NetworkAddRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
                                    const std::string &nextHop) = 0;
    virtual int32_t NetworkRemoveRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
                                       const std::string &nextHop) = 0;
    virtual int32_t NetworkAddRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) = 0;
    virtual int32_t NetworkRemoveRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) = 0;
    virtual int32_t NetworkSetDefault(int32_t netId) = 0;
    virtual int32_t NetworkGetDefault() = 0;
    virtual int32_t NetworkClearDefault() = 0;
    virtual int32_t GetProcSysNet(int32_t family, int32_t which, const std::string &ifname,
                                  const std::string &parameter, std::string &value) = 0;
    virtual int32_t SetProcSysNet(int32_t family, int32_t which, const std::string &ifname,
                                  const std::string &parameter, std::string &value) = 0;
    virtual int32_t SetInternetPermission(uint32_t uid, uint8_t allow, uint8_t isBroker) = 0;
    virtual int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) = 0;
    virtual int32_t NetworkCreateVirtual(int32_t netId, bool hasDns) = 0;
    virtual int32_t NetworkAddUids(int32_t netId, const std::vector<UidRange> &uidRanges) = 0;
    virtual int32_t NetworkDelUids(int32_t netId, const std::vector<UidRange> &uidRanges) = 0;
    virtual int32_t AddInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                        int32_t prefixLength) = 0;
    virtual int32_t DelInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                        int32_t prefixLength) = 0;
    virtual int32_t InterfaceSetIpAddress(const std::string &ifaceName, const std::string &ipAddress) = 0;
    virtual int32_t InterfaceSetIffUp(const std::string &ifaceName) = 0;
    virtual int32_t NetworkAddInterface(int32_t netId, const std::string &iface) = 0;
    virtual int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) = 0;
    virtual int32_t NetworkDestroy(int32_t netId) = 0;
    virtual int32_t GetFwmarkForNetwork(int32_t netId, MarkMaskParcel &markMaskParcel) = 0;
    virtual int32_t SetInterfaceConfig(const InterfaceConfigurationParcel &cfg) = 0;
    virtual int32_t GetInterfaceConfig(InterfaceConfigurationParcel &cfg) = 0;
    virtual int32_t InterfaceGetList(std::vector<std::string> &ifaces) = 0;
    virtual int32_t StartDhcpClient(const std::string &iface, bool bIpv6) = 0;
    virtual int32_t StopDhcpClient(const std::string &iface, bool bIpv6) = 0;
    virtual int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr) = 0;
    virtual int32_t StopDhcpService(const std::string &iface) = 0;
    virtual int32_t IpEnableForwarding(const std::string &requestor) = 0;
    virtual int32_t IpDisableForwarding(const std::string &requestor) = 0;
    virtual int32_t EnableNat(const std::string &downstreamIface, const std::string &upstreamIface) = 0;
    virtual int32_t DisableNat(const std::string &downstreamIface, const std::string &upstreamIface) = 0;
    virtual int32_t IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface) = 0;
    virtual int32_t IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface) = 0;
    virtual int32_t BandwidthAddAllowedList(uint32_t uid) = 0;
    virtual int32_t BandwidthRemoveAllowedList(uint32_t uid) = 0;
    virtual int32_t BandwidthEnableDataSaver(bool enable) = 0;
    virtual int32_t BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes) = 0;
    virtual int32_t BandwidthAddDeniedList(uint32_t uid) = 0;
    virtual int32_t BandwidthRemoveDeniedList(uint32_t uid) = 0;
    virtual int32_t BandwidthRemoveIfaceQuota(const std::string &ifName) = 0;
    virtual int32_t FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids) = 0;
    virtual int32_t FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids) = 0;
    virtual int32_t FirewallEnableChain(uint32_t chain, bool enable) = 0;
    virtual int32_t FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids, uint32_t firewallRule) = 0;
    virtual int32_t ShareDnsSet(uint16_t netId) = 0;
    virtual int32_t StartDnsProxyListen() = 0;
    virtual int32_t StopDnsProxyListen() = 0;
    virtual int32_t GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                             NetworkSharingTraffic &traffic) = 0;
    virtual int32_t GetTotalStats(uint64_t &stats, uint32_t type) = 0;
    virtual int32_t GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid) = 0;
    virtual int32_t GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName) = 0;
    virtual int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) = 0;
    virtual int32_t GetAllContainerStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) = 0;
    virtual int32_t SetIptablesCommandForRes(const std::string &cmd, std::string &respond) = 0;
    virtual int32_t NetDiagPingHost(const NetDiagPingOption &pingOption, const sptr<INetDiagCallback> &callback) = 0;
    virtual int32_t NetDiagGetRouteTable(std::list<NetDiagRouteTable> &routeTables) = 0;
    virtual int32_t NetDiagGetSocketsInfo(NetDiagProtocolType socketType, NetDiagSocketsInfo &socketsInfo) = 0;
    virtual int32_t NetDiagGetInterfaceConfig(std::list<NetDiagIfaceConfig> &configs, const std::string &ifaceName) = 0;
    virtual int32_t NetDiagUpdateInterfaceConfig(const NetDiagIfaceConfig &config, const std::string &ifaceName,
                                                 bool add) = 0;
    virtual int32_t NetDiagSetInterfaceActiveState(const std::string &ifaceName, bool up) = 0;
    virtual int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                 const std::string &ifName) = 0;
    virtual int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                 const std::string &ifName) = 0;
    virtual int32_t RegisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback, uint32_t delay) = 0;
    virtual int32_t UnregisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback) = 0;
    virtual int32_t RegisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback) = 0;
    virtual int32_t UnregisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback) = 0;
    virtual int32_t GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie) = 0;
    virtual int32_t GetNetworkSharingType(std::set<uint32_t>& sharingTypeIsOn) = 0;
    virtual int32_t UpdateNetworkSharingType(uint32_t type, bool isOpen) = 0;
    virtual int32_t SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on) = 0;
    virtual int32_t SetEnableIpv6(const std::string &interfaceName, const uint32_t on) = 0;
    virtual int32_t SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag) = 0;
    virtual int32_t DeleteNetworkAccessPolicy(uint32_t uid) = 0;
    virtual int32_t NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetsysNative.INetsysService")
};
} // namespace NetsysNative
} // namespace OHOS
#endif // I_NETSYS_SERVICE_H
