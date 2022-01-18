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
#include "netd_controller_service_impl.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetdControllerServiceImpl::NetdControllerServiceImpl()
{}

NetdControllerServiceImpl::~NetdControllerServiceImpl() {}

void NetdControllerServiceImpl::Init()
{
    mockNetdClient_.RegisterMockApi();
    return;
}

int32_t NetdControllerServiceImpl::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOG_I("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
    if (mockNetdClient_.CheckMockApi(MOCK_NETWORKCREATEPHYSICAL_API)) {
        return mockNetdClient_.NetworkCreatePhysical(netId, permission);
    }
    return netdClient_.NetworkCreatePhysical(netId, permission);
}

int32_t NetdControllerServiceImpl::NetworkDestroy(int32_t netId)
{
    NETMGR_LOG_I("Destroy network: netId[%{public}d]", netId);
    if (mockNetdClient_.CheckMockApi(MOCK_NETWORKDESTROY_API)) {
        return mockNetdClient_.NetworkDestroy(netId);
    }
    return netdClient_.NetworkDestroy(netId);
}

int32_t NetdControllerServiceImpl::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_NETWORKADDINTERFACE_API)) {
        return mockNetdClient_.NetworkAddInterface(netId, iface);
    }
    return netdClient_.NetworkAddInterface(netId, iface);
}

int32_t NetdControllerServiceImpl::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_NETWORKREMOVEINTERFACE_API)) {
        return mockNetdClient_.NetworkRemoveInterface(netId, iface);
    }
    return netdClient_.NetworkRemoveInterface(netId, iface);
}

int32_t NetdControllerServiceImpl::NetworkAddRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_NETWORKADDROUTE_API)) {
        return mockNetdClient_.NetworkAddRoute(netId, ifName, destination, nextHop);
    }
    return netdClient_.NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetdControllerServiceImpl::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_NETWORKREMOVEROUTE_API)) {
        return mockNetdClient_.NetworkRemoveRoute(netId, ifName, destination, nextHop);
    }
    return netdClient_.NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

void NetdControllerServiceImpl::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOG_I("Set interface down: iface[%{public}s]", iface.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_SETINTERFACEDOWN_API)) {
        return mockNetdClient_.SetInterfaceDown(iface);
    }
    return netdClient_.SetInterfaceDown(iface);
}

void NetdControllerServiceImpl::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOG_I("Set interface up: iface[%{public}s]", iface.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_SETINTERFACEUP_API)) {
        return mockNetdClient_.SetInterfaceUp(iface);
    }
    return netdClient_.SetInterfaceUp(iface);
}

void NetdControllerServiceImpl::InterfaceClearAddrs(const std::string &ifName)
{
    NETMGR_LOG_I("Clear addrs: ifName[%{public}s]", ifName.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_INTERFACECLEARADDRS_API)) {
        return mockNetdClient_.InterfaceClearAddrs(ifName);
    }
    return netdClient_.InterfaceClearAddrs(ifName);
}

int32_t NetdControllerServiceImpl::InterfaceGetMtu(const std::string &ifName)
{
    NETMGR_LOG_I("Get mtu: ifName[%{public}s]", ifName.c_str());
    if (mockNetdClient_.CheckMockApi(MOCK_INTERFACEGETMTU_API)) {
        return mockNetdClient_.InterfaceGetMtu(ifName);
    }
    return netdClient_.InterfaceGetMtu(ifName);
}

int32_t NetdControllerServiceImpl::InterfaceSetMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOG_I("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
    if (mockNetdClient_.CheckMockApi(MOCK_INTERFACESETMTU_API)) {
        return mockNetdClient_.InterfaceSetMtu(ifName, mtu);
    }
    return netdClient_.InterfaceSetMtu(ifName, mtu);
}

int32_t NetdControllerServiceImpl::InterfaceAddAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    if (mockNetdClient_.CheckMockApi(MOCK_INTERFACEADDADDRESS_API)) {
        return mockNetdClient_.InterfaceAddAddress(ifName, ipAddr, prefixLength);
    }
    return netdClient_.InterfaceAddAddress(ifName, ipAddr, prefixLength);
}

int32_t NetdControllerServiceImpl::InterfaceDelAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    if (mockNetdClient_.CheckMockApi(MOCK_INTERFACEDELADDRESS_API)) {
        return mockNetdClient_.InterfaceDelAddress(ifName, ipAddr, prefixLength);
    }
    return netdClient_.InterfaceDelAddress(ifName, ipAddr, prefixLength);
}

int32_t NetdControllerServiceImpl::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    NETMGR_LOG_I("Set resolver config: netId[%{public}d]", netId);
    if (mockNetdClient_.CheckMockApi(MOCK_SETRESOLVERCONFIG_API)) {
        return mockNetdClient_.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    }
    return netdClient_.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
}

int32_t NetdControllerServiceImpl::GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    NETMGR_LOG_I("Get resolver config: netId[%{public}d]", netId);
    if (mockNetdClient_.CheckMockApi(MOCK_GETRESOLVERINFO_API)) {
        return mockNetdClient_.GetResolverInfo(netId, servers, domains, baseTimeoutMsec, retryCount);
    }
    return netdClient_.GetResolverInfo(netId, servers, domains, baseTimeoutMsec, retryCount);
}

int32_t NetdControllerServiceImpl::CreateNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("create dns cache: netId[%{public}d]", netId);
    if (mockNetdClient_.CheckMockApi(MOCK_CREATENETWORKCACHE_API)) {
        return mockNetdClient_.CreateNetworkCache(netId);
    }
    return netdClient_.CreateNetworkCache(netId);
}

int32_t NetdControllerServiceImpl::DestoryNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory dns cache: netId[%{public}d]", netId);
    if (mockNetdClient_.CheckMockApi(MOCK_DESTORYNETWORKCACHE_API)) {
        return mockNetdClient_.DestoryNetworkCache(netId);
    }
    return netdClient_.DestoryNetworkCache(netId);
}

int32_t NetdControllerServiceImpl::FlushNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory Flush dns cache: netId[%{public}d]", netId);
    if (mockNetdClient_.CheckMockApi(MOCK_FLUSHNETWORKCACHE_API)) {
        return mockNetdClient_.FlushNetworkCache(netId);
    }
    return netdClient_.FlushNetworkCache(netId);
}

int32_t NetdControllerServiceImpl::GetAddrInfo(const std::string &hostName, const std::string &serverName,
    const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId)
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetAddrInfo");
    if (mockNetdClient_.CheckMockApi(MOCK_GETADDRINFO_API)) {
        return mockNetdClient_.GetAddrInfo(hostName, serverName, hints, res, netId);
    }
    return netdClient_.GetAddrInfo(hostName, serverName, hints, res, netId);
}

int64_t NetdControllerServiceImpl::GetCellularRxBytes()
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetCellularRxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETCELLULARRXBYTES_API)) {
        return mockNetdClient_.GetCellularRxBytes();
    }
    return netdClient_.GetCellularRxBytes();
}

int64_t NetdControllerServiceImpl::GetCellularTxBytes()
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetCellularTxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETCELLULARTXBYTES_API)) {
        return mockNetdClient_.GetCellularTxBytes();
    }
    return netdClient_.GetCellularTxBytes();
}

int64_t NetdControllerServiceImpl::GetAllRxBytes()
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetAllRxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETALLRXBYTES_API)) {
        return mockNetdClient_.GetAllRxBytes();
    }
    return netdClient_.GetAllRxBytes();
}

int64_t NetdControllerServiceImpl::GetAllTxBytes()
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetAllTxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETALLTXBYTES_API)) {
        return mockNetdClient_.GetAllTxBytes();
    }
    return netdClient_.GetAllTxBytes();
}

int64_t NetdControllerServiceImpl::GetUidRxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetUidRxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETUIDRXBYTES_API)) {
        return mockNetdClient_.GetUidRxBytes(uid);
    }
    return netdClient_.GetUidRxBytes(uid);
}

int64_t NetdControllerServiceImpl::GetUidTxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetUidTxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETUIDTXBYTES_API)) {
        return mockNetdClient_.GetUidTxBytes(uid);
    }
    return netdClient_.GetUidTxBytes(uid);
}

int64_t NetdControllerServiceImpl::GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetUidOnIfaceRxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETUIDRXBYTES_API)) {
        return mockNetdClient_.GetUidOnIfaceRxBytes(uid, interfaceName);
    }
    return netdClient_.GetUidOnIfaceRxBytes(uid, interfaceName);
}

int64_t NetdControllerServiceImpl::GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetUidOnIfaceTxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETUIDTXBYTES_API)) {
        return mockNetdClient_.GetUidOnIfaceTxBytes(uid, interfaceName);
    }
    return netdClient_.GetUidOnIfaceTxBytes(uid, interfaceName);
}

int64_t NetdControllerServiceImpl::GetIfaceRxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetIfaceRxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETIFACERXBYTES_API)) {
        return mockNetdClient_.GetIfaceRxBytes(interfaceName);
    }
    return netdClient_.GetIfaceRxBytes(interfaceName);
}

int64_t NetdControllerServiceImpl::GetIfaceTxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdControllerServiceImpl GetIfaceTxBytes");
    if (mockNetdClient_.CheckMockApi(MOCK_GETIFACETXBYTES_API)) {
        return mockNetdClient_.GetIfaceTxBytes(interfaceName);
    }
    return netdClient_.GetIfaceTxBytes(interfaceName);
}

std::vector<std::string> NetdControllerServiceImpl::InterfaceGetList()
{
    NETMGR_LOG_I("NetdControllerServiceImpl InterfaceGetList");
    if (mockNetdClient_.CheckMockApi(MOCK_INTERFACEGETLIST_API)) {
        return mockNetdClient_.InterfaceGetList();
    }
    return netdClient_.InterfaceGetList();
}

std::vector<std::string> NetdControllerServiceImpl::UidGetList()
{
    NETMGR_LOG_I("NetdControllerServiceImpl UidGetList");
    if (mockNetdClient_.CheckMockApi(MOCK_UIDGETLIST_API)) {
        return mockNetdClient_.UidGetList();
    }
    return netdClient_.UidGetList();
}

int64_t NetdControllerServiceImpl::GetIfaceRxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetdControllerServiceImpl GetIfaceRxPackets");
    if (mockNetdClient_.CheckMockApi(MOCK_GETIFACERXPACKETS_API)) {
        return mockNetdClient_.GetIfaceRxPackets(interfaceName);
    }
    return netdClient_.GetIfaceRxPackets(interfaceName);
}

int64_t NetdControllerServiceImpl::GetIfaceTxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetdControllerServiceImpl GetIfaceTxPackets");
    if (mockNetdClient_.CheckMockApi(MOCK_GETIFACETXPACKETS_API)) {
        return mockNetdClient_.GetIfaceTxPackets(interfaceName);
    }
    return netdClient_.GetIfaceTxPackets(interfaceName);
}

int32_t NetdControllerServiceImpl::SetDefaultNetWork(int32_t netId)
{
    NETMGR_LOG_D("NetdControllerServiceImpl SetDefaultNetWork");
    if (mockNetdClient_.CheckMockApi(MOCK_SETDEFAULTNETWORK_API)) {
        return mockNetdClient_.SetDefaultNetWork(netId);
    }
    return netdClient_.SetDefaultNetWork(netId);
}

int32_t NetdControllerServiceImpl::ClearDefaultNetWorkNetId()
{
    NETMGR_LOG_D("NetdControllerServiceImpl ClearDefaultNetWorkNetId");
    if (mockNetdClient_.CheckMockApi(MOCK_CLEARDEFAULTNETWORK_API)) {
        return mockNetdClient_.ClearDefaultNetWorkNetId();
    }
    return netdClient_.ClearDefaultNetWorkNetId();
}

int32_t NetdControllerServiceImpl::BindSocket(int32_t socket_fd, uint32_t netId)
{
    NETMGR_LOG_D("NetdControllerServiceImpl BindSocket");
    if (mockNetdClient_.CheckMockApi(MOCK_BINDSOCKET_API)) {
        return mockNetdClient_.BindSocket(socket_fd, netId);
    }
    return netdClient_.BindSocket(socket_fd, netId);
}

int32_t NetdControllerServiceImpl::IpEnableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("NetdControllerServiceImpl IpEnableForwarding");
    if (mockNetdClient_.CheckMockApi(MOCK_IPENABLEFORWARDING_API)) {
        return mockNetdClient_.IpEnableForwarding(requester);
    }
    return netdClient_.IpEnableForwarding(requester);
}

int32_t NetdControllerServiceImpl::IpDisableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("NetdControllerServiceImpl IpDisableForwarding");
    if (mockNetdClient_.CheckMockApi(MOCK_IPDISABLEFORWARDING_API)) {
        return mockNetdClient_.IpDisableForwarding(requester);
    }
    return netdClient_.IpDisableForwarding(requester);
}

int32_t NetdControllerServiceImpl::TetherAddForward(const std::string& downstreamIface,
    const std::string& upstreamIface)
{
    NETMGR_LOG_D("NetdControllerServiceImpl TetherAddForward");
    if (mockNetdClient_.CheckMockApi(MOCK_TETHERADDFORWARD_API)) {
        return mockNetdClient_.TetherAddForward(downstreamIface, upstreamIface);
    }
    return netdClient_.TetherAddForward(downstreamIface, upstreamIface);
}

int32_t NetdControllerServiceImpl::TetherRemoveForward(const std::string& downstreamIface,
    const std::string& upstreamIface)
{
    NETMGR_LOG_D("NetdControllerServiceImpl TetherRemoveForward");
    if (mockNetdClient_.CheckMockApi(MOCK_TETHERREMOVEFORWARD_API)) {
        return mockNetdClient_.TetherRemoveForward(downstreamIface, upstreamIface);
    }
    return netdClient_.TetherRemoveForward(downstreamIface, upstreamIface);
}

int32_t NetdControllerServiceImpl::IpfwdAddInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("NetdControllerServiceImpl IpfwdAddInterfaceForward");
    if (mockNetdClient_.CheckMockApi(MOCK_IPFWDADDIFACEFORWARD_API)) {
        return mockNetdClient_.IpfwdAddInterfaceForward(fromIface, toIface);
    }
    return netdClient_.IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetdControllerServiceImpl::IpfwdRemoveInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("NetdControllerServiceImpl IpfwdRemoveInterfaceForward");
    if (mockNetdClient_.CheckMockApi(MOCK_IPFWDREMOVEIFACEFORWARD_API)) {
        return mockNetdClient_.IpfwdRemoveInterfaceForward(fromIface, toIface);
    }
    return netdClient_.IpfwdRemoveInterfaceForward(fromIface, toIface);
}

int32_t NetdControllerServiceImpl::TetherDnsSet(uint32_t netId, const std::vector<std::string>& dnsAddrs)
{
    NETMGR_LOG_D("NetdControllerServiceImpl IpfwdRemoveInterfaceForward");
    if (mockNetdClient_.CheckMockApi(MOCK_TETHERDNSSET_API)) {
        return mockNetdClient_.TetherDnsSet(netId, dnsAddrs);
    }
    return netdClient_.TetherDnsSet(netId, dnsAddrs);
}

int32_t NetdControllerServiceImpl::RegisterNetdNotifyCallback(const NetdNotifyCallback &callback)
{
    NETMGR_LOG_D("NetdControllerServiceImpl IpfwdRemoveInterfaceForward");
    if (mockNetdClient_.CheckMockApi(MOCK_REGISTERNETDNOTIFYCALLBACK_API)) {
        return mockNetdClient_.RegisterNetdNotifyCallback(callback);
    }
    return netdClient_.RegisterNetdNotifyCallback(callback);
}


int32_t NetdControllerServiceImpl::BindNetworkServiceVpn(int32_t socketFd)
{
    NETMGR_LOG_D("NetdNativeClient::BindNetworkServiceVpn");
    if (mockNetdClient_.CheckMockApi(MOCK_BINDNETWORKSERVICEVPN_API)) {
        return mockNetdClient_.BindNetworkServiceVpn(socketFd);
    }
    return netdClient_.BindNetworkServiceVpn(socketFd);
}

int32_t NetdControllerServiceImpl::EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest,
    int32_t &ifaceFd)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::EnableVirtualNetIfaceCard");
    if (mockNetdClient_.CheckMockApi(MOCK_ENABLEVIRTUALNETIFACECARD_API)) {
        return mockNetdClient_.EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
    }
    return netdClient_.EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
}

int32_t NetdControllerServiceImpl::SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
    struct ifreq &ifRequest)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::SetIpAddress");
    if (mockNetdClient_.CheckMockApi(MOCK_SETIPADDRESS_API)) {
        return mockNetdClient_.SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    }
    return netdClient_.SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
}

int32_t NetdControllerServiceImpl::SetBlocking(int32_t ifaceFd, bool isBlock)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::SetBlocking");
    if (mockNetdClient_.CheckMockApi(MOCK_SETBLOCKING_API)) {
        return mockNetdClient_.SetBlocking(ifaceFd, isBlock);
    }
    return netdClient_.SetBlocking(ifaceFd, isBlock);
}

int32_t NetdControllerServiceImpl::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::StartDhcpClient");
    if (mockNetdClient_.CheckMockApi(MOCK_STARTDHCPCLIENT_API)) {
        return mockNetdClient_.StartDhcpClient(iface, bIpv6);
    }
    return netdClient_.StartDhcpClient(iface, bIpv6);
}

int32_t NetdControllerServiceImpl::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::StopDhcpClient");
    if (mockNetdClient_.CheckMockApi(MOCK_STOPDHCPCLIENT_API)) {
        return mockNetdClient_.StopDhcpClient(iface, bIpv6);
    }
    return netdClient_.StopDhcpClient(iface, bIpv6);
}

int32_t NetdControllerServiceImpl::RegisterCallback(sptr<NetdControllerCallback> callback)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::RegisterCallback");
    if (mockNetdClient_.CheckMockApi(MOCK_REGISTERNOTIFYCALLBACK_API)) {
        return mockNetdClient_.RegisterCallback(callback);
    }
    return netdClient_.RegisterCallback(callback);
}

int32_t NetdControllerServiceImpl::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::SetBlocking");
    if (mockNetdClient_.CheckMockApi(MOCK_STARTDHCPSERVICE_API)) {
        return mockNetdClient_.StartDhcpService(iface, ipv4addr);
    }
    return netdClient_.StartDhcpService(iface, ipv4addr);
}

int32_t NetdControllerServiceImpl::StopDhcpService(const std::string &iface)
{
    NETMGR_LOG_D("NetdControllerServiceImpl::StopDhcpService");
    if (mockNetdClient_.CheckMockApi(MOCK_STOPDHCPSERVICE_API)) {
        return mockNetdClient_.StopDhcpService(iface);
    }
    return netdClient_.StopDhcpService(iface);
}
} // namespace NetManagerStandard
} // namespace OHOS