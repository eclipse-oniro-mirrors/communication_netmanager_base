/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "traffic_manager.h"
#include <algorithm>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "securec.h"
#include "net_manager_native.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
const std::string interfaceListDir = "/sys/class/net/";

std::vector<std::string> GetInterfaceList()
{
    DIR *dir(nullptr);
    struct dirent *ptr(nullptr);
    std::vector<std::string> ifList;

    dir = opendir(interfaceListDir.c_str());
    if (dir == nullptr) {
        NETNATIVE_LOGI("GetInterfaceList open %{public}s failed", interfaceListDir.c_str());
        return ifList;
    }

    ptr = readdir(dir);
    while (ptr!= nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            ifList.push_back(ptr->d_name);
        }
        ptr = readdir(dir);
    }
    closedir(dir);

    return ifList;
}

long GetInterfaceTrafficByType(const std::string &path, const std::string &type)
{
    if (path.empty()) {
        return -1;
    }

    std::string trafficPath = path + type;

    int fd = open(trafficPath.c_str(), 0, 0666);
    if (fd == -1) {
        NETNATIVE_LOGI("GetInterfaceTrafficByType open %{public}s failed", interfaceListDir.c_str());
        return -1;
    }

    char buf[100] = {0};
    int nread = read(fd, buf, sizeof(long));
    if (nread == -1) {
        NETNATIVE_LOGI("GetInterfaceTrafficByType read %{public}s failed", interfaceListDir.c_str());
        close(fd);
        return -1;
    }
    close(fd);

    return atol(buf);
}

long TrafficManager::GetAllRxTraffic()
{
    std::vector<std::string> ifNameList = GetInterfaceList();
    if (ifNameList.empty()) {
        return 0;
    }

    long allRxBytes = 0;
    for (auto iter = ifNameList.begin(); iter != ifNameList.end(); iter++) {
        if (*iter != "lo") {
            std::string baseTrafficPath = interfaceListDir + (*iter) + "/" + "statistics" + "/";
            long rxBytes = GetInterfaceTrafficByType(baseTrafficPath, "rx_bytes");
            allRxBytes += rxBytes;
        }
    }
    return allRxBytes;
}

long TrafficManager::GetAllTxTraffic()
{
    std::vector<std::string> ifNameList = GetInterfaceList();
    if (ifNameList.empty()) {
        return 0;
    }

    long allTxBytes = 0;
    for (auto iter = ifNameList.begin(); iter != ifNameList.end(); iter++) {
        if (*iter != "lo") {
            std::string baseTrafficPath = interfaceListDir + (*iter) + "/" + "statistics" + "/";
            long txBytes = GetInterfaceTrafficByType(baseTrafficPath, "tx_bytes");
            allTxBytes = allTxBytes + txBytes;
        }
    }
    return allTxBytes;
}

TrafficStatsParcel TrafficManager::GetInterfaceTraffic(const std::string &ifName)
{
    nmd::TrafficStatsParcel interfaceTrafficBytes = {"", 0, 0, 0, 0, 0};
    std::vector<std::string> ifNameList = GetInterfaceList();
    if (ifNameList.empty()) {
        return interfaceTrafficBytes;
    }
    for (auto iter = ifNameList.begin(); iter != ifNameList.end(); iter++) {
        if (ifName != *iter) {
            continue;
        }
        std::string baseTrafficPath = interfaceListDir + (*iter) + "/" + "statistics" + "/";
        long infRxBytes = GetInterfaceTrafficByType(baseTrafficPath, "rx_bytes");
        long infRxPackets = GetInterfaceTrafficByType(baseTrafficPath, "rx_packets");
        long infTxBytes = GetInterfaceTrafficByType(baseTrafficPath, "tx_bytes");
        long infTxPackets = GetInterfaceTrafficByType(baseTrafficPath, "tx_packets");

        interfaceTrafficBytes.iface = ifName;
        interfaceTrafficBytes.ifIndex = if_nametoindex(ifName.c_str());

        interfaceTrafficBytes.rxBytes = infRxBytes == -1 ? 0 : infRxBytes;
        interfaceTrafficBytes.rxPackets = infRxPackets == -1 ? 0 : infRxPackets;
        interfaceTrafficBytes.txBytes = infTxBytes == -1 ? 0 : infTxBytes;
        interfaceTrafficBytes.txPackets = infTxPackets == -1 ? 0 : infTxPackets;
    }
    return interfaceTrafficBytes;
}
} // namespace nmd
} // namespace OHOS
