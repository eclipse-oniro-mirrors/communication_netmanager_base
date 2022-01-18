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

#ifndef NET_POLICY_CELLULAR_POLICY_H
#define NET_POLICY_CELLULAR_POLICY_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
struct NetPolicyCellularPolicy : public Parcelable {
    /* subscriber ID, get from telephone module */
    int32_t slotId_ = 0;
    /*  Time rubbing, for example:1636598990 */
    int64_t periodStartTime_ = -1;
    /* Unit: Month for example:M1 (The 1st of each month) */
    std::string periodDuration_ = "M1";
    std::string title_ = "";
    std::string summary_ = "";
    /* Limit threshold */
    int64_t limitBytes_ = -1;
    /* Limit action; 1 : prohibit; 2 : charging; 3 : Speed limit */
    int32_t limitAction_ = -1;
    /*  Used traffic */
    int64_t usedBytes_ = -1;
    int64_t usedTimeDuration_ = -1;
    /*  possessor */
    std::string possessor_ = "";

    virtual bool Marshalling(Parcel &parcel) const override;
    static bool Marshalling(Parcel &parcel, const std::vector<NetPolicyCellularPolicy> &cellularPolicies);
    static bool Unmarshalling(Parcel &parcel, std::vector<NetPolicyCellularPolicy> &cellularPolicies);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_CELLULAR_POLICY_H