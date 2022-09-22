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

#include "netsys_bpf_stats.h"

#include <sys/resource.h>

#include <linux/bpf.h>
#include "securec.h"

#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr const char *IFACE_STATS_MAP_PATH = "/sys/fs/bpf/netsys_iface_stats_map";
static constexpr const char *IFACE_INDEX_NAME_MAP_PATH = "/sys/fs/bpf/netsys_iface_name_map";
static constexpr const char *APP_UID_STATS_MAP_PATH = "/sys/fs/bpf/netsys_app_uid_stats_map";

NetsysBpfStats::NetsysBpfStats() = default;
NetsysBpfStats::~NetsysBpfStats() = default;

bool NetsysBpfStats::IsStatsValueValid(StatsValue value)
{
    return (value.rxBytes >= 0 && value.rxPackets >= 0 && value.txBytes >= 0 && value.txPackets >= 0);
}

int64_t NetsysBpfStats::CastResult(const NetStatsResultCode &code)
{
    if (static_cast<int64_t>(code) < 0) {
        NETNATIVE_LOGE("Error at bpf reader, code %{public}d, errno: %{public}d, err: %{public}s",
                       static_cast<int32_t>(code), errno, strerror(errno));
    }
    // The return value mains the stats data if any error the stats this time read no stats data.
    return 0;
}

int64_t NetsysBpfStats::GetTotalStats(StatsType statsType)
{
    NetsysBpfMap<uint32_t, StatsValue> ifaceStatsMap(IFACE_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!ifaceStatsMap.IsValid()) {
        return CastResult(NetStatsResultCode::ERR_INVALID_IFACE_STATS_MAP);
    }
    return NetsysBpfStats::BpfGetTotalStats(statsType, ifaceStatsMap);
}

int64_t NetsysBpfStats::BpfGetTotalStats(StatsType statsType, const NetsysBpfMap<uint32_t, StatsValue> &ifaceStatsMap)
{
    StatsValue totalStats = {0};
    uint32_t lookupKey, nextKey;
    lookupKey = -1;
    while (ifaceStatsMap.GetNextKeyFromStatsMap(lookupKey, nextKey) == 0) {
        lookupKey = nextKey;
        StatsValue statsValue = ifaceStatsMap.ReadValueFromMap(lookupKey);
        if (!IsStatsValueValid(statsValue)) {
            return CastResult(NetStatsResultCode::ERR_INVALID_STATS_VALUE);
        }
        totalStats += statsValue;
    }
    switch (statsType) {
        case StatsType::STATS_TYPE_RX_BYTES:
            return totalStats.rxBytes;
        case StatsType::STATS_TYPE_RX_PACKETS:
            return totalStats.rxPackets;
        case StatsType::STATS_TYPE_TX_BYTES:
            return totalStats.txBytes;
        case StatsType::STATS_TYPE_TX_PACKETS:
            return totalStats.txPackets;
        default:
            break;
    }
    return CastResult(NetStatsResultCode::ERR_INVALID_STATS_TYPE);
}

int64_t NetsysBpfStats::BpfGetUidStats(StatsType statsType, uint32_t uid,
                                       const NetsysBpfMap<uint32_t, StatsValue> &appUidStatsMap)
{
    StatsValue uidStats = {0};
    StatsValue statsValue = appUidStatsMap.ReadValueFromMap(uid);
    if (!IsStatsValueValid(statsValue)) {
        return CastResult(NetStatsResultCode::ERR_INVALID_STATS_VALUE);
    }
    uidStats += statsValue;
    switch (statsType) {
        case StatsType::STATS_TYPE_RX_BYTES:
            return uidStats.rxBytes;
        case StatsType::STATS_TYPE_RX_PACKETS:
            return uidStats.rxPackets;
        case StatsType::STATS_TYPE_TX_BYTES:
            return uidStats.txBytes;
        case StatsType::STATS_TYPE_TX_PACKETS:
            return uidStats.txPackets;
        default:
            break;
    }
    return CastResult(NetStatsResultCode::ERR_INVALID_STATS_TYPE);
}

int64_t NetsysBpfStats::GetUidStats(StatsType type, uint32_t uid)
{
    NetsysBpfMap<uint32_t, StatsValue> appUidStatsMap(APP_UID_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!appUidStatsMap.IsValid()) {
        return CastResult(NetStatsResultCode::ERR_INVALID_UID_STATS_MAP);
    }
    return BpfGetUidStats(type, uid, appUidStatsMap);
}

int64_t NetsysBpfStats::BpfGetIfaceStats(const StatsType statsType, const std::string &interfaceName,
                                         const NetsysBpfMap<uint32_t, IfaceName> &ifaceNameMap,
                                         const NetsysBpfMap<uint32_t, StatsValue> &ifaceStatsMap)
{
    StatsValue ifaceStats = {0};
    std::string ifName;
    const auto executeIfaceStats = [&interfaceName, &ifaceNameMap, &ifaceStats, &ifName,
                                    this](const uint32_t key,
                                          const NetsysBpfMap<uint32_t, StatsValue> &ifaceStatsMap) -> void {
        if (GetIfaceName(ifaceNameMap, key, ifName)) {
            NETNATIVE_LOGE("Get iface name failed");
            return;
        }
        if (ifName == interfaceName) {
            StatsValue statsValue = ifaceStatsMap.ReadValueFromMap(key);
            if (!IsStatsValueValid(statsValue)) {
                NETNATIVE_LOGE("Error key is %{public}u", key);
                return;
            }
            ifaceStats += statsValue;
        }
    };
    ifaceStatsMap.Iterate(executeIfaceStats);
    switch (statsType) {
        case StatsType::STATS_TYPE_RX_BYTES:
            return ifaceStats.rxBytes;
        case StatsType::STATS_TYPE_RX_PACKETS:
            return ifaceStats.rxPackets;
        case StatsType::STATS_TYPE_TX_BYTES:
            return ifaceStats.txBytes;
        case StatsType::STATS_TYPE_TX_PACKETS:
            return ifaceStats.txPackets;
        default:
            break;
    }
    return CastResult(NetStatsResultCode::ERR_INVALID_STATS_TYPE);
}

int64_t NetsysBpfStats::GetIfaceStats(const StatsType statsType, const std::string &interfaceName)
{
    NetsysBpfMap<uint32_t, IfaceName> ifaceNameMap(IFACE_INDEX_NAME_MAP_PATH, BPF_F_RDONLY);
    if (!ifaceNameMap.IsValid()) {
        return CastResult(NetStatsResultCode::ERR_INVALID_IFACE_NAME_MAP);
    }
    NetsysBpfMap<uint32_t, StatsValue> ifaceStatsMap(IFACE_STATS_MAP_PATH, BPF_F_RDONLY);
    if (!ifaceStatsMap.IsValid()) {
        return CastResult(NetStatsResultCode::ERR_INVALID_IFACE_STATS_MAP);
    }
    return BpfGetIfaceStats(statsType, interfaceName, ifaceNameMap, ifaceStatsMap);
}

bool NetsysBpfStats::GetIfaceName(const NetsysBpfMap<uint32_t, IfaceName> &ifaceNameMap, uint32_t ifaceIndex,
                                  std::string &ifName)
{
    IfaceName ifaceName = ifaceNameMap.ReadValueFromMap(ifaceIndex);
    ifName = ifaceName.name;
    return ifName.empty();
}
} // namespace NetManagerStandard
} // namespace OHOS
