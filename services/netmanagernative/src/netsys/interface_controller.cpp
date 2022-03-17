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

#include "interface_controller.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <system_error>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include "securec.h"
#include "netlink_socket.h"
#include "netlink_manager.h"
#include "netlink_msg.h"
#include "netnative_log_wrapper.h"

const char g_sysNetPath[] = "/sys/class/net/";

namespace OHOS {
namespace nmd {
namespace {
    constexpr int32_t FILE_PERMISSION = 0666;
    constexpr uint32_t BIT_MAX = 32;
}

InterfaceController::InterfaceController() {}

InterfaceController::~InterfaceController() {}

bool IfaceNameValidCheck(const std::string &name)
{
    int index = 0;

    if (name.empty()) {
        return false;
    }

    int len = static_cast<int>(name.size());
    if (len > 16) { /* 16: interface name min size. */
        return false;
    }

    while (index < len) {
        if ((index == 0) && !isalnum(name[index])) {
            return false;
        }

        if (!isalnum(name[index]) &&
            (name[index] != '-') &&
            (name[index] != '_') &&
            (name[index] != '.') &&
            (name[index] != ':')) {
            return false;
        }
        index++;
    }

    return true;
}

int InterfaceController::GetMtu(const char *interfaceName)
{
    if (!IfaceNameValidCheck(interfaceName)) {
        NETNATIVE_LOGE("InterfaceController::GetMtu isIfaceName fail %{public}d", errno);
        return -1;
    }

    std::string setMtuPath = std::string(g_sysNetPath).append(interfaceName).append("/mtu");

    int fd = open(setMtuPath.c_str(), 0, FILE_PERMISSION);
    if (fd == -1) {
        NETNATIVE_LOGE("InterfaceController::GetMtu open fail %{public}d", errno);
        return -1;
    }

    int originMtuValue = 0;
    int nread = read(fd, &originMtuValue, sizeof(originMtuValue));
    if (nread == -1) {
        NETNATIVE_LOGE("InterfaceController::GetMtu read fail %{public}d", errno);
        close(fd);
        return -1;
    }
    close(fd);

    return atoi((char *)&originMtuValue);
}

int InterfaceController::SetMtu(const char *interfaceName, const char *mtuValue)
{
    if (!IfaceNameValidCheck(interfaceName)) {
        NETNATIVE_LOGE("InterfaceController::SetMtu isIfaceName fail %{public}d", errno);
        return -1;
    }

    std::string setMtuPath = std::string(g_sysNetPath).append(interfaceName).append("/mtu");

    int fd = open(setMtuPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, FILE_PERMISSION);
    if (fd == -1) {
        NETNATIVE_LOGE("InterfaceController::SetMtu open fail %{public}d", errno);
        return -1;
    }

    int nwrite = write(fd, mtuValue, strlen(mtuValue));
    if (nwrite == -1) {
        NETNATIVE_LOGE("InterfaceController::SetMtu write fail %{public}d", errno);
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

std::vector<std::string> InterfaceController::GetInterfaceNames()
{
    std::vector<std::string> ifaceNames;
    DIR *dir(nullptr);
    struct dirent *de(nullptr);

    dir = opendir(g_sysNetPath);
    if (dir == nullptr) {
        NETNATIVE_LOGE("InterfaceController::GetInterfaceNames opendir fail %{public}d", errno);
        return ifaceNames;
    }

    de = readdir(dir);
    while (de != nullptr) {
        if ((de->d_name[0] == '.') &&
            ((de->d_type == DT_DIR) || (de->d_type == DT_LNK))) {
            ifaceNames.push_back(std::string(de->d_name));
        }
        de = readdir(dir);
    }
    closedir(dir);

    return ifaceNames;
}

int InterfaceController::ModifyAddress(uint32_t action, const char *interfaceName, const char *addr, int prefixLen)
{
    uint32_t index = if_nametoindex(interfaceName);
    if (index == 0) {
        NETNATIVE_LOGE("InterfaceController::ModifyAddress, if_nametoindex error %{public}d", errno);
        return -errno;
    }

    nmd::NetlinkSocket netLinker;
    netLinker.Create(NETLINK_ROUTE);
    nmd::NetlinkMsg nlmsg(NLM_F_CREATE | NLM_F_EXCL, nmd::NETLINK_MAX_LEN, NetlinkManager::GetPid());

    struct ifaddrmsg ifm = {0};
    ifm.ifa_family = AF_INET;
    ifm.ifa_index = index;
    ifm.ifa_scope = 0;
    ifm.ifa_prefixlen = static_cast<uint32_t>(prefixLen);

    nlmsg.AddAddress(action, ifm);

    struct in_addr inAddr;
    int ret = inet_pton(AF_INET, addr, &inAddr);
    if (ret == -1) {
        NETNATIVE_LOGE("InterfaceController::ModifyAddress, inet_pton error %{public}d", errno);
        return -errno;
    }

    nlmsg.AddAttr(IFA_LOCAL, &inAddr, sizeof(inAddr));

    if (action == RTM_NEWADDR) {
        inAddr.s_addr |= htonl((1U << (BIT_MAX - prefixLen)) - 1);
        nlmsg.AddAttr(IFA_BROADCAST, &inAddr, sizeof(inAddr));
    }

    NETNATIVE_LOGI("InterfaceController::ModifyAddress:%{public}u %{public}s %{public}s %{public}d",
        action, interfaceName, addr, prefixLen);

    ret = netLinker.SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
    if (ret < 0) {
        return -EIO;
    }

    return 0;
}

int InterfaceController::AddAddress(const char *interfaceName, const char *addr, int prefixLen)
{
    return ModifyAddress(RTM_NEWADDR, interfaceName, addr, prefixLen);
}

int InterfaceController::DelAddress(const char *interfaceName, const char *addr, int prefixLen)
{
    return ModifyAddress(RTM_DELADDR, interfaceName, addr, prefixLen);
}
} // namespace nmd
} // namespace OHOS