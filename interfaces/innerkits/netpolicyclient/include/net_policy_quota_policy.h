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

#ifndef NET_POLICY_QUOTA_POLICY_H
#define NET_POLICY_QUOTA_POLICY_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
struct NetPolicyQuotaPolicy : public Parcelable {
    /* netType value in NetBearType */
    int8_t netType_ = -1;
    /* IccId, get from telephone module */
    std::string simId_;
    /*  Time rubbing, for example:1636598990 */
    int64_t periodStartTime_ = -1;
    /* Unit: Month for example:M1 (The 1st of each month) */
    std::string periodDuration_ = "M1";
    /* Alarm threshold */
    int64_t warningBytes_ = -1;
    /* Limit threshold */
    int64_t limitBytes_ = -1;
    /*  Time rubbing, for example:1636598990 */
    int64_t lastLimitSnooze_ = -1;
    /* 1 for metering and 0 for non metering */
    int8_t metered_ = -1;
    /* System application send 1, common application send 0 */
    int8_t source_ = -1;

    virtual bool Marshalling(Parcel &parcel) const override;
    static bool Marshalling(Parcel &parcel, const NetPolicyQuotaPolicy &quotaPolicy);
    static bool Marshalling(Parcel &parcel, const std::vector<NetPolicyQuotaPolicy> &quotaPolicies);
    static bool Unmarshalling(Parcel &parcel, NetPolicyQuotaPolicy &quotaPolicy);
    static bool Unmarshalling(Parcel &parcel, std::vector<NetPolicyQuotaPolicy> &quotaPolicies);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_QUOTA_POLICY_H