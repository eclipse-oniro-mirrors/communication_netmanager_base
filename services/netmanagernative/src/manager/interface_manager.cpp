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

#include "interface_manager.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>
#include <regex>

#include "netlink_manager.h"
#include "netlink_msg.h"
#include "netlink_socket.h"
#include "netlink_socket_diag.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;
using namespace NetManagerStandard::CommonUtils;

namespace {
constexpr const char *SYS_NET_PATH = "/sys/class/net/";
constexpr const char *MTU_PATH = "/mtu";
constexpr int32_t FILE_PERMISSION = 0666;
constexpr uint32_t ARRAY_OFFSET_1_INDEX = 1;
constexpr uint32_t ARRAY_OFFSET_2_INDEX = 2;
constexpr uint32_t ARRAY_OFFSET_3_INDEX = 3;
constexpr uint32_t ARRAY_OFFSET_4_INDEX = 4;
constexpr uint32_t ARRAY_OFFSET_5_INDEX = 5;
constexpr uint32_t MOVE_BIT_LEFT31 = 31;
constexpr uint32_t BIT_MAX = 32;
constexpr uint32_t IOCTL_RETRY_TIME = 32;
constexpr int32_t MAX_MTU_LEN = 11;
constexpr int32_t MAC_ADDRESS_STR_LEN = 18;
constexpr int32_t MAC_ADDRESS_INT_LEN = 6;
constexpr int32_t MAC_SSCANF_SPACE = 3;
const std::regex REGEX_CMD_MAC_ADDRESS("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");

bool CheckFilePath(const std::string &fileName, std::string &realPath)
{
    char tmpPath[PATH_MAX] = {0};
    if (!realpath(fileName.c_str(), tmpPath)) {
        NETNATIVE_LOGE("file name is illegal");
        return false;
    }
    realPath = tmpPath;
    return true;
}
} // namespace

int InterfaceManager::GetMtu(const char *interfaceName)
{
    if (interfaceName == nullptr) {
        NETNATIVE_LOGE("interfaceName is null");
        return -1;
    }

    if (!CheckIfaceName(interfaceName)) {
        NETNATIVE_LOGE("GetMtu isIfaceName fail %{public}d", errno);
        return -1;
    }
    std::string mtuPath = std::string(SYS_NET_PATH).append(interfaceName).append(MTU_PATH);
    std::string realPath;
    if (!CheckFilePath(mtuPath, realPath)) {
        NETNATIVE_LOGE("file does not exist! ");
        return -1;
    }
    int fd = open(realPath.c_str(), 0, FILE_PERMISSION);
    if (fd == -1) {
        NETNATIVE_LOGE("GetMtu open fail %{public}d", errno);
        return -1;
    }

    char originMtuValue[MAX_MTU_LEN] = {0};
    int nread = read(fd, originMtuValue, (sizeof(char) * (MAX_MTU_LEN - 1)));
    if (nread == -1 || nread == 0) {
        NETNATIVE_LOGE("GetMtu read fail %{public}d", errno);
        close(fd);
        return -1;
    }
    close(fd);

    int32_t mtu = -1;
    mtu = StrToInt(originMtuValue);
    return mtu;
}

int InterfaceManager::SetMtu(const char *interfaceName, const char *mtuValue)
{
    if (interfaceName == nullptr || mtuValue == nullptr) {
        NETNATIVE_LOGE("interfaceName or mtuValue is null");
        return -1;
    }

    if (!CheckIfaceName(interfaceName)) {
        NETNATIVE_LOGE("SetMtu isIfaceName fail %{public}d", errno);
    }
    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        NETNATIVE_LOGE("SetMtu socket fail %{public}d", errno);
        return -1;
    }

    struct ifreq ifr;
    if (memset_s(&ifr, sizeof(ifr), 0, sizeof(ifr)) != EOK) {
        close(sockfd);
        return -1;
    }
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, interfaceName, strlen(interfaceName)) != EOK) {
        close(sockfd);
        return -1;
    }

    int32_t mtu = StrToInt(mtuValue);
    ifr.ifr_mtu = mtu;

    if (ioctl(sockfd, SIOCSIFMTU, &ifr) < 0) {
        NETNATIVE_LOGE("SetMtu ioctl fail %{public}d", errno);
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

std::vector<std::string> InterfaceManager::GetInterfaceNames()
{
    std::vector<std::string> ifaceNames;
    DIR *dir(nullptr);
    struct dirent *de(nullptr);

    dir = opendir(SYS_NET_PATH);
    if (dir == nullptr) {
        NETNATIVE_LOGE("GetInterfaceNames opendir fail %{public}d", errno);
        return ifaceNames;
    }

    de = readdir(dir);
    while (de != nullptr) {
        if ((de->d_name[0] != '.') && ((de->d_type == DT_DIR) || (de->d_type == DT_LNK))) {
            ifaceNames.push_back(std::string(de->d_name));
        }
        de = readdir(dir);
    }
    closedir(dir);

    return ifaceNames;
}

int InterfaceManager::ModifyAddress(uint32_t action, const char *interfaceName, const char *addr, int prefixLen)
{
    if (interfaceName == nullptr || addr == nullptr) {
        return -1;
    }
    uint32_t index = if_nametoindex(interfaceName);
    if (index == 0) {
        NETNATIVE_LOGE("ModifyAddress, if_nametoindex error %{public}d", errno);
        return -errno;
    }
    auto family = CommonUtils::GetAddrFamily(addr);
    if (family != AF_INET && family != AF_INET6) {
        NETNATIVE_LOGE("Ivalid ip address: %{public}s", addr);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    ifaddrmsg ifm = {static_cast<uint8_t>(family), static_cast<uint8_t>(prefixLen), 0, 0, index};
    nmd::NetlinkMsg nlmsg(NLM_F_CREATE | NLM_F_EXCL, nmd::NETLINK_MAX_LEN, getpid());
    nlmsg.AddAddress(action, ifm);

    if (family == AF_INET6) {
        in6_addr in6Addr;
        if (inet_pton(AF_INET6, addr, &in6Addr) == -1) {
            NETNATIVE_LOGE("Modify ipv6 address, inet_pton error %{public}d", errno);
            return NETMANAGER_ERR_INTERNAL;
        }
        nlmsg.AddAttr(IFA_LOCAL, &in6Addr, sizeof(in6Addr));
    } else {
        in_addr inAddr;
        if (inet_pton(AF_INET, addr, &inAddr) == -1) {
            NETNATIVE_LOGE("Modify ipv4 address, inet_pton error %{public}d", errno);
            return NETMANAGER_ERR_INTERNAL;
        }
        nlmsg.AddAttr(IFA_LOCAL, &inAddr, sizeof(inAddr));
        if (action == RTM_NEWADDR) {
            inAddr.s_addr |= htonl((1U << (BIT_MAX - prefixLen)) - 1);
            nlmsg.AddAttr(IFA_BROADCAST, &inAddr, sizeof(inAddr));
        }
    }

    NETNATIVE_LOGI("ModifyAddress:%{public}u %{public}s %{public}s %{public}d", action, interfaceName,
                   ToAnonymousIp(addr).c_str(), prefixLen);

    return SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
}

int InterfaceManager::AddAddress(const char *interfaceName, const char *addr, int prefixLen)
{
    return ModifyAddress(RTM_NEWADDR, interfaceName, addr, prefixLen);
}

int InterfaceManager::DelAddress(const char *interfaceName, const char *addr, int prefixLen)
{
    NetLinkSocketDiag socketDiag;
    socketDiag.DestroyLiveSockets(addr, true);
    return ModifyAddress(RTM_DELADDR, interfaceName, addr, prefixLen);
}

int Ipv4NetmaskToPrefixLength(in_addr_t mask)
{
    int prefixLength = 0;
    uint32_t m = ntohl(mask);
    const uint32_t referenceValue = 1;
    while (m & (referenceValue << MOVE_BIT_LEFT31)) {
        prefixLength++;
        m = m << referenceValue;
    }
    return prefixLength;
}

std::string HwAddrToStr(char *hwaddr)
{
    char buf[64] = {'\0'};
    if (hwaddr != nullptr) {
        errno_t result =
            sprintf_s(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", hwaddr[0], hwaddr[ARRAY_OFFSET_1_INDEX],
                      hwaddr[ARRAY_OFFSET_2_INDEX], hwaddr[ARRAY_OFFSET_3_INDEX], hwaddr[ARRAY_OFFSET_4_INDEX],
                      hwaddr[ARRAY_OFFSET_5_INDEX]);
        if (result != 0) {
            NETNATIVE_LOGE("[hwAddrToStr]: result %{public}d", result);
        }
    }
    return std::string(buf);
}

void UpdateIfaceConfigFlags(unsigned flags, nmd::InterfaceConfigurationParcel &ifaceConfig)
{
    ifaceConfig.flags.emplace_back(flags & IFF_UP ? "up" : "down");
    if (flags & IFF_BROADCAST) {
        ifaceConfig.flags.emplace_back("broadcast");
    }
    if (flags & IFF_LOOPBACK) {
        ifaceConfig.flags.emplace_back("loopback");
    }
    if (flags & IFF_POINTOPOINT) {
        ifaceConfig.flags.emplace_back("point-to-point");
    }
    if (flags & IFF_RUNNING) {
        ifaceConfig.flags.emplace_back("running");
    }
    if (flags & IFF_MULTICAST) {
        ifaceConfig.flags.emplace_back("multicast");
    }
}

InterfaceConfigurationParcel InterfaceManager::GetIfaceConfig(const std::string &ifName)
{
    NETNATIVE_LOGI("GetIfaceConfig in. ifName %{public}s", ifName.c_str());
    struct in_addr addr = {};
    nmd::InterfaceConfigurationParcel ifaceConfig;

    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    struct ifreq ifr = {};
    strncpy_s(ifr.ifr_name, IFNAMSIZ, ifName.c_str(), ifName.length());

    ifaceConfig.ifName = ifName;
    if (ioctl(fd, SIOCGIFADDR, &ifr) != -1) {
        addr.s_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
        ifaceConfig.ipv4Addr = std::string(inet_ntoa(addr));
    }
    if (ioctl(fd, SIOCGIFNETMASK, &ifr) != -1) {
        ifaceConfig.prefixLength = Ipv4NetmaskToPrefixLength(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
    }
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) != -1) {
        UpdateIfaceConfigFlags(ifr.ifr_flags, ifaceConfig);
    }
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) != -1) {
        ifaceConfig.hwAddr = HwAddrToStr(ifr.ifr_hwaddr.sa_data);
    }
    close(fd);
    return ifaceConfig;
}

int InterfaceManager::SetIfaceConfig(const nmd::InterfaceConfigurationParcel &ifaceConfig)
{
    struct ifreq ifr = {};
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, ifaceConfig.ifName.c_str(), ifaceConfig.ifName.length()) != 0) {
        NETNATIVE_LOGE("ifaceConfig strncpy_s error.");
        return -1;
    }

    if (ifaceConfig.flags.empty()) {
        NETNATIVE_LOGE("ifaceConfig flags is empty.");
        return -1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        NETNATIVE_LOGE("ifaceConfig socket error, errno[%{public}d]", errno);
        return -1;
    }
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == -1) {
        char errmsg[INTERFACE_ERR_MAX_LEN] = {0};
        strerror_r(errno, errmsg, INTERFACE_ERR_MAX_LEN);
        NETNATIVE_LOGE("fail to set interface config. strerror[%{public}s]", errmsg);
        close(fd);
        return -1;
    }
    short flags = ifr.ifr_flags;
    auto fit = std::find(ifaceConfig.flags.begin(), ifaceConfig.flags.end(), "up");
    if (fit != std::end(ifaceConfig.flags)) {
        uint16_t ifrFlags = static_cast<uint16_t>(ifr.ifr_flags);
        ifrFlags = ifrFlags | IFF_UP;
        ifr.ifr_flags = static_cast<short>(ifrFlags);
    }
    fit = std::find(ifaceConfig.flags.begin(), ifaceConfig.flags.end(), "down");
    if (fit != std::end(ifaceConfig.flags)) {
        ifr.ifr_flags = (short)((uint16_t)ifr.ifr_flags & (~IFF_UP));
    }
    if (ifr.ifr_flags == flags) {
        close(fd);
        return 1;
    }
    uint32_t retry = 0;
    do {
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) != -1) {
            break;
        }
        ++retry;
    } while (errno == ETIMEDOUT && retry < IOCTL_RETRY_TIME);
    NETNATIVE_LOGI("set ifr flags=[%{public}d] strerror=[%{public}s] retry=[%{public}u]", ifr.ifr_flags,
                   strerror(errno), retry);
    close(fd);
    return 1;
}

int InterfaceManager::SetIpAddress(const std::string &ifaceName, const std::string &ipAddress)
{
    struct ifreq ifr;
    struct in_addr ipv4Addr = {INADDR_ANY};

    if (memset_s(&ifr, sizeof(ifr), 0, sizeof(ifr)) != EOK) {
        NETNATIVE_LOGE("memset is false");
        return -1;
    }
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, ifaceName.c_str(), strlen(ifaceName.c_str())) != EOK) {
        NETNATIVE_LOGE("strncpy is false");
        return -1;
    }
    if (inet_aton(ipAddress.c_str(), &ipv4Addr) == 0) {
        NETNATIVE_LOGE("set net ip is false");
        return -1;
    }
    sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_addr);
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr = ipv4Addr;
    int32_t inetSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ioctl(inetSocket, SIOCSIFADDR, &ifr) < 0) {
        NETNATIVE_LOGE("set ip address ioctl SIOCSIFADDR error: %{public}s", strerror(errno));
        close(inetSocket);
        return -1;
    }
    close(inetSocket);
    return 0;
}

int InterfaceManager::SetIffUp(const std::string &ifaceName)
{
    struct ifreq ifr;

    if (memset_s(&ifr, sizeof(ifr), 0, sizeof(ifr)) != EOK) {
        NETNATIVE_LOGE("memset is false");
        return -1;
    }
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, ifaceName.c_str(), strlen(ifaceName.c_str())) != EOK) {
        NETNATIVE_LOGE("strncpy is false");
        return -1;
    }
    uint32_t flagVal = (IFF_UP | IFF_MULTICAST);
    ifr.ifr_flags = static_cast<short int>(flagVal);

    int32_t inetSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ioctl(inetSocket, SIOCSIFFLAGS, &ifr) < 0) {
        NETNATIVE_LOGE("set iface up ioctl SIOCSIFFLAGS error: %{public}s", strerror(errno));
        close(inetSocket);
        return -1;
    }
    close(inetSocket);
    return 0;
}

int32_t InterfaceManager::AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                       const std::string &ifName)
{
    arpreq req = {};
    int32_t res = AssembleArp(ipAddr, macAddr, ifName, req);
    if (res != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("AssembleArp error");
        return res;
    }

    int32_t inetSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ioctl(inetSocket, SIOCSARP, &req) < 0) {
        NETNATIVE_LOGE("AddStaticArp ioctl SIOCSARP error: %{public}s", strerror(errno));
        close(inetSocket);
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    close(inetSocket);
    return NETMANAGER_SUCCESS;
}

int32_t InterfaceManager::DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                       const std::string &ifName)
{
    arpreq req = {};
    int32_t res = AssembleArp(ipAddr, macAddr, ifName, req);
    if (res != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("AssembleArp error");
        return res;
    }

    int32_t inetSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ioctl(inetSocket, SIOCDARP, &req) < 0) {
        NETNATIVE_LOGE("DelStaticArp ioctl SIOCDARP error: %{public}s", strerror(errno));
        close(inetSocket);
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    close(inetSocket);
    return NETMANAGER_SUCCESS;
}

int32_t InterfaceManager::AssembleArp(const std::string &ipAddr, const std::string &macAddr,
                                      const std::string &ifName, arpreq &req)
{
    if (!IsValidIPV4(ipAddr)) {
        NETNATIVE_LOGE("ipAddr error");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    if (!regex_match(macAddr, REGEX_CMD_MAC_ADDRESS)) {
        NETNATIVE_LOGE("macAddr error");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    sockaddr& ethAddrStruct = req.arp_ha;
    ethAddrStruct.sa_family = ARPHRD_ETHER;
    if (MacStringToArray(macAddr, ethAddrStruct) != 0) {
        NETNATIVE_LOGE("macStringToArray error");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    in_addr ipv4Addr = {};
    if (inet_aton(ipAddr.c_str(), &ipv4Addr) == 0) {
        NETNATIVE_LOGE("addr inet_aton error");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    auto sin = reinterpret_cast<sockaddr_in *>(&req.arp_pa);
    sin->sin_family = AF_INET;
    sin->sin_addr = ipv4Addr;

    if (strncpy_s(req.arp_dev, sizeof(req.arp_dev),
                  ifName.c_str(), ifName.size()) != 0) {
        NETNATIVE_LOGE("strncpy_s is false");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    req.arp_flags = ATF_COM;

    return NETMANAGER_SUCCESS;
}

int32_t InterfaceManager::MacStringToArray(const std::string &macAddr, sockaddr &macSock)
{
    char strMac[MAC_ADDRESS_INT_LEN] = {};
    char strAddr[MAC_ADDRESS_STR_LEN] = {};
    uint32_t v = 0;
    if (memcpy_s(strAddr, MAC_ADDRESS_STR_LEN, macAddr.c_str(), macAddr.size()) != 0) {
        NETNATIVE_LOGE("memcpy_s is false");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    for (int i = 0; i < MAC_ADDRESS_INT_LEN; i++) {
        if (sscanf_s(strAddr+MAC_SSCANF_SPACE*i, "%2x", &v) <= 0) {
            NETNATIVE_LOGE("sscanf_s is false");
            return NETMANAGER_ERR_OPERATION_FAILED;
        }
        strMac[i] = (char)v;
    }

    if (memcpy_s(macSock.sa_data, sizeof(macSock.sa_data),
                  strMac, MAC_ADDRESS_INT_LEN) != 0) {
        NETNATIVE_LOGE("memcpy_s is false");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    return NETMANAGER_SUCCESS;
}
} // namespace nmd
} // namespace OHOS
