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
#include "net_policy_firewall.h"

#include "ipc_skeleton.h"

#include "net_settings.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyFirewall::NetPolicyFirewall(sptr<NetPolicyFile> netPolicyFile) : netPolicyFile_(netPolicyFile)
{
}

bool NetPolicyFirewall::GetBackgroundPolicyByUid(uint32_t uid)
{
    if (NetSettings::GetInstance().IsSystem(uid)) {
        return true;
    }

    NetUidPolicy policy = netPolicyFile_->GetUidPolicy(uid);
    if ((static_cast<uint32_t>(policy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL)) ||
        (static_cast<uint32_t>(policy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED)) ||
        (static_cast<uint32_t>(policy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED))) {
        return true;
    }

    if (netPolicyFile_->GetBackgroundPolicy()) {
        return true;
    } else if (static_cast<uint32_t>(policy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND)) {
        return false;
    } else if (static_cast<uint32_t>(policy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND)) {
        return true;
    }

    return false;
}

bool NetPolicyFirewall::GetCurrentBackgroundPolicy()
{
    uint32_t uid = IPCSkeleton::GetCallingUid();
    return GetBackgroundPolicyByUid(uid);
}
} // namespace NetManagerStandard
} // namespace OHOS
