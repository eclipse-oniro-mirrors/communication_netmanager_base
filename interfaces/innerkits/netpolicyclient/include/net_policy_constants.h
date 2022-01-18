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

#ifndef NET_POLICY_CONSTANTS_H
#define NET_POLICY_CONSTANTS_H

namespace OHOS {
namespace NetManagerStandard {
enum class NetPolicyResultCode {
    ERR_NONE = 0,
    ERR_INTERNAL_ERROR = (-1),
    ERR_INVALID_UID = (-10001),
    ERR_INVALID_POLICY = (-10002),
    ERR_INVALID_QUOTA_POLICY = (-10003),
    ERR_QUOTA_POLICY_NOT_EXIST = (-10004),
};

enum class NetUidPolicy {
    NET_POLICY_NONE = 0,
    NET_POLICY_ALLOW_METERED_BACKGROUND = 1 << 0,
    NET_POLICY_TEMPORARY_ALLOW_METERED = 1 << 1,
    NET_POLICY_REJECT_METERED_BACKGROUND = 1 << 2,
    NET_POLICY_ALLOW_METERED = 1 << 3,
    NET_POLICY_REJECT_METERED = 1 << 4,
    NET_POLICY_ALLOW_ALL = 1 << 5,
    NET_POLICY_REJECT_ALL = 1 << 6,
};

enum class NetBackgroundPolicy {
    NET_BACKGROUND_POLICY_NONE = 0,
    // Indicates that applications can use metered networks.
    NET_BACKGROUND_POLICY_DISABLE = 1,
    // Indicates that only applications in the allowlist can use metered networks.
    NET_BACKGROUND_POLICY_ALLOWLISTED = 2,
    // Indicates that applications cannot use metered networks.
    NET_BACKGROUND_POLICY_ENABLED = 3,
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_CONSTANTS_H
