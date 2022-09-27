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
#include "net_policy_service.h"
#include "net_policy_inner_define.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
std::shared_ptr<NetPolicyClient> g_NetPolicyClient = nullptr;
std::shared_ptr<NetPolicyService> g_NetPolicyService = nullptr;
constexpr int32_t TRIGER_DELAY_US = 100000;
constexpr int32_t WAIT_TIME_SECOND_LONG = 10;
constexpr uint32_t TEST_UID = 10000;
const std::string TEST_STRING_PERIODDURATION = "M1";

HapInfoParams testInfoParms = {.bundleName = "net_policy_service_test", .userID = 1, .instIndex = 0, .appIDDesc = "test"};

PermissionDef testPermDef = {.permissionName = "ohos.permission.test",
                             .bundleName = "net_policy_service_test",
                             .grantMode = 1,
                             .label = "label",
                             .labelId = 1,
                             .description = "Test net policy service",
                             .descriptionId = 1,
                             .availableLevel = APL_SYSTEM_BASIC};

PermissionStateFull testState = {.grantFlags = {2},
                                 .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                 .isGeneral = true,
                                 .permissionName = "ohos.permission.test",
                                 .resDeviceID = {"local"}};

HapPolicyParams testPolicyPrams = {.apl = APL_SYSTEM_BASIC,
                                   .domain = "test.domain",
                                   .permList = {testPermDef},
                                   .permStateList = {testState}};
} // namespace

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
    g_NetPolicyClient = DelayedSingleton<NetPolicyClient>::GetInstance();
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
 * @tc.name: NetPolicyService001
 * @tc.desc: Test NetPolicyService SetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService001, TestSize.Level1)
{
    uint32_t ret = g_NetPolicyClient->SetPolicyByUid(TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    std::cout << "NetPolicyService001 SetPolicyByUid ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService002
 * @tc.desc: Test NetPolicyService GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService002, TestSize.Level1)
{
    uint32_t ret = g_NetPolicyClient->GetPolicyByUid(TEST_UID);
    std::cout << "NetPolicyService002 GetPolicyByUid ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyService003
 * @tc.desc: Test NetPolicyService GetUidsByPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService003, TestSize.Level1)
{
    std::vector<uint32_t> ret = g_NetPolicyClient->GetUidsByPolicy(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(ret.size() > 0);
}

/**
 * @tc.name: NetPolicyService004
 * @tc.desc: Test NetPolicyService IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService004, TestSize.Level1)
{
    bool ret = g_NetPolicyClient->IsUidNetAllowed(TEST_UID, false);
    std::cout << "NetPolicyService004 IsUidNetAllowed ret:" << ret << std::endl;
    ASSERT_TRUE(ret == true);
}

/**
 * @tc.name: NetPolicyService005
 * @tc.desc: Test NetPolicyService IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService005, TestSize.Level1)
{
    const std::string ifaceName = "iface";
    bool ret = g_NetPolicyClient->IsUidNetAllowed(TEST_UID, ifaceName);
    std::cout << "NetPolicyService005 IsUidNetAllowed ret:" << ret << std::endl;
    ASSERT_TRUE(ret == true);
}

/**
 * @tc.name: NetPolicyService006
 * @tc.desc: Test NetPolicyService SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService006, TestSize.Level1)
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
    int32_t ret = g_NetPolicyClient->SetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyService006 SetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService007
 * @tc.desc: Test NetPolicyService GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService007, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = g_NetPolicyClient->GetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyService007 GetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService008
 * @tc.desc: Test NetPolicyService ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService008, TestSize.Level1)
{
    std::string iccid = "0";

    int32_t ret = g_NetPolicyClient->ResetPolicies(iccid);
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService009
 * @tc.desc: Test NetPolicyService SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService009, TestSize.Level1)
{
    uint32_t ret = g_NetPolicyClient->SetBackgroundPolicy(true);
    std::cout << "NetPolicyService009 SetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService010
 * @tc.desc: Test NetPolicyService GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService010, TestSize.Level1)
{
    bool ret = g_NetPolicyClient->GetBackgroundPolicy();
    std::cout << "NetPolicyService010 GetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: NetPolicyService011
 * @tc.desc: Test NetPolicyService GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService011, TestSize.Level1)
{
    uint32_t ret1 = g_NetPolicyClient->SetBackgroundPolicy(false);
    ASSERT_TRUE(ret1 == NetPolicyResultCode::ERR_NONE);
    uint32_t ret2 = g_NetPolicyClient->GetBackgroundPolicyByUid(TEST_UID);
    std::cout << "NetPolicyService011 GetBackgroundPolicyByUid ret2:" << ret2 << std::endl;
    ASSERT_EQ(ret2, NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyService012
 * @tc.desc: Test NetPolicyService UpdateRemindPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService012, TestSize.Level1)
{
    uint32_t ret =
        g_NetPolicyClient->UpdateRemindPolicy(0, std::to_string(TRIGER_DELAY_US), RemindType::REMIND_TYPE_LIMIT);
    std::cout << "NetPolicyService012 UpdateRemindPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService013
 * @tc.desc: Test NetPolicyService SetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService013, TestSize.Level1)
{
    uint32_t ret = g_NetPolicyClient->SetDeviceIdleAllowedList(TEST_UID, true);
    std::cout << "NetPolicyService013 SetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService014
 * @tc.desc: Test NetPolicyService GetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService014, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    uint32_t ret = g_NetPolicyClient->GetDeviceIdleAllowedList(uids);
    std::cout << "NetPolicyService014 GetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyService015
 * @tc.desc: Test NetPolicyService SetDeviceIdlePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService015, TestSize.Level1)
{
    uint32_t ret = g_NetPolicyClient->SetDeviceIdlePolicy(true);
    std::cout << "NetPolicyService015 SetDeviceIdlePolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_NONE);
}

void PolicyServiceCallback()
{
    usleep(TRIGER_DELAY_US);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}
/**
 * @tc.name: NetPolicyService016
 * @tc.desc: Test NetPolicyService RegisterNetPolicyCallback UnregisterNetPolicyCallback.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService016, TestSize.Level1)
{
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    uint32_t ret1 = g_NetPolicyClient->RegisterNetPolicyCallback(callback);
    if (ret1 == ERR_NONE && callback != nullptr) {
        std::thread trigerCallback(PolicyServiceCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        trigerCallback.join();
        uint32_t uid = callback->GetUid();
        uint32_t netPolicy = callback->GetPolicy();
        std::cout << "NetPolicyService016 RegisterNetPolicyCallback uid:" << uid
                  << " netPolicy:" << static_cast<uint32_t>(netPolicy) << std::endl;
        ASSERT_EQ(uid, TEST_UID);
        ASSERT_EQ(netPolicy, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
        ASSERT_TRUE(ret1 == ERR_NONE);
    } else {
        std::cout << "NetPolicyService016 RegisterNetPolicyCallback return fail" << std::endl;
    }
    uint32_t ret2 = g_NetPolicyClient->UnregisterNetPolicyCallback(callback);
    ASSERT_TRUE(ret2 == ERR_NONE);
}

/**
 * @tc.name: NetPolicyService017
 * @tc.desc: Test NetPolicyService SetPolicyByUid Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService017, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetPolicyByUid(TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    std::cout << "NetPolicyService017 SetPolicyByUid ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService018
 * @tc.desc: Test NetPolicyService GetPolicyByUid Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService018, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->GetPolicyByUid(TEST_UID);
    std::cout << "NetPolicyService018 GetPolicyByUid ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService019
 * @tc.desc: Test NetPolicyService GetUidsByPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService019, TestSize.Level1)
{
    AccessToken token;
    std::vector<uint32_t> ret = g_NetPolicyService->GetUidsByPolicy(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(ret == std::vector<uint32_t>(0));
}

/**
 * @tc.name: NetPolicyService020
 * @tc.desc: Test NetPolicyService IsUidNetAllowed Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService020, TestSize.Level1)
{
    AccessToken token;
    bool ret = g_NetPolicyService->IsUidNetAllowed(TEST_UID, false);
    std::cout << "NetPolicyService020 IsUidNetAllowed ret:" << ret << std::endl;
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: NetPolicyService021
 * @tc.desc: Test NetPolicyService IsUidNetAllowed Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService021, TestSize.Level1)
{
    AccessToken token;
    const std::string ifaceName = "iface";
    bool ret = g_NetPolicyService->IsUidNetAllowed(TEST_UID, ifaceName);
    std::cout << "NetPolicyService021 IsUidNetAllowed ret:" << ret << std::endl;
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: NetPolicyService022
 * @tc.desc: Test NetPolicyService SetNetQuotaPolicies Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService022, TestSize.Level1)
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
    std::cout << "NetPolicyService022 SetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService023
 * @tc.desc: Test NetPolicyService GetNetQuotaPolicies Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService023, TestSize.Level1)
{
    AccessToken token;
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = g_NetPolicyService->GetNetQuotaPolicies(quotaPolicies);
    std::cout << "NetPolicyService023 GetNetQuotaPolicies ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService024
 * @tc.desc: Test NetPolicyService ResetPolicies Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService024, TestSize.Level1)
{
    AccessToken token;
    std::string iccid = "0";

    int32_t ret = g_NetPolicyService->ResetPolicies(iccid);
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService025
 * @tc.desc: Test NetPolicyService SetBackgroundPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService025, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetBackgroundPolicy(true);
    std::cout << "NetPolicyService025 SetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService026
 * @tc.desc: Test NetPolicyService GetBackgroundPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService026, TestSize.Level1)
{
    AccessToken token;
    bool ret = g_NetPolicyService->GetBackgroundPolicy();
    std::cout << "NetPolicyService026 GetBackgroundPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == false);
}

/**
 * @tc.name: NetPolicyService027
 * @tc.desc: Test NetPolicyService UpdateRemindPolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService027, TestSize.Level1)
{
    AccessToken token;
    int32_t ret =
        g_NetPolicyService->UpdateRemindPolicy(0, std::to_string(TRIGER_DELAY_US), RemindType::REMIND_TYPE_LIMIT);
    std::cout << "NetPolicyService027 UpdateRemindPolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService028
 * @tc.desc: Test NetPolicyService GetBackgroundPolicyByUid Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService028, TestSize.Level1)
{
    AccessToken token;
    uint32_t ret = g_NetPolicyService->GetBackgroundPolicyByUid(TEST_UID);
    std::cout << "NetPolicyService028 GetBackgroundPolicyByUid ret:" << ret << std::endl;
    uint32_t ret2 = static_cast<uint32_t>(NetPolicyResultCode::ERR_PERMISSION_DENIED);
    std::cout << "NetPolicyService028 GetBackgroundPolicyByUid ret2:" << ret2 << std::endl;
    ASSERT_TRUE(ret == ret2);
}


/**
 * @tc.name: NetPolicyService029
 * @tc.desc: Test NetPolicyService SetDeviceIdleAllowedList Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService029, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetDeviceIdleAllowedList(TEST_UID, true);
    std::cout << "NetPolicyService029 SetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService030
 * @tc.desc: Test NetPolicyService GetDeviceIdleAllowedList Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService030, TestSize.Level1)
{
    AccessToken token;
    std::vector<uint32_t> uids;
    int32_t ret = g_NetPolicyService->GetDeviceIdleAllowedList(uids);
    std::cout << "NetPolicyService030 GetDeviceIdleAllowedList ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService031
 * @tc.desc: Test NetPolicyService SetDeviceIdlePolicy Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService031, TestSize.Level1)
{
    AccessToken token;
    int32_t ret = g_NetPolicyService->SetDeviceIdlePolicy(true);
    std::cout << "NetPolicyService031 SetDeviceIdlePolicy ret:" << ret << std::endl;
    ASSERT_TRUE(ret == NetPolicyResultCode::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: NetPolicyService032
 * @tc.desc: Test NetPolicyService RegisterNetPolicyCallback UnregisterNetPolicyCallback Without Permission.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, NetPolicyService032, TestSize.Level1)
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