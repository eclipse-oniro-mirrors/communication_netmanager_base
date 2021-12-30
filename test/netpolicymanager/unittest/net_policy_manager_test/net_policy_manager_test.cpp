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
 * @tc.desc: Test NetPolicyManager SetUidPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager001, TestSize.Level1)
{
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetUidPolicy(1,
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result != NetPolicyResultCode::ERR_INVALID_UID);
}

/**
 * @tc.name: NetPolicyManager002
 * @tc.desc: Test NetPolicyManager GetUidPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager002, TestSize.Level1)
{
    NetUidPolicy result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidPolicy(1);
    ASSERT_TRUE(result == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyManager003
 * @tc.desc: Test NetPolicyManager GetUids.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager003, TestSize.Level1)
{
    std::vector<uint32_t> result;
    NetUidPolicy policy = NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND;
    result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUids(policy);
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
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetUidPolicy(1,
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
 * @tc.desc: Test NetPolicyManager SetNetPolicys.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager007, TestSize.Level1)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicys;

    NetPolicyQuotaPolicy quotaPolicy;
    for (uint8_t i = 0; i < TEST_CONSTANT_NUM; ++i) {
        quotaPolicy.netType_ = static_cast<uint8_t>(NetQuotaPolicyType::NET_POLICY_MOBILE) + i;
        quotaPolicy.subscriberId_ = std::to_string(TRIGER_DELAY_US + i);
        quotaPolicy.periodStartTime_ = TRIGER_DELAY_US + i;
        quotaPolicy.periodDuration_ = TEST_STRING_PERIODDURATION;
        quotaPolicy.warningBytes_ = TRIGER_DELAY_US + i;
        quotaPolicy.limitBytes_ = TRIGER_DELAY_US + i;
        quotaPolicy.lastLimitSnooze_ = TRIGER_DELAY_US + i;
        quotaPolicy.metered_ = i;
        quotaPolicy.source_ = i;

        quotaPolicys.push_back(quotaPolicy);
    }

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetPolicys(quotaPolicys);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager008
 * @tc.desc: Test NetPolicyManager GetNetPolicys.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager008, TestSize.Level1)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicys;

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetPolicys(quotaPolicys);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager009
 * @tc.desc: Test NetPolicyManager SetCellularPolicys.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager009, TestSize.Level1)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicys;

    NetPolicyCellularPolicy cellularPolicy;
    for (uint32_t i = 0; i < TEST_CONSTANT_NUM; ++i) {
        cellularPolicy.subscriberId_ = std::to_string(static_cast<uint32_t>(NetQuotaPolicyType::NET_POLICY_MOBILE) + i);
        cellularPolicy.periodStartTime_ = TRIGER_DELAY_US + i;
        cellularPolicy.periodDuration_ = TEST_STRING_PERIODDURATION;
        cellularPolicy.title_ = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.summary_ = std::to_string(TRIGER_DELAY_US + i);
        cellularPolicy.limitBytes_ = TRIGER_DELAY_US + i;
        cellularPolicy.limitAction_ = TEST_CONSTANT_NUM;
        cellularPolicy.usedBytes_ = TRIGER_DELAY_US + i;
        cellularPolicy.usedTimeDuration_ = TRIGER_DELAY_US + i;
        cellularPolicy.possessor_ = std::to_string(TRIGER_DELAY_US + i);

        cellularPolicys.push_back(cellularPolicy);
    }

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetCellularPolicys(cellularPolicys);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager010
 * @tc.desc: Test NetPolicyManager GetCellularPolicys.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager010, TestSize.Level1)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicys;

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetCellularPolicys(cellularPolicys);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager011
 * @tc.desc: Test NetPolicyManager ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager011, TestSize.Level1)
{
    std::string subscriberId=std::to_string(static_cast<uint32_t>(NetQuotaPolicyType::NET_POLICY_MOBILE));

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetFactory(subscriberId);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0012
 * @tc.desc: Test NetPolicyManager SnoozePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager012, TestSize.Level1)
{
    NetPolicyQuotaPolicy quotaPolicy;
    quotaPolicy.netType_ = static_cast<uint8_t>(NetQuotaPolicyType::NET_POLICY_MOBILE);
    quotaPolicy.subscriberId_ = std::to_string(TRIGER_DELAY_US);
    quotaPolicy.periodStartTime_ = TRIGER_DELAY_US;
    quotaPolicy.periodDuration_ = std::to_string(TRIGER_DELAY_US);
    quotaPolicy.warningBytes_ = TRIGER_DELAY_US;
    quotaPolicy.limitBytes_ = TRIGER_DELAY_US;
    quotaPolicy.lastLimitSnooze_ = TRIGER_DELAY_US;
    quotaPolicy.metered_ = 0;
    quotaPolicy.source_ = 0;

    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SnoozePolicy(quotaPolicy);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0013
 * @tc.desc: Test NetPolicyManager SetIdleWhitelist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager013, TestSize.Level1)
{
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetIdleWhitelist(1, true);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyManager0014
 * @tc.desc: Test NetPolicyManager GetIdleWhitelist.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager014, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetIdleWhitelist(uids);
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
    bool result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetCurrentBackgroundPolicy();
    ASSERT_TRUE(result);
}
}
}
