/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <system_error>

#include "bpf_loader.h"
#include "netnative_log_wrapper.h"
#include "netsys_bpf_map.h"

namespace OHOS {
namespace nmd {
namespace {
constexpr const char *ELF_DIR = "/system/etc/bpf/";
constexpr const char *IFACE_NAME_MAP_PATH = "/sys/fs/bpf/netsys_iface_name_map";
constexpr const char *IFACE_STATS_MAP_PATH = "/sys/fs/bpf/netsys_iface_stats_map";
constexpr const char *APP_UID_STATS_MAP_PATH = "/sys/fs/bpf/netsys_app_uid_stats_map";
constexpr const char *APP_UID_IFINDEX_STATS_MAP_PATH = "/sys/fs/bpf/netsys_app_uid_if_stats_map";
constexpr const char *SYS_NET_PATH = "/sys/class/net/";
} // namespace

std::vector<std::string> GetInterfaceNames()
{
    std::vector<std::string> ifaceNames;
    DIR *dir(nullptr);
    struct dirent *de(nullptr);

    dir = opendir(SYS_NET_PATH);
    if (dir == nullptr) {
        NETNATIVE_LOGE("InterfaceManager::GetInterfaceNames opendir fail %{public}d", errno);
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

bool BpfManager::Init() const
{
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

    NetManagerStandard::NetsysBpfMap<uint64_t, NetManagerStandard::IfaceName> ifaceNameMap(IFACE_NAME_MAP_PATH, 0);
    const auto &ifaceList = GetInterfaceNames();
    for (const auto &iface : ifaceList) {
        uint64_t ifaceIndex = if_nametoindex(iface.c_str());
        if (ifaceIndex == 0) {
            NETNATIVE_LOGE("Unknown interface %{public}s", iface.c_str());
            continue;
        }
        NetManagerStandard::IfaceName tempIfaceName;
        if (strcpy_s(tempIfaceName.name, sizeof(tempIfaceName.name), iface.c_str()) != 0) {
            NETNATIVE_LOGE("Failed to add iface %{public}s (%{public}s): errno = %{public}d", iface.c_str(),
                           std::to_string(ifaceIndex).c_str(), errno);
            return false;
        }
        if (!ifaceNameMap.WriteValue(ifaceIndex, tempIfaceName, BPF_ANY)) {
            NETNATIVE_LOGE("Failed to add iface %{public}s (%{public}s): errno = %{public}d", iface.c_str(),
                           std::to_string(ifaceIndex).c_str(), errno);
            return false;
        }
    }

    NETNATIVE_LOGE("BpfManager init success");
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

    if (chmod(APP_UID_IFINDEX_STATS_MAP_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
        NETNATIVE_LOGE("chmod netsys_app_uid_if_stats_map failed: errno = %{public}d", errno);
        return false;
    }
    return true;
}
} // namespace nmd
} // namespace OHOS
