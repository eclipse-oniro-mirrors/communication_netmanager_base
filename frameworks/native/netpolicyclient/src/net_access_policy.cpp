
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

#include "net_access_policy.h"

#include <ctime>

#include "parcel.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NetworkAccessPolicy::Marshalling(Parcel &parcel, AccessPolicySave& policies, bool flag)
{
    if (flag) {
        if (!parcel.WriteBool(policies.policy.wifiAllow)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }

        if (!parcel.WriteBool(policies.policy.cellularAllow)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    } else {
        if (!parcel.WriteUint32(policies.uid_policies.size())) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }

        for (const auto &policies : policies.uid_policies) {
            if (!parcel.WriteInt32(policies.first)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
            if (!parcel.WriteBool(policies.second.wifiAllow)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
            if (!parcel.WriteBool(policies.second.cellularAllow)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetworkAccessPolicy::Unmarshalling(Parcel &parcel, AccessPolicySave& policies, bool flag)
{
    if (flag) {
        NetworkAccessPolicy accessPolicyTmp;
        if (!parcel.ReadBool(accessPolicyTmp.wifiAllow)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }

        if (!parcel.ReadBool(accessPolicyTmp.cellularAllow)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        policies.policy = accessPolicyTmp;
    } else {
        uint32_t size = 0;
        if (!parcel.ReadUint32(size)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }

        for (uint32_t i = 0; i < size; ++i) {
            NetworkAccessPolicy tmp_policy;
            uint32_t uid;
            if (!parcel.ReadUint32(uid)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
            if (!parcel.ReadBool(tmp_policy.wifiAllow)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }

            if (!parcel.ReadBool(tmp_policy.cellularAllow)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
            policies.uid_policies.insert(std::pair<int32_t, NetworkAccessPolicy>(uid, tmp_policy));
        }
    }

    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
