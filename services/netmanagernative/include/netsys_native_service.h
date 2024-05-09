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

#ifndef NETSYS_NATIVE_SERVICE_H
#define NETSYS_NATIVE_SERVICE_H

#include <mutex>

#include "system_ability.h"
#include "system_ability_status_change_stub.h"

#include "bpf_stats.h"
#include "dhcp_controller.h"
#include "fwmark_network.h"
#include "i_netsys_service.h"
#include "iremote_stub.h"
#include "net_diag_wrapper.h"
#include "net_manager_native.h"
#include "netlink_manager.h"
#include "netsys_native_service_stub.h"
#include "sharing_manager.h"
#include "netsys_access_policy.h"

namespace OHOS {
namespace NetsysNative {
class NetsysNativeService : public SystemAbility, public NetsysNativeServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(NetsysNativeService);

public:
    explicit NetsysNativeService(int32_t saID, bool runOnCreate = true) : SystemAbility(saID, runOnCreate){};
    ~NetsysNativeService() override = default;

    void OnStart() override;
    void OnStop() override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                              const std::vector<std::string> &servers,
                              const std::vector<std::string> &domains) override;
    int32_t GetResolverConfig(uint16_t netId, std::vector<std::string> &servers, std::vector<std::string> &domains,
                              uint16_t &baseTimeoutMsec, uint8_t &retryCount) override;
    int32_t CreateNetworkCache(uint16_t netId) override;
    int32_t DestroyNetworkCache(uint16_t netId) override;
    int32_t GetAddrInfo(const std::string &hostName, const std::string &serverName, const AddrInfo &hints,
                        uint16_t netId, std::vector<AddrInfo> &res) override;
    int32_t SetInterfaceMtu(const std::string &interfaceName, int32_t mtu) override;
    int32_t GetInterfaceMtu(const std::string &interfaceName) override;

    int32_t SetTcpBufferSizes(const std::string &tcpBufferSizes) override;

    int32_t RegisterNotifyCallback(sptr<INotifyCallback> &callback) override;
    int32_t UnRegisterNotifyCallback(sptr<INotifyCallback> &callback) override;

    int32_t NetworkAddRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
                            const std::string &nextHop) override;
    int32_t NetworkRemoveRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
                               const std::string &nextHop) override;
    int32_t NetworkAddRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) override;
    int32_t NetworkRemoveRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) override;
    int32_t NetworkSetDefault(int32_t netId) override;
    int32_t NetworkGetDefault() override;
    int32_t NetworkClearDefault() override;
    int32_t GetProcSysNet(int32_t family, int32_t which, const std::string &ifname, const std::string &parameter,
                          std::string &value) override;
    int32_t SetProcSysNet(int32_t family, int32_t which, const std::string &ifname, const std::string &parameter,
                          std::string &value) override;
    int32_t SetInternetPermission(uint32_t uid, uint8_t allow, uint8_t isBroker) override;
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) override;
    int32_t NetworkCreateVirtual(int32_t netId, bool hasDns) override;
    int32_t NetworkAddUids(int32_t netId, const std::vector<UidRange> &uidRanges) override;
    int32_t NetworkDelUids(int32_t netId, const std::vector<UidRange> &uidRanges) override;
    int32_t AddInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                int32_t prefixLength) override;
    int32_t DelInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                int32_t prefixLength) override;
    int32_t InterfaceSetIpAddress(const std::string &ifaceName, const std::string &ipAddress) override;
    int32_t InterfaceSetIffUp(const std::string &ifaceName) override;
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkDestroy(int32_t netId) override;
    int32_t GetFwmarkForNetwork(int32_t netId, MarkMaskParcel &markMaskParcel) override;
    int32_t SetInterfaceConfig(const InterfaceConfigurationParcel &cfg) override;
    int32_t GetInterfaceConfig(InterfaceConfigurationParcel &cfg) override;
    int32_t InterfaceGetList(std::vector<std::string> &ifaces) override;
    int32_t StartDhcpClient(const std::string &iface, bool bIpv6) override;
    int32_t StopDhcpClient(const std::string &iface, bool bIpv6) override;
    int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr) override;
    int32_t StopDhcpService(const std::string &iface) override;
    int32_t IpEnableForwarding(const std::string &requester) override;
    int32_t IpDisableForwarding(const std::string &requester) override;
    int32_t EnableNat(const std::string &downstreamIface, const std::string &upstreamIface) override;
    int32_t DisableNat(const std::string &downstreamIface, const std::string &upstreamIface) override;
    int32_t IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toiIface) override;
    int32_t IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toiIface) override;
    int32_t FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override;
    int32_t FirewallEnableChain(uint32_t chain, bool enable) override;
    int32_t FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids, uint32_t firewallRule) override;
    int32_t BandwidthEnableDataSaver(bool enable) override;
    int32_t BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes) override;
    int32_t BandwidthRemoveIfaceQuota(const std::string &ifName) override;
    int32_t FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override;
    int32_t BandwidthAddAllowedList(uint32_t uid) override;
    int32_t BandwidthRemoveAllowedList(uint32_t uid) override;
    int32_t BandwidthAddDeniedList(uint32_t uid) override;
    int32_t BandwidthRemoveDeniedList(uint32_t uid) override;
    int32_t ShareDnsSet(uint16_t netId) override;
    int32_t StartDnsProxyListen() override;
    int32_t StopDnsProxyListen() override;
    int32_t GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                     NetworkSharingTraffic &traffic) override;
    int32_t GetTotalStats(uint64_t &stats, uint32_t type) override;
    int32_t GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid) override;
    int32_t GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName) override;
    int32_t GetAllContainerStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override;
    int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override;
    int32_t SetIptablesCommandForRes(const std::string &cmd, std::string &respond) override;
    int32_t NetDiagPingHost(const NetDiagPingOption &pingOption, const sptr<INetDiagCallback> &callback) override;
    int32_t NetDiagGetRouteTable(std::list<NetDiagRouteTable> &routeTables) override;
    int32_t NetDiagGetSocketsInfo(NetDiagProtocolType socketType, NetDiagSocketsInfo &socketsInfo) override;
    int32_t NetDiagGetInterfaceConfig(std::list<NetDiagIfaceConfig> &configs, const std::string &ifaceName) override;
    int32_t NetDiagUpdateInterfaceConfig(const NetDiagIfaceConfig &config, const std::string &ifaceName,
                                         bool add) override;
    int32_t NetDiagSetInterfaceActiveState(const std::string &ifaceName, bool up) override;
    int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName) override;
    int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName) override;
    int32_t RegisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback, uint32_t timeStep) override;
    int32_t UnregisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback) override;
    int32_t RegisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback) override;
    int32_t UnregisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback) override;
    int32_t GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie) override;
    int32_t GetNetworkSharingType(std::set<uint32_t>& sharingTypeIsOn) override;
    int32_t UpdateNetworkSharingType(uint32_t type, bool isOpen) override;
    int32_t SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on) override;
    int32_t SetEnableIpv6(const std::string &interfaceName, const uint32_t on) override;

    int32_t SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag) override;
    int32_t DeleteNetworkAccessPolicy(uint32_t uid) override;
    int32_t NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes) override;
protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    NetsysNativeService();
    bool Init();
    void GetDumpMessage(std::string &message);
    void OnNetManagerRestart();

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    ServiceRunningState state_{ServiceRunningState::STATE_STOPPED};

    static sptr<NetsysNativeService> instance_;

    std::shared_ptr<IptablesWrapper> iptablesWrapper_ = nullptr;
    std::unique_ptr<OHOS::nmd::NetManagerNative> netsysService_ = nullptr;
    std::unique_ptr<OHOS::nmd::NetlinkManager> manager_ = nullptr;
    std::unique_ptr<OHOS::nmd::DhcpController> dhcpController_ = nullptr;
    std::unique_ptr<OHOS::nmd::FwmarkNetwork> fwmarkNetwork_ = nullptr;
    std::unique_ptr<OHOS::nmd::SharingManager> sharingManager_ = nullptr;
    std::unique_ptr<OHOS::NetManagerStandard::NetsysBpfStats> bpfStats_ = nullptr;
    std::shared_ptr<OHOS::nmd::NetDiagWrapper> netDiagWrapper = nullptr;

    sptr<INotifyCallback> notifyCallback_ = nullptr;

    std::mutex instanceLock_;
    bool hasSARemoved_ = false;
    std::set<uint32_t> sharingTypeIsOn_;
};
} // namespace NetsysNative
} // namespace OHOS
#endif // NETSYS_NATIVE_SERVICE_H
