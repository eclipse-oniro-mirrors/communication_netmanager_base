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

#include "netd_native_service_proxy.h"
#include "netnative_log_wrapper.h"
namespace OHOS {
namespace NetdNative {
using namespace std;

int32_t NetdNativeServiceProxy::SetResolverConfigParcel(const DnsresolverParamsParcel& resolvParams)
{
    NETNATIVE_LOGI("Begin to SetResolverConfig %{public}d", resolvParams.retryCount_);
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteParcelable(&resolvParams);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_SET_RESOLVER_CONFIG_PARCEL, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::SetResolverConfig(const dnsresolver_params &resolvParams)
{
    NETNATIVE_LOGI("Begin to SetResolverConfig %{public}d", resolvParams.retryCount);
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());

    if (!data.WriteUint16(resolvParams.netId)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteUint16(resolvParams.baseTimeoutMsec)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteUint8(resolvParams.retryCount)) {
        return ERR_FLATTEN_OBJECT;
    }

    int32_t vServerSize1 = resolvParams.servers.size();
    if (!data.WriteInt32(vServerSize1)) {
        return ERR_FLATTEN_OBJECT;
    }
    std::vector<std::string> vServers;
    vServers.assign(resolvParams.servers.begin(), resolvParams.servers.end());
    NETNATIVE_LOGI("PROXY: SetResolverConfig Write Servers  String_SIZE: %{public}d",
        static_cast<int32_t>(vServers.size()));
    for (std::vector<std::string>::iterator it = vServers.begin(); it != vServers.end(); ++it) {
        data.WriteString(*it);
    }

    int vDomainSize1 = resolvParams.domains.size();
    if (!data.WriteInt32(vDomainSize1)) {
        return ERR_FLATTEN_OBJECT;
    }

    std::vector<std::string> vDomains;
    vDomains.assign(resolvParams.domains.begin(), resolvParams.domains.end());
    NETNATIVE_LOGI("PROXY: InterfaceSetConfig Write Domains String_SIZE: %{public}d",
        static_cast<int32_t>(vDomains.size()));
    for (std::vector<std::string>::iterator it = vDomains.begin(); it != vDomains.end(); ++it) {
        data.WriteString(*it);
    }

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_SET_RESOLVER_CONFIG, data, reply, option);
    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::GetResolverConfig(const uint16_t netid, std::vector<std::string> &servers,
    std::vector<std::string> &domains, nmd::dns_res_params &param)
{
    NETNATIVE_LOGI("PROXY:Begin to GetResolverConfig %{public}d", netid);
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteUint16(netid);
    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_GET_RESOLVER_CONFIG, data, reply, option);
    int result = reply.ReadInt32();
    reply.ReadUint16(param.baseTimeoutMsec);
    reply.ReadUint8(param.retryCount);
    int32_t vServerSize = reply.ReadInt32();
    std::vector<std::string> vecString;
    for (int i = 0; i<vServerSize; i++) {
        vecString.push_back(reply.ReadString());
    }
    if (vServerSize > 0) {
        servers.assign(vecString.begin(), vecString.end()) ;
    }
    int32_t vDomainSize = reply.ReadInt32();
    std::vector<std::string> vecDomain;
    for (int i = 0; i < vDomainSize; i++) {
        vecDomain.push_back(reply.ReadString());
    }
    if (vDomainSize > 0) {
        domains.assign(vecDomain.begin(), vecDomain.end()) ;
    }
    NETNATIVE_LOGI("Begin to GetResolverConfig %{public}d", result);
    return  result ;
}

int32_t NetdNativeServiceProxy::CreateNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("Begin to CreateNetworkCache");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteUint16(netid);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_CREATE_NETWORK_CACHE, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::FlushNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("Begin to FlushNetworkCache");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteUint16(netid);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_FLUSH_NETWORK_CACHE, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::DestoryNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("Begin to DestoryNetworkCache");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteUint16(netid);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_DESTORY_NETWORK_CACHE, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
    struct addrinfo** result, const uint16_t  netid)
{
#ifdef SYS_DNS
    return getaddrinfo(node, service, hints, result);
#else
    NETNATIVE_LOGI("Begin to Getaddrinfo");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteCString(node);
    data.WriteCString(service);
    data.WriteInt16(hints->ai_family);
    data.WriteInt16(hints->ai_socktype);
    data.WriteInt16(hints->ai_flags);
    data.WriteInt16(hints->ai_protocol);
    data.WriteUint16(netid);
    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_GET_ADDR_INFO, data, reply, option);
    return reply.ReadInt32();
#endif
}

int32_t NetdNativeServiceProxy::InterfaceSetMtu(const std::string &interfaceName, int32_t mtu)
{
    NETNATIVE_LOGI("Begin to InterfaceSetMtu");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(interfaceName);
    data.WriteInt32(mtu);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_INTERFACE_SET_MTU, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::InterfaceGetMtu(const std::string &interfaceName)
{
    NETNATIVE_LOGI("Begin to InterfaceGetMtu");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(interfaceName);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_INTERFACE_GET_MTU, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::RegisterNotifyCallback(sptr<INotifyCallback> &callback)
{
    NETNATIVE_LOGI("Begin to RegisterNotifyCallback");
    MessageParcel data;
    if (callback == nullptr) {
        NETNATIVE_LOGE("The parameter of callback is nullptr");
        return ERR_NULL_OBJECT;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        NETNATIVE_LOGE("WriteInterfaceToken failed");
        return false;
    }
    data.WriteRemoteObject(callback->AsObject().GetRefPtr());

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_REGISTER_NOTIFY_CALLBACK, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkAddRoute(int32_t netId, const std::string &interfaceName,
    const std::string &destination, const std::string &nextHop)
{
    NETNATIVE_LOGI("Begin to NetworkAddRoute");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteString(interfaceName);
    data.WriteString(destination);
    data.WriteString(nextHop);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_ADD_ROUTE, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkRemoveRoute(int32_t netId, const std::string &interfaceName,
    const std::string &destination, const std::string &nextHop)
{
    NETNATIVE_LOGI("Begin to NetworkRemoveRoute");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteString(interfaceName);
    data.WriteString(destination);
    data.WriteString(nextHop);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_REMOVE_ROUTE, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkAddRouteParcel(int32_t netId, const route_info_parcel &routeInfo)
{
    NETNATIVE_LOGI("Begin to NetworkAddRouteParcel");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteString(routeInfo.ifName);
    data.WriteString(routeInfo.destination);
    data.WriteString(routeInfo.nextHop);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_ADD_ROUTE_PARCEL, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkRemoveRouteParcel(int32_t netId, const route_info_parcel &routeInfo)
{
    NETNATIVE_LOGI("Begin to NetworkRemoveRouteParcel");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteString(routeInfo.ifName);
    data.WriteString(routeInfo.destination);
    data.WriteString(routeInfo.nextHop);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_REMOVE_ROUTE_PARCEL, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkSetDefault(int32_t netId)
{
    NETNATIVE_LOGI("Begin to NetworkSetDefault");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_SET_DEFAULT, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkGetDefault()
{
    NETNATIVE_LOGI("Begin to NetworkGetDefault");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_GET_DEFAULT, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETNATIVE_LOGI("Begin to NetworkCreatePhysical");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteInt32(permission);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_CREATE_PHYSICAL, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    NETNATIVE_LOGI("Begin to InterfaceAddAddress");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(interfaceName);
    data.WriteString(addrString);
    data.WriteInt32(prefixLength);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_INTERFACE_ADD_ADDRESS, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    NETNATIVE_LOGI("Begin to InterfaceDelAddress");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(interfaceName);
    data.WriteString(addrString);
    data.WriteInt32(prefixLength);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_INTERFACE_DEL_ADDRESS, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETNATIVE_LOGI("Begin to NetworkAddInterface");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteString(iface);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_ADD_INTERFACE, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETNATIVE_LOGI("Begin to NetworkRemoveInterface");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteString(iface);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_REMOVE_INTERFACE, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::NetworkDestroy(int32_t netId)
{
    NETNATIVE_LOGI("Begin to NetworkDestroy");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_NETWORK_DESTROY, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::GetFwmarkForNetwork(int32_t netId, mark_mask_parcel &markMaskParcel)
{
    NETNATIVE_LOGI("Begin to GetFwmarkForNetwork");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(netId);
    data.WriteInt32(markMaskParcel.mark);
    data.WriteInt32(markMaskParcel.mask);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_GET_FWMARK_FOR_NETWORK, data, reply, option);

    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::InterfaceSetConfig(const interface_configuration_parcel &cfg)
{
    NETNATIVE_LOGI("Begin to InterfaceSetConfig");
    MessageParcel data;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(cfg.ifName);
    data.WriteString(cfg.hwAddr);
    data.WriteString(cfg.ipv4Addr);
    data.WriteInt32(cfg.prefixLength);
    int32_t vsize = cfg.flags.size();
    if (!data.WriteInt32(vsize)) {
        return ERR_FLATTEN_OBJECT;
    }
    std::vector<std::string> vCflags;
    vCflags.assign(cfg.flags.begin(), cfg.flags.end());
    NETNATIVE_LOGI("PROXY: InterfaceSetConfig Write flags String_SIZE: %{public}d",
        static_cast<int32_t>(vCflags.size()));
    for (std::vector<std::string>::iterator it = vCflags.begin(); it != vCflags.end(); ++it) {
        NETNATIVE_LOGI("PROXY: InterfaceSetConfig Write: %{public}s", (*it).c_str());
        data.WriteString(*it) ;
    }
    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_INTERFACE_SET_CONFIG, data, reply, option);
    return reply.ReadInt32();
}

int32_t NetdNativeServiceProxy::InterfaceGetConfig(interface_configuration_parcel &cfg)
{
    NETNATIVE_LOGI("Begin to InterfaceGetConfig");
    MessageParcel data;
    int32_t ret;
    int32_t vSize;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(cfg.ifName);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_INTERFACE_GET_CONFIG, data, reply, option);
    ret = reply.ReadInt32();
    reply.ReadString(cfg.ifName);
    reply.ReadString(cfg.hwAddr);
    reply.ReadString(cfg.ipv4Addr);
    reply.ReadInt32(cfg.prefixLength);
    vSize =  reply.ReadInt32();
    std::vector<std::string> vecString;
    for (int i = 0; i < vSize; i++) {
        vecString.push_back(reply.ReadString());
    }
    if (vSize > 0) {
        cfg.flags.assign(vecString.begin(), vecString.end());
    }
    NETNATIVE_LOGI("End to InterfaceGetConfig, ret =%{public}d", ret);
    return  ret ;
}

int32_t NetdNativeServiceProxy::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOGI("Begin to StartDhcpClient");
    MessageParcel data;
    int32_t   ret  ;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(iface);
    data.WriteBool(bIpv6);
    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(INetdService::NETD_START_DHCP_CLIENT, data, reply, option);
    ret = reply.ReadInt32();
    NETNATIVE_LOGI("End to StartDhcpClient, ret =%{public}d", ret);
    return ret ;
}

int32_t NetdNativeServiceProxy::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOGI("Begin to StopDhcpClient");
    MessageParcel data;
    int32_t   ret  ;
    data.WriteInterfaceToken(GetDescriptor());
    data.WriteString(iface);
    data.WriteBool(bIpv6);

    MessageParcel reply;
    MessageOption option;
    ret = Remote()->SendRequest(INetdService::NETD_STOP_DHCP_CLIENT, data, reply, option);
    ret = reply.ReadInt32();
    NETNATIVE_LOGI("End to StopDhcpClient, ret =%{public}d", ret);
    return ret ;
}
} // namespace NetdNative
} // namespace OHOS