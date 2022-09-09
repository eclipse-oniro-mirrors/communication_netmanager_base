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
#include "net_policy_inner_define.h"
#include "net_policy_client.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string TEST_STRING_PERIODDURATION = "M1";
constexpr uint32_t NET_TEST_UID = 20010035;
constexpr uint32_t LIMIT_BYTES = 5000000;
constexpr uint32_t WARNING_BYTES = 20000;

using namespace testing::ext;
class NetPolicyManualTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void NetPolicyManualTest::SetUpTestCase() {}

void NetPolicyManualTest::TearDownTestCase() {}

void NetPolicyManualTest::SetUp() {}

void NetPolicyManualTest::TearDown() {}

sptr<NetPolicyCallbackTest> NetPolicyManualTest::GetINetPolicyCallbackSample() const
{
    sptr<NetPolicyCallbackTest> callback = (std::make_unique<NetPolicyCallbackTest>()).release();
    return callback;
}

static void SetNetQuotaPolicy(int step)
{
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.periodDuration = "M1";
    quotaPolicy1.netType = NetBearType::BEARER_WIFI; // NetBearType::BEARER_WIFI
    quotaPolicy1.warningBytes = WARNING_BYTES;
    quotaPolicy1.limitBytes = LIMIT_BYTES;
    quotaPolicy1.metered = true;
    quotaPolicy1.ident = "";
    quotaPolicy1.limitAction = LimitAction::LIMIT_ACTION_AUTO_BILL;
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    // set quota policy
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
}

/**
 * @tc.name: NetPolicyManualCase1
 * @tc.desc: Set net quota, allow uid metered background, enable data saver, get the background policy of the uid.
 *      expect: use the uid traffic , to see if can be disable net after 2M used.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase1, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase1 client: " << client << std::endl;
    // Make sure the UID
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // For now, can't get the foreground state
    // set background allow
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    // get the uid policy and check
    uint32_t policy = client->GetPolicyByUid(NET_TEST_UID);
    std::cout << "policy: " << policy << std::endl;
    ASSERT_TRUE(policy == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    SetNetQuotaPolicy(1);
    // dis-allow background
    client->SetBackgroundPolicy(false);
    ASSERT_TRUE(client->GetBackgroundPolicy() == false);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) ==
                NetBackgroundPolicy::NET_BACKGROUND_POLICY_ALLOWEDLIST);
}

/**
 * @tc.name: NetPolicyManualCase2
 * @tc.desc: Reject uid metered backgroud, enable data saver, get the background policy of the uid.
 *      expected： can't access net
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase2, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase2 client: " << client << std::endl;
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // set reject metered background
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    // get the uid policy and check
    uint32_t policy = client->GetPolicyByUid(NET_TEST_UID);
    std::cout << "policy: " << policy << std::endl;
    ASSERT_TRUE(policy == NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    SetNetQuotaPolicy(1);
    // dis-allow background
    client->SetBackgroundPolicy(false);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) == NetBackgroundPolicy::NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyManualCase3
 * @tc.desc: Set uid policy none, enable data saver, get the background policy of the uid.
 *      expect: app can't access net.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase3, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase3 client: " << client << std::endl;
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_NONE);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    uint32_t policy = client->GetPolicyByUid(NET_TEST_UID);
    std::cout << "policy: " << policy << std::endl;
    ASSERT_TRUE(policy == NetUidPolicy::NET_POLICY_NONE);
    SetNetQuotaPolicy(1);
    // dis-allow background
    client->SetBackgroundPolicy(false);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) == NetBackgroundPolicy::NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyManualCase4
 * @tc.desc: .Set net policy none, disable data saver, get the background policy of the uid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase4, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase4 client: " << client << std::endl;
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_NONE);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    SetNetQuotaPolicy(6);
    // allow background
    client->SetBackgroundPolicy(true);
    // Expect: can access net.
    ASSERT_TRUE(client->GetBackgroundPolicy() == true);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) == NetBackgroundPolicy::NET_BACKGROUND_POLICY_ENABLE);
}

/**
 * @tc.name: NetPolicyManualCase5
 * @tc.desc: Reject metered background, disable date saver, get the background policy of uid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase5, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase5 client: " << client << std::endl;
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    // allow background.
    client->SetBackgroundPolicy(true);
    // expect: can access net
    ASSERT_TRUE(client->GetBackgroundPolicy() == true);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) == NetBackgroundPolicy::NET_BACKGROUND_POLICY_DISABLE);
}

/**
 * @tc.name: NetPolicyManualCase6
 * @tc.desc: Allow metered background, disable data saver, get the background policy of the uid.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase6, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase6 client: " << client << std::endl;
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    // allow background
    client->SetBackgroundPolicy(true);
    // expect: can access net
    ASSERT_TRUE(client->GetBackgroundPolicy() == true);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) == NetBackgroundPolicy::NET_BACKGROUND_POLICY_ENABLE);
}

/**
 * @tc.desc: Set device idle allow list and enable firewall.
 *      expect: app can access net
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase7, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase7 client: " << client << std::endl;
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // add uid to device idle allow list
    client->SetDeviceIdleAllowedList(NET_TEST_UID, true);
    // enable device idle firewall
    client->SetDeviceIdlePolicy(true);
}

/**
 * @tc.desc: Remove device idle allow list and enable firewall.
 *      can't access net.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase8, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase8 client: " << client << std::endl;
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // remove from device idle allow list
    client->SetDeviceIdleAllowedList(NET_TEST_UID, false);
    // enable device idle firewall
    client->SetDeviceIdlePolicy(true);
}

/**
 * @tc.desc: Set device idle allow list and disable firewall.
 *      expect: can access net.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase9, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase9 client: " << client << std::endl;
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // add uid to device idle allow list
    client->SetDeviceIdleAllowedList(NET_TEST_UID, true);
    // disable device idle firewall.
    client->SetDeviceIdlePolicy(false);
}

/**
 * @tc.desc: Remove device idle allow list and disable firewall.
 *      expect: can access net
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase10, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase10 client: " << client << std::endl;
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // remove uid from device idle allow list
    client->SetDeviceIdleAllowedList(NET_TEST_UID, false);
    // disable device idle firewall
    client->SetDeviceIdlePolicy(false);
}

/**
 * @tc.desc: Reset all things.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase100, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManager0033 client: " << client << std::endl;
    client->ResetPolicies("");
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.clear();
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
}

/**
 * @tc.desc: Set Device Idle Policy false.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase200, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase200 client: " << client << std::endl;
    client->SetDeviceIdlePolicy(false);
}

/**
 * @tc.desc: Set Device Idle Policy true.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase300, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase300 client: " << client << std::endl;
    client->SetDeviceIdlePolicy(true);
}

/**
 * @tc.desc: Update Remind Policy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase400, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase400 client: " << client << std::endl;
    client->UpdateRemindPolicy(1, "", RemindType::REMIND_TYPE_LIMIT);
}

static void SetNetQuotaPolicy2(int step)
{
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.iccid = "-1";
    quotaPolicy1.periodDuration = "M1";
    quotaPolicy1.netType = NetBearType::BEARER_CELLULAR; // NetBearType::BEARER_CELLULAR
    quotaPolicy1.warningBytes = WARNING_BYTES;
    quotaPolicy1.limitBytes = LIMIT_BYTES;
    quotaPolicy1.metered = true;
    quotaPolicy1.ident = "simId";
    quotaPolicy1.limitAction = LimitAction::LIMIT_ACTION_AUTO_BILL;
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    // set quota policy
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
}

/**
 * @tc.desc: Update Remind Policy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase500, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase1 client: " << client << std::endl;
    // Make sure the UID
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // For now, can't get the foreground state
    // set background allow
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    // get the uid policy and check
    uint32_t policy = client->GetPolicyByUid(NET_TEST_UID);
    std::cout << "policy: " << policy << std::endl;
    ASSERT_TRUE(policy == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    SetNetQuotaPolicy2(1);
    // dis-allow background
    client->SetBackgroundPolicy(false);
    ASSERT_TRUE(client->GetBackgroundPolicy() == false);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) ==
                NetBackgroundPolicy::NET_BACKGROUND_POLICY_ALLOWEDLIST);
}

/**
 * @tc.desc: Update Remind Policy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase600, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase600 client: " << client << std::endl;
    client->UpdateRemindPolicy(0, "", RemindType::REMIND_TYPE_LIMIT);
}

static void SetNetQuotaPolicy3(int step)
{
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.periodDuration = "M1";
    quotaPolicy1.netType = NetBearType::BEARER_ETHERNET; // NetBearType::BEARER_ETHERNET
    quotaPolicy1.warningBytes = WARNING_BYTES;
    quotaPolicy1.limitBytes = LIMIT_BYTES;
    quotaPolicy1.metered = true;
    quotaPolicy1.limitAction = LimitAction::LIMIT_ACTION_AUTO_BILL;
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    // set quota policy
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetQuotaPolicies(quotaPolicies);
}

/**
 * @tc.desc: Update Remind Policy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase700, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase1 client: " << client << std::endl;
    // Make sure the UID
    std::cout << "UID: " << NET_TEST_UID << std::endl;
    // For now, can't get the foreground state
    // set background allow
    int32_t result = client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    // get the uid policy and check
    uint32_t policy = client->GetPolicyByUid(NET_TEST_UID);
    std::cout << "policy: " << policy << std::endl;
    ASSERT_TRUE(policy == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    SetNetQuotaPolicy3(1);
    // dis-allow background
    client->SetBackgroundPolicy(false);
    ASSERT_TRUE(client->GetBackgroundPolicy() == false);
    ASSERT_TRUE(client->GetBackgroundPolicyByUid(NET_TEST_UID) ==
                NetBackgroundPolicy::NET_BACKGROUND_POLICY_ALLOWEDLIST);
}

/**
 * @tc.desc: Update Remind Policy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManualTest, NetPolicyManualCase800, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase800 client: " << client << std::endl;
    client->UpdateRemindPolicy(3, "", RemindType::REMIND_TYPE_LIMIT);
}

HWTEST_F(NetPolicyManualTest, NetPolicyManualCase900, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.periodDuration = "M1";
    quotaPolicy1.netType = NetBearType::BEARER_ETHERNET; // NetBearType::BEARER_ETHERNET
    quotaPolicy1.warningBytes = WARNING_BYTES;
    quotaPolicy1.limitBytes = LIMIT_BYTES;
    quotaPolicy1.metered = true;
    quotaPolicy1.ident = "eth1";
    quotaPolicy1.limitAction = LimitAction::LIMIT_ACTION_AUTO_BILL;
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    // set quota policy
    client->SetNetQuotaPolicies(quotaPolicies);
}

HWTEST_F(NetPolicyManualTest, NetPolicyManualCase1000, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase800 client: " << client << std::endl;
    client->SetBackgroundPolicy(true);
}

HWTEST_F(NetPolicyManualTest, NetPolicyManualCase1100, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase800 client: " << client << std::endl;
    client->SetBackgroundPolicy(false);
}

HWTEST_F(NetPolicyManualTest, NetPolicyManualCase1200, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase800 client: " << client << std::endl;
    client->SetDeviceIdleAllowedList(NET_TEST_UID, false);
}

HWTEST_F(NetPolicyManualTest, NetPolicyManualCase1300, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase800 client: " << client << std::endl;
    client->SetDeviceIdleAllowedList(NET_TEST_UID, true);
}

HWTEST_F(NetPolicyManualTest, NetPolicyManualCase1400, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    std::cout << "NetPolicyManualCase800 client: " << client << std::endl;
    client->SetPolicyByUid(NET_TEST_UID, NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND);
}

HWTEST_F(NetPolicyManualTest, NetPolicyManualCase1500, TestSize.Level1)
{
    auto client = DelayedSingleton<NetPolicyClient>::GetInstance();
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.periodDuration = "M1";
    quotaPolicy1.netType = NetBearType::BEARER_ETHERNET; // NetBearType::BEARER_ETHERNET
    quotaPolicy1.warningBytes = WARNING_BYTES;
    quotaPolicy1.limitBytes = LIMIT_BYTES;
    quotaPolicy1.metered = true;
    quotaPolicy1.ident = "eth0";
    quotaPolicy1.limitAction = LimitAction::LIMIT_ACTION_AUTO_BILL;
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    // set quota policy
    client->SetNetQuotaPolicies(quotaPolicies);
}
}
}
