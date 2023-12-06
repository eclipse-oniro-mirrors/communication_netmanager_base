/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "vpn_manager.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/ipv6.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "init_socket.h"
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "securec.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
constexpr const char *TUN_CARD_NAME = "vpn-tun";
constexpr const char *TUN_DEVICE_PATH = "/dev/tun";
constexpr int32_t NET_MASK_MAX_LENGTH = 32;
constexpr int32_t MAX_UNIX_SOCKET_CLIENT = 5;
} // namespace

int32_t VpnManager::CreateVpnInterface()
{
    if (tunFd_ != 0) {
        StartVpnInterfaceFdListen();
        return NETMANAGER_SUCCESS;
    }

    ifreq ifr = {};
    if (InitIfreq(ifr, TUN_CARD_NAME) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    int32_t tunfd = open(TUN_DEVICE_PATH, O_RDWR | O_NONBLOCK);
    if (tunfd <= 0) {
        NETNATIVE_LOGE("open virtual device failed: %{public}d", errno);
        return NETMANAGER_ERROR;
    }

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, &ifr) < 0) {
        close(tunfd);
        NETNATIVE_LOGE("tun set iff error: %{public}d", errno);
        return NETMANAGER_ERROR;
    }

    net4Sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (net4Sock_ < 0) {
        close(tunfd);
        NETNATIVE_LOGE("create SOCK_DGRAM ipv4 failed: %{public}d", errno);
        return NETMANAGER_ERROR;
    }
    net6Sock_ = socket(AF_INET6, SOCK_DGRAM, 0);
    if (net6Sock_ < 0) {
        close(tunfd);
        NETNATIVE_LOGE("create SOCK_DGRAM ipv6 failed: %{public}d", errno);
        return NETMANAGER_ERROR;
    }

    NETNATIVE_LOGI("open virtual device successfully, [%{public}d]", tunfd);
    tunFd_ = tunfd;
    SetVpnUp();
    StartVpnInterfaceFdListen();
    return NETMANAGER_SUCCESS;
}

void VpnManager::DestroyVpnInterface()
{
    SetVpnDown();
    if (net4Sock_ != 0) {
        close(net4Sock_);
        net4Sock_ = 0;
    }
    if (net6Sock_ != 0) {
        close(net6Sock_);
        net6Sock_ = 0;
    }
    if (tunFd_ != 0) {
        close(tunFd_);
        tunFd_ = 0;
    }
}

int32_t VpnManager::SetVpnMtu(const std::string &ifName, int32_t mtu)
{
    if (mtu <= 0) {
        NETNATIVE_LOGE("invalid mtu value");
        return NETMANAGER_ERROR;
    }

    ifreq ifr;
    if (InitIfreq(ifr, ifName) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    ifr.ifr_mtu = mtu;
    if (ioctl(net4Sock_, SIOCSIFMTU, &ifr) < 0) {
        NETNATIVE_LOGE("set MTU error, errno:%{public}d", errno);
        return NETMANAGER_ERROR;
    }
    NETNATIVE_LOGI("set MTU success");
    return NETMANAGER_SUCCESS;
}

int32_t VpnManager::SetVpnAddress(const std::string &ifName, const std::string &tunAddr, int32_t prefix)
{
    ifreq ifr = {};
    if (InitIfreq(ifr, ifName) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    bool isIpv6 = CommonUtils::IsValidIPV6(tunAddr);
    if (isIpv6) {
        struct in6_ifreq ifr6 = {};
        if (ioctl(net6Sock_, SIOCGIFINDEX, &ifr) <0) {
            NETNATIVE_LOGE(" get network interface ipv6 failed: %{public}d", errno);
            return NETMANAGER_ERROR;
        }
        if (inet_pton(AF_INET6, tunAddr.c_str(), &ifr6.ifr6_addr) == 0) {
            NETNATIVE_LOGE("inet_pton ipv6 address failed: %{public}d", errno);
        }
        ifr6.ifr6_prefixlen = prefix;
        ifr6.ifr6_ifindex = ifr.ifr_ifindex;
        if (ioctl(net6Sock_, SIOCSIFADDR, &ifr6) < 0) {
            NETNATIVE_LOGE("ioctl set ipv6 address failed: %{public}d", errno);
            return NETMANAGER_ERROR;
        }
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

        if (prefix <= 0 || prefix >= NET_MASK_MAX_LENGTH) {
            NETNATIVE_LOGE("prefix: %{public}d error", prefix);
            return NETMANAGER_ERROR;
        }
        in_addr_t mask = prefix ? (~0 << (NET_MASK_MAX_LENGTH - prefix)) : 0;
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

int32_t VpnManager::SetVpnUp()
{
    ifreq ifr = {};
    if (InitIfreq(ifr, TUN_CARD_NAME) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    ifr.ifr_flags = IFF_UP;
    if (ioctl(net4Sock_, SIOCSIFFLAGS, &ifr) < 0) {
        NETNATIVE_LOGE("set iff up failed: %{public}d", errno);
        return NETMANAGER_ERROR;
    }
    NETNATIVE_LOGI("set iff up success");
    return NETMANAGER_SUCCESS;
}

int32_t VpnManager::SetVpnDown()
{
    ifreq ifr = {};
    if (InitIfreq(ifr, TUN_CARD_NAME) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }

    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(net4Sock_, SIOCSIFFLAGS, &ifr) < 0) {
        NETNATIVE_LOGE("set iff down failed: %{public}d", errno);
        return NETMANAGER_ERROR;
    }
    NETNATIVE_LOGI("set iff down success");
    return NETMANAGER_SUCCESS;
}

int32_t VpnManager::InitIfreq(ifreq &ifr, const std::string &cardName)
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

int32_t VpnManager::SendVpnInterfaceFdToClient(int32_t clientFd, int32_t tunFd)
{
    char buf[1] = {0};
    iovec iov;
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    union {
        cmsghdr align;
        char cmsg[CMSG_SPACE(sizeof(int32_t))];
    } cmsgu;
    if (memset_s(cmsgu.cmsg, sizeof(cmsgu.cmsg), 0, sizeof(cmsgu.cmsg)) != EOK) {
        NETNATIVE_LOGE("memset_s cmsgu.cmsg failed!");
        return NETMANAGER_ERROR;
    }
    msghdr message;
    if (memset_s(&message, sizeof(message), 0, sizeof(message)) != EOK) {
        NETNATIVE_LOGE("memset_s message failed!");
        return NETMANAGER_ERROR;
    }

    message.msg_iov = &iov;
    message.msg_iovlen = 1;
    message.msg_control = cmsgu.cmsg;
    message.msg_controllen = sizeof(cmsgu.cmsg);
    cmsghdr *cmsgh = CMSG_FIRSTHDR(&message);
    cmsgh->cmsg_len = CMSG_LEN(sizeof(tunFd));
    cmsgh->cmsg_level = SOL_SOCKET;
    cmsgh->cmsg_type = SCM_RIGHTS;
    if (memcpy_s(CMSG_DATA(cmsgh), sizeof(tunFd), &tunFd, sizeof(tunFd)) != EOK) {
        NETNATIVE_LOGE("memcpy_s cmsgu failed!");
        return NETMANAGER_ERROR;
    }
    if (sendmsg(clientFd, &message, 0) < 0) {
        NETNATIVE_LOGE("sendmsg error: %{public}d, clientfd[%{public}d], tunfd[%{public}d]", errno, clientFd, tunFd);
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}

void VpnManager::StartUnixSocketListen()
{
    NETNATIVE_LOGI("StartUnixSocketListen...");
    int32_t serverfd = GetControlSocket("tunfd");
    if (listen(serverfd, MAX_UNIX_SOCKET_CLIENT) < 0) {
        close(serverfd);
        NETNATIVE_LOGE("listen socket error: %{public}d", errno);
        return;
    }

    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    while (true) {
        int32_t clientFd = accept(serverfd, reinterpret_cast<sockaddr *>(&clientAddr), &len);
        if (clientFd < 0) {
            NETNATIVE_LOGE("accept socket error: %{public}d", errno);
            continue;
        }

        SendVpnInterfaceFdToClient(clientFd, tunFd_);
        close(clientFd);
    }

    close(serverfd);
    listeningFlag_ = false;
}

void VpnManager::StartVpnInterfaceFdListen()
{
    if (listeningFlag_) {
        NETNATIVE_LOGI("VpnInterface fd is listening...");
        return;
    }

    NETNATIVE_LOGI("StartVpnInterfaceFdListen...");
    std::thread unixThread([this]() { StartUnixSocketListen(); });
    unixThread.detach();
    pthread_setname_np(unixThread.native_handle(), "unix_socket_tunfd");
    listeningFlag_ = true;
}

} // namespace NetManagerStandard
} // namespace OHOS
