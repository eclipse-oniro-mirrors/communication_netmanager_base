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

#ifndef NETSYS_NATIVE_SERVICE_H
#define NETSYS_NATIVE_SERVICE_H

#include <mutex>

#include "system_ability.h"
#include "system_ability_status_change_stub.h"

#include "dhcp_controller.h"
#include "fwmark_network.h"
#include "i_netsys_service.h"
#include "iremote_stub.h"
#include "net_manager_native.h"
#include "netlink_manager.h"
#include "netsys_native_service_stub.h"
#include "sharing_manager.h"

namespace OHOS {
namespace NetsysNative {
class NetsysNativeService : public SystemAbility, public NetsysNativeServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(NetsysNativeService);

public:
    explicit NetsysNativeService(int32_t saID, bool runOnCreate = true) : SystemAbility(saID, runOnCreate){};
    ~NetsysNativeService() = default;

    void OnStart() override;
    void OnStop() override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                              const std::vector<std::string> &servers,
                              const std::vector<std::string> &domains) override;
    int32_t GetResolverConfig(const uint16_t netId, std::vector<std::string> &servers,
                              std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                              uint8_t &retryCount) override;
    int32_t CreateNetworkCache(const uint16_t netId) override;
    int32_t DestroyNetworkCache(const uint16_t netId) override;
    int32_t GetAddrInfo(const std::string node, const std::string service, const struct addrinfo *hints,
                        uint16_t netId, addrinfo **result) override;
    int32_t InterfaceSetMtu(const std::string &interfaceName, int32_t mtu) override;
    int32_t InterfaceGetMtu(const std::string &interfaceName) override;

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
    int32_t GetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname, const std::string &parameter,
                          std::string &value) override;
    int32_t SetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname, const std::string &parameter,
                          std::string &value) override;
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) override;
    int32_t InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
                                int32_t prefixLength) override;
    int32_t InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
                                int32_t prefixLength) override;
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkDestroy(int32_t netId) override;
    int32_t GetFwmarkForNetwork(int32_t netId, MarkMaskParcel &markMaskParcel) override;
    int32_t InterfaceSetConfig(const InterfaceConfigurationParcel &cfg) override;
    int32_t InterfaceGetConfig(InterfaceConfigurationParcel &cfg) override;
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
    int32_t FirewallSetUidRule(uint32_t chain, uint32_t uid, uint32_t firewallRule) override;
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

    std::unique_ptr<OHOS::nmd::NetManagerNative> netsysService_ = nullptr;
    std::unique_ptr<OHOS::nmd::NetlinkManager> manager_ = nullptr;
    std::unique_ptr<OHOS::nmd::DhcpController> dhcpController_ = nullptr;
    std::unique_ptr<OHOS::nmd::FwmarkNetwork> fwmarkNetwork_ = nullptr;
    std::unique_ptr<OHOS::nmd::SharingManager> sharingManager_ = nullptr;

    sptr<INotifyCallback> notifyCallback_ = nullptr;

    std::mutex instanceLock_;
    bool hasSARemoved_ = false;
};
} // namespace NetsysNative
} // namespace OHOS
#endif // NETSYS_NATIVE_SERVICE_H
