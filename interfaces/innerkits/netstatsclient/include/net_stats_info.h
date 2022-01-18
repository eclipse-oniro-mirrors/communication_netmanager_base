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

#ifndef NET_STATS_INFO_H
#define NET_STATS_INFO_H

#include "parcel.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
struct NetStatsInfo : public Parcelable {
    int64_t rxBytes_ = 0;
    int64_t txBytes_ = 0;
    int64_t rxPackets_ = 0;
    int64_t txPackets_ = 0;

    virtual bool Marshalling(Parcel &parcel) const override;
    static bool Marshalling(Parcel &parcel, const NetStatsInfo &stats);
    static bool Unmarshalling(Parcel &parcel, NetStatsInfo &stats);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_INFO_H
