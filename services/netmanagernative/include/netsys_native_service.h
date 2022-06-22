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

#ifndef  NETSYS_NATIVE_SERVICE_H__
#define  NETSYS_NATIVE_SERVICE_H__

#include <mutex>
#include "dhcp_controller.h"
#include "dnsresolv.h"
#include "i_netsys_service.h"
#include "iremote_stub.h"
#include "net_manager_native.h"
#include "netsys_native_service_stub.h"
#include "netlink_manager.h"
#include "system_ability.h"

namespace OHOS {
namespace NetsysNative {
class NetsysNativeService : public SystemAbility, public NetsysNativeServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(NetsysNativeService);

public:
    explicit NetsysNativeService(int32_t saID, bool runOnCreate = true) : SystemAbility(saID, runOnCreate) {};
    ~NetsysNativeService() = default;

    void OnStart() override;
    void OnStop() override;

    int32_t SetResolverConfigParcel(const DnsresolverParamsParcel& resolvParams) override;
    int32_t SetResolverConfig(const DnsresolverParams &resolvParams) override;
    int32_t GetResolverConfig(const  uint16_t  netid,  std::vector<std::string> &servers,
           std::vector<std::string> &domains, nmd::DnsResParams &param) override;
    int32_t CreateNetworkCache(const uint16_t netid) override;
    int32_t FlushNetworkCache(const uint16_t netid) override;
    int32_t DestroyNetworkCache(const uint16_t netid) override;
    int32_t  Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
        struct addrinfo** result, uint16_t netid) override;
    int32_t InterfaceSetMtu(const std::string &interfaceName, int32_t mtu) override;
    int32_t InterfaceGetMtu(const std::string &interfaceName) override;

    int32_t RegisterNotifyCallback(sptr<INotifyCallback> &callback) override;

    int32_t NetworkAddRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
        const std::string &nextHop) override;
    int32_t NetworkRemoveRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
        const std::string &nextHop) override;
    int32_t NetworkAddRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) override;
    int32_t NetworkRemoveRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) override;
    int32_t NetworkSetDefault(int32_t netId) override;
    int32_t NetworkGetDefault() override;
    int32_t NetworkClearDefault() override;
    int32_t GetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
        const std::string &parameter, std::string  &value) override;
    int32_t SetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
        const std::string &parameter, std::string &value) override;
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) override;
    int32_t InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
        int32_t prefixLength) override;
    int32_t InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
        int32_t prefixLength) override;
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkDestroy(int32_t netId) override;
    int32_t GetFwmarkForNetwork(int32_t netId,       MarkMaskParcel &markMaskParcel) override;
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
private:
    NetsysNativeService();
    bool Init();

    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    ServiceRunningState state_ {ServiceRunningState::STATE_STOPPED};

    static sptr<NetsysNativeService> instance_;

    std::unique_ptr<OHOS::nmd::NetManagerNative> netsysService_ = nullptr;
    std::unique_ptr<OHOS::nmd::NetlinkManager> manager_ = nullptr;
    std::unique_ptr<OHOS::nmd::DhcpController> dhcpController_ = nullptr;

    sptr<INotifyCallback> notifyCallback_ = nullptr;

    std::mutex instanceLock_;
    const int32_t startTime_ = 1900;
    const int32_t extraMonth_ = 1;
};
} // namespace NetsysNative
} // namespace OHOS
#endif // NETSYS_NATIVE_SERVICE_H__
