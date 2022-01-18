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

#include "net_policy_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetPolicyQuotaPolicy::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt8(netType_)) {
        return false;
    }
    if (!parcel.WriteInt32(slotId_)) {
        return false;
    }
    if (!parcel.WriteInt64(periodStartTime_)) {
        return false;
    }
    if (!parcel.WriteString(periodDuration_)) {
        return false;
    }
    if (!parcel.WriteInt64(warningBytes_)) {
        return false;
    }
    if (!parcel.WriteInt64(limitBytes_)) {
        return false;
    }
    if (!parcel.WriteInt64(lastLimitSnooze_)) {
        return false;
    }
    if (!parcel.WriteInt8(metered_)) {
        return false;
    }
    if (!parcel.WriteInt8(source_)) {
        return false;
    }

    return true;
}

bool NetPolicyQuotaPolicy::Marshalling(Parcel &parcel, const NetPolicyQuotaPolicy &quotaPolicy)
{
    quotaPolicy.Marshalling(parcel);
    return true;
}

bool NetPolicyQuotaPolicy::Marshalling(Parcel &parcel, const std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    uint32_t vsize = static_cast<uint32_t>(quotaPolicies.size());
    if (!parcel.WriteInt32(vsize)) {
        return false;
    }

    for (uint32_t i = 0; i < vsize; ++i) {
        quotaPolicies[i].Marshalling(parcel);
    }

    return true;
}

bool NetPolicyQuotaPolicy::Unmarshalling(Parcel &parcel, NetPolicyQuotaPolicy &quotaPolicy)
{
    if (!parcel.ReadInt8(quotaPolicy.netType_)) {
        return false;
    }
    if (!parcel.ReadInt32(quotaPolicy.slotId_)) {
        return false;
    }
    if (!parcel.ReadInt64(quotaPolicy.periodStartTime_)) {
        return false;
    }
    if (!parcel.ReadString(quotaPolicy.periodDuration_)) {
        return false;
    }
    if (!parcel.ReadInt64(quotaPolicy.warningBytes_)) {
        return false;
    }
    if (!parcel.ReadInt64(quotaPolicy.limitBytes_)) {
        return false;
    }
    if (!parcel.ReadInt64(quotaPolicy.lastLimitSnooze_)) {
        return false;
    }
    if (!parcel.ReadInt8(quotaPolicy.metered_)) {
        return false;
    }
    if (!parcel.ReadInt8(quotaPolicy.source_)) {
        return false;
    }

    return true;
}

bool NetPolicyQuotaPolicy::Unmarshalling(Parcel &parcel, std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    uint32_t vSize = 0;
    if (!parcel.ReadUint32(vSize)) {
        return false;
    }

    NetPolicyQuotaPolicy quotaPolicyTmp;
    for (uint32_t i = 0; i < vSize; i++) {
        if (!parcel.ReadInt8(quotaPolicyTmp.netType_)) {
            return false;
        }
        if (!parcel.ReadInt32(quotaPolicyTmp.slotId_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.periodStartTime_)) {
            return false;
        }
        if (!parcel.ReadString(quotaPolicyTmp.periodDuration_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.warningBytes_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.limitBytes_)) {
            return false;
        }
        if (!parcel.ReadInt64(quotaPolicyTmp.lastLimitSnooze_)) {
            return false;
        }
        if (!parcel.ReadInt8(quotaPolicyTmp.metered_)) {
            return false;
        }
        if (!parcel.ReadInt8(quotaPolicyTmp.source_)) {
            return false;
        }
        quotaPolicies.push_back(quotaPolicyTmp);
    }

    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS