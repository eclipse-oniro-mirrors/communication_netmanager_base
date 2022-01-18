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
#ifndef  NETD_NATIVE_SERVICE_PROXY_H__
#define  NETD_NATIVE_SERVICE_PROXY_H__

#include "i_netd_service.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace NetdNative {
class NetdNativeServiceProxy : public IRemoteProxy<INetdService> {
public:
    explicit NetdNativeServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetdService>(impl) {}
    ~NetdNativeServiceProxy() override {}
    bool WriteInterfaceToken(MessageParcel &data);
    int32_t SetResolverConfigParcel(const DnsresolverParamsParcel& resolvParams) override;
    int32_t SetResolverConfig(const dnsresolver_params &resolvParams) override;
    int32_t GetResolverConfig(const  uint16_t  netid,  std::vector<std::string> &servers,
        std::vector<std::string> &domains, nmd::dns_res_params &param)  override;
    int32_t CreateNetworkCache(const uint16_t netid) override;
    int32_t FlushNetworkCache(const uint16_t netid) override;
    int32_t DestoryNetworkCache(const uint16_t netid) override;
    int32_t Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
        struct addrinfo** result, uint16_t netid) override;
    int32_t InterfaceSetMtu(const std::string &interfaceName, int32_t mtu) override;
    int32_t InterfaceGetMtu(const std::string &interfaceName) override;

    int32_t RegisterNotifyCallback(sptr<INotifyCallback> &callback) override;

    int32_t NetworkAddRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
        const std::string &nextHop) override;
    int32_t NetworkRemoveRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
        const std::string &nextHop) override;
    int32_t NetworkAddRouteParcel(int32_t netId, const route_info_parcel &routeInfo) override;
    int32_t NetworkRemoveRouteParcel(int32_t netId, const route_info_parcel &routeInfo) override;
    int32_t NetworkSetDefault(int32_t netId) override;
    int32_t NetworkGetDefault() override;
    int32_t NetworkClearDefault() override;
    int32_t GetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
        const std::string &parameter, std::string  &value) override;
    int32_t SetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
        const std::string &parameter, std::string  &value) override;
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) override;
    int32_t InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
        int32_t prefixLength) override;
    int32_t InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
        int32_t prefixLength) override;
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) override;
    int32_t NetworkDestroy(int32_t netId) override;
    int32_t GetFwmarkForNetwork(int32_t netId, mark_mask_parcel &markMaskParcel) override;
    int32_t InterfaceSetConfig(const interface_configuration_parcel &cfg) override;
    int32_t InterfaceGetConfig(interface_configuration_parcel &cfg) override ;
    int32_t StartDhcpClient(const std::string &iface, bool bIpv6) override;
    int32_t StopDhcpClient(const std::string &iface, bool bIpv6) override;
    int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr) override;
    int32_t StopDhcpService(const std::string &iface) override;
private:
    static inline BrokerDelegator<NetdNativeServiceProxy> delegator_;
};
} // namespace NetdNative
} // namespace OHOS
#endif