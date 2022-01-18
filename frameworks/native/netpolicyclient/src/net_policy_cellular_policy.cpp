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
#include "net_policy_cellular_policy.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetPolicyCellularPolicy::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(slotId_)) {
        return false;
    }
    if (!parcel.WriteInt64(periodStartTime_)) {
        return false;
    }
    if (!parcel.WriteString(periodDuration_)) {
        return false;
    }
    if (!parcel.WriteString(title_)) {
        return false;
    }
    if (!parcel.WriteString(summary_)) {
        return false;
    }
    if (!parcel.WriteInt64(limitBytes_)) {
        return false;
    }
    if (!parcel.WriteInt32(limitAction_)) {
        return false;
    }
    if (!parcel.WriteInt64(usedBytes_)) {
        return false;
    }
    if (!parcel.WriteInt64(usedTimeDuration_)) {
        return false;
    }
    if (!parcel.WriteString(possessor_)) {
        return false;
    }

    return true;
}

bool NetPolicyCellularPolicy::Marshalling(Parcel &parcel, const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    uint32_t vsize = static_cast<uint32_t>(cellularPolicies.size());
    if (!parcel.WriteInt32(vsize)) {
        return false;
    }

    for (uint32_t i = 0; i < vsize; ++i) {
        cellularPolicies[i].Marshalling(parcel);
    }

    return true;
}

bool NetPolicyCellularPolicy::Unmarshalling(Parcel &parcel, std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    uint32_t vSize = 0;
    if (!parcel.ReadUint32(vSize)) {
        return false;
    }

    NetPolicyCellularPolicy quotaPolicyTmp;
    for (uint32_t i = 0; i < vSize; i++) {
        if (!parcel.ReadInt32(quotaPolicyTmp.slotId_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.periodStartTime_)) {
            return false;
        }
        if (!parcel.ReadString(quotaPolicyTmp.periodDuration_)) {
            return false;
        }
        if (!parcel.ReadString(quotaPolicyTmp.title_)) {
            return false;
        }
        if (!parcel.ReadString(quotaPolicyTmp.summary_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.limitBytes_)) {
            return false;
        }
        if (!parcel.ReadInt32(quotaPolicyTmp.limitAction_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.usedBytes_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.usedTimeDuration_)) {
            return false;
        }
        if (!parcel.ReadString(quotaPolicyTmp.possessor_)) {
            return false;
        }
        cellularPolicies.push_back(quotaPolicyTmp);
    }

    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS