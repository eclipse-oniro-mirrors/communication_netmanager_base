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

#include <vector>
#include <net/if.h>

#include "securec.h"
#include "bpf_stats.h"

#include "netnative_log_wrapper.h"

namespace OHOS::NetManagerStandard {
int32_t NetsysBpfStats::GetNumberFromStatsValue(uint64_t &stats, StatsType statsType, stats_value value)
{
    return 0;
}

int32_t NetsysBpfStats::GetTotalStats(uint64_t &stats, StatsType statsType)
{
    return 0;
}

int32_t NetsysBpfStats::GetUidStats(uint64_t &stats, StatsType statsType, uint32_t uid)
{
    return 0;
}

int32_t NetsysBpfStats::GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    return 0;
}

int32_t NetsysBpfStats::BpfGetAllStatsInfo(const BpfMapper<uint64_t, iface_name> &ifaceNameMap,
                                           const BpfMapper<stats_key, stats_value> &uidIfaceStatsMap,
                                           std::vector<NetStatsInfo> &stats)
{
    return 0;
}

bool NetsysBpfStats::IsStatsValueValid(stats_value value)
{
    return 0;
}

int32_t NetsysBpfStats::GetIfaceStats(uint64_t &stats, const StatsType statsType, const std::string &interfaceName)
{
    return 0;
}

} // namespace OHOS::NetManagerStandard
