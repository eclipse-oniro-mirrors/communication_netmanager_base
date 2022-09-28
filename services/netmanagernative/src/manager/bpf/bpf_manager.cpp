/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "bpf_manager.h"

#include <cerrno>
#include <net/if.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include "bpf_loader.h"
#include "interface_manager.h"
#include "iptables_wrapper.h"
#include "netnative_log_wrapper.h"
#include "netsys_bpf_map.h"

namespace OHOS {
namespace nmd {
const char *ELF_DIR = "/system/etc/bpf/";
const char *MOUNT_BPF_FS = "mount -t bpf /sys/fs/bpf /sys/fs/bpf";
const char *MOUNT_CGROUP2_FS = "mount -t cgroup2 none /sys/fs/cgroup";

const char *IFACE_NAME_MAP_PATH = "/sys/fs/bpf/netsys_iface_name_map";
const char *IFACE_STATS_MAP_PATH = "/sys/fs/bpf/netsys_iface_stats_map";
const char *APP_UID_STATS_MAP_PATH = "/sys/fs/bpf/netsys_app_uid_stats_map";

sptr<IRemoteObject> BpfManager::IfacelistNotifyCallback::AsObject()
{
    return nullptr;
}

int32_t BpfManager::IfacelistNotifyCallback::OnInterfaceAddressUpdated(const std::string &addr,
                                                                       const std::string &ifName, int flags, int scope)
{
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnInterfaceAddressRemoved(const std::string &addr,
                                                                       const std::string &ifName, int flags, int scope)
{
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnInterfaceAdded(const std::string &ifName)
{
    NetManagerStandard::NetsysBpfMap<uint32_t, std::string> ifaceNameMap(IFACE_NAME_MAP_PATH, 0);
    uint32_t ifaceIndex = if_nametoindex(ifName.c_str());
    NETNATIVE_LOGI("ifaceIndex = %{public}u, ifaceName = %{public}s", ifaceIndex, ifName.c_str());

    if (ifaceIndex == 0) {
        NETNATIVE_LOGE("Unknown interface %{public}s (%{public}u)", ifName.c_str(), ifaceIndex);
        return -1;
    }

    if (!ifaceNameMap.WriteValue(ifaceIndex, ifName, BPF_ANY)) {
        NETNATIVE_LOGE("Failed to add iface %{public}s (%{public}u): errno = %{public}d", ifName.c_str(), ifaceIndex,
                       errno);
        return -1;
    }
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnInterfaceRemoved(const std::string &ifName)
{
    NetManagerStandard::NetsysBpfMap<uint32_t, std::string> ifaceNameMap(IFACE_NAME_MAP_PATH, 0);
    uint32_t ifaceIndex = if_nametoindex(ifName.c_str());
    NETNATIVE_LOGI("ifaceIndex = %{public}u, ifaceName = %{public}s", ifaceIndex, ifName.c_str());

    if (ifaceIndex == 0) {
        NETNATIVE_LOGE("Unknown interface %{public}s (%{public}u)", ifName.c_str(), ifaceIndex);
        return -1;
    }

    if (!ifaceNameMap.DeleteEntryFromMap(ifaceIndex)) {
        NETNATIVE_LOGE("Failed to remove iface %{public}s (%{public}u): errno = %{public}d", ifName.c_str(), ifaceIndex,
                       errno);
        return -1;
    }
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnInterfaceChanged(const std::string &ifName, bool up)
{
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
{
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnRouteChanged(bool updated, const std::string &route,
                                                            const std::string &gateway, const std::string &ifName)
{
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnDhcpSuccess(sptr<NetsysNative::DhcpResultParcel> &dhcpResult)
{
    return 0;
}

int32_t BpfManager::IfacelistNotifyCallback::OnBandwidthReachedLimit(const std::string &limitName,
                                                                     const std::string &iface)
{
    return 0;
}

bool BpfManager::Init() const
{
    if (system(MOUNT_BPF_FS) < 0) {
        NETNATIVE_LOGE("mount bpf fs failed: errno = %{public}d", errno);
        return false;
    }
    if (system(MOUNT_CGROUP2_FS) < 0) {
        NETNATIVE_LOGE("mount cgroup2 fs failed: errno = %{public}d", errno);
        return false;
    }

    rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
    setrlimit(RLIMIT_MEMLOCK, &r);

    if (!OHOS::Bpf::BpfLoader::GetInstance().HandleElfFiles(ELF_DIR)) {
        NETNATIVE_LOGE("Failed to handle bpf programs!");
        return false;
    }

    if (!ModifyMapPermission()) {
        NETNATIVE_LOGE("Failed to modify the maps permission!");
        return false;
    }

    NetManagerStandard::NetsysBpfMap<uint32_t, std::string> ifaceNameMap(IFACE_NAME_MAP_PATH, 0);
    const auto &ifaceList = InterfaceManager::GetInterfaceNames();
    for (const auto &iface : ifaceList) {
        uint32_t ifaceIndex = if_nametoindex(iface.c_str());
        NETNATIVE_LOGI("ifaceIndex = %{public}u, ifaceName = %{public}s", ifaceIndex, iface.c_str());

        if (ifaceIndex == 0) {
            NETNATIVE_LOGE("Unknown interface %{public}s (%{public}u)", iface.c_str(), ifaceIndex);
            continue;
        }

        if (!ifaceNameMap.WriteValue(ifaceIndex, iface, BPF_ANY)) {
            NETNATIVE_LOGE("Failed to add iface %{public}s (%{public}u): errno = %{public}d", iface.c_str(), ifaceIndex,
                           errno);
            return false;
        }
    }

    if (!CreateIptablesChain()) {
        NETNATIVE_LOGE("Failed to create iptables chain!");
        return false;
    }

    sptr<NetsysNative::INotifyCallback> cb = new BpfManager::IfacelistNotifyCallback;
    if (NetlinkManager::RegisterNetlinkCallback(cb)) {
        NETNATIVE_LOGE("Failed to register netlink callback!");
        return false;
    }
    return true;
}

bool BpfManager::ModifyMapPermission() const
{
    if (chmod(IFACE_NAME_MAP_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
        NETNATIVE_LOGE("chmod netsys_iface_name_map failed: errno = %{public}d", errno);
        return false;
    }

    if (chmod(IFACE_STATS_MAP_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
        NETNATIVE_LOGE("chmod netsys_iface_stats_map failed: errno = %{public}d", errno);
        return false;
    }

    if (chmod(APP_UID_STATS_MAP_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
        NETNATIVE_LOGE("chmod netsys_app_uid_stats_map failed: errno = %{public}d", errno);
        return false;
    }

    return true;
}

bool BpfManager::CreateIptablesChain() const
{
    std::string command;

    command = "-t raw -P PREROUTING";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create raw PREROUTING failed");
        return false;
    }

    command = "-t raw -N bw_raw_PREROUTING";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create raw bw_raw_PREROUTING failed");
        return false;
    }

    command = "-t raw -A PREROUTING -j bw_raw_PREROUTING";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create raw PREROUTING and bw_raw_PREROUTING failed");
        return false;
    }

    command = "-t raw -A bw_raw_PREROUTING -m bpf --object-pinned /sys/fs/bpf/prog_netsys_socket_iface_ingress";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create prog_netsys_socket_iface_ingress chain failed");
        return false;
    }

    command = "-t mangle -P POSTROUTING";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create mangle POSTROUTING failed");
        return false;
    }

    command = "-t mangle -N bw_mangle_POSTROUTING";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create mangle bw_mangle_POSTROUTING failed");
        return false;
    }

    command = "-t mangle -A POSTROUTING -j bw_mangle_POSTROUTING";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create mangle POSTROUTING and bw_mangle_POSTROUTING failed");
        return false;
    }

    command = "-t mangle -A bw_mangle_POSTROUTING -m bpf --object-pinned /sys/fs/bpf/prog_netsys_socket_iface_egress";
    if (DelayedSingleton<IptablesWrapper>::GetInstance()->RunCommand(IPTYPE_IPV4, command)) {
        NETNATIVE_LOGE("create prog_netsys_socket_iface_egress chain failed");
        return false;
    }

    return true;
}
} // namespace nmd
} // namespace OHOS
