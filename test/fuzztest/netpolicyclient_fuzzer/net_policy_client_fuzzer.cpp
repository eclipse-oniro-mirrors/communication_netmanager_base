/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <thread>
#include <net_quota_policy.h>
#include "net_policy_constants.h"
#include "net_policy_client.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
class INetPolicyCallbackTest : public INetPolicyCallback {
public:
    INetPolicyCallbackTest() : INetPolicyCallback() {}
    virtual ~INetPolicyCallbackTest() {}
};

void SetPolicyByUidFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    uint32_t uid = *(reinterpret_cast<const uint32_t*>(data));
    NetUidPolicy policy = NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(uid, policy);
}

void GetPolicyByUidFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    uint32_t uid = *(reinterpret_cast<const uint32_t*>(data));
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetPolicyByUid(uid);
}

void GetUidsByPolicyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    NetUidPolicy policy = NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(policy);
}

void IsUidNetAccessFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    uint32_t uid = *(reinterpret_cast<const uint32_t*>(data));
    bool metered = *(reinterpret_cast<const bool*>(data));
    std::string ifaceName(reinterpret_cast<const char*>(data), size);
    DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(uid, metered);
    DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(uid, ifaceName);
}

void SetBackgroundPolicyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    bool isBackgroundPolicyAllow = *(reinterpret_cast<const bool*>(data));
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(isBackgroundPolicyAllow);
}

void SetFactoryPolicyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string simId(reinterpret_cast<const char*>(data), size);
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetFactoryPolicy(simId);
}

void SetSnoozePolicyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    int8_t netType = *(reinterpret_cast<const int8_t*>(data));
    std::string simId(reinterpret_cast<const char*>(data), size);
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetSnoozePolicy(netType, simId);
}

void GetBackgroundPolicyByUidFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    uint32_t uid = *(reinterpret_cast<const uint32_t*>(data));
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(uid);
}

void SetIdleTrustlistFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    uint32_t uid = *(reinterpret_cast<const uint32_t*>(data));
    bool isTrustlist = *(reinterpret_cast<const bool*>(data));
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetIdleTrustlist(uid, isTrustlist);
}

void SetCellularPoliciesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<NetPolicyCellularPolicy> cellularPolicies;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetCellularPolicies(cellularPolicies);
}

void RegisterNetPolicyCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    sptr<INetPolicyCallbackTest> callback = sptr<INetPolicyCallbackTest>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
}

void UnregisterNetPolicyCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    sptr<INetPolicyCallbackTest> callback = sptr<INetPolicyCallbackTest>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
}

void GetIdleTrustlistFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<uint32_t> uids;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetIdleTrustlist(uids);
}

void GetNetQuotaPoliciesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
}

void SetNetQuotaPoliciesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
}

void GetCellularPoliciesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<NetPolicyCellularPolicy> cellularPolicies;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetCellularPolicies(cellularPolicies);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::SetPolicyByUidFuzzTest(data, size);
    OHOS::NetManagerStandard::GetPolicyByUidFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidsByPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::IsUidNetAccessFuzzTest(data, size);
    OHOS::NetManagerStandard::SetFactoryPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::SetSnoozePolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::GetBackgroundPolicyByUidFuzzTest(data, size);
    OHOS::NetManagerStandard::SetIdleTrustlistFuzzTest(data, size);
    OHOS::NetManagerStandard::SetCellularPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetPolicyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetPolicyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetQuotaPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::SetNetQuotaPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetCellularPoliciesFuzzTest(data, size);
    
    return 0;
}
