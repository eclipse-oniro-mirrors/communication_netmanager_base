/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "net_stats_info.h"

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetStatsInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt64(rxBytes_)) {
        return false;
    }
    if (!parcel.WriteInt64(txBytes_)) {
        return false;
    }
    if (!parcel.WriteInt64(rxPackets_)) {
        return false;
    }
    if (!parcel.WriteInt64(txPackets_)) {
        return false;
    }
    return true;
}

bool NetStatsInfo::Marshalling(Parcel &parcel, const NetStatsInfo &stats)
{
    if (!parcel.WriteInt64(stats.rxBytes_)) {
        return false;
    }
    if (!parcel.WriteInt64(stats.txBytes_)) {
        return false;
    }
    if (!parcel.WriteInt64(stats.rxPackets_)) {
        return false;
    }
    if (!parcel.WriteInt64(stats.txPackets_)) {
        return false;
    }
    return true;
}

bool NetStatsInfo::Unmarshalling(Parcel &parcel, NetStatsInfo &stats)
{
    if (!parcel.ReadInt64(stats.rxBytes_)) {
        return false;
    }
    if (!parcel.ReadInt64(stats.txBytes_)) {
        return false;
    }
    if (!parcel.ReadInt64(stats.rxPackets_)) {
        return false;
    }
    if (!parcel.ReadInt64(stats.txPackets_)) {
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
