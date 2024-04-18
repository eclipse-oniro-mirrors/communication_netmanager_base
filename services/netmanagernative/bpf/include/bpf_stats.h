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

#ifndef BPF_STATS_H
#define BPF_STATS_H

#include <vector>
#include <cstdint>
#include <string>

#include "bpf_def.h"
#include "bpf_mapper.h"
#include "net_manager_constants.h"
#include "net_stats_info.h"

namespace OHOS::NetManagerStandard {
enum class StatsType {
    STATS_TYPE_RX_BYTES = 0,
    STATS_TYPE_RX_PACKETS = 1,
    STATS_TYPE_TX_BYTES = 2,
    STATS_TYPE_TX_PACKETS = 3,
};

class NetsysBpfStats {
public:
    NetsysBpfStats() = default;
    ~NetsysBpfStats() = default;

    /**
     * Get the Total Stats
     *
     * @param stats Output traffic data
     * @param type StatsType traffic data type
     * @return returns total stats
     */
    int32_t GetTotalStats(uint64_t &stats, StatsType type);

    /**
     * Get the Uid Stats
     *
     * @param stats Output traffic data
     * @param type StatsType traffic data type
     * @param uid app uid
     * @return returns uid stats
     */
    int32_t GetUidStats(uint64_t &stats, StatsType type, uint32_t uid);

    /**
     * Get the Iface Stats
     *
     * @param stats Output traffic data
     * @param type StatsType traffic data type
     * @param interfaceName iface name
     * @return returns iface stats.
     */
    int32_t GetIfaceStats(uint64_t &stats, StatsType type, const std::string &interfaceName);

    /**
     * Get the container uid Stats
     *
     * @param stats Stats data.
     * @return returns 0 for success other as failed.
     */
    int32_t GetAllContainerStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats);

    /**
     * Get the Iface with uid Stats
     *
     * @param stats Stats data.
     * @return returns 0 for success other as failed.
     */
    int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats);

    int32_t GetCookieStats(uint64_t &stats, StatsType statsType, uint64_t cookie);

private:
    static int32_t GetNumberFromStatsValue(uint64_t &stats, StatsType statsType, const stats_value &value);
};
} // namespace OHOS::NetManagerStandard
#endif // BPF_STATS_H
