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

#ifndef NET_STATS_WRAPPER_H
#define NET_STATS_WRAPPER_H

#include <memory>
#include <string>

#include "netsys_bpf_stats.h"

namespace OHOS {
namespace NetManagerStandard {
class NetStatsWrapper {
public:
    ~NetStatsWrapper();

    static NetStatsWrapper &GetInstance();

    /**
     * Get the Total Stats object
     *
     * @param type stats type
     * @return int64_t Total traffic bytes
     */
    int64_t GetTotalStats(StatsType type);

    /**
     * Get the Uid Stat object
     *
     * @param type stats type
     * @param uid  uid
     * @return int64_t traffic bytes for uid
     */
    int64_t GetUidStats(StatsType type, uint32_t uid);

    /**
     * Get the Iface Stat object
     *
     * @param type stats type
     * @param interfaceName iface name
     * @return int64_t traffic bytes for iface
     */
    int64_t GetIfaceStats(StatsType type, const std::string &interfaceName);

private:
    NetStatsWrapper();

private:
    std::unique_ptr<NetsysBpfStats> netSysBpf_ = nullptr;
    NetStatsWrapper(const NetStatsWrapper &) = delete;
    NetStatsWrapper &operator=(const NetStatsWrapper &) = delete;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_WRAPPER_H