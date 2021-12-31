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
#include "netd_native_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "securec.h"
#include "netd_native_service_proxy.h"

#include "net_conn_types.h"
#include "net_mgr_log_wrapper.h"
#include <cstdlib>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <net/route.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

namespace OHOS {
namespace NetManagerStandard {
NetdNativeClient::NetdNativeClient()
{
    Init();
}

NetdNativeClient::~NetdNativeClient() {}

void NetdNativeClient::Init()
{
    NETMGR_LOG_I("netd Init");
    if (initFlag_) {
        NETMGR_LOG_I("netd initialization is complete");
        return;
    }
    initFlag_ = true;
    netdNativeService_ = GetProxy();
}

int32_t NetdNativeClient::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOG_I("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->NetworkCreatePhysical(netId, permission);
}

int32_t NetdNativeClient::NetworkDestroy(int32_t netId)
{
    NETMGR_LOG_I("Destroy network: netId[%{public}d]", netId);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->NetworkDestroy(netId);
}

int32_t NetdNativeClient::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->NetworkAddInterface(netId, iface);
}

int32_t NetdNativeClient::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->NetworkRemoveInterface(netId, iface);
}

int32_t NetdNativeClient::NetworkAddRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetdNativeClient::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

void NetdNativeClient::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOG_I("Set interface down: iface[%{public}s]", iface.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return;
    }
}

void NetdNativeClient::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOG_I("Set interface up: iface[%{public}s]", iface.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return;
    }
}

void NetdNativeClient::InterfaceClearAddrs(const std::string &ifName)
{
    NETMGR_LOG_I("Clear addrs: ifName[%{public}s]", ifName.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return;
    }
    return;
}

int32_t NetdNativeClient::InterfaceGetMtu(const std::string &ifName)
{
    NETMGR_LOG_I("Get mtu: ifName[%{public}s]", ifName.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->InterfaceGetMtu(ifName);
}

int32_t NetdNativeClient::InterfaceSetMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOG_I("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->InterfaceSetMtu(ifName, mtu);
}

int32_t NetdNativeClient::InterfaceAddAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->InterfaceAddAddress(ifName, ipAddr, prefixLength);
}

int32_t NetdNativeClient::InterfaceDelAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOG_I("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->InterfaceDelAddress(ifName, ipAddr, prefixLength);
}

int32_t NetdNativeClient::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    NETMGR_LOG_I("Set resolver config: netId[%{public}d]", netId);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    const nmd::dnsresolver_params params = {netId, baseTimeoutMsec, retryCount, servers, domains};
    return netdNativeService_->SetResolverConfig(params);
}

int32_t NetdNativeClient::GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    NETMGR_LOG_I("Get resolver config: netId[%{public}d]", netId);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int32_t NetdNativeClient::CreateNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("create dns cache: netId[%{public}d]", netId);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->CreateNetworkCache(netId);
}

int32_t NetdNativeClient::DestoryNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory dns cache: netId[%{public}d]", netId);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->DestoryNetworkCache(netId);
}

int32_t NetdNativeClient::FlushNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory Flush dns cache: netId[%{public}d]", netId);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdNativeService_->FlushNetworkCache(netId);
}

int32_t NetdNativeClient::GetAddrInfo(const std::string &hostName,
    const std::string &serverName, const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId)
{
    NETMGR_LOG_I("NetdNativeClient GetAddrInfo");
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("GetAddrInfo netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    struct addrinfo *addrRes = nullptr;
    int ret = netdNativeService_->Getaddrinfo(hostName.c_str(), nullptr, &hints, &addrRes, netId);
    res.reset(addrRes);
    return ret;
}

int64_t NetdNativeClient::GetCellularRxBytes()
{
    NETMGR_LOG_I("NetdNativeClient GetCellularRxBytes");
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdNativeClient::GetCellularTxBytes()
{
    NETMGR_LOG_I("NetdNativeClient GetCellularTxBytes");
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdNativeClient::GetAllRxBytes()
{
    NETMGR_LOG_I("NetdNativeClient GetAllRxBytes");
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdNativeClient::GetAllTxBytes()
{
    NETMGR_LOG_I("NetdNativeClient GetAllTxBytes");
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdNativeClient::GetUidRxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetdNativeClient GetUidRxBytes uid is [%{public}u]", uid);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdNativeClient::GetUidTxBytes(uint32_t uid)
{
    NETMGR_LOG_I("NetdNativeClient GetUidTxBytes uid is [%{public}u]", uid);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdNativeClient::GetIfaceRxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdNativeClient GetIfaceRxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int64_t NetdNativeClient::GetIfaceTxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("NetdNativeClient GetIfaceTxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

std::vector<std::string> NetdNativeClient::InterfaceGetList()
{
    NETMGR_LOG_I("NetdNativeClient InterfaceGetList");
    std::vector<std::string> ret;
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ret;
    }
    return {};
}

int64_t NetdNativeClient::GetIfaceRxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetdNativeClient GetIfaceRxPackets iface name is [%{public}s]", interfaceName.c_str());
    return 0;
}

int64_t NetdNativeClient::GetIfaceTxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetdNativeClient GetIfaceTxPackets iface name is [%{public}s]", interfaceName.c_str());
    return 0;
}

int32_t NetdNativeClient::SetDefaultNetWork(int32_t netId)
{
    NETMGR_LOG_D("NetdNativeClient SetDefaultNetWork");
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int32_t NetdNativeClient::ClearDefaultNetWorkNetId()
{
    NETMGR_LOG_D("NetdNativeClient ClearDefaultNetWorkNetId");
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int32_t NetdNativeClient::BindSocket(int32_t socket_fd, uint32_t netId)
{
    NETMGR_LOG_D("NetdNativeClient::BindSocket: netId = [%{public}u]", netId);
    if (netdNativeService_ == nullptr) {
        NETMGR_LOG_E("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return 0;
}

int32_t NetdNativeClient::IpEnableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("NetdNativeClient IpEnableForwarding: requester[%{public}s]", requester.c_str());
    return 0;
}

int32_t NetdNativeClient::IpDisableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("NetdNativeClient IpDisableForwarding: requester[%{public}s]", requester.c_str());
    return 0;
}

int32_t NetdNativeClient::TetherAddForward(const std::string& downstreamIface, const std::string& upstreamIface)
{
    NETMGR_LOG_D("NetdNativeClient TetherAddForward: downstreamIface[%{public}s], upstreamIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    return 0;
}

int32_t NetdNativeClient::TetherRemoveForward(const std::string& downstreamIface, const std::string& upstreamIface)
{
    NETMGR_LOG_D("NetdNativeClient TetherRemoveForward: downstreamIface[%{public}s], upstreamIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    return 0;
}

int32_t NetdNativeClient::IpfwdAddInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("NetdNativeClient IpfwdAddInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    return 0;
}

int32_t NetdNativeClient::IpfwdRemoveInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("NetdNativeClient IpfwdRemoveInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    return 0;
}

int32_t NetdNativeClient::TetherDnsSet(uint32_t netId, const std::vector<std::string>& dnsAddrs)
{
    NETMGR_LOG_D("NetdNativeClient TetherDnsSet: netId[%{public}d]", netId);
    for (auto iter = dnsAddrs.begin(); iter != dnsAddrs.end(); ++iter) {
        NETMGR_LOG_D("NetdNativeClient TetherDnsSet: dnsAddrs[%{public}s]", iter->c_str());
    }
    return 0;
}

int32_t NetdNativeClient::RegisterNetdNotifyCallback(const NetdNotifyCallback &callback)
{
    NETMGR_LOG_D("NetdNativeClient RegisterNetdNotifyCallback");
    return 0;
}

sptr<OHOS::NetdNative::INetdService> NetdNativeClient::GetProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remote = samgr->GetSystemAbility(OHOS::COMM_NETD_NATIVE_SYS_ABILITY_ID);
    auto proxy = iface_cast<NetdNative::INetdService>(remote);
    return proxy;
}

int32_t NetdNativeClient::BindNetworkServiceVpn(int32_t socketFd)
{
    NETMGR_LOG_D("NetdNativeClient::BindNetworkServiceVpn: socketFd[%{public}d]", socketFd);
    /* netd provide default interface name */
    const char* defaultNetName = "wlan0";
    int defaultNetNameLen = strlen(defaultNetName);
    /* set socket by option. */
    int32_t ret = setsockopt(socketFd, SOL_SOCKET, SO_MARK, defaultNetName, defaultNetNameLen);
    if (ret < 0) {
        NETMGR_LOG_E("The SO_BINDTODEVICE of setsockopt failed.");
        return ERR_VPN;
    }
    return 0;
}

int32_t NetdNativeClient::EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd)
{
    NETMGR_LOG_D("NetdNativeClient::EnableVirtualNetIfaceCard: socketFd[%{public}d]", socketFd);
    int32_t ifaceFdTemp = 0;
    if ((ifaceFdTemp = open("/dev/net/tun", O_RDWR)) < 0) {
        NETMGR_LOG_E("VPN tunnel device open was failed.");
        return ERR_VPN;
    }

    /*
    * Flags:
    * IFF_TUN   - TUN device (no Ethernet headers)
    * IFF_TAP   - TAP device
    * IFF_NO_PI - Do not provide packet information
    **/
    ifRequest.ifr_flags = IFF_TUN | IFF_NO_PI;
    /**
    * Try to create the device. if it cannot assign the device interface name, kernel can
    * allocate the next device interface name. for example, there is tun0, kernel can
    * allocate tun1.
    **/
    if (ioctl(ifaceFdTemp, TUNSETIFF, &ifRequest) < 0) {
        NETMGR_LOG_E("The TUNSETIFF of ioctl failed, ifRequest.ifr_name[%{public}s]",
            ifRequest.ifr_name);
        return ERR_VPN;
    }

    /* Activate the device */
    ifRequest.ifr_flags = IFF_UP;
    if (ioctl(socketFd, SIOCSIFFLAGS, &ifRequest) < 0) {
        NETMGR_LOG_E("The SIOCSIFFLAGS of ioctl failed.");
        return ERR_VPN;
    }

    ifaceFd = ifaceFdTemp;
    return 0;
}

static inline in_addr_t *as_in_addr(sockaddr *sa)
{
    return &((sockaddr_in *)sa)->sin_addr.s_addr;
}

int32_t NetdNativeClient::SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
    struct ifreq &ifRequest)
{
    NETMGR_LOG_D("NetdNativeClient::SetIpAddress: socketFd[%{public}d]", socketFd);

    ifRequest.ifr_addr.sa_family = AF_INET;
    ifRequest.ifr_netmask.sa_family = AF_INET;

    /* inet_pton is IP ipAddress translation to binary network byte order. */
    if (inet_pton(AF_INET, ipAddress.c_str(), as_in_addr(&ifRequest.ifr_addr)) != 1) {
        NETMGR_LOG_E("inet_pton failed.");
        return ERR_VPN;
    }
    if (ioctl(socketFd, SIOCSIFADDR, &ifRequest) < 0) {
        NETMGR_LOG_E("The SIOCSIFADDR of ioctl failed.");
        return ERR_VPN;
    }
    in_addr_t addressPrefixLength = prefixLen ? (~0 << (MAX_IPV4_ADDRESS_LEN - prefixLen)) : 0;
    *as_in_addr(&ifRequest.ifr_netmask) = htonl(addressPrefixLength);
    if (ioctl(socketFd, SIOCSIFNETMASK, &ifRequest)) {
        NETMGR_LOG_E("The SIOCSIFNETMASK of ioctl failed.");
        return ERR_VPN;
    }

    return 0;
}

int32_t NetdNativeClient::SetBlocking(int32_t ifaceFd, bool isBlock)
{
    NETMGR_LOG_D("NetdNativeClient::SetBlocking");
    int32_t blockingFlag = 0;
    blockingFlag = fcntl(ifaceFd, F_GETFL);
    if (blockingFlag < 0) {
        NETMGR_LOG_E("The blockingFlag of fcntl failed.");
        return ERR_VPN;
    }

    if (!isBlock) {
        blockingFlag = blockingFlag | O_NONBLOCK;
    } else {
        blockingFlag = blockingFlag & (~O_NONBLOCK);
    }

    if (fcntl(ifaceFd, F_SETFL, blockingFlag) < 0) {
        NETMGR_LOG_E("The F_SETFL of fcntl failed.");
        return ERR_VPN;
    }
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS