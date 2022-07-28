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
#include "netsys_controller.h"

#include "netmanager_base_common_utils.h"
#include "netsys_controller_service_impl.h"
#include "net_conn_types.h"
#include "net_mgr_log_wrapper.h"

using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace NetManagerStandard {
NetsysController::NetsysController()
{}

NetsysController::~NetsysController() {}

void NetsysController::Init()
{
    NETMGR_LOG_I("netsys Init");
    if (initFlag_) {
        NETMGR_LOG_I("netsys initialization is complete");
        return;
    }
    initFlag_ = true;
    netsysService_ = std::make_unique<NetsysControllerServiceImpl>().release();
    netsysService_->Init();
}

NetsysController &NetsysController::GetInstance()
{
    static NetsysController g_singleInstance_;
    static std::mutex g_mutex_;
    if (!g_singleInstance_.initFlag_) {
        std::unique_lock<std::mutex> lock(g_mutex_);
        if (!g_singleInstance_.initFlag_) {
            g_singleInstance_.Init();
        }
    }
    return g_singleInstance_;
}

int32_t NetsysController::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOG_I("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->NetworkCreatePhysical(netId, permission);
}

int32_t NetsysController::NetworkDestroy(int32_t netId)
{
    NETMGR_LOG_I("Destroy network: netId[%{public}d]", netId);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->NetworkDestroy(netId);
}

int32_t NetsysController::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->NetworkAddInterface(netId, iface);
}

int32_t NetsysController::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->NetworkRemoveInterface(netId, iface);
}

int32_t NetsysController::NetworkAddRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetsysController::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

int32_t NetsysController::InterfaceGetConfig(OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    NETMGR_LOG_I("get interface config");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->InterfaceGetConfig(cfg);
}

int32_t NetsysController::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOG_I("Set interface down: iface[%{public}s]", iface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->SetInterfaceDown(iface);
}

int32_t NetsysController::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOG_I("Set interface up: iface[%{public}s]", iface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->SetInterfaceUp(iface);
}

void NetsysController::InterfaceClearAddrs(const std::string &ifName)
{
    NETMGR_LOG_I("Clear addrs: ifName[%{public}s]", ifName.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return;
    }
    return netsysService_->InterfaceClearAddrs(ifName);
}

int32_t NetsysController::InterfaceGetMtu(const std::string &ifName)
{
    NETMGR_LOG_I("Get mtu: ifName[%{public}s]", ifName.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->InterfaceGetMtu(ifName);
}

int32_t NetsysController::InterfaceSetMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOG_I("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->InterfaceSetMtu(ifName, mtu);
}

int32_t NetsysController::InterfaceAddAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ToAnonymousIp(ipAddr).c_str(), prefixLength);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->InterfaceAddAddress(ifName, ipAddr, prefixLength);
}

int32_t NetsysController::InterfaceDelAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ToAnonymousIp(ipAddr).c_str(), prefixLength);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->InterfaceDelAddress(ifName, ipAddr, prefixLength);
}

int32_t NetsysController::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    NETMGR_LOG_I("Set resolver config: netId[%{public}d]", netId);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
}

int32_t NetsysController::GetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    NETMGR_LOG_I("Get resolver config: netId[%{public}d]", netId);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
}

int32_t NetsysController::CreateNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("create dns cache: netId[%{public}d]", netId);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->CreateNetworkCache(netId);
}

int32_t NetsysController::DestroyNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destroy dns cache: netId[%{public}d]", netId);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->DestroyNetworkCache(netId);
}

int32_t NetsysController::FlushNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destroy Flush dns cache: netId[%{public}d]", netId);
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->FlushNetworkCache(netId);
}

int32_t NetsysController::GetAddrInfo(const std::string &hostName, const std::string &serverName,
    const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId)
{
    NETMGR_LOG_I("NetsysController GetAddrInfo");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetAddrInfo(hostName, serverName, hints, res, netId);
}

int64_t NetsysController::GetCellularRxBytes()
{
    NETMGR_LOG_I("NetsysController GetCellularRxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetCellularRxBytes();
}

int64_t NetsysController::GetCellularTxBytes()
{
    NETMGR_LOG_I("NetsysController GetCellularTxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetCellularTxBytes();
}

int64_t NetsysController::GetAllRxBytes()
{
    NETMGR_LOG_I("NetsysController GetAllRxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetAllRxBytes();
}

int64_t NetsysController::GetAllTxBytes()
{
    NETMGR_LOG_I("NetsysController GetAllTxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetAllTxBytes();
}

int64_t NetsysController::GetUidRxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetsysController GetUidRxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetUidRxBytes(uid);
}

int64_t NetsysController::GetUidTxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetsysController GetUidTxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetUidTxBytes(uid);
}

int64_t NetsysController::GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("NetsysController GetUidOnIfaceRxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetUidOnIfaceRxBytes(uid, interfaceName);
}

int64_t NetsysController::GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_I("NetsysController GetUidOnIfaceTxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetUidOnIfaceTxBytes(uid, interfaceName);
}

int64_t NetsysController::GetIfaceRxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetsysController GetIfaceRxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetIfaceRxBytes(interfaceName);
}

int64_t NetsysController::GetIfaceTxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetsysController GetIfaceTxBytes");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetIfaceTxBytes(interfaceName);
}

std::vector<std::string> NetsysController::InterfaceGetList()
{
    NETMGR_LOG_I("NetsysController InterfaceGetList");
    std::vector<std::string> ret;
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ret;
    }
    return netsysService_->InterfaceGetList();
}

std::vector<std::string> NetsysController::UidGetList()
{
    NETMGR_LOG_I("NetsysController UidGetList");
    std::vector<std::string> ret;
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ret;
    }
    return netsysService_->UidGetList();
}

int64_t NetsysController::GetIfaceRxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysController GetIfaceRxPackets");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetIfaceRxPackets(interfaceName);
}

int64_t NetsysController::GetIfaceTxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysController GetIfaceTxPackets");
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->GetIfaceTxPackets(interfaceName);
}

int32_t NetsysController::SetDefaultNetWork(int32_t netId)
{
    NETMGR_LOG_D("Set DefaultNetWork: netId[%{public}d]", netId);
    return netsysService_->SetDefaultNetWork(netId);
}

int32_t NetsysController::ClearDefaultNetWorkNetId()
{
    NETMGR_LOG_D("ClearDefaultNetWorkNetId");
    return netsysService_->ClearDefaultNetWorkNetId();
}

int32_t NetsysController::BindSocket(int32_t socket_fd, uint32_t netId)
{
    NETMGR_LOG_D("NetsysController::BindSocket: netId = [%{public}u]", netId);
    return netsysService_->BindSocket(socket_fd, netId);
}

int32_t NetsysController::IpEnableForwarding(const std::string& requestor)
{
    NETMGR_LOG_D("IpEnableForwarding: requestor[%{public}s]", requestor.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->IpEnableForwarding(requestor);
}

int32_t NetsysController::IpDisableForwarding(const std::string& requestor)
{
    NETMGR_LOG_D("IpDisableForwarding: requestor[%{public}s]", requestor.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->IpDisableForwarding(requestor);
}

int32_t NetsysController::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETMGR_LOG_D("MockNetsysNativeClient EnableNat: intIface[%{public}s] intIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->EnableNat(downstreamIface, upstreamIface);
}

int32_t NetsysController::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETMGR_LOG_D("MockNetsysNativeClient DisableNat: intIface[%{public}s] intIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->DisableNat(downstreamIface, upstreamIface);
}

int32_t NetsysController::IpfwdAddInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("IpfwdAddInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetsysController::IpfwdRemoveInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("IpfwdRemoveInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->IpfwdRemoveInterfaceForward(fromIface, toIface);
}

int32_t NetsysController::TetherDnsSet(uint32_t netId, const std::vector<std::string>& dnsAddrs)
{
    NETMGR_LOG_D("TetherDnsSet: netId[%{public}d]", netId);
    for (auto iter = dnsAddrs.begin(); iter != dnsAddrs.end(); ++iter) {
        NETMGR_LOG_D("TetherDnsSet: dnsAddrs[%{public}s]", iter->c_str());
    }
    if (netsysService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysService_->TetherDnsSet(netId, dnsAddrs);
}

int32_t NetsysController::RegisterNetsysNotifyCallback(const NetsysNotifyCallback &callback)
{
    return netsysService_->RegisterNetsysNotifyCallback(callback);
}

int32_t NetsysController::BindNetworkServiceVpn(int32_t socketFd)
{
    NETMGR_LOG_D("NetsysController::BindNetworkServiceVpn: socketFd[%{public}d]", socketFd);
    if (socketFd <= 0) {
        NETMGR_LOG_E("socketFd is null");
        return ERR_VPN;
    }
    return netsysService_->BindNetworkServiceVpn(socketFd);
}

int32_t NetsysController::EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd)
{
    NETMGR_LOG_D("NetsysController::EnableVirtualNetIfaceCard: socketFd[%{public}d]", socketFd);
    if (socketFd <= 0) {
        NETMGR_LOG_E("socketFd is null");
        return ERR_VPN;
    }
    return netsysService_->EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
}

int32_t NetsysController::SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
    struct ifreq &ifRequest)
{
    NETMGR_LOG_D("NetsysController::set addr");
    if ((socketFd <= 0) || (ipAddress.length() == 0) || (ipAddress.length() > MAX_IPV4_ADDRESS_LEN) ||
        (prefixLen <= 0) || (prefixLen > MAX_IPV4_ADDRESS_LEN)) {
        NETMGR_LOG_E("The paramemters of SetIpAddress is failed, socketFd[%{public}d], "
            "ipAddress[%{public}s], prefixLen[%{public}d].",
            socketFd, ToAnonymousIp(ipAddress).c_str(), prefixLen);
        return ERR_VPN;
    }
    return netsysService_->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
}

int32_t NetsysController::SetBlocking(int32_t ifaceFd, bool isBlock)
{
    NETMGR_LOG_D("NetsysController::SetBlocking: ifaceFd[%{public}d], isBlock[%{public}d]", ifaceFd, isBlock);
    return netsysService_->SetBlocking(ifaceFd, isBlock);
}

int32_t NetsysController::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetsysController::StartDhcpClient: iface[%{public}s], bIpv6[%{public}d]", iface.c_str(), bIpv6);
    return netsysService_->StartDhcpClient(iface, bIpv6);
}

int32_t NetsysController::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetsysController::SetBlocking: iface[%{public}s], bIpv6[%{public}d]", iface.c_str(), bIpv6);
    return netsysService_->StopDhcpClient(iface, bIpv6);
}

int32_t NetsysController::RegisterCallback(sptr<NetsysControllerCallback> callback)
{
    NETMGR_LOG_D("NetsysController::RegisterCallback");
    return netsysService_->RegisterCallback(callback);
}

int32_t NetsysController::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETMGR_LOG_D("NetsysController::StartDhcpService: iface[%{public}s], ipv4addr[%{public}s]",
        iface.c_str(), ToAnonymousIp(ipv4addr).c_str());
    return netsysService_->StartDhcpService(iface, ipv4addr);
}

int32_t NetsysController::StopDhcpService(const std::string &iface)
{
    NETMGR_LOG_D("NetsysController::StopDhcpService: ifaceFd[%{public}s]", iface.c_str());
    return netsysService_->StopDhcpService(iface);
}
} // namespace NetManagerStandard
} // namespace OHOS
