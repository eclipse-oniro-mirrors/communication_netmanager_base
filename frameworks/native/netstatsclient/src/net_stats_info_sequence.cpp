/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "net_stats_info_sequence.h"
#include "net_mgr_log_wrapper.h"
#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr uint32_t STATS_INFO_MAX_SIZE = 5000;
bool NetStatsInfoSequence::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint64(startTime_)) {
        return false;
    }
    if (!parcel.WriteUint64(endTime_)) {
        return false;
    }
    return NetStatsInfo::Marshalling(parcel, info_);
}

bool NetStatsInfoSequence::Marshalling(Parcel &parcel, const NetStatsInfoSequence &statsSequence)
{
    if (!parcel.WriteUint64(statsSequence.startTime_)) {
        return false;
    }
    if (!parcel.WriteUint64(statsSequence.endTime_)) {
        return false;
    }
    return NetStatsInfo::Marshalling(parcel, statsSequence.info_);
}

bool NetStatsInfoSequence::Marshalling(Parcel &parcel, const std::vector<NetStatsInfoSequence> &statsSequence)
{
    uint32_t vSize = statsSequence.size();
    if (vSize > STATS_INFO_MAX_SIZE) {
        NETMGR_LOG_E("Size of the statsInfoSequence exceeds maximum.");
        return false;
    }
    if (!parcel.WriteUint32(vSize)) {
        return false;
    }
    std::for_each(statsSequence.begin(), statsSequence.end(),
                  [&parcel](const NetStatsInfoSequence &info) { info.Marshalling(parcel); });
    return true;
}

bool NetStatsInfoSequence::Unmarshalling(Parcel &parcel, NetStatsInfoSequence &statsSequence)
{
    if (!parcel.ReadUint64(statsSequence.startTime_)) {
        return false;
    }
    if (!parcel.ReadUint64(statsSequence.endTime_)) {
        return false;
    }
    return NetStatsInfo::Unmarshalling(parcel, statsSequence.info_);
}

bool NetStatsInfoSequence::Unmarshalling(Parcel &parcel, std::vector<NetStatsInfoSequence> &statsSequence)
{
    uint32_t vSize = 0;
    if (!parcel.ReadUint32(vSize)) {
        return false;
    }
    if (vSize > STATS_INFO_MAX_SIZE) {
        NETMGR_LOG_E("Size of the statsInfoSequence exceeds maximum.");
        return false;
    }
    statsSequence.reserve(vSize);
    for (uint32_t i = 0; i < vSize; i++) {
        NetStatsInfoSequence tmp;
        NetStatsInfoSequence::Unmarshalling(parcel, tmp);
        statsSequence.push_back(tmp);
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS