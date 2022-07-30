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

#ifndef NETSYS_NATIVE_SERVICE_PROXY_H__
#define NETSYS_NATIVE_SERVICE_PROXY_H__

#include "i_netsys_service.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace NetsysNative {
class NetsysNativeServiceProxy : public IRemoteProxy<INetsysService> {
public:
    explicit NetsysNativeServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetsysService>(impl) {}
    ~NetsysNativeServiceProxy() override {}
    bool WriteInterfaceToken(MessageParcel &data);
    int32_t SetResolverConfigParcel(const DnsResolverParamsParcel &resolvParams) override;
    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                              const std::vector<std::string> &servers,
                              const std::vector<std::string> &domains) override;
    int32_t GetResolverConfig(const uint16_t netid, std::vector<std::string> &servers,
                              std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                              uint8_t &retryCount) override;
    int32_t CreateNetworkCache(const uint16_t netid) override;
    int32_t FlushNetworkCache(const uint16_t netid) override;
    int32_t DestroyNetworkCache(const uint16_t netid) override;
    int32_t Getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **result,
                        uint16_t netid) override;
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
    int32_t StartDhcpClient(const std::string &iface, bool bIpv6) override;
    int32_t InterfaceGetList(std::vector<std::string> &ifaces) override;
    int32_t StopDhcpClient(const std::string &iface, bool bIpv6) override;
    int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr) override;
    int32_t StopDhcpService(const std::string &iface) override;
    int32_t IpEnableForwarding(const std::string &requestor) override;
    int32_t IpDisableForwarding(const std::string &requestor) override;
    int32_t EnableNat(const std::string &downstreamIface, const std::string &upstreamIface) override;
    int32_t DisableNat(const std::string &downstreamIface, const std::string &upstreamIface) override;
    int32_t IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface) override;
    int32_t IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface) override;

private:
    static inline BrokerDelegator<NetsysNativeServiceProxy> delegator_;
};
} // namespace NetsysNative
} // namespace OHOS
#endif
