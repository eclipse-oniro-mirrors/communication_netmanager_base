/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <net/if.h>
#include <vector>

#include "bpf_path.h"
#include "bpf_def.h"
#include "bpf_stats.h"
#include "securec.h"
#include "netnative_log_wrapper.h"

namespace OHOS::NetManagerStandard {
int32_t NetsysBpfStats::GetNumberFromStatsValue(uint64_t &stats, StatsType statsType, const stats_value &value)
{
    switch (statsType) {
        case StatsType::STATS_TYPE_RX_BYTES:
            stats = value.rxBytes;
            break;
        case StatsType::STATS_TYPE_RX_PACKETS:
            stats = value.rxPackets;
            break;
        case StatsType::STATS_TYPE_TX_BYTES:
            stats = value.txBytes;
            break;
        case StatsType::STATS_TYPE_TX_PACKETS:
            stats = value.txPackets;
            break;
        default:
            NETNATIVE_LOGE("invalid StatsType type %{public}d", statsType);
            return NETMANAGER_ERROR;
    }
    return NETSYS_SUCCESS;
}

int32_t NetsysBpfStats::GetTotalStats(uint64_t &stats, StatsType statsType)
{
    stats = 0;
    BpfMapper<iface_stats_key, iface_stats_value> ifaceStatsMap(IFACE_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!ifaceStatsMap.IsValid()) {
        NETNATIVE_LOGE("ifaceStatsMap IsValid");
        return NETMANAGER_ERROR;
    }

    iface_stats_value totalStats = {0};
    auto keys = ifaceStatsMap.GetAllKeys();
    for (const auto &k : keys) {
        iface_stats_value v = {0};
        if (ifaceStatsMap.Read(k, v) < NETSYS_SUCCESS) {
            NETNATIVE_LOGE("Read ifaceStatsMap err");
            return NETMANAGER_ERROR;
        }
        totalStats.rxPackets += v.rxPackets;
        totalStats.rxBytes += v.rxBytes;
        totalStats.txPackets += v.txPackets;
        totalStats.txBytes += v.txBytes;
    }

    return GetNumberFromStatsValue(stats, statsType, totalStats);
}

int32_t NetsysBpfStats::GetUidStats(uint64_t &stats, StatsType statsType, uint32_t uid)
{
    stats = 0;
    BpfMapper<app_uid_stats_key, app_uid_stats_value> appUidStatsMap(APP_UID_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!appUidStatsMap.IsValid()) {
        return NETMANAGER_ERROR;
    }

    app_uid_stats_value uidStats = {0};
    if (appUidStatsMap.Read(uid, uidStats) < 0) {
        return NETMANAGER_ERROR;
    }
    return GetNumberFromStatsValue(stats, statsType, uidStats);
}

int32_t NetsysBpfStats::GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    BpfMapper<stats_key, stats_value> uidIfaceStatsMap(APP_UID_IF_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!uidIfaceStatsMap.IsValid()) {
        return NETMANAGER_ERROR;
    }

    stats.clear();
    char if_name[IFNAME_SIZE] = {0};
    auto keys = uidIfaceStatsMap.GetAllKeys();
    for (const auto &k : keys) {
        stats_value v = {};
        if (uidIfaceStatsMap.Read(k, v) < 0) {
            NETNATIVE_LOGE("Read ifaceStatsMap err");
            return NETMANAGER_ERROR;
        }

        NetStatsInfo tempStats;
        tempStats.uid_ = k.uId;
        if (memset_s(if_name, sizeof(if_name), 0, sizeof(if_name)) != EOK) {
            return NETMANAGER_ERROR;
        }

        char *pName = if_indextoname(k.ifIndex, if_name);
        if (pName != nullptr) {
            tempStats.iface_ = pName;
        }
        tempStats.rxBytes_ = v.rxBytes;
        tempStats.txBytes_ = v.txBytes;
        tempStats.rxPackets_ = v.rxPackets;
        tempStats.txPackets_ = v.txPackets;
        stats.emplace_back(tempStats);
    }

    return NETSYS_SUCCESS;
}

int32_t NetsysBpfStats::GetIfaceStats(uint64_t &stats, const StatsType statsType, const std::string &interfaceName)
{
    stats = 0;
    BpfMapper<iface_stats_key, iface_stats_value> ifaceStatsMap(IFACE_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!ifaceStatsMap.IsValid()) {
        return NETMANAGER_ERROR;
    }

    auto ifIndex = if_nametoindex(interfaceName.c_str());
    if (ifIndex <= 0) {
        return NETMANAGER_ERROR;
    }

    iface_stats_value ifaceStats = {0};
    if (ifaceStatsMap.Read(ifIndex, ifaceStats) < 0) {
        return NETMANAGER_ERROR;
    }
    return GetNumberFromStatsValue(stats, statsType, ifaceStats);
}

} // namespace OHOS::NetManagerStandard
