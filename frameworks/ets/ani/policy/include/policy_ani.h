/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef POLICY_ANI_H
#define POLICY_ANI_H

#include "cxx.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

int32_t SetBackgroundAllowed(bool allowed);
int32_t GetBackgroundPolicy(bool &backgroundPolicy);
int32_t GetBackgroundPolicyByUid(int32_t uid, uint32_t &backgroundPolicyOfUid);
int32_t SetPolicyByUid(int32_t uid, uint32_t policy);
int32_t GetPolicyByUid(int32_t uid, uint32_t &policy);
int32_t GetUidsByPolicy(uint32_t policy, rust::Vec<uint32_t> &result);
int32_t IsUidNetAllowed(int32_t uid, bool metered, bool &isAllowed);
int32_t IsUidNetAllowedByIface(int32_t uid, rust::String iface, bool &isAllowed);
int32_t SetNetQuotaPolicies(rust::Vec<NetQuotaPolicyAni> policies);
int32_t RestoreAllPolicies(rust::String iccid);
int32_t GetNetQuotaPolicies(rust::Vec<NetQuotaPolicyAni> &result);
int32_t UpdateRemindPolicy(int32_t netType, rust::String iccid, uint32_t remindType);
int32_t SetDeviceIdleTrustlist(rust::Vec<int32_t> uids, bool isAllowed);
int32_t GetDeviceIdleTrustlist(rust::Vec<uint32_t> &result);
int32_t SetPowerTrustlist(rust::Vec<int32_t> ids, bool allowed);
int32_t GetPowerSaveTrustlist(rust::Vec<uint32_t> &result);
int32_t SetNetworkAccessPolicyTyped(int32_t uid, NetworkAccessPolicyAni policy, bool reconfirmFlag);
int32_t GetNetworkAccessPolicyByUid(int32_t uid, NetworkAccessPolicyAni &result);
int32_t GetAllNetworkAccessPolicies(rust::Vec<UidNetworkPolicyAni> &result);
int32_t GetSelfNetworkAccessPolicy(NetAccessPolicyAni &result);
int32_t ShowAppNetPolicySettings(AniEnv **env, AniObject *context);

rust::String GetErrorCodeAndMessage(int32_t &errorCode);

int32_t RegisterNetPolicyChangeCallback();
int32_t UnregisterNetPolicyChangeCallback();
int32_t CheckPolicyPermission();

} // namespace NetManagerAni
} // namespace OHOS

#endif // POLICY_ANI_H
