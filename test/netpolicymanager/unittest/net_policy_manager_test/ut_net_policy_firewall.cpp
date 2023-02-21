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

#include "net_mgr_log_wrapper.h"
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_policy_firewall.h"
#include "net_policy_inner_define.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string TEST_STRING_PERIODDURATION = "M1";
const std::string ICCID_1 = "sim_abcdefg_1";
const std::string ICCID_2 = "sim_abcdefg_2";

static std::shared_ptr<NetPolicyFirewall> netPolicyFirewall_ = nullptr;

using namespace testing::ext;
class UtNetPolicyFirewall : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void UtNetPolicyFirewall::SetUpTestCase()
{
    netPolicyFirewall_ = std::make_shared<NetPolicyFirewall>();
    netPolicyFirewall_->Init();
}

void UtNetPolicyFirewall::TearDownTestCase()
{
    netPolicyFirewall_.reset();
}

void UtNetPolicyFirewall::SetUp() {}

void UtNetPolicyFirewall::TearDown() {}

/**
 * @tc.name: NetPolicyFirewall001
 * @tc.desc: Test NetPolicyFirewall SetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, NetPolicyFirewall001, TestSize.Level1)
{
    const uint32_t uid = 123;
    netPolicyFirewall_->SetDeviceIdleAllowedList(uid, false);
    std::vector<uint32_t> allowedList;
    netPolicyFirewall_->GetDeviceIdleAllowedList(allowedList);
    ASSERT_TRUE(std::find(allowedList.begin(), allowedList.end(), uid) == allowedList.end());
}

/**
 * @tc.name: NetPolicyFirewall002
 * @tc.desc: Test NetPolicyFirewall GetDeviceIdleAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, NetPolicyFirewall002, TestSize.Level1)
{
    const uint32_t uid = 456;
    netPolicyFirewall_->SetDeviceIdleAllowedList(uid, true);
    std::vector<uint32_t> allowedList;
    netPolicyFirewall_->GetDeviceIdleAllowedList(allowedList);
    ASSERT_TRUE(std::find(allowedList.begin(), allowedList.end(), uid) != allowedList.end());
}

/**
 * @tc.name: NetPolicyFirewall003
 * @tc.desc: Test NetPolicyFirewall UpdateDeviceIdlePolicy
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, NetPolicyFirewall003, TestSize.Level1)
{
    const uint32_t uid = 789;
    netPolicyFirewall_->SetDeviceIdleAllowedList(uid, true);
    netPolicyFirewall_->UpdateDeviceIdlePolicy(true);
    std::vector<uint32_t> allowedList;
    netPolicyFirewall_->GetDeviceIdleAllowedList(allowedList);
    ASSERT_TRUE(std::find(allowedList.begin(), allowedList.end(), uid) != allowedList.end());
}

/**
 * @tc.name: NetPolicyFirewall004
 * @tc.desc: Test NetPolicyFirewall ResetPolicies
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, NetPolicyFirewall004, TestSize.Level1)
{
    netPolicyFirewall_->ResetPolicies();
    std::vector<uint32_t> allowedList;
    netPolicyFirewall_->GetDeviceIdleAllowedList(allowedList);
    ASSERT_TRUE(allowedList.size() == 0);
}

/**
 * @tc.name: NetPolicyFirewall005
 * @tc.desc: Test NetPolicyFirewall HandleEvent.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, NetPolicyFirewall005, TestSize.Level1)
{
    const uint32_t uid = 101;
    netPolicyFirewall_->SetDeviceIdleAllowedList(uid, true);
    std::vector<uint32_t> allowedList;
    netPolicyFirewall_->GetDeviceIdleAllowedList(allowedList);
    ASSERT_TRUE(std::find(allowedList.begin(), allowedList.end(), uid) != allowedList.end());
    auto policyEvent = std::make_shared<PolicyEvent>();
    policyEvent->deletedUid = uid;
    netPolicyFirewall_->HandleEvent(NetPolicyEventHandler::MSG_UID_REMOVED, policyEvent);
    netPolicyFirewall_->GetDeviceIdleAllowedList(allowedList);
    ASSERT_TRUE(std::find(allowedList.begin(), allowedList.end(), uid) == allowedList.end());
}

/**
 * @tc.name: NetPolicyFirewall006
 * @tc.desc: Test NetPolicyFirewall UpdateDeviceIdlePolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, NetPolicyFirewall006, TestSize.Level1)
{
    netPolicyFirewall_->UpdateDeviceIdlePolicy(false);
    netPolicyFirewall_->UpdateDeviceIdlePolicy(true);
    int32_t ret = netPolicyFirewall_->UpdateDeviceIdlePolicy(true);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}
} // namespace NetManagerStandard
} // namespace OHOS
