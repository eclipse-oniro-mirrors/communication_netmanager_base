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
#include <mutex>
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
#include "net_policy_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
std::shared_ptr<NetPolicyClient> g_netPolicyClient = nullptr;
constexpr int32_t TRIGER_DELAY_US = 100000;
constexpr int32_t WAIT_TIME_SECOND_LONG = 10;
constexpr uint32_t TEST_UID = 10000;
const std::string TEST_STRING_PERIODDURATION = "M1";

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

NetQuotaPolicy GetQuota()
{
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
    return quotaPolicy;
}
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
    AccessTokenID accessID_;
};

class UtNetPolicyClient : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void UtNetPolicyClient::SetUpTestCase()
{
    g_netPolicyClient = DelayedSingleton<NetPolicyClient>::GetInstance();
}

void UtNetPolicyClient::TearDownTestCase() {}

void UtNetPolicyClient::SetUp() {}

void UtNetPolicyClient::TearDown() {}

sptr<NetPolicyCallbackTest> UtNetPolicyClient::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callback = new (std::nothrow) NetPolicyCallbackTest();
    return callback;
}

/**
 * @tc.name: SetPolicyByUid001
 * @tc.desc: Test NetPolicyClient SetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetPolicyByUid001, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret = g_netPolicyClient->SetPolicyByUid(TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    std::cout << "NetPolicyClient001 SetPolicyByUid ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetPolicyByUid001
 * @tc.desc: Test NetPolicyClient GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, GetPolicyByUid001, TestSize.Level1)
{
    AccessToken token2(testInfoParms3, testPolicyPrams3);
    uint32_t policy = 0;
    int32_t ret = g_netPolicyClient->GetPolicyByUid(TEST_UID, policy);
    std::cout << "NetPolicyClient002 GetPolicyByUid policy:" << policy << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ASSERT_EQ(policy, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: GetUidsByPolicy001
 * @tc.desc: Test NetPolicyClient GetUidsByPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, GetUidsByPolicy001, TestSize.Level1)
{
    AccessToken token(testInfoParms3, testPolicyPrams3);
    std::vector<uint32_t> uids;
    int32_t ret = g_netPolicyClient->GetUidsByPolicy(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND, uids);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ASSERT_TRUE(uids.size() > 0);
}

/**
 * @tc.name: IsUidNetAllowed001
 * @tc.desc: Test NetPolicyClient IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, IsUidNetAllowed001, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool isAllowed = false;
    int32_t ret = g_netPolicyClient->IsUidNetAllowed(TEST_UID, false, isAllowed);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "NetPolicyClient004 IsUidNetAllowed ret:" << ret << std::endl;
    ASSERT_TRUE(isAllowed == true);
}

/**
 * @tc.name: IsUidNetAllowed002
 * @tc.desc: Test NetPolicyClient IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, IsUidNetAllowed002, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool isAllowed = false;
    const std::string ifaceName = "iface";
    int32_t ret = g_netPolicyClient->IsUidNetAllowed(TEST_UID, ifaceName, isAllowed);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << "NetPolicyClient005 IsUidNetAllowed isAllowed:" << isAllowed << std::endl;
    ASSERT_TRUE(isAllowed == true);
}

/**
 * @tc.name: IsUidNetAccess001
 * @tc.desc: Test NetPolicyClient IsUidNetAccess.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, IsUidNetAccess001, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool isAllowed = false;
    int32_t ret = g_netPolicyClient->IsUidNetAccess(TEST_UID, false, isAllowed);
    std::cout << "NetPolicyClient006 IsUidNetAccess isAllowed:" << isAllowed << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ASSERT_TRUE(isAllowed == true);
}

/**
 * @tc.name: IsUidNetAccess002
 * @tc.desc: Test NetPolicyClient IsUidNetAccess.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, IsUidNetAccess002, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool isAllowed = false;
    const std::string ifaceName = "iface";
    int32_t ret = g_netPolicyClient->IsUidNetAccess(TEST_UID, ifaceName, isAllowed);
    std::cout << "NetPolicyClient007 IsUidNetAccess isAllowed:" << isAllowed << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ASSERT_TRUE(isAllowed == true);
}

/**
 * @tc.name: SetNetQuotaPolicies001
 * @tc.desc: Test NetPolicyClient SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetNetQuotaPolicies001, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(GetQuota());
    int32_t ret = g_netPolicyClient->SetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyClient008 SetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNetQuotaPolicies002
 * @tc.desc: Test NetPolicyClient SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetNetQuotaPolicies002, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = g_netPolicyClient->SetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyClient008 SetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_EQ(ret, POLICY_ERR_INVALID_QUOTA_POLICY);
}

/**
 * @tc.name: SetNetQuotaPolicies003
 * @tc.desc: Test NetPolicyClient SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetNetQuotaPolicies003, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    std::vector<NetQuotaPolicy> quotaPolicies;
    for (int32_t i = 0; i < QUOTA_POLICY_MAX_SIZE; i++) {
        quotaPolicies.push_back(GetQuota());
    }
    quotaPolicies.push_back(GetQuota());
    int32_t ret = g_netPolicyClient->SetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyClient008 SetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_EQ(ret, POLICY_ERR_INVALID_QUOTA_POLICY);
}

/**
 * @tc.name: GetNetQuotaPolicies001
 * @tc.desc: Test NetPolicyClient GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, GetNetQuotaPolicies001, TestSize.Level1)
{
    AccessToken token(testInfoParms3, testPolicyPrams3);
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = g_netPolicyClient->GetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyClient009 GetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetFactoryPolicy001
 * @tc.desc: Test NetPolicyClient SetFactoryPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetFactoryPolicy001, TestSize.Level1)
{
    std::string iccid = "0";
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret = g_netPolicyClient->SetFactoryPolicy(iccid);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: ResetPolicies001
 * @tc.desc: Test NetPolicyClient ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, ResetPolicies001, TestSize.Level1)
{
    std::string iccid = "0";
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret = g_netPolicyClient->ResetPolicies(iccid);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetBackgroundPolicy001
 * @tc.desc: Test NetPolicyClient SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetBackgroundPolicy001, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret = g_netPolicyClient->SetBackgroundPolicy(true);
    std::cout << "NetPolicyClient012 SetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: GetBackgroundPolicy001
 * @tc.desc: Test NetPolicyClient GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, GetBackgroundPolicy001, TestSize.Level1)
{
    bool backgroundPolicy;
    AccessToken token(testInfoParms3, testPolicyPrams3);
    int32_t ret = g_netPolicyClient->GetBackgroundPolicy(backgroundPolicy);
    std::cout << "NetPolicyClient013 GetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ASSERT_TRUE(backgroundPolicy == true);
}

/**
 * @tc.name: GetBackgroundPolicyByUid001
 * @tc.desc: Test NetPolicyClient GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, GetBackgroundPolicyByUid001, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret1 = g_netPolicyClient->SetBackgroundPolicy(false);
    ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    uint32_t backgroundPolicyOfUid = 0;
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    int32_t ret2 = g_netPolicyClient->GetBackgroundPolicyByUid(TEST_UID, backgroundPolicyOfUid);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
    ASSERT_EQ(backgroundPolicyOfUid, NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: SetSnoozePolicy001
 * @tc.desc: Test NetPolicyClient SetSnoozePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetSnoozePolicy001, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret = g_netPolicyClient->SetSnoozePolicy(0, std::to_string(TRIGER_DELAY_US));
    std::cout << "NetPolicyClient015 SetSnoozePolicy ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateRemindPolicy001
 * @tc.desc: Test NetPolicyClient UpdateRemindPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, UpdateRemindPolicy001, TestSize.Level1)
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t ret =
        g_netPolicyClient->UpdateRemindPolicy(0, std::to_string(TRIGER_DELAY_US), RemindType::REMIND_TYPE_LIMIT);
    std::cout << "NetPolicyClient016 UpdateRemindPolicy ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetIdleTrustlist001
 * @tc.desc: Test NetPolicyClient SetIdleTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetIdleTrustlist001, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t ret = g_netPolicyClient->SetIdleTrustlist(TEST_UID, true);
    std::cout << "NetPolicyClient017 SetIdleTrustlist ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetDeviceIdleAllowedList001
 * @tc.desc: Test NetPolicyClient SetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetDeviceIdleAllowedList001, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t ret = g_netPolicyClient->SetDeviceIdleAllowedList(TEST_UID, true);
    std::cout << "NetPolicyClient018 SetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIdleTrustlist001
 * @tc.desc: Test NetPolicyClient GetIdleTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, GetIdleTrustlist001, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t ret = g_netPolicyClient->GetIdleTrustlist(uids);
    std::cout << "NetPolicyClient019 GetIdleTrustlist ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetDeviceIdleAllowedList001
 * @tc.desc: Test NetPolicyClient GetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, GetDeviceIdleAllowedList001, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t ret = g_netPolicyClient->GetDeviceIdleAllowedList(uids);
    std::cout << "NetPolicyClient020 GetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetDeviceIdlePolicy001
 * @tc.desc: Test NetPolicyClient SetDeviceIdlePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, SetDeviceIdlePolicy001, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t ret = g_netPolicyClient->SetDeviceIdlePolicy(true);
    std::cout << "NetPolicyClient021 SetDeviceIdlePolicy ret:" << ret << std::endl;
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

void PolicyServiceCallback()
{
    AccessToken token(testInfoParms2, testPolicyPrams2);
    usleep(TRIGER_DELAY_US);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
/**
 * @tc.name: RegisterNetPolicyCallback001
 * @tc.desc: Test NetPolicyClient RegisterNetPolicyCallback UnregisterNetPolicyCallback.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyClient, RegisterNetPolicyCallback001, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t ret1 = g_netPolicyClient->RegisterNetPolicyCallback(callback);
    if (ret1 == NETMANAGER_SUCCESS && callback != nullptr) {
        std::thread trigerCallback(PolicyServiceCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        trigerCallback.join();
        uint32_t uid = callback->GetUid();
        uint32_t netPolicy = callback->GetPolicy();
        std::cout << "NetPolicyClient022 RegisterNetPolicyCallback uid:" << uid
                  << " netPolicy:" << static_cast<uint32_t>(netPolicy) << std::endl;
        ASSERT_EQ(uid, TEST_UID);
        ASSERT_EQ(netPolicy, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
        ASSERT_EQ(ret1, NETMANAGER_SUCCESS);
    } else {
        std::cout << "NetPolicyClient022 RegisterNetPolicyCallback return fail" << std::endl;
    }
    AccessToken token2(testInfoParms1, testPolicyPrams1);
    int32_t ret2 = g_netPolicyClient->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(ret2, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
