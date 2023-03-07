/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "net_mgr_log_wrapper.h"
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_policy_inner_define.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string TEST_STRING_PERIODDURATION = "M1";
constexpr int32_t WAIT_TIME_SECOND_LONG = 10;
constexpr int32_t TRIGER_DELAY_US = 100000;
constexpr int32_t TEST_CONSTANT_NUM = 3;
constexpr int32_t BACKGROUND_POLICY_TEST_UID = 123;
constexpr uint32_t TEST_UID1 = 10;
constexpr uint32_t TEST_UID2 = 2;
constexpr uint32_t TEST_UID3 = 3;
constexpr uint32_t TEST_UID4 = 4;
constexpr uint32_t TEST_UID5 = 5;
constexpr uint32_t TEST_UID6 = 100;
constexpr uint32_t TEST_UID7 = 1;
constexpr uint32_t TEST_WARNING_BYTES = 1234;
constexpr uint32_t TEST_LIMIT_BYTES = 5678;

using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms1 = {.userID = 1,
                                .bundleName = "net_policy_manager_test",
                                .instIndex = 0,
                                .appIDDesc = "test"};

PermissionDef testPermDef1 = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                              .bundleName = "net_policy_manager_test",
                              .grantMode = 1,
                              .availableLevel = APL_SYSTEM_BASIC,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net policy connectivity internal",
                              .descriptionId = 1};

PermissionStateFull testState1 = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                  .isGeneral = true,
                                  .resDeviceID = {"local"},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .grantFlags = {2}};

HapPolicyParams testPolicyPrams1 = {.apl = APL_SYSTEM_BASIC,
                                    .domain = "test.domain",
                                    .permList = {testPermDef1},
                                    .permStateList = {testState1}};

HapInfoParams testInfoParms2 = {.userID = 1,
                                .bundleName = "net_policy_manager_test",
                                .instIndex = 0,
                                .appIDDesc = "test"};

PermissionDef testPermDef2 = {.permissionName = "ohos.permission.SET_NETWORK_POLICY",
                              .bundleName = "net_policy_manager_test",
                              .grantMode = 1,
                              .availableLevel = APL_SYSTEM_BASIC,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net policy connectivity internal",
                              .descriptionId = 1};

PermissionStateFull testState2 = {.permissionName = "ohos.permission.SET_NETWORK_POLICY",
                                  .isGeneral = true,
                                  .resDeviceID = {"local"},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .grantFlags = {2}};

HapPolicyParams testPolicyPrams2 = {.apl = APL_SYSTEM_BASIC,
                                    .domain = "test.domain",
                                    .permList = {testPermDef2},
                                    .permStateList = {testState2}};

HapInfoParams testInfoParms3 = {.userID = 1,
                                .bundleName = "net_policy_manager_test",
                                .instIndex = 0,
                                .appIDDesc = "test"};

PermissionDef testPermDef3 = {.permissionName = "ohos.permission.GET_NETWORK_POLICY",
                              .bundleName = "net_policy_manager_test",
                              .grantMode = 1,
                              .availableLevel = APL_SYSTEM_BASIC,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net policy connectivity internal",
                              .descriptionId = 1};

PermissionStateFull testState3 = {.permissionName = "ohos.permission.GET_NETWORK_POLICY",
                                  .isGeneral = true,
                                  .resDeviceID = {"local"},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .grantFlags = {2}};

HapPolicyParams testPolicyPrams3 = {.apl = APL_SYSTEM_BASIC,
                                    .domain = "test.domain",
                                    .permList = {testPermDef3},
                                    .permStateList = {testState3}};
} // namespace

class AccessToken {
public:
    AccessToken(HapInfoParams &testInfoParms, HapPolicyParams &testPolicyPrams) : currentID_(GetSelfTokenID())
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

using namespace testing::ext;
class NetPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void NetPolicyManagerTest::SetUpTestCase()
{
    const std::string tempIccid = "123";
    int32_t ret = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies(tempIccid);
    if (ret == NETMANAGER_SUCCESS) {
        std::cout << "ResetPolicies successful" << std::endl;
    }
}

void NetPolicyManagerTest::TearDownTestCase()
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret1 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID2, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret1" << ret1 << std::endl;
    int32_t ret2 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID3, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret2" << ret2 << std::endl;
    int32_t ret3 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID4, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret3" << ret3 << std::endl;
    int32_t ret4 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID5, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret4" << ret4 << std::endl;
    int32_t ret5 =
        DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID6, NetUidPolicy::NET_POLICY_NONE);
    std::cout << "Result ret5" << ret5 << std::endl;
    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.netType = -1;
    quotaPolicy1.iccid = std::to_string(TRIGER_DELAY_US);

    NetQuotaPolicy quotaPolicy2;
    quotaPolicy2.netType = -1;
    quotaPolicy2.iccid = "sim_abcdefg_1";

    quotaPolicies.push_back(quotaPolicy1);
    quotaPolicies.push_back(quotaPolicy2);

    int32_t ret6 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
    std::cout << "Result ret6" << ret6 << std::endl;
    int32_t ret7 = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies("sim_abcdefg_1");
    std::cout << "Result ret7" << ret7 << std::endl;
    int32_t ret8 = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies("100000");
    std::cout << "Result ret8" << ret8 << std::endl;
    int32_t ret9 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(true);
    std::cout << "Result ret9" << ret9 << std::endl;
}

void NetPolicyManagerTest::SetUp() {}

void NetPolicyManagerTest::TearDown() {}

sptr<NetPolicyCallbackTest> NetPolicyManagerTest::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callback = new (std::nothrow) NetPolicyCallbackTest();
    return callback;
}

/**
 * @tc.name: NetPolicyManager001
 * @tc.desc: Test NetPolicyManager SetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager001, TestSize.Level1)
{
    {
        AccessToken token(testInfoParms2, testPolicyPrams2);
        int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
            TEST_UID7, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
        std::cout << "NetPolicyManager001 SetPolicyByUid result:" << result << std::endl;
        ASSERT_EQ(result, NETMANAGER_SUCCESS);
    }
    AccessToken token(testInfoParms3, testPolicyPrams3);
    uint32_t policy = 0;
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetPolicyByUid(TEST_UID7, policy);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
    ASSERT_EQ(policy, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyManager002
 * @tc.desc: Test NetPolicyManager GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager002, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID2, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    uint32_t policy = 0;
    AccessToken token2(testInfoParms3, testPolicyPrams3);
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetPolicyByUid(TEST_UID2, policy);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
    ASSERT_EQ(policy, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyManager003
 * @tc.desc: Test NetPolicyManager GetUidsByPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager003, TestSize.Level1)
{
    AccessToken token(testInfoParms3, testPolicyPrams3);
    std::vector<uint32_t> uids;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND, uids);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager004
 * @tc.desc: Test NetPolicyManager IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager004, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool isAllowed = false;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAllowed(TEST_UID7, false, isAllowed);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    ASSERT_TRUE(isAllowed);
}

/**
 * @tc.name: NetPolicyManager005
 * @tc.desc: Test NetPolicyManager IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager005, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool isAllowed = false;
    int32_t result =
        DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAllowed(TEST_UID7, std::string("test"), isAllowed);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    ASSERT_TRUE(isAllowed);
}

void TrigerCallback()
{
    usleep(TRIGER_DELAY_US);
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID1, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager006
 * @tc.desc: Test NetPolicyManager RegisterNetPolicyCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager006, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = client->RegisterNetPolicyCallback(callback);
    if (result == NETMANAGER_SUCCESS) {
        ASSERT_EQ(result, NETMANAGER_SUCCESS);
        std::thread trigerCallback(TrigerCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        trigerCallback.join();
        uint32_t uid = callback->GetUid();
        uint32_t netPolicy = callback->GetPolicy();
        std::cout << "NetPolicyManager006 RegisterNetPolicyCallback uid:" << uid
                  << " netPolicy:" << static_cast<uint32_t>(netPolicy) << std::endl;
        ASSERT_EQ(uid, TEST_UID1);
        ASSERT_EQ(netPolicy, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    } else {
        std::cout << "NetPolicyManager006 RegisterNetPolicyCallback return fail" << std::endl;
    }
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    result = client->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager007
 * @tc.desc: Test NetPolicyManager SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager007, TestSize.Level1)
{
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
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager008
 * @tc.desc: Test NetPolicyManager GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager008, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    AccessToken token(testInfoParms3, testPolicyPrams3);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager009
 * @tc.desc: Test NetPolicyManager SetCellularPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager009, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy cellularPolicy;
    for (uint32_t i = 0; i < TEST_CONSTANT_NUM; ++i) {
        cellularPolicy.iccid = std::to_string(i);
        cellularPolicy.periodStartTime = TRIGER_DELAY_US + i;
        cellularPolicy.periodDuration = TEST_STRING_PERIODDURATION;
        cellularPolicy.title = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.summary = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.limitBytes = TRIGER_DELAY_US + i;
        cellularPolicy.limitAction = TEST_CONSTANT_NUM;
        cellularPolicy.usedBytes = TRIGER_DELAY_US + i;
        cellularPolicy.usedTimeDuration = TRIGER_DELAY_US + i;
        cellularPolicy.possessor = std::to_string(TRIGER_DELAY_US + i);

        quotaPolicies.push_back(cellularPolicy);
    }
    AccessToken token(testInfoParms3, testPolicyPrams3);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager010
 * @tc.desc: Test NetPolicyManager GetCellularPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager010, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    AccessToken token(testInfoParms3, testPolicyPrams3);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager011
 * @tc.desc: Test NetPolicyManager ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager011, TestSize.Level1)
{
    std::string iccid = "0";
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetPolicies(iccid);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0012
 * @tc.desc: Test NetPolicyManager UpdateRemindPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager012, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->UpdateRemindPolicy(
        0, std::to_string(TRIGER_DELAY_US), RemindType::REMIND_TYPE_LIMIT);
    std::cout << "NetPolicyManager012 result value:" << result << std::endl;
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0013
 * @tc.desc: Test NetPolicyManager SetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager013, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdleAllowedList(TEST_UID7, true);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0014
 * @tc.desc: Test NetPolicyManager GetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager014, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    std::vector<uint32_t> uids;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetDeviceIdleAllowedList(uids);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0015
 * @tc.desc: Test NetPolicyManager SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager015, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(true);
    ASSERT_EQ(result, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: NetPolicyManager0016
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager016, TestSize.Level1)
{
    AccessToken token(testInfoParms3, testPolicyPrams3);
    bool backgroundPolicy;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicy(backgroundPolicy);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0017
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager017, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(false);
    uint32_t backgroundPolicyOfUid = 0;
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    int32_t ret2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(
        BACKGROUND_POLICY_TEST_UID, backgroundPolicyOfUid);
    std::cout << "NetPolicyManager017 GetBackgroundPolicyByUid " << backgroundPolicyOfUid << std::endl;
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(backgroundPolicyOfUid, NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyManager0018
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager018, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(true);
    uint32_t backgroundPolicyOfUid = 0;
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    int32_t ret2 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(
        BACKGROUND_POLICY_TEST_UID, backgroundPolicyOfUid);
    std::cout << "NetPolicyManager0018 GetBackgroundPolicyByUid " << backgroundPolicyOfUid << std::endl;
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(backgroundPolicyOfUid, NET_BACKGROUND_POLICY_ENABLE);
}

/**
 * @tc.name: NetPolicyManager0019
 * @tc.desc: Test NetPolicyManager SetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager019, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdleAllowedList(16, true);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0020
 * @tc.desc: Test NetPolicyManager SetDeviceIdlePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager020, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    int32_t ret1 = client->SetDeviceIdleAllowedList(10, true);
    int32_t ret2 = client->SetDeviceIdleAllowedList(TEST_UID6, true);
    int32_t ret3 = client->SetDeviceIdleAllowedList(99, true);
    int32_t result = client->SetDeviceIdlePolicy(true);
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret3, NETMANAGER_SUCCESS);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0021
 * @tc.desc: Test NetPolicyManager SetDeviceIdlePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager021, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    int32_t ret1 = client->SetDeviceIdleAllowedList(10, false);
    int32_t ret2 = client->SetDeviceIdleAllowedList(TEST_UID6, false);
    int32_t ret3 = client->SetDeviceIdleAllowedList(99, false);
    int32_t ret4 = client->SetDeviceIdleAllowedList(16, false);
    int32_t result = client->SetDeviceIdlePolicy(false);
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret3, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret4, NETMANAGER_SUCCESS);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager0022
 * @tc.desc: Test NetPolicyManager GetUidsByPolicy.
 * @tc.type: FUNC
 */

HWTEST_F(NetPolicyManagerTest, NetPolicyManager022, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret1 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID2, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    int32_t ret2 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID3, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    int32_t ret3 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID4, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    int32_t ret4 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID5, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret3, NETMANAGER_SUCCESS);
    ASSERT_EQ(ret4, NETMANAGER_SUCCESS);
    std::vector<uint32_t> uids;
    AccessToken token2(testInfoParms3, testPolicyPrams3);
    int32_t ret5 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND, uids);
    ASSERT_EQ(ret5, NETMANAGER_SUCCESS);
    bool result = false;
    bool result2 = false;
    for (const auto &i : uids) {
        if (i == TEST_UID2) {
            result = true;
        }
    }
    for (const auto &i : uids) {
        if (i == TEST_UID3) {
            result2 = true;
        }
    }
    EXPECT_TRUE(result && result2);
    std::vector<uint32_t> uids2;
    int32_t ret6 = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(
        NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND, uids2);
    ASSERT_EQ(ret6, NETMANAGER_SUCCESS);
    result = false;
    result2 = false;
    for (const auto &i : uids2) {
        if (i == TEST_UID4) {
            result = true;
        }
    }
    for (const auto &i : uids2) {
        if (i == TEST_UID5) {
            result2 = true;
        }
    }
    EXPECT_TRUE(result && result2);
}

void SetBackgroundPolicyCallback()
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    usleep(TRIGER_DELAY_US);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(false);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
/**
 * @tc.name: NetPolicyManager023
 * @tc.desc: Test NetPolicyManager NetBackgroundPolicyChange.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager023, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = client->RegisterNetPolicyCallback(callback);
    if (result == NETMANAGER_SUCCESS) {
        std::thread setBackgroundPolicyCallback(SetBackgroundPolicyCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setBackgroundPolicyCallback.join();
        bool result2 = callback->GetBackgroundPolicy();
        std::cout << "NetPolicyManager023 Get background policy is:" << result2 << std::endl;
        ASSERT_EQ(result2, false);
    } else {
        std::cout << "NetPolicyManager023 RegisterNetPolicyCallback return fail" << std::endl;
    }
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    result = client->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

void SetNetRuleChangeCallback()
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    usleep(TRIGER_DELAY_US);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID6, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyManager024
 * @tc.desc: Test NetPolicyManager NetMeteredIfacesChange.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager024, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);

    if (result == NETMANAGER_SUCCESS) {
        std::thread setNetRuleChangedCallback(SetNetRuleChangeCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setNetRuleChangedCallback.join();
        uint32_t result2 = callback->GetRule();
        std::cout << "NetPolicyManager024 rule result:" << result2 << std::endl;
        ASSERT_NE(result2, 128);
    } else {
        std::cout << "NetPolicyManager024 SetNetRuleChangeCallback return fail" << std::endl;
    }
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    result = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

void SetNetQuotaPoliciesCallback()
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    usleep(TRIGER_DELAY_US);

    std::vector<NetQuotaPolicy> quotaPolicies;

    NetQuotaPolicy quotaPolicy;
    quotaPolicy.netType = 0;
    quotaPolicy.iccid = "sim_abcdefg_1";
    quotaPolicy.periodDuration = "m2";
    quotaPolicy.warningBytes = TEST_WARNING_BYTES;
    quotaPolicy.limitBytes = TEST_LIMIT_BYTES;
    quotaPolicy.lastLimitRemind = -1;
    quotaPolicy.metered = true;
    quotaPolicy.source = 0;
    quotaPolicies.push_back(quotaPolicy);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
/**
 * @tc.name: NetPolicyManager025
 * @tc.desc: Test NetPolicyManager NetQuotaPolicyChange.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager025, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
    if (result == NETMANAGER_SUCCESS) {
        std::thread setNetQuotaPoliciesCallback(SetNetQuotaPoliciesCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setNetQuotaPoliciesCallback.join();
        std::cout << "NetPolicyManager025 result is:" << result << std::endl;
    } else {
        std::cout << "NetPolicyManager025 RegisterNetPolicyCallback return fail" << std::endl;
    }
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    result = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
