/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "distributed_manager.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/ipv6.h>

#include "init_socket.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
constexpr const char *DISTRIBUTED_TUN_DEVICE_PATH = "/dev/tun";
constexpr int32_t NET_MASK_MAX_LENGTH = 24;
constexpr int32_t DISTRIBUTED_MTU = 1400;
constexpr const char *IP_CMD_PATH = "/system/bin/ip";
} // namespace

int32_t DistributedManager::CreateDistributedInterface(const std::string &ifName)
{
    net4Sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (net4Sock_ < 0) {
        NETNATIVE_LOGE("create SOCK_DGRAM ipv4 failed: %{public}d", errno);
        return NETMANAGER_ERROR;
    }

    if (tunFd_ != 0) {
        return NETMANAGER_SUCCESS;
    }

    ifreq ifr{};
    if (InitIfreq(ifr, ifName) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    int32_t tunfd = open(DISTRIBUTED_TUN_DEVICE_PATH, O_RDWR | O_NONBLOCK);
    if (tunfd <= 0) {
        NETNATIVE_LOGE("open virtual distributed nic failed: %{public}d", errno);
        return NETMANAGER_ERROR;
    }

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, &ifr) < 0) {
        close(tunfd);
        NETNATIVE_LOGE("tun set iff error: %{public}d", errno);
        return NETMANAGER_ERROR;
    }

    NETNATIVE_LOGI("create virtual device successfully, [%{public}d]", tunfd);
    tunFd_ = tunfd;

    return NETMANAGER_SUCCESS;
}

int32_t DistributedManager::SetDistributedNicResult(std::atomic_int &fd, unsigned long cmd, ifreq &ifr)
{
    if (fd > 0) {
        if (ioctl(fd, cmd, &ifr) < 0) {
            NETNATIVE_LOGE("set virnic error, errno:%{public}d", errno);
            return NETMANAGER_ERROR;
        }
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_ERROR;
}

int32_t DistributedManager::InitIfreq(ifreq &ifr, const std::string &cardName)
{
    if (memset_s(&ifr, sizeof(ifr), 0, sizeof(ifr)) != EOK) {
        NETNATIVE_LOGE("memset_s ifr failed!");
        return NETMANAGER_ERROR;
    }
    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, cardName.c_str(), strlen(cardName.c_str())) != EOK) {
        NETNATIVE_LOGE("strcpy_s ifr name fail");
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}

int32_t DistributedManager::SetDistributedNicMtu(const std::string &ifName, int32_t mtu)
{
    if (mtu <= 0) {
        NETNATIVE_LOGE("invalid mtu value");
        return NETMANAGER_ERROR;
    }

    ifreq ifr{};
    if (InitIfreq(ifr, ifName) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    ifr.ifr_mtu = mtu;
    int32_t ret4 = SetDistributedNicResult(net4Sock_, SIOCSIFMTU, ifr);
    if (ret4 == NETMANAGER_ERROR || (net4Sock_ < 0)) {
        NETNATIVE_LOGI("set MTU failed");
        return NETMANAGER_ERROR;
    } else {
        NETNATIVE_LOGI("set MTU success");
        return NETMANAGER_SUCCESS;
    }
}

int32_t DistributedManager::SetDistributedNicAddress(const std::string &ifName, const std::string &tunAddr)
{
    ifreq ifr{};
    if (InitIfreq(ifr, ifName) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    bool isIpv6 = CommonUtils::IsValidIPV6(tunAddr);
    if (isIpv6) {
        NETNATIVE_LOGE("distributed nic not support ipv6 by now");
        return NETMANAGER_ERROR;
    } else {
        in_addr ipv4Addr = {};
        if (inet_aton(tunAddr.c_str(), &ipv4Addr) == 0) {
            NETNATIVE_LOGE("addr inet_aton error");
            return NETMANAGER_ERROR;
        }

        auto sin = reinterpret_cast<sockaddr_in *>(&ifr.ifr_addr);
        sin->sin_family = AF_INET;
        sin->sin_addr = ipv4Addr;
        if (ioctl(net4Sock_, SIOCSIFADDR, &ifr) < 0) {
            NETNATIVE_LOGE("ioctl set ipv4 address failed: %{public}d", errno);
            return NETMANAGER_ERROR;
        }

        int32_t prefix = NET_MASK_MAX_LENGTH;
        in_addr_t mask = prefix ? (0xFFFFFFFF << (NET_MASK_MAX_LENGTH - prefix)) : 0;
        sin = reinterpret_cast<sockaddr_in *>(&ifr.ifr_netmask);
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = htonl(mask);
        if (ioctl(net4Sock_, SIOCSIFNETMASK, &ifr) < 0) {
            NETNATIVE_LOGE("ioctl set ip mask failed: %{public}d", errno);
            return NETMANAGER_ERROR;
        }
    }

    NETNATIVE_LOGI("set ip address success");
    return NETMANAGER_SUCCESS;
}

int32_t DistributedManager::SetDistributedNicUp(const std::string &ifName)
{
    ifreq ifr{};
    if (InitIfreq(ifr, ifName) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    ifr.ifr_flags = static_cast<uint16_t>(IFF_UP | IFF_NOARP);

    int32_t ret4 = SetDistributedNicResult(net4Sock_, SIOCSIFFLAGS, ifr);
    if (ret4 == NETMANAGER_ERROR || (net4Sock_ < 0)) {
        NETNATIVE_LOGI("set iff up failed");
        return NETMANAGER_ERROR;
    } else {
        NETNATIVE_LOGI("set iff up success");
        return NETMANAGER_SUCCESS;
    }
}

int32_t DistributedManager::SetDistributedNicDown(const std::string &ifName)
{
    ifreq ifr{};
    if (InitIfreq(ifr, ifName) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    ifr.ifr_flags = (uint16_t)ifr.ifr_flags & ~IFF_UP;
    int32_t ret4 = SetDistributedNicResult(net4Sock_, SIOCSIFFLAGS, ifr);
    if (ret4 == NETMANAGER_ERROR || (net4Sock_ < 0)) {
        NETNATIVE_LOGI("set iff down failed");
        return NETMANAGER_ERROR;
    } else {
        NETNATIVE_LOGI("set iff down success");
        return NETMANAGER_SUCCESS;
    }
}

void DistributedManager::CloseDistributedSocket()
{
    if (net4Sock_ > 0) {
        close(net4Sock_);
        net4Sock_ = 0;
    }
}

void DistributedManager::CloseDistributedTunFd()
{
    if (tunFd_ > 0) {
        close(tunFd_);
        tunFd_ = 0;
    }
}

int32_t DistributedManager::CreateDistributedNic(const std::string &virNicAddr, const std::string &ifName)
{
    NETNATIVE_LOGI("CreateVirnic, mtu:%{public}d", DISTRIBUTED_MTU);
    if (CreateDistributedInterface(ifName) != NETMANAGER_SUCCESS) {
        CloseDistributedSocket();
        return NETMANAGER_ERROR;
    }
    if (SetDistributedNicMtu(ifName, DISTRIBUTED_MTU) != NETMANAGER_SUCCESS ||
        SetDistributedNicAddress(ifName, virNicAddr) != NETMANAGER_SUCCESS) {
        SetDistributedNicDown(ifName);
        CloseDistributedSocket();
        return NETMANAGER_ERROR;
    }
    if (SetDistributedNicUp(ifName) != NETMANAGER_SUCCESS) {
        CloseDistributedSocket();
        return NETMANAGER_ERROR;
    }
    CloseDistributedSocket();

    return NETMANAGER_SUCCESS;
}

int32_t DistributedManager::DestroyDistributedNic(const std::string &ifName)
{
    SetDistributedNicDown(ifName);
    return NETMANAGER_SUCCESS;
}

void DistributedManager::SetServerNicInfo(const std::string &iif, const std::string &devIface)
{
    serverIif_ = iif;
    serverDevIface_ = devIface;
}

std::string DistributedManager::GetServerIifNic()
{
    return serverIif_;
}

std::string DistributedManager::GetServerDevIfaceNic()
{
    return serverDevIface_;
}

int32_t DistributedManager::ConfigVirnicAndVeth(const std::string &virNicAddr, const std::string &virnicName,
    const std::string &virnicVethName)
{
    if (virnicName.empty() || virnicVethName.empty()) {
        NETNATIVE_LOGE("NicName is nullptr");
        return NETMANAGER_ERROR;
    }

    if (!CommonUtils::IsValidIPV4(virNicAddr)) {
        NETNATIVE_LOGE("the virNicAddr is not valid");
        return NETMANAGER_ERROR;
    }
    
    // Step1: ip link add virnic type veth peer name virnic1
    std::string out;
    std::string createVirnic = std::string(IP_CMD_PATH) + " link add " + virnicName +
        " type veth peer name " + virnicVethName;
    NETNATIVE_LOGI("setup virnic and veth : %{public}s", createVirnic.c_str());
    if (CommonUtils::ForkExec(createVirnic.c_str(), &out) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("setup virnic and veth failed, output %{public}s", out.c_str());
        return NETMANAGER_ERROR;
    }
 
    // Step2-1: ip link set virnic up
    std::string virnicUp = std::string(IP_CMD_PATH) + " link set " + virnicName + " up";
    NETNATIVE_LOGI("set virnic up: %{public}s", virnicUp.c_str());
    if (CommonUtils::ForkExec(virnicUp.c_str(), &out) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("set virnic up, output: %{public}s.", out.c_str());
        return NETMANAGER_ERROR;
    }
    // Step2-2: ip link set virnic-veth up
    std::string virnicVethUp = std::string(IP_CMD_PATH) + " link set " + virnicVethName + " up";
    NETNATIVE_LOGI("set virnicVeth up: %{public}s", virnicVethUp.c_str());
    if (CommonUtils::ForkExec(virnicVethUp.c_str(), &out) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("set virnicVeth up failed, output: %{public}s.", out.c_str());
        return NETMANAGER_ERROR;
    }
 
    // Step3-1: ip addr add xx.xx.xx.xx/24 dev virnic
    std::string cfgVirnic = std::string(IP_CMD_PATH) + " addr add " + virNicAddr + "/24 dev " + virnicName;
    NETNATIVE_LOGI("add virnic ip: %{public}s", CommonUtils::ToAnonymousIp(cfgVirnic).c_str());
    if (CommonUtils::ForkExec(cfgVirnic.c_str(), &out) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("add virnic ip failed, output %{public}s.", out.c_str());
        return NETMANAGER_ERROR;
    }
 
    std::string virNicVethAddr = CommonUtils::GetGatewayAddr(virNicAddr, "255.255.255.0");
    if (virNicVethAddr.empty()) {
        NETNATIVE_LOGE("get gateway addr is empty");
        return NETMANAGER_ERROR;
    }

    // Step3-1: ip addr add xx.xx.xx.1/24 dev virnic
    std::string cfgVirnicVeth = std::string(IP_CMD_PATH) + " addr add " + virNicVethAddr + "/24 dev " + virnicVethName;
    NETNATIVE_LOGI("add virNic-veth ip: %{public}s", CommonUtils::ToAnonymousIp(cfgVirnicVeth).c_str());
    if (CommonUtils::ForkExec(cfgVirnicVeth.c_str(), &out) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("add virNic-veth ip failed, output %{public}s.", out.c_str());
        return NETMANAGER_ERROR;
    }
 
    return NETMANAGER_SUCCESS;
}
 
void DistributedManager::DisableVirnic(const std::string &virnicName)
{
    std::string out;
    std::string delVirnic = std::string(IP_CMD_PATH) + " link del " + virnicName;
    NETNATIVE_LOGI("del virnic: %{public}s", delVirnic.c_str());
    if (CommonUtils::ForkExec(delVirnic.c_str(), &out) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("DisableVirnic del Virnic failed, output %{public}s", out.c_str());
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
