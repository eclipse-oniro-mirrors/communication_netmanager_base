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
#ifndef I_NETSYS_SERVICE_H__
#define I_NETSYS_SERVICE_H__

#include <string>
#include <netdb.h>
#include "iremote_broker.h"
#include "dnsresolver_params_parcel.h"
#include "net_manager_native.h"
#include "i_notify_callback.h"

namespace OHOS {
namespace NetsysNative {
using namespace nmd;
class INetsysService : public IRemoteBroker {
public:
    enum {
        NETSYS_SET_RESOLVER_CONFIG_PARCEL,
        NETSYS_SET_RESOLVER_CONFIG,
        NETSYS_GET_RESOLVER_CONFIG,
        NETSYS_CREATE_NETWORK_CACHE,
        NETSYS_FLUSH_NETWORK_CACHE,
        NETSYS_DESTROY_NETWORK_CACHE,
        NETSYS_GET_ADDR_INFO,
        NETSYS_INTERFACE_SET_MTU,
        NETSYS_INTERFACE_GET_MTU,
        NETSYS_REGISTER_NOTIFY_CALLBACK,
        NETSYS_NETWORK_ADD_ROUTE,
        NETSYS_NETWORK_REMOVE_ROUTE,
        NETSYS_NETWORK_ADD_ROUTE_PARCEL,
        NETSYS_NETWORK_REMOVE_ROUTE_PARCEL,
        NETSYS_NETWORK_SET_DEFAULT,
        NETSYS_NETWORK_GET_DEFAULT,
        NETSYS_NETWORK_CLEAR_DEFAULT,
        NETSYS_GET_PROC_SYS_NET,
        NETSYS_SET_PROC_SYS_NET,
        NETSYS_NETWORK_CREATE_PHYSICAL,
        NETSYS_INTERFACE_ADD_ADDRESS,
        NETSYS_INTERFACE_DEL_ADDRESS,
        NETSYS_NETWORK_ADD_INTERFACE,
        NETSYS_NETWORK_REMOVE_INTERFACE,
        NETSYS_NETWORK_DESTROY,
        NETSYS_GET_FWMARK_FOR_NETWORK,
        NETSYS_INTERFACE_SET_CONFIG,
        NETSYS_INTERFACE_GET_CONFIG,
        NETSYS_INTERFACE_GET_LIST,
        NETSYS_START_DHCP_CLIENT,
        NETSYS_STOP_DHCP_CLIENT,
        NETSYS_START_DHCP_SERVICE,
        NETSYS_STOP_DHCP_SERVICE,
    };

    virtual int32_t SetResolverConfigParcel(const DnsresolverParamsParcel& resolvParams) = 0;
    virtual int32_t SetResolverConfig(const DnsresolverParams &resolvParams) = 0;
    virtual int32_t GetResolverConfig(const  uint16_t  netid,  std::vector<std::string> &servers,
           std::vector<std::string> &domains, nmd::DnsResParams &param)=0;
    virtual int32_t CreateNetworkCache(const uint16_t netid) = 0;
    virtual int32_t FlushNetworkCache(const uint16_t netid) = 0;
    virtual int32_t DestroyNetworkCache(const uint16_t netid) = 0;
    virtual int32_t Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
        struct addrinfo** result, uint16_t netid) = 0;
    virtual int32_t InterfaceSetMtu(const std::string &interfaceName, int mtu) = 0;
    virtual int32_t InterfaceGetMtu(const std::string &interfaceName) = 0;

    virtual int32_t RegisterNotifyCallback(sptr<INotifyCallback> &callback) = 0;

    virtual int32_t NetworkAddRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
        const std::string &nextHop) = 0;
    virtual int32_t NetworkRemoveRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
        const std::string &nextHop) = 0;
    virtual int32_t NetworkAddRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) = 0;
    virtual int32_t NetworkRemoveRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) = 0;
    virtual int32_t NetworkSetDefault(int32_t netId) = 0;
    virtual int32_t NetworkGetDefault() = 0;
    virtual int32_t NetworkClearDefault() = 0;
    virtual int32_t GetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
        const std::string &parameter, std::string  &value) = 0;
    virtual int32_t SetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
        const std::string &parameter, std::string  &value) = 0;
    virtual int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) = 0;
    virtual int32_t InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
        int32_t prefixLength) = 0;
    virtual int32_t InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
        int32_t prefixLength) = 0;
    virtual int32_t NetworkAddInterface(int32_t netId, const std::string &iface) = 0;
    virtual int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) = 0;
    virtual int32_t NetworkDestroy(int32_t netId) = 0;
    virtual int32_t GetFwmarkForNetwork(int32_t netId, MarkMaskParcel &markMaskParcel) = 0;
    virtual int32_t InterfaceSetConfig(const InterfaceConfigurationParcel &cfg) = 0;
    virtual int32_t InterfaceGetConfig(InterfaceConfigurationParcel &cfg) = 0;
    virtual int32_t InterfaceGetList(std::vector<std::string> &ifaces) = 0;
    virtual int32_t StartDhcpClient(const std::string &iface, bool bIpv6) = 0;
    virtual int32_t StopDhcpClient(const std::string &iface, bool bIpv6) = 0;
    virtual int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr) = 0;
    virtual int32_t StopDhcpService(const std::string &iface) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetsysNative.INetsysService")
};
} // namespace NetsysNative
} // namespace OHOS
#endif