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

#include "netd_native_service_stub.h"

#include <memory>

#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetdNative {
using namespace std;

NetdNativeServiceStub::NetdNativeServiceStub()
{
    opToInterfaceMap_[NETD_SET_RESOLVER_CONFIG_PARCEL] = &NetdNativeServiceStub::CmdSetResolverConfigParcel;
    opToInterfaceMap_[NETD_SET_RESOLVER_CONFIG] = &NetdNativeServiceStub::CmdSetResolverConfig;
    opToInterfaceMap_[NETD_GET_RESOLVER_CONFIG] = &NetdNativeServiceStub::CmdGetResolverConfig;
    opToInterfaceMap_[NETD_CREATE_NETWORK_CACHE] = &NetdNativeServiceStub::CmdCreateNetworkCache;
    opToInterfaceMap_[NETD_FLUSH_NETWORK_CACHE] = &NetdNativeServiceStub::CmdFlushNetworkCache;
    opToInterfaceMap_[NETD_DESTORY_NETWORK_CACHE] = &NetdNativeServiceStub::CmdDestoryNetworkCache;
    opToInterfaceMap_[NETD_GET_ADDR_INFO] = &NetdNativeServiceStub::CmdGetaddrinfo;
    opToInterfaceMap_[NETD_INTERFACE_SET_MTU] = &NetdNativeServiceStub::CmdInterfaceSetMtu;
    opToInterfaceMap_[NETD_INTERFACE_GET_MTU] = &NetdNativeServiceStub::CmdInterfaceGetMtu;

    opToInterfaceMap_[NETD_REGISTER_NOTIFY_CALLBACK] = &NetdNativeServiceStub::CmdRegisterNotifyCallback;

    opToInterfaceMap_[NETD_NETWORK_ADD_ROUTE] = &NetdNativeServiceStub::CmdNetworkAddRoute;
    opToInterfaceMap_[NETD_NETWORK_REMOVE_ROUTE] = &NetdNativeServiceStub::CmdNetworkRemoveRoute;
    opToInterfaceMap_[NETD_NETWORK_ADD_ROUTE_PARCEL] = &NetdNativeServiceStub::CmdNetworkAddRouteParcel;
    opToInterfaceMap_[NETD_NETWORK_REMOVE_ROUTE_PARCEL] = &NetdNativeServiceStub::CmdNetworkRemoveRouteParcel;
    opToInterfaceMap_[NETD_NETWORK_SET_DEFAULT] = &NetdNativeServiceStub::CmdNetworkSetDefault;
    opToInterfaceMap_[NETD_NETWORK_GET_DEFAULT] = &NetdNativeServiceStub::CmdNetworkGetDefault;
    opToInterfaceMap_[NETD_NETWORK_CREATE_PHYSICAL] = &NetdNativeServiceStub::CmdNetworkCreatePhysical;
    opToInterfaceMap_[NETD_INTERFACE_ADD_ADDRESS] = &NetdNativeServiceStub::CmdInterfaceAddAddress;
    opToInterfaceMap_[NETD_INTERFACE_DEL_ADDRESS] = &NetdNativeServiceStub::CmdInterfaceDelAddress;
    opToInterfaceMap_[NETD_NETWORK_ADD_INTERFACE] = &NetdNativeServiceStub::CmdNetworkAddInterface;
    opToInterfaceMap_[NETD_NETWORK_REMOVE_INTERFACE] = &NetdNativeServiceStub::CmdNetworkRemoveInterface;
    opToInterfaceMap_[NETD_NETWORK_DESTROY] = &NetdNativeServiceStub::CmdNetworkDestroy;
    opToInterfaceMap_[NETD_GET_FWMARK_FOR_NETWORK] = &NetdNativeServiceStub::CmdGetFwmarkForNetwork;
    opToInterfaceMap_[NETD_INTERFACE_SET_CONFIG] = &NetdNativeServiceStub::CmdInterfaceSetConfig;
    opToInterfaceMap_[NETD_INTERFACE_GET_CONFIG] = &NetdNativeServiceStub::CmdInterfaceGetConfig;
    opToInterfaceMap_[NETD_START_DHCP_CLIENT] = &NetdNativeServiceStub::CmdStartDhcpClient;
    opToInterfaceMap_[NETD_STOP_DHCP_CLIENT] = &NetdNativeServiceStub::CmdStopDhcpClient;
}

int32_t NetdNativeServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETNATIVE_LOGI("Begin to call procedure with code %{public}u", code);
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        NETNATIVE_LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    const std::u16string descriptor = NetdNativeServiceStub::GetDescriptor();
    const std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        NETNATIVE_LOGE("Check remote descriptor failed");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t NetdNativeServiceStub::CmdSetResolverConfigParcel(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd SetResolverConfig");
    auto resolvParamsParcel = DnsresolverParamsParcel::Unmarshalling(data);
    NETNATIVE_LOGI("Begin to CmdSetResolverConfig %{public}d",resolvParamsParcel->retryCount_);
    int32_t result = SetResolverConfigParcel(*resolvParamsParcel);
    delete resolvParamsParcel;
    reply.WriteInt32(result);
    NETNATIVE_LOGI("SetResolverConfig has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdSetResolverConfig(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd SetResolverConfig");
    dnsresolver_params resolvParams;

    uint16_t netId = 0;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;

    std::vector<std::string> servers;
    std::vector<std::string> domains;

    if (!data.ReadUint16(netId)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.ReadUint16(baseTimeoutMsec)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.ReadUint8(retryCount)) {
        return ERR_FLATTEN_OBJECT;
    }
    int32_t vServerSize;
    if (!data.ReadInt32(vServerSize)) {
        return ERR_FLATTEN_OBJECT;
    }

    std::string s;
    for (int32_t i = 0; i < vServerSize ; ++i) {
        std::string().swap(s);
        if (!data.ReadString(s)) {
            return ERR_FLATTEN_OBJECT;
        }
        servers.push_back(s);
    }

    int32_t vDomainSize;
    if (!data.ReadInt32(vDomainSize)) {
        return ERR_FLATTEN_OBJECT;
    }

    for (int32_t i = 0; i < vDomainSize; ++i) {
        std::string().swap(s);
        if (!data.ReadString(s)) {
            return ERR_FLATTEN_OBJECT;
        }
        domains.push_back(s);
    }

    resolvParams.netId = netId ;
    resolvParams.baseTimeoutMsec = baseTimeoutMsec;
    resolvParams.retryCount = retryCount;
    resolvParams.servers.assign(servers.begin(),servers.end());
    resolvParams.domains.assign(domains.begin(),domains.end());
    int32_t result = SetResolverConfig(resolvParams);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("SetResolverConfig has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdGetResolverConfig(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd GetResolverConfig");
    dns_res_params resParams;
    uint16_t netId = 0;
    std::vector<std::string> servers;
    std::vector<std::string> domains;

    data.ReadUint16(netId);
    int32_t result = GetResolverConfig(netId, servers,domains, resParams);
    reply.WriteInt32(result);
    reply.WriteUint16(resParams.baseTimeoutMsec );
    reply.WriteUint8(resParams.retryCount); 
    int32_t vServerSize=servers.size();
    reply.WriteInt32(vServerSize) ;
    std::vector<std::string>::iterator  iterServers;
    for (iterServers=servers.begin();iterServers!=servers.end();iterServers++) {
           reply.WriteString(*iterServers) ;
    }
    int32_t vDomainsSize=domains.size();
    reply.WriteInt32(vDomainsSize) ;
    std::vector<std::string>::iterator  iterDomains;
    for(iterDomains=domains.begin();iterDomains != domains.end();iterDomains++) {
           reply.WriteString(*iterDomains) ;
    }
    NETNATIVE_LOGI("GetResolverConfig has recved result %{public}d", result);
    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdCreateNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd CreateNetworkCache");
    uint16_t netid = data.ReadUint16();
    NETNATIVE_LOGI("CreateNetworkCache  netid %{public}d", netid);
    int32_t result = CreateNetworkCache(netid);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("CreateNetworkCache has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdFlushNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd FlushNetworkCache");
    uint16_t netid = data.ReadUint16();
    int32_t result = FlushNetworkCache(netid);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("FlushNetworkCache has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdDestoryNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd DestoryNetworkCache");
    uint16_t netid = data.ReadUint16();
    int32_t result = DestoryNetworkCache(netid);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("DestoryNetworkCache has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdGetaddrinfo(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd Getaddrinfo");
    struct addrinfo __hints;
    struct addrinfo** __result = nullptr;
    uint16_t __netid ;
    int16_t  ai_family;
    int16_t  ai_socktype;
    int16_t  ai_flags;
    int16_t  ai_protocol;

    char *__node = (char *)(data.ReadCString());
    if (__node == nullptr) {
        NETNATIVE_LOGI("CmdGetaddrinfo return, __node is nullptr.");
        return -1;
    }

    char *__service = (char *)(data.ReadCString());
    if (__service == nullptr) {
        NETNATIVE_LOGI("CmdGetaddrinfo return, __service is nullptr.");
        return -1;
    }

    bzero(&__hints, sizeof(addrinfo));
    ai_family = data.ReadInt16();
    __hints.ai_family = ai_family;
    ai_socktype = data.ReadInt16();
    __hints.ai_socktype = ai_socktype;
    ai_flags = data.ReadInt16();
    __hints.ai_flags = ai_flags;
    ai_protocol = data.ReadInt16();
    __hints.ai_protocol = ai_protocol;
    __netid = data.ReadUint16();
    int32_t result = Getaddrinfo(__node, __service, &__hints, __result, __netid);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("Getaddrinfo has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdInterfaceSetMtu(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd InterfaceSetMtu");
    string ifName = data.ReadString();
    int32_t mtu = data.ReadInt32();
    int32_t result = InterfaceSetMtu(ifName,mtu);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("InterfaceSetMtu has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdInterfaceGetMtu(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd InterfaceGetMtu");
    string ifName = data.ReadString();
    int32_t result = InterfaceGetMtu(ifName);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("InterfaceGetMtu has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdRegisterNotifyCallback(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd RegisterNotifyCallback");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("Callback ptr is nullptr.");
        return 0;
    }

    sptr<INotifyCallback> callback = iface_cast<INotifyCallback>(remote);
    int32_t result = RegisterNotifyCallback(callback);
    reply.WriteInt32(result);
    return ERR_NONE;
}

int32_t NetdNativeServiceStub::CmdNetworkAddRoute(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkAddRoute");
    int32_t netId = data.ReadInt32();
    std::string ifName = data.ReadString();
    std::string destination = data.ReadString();
    std::string nextHop = data.ReadString();

    NETNATIVE_LOGI("NetworkAddRoute unpacket %{public}d %{public}s %{public}s %{public}s",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());

    int32_t result = NetworkAddRoute(netId, ifName, destination, nextHop);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkAddRoute has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkRemoveRoute(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkAddRoute");
    int32_t netId = data.ReadInt32();
    std::string interfaceName = data.ReadString();
    std::string destination = data.ReadString();
    std::string nextHop = data.ReadString();
    int32_t result = NetworkRemoveRoute(netId, interfaceName, destination, nextHop);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkRemoveRoute has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkAddRouteParcel(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkAddRouteParcel");
    route_info_parcel routeInfo = {};
    int32_t netId = data.ReadInt32();
    routeInfo.ifName = data.ReadString();
    routeInfo.destination = data.ReadString();
    routeInfo.nextHop = data.ReadString();
    int32_t result = NetworkAddRouteParcel(netId, routeInfo);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkAddRouteParcel has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkRemoveRouteParcel(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkRemoveRouteParcel");
    route_info_parcel routeInfo = {};
    int32_t netId = data.ReadInt32();
    routeInfo.ifName = data.ReadString();
    routeInfo.destination = data.ReadString();
    routeInfo.nextHop = data.ReadString();

    int32_t result = NetworkRemoveRouteParcel(netId, routeInfo);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkRemoveRouteParcel has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkSetDefault(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkSetDefault");
    int32_t netId = data.ReadInt32();

    int32_t result = NetworkSetDefault(netId);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkSetDefault has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkGetDefault(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkGetDefault");
    int32_t result = NetworkGetDefault();
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkGetDefault has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkCreatePhysical(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkCreatePhysical");
    int32_t netId = data.ReadInt32();
    int32_t permission = data.ReadInt32();

    int32_t result = NetworkCreatePhysical(netId, permission);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkCreatePhysical has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdInterfaceAddAddress(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd InterfaceAddAddress");
    std::string interfaceName = data.ReadString();
    std::string ipAddr = data.ReadString();
    int32_t prefixLength = data.ReadInt32();

    int32_t result = InterfaceAddAddress(interfaceName, ipAddr, prefixLength);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("InterfaceAddAddress has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdInterfaceDelAddress(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkRemoveRouteParcel");
    std::string interfaceName = data.ReadString();
    std::string ipAddr = data.ReadString();
    int32_t prefixLength = data.ReadInt32();

    int32_t result = InterfaceDelAddress(interfaceName, ipAddr, prefixLength);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("InterfaceDelAddress has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkAddInterface(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkAddInterface");
    int32_t netId = data.ReadInt32();
    std::string iface = data.ReadString();

    int32_t result = NetworkAddInterface(netId, iface);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkAddInterface has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkRemoveInterface(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkRemoveRouteParcel");
    int32_t netId = data.ReadInt32();
    std::string iface = data.ReadString();

    int32_t result = NetworkRemoveInterface(netId, iface);

    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkRemoveRouteParcel has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdNetworkDestroy(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd NetworkDestroy");
    int32_t netId = data.ReadInt32();

    int32_t result = NetworkDestroy(netId);

    reply.WriteInt32(result);
    NETNATIVE_LOGI("NetworkDestroy has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdGetFwmarkForNetwork(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd GetFwmarkForNetwork");
    mark_mask_parcel markMaskParcel = {};
    int32_t netId = data.ReadInt32();
    markMaskParcel.mark = data.ReadInt32();
    markMaskParcel.mask = data.ReadInt32();

    int32_t result = GetFwmarkForNetwork(netId, markMaskParcel);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("GetFwmarkForNetwork has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdInterfaceSetConfig(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd InterfaceSetConfig");
    interface_configuration_parcel cfg = {};
    cfg.ifName = data.ReadString();
    cfg.hwAddr = data.ReadString();
    cfg.ipv4Addr = data.ReadString();
    cfg.prefixLength = data.ReadInt32();
    // data.ReadStringVector(&cfg.flags);
    int32_t vsize =  data.ReadInt32(); 
    std::string   vString ;
    std::vector<std::string>  vCflags;
    for (int i=0;i<vsize;i++) {
           vString=data.ReadString();
           vCflags.push_back(vString);
    }
    cfg.flags.assign(vCflags.begin(),vCflags.end());
    int32_t result = InterfaceSetConfig(cfg);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("InterfaceSetConfig has recved result %{public}d", result);

    return result;
}

int32_t NetdNativeServiceStub::CmdInterfaceGetConfig(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd InterfaceGetConfig");
    interface_configuration_parcel cfg = {};
    cfg.ifName = data.ReadString();
    int32_t result = InterfaceGetConfig(cfg);
    reply.WriteInt32(result);
    reply.WriteString(cfg.ifName) ;
    reply.WriteString(cfg.hwAddr) ;
    reply.WriteString(cfg.ipv4Addr) ;
    reply.WriteInt32(cfg.prefixLength) ;
    int32_t vsize =  cfg.flags.size();
    reply.WriteInt32(vsize);
    std::vector<std::string>::iterator    iter ;
    for (iter=cfg.flags.begin();iter != cfg.flags.end();iter++) {
           reply.WriteString(*iter) ;
    }
    return result;
}

int32_t NetdNativeServiceStub::CmdStartDhcpClient(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd CmdStartDhcpClient");
    std::string iface = data.ReadString();
    bool bIpv6 = data.ReadBool();

    int32_t result = StartDhcpClient(iface, bIpv6);
    reply.WriteInt32(result);
    return result;
}

int32_t NetdNativeServiceStub::CmdStopDhcpClient(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("Begin to dispatch cmd CmdStopDhcpClient");
    std::string iface = data.ReadString();
    bool bIpv6 = data.ReadBool();

    int32_t result = StopDhcpClient(iface, bIpv6);
    reply.WriteInt32(result);
    return result;
}
} // namespace NetdNative
} // namespace OHOS
