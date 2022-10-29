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

#ifndef NETSYS_BPF_STATS_H
#define NETSYS_BPF_STATS_H

#include "bpf_wrappers.h"
#include "net_stats_constants.h"
#include "netsys_bpf_map.h"

namespace OHOS {
namespace NetManagerStandard {
enum class StatsType {
    STATS_TYPE_RX_BYTES = 0,
    STATS_TYPE_RX_PACKETS = 1,
    STATS_TYPE_TX_BYTES = 2,
    STATS_TYPE_TX_PACKETS = 3,
};

class NetsysBpfStats {
public:
    NetsysBpfStats();
    ~NetsysBpfStats();

    /**
     * Get the Total Stats
     *
     * @param type StatsType traffic data type
     * @return returns total stats
     */
    int64_t GetTotalStats(StatsType type);

    /**
     * Get the Uid Stats
     *
     * @param type StatsType traffic data type
     * @param uid app uid
     * @return returns uid stats
     */
    int64_t GetUidStats(StatsType type, uint32_t uid);

    /**
     * Get the Iface Stats
     *
     * @param type StatsType traffic data type
     * @param interfaceName iface name
     * @return returns iface stats.
     */
    int64_t GetIfaceStats(StatsType type, const std::string &interfaceName);

    /**
     * Get the Iface Stats but for test only
     *
     * @param statsType traffic data type
     * @param interfaceName iface name
     * @param ifaceNameMap map storing relationship between ifacename and index
     * @param ifaceStatsMap map storing index ifacename and stats
     * @return returns iface stats
     */
    int64_t BpfGetIfaceStats(const StatsType statsType, const std::string &interfaceName,
                             const NetsysBpfMap<uint32_t, IfaceName> &ifaceNameMap,
                             const NetsysBpfMap<uint32_t, StatsValue> &ifaceStatsMap);
    /**
     * Get the Uid Stat but for test only
     *
     * @param statsType traffic data type
     * @param uid app uid
     * @param appUidStatsMap map storing uid and stats
     * @return returns uid stats
     */
    int64_t BpfGetUidStats(StatsType statsType, uint32_t uid, const NetsysBpfMap<uint32_t, StatsValue> &appUidStatsMap);

    /**
     * For test only
     *
     * @param statsType traffic data type
     * @param ifaceStatsMap map storing index ifacename and stats
     * @return returns uid stats
     */
    int64_t BpfGetTotalStats(StatsType statsType, const NetsysBpfMap<uint32_t, StatsValue> &ifaceStatsMap);

private:
    bool IsStatsValueValid(StatsValue value);
    bool GetIfaceName(const NetsysBpfMap<uint32_t, IfaceName> &ifaceNameMap, uint32_t ifaceIndex,
                      std::string &ifaceName);
    int64_t CastResult(const NetStatsResultCode &code);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETSYS_BPF_STATS_H