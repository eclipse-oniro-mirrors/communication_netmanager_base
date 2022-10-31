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

#include <securec.h>
#include <thread>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "net_mgr_log_wrapper.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms = {.userID = 1,
                               .bundleName = "net_policy_client_fuzzer",
                               .instIndex = 0,
                               .appIDDesc = "test"};

PermissionDef testPermDef = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                             .bundleName = "net_policy_client_fuzzer",
                             .grantMode = 1,
                             .availableLevel = APL_SYSTEM_BASIC,
                             .label = "label",
                             .labelId = 1,
                             .description = "Test net policy connectivity internal",
                             .descriptionId = 1};

PermissionStateFull testState = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                 .isGeneral = true,
                                 .resDeviceID = {"local"},
                                 .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                 .grantFlags = {2}
                                 };

HapPolicyParams testPolicyPrams = {.apl = APL_SYSTEM_BASIC,
                                   .domain = "test.domain",
                                   .permList = {testPermDef},
                                   .permStateList = {testState}};
} // namespace

template <class T> T GetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

std::string GetStringFromData(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = GetData<char>();
    }
    std::string str(cstr);
    return str;
}

class AccessToken {
public:
    AccessToken()
    {
        currentID_ = GetSelfTokenID();
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};

class INetPolicyCallbackTest : public INetPolicyCallback {
public:
    INetPolicyCallbackTest() : INetPolicyCallback() {}
    virtual ~INetPolicyCallbackTest() {}
};

void SetPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    uint32_t uid = GetData<uint32_t>();
    NetUidPolicy policy = NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(uid, policy);
}

void GetPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    uint32_t uid = GetData<uint32_t>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetPolicyByUid(uid);
}

void GetUidsByPolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    AccessToken token;
    NetUidPolicy policy = NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(policy);
}

void IsUidNetAccessFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    uint32_t uid = GetData<uint32_t>();
    bool metered = GetData<bool>();
    std::string ifaceName = GetStringFromData(STR_LEN);
    DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(uid, metered);
    DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(uid, ifaceName);
}

void SetBackgroundPolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    bool isBackgroundPolicyAllow = GetData<bool>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(isBackgroundPolicyAllow);
}

void SetFactoryPolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    std::string simId = GetStringFromData(STR_LEN);
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetFactoryPolicy(simId);
}

void SetSnoozePolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    int8_t netType = GetData<int8_t>();
    std::string simId(reinterpret_cast<const char *>(data), size);
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetSnoozePolicy(netType, simId);
}

void GetBackgroundPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    uint32_t uid = GetData<uint32_t>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(uid);
}

void SetIdleTrustlistFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    uint32_t uid = GetData<uint32_t>();
    bool isTrustlist = *(reinterpret_cast<const bool *>(data));
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetIdleTrustlist(uid, isTrustlist);
}

void SetCellularPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<NetQuotaPolicy> quotaPolicies;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
}

void RegisterNetPolicyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    AccessToken token;
    sptr<INetPolicyCallbackTest> callback = sptr<INetPolicyCallbackTest>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
}

void UnregisterNetPolicyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    AccessToken token;
    sptr<INetPolicyCallbackTest> callback = sptr<INetPolicyCallbackTest>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
}

void GetIdleTrustlistFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    AccessToken token;
    std::vector<uint32_t> uids;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetIdleTrustlist(uids);
}

void GetNetQuotaPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<NetQuotaPolicy> quotaPolicies;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
}

void SetNetQuotaPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::vector<NetQuotaPolicy> quotaPolicies;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
}

void IsUidNetAllowedFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    uint32_t uid = GetData<uint32_t>();
    bool isMetered = uid % 2 == 0;
    std::string ifaceName = GetStringFromData(STR_LEN);
    DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAllowed(uid, isMetered);
    DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAllowed(uid, ifaceName);
}

void ResetPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    std::string iccid = GetStringFromData(STR_LEN);
    DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies(iccid);
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicy();
}

void UpdateRemindPolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    int32_t netType = GetData<int32_t>();
    uint32_t remindType = GetData<uint32_t>();
    std::string iccid = GetStringFromData(STR_LEN);
    DelayedSingleton<NetPolicyClient>::GetInstance()->UpdateRemindPolicy(netType, iccid, remindType);
}

void SetDeviceIdleAllowedListFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    bool isAllowed = GetData<int32_t>() % 2 == 0;
    uint32_t uid = GetData<uint32_t>();
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdleAllowedList(uid, isAllowed);
    std::vector<uint32_t> uids;
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetDeviceIdleAllowedList(uids);
}

void SetDeviceIdlePolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    bool enable = GetData<int32_t>() % 2 == 0;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdlePolicy(enable);
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
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
    OHOS::NetManagerStandard::IsUidNetAllowedFuzzTest(data, size);
    OHOS::NetManagerStandard::ResetPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateRemindPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::SetDeviceIdleAllowedListFuzzTest(data, size);
    OHOS::NetManagerStandard::SetDeviceIdlePolicyFuzzTest(data, size);
    return 0;
}