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

#include <cstdlib>
#include <net/route.h>
#include <netdb.h>
#include <unistd.h>

#include "ipc_skeleton.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_permission.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_stub.h"
#include "securec.h"
#include "i_net_dns_result_callback.h"
#include "i_net_dns_health_callback.h"

using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace NetsysNative {
namespace {
constexpr int32_t MAX_FLAG_NUM = 64;
constexpr int32_t MAX_DNS_CONFIG_SIZE = 4;
constexpr int32_t NETMANAGER_ERR_PERMISSION_DENIED = 201;
constexpr uint32_t UIDS_LIST_MAX_SIZE = 1024;
constexpr uint32_t MAX_UID_ARRAY_SIZE = 1024;
constexpr uint32_t MAX_CONFIG_LIST_SIZE = 1024;
constexpr uint32_t MAX_ROUTE_TABLE_SIZE = 128;
} // namespace

NetsysNativeServiceStub::NetsysNativeServiceStub()
{
    InitNetInfoOpToInterfaceMap();
    InitBandwidthOpToInterfaceMap();
    InitFirewallOpToInterfaceMap();
    InitOpToInterfaceMapExt();
    InitNetDiagOpToInterfaceMap();
    InitNetDnsDiagOpToInterfaceMap();
    InitStaticArpToInterfaceMap();
    uids_ = {UID_ROOT, UID_SHELL, UID_NET_MANAGER, UID_WIFI, UID_RADIO, UID_HIDUMPER_SERVICE,
        UID_SAMGR, UID_PARAM_WATCHER, UID_EDM, UID_SECURITY_COLLECTOR};
}

void NetsysNativeServiceStub::InitNetInfoOpToInterfaceMap()
{
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_SET_RESOLVER_CONFIG)] =
        &NetsysNativeServiceStub::CmdSetResolverConfig;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_RESOLVER_CONFIG)] =
        &NetsysNativeServiceStub::CmdGetResolverConfig;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_CREATE_NETWORK_CACHE)] =
        &NetsysNativeServiceStub::CmdCreateNetworkCache;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_DESTROY_NETWORK_CACHE)] =
        &NetsysNativeServiceStub::CmdDestroyNetworkCache;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_ADDR_INFO)] =
        &NetsysNativeServiceStub::CmdGetAddrInfo;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_SET_MTU)] =
        &NetsysNativeServiceStub::CmdSetInterfaceMtu;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_GET_MTU)] =
        &NetsysNativeServiceStub::CmdGetInterfaceMtu;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_SET_TCP_BUFFER_SIZES)] =
        &NetsysNativeServiceStub::CmdSetTcpBufferSizes;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_REGISTER_NOTIFY_CALLBACK)] =
        &NetsysNativeServiceStub::CmdRegisterNotifyCallback;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_UNREGISTER_NOTIFY_CALLBACK)] =
        &NetsysNativeServiceStub::CmdUnRegisterNotifyCallback;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_ADD_ROUTE)] =
        &NetsysNativeServiceStub::CmdNetworkAddRoute;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_REMOVE_ROUTE)] =
        &NetsysNativeServiceStub::CmdNetworkRemoveRoute;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_ADD_ROUTE_PARCEL)] =
        &NetsysNativeServiceStub::CmdNetworkAddRouteParcel;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_REMOVE_ROUTE_PARCEL)] =
        &NetsysNativeServiceStub::CmdNetworkRemoveRouteParcel;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_SET_DEFAULT)] =
        &NetsysNativeServiceStub::CmdNetworkSetDefault;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_GET_DEFAULT)] =
        &NetsysNativeServiceStub::CmdNetworkGetDefault;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_CLEAR_DEFAULT)] =
        &NetsysNativeServiceStub::CmdNetworkClearDefault;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_PROC_SYS_NET)] =
        &NetsysNativeServiceStub::CmdGetProcSysNet;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_SET_PROC_SYS_NET)] =
        &NetsysNativeServiceStub::CmdSetProcSysNet;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_CREATE_PHYSICAL)] =
        &NetsysNativeServiceStub::CmdNetworkCreatePhysical;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_ADD_ADDRESS)] =
        &NetsysNativeServiceStub::CmdAddInterfaceAddress;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_DEL_ADDRESS)] =
        &NetsysNativeServiceStub::CmdDelInterfaceAddress;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_SET_IPV6_PRIVCAY_EXTENSION)] =
        &NetsysNativeServiceStub::CmdSetIpv6PrivacyExtensions;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_ENABLE_IPV6)] =
        &NetsysNativeServiceStub::CmdSetIpv6Enable;
}

void NetsysNativeServiceStub::InitBandwidthOpToInterfaceMap()
{
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_SHARING_NETWORK_TRAFFIC)] =
        &NetsysNativeServiceStub::CmdGetNetworkSharingTraffic;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_TOTAL_STATS)] =
        &NetsysNativeServiceStub::CmdGetTotalStats;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_UID_STATS)] =
        &NetsysNativeServiceStub::CmdGetUidStats;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_IFACE_STATS)] =
        &NetsysNativeServiceStub::CmdGetIfaceStats;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_ALL_CONTAINER_STATS_INFO)] =
        &NetsysNativeServiceStub::CmdGetAllContainerStatsInfo;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_ALL_STATS_INFO)] =
        &NetsysNativeServiceStub::CmdGetAllStatsInfo;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_COOKIE_STATS)] =
        &NetsysNativeServiceStub::CmdGetCookieStats;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_CREATE_VIRTUAL)] =
        &NetsysNativeServiceStub::CmdNetworkCreateVirtual;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_ADD_UIDS)] =
        &NetsysNativeServiceStub::CmdNetworkAddUids;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_DEL_UIDS)] =
        &NetsysNativeServiceStub::CmdNetworkDelUids;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_BANDWIDTH_ENABLE_DATA_SAVER)] =
        &NetsysNativeServiceStub::CmdBandwidthEnableDataSaver;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_BANDWIDTH_SET_IFACE_QUOTA)] =
        &NetsysNativeServiceStub::CmdBandwidthSetIfaceQuota;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_BANDWIDTH_REMOVE_IFACE_QUOTA)] =
        &NetsysNativeServiceStub::CmdBandwidthRemoveIfaceQuota;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_BANDWIDTH_ADD_DENIED_LIST)] =
        &NetsysNativeServiceStub::CmdBandwidthAddDeniedList;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_BANDWIDTH_REMOVE_DENIED_LIST)] =
        &NetsysNativeServiceStub::CmdBandwidthRemoveDeniedList;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_BANDWIDTH_ADD_ALLOWED_LIST)] =
        &NetsysNativeServiceStub::CmdBandwidthAddAllowedList;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_BANDWIDTH_REMOVE_ALLOWED_LIST)] =
        &NetsysNativeServiceStub::CmdBandwidthRemoveAllowedList;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_SET_INTERNET_PERMISSION)] =
        &NetsysNativeServiceStub::CmdSetInternetPermission;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_SET_NETWORK_ACCESS_POLICY)] =
        &NetsysNativeServiceStub::CmdSetNetworkAccessPolicy;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_DEL_NETWORK_ACCESS_POLICY)] =
        &NetsysNativeServiceStub::CmdDelNetworkAccessPolicy;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NOTIFY_NETWORK_BEARER_TYPE_CHANGE)] =
        &NetsysNativeServiceStub::CmdNotifyNetBearerTypeChange;
}

void NetsysNativeServiceStub::InitFirewallOpToInterfaceMap()
{
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_FIREWALL_SET_UID_ALLOWED_LIST_CHAIN)] =
        &NetsysNativeServiceStub::CmdFirewallSetUidsAllowedListChain;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_FIREWALL_SET_UID_DENIED_LIST_CHAIN)] =
        &NetsysNativeServiceStub::CmdFirewallSetUidsDeniedListChain;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_FIREWALL_ENABLE_CHAIN)] =
        &NetsysNativeServiceStub::CmdFirewallEnableChain;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_FIREWALL_SET_UID_RULE)] =
        &NetsysNativeServiceStub::CmdFirewallSetUidRule;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_NETWORK_SHARING_TYPE)] =
        &NetsysNativeServiceStub::CmdGetNetworkSharingType;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_UPDATE_NETWORK_SHARING_TYPE)] =
        &NetsysNativeServiceStub::CmdUpdateNetworkSharingType;
}

void NetsysNativeServiceStub::InitOpToInterfaceMapExt()
{
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_SET_IP_ADDRESS)] =
        &NetsysNativeServiceStub::CmdInterfaceSetIpAddress;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_SET_IFF_UP)] =
        &NetsysNativeServiceStub::CmdInterfaceSetIffUp;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_ADD_INTERFACE)] =
        &NetsysNativeServiceStub::CmdNetworkAddInterface;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_REMOVE_INTERFACE)] =
        &NetsysNativeServiceStub::CmdNetworkRemoveInterface;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETWORK_DESTROY)] =
        &NetsysNativeServiceStub::CmdNetworkDestroy;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_GET_FWMARK_FOR_NETWORK)] =
        &NetsysNativeServiceStub::CmdGetFwmarkForNetwork;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_SET_CONFIG)] =
        &NetsysNativeServiceStub::CmdSetInterfaceConfig;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_GET_CONFIG)] =
        &NetsysNativeServiceStub::CmdGetInterfaceConfig;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_INTERFACE_GET_LIST)] =
        &NetsysNativeServiceStub::CmdInterfaceGetList;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_START_DHCP_CLIENT)] =
        &NetsysNativeServiceStub::CmdStartDhcpClient;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_STOP_DHCP_CLIENT)] =
        &NetsysNativeServiceStub::CmdStopDhcpClient;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_START_DHCP_SERVICE)] =
        &NetsysNativeServiceStub::CmdStartDhcpService;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_STOP_DHCP_SERVICE)] =
        &NetsysNativeServiceStub::CmdStopDhcpService;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_IPENABLE_FORWARDING)] =
        &NetsysNativeServiceStub::CmdIpEnableForwarding;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_IPDISABLE_FORWARDING)] =
        &NetsysNativeServiceStub::CmdIpDisableForwarding;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_ENABLE_NAT)] =
        &NetsysNativeServiceStub::CmdEnableNat;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_DISABLE_NAT)] =
        &NetsysNativeServiceStub::CmdDisableNat;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_IPFWD_ADD_INTERFACE_FORWARD)] =
        &NetsysNativeServiceStub::CmdIpfwdAddInterfaceForward;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_IPFWD_REMOVE_INTERFACE_FORWARD)] =
        &NetsysNativeServiceStub::CmdIpfwdRemoveInterfaceForward;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_SET_IPTABLES_CMD_FOR_RES)] =
        &NetsysNativeServiceStub::CmdSetIptablesCommandForRes;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_TETHER_DNS_SET)] =
        &NetsysNativeServiceStub::CmdShareDnsSet;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_START_DNS_PROXY_LISTEN)] =
        &NetsysNativeServiceStub::CmdStartDnsProxyListen;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_STOP_DNS_PROXY_LISTEN)] =
        &NetsysNativeServiceStub::CmdStopDnsProxyListen;
}

void NetsysNativeServiceStub::InitNetDiagOpToInterfaceMap()
{
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETDIAG_PING_HOST)] =
        &NetsysNativeServiceStub::CmdNetDiagPingHost;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETDIAG_GET_ROUTE_TABLE)] =
        &NetsysNativeServiceStub::CmdNetDiagGetRouteTable;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETDIAG_GET_SOCKETS_INFO)] =
        &NetsysNativeServiceStub::CmdNetDiagGetSocketsInfo;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETDIAG_GET_IFACE_CONFIG)] =
        &NetsysNativeServiceStub::CmdNetDiagGetInterfaceConfig;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETDIAG_UPDATE_IFACE_CONFIG)] =
        &NetsysNativeServiceStub::CmdNetDiagUpdateInterfaceConfig;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NETDIAG_SET_IFACE_ACTIVE_STATE)] =
        &NetsysNativeServiceStub::CmdNetDiagSetInterfaceActiveState;
}

void NetsysNativeServiceStub::InitStaticArpToInterfaceMap()
{
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_ADD_STATIC_ARP)] =
        &NetsysNativeServiceStub::CmdAddStaticArp;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_DEL_STATIC_ARP)] =
        &NetsysNativeServiceStub::CmdDelStaticArp;
}

void NetsysNativeServiceStub::InitNetDnsDiagOpToInterfaceMap()
{
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_REGISTER_DNS_RESULT_LISTENER)] =
        &NetsysNativeServiceStub::CmdRegisterDnsResultListener;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_UNREGISTER_DNS_RESULT_LISTENER)] =
        &NetsysNativeServiceStub::CmdUnregisterDnsResultListener;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_REGISTER_DNS_HEALTH_LISTENER)] =
        &NetsysNativeServiceStub::CmdRegisterDnsHealthListener;
    opToInterfaceMap_[static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_UNREGISTER_DNS_HEALTH_LISTENER)] =
        &NetsysNativeServiceStub::CmdUnregisterDnsHealthListener;
}

int32_t NetsysNativeServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                                 MessageOption &option)
{
    NETNATIVE_LOG_D("Begin to call procedure with code %{public}u", code);
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        NETNATIVE_LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    auto uid = IPCSkeleton::GetCallingUid();
    if (std::find(uids_.begin(), uids_.end(), uid) == uids_.end()) {
        NETNATIVE_LOGE("This uid connot use netsys");
        if (!reply.WriteInt32(NETMANAGER_ERR_PERMISSION_DENIED)) {
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    if (code == static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_SET_IPTABLES_CMD_FOR_RES) && uid != UID_EDM &&
        uid != UID_NET_MANAGER) {
        if (!reply.WriteInt32(NETMANAGER_ERR_PERMISSION_DENIED)) {
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    const std::u16string descriptor = NetsysNativeServiceStub::GetDescriptor();
    const std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        NETNATIVE_LOGE("Check remote descriptor failed");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t NetsysNativeServiceStub::CmdSetResolverConfig(MessageParcel &data, MessageParcel &reply)
{
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
    vServerSize = (vServerSize > MAX_DNS_CONFIG_SIZE) ? MAX_DNS_CONFIG_SIZE : vServerSize;
    std::string s;
    for (int32_t i = 0; i < vServerSize; ++i) {
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
    vDomainSize = (vDomainSize > MAX_DNS_CONFIG_SIZE) ? MAX_DNS_CONFIG_SIZE : vDomainSize;
    for (int32_t i = 0; i < vDomainSize; ++i) {
        std::string().swap(s);
        if (!data.ReadString(s)) {
            return ERR_FLATTEN_OBJECT;
        }
        domains.push_back(s);
    }

    int32_t result = SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("SetResolverConfig has received result %{public}d", result);

    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdGetResolverConfig(MessageParcel &data, MessageParcel &reply)
{
    uint16_t baseTimeoutMsec;
    uint8_t retryCount;
    uint16_t netId = 0;
    std::vector<std::string> servers;
    std::vector<std::string> domains;

    data.ReadUint16(netId);
    int32_t result = GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
    reply.WriteInt32(result);
    reply.WriteUint16(baseTimeoutMsec);
    reply.WriteUint8(retryCount);
    auto vServerSize = static_cast<int32_t>(servers.size());
    vServerSize = (vServerSize > MAX_DNS_CONFIG_SIZE) ? MAX_DNS_CONFIG_SIZE : vServerSize;
    reply.WriteInt32(vServerSize);
    int32_t index = 0;
    for (auto &server : servers) {
        if (++index > MAX_DNS_CONFIG_SIZE) {
            break;
        }
        reply.WriteString(server);
    }
    auto vDomainsSize = static_cast<int32_t>(domains.size());
    vDomainsSize = (vDomainsSize > MAX_DNS_CONFIG_SIZE) ? MAX_DNS_CONFIG_SIZE : vDomainsSize;
    reply.WriteInt32(vDomainsSize);
    std::vector<std::string>::iterator iterDomains;
    index = 0;
    for (iterDomains = domains.begin(); iterDomains != domains.end(); ++iterDomains) {
        if (++index > MAX_DNS_CONFIG_SIZE) {
            break;
        }
        reply.WriteString(*iterDomains);
    }
    NETNATIVE_LOG_D("GetResolverConfig has recved result %{public}d", result);
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdCreateNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netid = data.ReadUint16();
    NETNATIVE_LOGI("CreateNetworkCache  netid %{public}d", netid);
    int32_t result = CreateNetworkCache(netid);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("CreateNetworkCache has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdDestroyNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netId = data.ReadUint16();
    int32_t result = DestroyNetworkCache(netId);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("DestroyNetworkCache has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::NetsysFreeAddrinfo(struct addrinfo *aihead)
{
    struct addrinfo *ai;
    struct addrinfo *ainext;
    for (ai = aihead; ai != nullptr; ai = ainext) {
        if (ai->ai_addr != nullptr)
            free(ai->ai_addr);
        if (ai->ai_canonname != nullptr)
            free(ai->ai_canonname);
        ainext = ai->ai_next;
        free(ai);
    }
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdGetAddrInfo(MessageParcel &data, MessageParcel &reply)
{
    std::string hostName;
    std::string serverName;
    AddrInfo hints = {};
    uint16_t netId;
    if (!data.ReadString(hostName)) {
        return IPC_STUB_INVALID_DATA_ERR;
    }

    if (!data.ReadString(serverName)) {
        return IPC_STUB_INVALID_DATA_ERR;
    }

    auto p = data.ReadRawData(sizeof(AddrInfo));
    if (p == nullptr) {
        return IPC_STUB_INVALID_DATA_ERR;
    }
    if (memcpy_s(&hints, sizeof(AddrInfo), p, sizeof(AddrInfo)) != EOK) {
        return IPC_STUB_INVALID_DATA_ERR;
    }

    if (!data.ReadUint16(netId)) {
        return IPC_STUB_INVALID_DATA_ERR;
    }

    std::vector<AddrInfo> retInfo;
    auto ret = GetAddrInfo(hostName, serverName, hints, netId, retInfo);
    if (retInfo.size() > MAX_RESULTS) {
        return IPC_STUB_INVALID_DATA_ERR;
    }

    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_WRITE_PARCEL_ERR;
    }

    if (ret != ERR_NONE) {
        return ERR_NONE;
    }

    if (!reply.WriteUint32(static_cast<uint32_t>(retInfo.size()))) {
        return IPC_STUB_WRITE_PARCEL_ERR;
    }

    for (const auto &info : retInfo) {
        if (!reply.WriteRawData(&info, sizeof(AddrInfo))) {
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
    }
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdSetInterfaceMtu(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = data.ReadString();
    int32_t mtu = data.ReadInt32();
    int32_t result = SetInterfaceMtu(ifName, mtu);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("SetInterfaceMtu has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdGetInterfaceMtu(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = data.ReadString();
    int32_t result = GetInterfaceMtu(ifName);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("GetInterfaceMtu has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdSetTcpBufferSizes(MessageParcel &data, MessageParcel &reply)
{
    std::string tcpBufferSizes = data.ReadString();
    int32_t result = SetTcpBufferSizes(tcpBufferSizes);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("SetTcpBufferSizes has recved result %{public}d", result);

    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdRegisterNotifyCallback(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd RegisterNotifyCallback");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("Callback ptr is nullptr.");
        return -1;
    }

    sptr<INotifyCallback> callback = iface_cast<INotifyCallback>(remote);
    int32_t result = RegisterNotifyCallback(callback);
    reply.WriteInt32(result);
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdUnRegisterNotifyCallback(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd UnRegisterNotifyCallback");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("Callback ptr is nullptr.");
        return -1;
    }

    sptr<INotifyCallback> callback = iface_cast<INotifyCallback>(remote);
    int32_t result = UnRegisterNotifyCallback(callback);
    reply.WriteInt32(result);
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdNetworkAddRoute(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = data.ReadInt32();
    std::string ifName = data.ReadString();
    std::string destination = data.ReadString();
    std::string nextHop = data.ReadString();

    NETNATIVE_LOGI("netId[%{public}d}, ifName[%{public}s], destination[%{public}s}, nextHop[%{public}s]", netId,
                   ifName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    int32_t result = NetworkAddRoute(netId, ifName, destination, nextHop);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkAddRoute has recved result %{public}d", result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkRemoveRoute(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = data.ReadInt32();
    std::string interfaceName = data.ReadString();
    std::string destination = data.ReadString();
    std::string nextHop = data.ReadString();

    NETNATIVE_LOGI("netId[%{public}d}, ifName[%{public}s], destination[%{public}s}, nextHop[%{public}s]", netId,
                   interfaceName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    int32_t result = NetworkRemoveRoute(netId, interfaceName, destination, nextHop);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkRemoveRoute has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkAddRouteParcel(MessageParcel &data, MessageParcel &reply)
{
    RouteInfoParcel routeInfo = {};
    int32_t netId = data.ReadInt32();
    routeInfo.ifName = data.ReadString();
    routeInfo.destination = data.ReadString();
    routeInfo.nextHop = data.ReadString();
    int32_t result = NetworkAddRouteParcel(netId, routeInfo);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkAddRouteParcel has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkRemoveRouteParcel(MessageParcel &data, MessageParcel &reply)
{
    RouteInfoParcel routeInfo = {};
    int32_t netId = data.ReadInt32();
    routeInfo.ifName = data.ReadString();
    routeInfo.destination = data.ReadString();
    routeInfo.nextHop = data.ReadString();

    int32_t result = NetworkRemoveRouteParcel(netId, routeInfo);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkRemoveRouteParcel has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkSetDefault(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = data.ReadInt32();

    int32_t result = NetworkSetDefault(netId);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkSetDefault has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkGetDefault(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NetworkGetDefault();
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkGetDefault has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkClearDefault(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NetworkClearDefault();
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkClearDefault has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdGetProcSysNet(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd GetProcSysNet");
    int32_t family = data.ReadInt32();
    int32_t which = data.ReadInt32();
    std::string ifname = data.ReadString();
    std::string parameter = data.ReadString();
    std::string value;
    int32_t result = GetProcSysNet(family, which, ifname, parameter, value);
    reply.WriteInt32(result);
    std::string valueRsl = value;
    reply.WriteString(valueRsl);
    return result;
}

int32_t NetsysNativeServiceStub::CmdSetProcSysNet(MessageParcel &data, MessageParcel &reply)
{
    int32_t family = data.ReadInt32();
    int32_t which = data.ReadInt32();
    std::string ifname = data.ReadString();
    std::string parameter = data.ReadString();
    std::string value = data.ReadString();
    int32_t result = SetProcSysNet(family, which, ifname, parameter, value);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("SetProcSysNet has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdSetInternetPermission(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = data.ReadUint32();
    uint8_t allow = data.ReadUint8();
    uint8_t isBroker = data.ReadUint8();
    int32_t result = SetInternetPermission(uid, allow, isBroker);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("SetInternetPermission has recved result %{public}d", result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkCreatePhysical(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = data.ReadInt32();
    int32_t permission = data.ReadInt32();

    int32_t result = NetworkCreatePhysical(netId, permission);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkCreatePhysical has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkCreateVirtual(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    bool hasDns = false;
    if (!data.ReadInt32(netId) || !data.ReadBool(hasDns)) {
        NETNATIVE_LOGE("read net id or hasDns failed");
        return IPC_STUB_ERR;
    }

    int32_t result = NetworkCreateVirtual(netId, hasDns);
    if (!reply.WriteInt32(result)) {
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    NETNATIVE_LOG_D("NetworkCreateVirtual has recved result %{public}d", result);
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdNetworkAddUids(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    int32_t size = 0;
    if (!data.ReadInt32(netId) || !data.ReadInt32(size)) {
        NETNATIVE_LOGE("read net id or size failed");
        return IPC_STUB_ERR;
    }
    size = (size > static_cast<int32_t>(MAX_UID_ARRAY_SIZE)) ? static_cast<int32_t>(MAX_UID_ARRAY_SIZE) : size;

    sptr<UidRange> uid;
    std::vector<UidRange> uidRanges;
    for (int32_t index = 0; index < size; index++) {
        uid = UidRange::Unmarshalling(data);
        if (uid == nullptr) {
            NETNATIVE_LOGE("UidRange::Unmarshalling(parcel) is null");
            return IPC_STUB_ERR;
        }
        uidRanges.push_back(*uid);
    }
    int32_t result = NetworkAddUids(netId, uidRanges);
    if (!reply.WriteInt32(result)) {
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    NETNATIVE_LOG_D("NetworkAddUids has recved result %{public}d", result);
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdNetworkDelUids(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    int32_t size = 0;
    if (!data.ReadInt32(netId) || !data.ReadInt32(size)) {
        NETNATIVE_LOGE("read net id or size failed");
        return IPC_STUB_ERR;
    }

    size = (size > static_cast<int32_t>(MAX_UID_ARRAY_SIZE)) ? static_cast<int32_t>(MAX_UID_ARRAY_SIZE) : size;

    sptr<UidRange> uid;
    std::vector<UidRange> uidRanges;
    for (int32_t index = 0; index < size; index++) {
        uid = UidRange::Unmarshalling(data);
        if (uid == nullptr) {
            NETNATIVE_LOGE("UidRange::Unmarshalling(parcel) is null");
            return IPC_STUB_ERR;
        }
        uidRanges.push_back(*uid);
    }
    int32_t result = NetworkDelUids(netId, uidRanges);
    if (!reply.WriteInt32(result)) {
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    NETNATIVE_LOG_D("NetworkDelUids has recved result %{public}d", result);
    return ERR_NONE;
}

int32_t NetsysNativeServiceStub::CmdAddInterfaceAddress(MessageParcel &data, MessageParcel &reply)
{
    std::string interfaceName = data.ReadString();
    std::string ipAddr = data.ReadString();
    int32_t prefixLength = data.ReadInt32();

    int32_t result = AddInterfaceAddress(interfaceName, ipAddr, prefixLength);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("AddInterfaceAddress has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdDelInterfaceAddress(MessageParcel &data, MessageParcel &reply)
{
    std::string interfaceName = data.ReadString();
    std::string ipAddr = data.ReadString();
    int32_t prefixLength = data.ReadInt32();

    int32_t result = DelInterfaceAddress(interfaceName, ipAddr, prefixLength);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("DelInterfaceAddress has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdInterfaceSetIpAddress(MessageParcel &data, MessageParcel &reply)
{
    std::string ifaceName = data.ReadString();
    std::string ipAddress = data.ReadString();

    int32_t result = InterfaceSetIpAddress(ifaceName, ipAddress);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("InterfaceSetIpAddress has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdInterfaceSetIffUp(MessageParcel &data, MessageParcel &reply)
{
    std::string ifaceName = data.ReadString();

    int32_t result = InterfaceSetIffUp(ifaceName);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("InterfaceSetIffUp has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkAddInterface(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = data.ReadInt32();
    std::string iface = data.ReadString();

    int32_t result = NetworkAddInterface(netId, iface);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkAddInterface has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkRemoveInterface(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = data.ReadInt32();
    std::string iface = data.ReadString();
    int32_t result = NetworkRemoveInterface(netId, iface);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkRemoveInterface has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdNetworkDestroy(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = data.ReadInt32();
    int32_t result = NetworkDestroy(netId);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("NetworkDestroy has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdGetFwmarkForNetwork(MessageParcel &data, MessageParcel &reply)
{
    MarkMaskParcel markMaskParcel = {};
    int32_t netId = data.ReadInt32();
    markMaskParcel.mark = data.ReadInt32();
    markMaskParcel.mask = data.ReadInt32();
    int32_t result = GetFwmarkForNetwork(netId, markMaskParcel);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("GetFwmarkForNetwork has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdSetInterfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    InterfaceConfigurationParcel cfg = {};
    cfg.ifName = data.ReadString();
    cfg.hwAddr = data.ReadString();
    cfg.ipv4Addr = data.ReadString();
    cfg.prefixLength = data.ReadInt32();
    int32_t vSize = data.ReadInt32();
    vSize = (vSize > MAX_FLAG_NUM) ? MAX_FLAG_NUM : vSize;
    std::vector<std::string> vFlags;
    for (int i = 0; i < vSize; i++) {
        vFlags.emplace_back(data.ReadString());
    }
    cfg.flags.assign(vFlags.begin(), vFlags.end());
    int32_t result = SetInterfaceConfig(cfg);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("SetInterfaceConfig has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdGetInterfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd GetInterfaceConfig");
    InterfaceConfigurationParcel cfg = {};
    cfg.ifName = data.ReadString();
    int32_t result = GetInterfaceConfig(cfg);
    reply.WriteInt32(result);
    reply.WriteString(cfg.ifName);
    reply.WriteString(cfg.hwAddr);
    reply.WriteString(cfg.ipv4Addr);
    reply.WriteInt32(cfg.prefixLength);
    int32_t vsize = static_cast<int32_t>(cfg.flags.size());
    vsize = vsize > MAX_DNS_CONFIG_SIZE ? MAX_DNS_CONFIG_SIZE : vsize;
    reply.WriteInt32(vsize);
    std::vector<std::string>::iterator iter;
    int32_t index = 0;
    for (iter = cfg.flags.begin(); iter != cfg.flags.end(); ++iter) {
        if (++index > MAX_DNS_CONFIG_SIZE) {
            break;
        }
        reply.WriteString(*iter);
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdInterfaceGetList(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd InterfaceGetList");
    std::vector<std::string> ifaces;
    int32_t result = InterfaceGetList(ifaces);
    reply.WriteInt32(result);
    auto vsize = static_cast<int32_t>(ifaces.size());
    reply.WriteInt32(vsize);
    std::vector<std::string>::iterator iter;
    for (iter = ifaces.begin(); iter != ifaces.end(); ++iter) {
        reply.WriteString(*iter);
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdStartDhcpClient(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdStartDhcpClient");
    std::string iface = data.ReadString();
    bool bIpv6 = data.ReadBool();
    int32_t result = StartDhcpClient(iface, bIpv6);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdStopDhcpClient(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdStopDhcpClient");
    std::string iface = data.ReadString();
    bool bIpv6 = data.ReadBool();
    int32_t result = StopDhcpClient(iface, bIpv6);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdStartDhcpService(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdStartDhcpService");
    std::string iface = data.ReadString();
    std::string ipv4addr = data.ReadString();
    int32_t result = StartDhcpService(iface, ipv4addr);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdStopDhcpService(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdStopDhcpService");
    std::string iface = data.ReadString();
    int32_t result = StopDhcpService(iface);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdIpEnableForwarding(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdIpEnableForwarding");
    const auto &requester = data.ReadString();
    int32_t result = IpEnableForwarding(requester);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdIpDisableForwarding(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdIpDisableForwarding");
    const auto &requester = data.ReadString();
    int32_t result = IpDisableForwarding(requester);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdEnableNat(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdEnableNat");
    const auto &downstreamIface = data.ReadString();
    const auto &upstreamIface = data.ReadString();
    int32_t result = EnableNat(downstreamIface, upstreamIface);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdDisableNat(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdDisableNat");
    const auto &downstreamIface = data.ReadString();
    const auto &upstreamIface = data.ReadString();
    int32_t result = DisableNat(downstreamIface, upstreamIface);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdIpfwdAddInterfaceForward(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdIpfwdAddInterfaceForward");
    std::string fromIface = data.ReadString();
    std::string toIface = data.ReadString();
    int32_t result = IpfwdAddInterfaceForward(fromIface, toIface);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdIpfwdRemoveInterfaceForward(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdIpfwdRemoveInterfaceForward");
    const auto &fromIface = data.ReadString();
    const auto &toIface = data.ReadString();
    int32_t result = IpfwdRemoveInterfaceForward(fromIface, toIface);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdBandwidthEnableDataSaver(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdBandwidthEnableDataSaver");
    bool enable = data.ReadBool();
    int32_t result = BandwidthEnableDataSaver(enable);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdBandwidthSetIfaceQuota(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdBandwidthSetIfaceQuota");
    std::string ifName = data.ReadString();
    int64_t bytes = data.ReadInt64();
    int32_t result = BandwidthSetIfaceQuota(ifName, bytes);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdBandwidthRemoveIfaceQuota(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdBandwidthRemoveIfaceQuota");
    std::string ifName = data.ReadString();
    int32_t result = BandwidthRemoveIfaceQuota(ifName);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdBandwidthAddDeniedList(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdBandwidthAddDeniedList");
    uint32_t uid = data.ReadUint32();
    int32_t result = BandwidthAddDeniedList(uid);
    reply.WriteInt32(result);
    return result;
}
int32_t NetsysNativeServiceStub::CmdBandwidthRemoveDeniedList(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdBandwidthRemoveDeniedList");
    uint32_t uid = data.ReadUint32();
    int32_t result = BandwidthRemoveDeniedList(uid);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdBandwidthAddAllowedList(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdBandwidthAddAllowedList");
    uint32_t uid = data.ReadUint32();
    int32_t result = BandwidthAddAllowedList(uid);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdBandwidthRemoveAllowedList(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdBandwidthRemoveAllowedList");
    uint32_t uid = data.ReadUint32();
    int32_t result = BandwidthRemoveAllowedList(uid);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdFirewallSetUidsAllowedListChain(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdFirewallSetUidsAllowedListChain");
    uint32_t chain = data.ReadUint32();
    std::vector<uint32_t> uids;
    uint32_t uidSize = data.ReadUint32();
    uidSize = (uidSize > UIDS_LIST_MAX_SIZE) ? UIDS_LIST_MAX_SIZE : uidSize;
    for (uint32_t i = 0; i < uidSize; i++) {
        uint32_t uid = data.ReadUint32();
        uids.push_back(uid);
    }
    int32_t result = FirewallSetUidsAllowedListChain(chain, uids);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdFirewallSetUidsDeniedListChain(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdFirewallSetUidsDeniedListChain");
    uint32_t chain = data.ReadUint32();
    std::vector<uint32_t> uids;
    uint32_t uidSize = data.ReadUint32();
    uidSize = (uidSize > UIDS_LIST_MAX_SIZE) ? UIDS_LIST_MAX_SIZE : uidSize;
    for (uint32_t i = 0; i < uidSize; i++) {
        uint32_t uid = data.ReadUint32();
        uids.push_back(uid);
    }
    int32_t result = FirewallSetUidsDeniedListChain(chain, uids);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdFirewallEnableChain(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdFirewallEnableChain");
    uint32_t chain = data.ReadUint32();
    bool enable = data.ReadBool();
    int32_t result = FirewallEnableChain(chain, enable);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdFirewallSetUidRule(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd CmdFirewallSetUidRule");
    uint32_t chain = (unsigned)data.ReadUint32();
    std::vector<uint32_t> uids;
    data.ReadUInt32Vector(&uids);
    uint32_t firewallRule = (unsigned)data.ReadInt32();
    int32_t result = FirewallSetUidRule(chain, uids, firewallRule);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdShareDnsSet(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netId = 0;
    data.ReadUint16(netId);
    int32_t result = ShareDnsSet(netId);
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("ShareDnsSet has received result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdStartDnsProxyListen(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = StartDnsProxyListen();
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("StartDnsProxyListen has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdStopDnsProxyListen(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = StopDnsProxyListen();
    reply.WriteInt32(result);
    NETNATIVE_LOG_D("StopDnsProxyListen has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdGetNetworkSharingTraffic(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOG_D("Begin to dispatch cmd GetNetworkSharingTraffic");
    std::string downIface = data.ReadString();
    std::string upIface = data.ReadString();
    NetworkSharingTraffic traffic;
    int32_t result = GetNetworkSharingTraffic(downIface, upIface, traffic);
    reply.WriteInt32(result);
    reply.WriteInt64(traffic.receive);
    reply.WriteInt64(traffic.send);
    reply.WriteInt64(traffic.all);

    return result;
}

int32_t NetsysNativeServiceStub::CmdGetTotalStats(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = data.ReadUint32();
    uint64_t stats = 0;
    int32_t result = GetTotalStats(stats, type);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteUint64(stats)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdGetUidStats(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = data.ReadUint32();
    uint32_t uId = data.ReadUint32();
    uint64_t stats = 0;
    int32_t result = GetUidStats(stats, type, uId);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteUint64(stats)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdGetIfaceStats(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = data.ReadUint32();
    std::string interfaceName = data.ReadString();
    uint64_t stats = 0;
    int32_t result = GetIfaceStats(stats, type, interfaceName);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteUint64(stats)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdGetAllContainerStatsInfo(MessageParcel &data, MessageParcel &reply)
{
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> stats;
    int32_t result = GetAllContainerStatsInfo(stats);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!OHOS::NetManagerStandard::NetStatsInfo::Marshalling(reply, stats)) {
        NETNATIVE_LOGE("Read stats info failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}


int32_t NetsysNativeServiceStub::CmdGetAllStatsInfo(MessageParcel &data, MessageParcel &reply)
{
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> stats;
    int32_t result = GetAllStatsInfo(stats);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!OHOS::NetManagerStandard::NetStatsInfo::Marshalling(reply, stats)) {
        NETNATIVE_LOGE("Read stats info failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdSetIptablesCommandForRes(MessageParcel &data, MessageParcel &reply)
{
    if (!NetManagerStandard::NetManagerPermission::CheckNetSysInternalPermission(
        NetManagerStandard::Permission::NETSYS_INTERNAL)) {
        NETNATIVE_LOGE("CmdSetIptablesCommandForRes CheckNetSysInternalPermission failed");
        return NETMANAGER_ERR_PERMISSION_DENIED;
    }
    std::string cmd = data.ReadString();
    std::string respond;
    int32_t result = SetIptablesCommandForRes(cmd, respond);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write CmdSetIptablesCommandForRes result failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteString(respond)) {
        NETNATIVE_LOGE("Write CmdSetIptablesCommandForRes respond failed");
        return ERR_FLATTEN_OBJECT;
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetsysNativeServiceStub::CmdNetDiagPingHost(MessageParcel &data, MessageParcel &reply)
{
    NetDiagPingOption pingOption;
    if (!NetDiagPingOption::Unmarshalling(data, pingOption)) {
        NETNATIVE_LOGE("Unmarshalling failed.");
        return IPC_STUB_ERR;
    }

    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("remote is nullptr.");
        return IPC_STUB_ERR;
    }

    sptr<INetDiagCallback> callback = iface_cast<INetDiagCallback>(remote);
    int32_t result = NetDiagPingHost(pingOption, callback);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdNetDiagGetRouteTable(MessageParcel &data, MessageParcel &reply)
{
    std::list<NetDiagRouteTable> routeTables;
    int32_t result = NetDiagGetRouteTable(routeTables);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (result == NetManagerStandard::NETMANAGER_SUCCESS) {
        if (!reply.WriteUint32(
            static_cast<uint32_t>(std::min(MAX_ROUTE_TABLE_SIZE, static_cast<uint32_t>(routeTables.size()))))) {
            NETNATIVE_LOGE("Write uint32 failed");
            return ERR_FLATTEN_OBJECT;
        }
        uint32_t count = 0;
        for (const auto &routeTable : routeTables) {
            if (!routeTable.Marshalling(reply)) {
                NETNATIVE_LOGE("NetDiagRouteTable marshalling failed");
                return ERR_FLATTEN_OBJECT;
            }
            ++count;
            if (count >= MAX_ROUTE_TABLE_SIZE) {
                break;
            }
        }
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdNetDiagGetSocketsInfo(MessageParcel &data, MessageParcel &reply)
{
    uint8_t socketType = 0;
    if (!data.ReadUint8(socketType)) {
        NETNATIVE_LOGE("Read uint8 failed");
        return ERR_FLATTEN_OBJECT;
    }
    NetDiagSocketsInfo socketsInfo;
    int32_t result = NetDiagGetSocketsInfo(static_cast<NetDiagProtocolType>(socketType), socketsInfo);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (result == NetManagerStandard::NETMANAGER_SUCCESS) {
        if (!socketsInfo.Marshalling(reply)) {
            NETNATIVE_LOGE("NetDiagSocketsInfo marshalling failed.");
            return ERR_FLATTEN_OBJECT;
        }
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdNetDiagGetInterfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    std::string ifaceName;
    if (!data.ReadString(ifaceName)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }
    std::list<NetDiagIfaceConfig> configList;
    int32_t result = NetDiagGetInterfaceConfig(configList, ifaceName);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (result == NetManagerStandard::NETMANAGER_SUCCESS) {
        if (!reply.WriteUint32(
            static_cast<uint32_t>(std::min(MAX_CONFIG_LIST_SIZE, static_cast<uint32_t>(configList.size()))))) {
            NETNATIVE_LOGE("Write uint32 failed");
            return ERR_FLATTEN_OBJECT;
        }
        uint32_t count = 0;
        for (const auto &config : configList) {
            if (!config.Marshalling(reply)) {
                NETNATIVE_LOGE("NetDiagIfaceConfig marshalling failed");
                return ERR_FLATTEN_OBJECT;
            }
            ++count;
            if (count >= MAX_CONFIG_LIST_SIZE) {
                break;
            }
        }
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdNetDiagUpdateInterfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    NetDiagIfaceConfig config;
    if (!NetDiagIfaceConfig::Unmarshalling(data, config)) {
        NETNATIVE_LOGE("NetDiagIfaceConfig unmarshalling failed.");
        return IPC_STUB_ERR;
    }

    std::string ifaceName;
    if (!data.ReadString(ifaceName)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    bool add = false;
    if (!data.ReadBool(add)) {
        NETNATIVE_LOGE("Read bool failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = NetDiagUpdateInterfaceConfig(config, ifaceName, add);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdNetDiagSetInterfaceActiveState(MessageParcel &data, MessageParcel &reply)
{
    std::string ifaceName;
    if (!data.ReadString(ifaceName)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    bool up = false;
    if (!data.ReadBool(up)) {
        NETNATIVE_LOGE("Read bool failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = NetDiagSetInterfaceActiveState(ifaceName, up);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdAddStaticArp(MessageParcel &data, MessageParcel &reply)
{
    std::string ipAddr = "";
    if (!data.ReadString(ipAddr)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    std::string macAddr = "";
    if (!data.ReadString(macAddr)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = AddStaticArp(ipAddr, macAddr, ifName);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    NETNATIVE_LOG_D("CmdAddStaticArp has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdDelStaticArp(MessageParcel &data, MessageParcel &reply)
{
    std::string ipAddr = "";
    if (!data.ReadString(ipAddr)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    std::string macAddr = "";
    if (!data.ReadString(macAddr)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        NETNATIVE_LOGE("Read string failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = DelStaticArp(ipAddr, macAddr, ifName);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write result failed");
        return ERR_FLATTEN_OBJECT;
    }
    NETNATIVE_LOG_D("CmdDelStaticArp has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdRegisterDnsResultListener(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("Callback ptr is nullptr.");
        result = IPC_STUB_ERR;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetDnsResultCallback> callback = iface_cast<INetDnsResultCallback>(remote);
    if (callback == nullptr) {
        result = ERR_FLATTEN_OBJECT;
        reply.WriteInt32(result);
        return result;
    }

    uint32_t delay;
    if (!data.ReadUint32(delay)) {
        NETNATIVE_LOGE("Read uint32 failed");
        return ERR_FLATTEN_OBJECT;
    }

    result = RegisterDnsResultCallback(callback, delay);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdUnregisterDnsResultListener(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("Callback ptr is nullptr.");
        result = IPC_STUB_ERR;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetDnsResultCallback> callback = iface_cast<INetDnsResultCallback>(remote);
    if (callback == nullptr) {
        result = ERR_FLATTEN_OBJECT;
        reply.WriteInt32(result);
        return result;
    }

    result = UnregisterDnsResultCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdRegisterDnsHealthListener(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("Callback ptr is nullptr.");
        result = IPC_STUB_ERR;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetDnsHealthCallback> callback = iface_cast<INetDnsHealthCallback>(remote);
    if (callback == nullptr) {
        result = ERR_FLATTEN_OBJECT;
        reply.WriteInt32(result);
        return result;
    }

    result = RegisterDnsHealthCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdUnregisterDnsHealthListener(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETNATIVE_LOGE("Callback ptr is nullptr.");
        result = IPC_STUB_ERR;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetDnsHealthCallback> callback = iface_cast<INetDnsHealthCallback>(remote);
    if (callback == nullptr) {
        result = ERR_FLATTEN_OBJECT;
        reply.WriteInt32(result);
        return result;
    }

    result = UnregisterDnsHealthCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdGetCookieStats(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = 0;
    if (!data.ReadUint32(type)) {
        NETNATIVE_LOGE("Read uint32 failed");
        return ERR_FLATTEN_OBJECT;
    }

    uint64_t cookie = 0;
    if (!data.ReadUint64(cookie)) {
        NETNATIVE_LOGE("Read uint64 failed");
        return ERR_FLATTEN_OBJECT;
    }

    uint64_t stats = 0;
    int32_t result = GetCookieStats(stats, type, cookie);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteUint64(stats)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t NetsysNativeServiceStub::CmdGetNetworkSharingType(MessageParcel &data, MessageParcel &reply)
{
    std::set<uint32_t> sharingTypeIsOn;
    int32_t ret = GetNetworkSharingType(sharingTypeIsOn);
    if (!reply.WriteInt32(ret)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteUint32(sharingTypeIsOn.size())) {
            NETNATIVE_LOGE("Write parcel failed");
            return ERR_FLATTEN_OBJECT;
    }
    for (auto mem : sharingTypeIsOn) {
        if (!reply.WriteUint32(mem)) {
            NETNATIVE_LOGE("Write parcel failed");
            return ERR_FLATTEN_OBJECT;
        }
    }
    
    return ret;
}

int32_t NetsysNativeServiceStub::CmdUpdateNetworkSharingType(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = ERR_NONE;
    if (!data.ReadUint32(type)) {
        NETNATIVE_LOGE("Read uint32 failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (type < ERR_NONE) {
        NETNATIVE_LOGE("type parameter invalid");
        return ERR_INVALID_DATA;
    }

    bool isOpen = false;
    if (!data.ReadBool(isOpen)) {
        NETNATIVE_LOGE("Read bool failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = UpdateNetworkSharingType(type, isOpen);
    if (!reply.WriteInt32(ret)) {
        NETNATIVE_LOGE("Write parcel failed");
        return ERR_FLATTEN_OBJECT;
    }

    return ret;
}

int32_t NetsysNativeServiceStub::CmdSetIpv6PrivacyExtensions(MessageParcel &data, MessageParcel &reply)
{
    std::string interfaceName = data.ReadString();
    int32_t on = data.ReadInt32();

    int32_t result = SetIpv6PrivacyExtensions(interfaceName, on);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("SetIpv6PrivacyExtensions has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdSetIpv6Enable(MessageParcel &data, MessageParcel &reply)
{
    std::string interfaceName = data.ReadString();
    int32_t on = data.ReadInt32();

    int32_t result = SetEnableIpv6(interfaceName, on);
    reply.WriteInt32(result);
    NETNATIVE_LOGI("SetIpv6Enable has recved result %{public}d", result);

    return result;
}

int32_t NetsysNativeServiceStub::CmdSetNetworkAccessPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        NETNATIVE_LOGE("Read uint32 failed");
        return ERR_FLATTEN_OBJECT;
    }
    uint8_t wifi_allow = 0;
    if (!data.ReadUint8(wifi_allow)) {
        NETNATIVE_LOGE("Read uint8 failed");
        return ERR_FLATTEN_OBJECT;
    }
    uint8_t cellular_allow = 0;
    if (!data.ReadUint8(cellular_allow)) {
        NETNATIVE_LOGE("Read uint8 failed");
        return ERR_FLATTEN_OBJECT;
    }
    bool reconfirmFlag = true;
    if (!data.ReadBool(reconfirmFlag)) {
        NETNATIVE_LOGE("Read bool failed");
        return ERR_FLATTEN_OBJECT;
    }

    NetworkAccessPolicy policy;
    policy.wifiAllow = wifi_allow;
    policy.cellularAllow = cellular_allow;
    int32_t result = SetNetworkAccessPolicy(uid, policy, reconfirmFlag);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdDelNetworkAccessPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        NETNATIVE_LOGE("Read uint32 failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = DeleteNetworkAccessPolicy(uid);
    reply.WriteInt32(result);
    return result;
}

int32_t NetsysNativeServiceStub::CmdNotifyNetBearerTypeChange(MessageParcel &data, MessageParcel &reply)
{
    std::set<NetBearType> bearerTypes;

    uint32_t size = 0;
    uint32_t value = 0;
    if (!data.ReadUint32(size)) {
        return ERR_FLATTEN_OBJECT;
    }

    for (uint32_t i = 0; i < size; i++) {
        if (!data.ReadUint32(value)) {
            return ERR_FLATTEN_OBJECT;
        }
        if (value >= BEARER_DEFAULT) {
            return ERR_FLATTEN_OBJECT;
        }
        bearerTypes.insert(static_cast<NetBearType>(value));
    }
    int32_t result = NotifyNetBearerTypeChange(bearerTypes);
    reply.WriteInt32(result);
    return result;
}

} // namespace NetsysNative
} // namespace OHOS
