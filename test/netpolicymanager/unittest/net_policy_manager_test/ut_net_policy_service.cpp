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

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_policy_inner_define.h"
#include "net_policy_service.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
std::shared_ptr<NetPolicyService> g_NetPolicyService = nullptr;
constexpr int32_t TRIGER_DELAY_US = 100000;
constexpr uint32_t TEST_UID = 10000;
const std::string TEST_STRING_PERIODDURATION = "M1";

HapInfoParams testInfoParms = {
    .userID = 1,
    .bundleName = "net_policy_service_test",
    .instIndex = 0,
    .appIDDesc = "test",
};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.test",
    .bundleName = "net_policy_service_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net policy service",
    .descriptionId = 1,
};

PermissionStateFull testState = {
    .permissionName = "ohos.permission.test",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef},
    .permStateList = {testState},
};
} // namespace

class AccessToken {
public:
    AccessToken() : currentID_(GetSelfTokenID())
    {
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
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};

class UtNetPolicyService : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void UtNetPolicyService::SetUpTestCase()
{
    g_NetPolicyService = DelayedSingleton<NetPolicyService>::GetInstance();
}

void UtNetPolicyService::TearDownTestCase() {}

void UtNetPolicyService::SetUp() {}

void UtNetPolicyService::TearDown() {}

sptr<NetPolicyCallbackTest> UtNetPolicyService::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callback = new (std::nothrow) NetPolicyCallbackTest();
    return callback;
}

/**
 * @tc.name: SetPolicyByUid001
 * @tc.desc: Test NetPolicyService SetPolicyByUid Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetPolicyByUid001, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetPolicyByUid(TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    std::cout << "NetPolicyService023 SetPolicyByUid ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetPolicyByUid001
 * @tc.desc: Test NetPolicyService GetPolicyByUid Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetPolicyByUid001, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->GetPolicyByUid(TEST_UID);
    std::cout << "NetPolicyService024 GetPolicyByUid ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetUidsByPolicy001
 * @tc.desc: Test NetPolicyService GetUidsByPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetUidsByPolicy001, TestSize.Level1)
{
    AccessToken token;
    std::vector<uint32_t> ret = g_NetPolicyService->GetUidsByPolicy(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(ret == std::vector<uint32_t>(0));
}

/**
 * @tc.name: IsUidNetAllowed001
 * @tc.desc: Test NetPolicyService IsUidNetAllowed Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, IsUidNetAllowed001, TestSize.Level1)
{
    AccessToken token;
    bool ret = g_NetPolicyService->IsUidNetAllowed(TEST_UID, false);
    std::cout << "NetPolicyService026 IsUidNetAllowed ret:" << ret << std::endl;
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: IsUidNetAllowed002
 * @tc.desc: Test NetPolicyService IsUidNetAllowed Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, IsUidNetAllowed002, TestSize.Level1)
{
    AccessToken token;
    const std::string ifaceName = "iface";
    bool ret = g_NetPolicyService->IsUidNetAllowed(TEST_UID, ifaceName);
    std::cout << "NetPolicyService027 IsUidNetAllowed ret:" << ret << std::endl;
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: SetNetQuotaPolicies001
 * @tc.desc: Test NetPolicyService SetNetQuotaPolicies Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNetQuotaPolicies001, TestSize.Level1)
{
    AccessToken token;
    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy quotaPolicy;
    quotaPolicy.netType = 0;
    quotaPolicy.iccid = std::to_string(TRIGER_DELAY_US);
    quotaPolicy.periodStartTime = TRIGER_DELAY_US;
    quotaPolicy.periodDuration = TEST_STRING_PERIODDURATION;
    quotaPolicy.warningBytes = TRIGER_DELAY_US;
    quotaPolicy.limitBytes = TRIGER_DELAY_US;
    quotaPolicy.lastLimitRemind = -1;
    quotaPolicy.metered = true;
    quotaPolicy.source = 0;
    quotaPolicies.push_back(quotaPolicy);
    int32_t ret = g_NetPolicyService->SetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyService028 SetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetNetQuotaPolicies001
 * @tc.desc: Test NetPolicyService GetNetQuotaPolicies Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetNetQuotaPolicies001, TestSize.Level1)
{
    AccessToken token;
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = g_NetPolicyService->GetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyService029 GetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: ResetPolicies001
 * @tc.desc: Test NetPolicyService ResetPolicies Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, ResetPolicies001, TestSize.Level1)
{
    AccessToken token;
    std::string iccid = "0";

    int32_t ret = g_NetPolicyService->ResetPolicies(iccid);
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: SetBackgroundPolicy001
 * @tc.desc: Test NetPolicyService SetBackgroundPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetBackgroundPolicy001, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetBackgroundPolicy(true);
    std::cout << "NetPolicyService031 SetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetBackgroundPolicy001
 * @tc.desc: Test NetPolicyService GetBackgroundPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetBackgroundPolicy001, TestSize.Level1)
{
    AccessToken token;
    bool ret = g_NetPolicyService->GetBackgroundPolicy();
    std::cout << "NetPolicyService032 GetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: UpdateRemindPolicy001
 * @tc.desc: Test NetPolicyService UpdateRemindPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, UpdateRemindPolicy001, TestSize.Level1)
{
    AccessToken token;
    int32_t ret =
        g_NetPolicyService->UpdateRemindPolicy(0, std::to_string(TRIGER_DELAY_US), RemindType::REMIND_TYPE_LIMIT);
    std::cout << "NetPolicyService033 UpdateRemindPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetBackgroundPolicyByUid001
 * @tc.desc: Test NetPolicyService GetBackgroundPolicyByUid Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetBackgroundPolicyByUid001, TestSize.Level1)
{
    AccessToken token;
    uint32_t ret = g_NetPolicyService->GetBackgroundPolicyByUid(TEST_UID);
    std::cout << "NetPolicyService034 GetBackgroundPolicyByUid ret:" << ret << std::endl;
    uint32_t ret2 = static_cast<uint32_t>(NetPolicyResultCode::ERR_PERMISSION_DENIED);
    std::cout << "NetPolicyService034 GetBackgroundPolicyByUid ret2:" << ret2 << std::endl;
    ASSERT_TRUE(ret == ret2);
}

/**
 * @tc.name: SetDeviceIdleAllowedList001
 * @tc.desc: Test NetPolicyService SetDeviceIdleAllowedList Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetDeviceIdleAllowedList001, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetDeviceIdleAllowedList(TEST_UID, true);
    std::cout << "NetPolicyService035 SetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetDeviceIdleAllowedList001
 * @tc.desc: Test NetPolicyService GetDeviceIdleAllowedList Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetDeviceIdleAllowedList001, TestSize.Level1)
{
    AccessToken token;
    std::vector<uint32_t> uids;
    int32_t ret = g_NetPolicyService->GetDeviceIdleAllowedList(uids);
    std::cout << "NetPolicyService036 GetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: SetDeviceIdlePolicy001
 * @tc.desc: Test NetPolicyService SetDeviceIdlePolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetDeviceIdlePolicy001, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetDeviceIdlePolicy(true);
    std::cout << "NetPolicyService037 SetDeviceIdlePolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetPolicyCallback001
 * @tc.desc: Test NetPolicyService RegisterNetPolicyCallback UnregisterNetPolicyCallback Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, RegisterNetPolicyCallback001, TestSize.Level1)
{
    AccessToken token;
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t ret1 = g_NetPolicyService->RegisterNetPolicyCallback(callback);
    ASSERT_TRUE(ret1 == NetPolicyResultCode::ERR_PERMISSION_DENIED);

    int32_t ret2 = g_NetPolicyService->UnregisterNetPolicyCallback(callback);
    ASSERT_TRUE(ret2 == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}
} // namespace NetManagerStandard
} // namespace OHOS