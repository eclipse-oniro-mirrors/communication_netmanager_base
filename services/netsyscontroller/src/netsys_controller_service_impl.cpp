/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "netsys_controller_service_impl.h"

#include "net_mgr_log_wrapper.h"
#include "netmanager_base_common_utils.h"

using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace OHOS::NetsysNative;
} // namespace
	
void NetsysControllerServiceImpl::Init()
{
    mockNetsysClient_.RegisterMockApi();
}

int32_t NetsysControllerServiceImpl::SetInternetPermission(uint32_t uid, uint8_t allow)
{
    return netsysClient_.SetInternetPermission(uid, allow);
}

int32_t NetsysControllerServiceImpl::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOG_I("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
    if (mockNetsysClient_.CheckMockApi(MOCK_NETWORKCREATEPHYSICAL_API)) {
        return mockNetsysClient_.NetworkCreatePhysical(netId, permission);
    }
    return netsysClient_.NetworkCreatePhysical(netId, permission);
}

int32_t NetsysControllerServiceImpl::NetworkCreateVirtual(int32_t netId, bool hasDns)
{
    NETMGR_LOG_I("Create Virtual network: netId[%{public}d], hasDns[%{public}d]", netId, hasDns);
    return netsysClient_.NetworkCreateVirtual(netId, hasDns);
}

int32_t NetsysControllerServiceImpl::NetworkDestroy(int32_t netId)
{
    NETMGR_LOG_I("Destroy network: netId[%{public}d]", netId);
    if (mockNetsysClient_.CheckMockApi(MOCK_NETWORKDESTROY_API)) {
        return mockNetsysClient_.NetworkDestroy(netId);
    }
    return netsysClient_.NetworkDestroy(netId);
}

int32_t NetsysControllerServiceImpl::NetworkAddUids(int32_t netId, const std::vector<UidRange> &uidRanges)
{
    NETMGR_LOG_I("Add uids to vpn network: netId[%{public}d]", netId);
    return netsysClient_.NetworkAddUids(netId, uidRanges);
}

int32_t NetsysControllerServiceImpl::NetworkDelUids(int32_t netId, const std::vector<UidRange> &uidRanges)
{
    NETMGR_LOG_I("Remove uids from vpn network: netId[%{public}d]", netId);
    return netsysClient_.NetworkDelUids(netId, uidRanges);
}

int32_t NetsysControllerServiceImpl::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_NETWORKADDINTERFACE_API)) {
        return mockNetsysClient_.NetworkAddInterface(netId, iface);
    }
    return netsysClient_.NetworkAddInterface(netId, iface);
}

int32_t NetsysControllerServiceImpl::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_NETWORKREMOVEINTERFACE_API)) {
        return mockNetsysClient_.NetworkRemoveInterface(netId, iface);
    }
    return netsysClient_.NetworkRemoveInterface(netId, iface);
}

int32_t NetsysControllerServiceImpl::NetworkAddRoute(int32_t netId, const std::string &ifName,
                                                     const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
                 netId, ifName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_NETWORKADDROUTE_API)) {
        return mockNetsysClient_.NetworkAddRoute(netId, ifName, destination, nextHop);
    }
    return netsysClient_.NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetsysControllerServiceImpl::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
                                                        const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
                 netId, ifName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_NETWORKREMOVEROUTE_API)) {
        return mockNetsysClient_.NetworkRemoveRoute(netId, ifName, destination, nextHop);
    }
    return netsysClient_.NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

int32_t NetsysControllerServiceImpl::GetInterfaceConfig(OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    NETMGR_LOG_I("Interface get config");
    return netsysClient_.GetInterfaceConfig(cfg);
}

int32_t NetsysControllerServiceImpl::SetInterfaceConfig(const OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    NETMGR_LOG_I("Interface set config");
    return netsysClient_.SetInterfaceConfig(cfg);
}

int32_t NetsysControllerServiceImpl::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOG_I("Set interface down: iface[%{public}s]", iface.c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_SETINTERFACEDOWN_API)) {
        return mockNetsysClient_.SetInterfaceDown(iface);
    }
    return netsysClient_.SetInterfaceDown(iface);
}

int32_t NetsysControllerServiceImpl::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOG_I("Set interface up: iface[%{public}s]", iface.c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_SETINTERFACEUP_API)) {
        return mockNetsysClient_.SetInterfaceUp(iface);
    }
    return netsysClient_.SetInterfaceUp(iface);
}

void NetsysControllerServiceImpl::ClearInterfaceAddrs(const std::string &ifName)
{
    NETMGR_LOG_I("Clear addrs: ifName[%{public}s]", ifName.c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_INTERFACECLEARADDRS_API)) {
        return mockNetsysClient_.ClearInterfaceAddrs(ifName);
    }
    return netsysClient_.ClearInterfaceAddrs(ifName);
}

int32_t NetsysControllerServiceImpl::GetInterfaceMtu(const std::string &ifName)
{
    NETMGR_LOG_I("Get mtu: ifName[%{public}s]", ifName.c_str());
    if (mockNetsysClient_.CheckMockApi(MOCK_INTERFACEGETMTU_API)) {
        return mockNetsysClient_.GetInterfaceMtu(ifName);
    }
    return netsysClient_.GetInterfaceMtu(ifName);
}

int32_t NetsysControllerServiceImpl::SetInterfaceMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOG_I("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
    if (mockNetsysClient_.CheckMockApi(MOCK_INTERFACESETMTU_API)) {
        return mockNetsysClient_.SetInterfaceMtu(ifName, mtu);
    }
    return netsysClient_.SetInterfaceMtu(ifName, mtu);
}

int32_t NetsysControllerServiceImpl::SetTcpBufferSizes(const std::string &tcpBufferSizes)
{
    NETMGR_LOG_I("Set tcp buffer sizes: tcpBufferSizes[%{public}s]", tcpBufferSizes.c_str());
    return netsysClient_.SetTcpBufferSizes(tcpBufferSizes);
}

int32_t NetsysControllerServiceImpl::AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                                         int32_t prefixLength)
{
    NETMGR_LOG_I("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]", ifName.c_str(),
                 ToAnonymousIp(ipAddr).c_str(), prefixLength);
    if (mockNetsysClient_.CheckMockApi(MOCK_INTERFACEADDADDRESS_API)) {
        return mockNetsysClient_.AddInterfaceAddress(ifName, ipAddr, prefixLength);
    }
    return netsysClient_.AddInterfaceAddress(ifName, ipAddr, prefixLength);
}

int32_t NetsysControllerServiceImpl::DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                                         int32_t prefixLength)
{
    NETMGR_LOG_I("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]", ifName.c_str(),
                 ToAnonymousIp(ipAddr).c_str(), prefixLength);
    if (mockNetsysClient_.CheckMockApi(MOCK_INTERFACEDELADDRESS_API)) {
        return mockNetsysClient_.DelInterfaceAddress(ifName, ipAddr, prefixLength);
    }
    return netsysClient_.DelInterfaceAddress(ifName, ipAddr, prefixLength);
}

int32_t NetsysControllerServiceImpl::InterfaceSetIpAddress(const std::string &ifaceName, const std::string &ipAddress)
{
    NETMGR_LOG_I("set ip address: ifName[%{public}s], ipAddr[%{public}s]", ifaceName.c_str(),
                 ToAnonymousIp(ipAddress).c_str());
    return netsysClient_.InterfaceSetIpAddress(ifaceName, ipAddress);
}

int32_t NetsysControllerServiceImpl::InterfaceSetIffUp(const std::string &ifaceName)
{
    NETMGR_LOG_I("set iff up: ifName[%{public}s]", ifaceName.c_str());
    return netsysClient_.InterfaceSetIffUp(ifaceName);
}

int32_t NetsysControllerServiceImpl::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                                       const std::vector<std::string> &servers,
                                                       const std::vector<std::string> &domains)
{
    NETMGR_LOG_I("Set resolver config: netId[%{public}d]", netId);
    if (mockNetsysClient_.CheckMockApi(MOCK_SETRESOLVERCONFIG_API)) {
        return mockNetsysClient_.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    }
    return netsysClient_.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
}

int32_t NetsysControllerServiceImpl::GetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
                                                       std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                                       uint8_t &retryCount)
{
    NETMGR_LOG_I("Get resolver config: netId[%{public}d]", netId);
    if (mockNetsysClient_.CheckMockApi(MOCK_GETRESOLVERICONFIG_API)) {
        return mockNetsysClient_.GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
    }
    return netsysClient_.GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
}

int32_t NetsysControllerServiceImpl::CreateNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("create dns cache: netId[%{public}d]", netId);
    if (mockNetsysClient_.CheckMockApi(MOCK_CREATENETWORKCACHE_API)) {
        return mockNetsysClient_.CreateNetworkCache(netId);
    }
    return netsysClient_.CreateNetworkCache(netId);
}

int32_t NetsysControllerServiceImpl::DestroyNetworkCache(uint16_t netId)
{
    NETMGR_LOG_D("Destroy dns cache: netId[%{public}d]", netId);
    return netsysClient_.DestroyNetworkCache(netId);
}

int32_t NetsysControllerServiceImpl::GetAddrInfo(const std::string &hostName, const std::string &serverName,
                                                 const AddrInfo &hints, uint16_t netId, std::vector<AddrInfo> &res)
{
    return netsysClient_.GetAddrInfo(hostName, serverName, hints, netId, res);
}

int32_t NetsysControllerServiceImpl::GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                                              nmd::NetworkSharingTraffic &traffic)
{
    NETMGR_LOG_I("GetNetworkSharingTraffic");
    return netsysClient_.GetNetworkSharingTraffic(downIface, upIface, traffic);
}

int64_t NetsysControllerServiceImpl::GetCellularRxBytes()
{
    NETMGR_LOG_I("GetCellularRxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETCELLULARRXBYTES_API)) {
        return mockNetsysClient_.GetCellularRxBytes();
    }
    return netsysClient_.GetCellularRxBytes();
}

int64_t NetsysControllerServiceImpl::GetCellularTxBytes()
{
    NETMGR_LOG_I("GetCellularTxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETCELLULARTXBYTES_API)) {
        return mockNetsysClient_.GetCellularTxBytes();
    }
    return netsysClient_.GetCellularTxBytes();
}

int64_t NetsysControllerServiceImpl::GetAllRxBytes()
{
    NETMGR_LOG_I("GetAllRxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETALLRXBYTES_API)) {
        return mockNetsysClient_.GetAllRxBytes();
    }
    return netsysClient_.GetAllRxBytes();
}

int64_t NetsysControllerServiceImpl::GetAllTxBytes()
{
    NETMGR_LOG_I("GetAllTxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETALLTXBYTES_API)) {
        return mockNetsysClient_.GetAllTxBytes();
    }
    return netsysClient_.GetAllTxBytes();
}

int64_t NetsysControllerServiceImpl::GetUidRxBytes(uint32_t uid)
{
    NETMGR_LOG_I("GetUidRxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETUIDRXBYTES_API)) {
        return mockNetsysClient_.GetUidRxBytes(uid);
    }
    return netsysClient_.GetUidRxBytes(uid);
}

int64_t NetsysControllerServiceImpl::GetUidTxBytes(uint32_t uid)
{
    NETMGR_LOG_I("GetUidTxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETUIDTXBYTES_API)) {
        return mockNetsysClient_.GetUidTxBytes(uid);
    }
    return netsysClient_.GetUidTxBytes(uid);
}

int64_t NetsysControllerServiceImpl::GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("GetUidOnIfaceRxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETUIDRXBYTES_API)) {
        return mockNetsysClient_.GetUidOnIfaceRxBytes(uid, interfaceName);
    }
    return netsysClient_.GetUidOnIfaceRxBytes(uid, interfaceName);
}

int64_t NetsysControllerServiceImpl::GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("GetUidOnIfaceTxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETUIDTXBYTES_API)) {
        return mockNetsysClient_.GetUidOnIfaceTxBytes(uid, interfaceName);
    }
    return netsysClient_.GetUidOnIfaceTxBytes(uid, interfaceName);
}

int64_t NetsysControllerServiceImpl::GetIfaceRxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("GetIfaceRxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETIFACERXBYTES_API)) {
        return mockNetsysClient_.GetIfaceRxBytes(interfaceName);
    }
    return netsysClient_.GetIfaceRxBytes(interfaceName);
}

int64_t NetsysControllerServiceImpl::GetIfaceTxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("GetIfaceTxBytes");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETIFACETXBYTES_API)) {
        return mockNetsysClient_.GetIfaceTxBytes(interfaceName);
    }
    return netsysClient_.GetIfaceTxBytes(interfaceName);
}

std::vector<std::string> NetsysControllerServiceImpl::InterfaceGetList()
{
    NETMGR_LOG_I("InterfaceGetList");
    if (mockNetsysClient_.CheckMockApi(MOCK_INTERFACEGETLIST_API)) {
        return mockNetsysClient_.InterfaceGetList();
    }
    return netsysClient_.InterfaceGetList();
}

std::vector<std::string> NetsysControllerServiceImpl::UidGetList()
{
    NETMGR_LOG_I("UidGetList");
    if (mockNetsysClient_.CheckMockApi(MOCK_UIDGETLIST_API)) {
        return mockNetsysClient_.UidGetList();
    }
    return netsysClient_.UidGetList();
}

int64_t NetsysControllerServiceImpl::GetIfaceRxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("GetIfaceRxPackets");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETIFACERXPACKETS_API)) {
        return mockNetsysClient_.GetIfaceRxPackets(interfaceName);
    }
    return netsysClient_.GetIfaceRxPackets(interfaceName);
}

int64_t NetsysControllerServiceImpl::GetIfaceTxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("GetIfaceTxPackets");
    if (mockNetsysClient_.CheckMockApi(MOCK_GETIFACETXPACKETS_API)) {
        return mockNetsysClient_.GetIfaceTxPackets(interfaceName);
    }
    return netsysClient_.GetIfaceTxPackets(interfaceName);
}

int32_t NetsysControllerServiceImpl::SetDefaultNetWork(int32_t netId)
{
    NETMGR_LOG_D("SetDefaultNetWork");
    if (mockNetsysClient_.CheckMockApi(MOCK_SETDEFAULTNETWORK_API)) {
        return mockNetsysClient_.SetDefaultNetWork(netId);
    }
    return netsysClient_.SetDefaultNetWork(netId);
}

int32_t NetsysControllerServiceImpl::ClearDefaultNetWorkNetId()
{
    NETMGR_LOG_D("ClearDefaultNetWorkNetId");
    if (mockNetsysClient_.CheckMockApi(MOCK_CLEARDEFAULTNETWORK_API)) {
        return mockNetsysClient_.ClearDefaultNetWorkNetId();
    }
    return netsysClient_.ClearDefaultNetWorkNetId();
}

int32_t NetsysControllerServiceImpl::BindSocket(int32_t socketFd, uint32_t netId)
{
    NETMGR_LOG_D("BindSocket");
    if (mockNetsysClient_.CheckMockApi(MOCK_BINDSOCKET_API)) {
        return mockNetsysClient_.BindSocket(socketFd, netId);
    }
    return netsysClient_.BindSocket(socketFd, netId);
}

int32_t NetsysControllerServiceImpl::IpEnableForwarding(const std::string &requestor)
{
    NETMGR_LOG_D("IpEnableForwarding");
    return netsysClient_.IpEnableForwarding(requestor);
}

int32_t NetsysControllerServiceImpl::IpDisableForwarding(const std::string &requestor)
{
    NETMGR_LOG_D("IpDisableForwarding");
    return netsysClient_.IpDisableForwarding(requestor);
}

int32_t NetsysControllerServiceImpl::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETMGR_LOG_D("EnableNat");
    return netsysClient_.EnableNat(downstreamIface, upstreamIface);
}

int32_t NetsysControllerServiceImpl::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETMGR_LOG_D("DisableNat");
    return netsysClient_.DisableNat(downstreamIface, upstreamIface);
}

int32_t NetsysControllerServiceImpl::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    NETMGR_LOG_D("IpfwdAddInterfaceForward");
    return netsysClient_.IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetsysControllerServiceImpl::IpfwdRemoveInterfaceForward(const std::string &fromIface,
                                                                 const std::string &toIface)
{
    NETMGR_LOG_D("IpfwdRemoveInterfaceForward");
    return netsysClient_.IpfwdRemoveInterfaceForward(fromIface, toIface);
}

int32_t NetsysControllerServiceImpl::ShareDnsSet(uint16_t netId)
{
    NETMGR_LOG_D("IpfwdRemoveInterfaceForward");
    if (mockNetsysClient_.CheckMockApi(MOCK_SHAREDNSSET_API)) {
        return mockNetsysClient_.ShareDnsSet(netId);
    }
    return netsysClient_.ShareDnsSet(netId);
}

int32_t NetsysControllerServiceImpl::StartDnsProxyListen()
{
    NETMGR_LOG_D("StartDnsProxyListen");
    return netsysClient_.StartDnsProxyListen();
}

int32_t NetsysControllerServiceImpl::StopDnsProxyListen()
{
    NETMGR_LOG_D("StopDnsProxyListen");
    return netsysClient_.StopDnsProxyListen();
}

int32_t NetsysControllerServiceImpl::RegisterNetsysNotifyCallback(const NetsysNotifyCallback &callback)
{
    NETMGR_LOG_D("IpfwdRemoveInterfaceForward");
    if (mockNetsysClient_.CheckMockApi(MOCK_REGISTERNETSYSNOTIFYCALLBACK_API)) {
        return mockNetsysClient_.RegisterNetsysNotifyCallback(callback);
    }
    return netsysClient_.RegisterNetsysNotifyCallback(callback);
}

int32_t NetsysControllerServiceImpl::BindNetworkServiceVpn(int32_t socketFd)
{
    NETMGR_LOG_D("BindNetworkServiceVpn");
    if (mockNetsysClient_.CheckMockApi(MOCK_BINDNETWORKSERVICEVPN_API)) {
        return mockNetsysClient_.BindNetworkServiceVpn(socketFd);
    }
    return netsysClient_.BindNetworkServiceVpn(socketFd);
}

int32_t NetsysControllerServiceImpl::EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest,
                                                               int32_t &ifaceFd)
{
    NETMGR_LOG_D("EnableVirtualNetIfaceCard");
    if (mockNetsysClient_.CheckMockApi(MOCK_ENABLEVIRTUALNETIFACECARD_API)) {
        return mockNetsysClient_.EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
    }
    return netsysClient_.EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
}

int32_t NetsysControllerServiceImpl::SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
                                                  struct ifreq &ifRequest)
{
    NETMGR_LOG_D("SetIpAddress");
    if (mockNetsysClient_.CheckMockApi(MOCK_SETIPADDRESS_API)) {
        return mockNetsysClient_.SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    }
    return netsysClient_.SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
}

int32_t NetsysControllerServiceImpl::SetBlocking(int32_t ifaceFd, bool isBlock)
{
    NETMGR_LOG_D("SetBlocking");
    if (mockNetsysClient_.CheckMockApi(MOCK_SETBLOCKING_API)) {
        return mockNetsysClient_.SetBlocking(ifaceFd, isBlock);
    }
    return netsysClient_.SetBlocking(ifaceFd, isBlock);
}

int32_t NetsysControllerServiceImpl::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("StartDhcpClient");
    if (mockNetsysClient_.CheckMockApi(MOCK_STARTDHCPCLIENT_API)) {
        return mockNetsysClient_.StartDhcpClient(iface, bIpv6);
    }
    return netsysClient_.StartDhcpClient(iface, bIpv6);
}

int32_t NetsysControllerServiceImpl::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("StopDhcpClient");
    if (mockNetsysClient_.CheckMockApi(MOCK_STOPDHCPCLIENT_API)) {
        return mockNetsysClient_.StopDhcpClient(iface, bIpv6);
    }
    return netsysClient_.StopDhcpClient(iface, bIpv6);
}

int32_t NetsysControllerServiceImpl::RegisterCallback(sptr<NetsysControllerCallback> callback)
{
    NETMGR_LOG_D("RegisterCallback");
    if (mockNetsysClient_.CheckMockApi(MOCK_REGISTERNOTIFYCALLBACK_API)) {
        return mockNetsysClient_.RegisterCallback(callback);
    }
    return netsysClient_.RegisterCallback(callback);
}

int32_t NetsysControllerServiceImpl::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETMGR_LOG_D("SetBlocking");
    if (mockNetsysClient_.CheckMockApi(MOCK_STARTDHCPSERVICE_API)) {
        return mockNetsysClient_.StartDhcpService(iface, ipv4addr);
    }
    return netsysClient_.StartDhcpService(iface, ipv4addr);
}

int32_t NetsysControllerServiceImpl::StopDhcpService(const std::string &iface)
{
    NETMGR_LOG_D("StopDhcpService");
    if (mockNetsysClient_.CheckMockApi(MOCK_STOPDHCPSERVICE_API)) {
        return mockNetsysClient_.StopDhcpService(iface);
    }
    return netsysClient_.StopDhcpService(iface);
}

int32_t NetsysControllerServiceImpl::BandwidthEnableDataSaver(bool enable)
{
    NETMGR_LOG_D("BandwidthEnableDataSaver: enable=%{public}d", enable);
    return netsysClient_.BandwidthEnableDataSaver(enable);
}

int32_t NetsysControllerServiceImpl::BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes)
{
    NETMGR_LOG_D("BandwidthSetIfaceQuota: ifName=%{public}s", ifName.c_str());
    return netsysClient_.BandwidthSetIfaceQuota(ifName, bytes);
}

int32_t NetsysControllerServiceImpl::BandwidthRemoveIfaceQuota(const std::string &ifName)
{
    NETMGR_LOG_D("BandwidthRemoveIfaceQuota: ifName=%{public}s", ifName.c_str());
    return netsysClient_.BandwidthRemoveIfaceQuota(ifName);
}

int32_t NetsysControllerServiceImpl::BandwidthAddDeniedList(uint32_t uid)
{
    NETMGR_LOG_D("BandwidthAddDeniedList: uid=%{public}d", uid);
    return netsysClient_.BandwidthAddDeniedList(uid);
}

int32_t NetsysControllerServiceImpl::BandwidthRemoveDeniedList(uint32_t uid)
{
    NETMGR_LOG_D("BandwidthRemoveDeniedList: uid=%{public}d", uid);
    return netsysClient_.BandwidthRemoveDeniedList(uid);
}

int32_t NetsysControllerServiceImpl::BandwidthAddAllowedList(uint32_t uid)
{
    NETMGR_LOG_D("BandwidthAddAllowedList: uid=%{public}d", uid);
    return netsysClient_.BandwidthAddAllowedList(uid);
}

int32_t NetsysControllerServiceImpl::BandwidthRemoveAllowedList(uint32_t uid)
{
    NETMGR_LOG_D("BandwidthRemoveAllowedList: uid=%{public}d", uid);
    return netsysClient_.BandwidthRemoveAllowedList(uid);
}

int32_t NetsysControllerServiceImpl::FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    NETMGR_LOG_D("FirewallSetUidsAllowedListChain: chain=%{public}d", chain);
    return netsysClient_.FirewallSetUidsAllowedListChain(chain, uids);
}

int32_t NetsysControllerServiceImpl::FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    NETMGR_LOG_D("FirewallSetUidsDeniedListChain: chain=%{public}d", chain);
    return netsysClient_.FirewallSetUidsDeniedListChain(chain, uids);
}

int32_t NetsysControllerServiceImpl::FirewallEnableChain(uint32_t chain, bool enable)
{
    NETMGR_LOG_D("FirewallEnableChain: chain=%{public}d, enable=%{public}d", chain, enable);
    return netsysClient_.FirewallEnableChain(chain, enable);
}

int32_t NetsysControllerServiceImpl::FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids,
                                                        uint32_t firewallRule)
{
    return netsysClient_.FirewallSetUidRule(chain, uids, firewallRule);
}

int32_t NetsysControllerServiceImpl::GetTotalStats(uint64_t &stats, uint32_t type)
{
    NETMGR_LOG_D("GetTotalStats: type=%{public}d", type);
    return netsysClient_.GetTotalStats(stats, type);
}

int32_t NetsysControllerServiceImpl::GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid)
{
    NETMGR_LOG_D("GetUidStats: type=%{public}d uid=%{public}d", type, uid);
    return netsysClient_.GetUidStats(stats, type, uid);
}

int32_t NetsysControllerServiceImpl::GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName)
{
    NETMGR_LOG_D("GetIfaceStats: type=%{public}d", type);
    return netsysClient_.GetIfaceStats(stats, type, interfaceName);
}

int32_t NetsysControllerServiceImpl::GetAllContainerStatsInfo(
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    NETMGR_LOG_D("GetAllContainerStatsInfo");
    return netsysClient_.GetAllContainerStatsInfo(stats);
}

int32_t NetsysControllerServiceImpl::GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    NETMGR_LOG_D("GetAllStatsInfo");
    return netsysClient_.GetAllStatsInfo(stats);
}

int32_t NetsysControllerServiceImpl::SetIptablesCommandForRes(const std::string &cmd, std::string &respond)
{
    return netsysClient_.SetIptablesCommandForRes(cmd, respond);
}

int32_t NetsysControllerServiceImpl::NetDiagPingHost(const OHOS::NetsysNative::NetDiagPingOption &pingOption,
                                                     const sptr<OHOS::NetsysNative::INetDiagCallback> &callback)
{
    NETMGR_LOG_D("NetDiagPingHost");
    return netsysClient_.NetDiagPingHost(pingOption, callback);
}

int32_t NetsysControllerServiceImpl::NetDiagGetRouteTable(std::list<OHOS::NetsysNative::NetDiagRouteTable> &routeTables)
{
    NETMGR_LOG_D("NetDiagGetRouteTable");
    return netsysClient_.NetDiagGetRouteTable(routeTables);
}

int32_t NetsysControllerServiceImpl::NetDiagGetSocketsInfo(OHOS::NetsysNative::NetDiagProtocolType socketType,
                                                           OHOS::NetsysNative::NetDiagSocketsInfo &socketsInfo)
{
    NETMGR_LOG_D("NetDiagGetSocketsInfo");
    return netsysClient_.NetDiagGetSocketsInfo(socketType, socketsInfo);
}

int32_t NetsysControllerServiceImpl::NetDiagGetInterfaceConfig(
    std::list<OHOS::NetsysNative::NetDiagIfaceConfig> &configs, const std::string &ifaceName)
{
    NETMGR_LOG_D("NetDiagGetInterfaceConfig");
    return netsysClient_.NetDiagGetInterfaceConfig(configs, ifaceName);
}

int32_t NetsysControllerServiceImpl::NetDiagUpdateInterfaceConfig(const OHOS::NetsysNative::NetDiagIfaceConfig &config,
                                                                  const std::string &ifaceName, bool add)
{
    NETMGR_LOG_D("NetDiagUpdateInterfaceConfig");
    return netsysClient_.NetDiagUpdateInterfaceConfig(config, ifaceName, add);
}

int32_t NetsysControllerServiceImpl::NetDiagSetInterfaceActiveState(const std::string &ifaceName, bool up)
{
    NETMGR_LOG_D("NetDiagSetInterfaceActiveState");
    return netsysClient_.NetDiagSetInterfaceActiveState(ifaceName, up);
}

int32_t NetsysControllerServiceImpl::AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                                  const std::string &ifName)
{
    NETMGR_LOG_D("AddStaticArp");
    return netsysClient_.AddStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetsysControllerServiceImpl::DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                                  const std::string &ifName)
{
    NETMGR_LOG_D("DelStaticArp");
    return netsysClient_.DelStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetsysControllerServiceImpl::RegisterDnsResultCallback(
    const sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> &callback, uint32_t timeStep)
{
    NETMGR_LOG_D("RegisterDnsResultListener");
    return netsysClient_.RegisterDnsResultCallback(callback, timeStep);
}

int32_t NetsysControllerServiceImpl::UnregisterDnsResultCallback(
    const sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> &callback)
{
    NETMGR_LOG_D("UnregisterDnsResultListener");
    return netsysClient_.UnregisterDnsResultCallback(callback);
}

int32_t NetsysControllerServiceImpl::RegisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback)
{
    NETMGR_LOG_D("RegisterDnsResultListener");
    return netsysClient_.RegisterDnsHealthCallback(callback);
}

int32_t NetsysControllerServiceImpl::UnregisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback)
{
    NETMGR_LOG_D("UnregisterDnsResultListener");
    return netsysClient_.UnregisterDnsHealthCallback(callback);
}

int32_t NetsysControllerServiceImpl::GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie)
{
    NETMGR_LOG_D("GetCookieStats: type=%{public}u", type);
    return netsysClient_.GetCookieStats(stats, type, cookie);
}

int32_t NetsysControllerServiceImpl::GetNetworkSharingType(std::set<uint32_t>& sharingTypeIsOn)
{
    NETMGR_LOG_D("GetNetworkSharingType");
    return netsysClient_.GetNetworkSharingType(sharingTypeIsOn);
}

int32_t NetsysControllerServiceImpl::UpdateNetworkSharingType(uint32_t type, bool isOpen)
{
    NETMGR_LOG_D("UpdateNetworkSharingType: type=%{public}d isOpen=%{public}d",
                 type, isOpen);
    return netsysClient_.UpdateNetworkSharingType(type, isOpen);
}

int32_t NetsysControllerServiceImpl::SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on)
{
    NETMGR_LOG_I("SetIpv6PrivacyExtensions: interfaceName=%{public}s on=%{public}d", interfaceName.c_str(), on);
    return netsysClient_.SetIpv6PrivacyExtensions(interfaceName, on);
}

int32_t NetsysControllerServiceImpl::SetEnableIpv6(const std::string &interfaceName, const uint32_t on)
{
    NETMGR_LOG_I("SetEnableIpv6: interfaceName=%{public}s on=%{public}d", interfaceName.c_str(), on);
    return netsysClient_.SetEnableIpv6(interfaceName, on);
}

int32_t NetsysControllerServiceImpl::SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy,
                                                            bool reconfirmFlag)
{
    return netsysClient_.SetNetworkAccessPolicy(uid, policy, reconfirmFlag);
}

int32_t NetsysControllerServiceImpl::DeleteNetworkAccessPolicy(uint32_t uid)
{
    return netsysClient_.DeleteNetworkAccessPolicy(uid);
}

int32_t NetsysControllerServiceImpl::NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes)
{
    return netsysClient_.NotifyNetBearerTypeChange(bearerTypes);
}
} // namespace NetManagerStandard
} // namespace OHOS
