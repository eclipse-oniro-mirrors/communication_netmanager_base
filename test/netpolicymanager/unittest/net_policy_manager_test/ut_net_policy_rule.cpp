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

#include <gtest/gtest.h>

#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_firewall.h"
#include "net_policy_rule.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t WAIT_TIME_SECOND_LONG = 10;
constexpr int32_t WAIT_TIME_THIRTY_SECOND_LONG = 30;
constexpr uint32_t TEST_UID1 = 200;
constexpr uint32_t TEST_UID2 = 13000;
std::shared_ptr<NetPolicyRule> netPolicyRule_ = nullptr;
std::shared_ptr<NetPolicyFirewall> netPolicyFirewallR_ = nullptr;

using namespace testing::ext;
class UtNetPolicyRule : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void UtNetPolicyRule::SetUpTestCase()
{
    netPolicyRule_ = std::make_shared<NetPolicyRule>();
    netPolicyFirewallR_ = std::make_shared<NetPolicyFirewall>();
    netPolicyRule_->Init();
}

void UtNetPolicyRule::TearDownTestCase()
{
    netPolicyRule_->TransPolicyToRule(TEST_UID2, NetUidPolicy::NET_POLICY_NONE);
    netPolicyRule_.reset();
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID1, NetUidPolicy::NET_POLICY_NONE);
}

void UtNetPolicyRule::SetUp() {}

void UtNetPolicyRule::TearDown() {}

sptr<NetPolicyCallbackTest> UtNetPolicyRule::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callbackR = (std::make_unique<NetPolicyCallbackTest>()).release();
    return callbackR;
}

/**
 * @tc.name: NetPolicyRule001
 * @tc.desc: Test NetPolicyRule TransPolicyToRule.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule001, TestSize.Level1)
{
    uint32_t result = netPolicyRule_->TransPolicyToRule(10000, 1);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetPolicyRule002
 * @tc.desc: Test NetPolicyRule IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule002, TestSize.Level1)
{
    uint32_t result = netPolicyRule_->TransPolicyToRule(15000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == ERR_NONE);
    bool result2 = netPolicyRule_->IsUidNetAllowed(15000, false);
    ASSERT_TRUE(result2);
}

/**
 * @tc.name: NetPolicyRule003
 * @tc.desc: Test NetPolicyRule GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule003, TestSize.Level1)
{
    uint32_t result = netPolicyRule_->TransPolicyToRule(16000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == ERR_NONE);
    uint32_t result2 = netPolicyRule_->GetPolicyByUid(16000);
    ASSERT_TRUE(result2 == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyRule004
 * @tc.desc: Test NetPolicyRule GetUidsByPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule004, TestSize.Level1)
{
    uint32_t result = netPolicyRule_->TransPolicyToRule(16000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == ERR_NONE);
    uint32_t result2 = netPolicyRule_->TransPolicyToRule(17000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result2 == ERR_NONE);
    uint32_t result3 = netPolicyRule_->TransPolicyToRule(18000, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_TRUE(result3 == ERR_NONE);
    uint32_t result4 = netPolicyRule_->TransPolicyToRule(19000, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_TRUE(result4 == ERR_NONE);

    std::vector<uint32_t> uids = netPolicyRule_->GetUidsByPolicy(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);

    bool result5 = false;
    bool result6 = false;
    for (const auto &i : uids) {
        if (i == 16000) {
            result5 = true;
        }
    }

    for (const auto &i : uids) {
        if (i == 17000) {
            result6 = true;
        }
    }
    ASSERT_TRUE(result5 && result6);
    result5 = false;
    result6 = false;
    uids = netPolicyRule_->GetUidsByPolicy(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    for (const auto &i : uids) {
        if (i == 18000) {
            result5 = true;
        }
    }

    for (const auto &i : uids) {
        if (i == 19000) {
            result6 = true;
        }
    }
    ASSERT_TRUE(result5 && result6);
}

/**
 * @tc.name: NetPolicyRule005
 * @tc.desc: Test NetPolicyRule ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule005, TestSize.Level1)
{
    uint32_t result = netPolicyRule_->TransPolicyToRule(TEST_UID2, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);

    uint32_t result2 = netPolicyRule_->SetBackgroundPolicy(false);
    ASSERT_TRUE(result2 == NetPolicyResultCode::ERR_NONE);

    uint32_t result3 = netPolicyRule_->ResetPolicies();
    ASSERT_TRUE(result3 == NetPolicyResultCode::ERR_NONE);

    uint32_t result4 = netPolicyRule_->GetPolicyByUid(TEST_UID2);
    ASSERT_TRUE(result4 == NET_POLICY_NONE);
    ASSERT_TRUE(netPolicyRule_->GetBackgroundPolicy());
}

/**DelayedSingleton
 * @tc.name: NetPolicyRule006
 * @tc.desc: Test NetPolicyRule SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule006, TestSize.Level1)
{
    uint32_t result = netPolicyRule_->TransPolicyToRule(TEST_UID2, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    uint32_t result2 = netPolicyRule_->GetBackgroundPolicyByUid(TEST_UID2);
    ASSERT_TRUE(result2 == NetBackgroundPolicy::NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyRule007
 * @tc.desc: Test NetPolicyRule GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule007, TestSize.Level1)
{
    uint32_t result = netPolicyRule_->SetBackgroundPolicy(true);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    bool result2 = netPolicyRule_->GetBackgroundPolicy();
    ASSERT_TRUE(result2);
}

void SetPolicyUid()
{
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(TEST_UID1,
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

void SendMessage()
{
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdlePolicy(true);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdleAllowedList(TEST_UID1, false);
    ASSERT_TRUE(result2 == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyRule008
 * @tc.desc: Test NetPolicyRule HandleEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule008, TestSize.Level1)
{
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
    uint32_t rule = 0;
    uint32_t rule2 = 0;
    if (result == ERR_NONE) {
        std::thread setPolicy(SetPolicyUid);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setPolicy.join();
        rule = callback->GetRule();
        std::cout << "rule:" << rule << std::endl;
    } else {
        std::cout << "RegisterNetPolicyCallback failed!" << std::endl;
    }
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    ASSERT_TRUE(result2 == ERR_NONE);

    sptr<NetPolicyCallbackTest> callbackR = GetINetPolicyCallbackSample();
    int32_t result3 = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callbackR);
    if (result3 == ERR_NONE) {
        std::thread sendMessage(SendMessage);
        callbackR->WaitFor(WAIT_TIME_THIRTY_SECOND_LONG);
        sendMessage.join();
        rule2 = callbackR->GetRule();
        std::cout << "rule2:" << rule2 << std::endl;
        ASSERT_TRUE(rule2 != rule);
    } else {
        std::cout << "RegisterNetPolicyCallbackR failed!" << std::endl;
    }
    int32_t result4 = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callbackR);
    ASSERT_TRUE(result4 == ERR_NONE);
}
} // namespace NetManagerStandard
} // namespace OHOS
