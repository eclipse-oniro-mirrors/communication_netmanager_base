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

#include "mock_netd_native_client.h"

#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <netdb.h>

#include "securec.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string INTERFACE_LIST_DIR = "/sys/class/net/";
const std::string TEST_CELL_RX = "/data/cell_rx";
const std::string TEST_CELL_TX = "/data/cell_tx";
const std::string TEST_IFACE_RX = "/data/iface_rx";
const std::string TEST_IFACE_TX = "/data/iface_tx";
const std::string TEST_IFACE_RX_P = "/data/iface_rx_p";
const std::string TEST_IFACE_TX_P = "/data/iface_tx_p";
const std::string TEST_ALL_RX = "/data/all_tx";
const std::string TEST_ALL_TX = "/data/all_tx";
const std::string NET_STATS_FILE_RX_BYTES = "rx_bytes";
const std::string NET_STATS_FILE_TX_BYTES = "tx_bytes";
const std::string NET_STATS_FILE_RX_PACKETS = "rx_packets";
const std::string NET_STATS_FILE_TX_PACKETS = "tx_packets";
MockNetdNativeClient::MockNetdNativeClient()
{
    Init();
}

MockNetdNativeClient::~MockNetdNativeClient() {}

void MockNetdNativeClient::Init()
{
    return;
}

void MockNetdNativeClient::RegisterMockApi()
{
    mockApi_.insert(MOCK_NETWORKCREATEPHYSICAL_API);
    mockApi_.insert(MOCK_NETWORKDESTROY_API);
    mockApi_.insert(MOCK_NETWORKADDINTERFACE_API);
    mockApi_.insert(MOCK_NETWORKREMOVEINTERFACE_API);
    mockApi_.insert(MOCK_NETWORKADDROUTE_API);
    mockApi_.insert(MOCK_NETWORKREMOVEROUTE_API);
    mockApi_.insert(MOCK_SETINTERFACEDOWN_API);
    mockApi_.insert(MOCK_SETINTERFACEUP_API);
    mockApi_.insert(MOCK_INTERFACECLEARADDRS_API);
    mockApi_.insert(MOCK_INTERFACEGETMTU_API);
    mockApi_.insert(MOCK_INTERFACESETMTU_API);
    mockApi_.insert(MOCK_INTERFACEADDADDRESS_API);
    mockApi_.insert(MOCK_INTERFACEDELADDRESS_API);
    mockApi_.insert(MOCK_SETRESOLVERCONFIG_API);
    mockApi_.insert(MOCK_GETRESOLVERINFO_API);
    mockApi_.insert(MOCK_CREATENETWORKCACHE_API);
    mockApi_.insert(MOCK_DESTORYNETWORKCACHE_API);
    mockApi_.insert(MOCK_FLUSHNETWORKCACHE_API);
    mockApi_.insert(MOCK_GETCELLULARRXBYTES_API);
    mockApi_.insert(MOCK_GETCELLULARTXBYTES_API);
    mockApi_.insert(MOCK_GETALLRXBYTES_API);
    mockApi_.insert(MOCK_GETALLTXBYTES_API);
    mockApi_.insert(MOCK_GETUIDRXBYTES_API);
    mockApi_.insert(MOCK_GETUIDTXBYTES_API);
    mockApi_.insert(MOCK_GETIFACERXBYTES_API);
    mockApi_.insert(MOCK_GETIFACETXBYTES_API);
    mockApi_.insert(MOCK_INTERFACEGETLIST_API);
    mockApi_.insert(MOCK_GETIFACERXPACKETS_API);
    mockApi_.insert(MOCK_GETIFACETXPACKETS_API);
    mockApi_.insert(MOCK_SETDEFAULTNETWORK_API);
    mockApi_.insert(MOCK_CLEARDEFAULTNETWORK_API);
    mockApi_.insert(MOCK_BINDSOCKET_API);
    mockApi_.insert(MOCK_IPENABLEFORWARDING_API);
    mockApi_.insert(MOCK_IPDISABLEFORWARDING_API);
    mockApi_.insert(MOCK_TETHERADDFORWARD_API);
    mockApi_.insert(MOCK_TETHERREMOVEFORWARD_API);
    mockApi_.insert(MOCK_IPFWDADDIFACEFORWARD_API);
    mockApi_.insert(MOCK_IPFWDREMOVEIFACEFORWARD_API);
    mockApi_.insert(MOCK_TETHERDNSSET_API);
    mockApi_.insert(MOCK_REGISTERNETDNOTIFYCALLBACK_API);
    mockApi_.insert(MOCK_BINDNETWORKSERVICEVPN_API);
    mockApi_.insert(MOCK_ENABLEVIRTUALNETIFACECARD_API);
    mockApi_.insert(MOCK_SETIPADDRESS_API);
    mockApi_.insert(MOCK_SETBLOCKING_API);
}

bool MockNetdNativeClient::CheckMockApi(const std::string &api)
{
    return mockApi_.find(api) != mockApi_.end();
}

int32_t MockNetdNativeClient::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOG_I("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
    return 0;
}

int32_t MockNetdNativeClient::NetworkDestroy(int32_t netId)
{
    NETMGR_LOG_I("Destroy network: netId[%{public}d]", netId);
    return 0;
}

int32_t MockNetdNativeClient::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    return 0;
}

int32_t MockNetdNativeClient::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    return 0;
}

int32_t MockNetdNativeClient::NetworkAddRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    std::string mask = "0.0.0.0";
    return AddRoute(destination, mask, nextHop, ifName);
}

int32_t MockNetdNativeClient::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_I("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
    return 0;
}

void MockNetdNativeClient::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOG_I("Set interface down: iface[%{public}s]", iface.c_str());
    return;
}

void MockNetdNativeClient::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOG_I("Set interface up: iface[%{public}s]", iface.c_str());
    return;
}

void MockNetdNativeClient::InterfaceClearAddrs(const std::string &ifName)
{
    NETMGR_LOG_I("Clear addrs: ifName[%{public}s]", ifName.c_str());
    return;
}

int32_t MockNetdNativeClient::InterfaceGetMtu(const std::string &ifName)
{
    NETMGR_LOG_I("Get mtu: ifName[%{public}s]", ifName.c_str());
    return 0;
}

int32_t MockNetdNativeClient::InterfaceSetMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOG_I("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
    return 0;
}

int32_t MockNetdNativeClient::InterfaceAddAddress(const std::string &ifName, const std::string &ipAddr,
    int32_t prefixLength)
{
    NETMGR_LOG_I("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    return 0;
}

int32_t MockNetdNativeClient::InterfaceDelAddress(const std::string &ifName, const std::string &ipAddr,
    int32_t prefixLength)
{
    NETMGR_LOG_I("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
    return 0;
}

int32_t MockNetdNativeClient::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    NETMGR_LOG_I("Set resolver config: netId[%{public}d]", netId);
    return 0;
}

int32_t MockNetdNativeClient::GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    NETMGR_LOG_I("Get resolver config: netId[%{public}d]", netId);
    return 0;
}

int32_t MockNetdNativeClient::CreateNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("create dns cache: netId[%{public}d]", netId);
    return 0;
}

int32_t MockNetdNativeClient::DestoryNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory dns cache: netId[%{public}d]", netId);
    return 0;
}

int32_t MockNetdNativeClient::FlushNetworkCache(uint16_t netId)
{
    NETMGR_LOG_I("Destory Flush dns cache: netId[%{public}d]", netId);
    return 0;
}

int32_t MockNetdNativeClient::GetAddrInfo(const std::string &hostName,
    const std::string &serverName, const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId)
{
    NETMGR_LOG_I("MockNetdNativeClient GetAddrInfo");
    struct addrinfo *resAddr = nullptr;
    int32_t ret = getaddrinfo(hostName.c_str(), nullptr, &hints, &resAddr);
    res.reset(resAddr);
    return ret;
}

static long getInterfaceTrafficByType(const std::string &path, const std::string &type)
{
    if (path.empty()) {
        return -1;
    }
    std::string trafficPath = path + type;
    int fd = open(trafficPath.c_str(), 0, 0666);
    if (fd == -1) {
        return -1;
    }
    char buf[100] = {0};
    if (read(fd, buf, sizeof(buf)) == -1) {
        close(fd);
        return -1;
    }
    close(fd);
    long infBytes = atol(buf);
    return infBytes;
}

int64_t MockNetdNativeClient::GetCellularRxBytes()
{
    NETMGR_LOG_I("MockNetdNativeClient GetCellularRxBytes");
    if (access(TEST_CELL_RX.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetIfaceBytes("usb0", NET_STATS_FILE_RX_BYTES.c_str());
}

int64_t MockNetdNativeClient::GetCellularTxBytes()
{
    NETMGR_LOG_I("MockNetdNativeClient GetCellularTxBytes");
    if (access(TEST_CELL_TX.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetIfaceBytes("usb0", NET_STATS_FILE_TX_BYTES.c_str());
}

int64_t MockNetdNativeClient::GetAllBytes(const std::string &filename)
{
    NETMGR_LOG_I("MockNetdNativeClient GetAllBytes");
    long allBytes = 0;
    std::vector<std::string> ifNameList = InterfaceGetList();
    if (ifNameList.empty()) {
        return allBytes;
    }
    for (auto iter = ifNameList.begin(); iter != ifNameList.end(); iter++) {
        if (*iter != "lo") {
            std::string base_traffic_path = INTERFACE_LIST_DIR + (*iter) + "/" + "statistics" + "/";
            long bytes = getInterfaceTrafficByType(base_traffic_path, filename.c_str());
            allBytes = allBytes + bytes;
        }
    }
    return allBytes;
}

int64_t MockNetdNativeClient::GetAllRxBytes()
{
    NETMGR_LOG_I("MockNetdNativeClient GetAllRxBytes");
    if (access(TEST_ALL_RX.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetAllBytes(NET_STATS_FILE_RX_BYTES.c_str());
}

int64_t MockNetdNativeClient::GetAllTxBytes()
{
    NETMGR_LOG_I("MockNetdNativeClient GetAllTxBytes");
    if (access(TEST_ALL_TX.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetAllBytes(NET_STATS_FILE_TX_BYTES.c_str());
}

int64_t MockNetdNativeClient::GetUidRxBytes(uint32_t uid)
{
    NETMGR_LOG_I("MockNetdNativeClient GetUidRxBytes uid is [%{public}u]", uid);
    return 0;
}

int64_t MockNetdNativeClient::GetUidTxBytes(uint32_t uid)
{
    NETMGR_LOG_I("MockNetdNativeClient GetUidTxBytes uid is [%{public}u]", uid);
    return 0;
}

int64_t MockNetdNativeClient::GetIfaceBytes(const std::string &interfaceName, const std::string &filename)
{
    int64_t bytes = 0;
    std::vector<std::string> ifNameList = InterfaceGetList();
    if (ifNameList.empty()) {
        return bytes;
    }
    for (auto iter = ifNameList.begin(); iter != ifNameList.end(); iter++) {
        if (interfaceName == *iter) {
            std::string base_traffic_path = INTERFACE_LIST_DIR + (*iter) + "/" + "statistics" + "/";
            long infRxBytes = getInterfaceTrafficByType(base_traffic_path, filename.c_str());
            bytes = infRxBytes == -1 ? 0 : infRxBytes;
        }
    }
    return bytes;
}

int64_t MockNetdNativeClient::GetIfaceRxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("MockNetdNativeClient GetIfaceRxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (access(TEST_IFACE_RX.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetIfaceBytes(interfaceName, NET_STATS_FILE_RX_BYTES.c_str());
}

int64_t MockNetdNativeClient::GetIfaceTxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_I("MockNetdNativeClient GetIfaceTxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (access(TEST_IFACE_TX.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetIfaceBytes(interfaceName, NET_STATS_FILE_TX_BYTES.c_str());
}

int64_t MockNetdNativeClient::GetIfaceRxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_I("MockNetdNativeClient GetIfaceRxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (access(TEST_IFACE_RX_P.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetIfaceBytes(interfaceName, NET_STATS_FILE_RX_PACKETS.c_str());
}

int64_t MockNetdNativeClient::GetIfaceTxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_I("MockNetdNativeClient GetIfaceTxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (access(TEST_IFACE_TX_P.c_str(), F_OK) == 0) {
        return 0;
    }
    return GetIfaceBytes(interfaceName, NET_STATS_FILE_TX_PACKETS.c_str());
}

std::vector<std::string> MockNetdNativeClient::InterfaceGetList()
{
    NETMGR_LOG_I("MockNetdNativeClient InterfaceGetList");
    DIR *dir(nullptr);
    std::vector<std::string> ifList;
    if ((dir = opendir(INTERFACE_LIST_DIR.c_str())) == NULL) {
        return ifList;
    }
    struct dirent *ptr(nullptr);
    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        ifList.push_back(ptr->d_name);
    }
    closedir(dir);
    return ifList;
}

int32_t MockNetdNativeClient::AddRoute(const std::string &ip, const std::string &mask,
    const std::string &gateWay, const std::string &devName)
{
    struct sockaddr_in _sin;
    struct rtentry  rt;
    bzero(&rt, sizeof(struct rtentry));
    bzero(&_sin, sizeof(struct sockaddr_in));
    _sin.sin_family = AF_INET;
    _sin.sin_port = 0;
    if (inet_aton(gateWay.c_str(), &(_sin.sin_addr)) < 0) {
        NETMGR_LOG_E("MockNetdNativeClient inet_aton gateWay[%{public}s]", gateWay.c_str());
        return -1;
    }
    memcpy_s(&rt.rt_gateway, sizeof(rt.rt_gateway), &_sin, sizeof(struct sockaddr_in));
    (reinterpret_cast<struct sockaddr_in*>(&rt.rt_dst))->sin_family=AF_INET;
    if (inet_aton(ip.c_str(), &((struct sockaddr_in*)&rt.rt_dst)->sin_addr) < 0) {
        NETMGR_LOG_E("MockNetdNativeClient inet_aton ip[%{public}s]", ip.c_str());
        return -1;
    }
    (reinterpret_cast<struct sockaddr_in*>(&rt.rt_genmask))->sin_family=AF_INET;
    if (inet_aton(mask.c_str(), &(reinterpret_cast<struct sockaddr_in*>(&rt.rt_genmask))->sin_addr) < 0) {
        NETMGR_LOG_E("MockNetdNativeClient inet_aton mask[%{public}s]", mask.c_str());
        return -1;
    }
    if (!devName.empty()) {
        rt.rt_dev = (char*)devName.c_str();
    }
    rt.rt_flags = RTF_GATEWAY;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        NETMGR_LOG_E("MockNetdNativeClient create socket fd[%{public}d]", fd);
        return -1;
    }
    if (ioctl(fd, SIOCADDRT, &rt) < 0) {
        NETMGR_LOG_I("MockNetdNativeClient ioctl error");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int32_t MockNetdNativeClient::SetDefaultNetWork(int32_t netId)
{
    NETMGR_LOG_D("MockNetdNativeClient SetDefaultNetWork netId is [%{public}d]", netId);
    return 0;
}

int32_t MockNetdNativeClient::ClearDefaultNetWorkNetId()
{
    NETMGR_LOG_D("MockNetdNativeClient ClearDefaultNetWorkNetId");
    return 0;
}

int32_t MockNetdNativeClient::BindSocket(int32_t socket_fd, uint32_t netId)
{
    NETMGR_LOG_D("MockNetdNativeClient::BindSocket: netId = [%{public}u]", netId);
    return 0;
}

int32_t MockNetdNativeClient::IpEnableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("MockNetdNativeClient IpEnableForwarding: requester[%{public}s]", requester.c_str());
    return 0;
}

int32_t MockNetdNativeClient::IpDisableForwarding(const std::string& requester)
{
    NETMGR_LOG_D("MockNetdNativeClient IpDisableForwarding: requester[%{public}s]", requester.c_str());
    return 0;
}

int32_t MockNetdNativeClient::TetherAddForward(const std::string& downstreamIface, const std::string& upstreamIface)
{
    NETMGR_LOG_D("MockNetdNativeClient TetherAddForward: downstreamIface[%{public}s], upstreamIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    return 0;
}

int32_t MockNetdNativeClient::TetherRemoveForward(const std::string& downstreamIface, const std::string& upstreamIface)
{
    NETMGR_LOG_D("MockNetdNativeClient TetherRemoveForward: downstreamIface[%{public}s], upstreamIface[%{public}s]",
        downstreamIface.c_str(), upstreamIface.c_str());
    return 0;
}

int32_t MockNetdNativeClient::IpfwdAddInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("MockNetdNativeClient IpfwdAddInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    return 0;
}

int32_t MockNetdNativeClient::IpfwdRemoveInterfaceForward(const std::string& fromIface, const std::string& toIface)
{
    NETMGR_LOG_D("MockNetdNativeClient IpfwdRemoveInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
        fromIface.c_str(), toIface.c_str());
    return 0;
}

int32_t MockNetdNativeClient::TetherDnsSet(uint32_t netId, const std::vector<std::string>& dnsAddrs)
{
    NETMGR_LOG_D("MockNetdNativeClient TetherDnsSet: netId[%{public}d]", netId);
    for (auto iter = dnsAddrs.begin(); iter != dnsAddrs.end(); ++iter) {
        NETMGR_LOG_D("MockNetdNativeClient TetherDnsSet: dnsAddrs[%{public}s]", iter->c_str());
    }
    return 0;
}

int32_t MockNetdNativeClient::RegisterNetdNotifyCallback(const NetdNotifyCallback &callback)
{
    NETMGR_LOG_D("MockNetdNativeClient RegisterNetdNotifyCallback");
    return 0;
}

int32_t MockNetdNativeClient::BindNetworkServiceVpn(int32_t socketFd)
{
    NETMGR_LOG_D("MockNetdNativeClient::BindNetworkServiceVpn: socketFd[%{public}d]", socketFd);
    return 0;
}

int32_t MockNetdNativeClient::EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd)
{
    NETMGR_LOG_D("MockNetdNativeClient::EnableVirtualNetIfaceCard: socketFd[%{public}d]", socketFd);
    const char *ifaceName = "wlan0";
    strncpy_s(ifRequest.ifr_name, sizeof(ifRequest.ifr_name), ifaceName, strlen(ifaceName));
    NETMGR_LOG_D("ifRequest.ifr_name[%{public}s]", ifRequest.ifr_name);
    return 0;
}

int32_t MockNetdNativeClient::SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
    struct ifreq &ifRequest)
{
    NETMGR_LOG_D("MockNetdNativeClient::SetIpAddress: socketFd[%{public}d], ipAddress[%{public}s], "
        "prefixLen[%{public}d]",
        socketFd, ipAddress.c_str(), prefixLen);
    return 0;
}

int32_t MockNetdNativeClient::SetBlocking(int32_t ifaceFd, bool isBlock)
{
    NETMGR_LOG_D("MockNetdNativeClient::SetBlocking: ifaceFd[%{public}d], isBlock[%{public}d]", ifaceFd, isBlock);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS