/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_firewall.h"
#include "net_policy_rule.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t WAIT_TIME_SECOND_LONG = 10;
constexpr int32_t WAIT_TIME_THIRTY_SECOND_LONG = 30;
constexpr int32_t INVALID_VALUE = 100;
constexpr uint32_t TEST_UID1 = 200;
constexpr uint32_t TEST_UID2 = 13000;
std::shared_ptr<NetPolicyRule> g_netPolicyRule = nullptr;
std::shared_ptr<NetPolicyFirewall> g_netPolicyFirewallR = nullptr;
} // namespace

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
    g_netPolicyRule = std::make_shared<NetPolicyRule>();
    g_netPolicyFirewallR = std::make_shared<NetPolicyFirewall>();
    g_netPolicyRule->Init();
    g_netPolicyRule->ResetPolicies();
}

void UtNetPolicyRule::TearDownTestCase()
{
    g_netPolicyRule->TransPolicyToRule(TEST_UID2, NetUidPolicy::NET_POLICY_NONE);
    g_netPolicyRule->TransPolicyToRule(TEST_UID1, NetUidPolicy::NET_POLICY_NONE);
    g_netPolicyRule.reset();
}

void UtNetPolicyRule::SetUp() {}

void UtNetPolicyRule::TearDown() {}

sptr<NetPolicyCallbackTest> UtNetPolicyRule::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callbackR = new (std::nothrow) NetPolicyCallbackTest();
    return callbackR;
}

/**
 * @tc.name: NetPolicyRule001
 * @tc.desc: Test NetPolicyRule TransPolicyToRule.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule001, TestSize.Level1)
{
    int32_t result = g_netPolicyRule->TransPolicyToRule(10000, 1);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyRule002
 * @tc.desc: Test NetPolicyRule IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule002, TestSize.Level1)
{
    int32_t result = g_netPolicyRule->TransPolicyToRule(15000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    bool isAllowed = false;
    g_netPolicyRule->IsUidNetAllowed(15000, false, isAllowed);
    ASSERT_TRUE(isAllowed);
}

/**
 * @tc.name: NetPolicyRule003
 * @tc.desc: Test NetPolicyRule GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule003, TestSize.Level1)
{
    int32_t result = g_netPolicyRule->TransPolicyToRule(16000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    uint32_t policy = 0;
    g_netPolicyRule->GetPolicyByUid(16000, policy);
    ASSERT_EQ(policy, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyRule004
 * @tc.desc: Test NetPolicyRule GetUidsByPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule004, TestSize.Level1)
{
    int32_t result = g_netPolicyRule->TransPolicyToRule(16000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    int32_t result2 = g_netPolicyRule->TransPolicyToRule(17000, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
    int32_t result3 = g_netPolicyRule->TransPolicyToRule(18000, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(result3, NETMANAGER_SUCCESS);
    int32_t result4 = g_netPolicyRule->TransPolicyToRule(19000, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(result4, NETMANAGER_SUCCESS);

    std::vector<uint32_t> uids;
    g_netPolicyRule->GetUidsByPolicy(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND, uids);

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
    g_netPolicyRule->GetUidsByPolicy(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND, uids);
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
    int32_t result = g_netPolicyRule->TransPolicyToRule(TEST_UID2, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);

    int32_t result2 = g_netPolicyRule->SetBackgroundPolicy(false);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);

    int32_t result3 = g_netPolicyRule->ResetPolicies();
    ASSERT_EQ(result3, NETMANAGER_SUCCESS);

    uint32_t policy = 0;
    g_netPolicyRule->GetPolicyByUid(TEST_UID2, policy);
    ASSERT_EQ(policy, NET_POLICY_NONE);
    bool backgroundPolicy = false;
    g_netPolicyRule->GetBackgroundPolicy(backgroundPolicy);
    ASSERT_TRUE(backgroundPolicy);
}

/**
 * @tc.name: NetPolicyRule006
 * @tc.desc: Test NetPolicyRule SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule006, TestSize.Level1)
{
    int32_t result = g_netPolicyRule->TransPolicyToRule(TEST_UID2, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    uint32_t backgroundPolicyOfUid = 0;
    g_netPolicyRule->GetBackgroundPolicyByUid(TEST_UID2, backgroundPolicyOfUid);
    ASSERT_EQ(backgroundPolicyOfUid, NetBackgroundPolicy::NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyRule007
 * @tc.desc: Test NetPolicyRule GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule007, TestSize.Level1)
{
    g_netPolicyRule->SetBackgroundPolicy(false);
    int32_t result = g_netPolicyRule->SetBackgroundPolicy(true);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    bool backgroundPolicy;
    g_netPolicyRule->GetBackgroundPolicy(backgroundPolicy);
    ASSERT_TRUE(backgroundPolicy);
}

void SetPolicyUid()
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPolicyByUid(
        TEST_UID1, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

void SendMessage()
{
    NetManagerBaseAccessToken token;
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdlePolicy(true);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdleTrustlist({TEST_UID1}, true);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyRule008
 * @tc.desc: Test NetPolicyRule HandleEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRule008, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdlePolicy(false);
    sptr<NetPolicyCallbackTest> callback = GetINetPolicyCallbackSample();
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
    uint32_t rule = 0;
    uint32_t rule2 = 0;
    if (result == NETMANAGER_SUCCESS) {
        std::thread setPolicy(SetPolicyUid);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        setPolicy.join();
        rule = callback->GetRule();
        std::cout << "rule:" << rule << std::endl;
    } else {
        std::cout << "RegisterNetPolicyCallback failed!" << std::endl;
    }
    NetManagerBaseAccessToken token1;
    int32_t result2 = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    ASSERT_EQ(result2, NETMANAGER_SUCCESS);

    NetManagerBaseAccessToken token2;
    sptr<NetPolicyCallbackTest> callbackR = GetINetPolicyCallbackSample();
    int32_t result3 = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callbackR);
    if (result3 == NETMANAGER_SUCCESS) {
        std::thread sendMessage(SendMessage);
        callbackR->WaitFor(WAIT_TIME_THIRTY_SECOND_LONG);
        sendMessage.join();
        rule2 = callbackR->GetRule();
        std::cout << "rule2:" << rule2 << std::endl;
        ASSERT_TRUE(rule2 != rule);
    } else {
        std::cout << "RegisterNetPolicyCallbackR failed!" << std::endl;
    }
    NetManagerBaseAccessToken token3;
    int32_t result4 = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callbackR);
    ASSERT_EQ(result4, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetPolicyRuleBranchTest001
 * @tc.desc: Test NetPolicyRule Branch.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyRule, NetPolicyRuleBranchTest001, TestSize.Level1)
{
    g_netPolicyRule->DeleteUid(TEST_UID2);

    auto policyEvent = std::make_shared<PolicyEvent>();
    int32_t eventId = static_cast<int32_t>(NetPolicyEventHandler::MSG_UID_REMOVED);
    g_netPolicyRule->HandleEvent(eventId, policyEvent);

    eventId = static_cast<int32_t>(NetPolicyEventHandler::MSG_UID_STATE_FOREGROUND);
    g_netPolicyRule->HandleEvent(eventId, policyEvent);

    eventId = static_cast<int32_t>(NetPolicyEventHandler::MSG_UID_STATE_BACKGROUND);
    g_netPolicyRule->HandleEvent(eventId, policyEvent);

    eventId = INVALID_VALUE;
    g_netPolicyRule->HandleEvent(eventId, policyEvent);

    g_netPolicyRule->UpdateForegroundUidList(TEST_UID2, false);
    g_netPolicyRule->UpdateForegroundUidList(TEST_UID2, true);

    std::string message = "";
    g_netPolicyRule->GetDumpMessage(message);

    bool ret = g_netPolicyRule->IsValidNetPolicy(INVALID_VALUE);
    ASSERT_FALSE(ret);
}
} // namespace NetManagerStandard
} // namespace OHOS
