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

#include <vector>
#include <net/if.h>

#include "securec.h"
#include "bpf_stats.h"
#include "bpf_path.h"

#include "netnative_log_wrapper.h"

namespace OHOS::NetManagerStandard {
int32_t NetsysBpfStats::GetNumberFromStatsValue(uint64_t &stats, StatsType statsType, stats_value value)
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
            return -1;
    }
    return 0;
}

int32_t NetsysBpfStats::GetTotalStats(uint64_t &stats, StatsType statsType)
{
    stats = 0;
    BpfMapper<iface_stats_key, iface_stats_value> ifaceStatsMap(IFACE_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!ifaceStatsMap.IsValid()) {
        NETNATIVE_LOGE("ifaceStatsMap IsValid");
        return -1;
    }

    iface_stats_value totalStats = {};
    auto keys = ifaceStatsMap.GetAllKeys();
    for (const auto &k : keys) {
        iface_stats_value v = {};
        if (ifaceStatsMap.Read(k, v) < 0) {
            NETNATIVE_LOGE("Read ifaceStatsMap err");
            return -1;
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
    BpfMapper<app_uid_stats_key, app_uid_stats_value> appUidStatsMap(APP_UID_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!appUidStatsMap.IsValid()) {
        return -1;
    }

    app_uid_stats_value uidStats = {0};
    if (appUidStatsMap.Read(uid, uidStats) < 0) {
        return -1;
    }
    return GetNumberFromStatsValue(stats, statsType, uidStats);
}

int32_t NetsysBpfStats::GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    BpfMapper<stats_key, stats_value> uidIfaceStatsMap(APP_UID_IF_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!uidIfaceStatsMap.IsValid()) {
        return -1;
    }

    stats.clear();
    char if_name[IFNAME_SIZE] = {0};
    auto keys = uidIfaceStatsMap.GetAllKeys();
    for (const auto &k : keys) {
        stats_value v = {};
        if (uidIfaceStatsMap.Read(k, v) < 0) {
            NETNATIVE_LOGE("Read ifaceStatsMap err");
            return -1;
        }

        NetStatsInfo tempStats;
        tempStats.uid_ = k.uId;
        if (memset_s(if_name, sizeof(if_name), 0, sizeof(if_name)) != EOK) {
            return -1;
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

    return 0;
}

int32_t NetsysBpfStats::GetIfaceStats(uint64_t &stats, const StatsType statsType, const std::string &interfaceName)
{
    BpfMapper<iface_stats_key, iface_stats_value> ifaceStatsMap(IFACE_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!ifaceStatsMap.IsValid()) {
        return -1;
    }

    auto ifIndex = if_nametoindex(interfaceName.c_str());
    if (ifIndex <= 0) {
        return -1;
    }

    iface_stats_value ifaceStats = {0};
    if (ifaceStatsMap.Read(ifIndex, ifaceStats) < 0) {
        return -1;
    }
    return GetNumberFromStatsValue(stats, statsType, ifaceStats);
}

} // namespace OHOS::NetManagerStandard
