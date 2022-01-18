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
#include "netd_controller.h"

#include "netd_controller_service_impl.h"
#include "net_conn_types.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetdController::NetdController()
{}

NetdController::~NetdController() {}

void NetdController::Init()
{
    NETMGR_LOG_I("netd Init");
    if (initFlag_) {
        NETMGR_LOG_I("netd initialization is complete");
        return;
    }
    initFlag_ = true;
    netdService_ = std::make_unique<NetdControllerServiceImpl>().release();
    netdService_->Init();
}

NetdController &NetdController::GetInstance()
{
    static NetdController g_singleInstance_;
    static std::mutex g_mutex_;
    if (!g_singleInstance_.initFlag_) {
        std::unique_lock<std::mutex> lock(g_mutex_);
        if (!g_singleInstance_.initFlag_) {
            g_singleInstance_.Init();
        }
    }
    return g_singleInstance_;
}

int32_t NetdController::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOG_I("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->NetworkCreatePhysical(netId, permission);
}

int32_t NetdController::NetworkDestroy(int32_t netId)
{
    NETMGR_LOG_I("Destroy network: netId[%{public}d]", netId);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->NetworkDestroy(netId);
}

int32_t NetdController::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->NetworkAddInterface(netId, iface);
}

int32_t NetdController::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->NetworkRemoveInterface(netId, iface);
}

int32_t NetdController::NetworkAddRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetdController::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

void NetdController::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOG_I("Set interface down: iface[%{public}s]", iface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return;
    }
    return netdService_->SetInterfaceDown(iface);
}

void NetdController::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOG_I("Set interface up: iface[%{public}s]", iface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return;
    }
    return netdService_->SetInterfaceUp(iface);
}

void NetdController::InterfaceClearAddrs(const std::string &ifName)
{
    NETMGR_LOG_I("Clear addrs: ifName[%{public}s]", ifName.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return;
    }
    return netdService_->InterfaceClearAddrs(ifName);
}

int32_t NetdController::InterfaceGetMtu(const std::string &ifName)
{
    NETMGR_LOG_I("Get mtu: ifName[%{public}s]", ifName.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->InterfaceGetMtu(ifName);
}

int32_t NetdController::InterfaceSetMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOG_I("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->InterfaceSetMtu(ifName, mtu);
}

int32_t NetdController::InterfaceAddAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->InterfaceAddAddress(ifName, ipAddr, prefixLength);
}

int32_t NetdController::InterfaceDelAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->InterfaceDelAddress(ifName, ipAddr, prefixLength);
}

int32_t NetdController::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    NETMGR_LOG_I("Set resolver config: netId[%{public}d]", netId);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
}

int32_t NetdController::GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    NETMGR_LOG_I("Get resolver config: netId[%{public}d]", netId);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetResolverInfo(netId, servers, domains, baseTimeoutMsec, retryCount);
}

int32_t NetdController::CreateNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("create dns cache: netId[%{public}d]", netId);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->CreateNetworkCache(netId);
}

int32_t NetdController::DestoryNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory dns cache: netId[%{public}d]", netId);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->DestoryNetworkCache(netId);
}

int32_t NetdController::FlushNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory Flush dns cache: netId[%{public}d]", netId);
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->FlushNetworkCache(netId);
}

int32_t NetdController::GetAddrInfo(const std::string &hostName, const std::string &serverName,
    const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId)
{
    NETMGR_LOG_I("NetdController GetAddrInfo");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetAddrInfo(hostName, serverName, hints, res, netId);
}

int64_t NetdController::GetCellularRxBytes()
{
    NETMGR_LOG_I("NetdController GetCellularRxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetCellularRxBytes();
}

int64_t NetdController::GetCellularTxBytes()
{
    NETMGR_LOG_I("NetdController GetCellularTxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetCellularTxBytes();
}

int64_t NetdController::GetAllRxBytes()
{
    NETMGR_LOG_I("NetdController GetAllRxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetAllRxBytes();
}

int64_t NetdController::GetAllTxBytes()
{
    NETMGR_LOG_I("NetdController GetAllTxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetAllTxBytes();
}

int64_t NetdController::GetUidRxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetdController GetUidRxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetUidRxBytes(uid);
}

int64_t NetdController::GetUidTxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetdController GetUidTxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetUidTxBytes(uid);
}

int64_t NetdController::GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdController GetUidOnIfaceRxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetUidOnIfaceRxBytes(uid, interfaceName);
}

int64_t NetdController::GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdController GetUidOnIfaceTxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetUidOnIfaceTxBytes(uid, interfaceName);
}

int64_t NetdController::GetIfaceRxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdController GetIfaceRxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetIfaceRxBytes(interfaceName);
}

int64_t NetdController::GetIfaceTxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdController GetIfaceTxBytes");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->GetIfaceTxBytes(interfaceName);
}

std::vector<std::string> NetdController::InterfaceGetList()
{
    NETMGR_LOG_I("NetdController InterfaceGetList");
    std::vector<std::string> ret;
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ret;
    }
    return netdService_->InterfaceGetList();
}

std::vector<std::string> NetdController::UidGetList()
{
    NETMGR_LOG_I("NetdController UidGetList");
    std::vector<std::string> ret;
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ret;
    }
    return netdService_->UidGetList();
}

int64_t NetdController::GetIfaceRxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetdController GetIfaceRxPackets");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdController::GetIfaceTxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetdController GetIfaceTxPackets");
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int32_t NetdController::SetDefaultNetWork(int32_t netId)
{
    NETMGR_LOG_D("Set DefaultNetWork: netId[%{public}d]", netId);
    return netdService_->SetDefaultNetWork(netId);
}

int32_t NetdController::ClearDefaultNetWorkNetId()
{
    NETMGR_LOG_D("ClearDefaultNetWorkNetId");
    return netdService_->ClearDefaultNetWorkNetId();
}

int32_t NetdController::BindSocket(int32_t socket_fd, uint32_t netId)
{
    NETMGR_LOG_D("NetdController::BindSocket: netId = [%{public}u]", netId);
    return netdService_->BindSocket(socket_fd, netId);
}

int32_t NetdController::IpEnableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("IpEnableForwarding: requester[%{public}s]", requester.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->IpEnableForwarding(requester);
}

int32_t NetdController::IpDisableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("IpDisableForwarding: requester[%{public}s]", requester.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->IpDisableForwarding(requester);
}

int32_t NetdController::TetherAddForward(const std::string& downstreamIface, const std::string& upstreamIface)
{
    NETMGR_LOG_D("TetherAddForward: downstreamIface[%{public}s], upstreamIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->TetherAddForward(downstreamIface, upstreamIface);
}

int32_t NetdController::TetherRemoveForward(const std::string& downstreamIface, const std::string& upstreamIface)
{
    NETMGR_LOG_D("TetherRemoveForward: downstreamIface[%{public}s], upstreamIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->TetherRemoveForward(downstreamIface, upstreamIface);
}

int32_t NetdController::IpfwdAddInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("IpfwdAddInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetdController::IpfwdRemoveInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("IpfwdRemoveInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->IpfwdRemoveInterfaceForward(fromIface, toIface);
}

int32_t NetdController::TetherDnsSet(uint32_t netId, const std::vector<std::string>& dnsAddrs)
{
    NETMGR_LOG_D("TetherDnsSet: netId[%{public}d]", netId);
    for (auto iter = dnsAddrs.begin(); iter != dnsAddrs.end(); ++iter) {
        NETMGR_LOG_D("TetherDnsSet: dnsAddrs[%{public}s]", iter->c_str());
    }
    if (netdService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->TetherDnsSet(netId, dnsAddrs);
}

int32_t NetdController::RegisterNetdNotifyCallback(const NetdNotifyCallback &callback)
{
    return netdService_->RegisterNetdNotifyCallback(callback);
}

int32_t NetdController::BindNetworkServiceVpn(int32_t socketFd)
{
    NETMGR_LOG_D("NetdController::BindNetworkServiceVpn: socketFd[%{public}d]", socketFd);
    if (socketFd <= 0) {
        NETMGR_LOG_E("socketFd is null");
        return ERR_VPN;
    }
    return netdService_->BindNetworkServiceVpn(socketFd);
}

int32_t NetdController::EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd)
{
    NETMGR_LOG_D("NetdController::EnableVirtualNetIfaceCard: socketFd[%{public}d]", socketFd);
    if (socketFd <= 0) {
        NETMGR_LOG_E("socketFd is null");
        return ERR_VPN;
    }
    return netdService_->EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
}

int32_t NetdController::SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
    struct ifreq &ifRequest)
{
    NETMGR_LOG_D("NetdController::SetIpAddress: socketFd[%{public}d]", socketFd);
    if ((socketFd <= 0) || (ipAddress.length() == 0) || (ipAddress.length() > MAX_IPV4_ADDRESS_LEN) ||
        (prefixLen <= 0) || (prefixLen > MAX_IPV4_ADDRESS_LEN)) {
        NETMGR_LOG_E("The paramemters of SetIpAddress is failed, socketFd[%{public}d], "
            "ipAddress[%{public}s], prefixLen[%{public}d].",
            socketFd, ipAddress.c_str(), prefixLen);
        return ERR_VPN;
    }
    return netdService_->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
}

int32_t NetdController::SetBlocking(int32_t ifaceFd, bool isBlock)
{
    NETMGR_LOG_D("NetdController::SetBlocking: ifaceFd[%{public}d], isBlock[%{public}d]", ifaceFd, isBlock);
    return netdService_->SetBlocking(ifaceFd, isBlock);
}

int32_t NetdController::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetdController::StartDhcpClient: iface[%{public}s], bIpv6[%{public}d]", iface.c_str(), bIpv6);
    return netdService_->StartDhcpClient(iface, bIpv6);
}

int32_t NetdController::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetdController::SetBlocking: iface[%{public}s], bIpv6[%{public}d]", iface.c_str(), bIpv6);
    return netdService_->StopDhcpClient(iface, bIpv6);
}

int32_t NetdController::RegisterCallback(sptr<NetdControllerCallback> callback)
{
    NETMGR_LOG_D("NetdController::RegisterCallback");
    return netdService_->RegisterCallback(callback);
}

int32_t NetdController::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETMGR_LOG_D("NetdController::StartDhcpService: iface[%{public}s], ipv4addr[%{public}s]",
        iface.c_str(), ipv4addr.c_str());
    return netdService_->StartDhcpService(iface, ipv4addr);
}

int32_t NetdController::StopDhcpService(const std::string &iface)
{
    NETMGR_LOG_D("NetdController::StopDhcpService: ifaceFd[%{public}s]", iface.c_str());
    return netdService_->StopDhcpService(iface);
}
} // namespace NetManagerStandard
} // namespace OHOS