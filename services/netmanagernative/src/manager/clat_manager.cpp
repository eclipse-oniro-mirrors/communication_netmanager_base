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

#include <algorithm>
#include <map>
#include <net/if.h>
#include <sys/socket.h>
#include <tuple>
#include <utility>

#include "clat_constants.h"
#include "clat_manager.h"
#include "clat_utils.h"
#include "clatd.h"
#include "fwmark.h"
#include "net_manager_constants.h"
#include "net_manager_native.h"
#include "netnative_log_wrapper.h"
#include "network_permission.h"

namespace OHOS {
namespace nmd {
using namespace OHOS::NetManagerStandard;
ClatManager::ClatManager() = default;

int32_t ClatManager::ClatStart(const std::string &v6Iface, int32_t netId, const std::string &nat64PrefixStr,
                               NetManagerNative *netsysService)
{
    NETNATIVE_LOGI("Start Clatd on %{public}s", v6Iface.c_str());
    if (clatdTrackers_.find(v6Iface) != clatdTrackers_.end()) {
        NETNATIVE_LOGW("Clatd is already running on %{public}s", v6Iface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    if (netsysService == nullptr) {
        NETNATIVE_LOGW("NetManagerNative pointer is null");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    uint32_t fwmark = GetFwmark(netId);
    INetAddr v4Addr;
    INetAddr v6Addr;
    int32_t ret = GenerateClatSrcAddr(v6Iface, fwmark, nat64PrefixStr, v4Addr, v6Addr);
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGW("Fail to get source addresses for clat");
        return ret;
    }

    int tunFd = -1;
    std::string tunIface = std::string(CLAT_PREFIX) + v6Iface;
    ret = CreateAndConfigureTunIface(v6Iface, tunIface, v4Addr, netsysService, tunFd);
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGW("Fail to create and configure tun interface for clat");
        return ret;
    }

    int readSock6 = -1;
    int writeSock6 = -1;
    ret = CreateAndConfigureClatSocket(v6Iface, v6Addr, fwmark, readSock6, writeSock6);
    if (ret != NETMANAGER_SUCCESS) {
        close(tunFd);
        NETNATIVE_LOGW("Fail to create and configure read/write sockets for clat");
        return ret;
    }

    clatds_.emplace(
        std::piecewise_construct, std::forward_as_tuple(v6Iface),
        std::forward_as_tuple(tunFd, readSock6, writeSock6, v6Iface, nat64PrefixStr, v4Addr.address_, v6Addr.address_));
    clatds_[v6Iface].Start();

    ret = RouteManager::AddClatTunInterface(tunIface, DEFAULT_V4_ADDR, v4Addr.address_);
    if (ret != NETMANAGER_SUCCESS) {
        close(tunFd);
        close(readSock6);
        close(writeSock6);
        NETNATIVE_LOGW("Add route on %{public}s failed", tunIface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    netsysService->SetClatDnsEnableIpv4(netId, true);
    clatdTrackers_[v6Iface] = {v6Iface, tunIface, v4Addr, v6Addr, nat64PrefixStr, tunFd, readSock6, writeSock6, netId};

    return NETMANAGER_SUCCESS;
}

int32_t ClatManager::ClatStop(const std::string &v6Iface, NetManagerNative *netsysService)
{
    NETNATIVE_LOGI("Stop Clatd on %{public}s", v6Iface.c_str());
    if (clatdTrackers_.find(v6Iface) == clatdTrackers_.end()) {
        NETNATIVE_LOGW("Clatd has not started on %{public}s", v6Iface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    if (netsysService == nullptr) {
        NETNATIVE_LOGW("NetManagerNative pointer is null");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    NETNATIVE_LOGI("Stopping clatd on %{public}s", v6Iface.c_str());
    netsysService->SetClatDnsEnableIpv4(clatdTrackers_[v6Iface].netId, false);
    RouteManager::RemoveClatTunInterface(clatdTrackers_[v6Iface].tunIface);

    clatds_[v6Iface].Stop();
    clatds_.erase(v6Iface);

    FreeTunV4Addr(clatdTrackers_[v6Iface].v4Addr.address_);

    close(clatdTrackers_[v6Iface].tunFd);
    close(clatdTrackers_[v6Iface].readSock6);
    close(clatdTrackers_[v6Iface].writeSock6);
    clatdTrackers_.erase(v6Iface);

    NETNATIVE_LOGI("clatd on %{public}s stopped", v6Iface.c_str());
    return NETMANAGER_SUCCESS;
}

uint32_t ClatManager::GetFwmark(int32_t netId)
{
    Fwmark mark;
    mark.netId = static_cast<uint16_t>(netId);
    mark.explicitlySelected = true;
    mark.protectedFromVpn = true;
    NetworkPermission permission = NetworkPermission::PERMISSION_SYSTEM;
    mark.permission = permission;
    mark.uidBillingDone = false;
    return mark.intValue;
}

int32_t ClatManager::GenerateClatSrcAddr(const std::string &v6Iface, uint32_t fwmark, const std::string &nat64PrefixStr,
                                         INetAddr &v4Addr, INetAddr &v6Addr)
{
    std::string v4AddrStr;
    int32_t ret = SelectIpv4Address(std::string(INIT_V4ADDR_STRING), INIT_V4ADDR_PREFIX_BIT_LEN, v4AddrStr);
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGW("no IPv4 addresses were available for clat");
        return ret;
    }
    v4Addr.type_ = INetAddr::IPV4;
    v4Addr.family_ = AF_INET;
    v4Addr.address_ = v4AddrStr;

    std::string v6AddrStr;
    ret = GenerateIpv6Address(v6Iface, v4AddrStr, nat64PrefixStr, fwmark, v6AddrStr);
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGW("no IPv6 addresses were available for clat");
        return ret;
    }
    v6Addr.type_ = INetAddr::IPV6;
    v6Addr.family_ = AF_INET6;
    v6Addr.address_ = v6AddrStr;

    return NETMANAGER_SUCCESS;
}

int32_t ClatManager::CreateAndConfigureTunIface(const std::string &v6Iface, const std::string &tunIface,
                                                const INetAddr &v4Addr, NetManagerNative *netsysService, int &tunFd)
{
    int32_t ret = CreateTunInterface(tunIface, tunFd);
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGW("Create tun interface %{public}s failed", tunIface.c_str());
        return ret;
    }

    uint32_t tunIfIndex = if_nametoindex(tunIface.c_str());
    if (tunIfIndex == INVALID_IFINDEX) {
        close(tunFd);
        NETNATIVE_LOGW("Fail to get interface index for interface %{public}s", tunIface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    ret = netsysService->SetEnableIpv6(tunIface, 0, false);
    if (ret != NETMANAGER_SUCCESS) {
        close(tunFd);
        NETNATIVE_LOGW("SetEnableIpv6 on %{public}s failed", tunIface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    int mtu = CLAT_IPV6_MIN_MTU - MTU_DELTA;
    ret = netsysService->SetInterfaceMtu(tunIface, mtu);
    if (ret != NETMANAGER_SUCCESS) {
        close(tunFd);
        NETNATIVE_LOGW("Set MTU on %{public}s failed", tunIface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    int v4AddrPrefixLen = V4ADDR_BIT_LEN;

    OHOS::nmd::InterfaceConfigurationParcel ifConfig;
    ifConfig.ifName = tunIface;
    ifConfig.hwAddr = "";
    ifConfig.ipv4Addr = v4Addr.address_;
    ifConfig.prefixLength = v4AddrPrefixLen;

    ifConfig.flags.emplace_back(IFACE_LINK_UP);
    netsysService->SetInterfaceConfig(ifConfig);

    ret = SetTunInterfaceAddress(tunIface, v4Addr.address_, v4AddrPrefixLen);
    if (ret != NETMANAGER_SUCCESS) {
        close(tunFd);
        NETNATIVE_LOGW("Set tun interface address on %{public}s failed", tunIface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    return NETMANAGER_SUCCESS;
}

int32_t ClatManager::CreateAndConfigureClatSocket(const std::string &v6Iface, const INetAddr &v6Addr, uint32_t fwmark,
                                                  int &readSock6, int &writeSock6)
{
    int32_t ret = OpenPacketSocket(readSock6);
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGW("Open packet socket failed");
        return ret;
    }

    ret = OpenRawSocket6(fwmark, writeSock6);
    if (ret != NETMANAGER_SUCCESS) {
        close(readSock6);
        NETNATIVE_LOGW("Open raw socket failed");
        return ret;
    }

    uint32_t v6IfIndex = if_nametoindex(v6Iface.c_str());
    if (v6IfIndex == INVALID_IFINDEX) {
        close(readSock6);
        close(writeSock6);
        NETNATIVE_LOGW("Fail to get interface index for interface %{public}s", v6Iface.c_str());
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    ret = ConfigureWriteSocket(writeSock6, v6Iface);
    if (ret != NETMANAGER_SUCCESS) {
        close(readSock6);
        close(writeSock6);
        NETNATIVE_LOGW("Configure write sockopt failed");
        return ret;
    }

    ret = ConfigureReadSocket(readSock6, v6Addr.address_, v6IfIndex);
    if (ret != NETMANAGER_SUCCESS) {
        close(readSock6);
        close(writeSock6);
        NETNATIVE_LOGW("Configure read socket failed");
        return ret;
    }

    return NETMANAGER_SUCCESS;
}
} // namespace nmd
} // namespace OHOS