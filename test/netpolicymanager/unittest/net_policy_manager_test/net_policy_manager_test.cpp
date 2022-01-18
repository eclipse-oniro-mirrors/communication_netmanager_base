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

#include <thread>

#include <gtest/gtest.h>

#include "net_policy_callback_test.h"
#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_policy_client.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t WAIT_TIME_SECOND_LONG = 60;
constexpr int32_t TRIGER_DELAY_US = 100000;
constexpr int32_t TEST_CONSTANT_NUM = 3;
const std::string TEST_STRING_PERIODDURATION = "M1";
constexpr int32_t BACKGROUND_POLICY_TEST_UID = 123;

using namespace testing::ext;
class NetPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void NetPolicyManagerTest::SetUpTestCase() {}

void NetPolicyManagerTest::TearDownTestCase() {}

void NetPolicyManagerTest::SetUp() {}

void NetPolicyManagerTest::TearDown() {}

sptr<NetPolicyCallbackTest> NetPolicyManagerTest::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callback = (std::make_unique<NetPolicyCallbackTest>()).release();
    return callback;
}

/**
 * @tc.name: NetPolicyManager001
 * @tc.desc: Test NetPolicyManager SetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager001, TestSize.Level1)
{
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(1,
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result != NetPolicyResultCode::ERR_INVALID_UID);
}

/**
 * @tc.name: NetPolicyManager002
 * @tc.desc: Test NetPolicyManager GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager002, TestSize.Level1)
{
    NetUidPolicy result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetPolicyByUid(1);
    ASSERT_TRUE(result == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyManager003
 * @tc.desc: Test NetPolicyManager GetUidsByPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager003, TestSize.Level1)
{
    std::vector<uint32_t> result;
    NetUidPolicy policy = NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND;
    result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidsByPolicy(policy);
    ASSERT_TRUE(result.size() != 0);
}

/**
 * @tc.name: NetPolicyManager004
 * @tc.desc: Test NetPolicyManager IsUidNetAccess.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager004, TestSize.Level1)
{
    bool result = DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(1, false);
    ASSERT_TRUE(result == true);
}

/**
 * @tc.name: NetPolicyManager005
 * @tc.desc: Test NetPolicyManager IsUidNetAccess.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager005, TestSize.Level1)
{
    bool result = DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(1, std::string("test"));
    ASSERT_TRUE(result == true);
}

void TrigerCallback()
{
    usleep(TRIGER_DELAY_US);
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(1,
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result != NetPolicyResultCode::ERR_INVALID_UID);
}

/**
 * @tc.name: NetPolicyManager006
 * @tc.desc: Test NetPolicyManager RegisterNetPolicyCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager006, TestSize.Level1)
{
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
    if (result == ERR_NONE) {
        std::thread trigerCallback(TrigerCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        trigerCallback.join();
        uint32_t uid = callback->GetUid();
        NetUidPolicy netPolicy = callback->GetPolicy();
        std::cout << "NetPolicyManager006 RegisterNetPolicyCallback uid:" << uid <<
            "netPolicy:" << static_cast<uint32_t>(netPolicy) << std::endl;
        ASSERT_EQ(uid, 1);
        ASSERT_EQ(netPolicy, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    } else {
        std::cout << "NetPolicyManager006 RegisterNetPolicyCallback return fail" << std::endl;
    }

    result = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager007
 * @tc.desc: Test NetPolicyManager SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager007, TestSize.Level1)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;

    NetPolicyQuotaPolicy quotaPolicy;
    quotaPolicy.netType_ = 0;
    quotaPolicy.slotId_ = TRIGER_DELAY_US;
    quotaPolicy.periodStartTime_ = TRIGER_DELAY_US;
    quotaPolicy.periodDuration_ = TEST_STRING_PERIODDURATION;
    quotaPolicy.warningBytes_ = TRIGER_DELAY_US;
    quotaPolicy.limitBytes_ = TRIGER_DELAY_US;
    quotaPolicy.lastLimitSnooze_ = -1;
    quotaPolicy.metered_ = 0;
    quotaPolicy.source_ = 0;
    quotaPolicies.push_back(quotaPolicy);

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager008
 * @tc.desc: Test NetPolicyManager GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager008, TestSize.Level1)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager009
 * @tc.desc: Test NetPolicyManager SetCellularPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager009, TestSize.Level1)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicies;

    NetPolicyCellularPolicy cellularPolicy;
    for (uint32_t i = 0; i < TEST_CONSTANT_NUM; ++i) {
        cellularPolicy.slotId_ = i;
        cellularPolicy.periodStartTime_ = TRIGER_DELAY_US + i;
        cellularPolicy.periodDuration_ = TEST_STRING_PERIODDURATION;
        cellularPolicy.title_ = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.summary_ = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.limitBytes_ = TRIGER_DELAY_US + i;
        cellularPolicy.limitAction_ = TEST_CONSTANT_NUM;
        cellularPolicy.usedBytes_ = TRIGER_DELAY_US + i;
        cellularPolicy.usedTimeDuration_ = TRIGER_DELAY_US + i;
        cellularPolicy.possessor_ = std::to_string(TRIGER_DELAY_US + i);

        cellularPolicies.push_back(cellularPolicy);
    }

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetCellularPolicies(
        cellularPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager010
 * @tc.desc: Test NetPolicyManager GetCellularPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager010, TestSize.Level1)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicies;

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetCellularPolicies(
        cellularPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager011
 * @tc.desc: Test NetPolicyManager SetFactoryPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager011, TestSize.Level1)
{
    std::string slotId = "0";

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetFactoryPolicy(slotId);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0012
 * @tc.desc: Test NetPolicyManager SetSnoozePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager012, TestSize.Level1)
{
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetSnoozePolicy(0,
        TRIGER_DELAY_US);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0013
 * @tc.desc: Test NetPolicyManager SetIdleTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager013, TestSize.Level1)
{
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetIdleTrustlist(1, true);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0014
 * @tc.desc: Test NetPolicyManager GetIdleTrustlist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager014, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetIdleTrustlist(uids);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0015
 * @tc.desc: Test NetPolicyManager SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager015, TestSize.Level1)
{
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(true);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0016
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager016, TestSize.Level1)
{
    bool result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicy();
    ASSERT_TRUE(result);
}

/**
 * @tc.name: NetPolicyManager0017
 * @tc.desc: Test NetPolicyManager GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager017, TestSize.Level1)
{
    bool result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(
        BACKGROUND_POLICY_TEST_UID);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: NetPolicyManager0018
 * @tc.desc: Test NetPolicyManager GetCurrentBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager018, TestSize.Level1)
{
    NetBackgroundPolicy result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetCurrentBackgroundPolicy();
    ASSERT_TRUE(result == NetBackgroundPolicy::NET_BACKGROUND_POLICY_DISABLE);
}
}
}
