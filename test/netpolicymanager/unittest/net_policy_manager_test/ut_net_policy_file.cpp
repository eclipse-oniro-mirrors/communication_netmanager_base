/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <random>
#include <thread>
#include <unistd.h>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_policy_file.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_LIST_SIZE = 10;
constexpr uint32_t SLEEP_SECOND_TIME = 5;
} // namespace
std::shared_ptr<NetPolicyFile> netPolicyFile_ = nullptr;

using namespace testing::ext;
class UtNetPolicyFile : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::set<uint32_t> white_;
    std::set<uint32_t> black_;
};

void UtNetPolicyFile::SetUpTestCase()
{
    netPolicyFile_ = DelayedSingleton<NetPolicyFile>::GetInstance();
    ASSERT_TRUE(DelayedSingleton<NetPolicyFile>::GetInstance());
    netPolicyFile_->InitPolicy();
}

void UtNetPolicyFile::TearDownTestCase()
{
    sleep(SLEEP_SECOND_TIME);
    netPolicyFile_.reset();
}

void UtNetPolicyFile::SetUp()
{
    netPolicyFile_->ReadFirewallRules(FIREWALL_CHAIN_DEVICE_IDLE, white_, black_);
}

void UtNetPolicyFile::TearDown()
{
    netPolicyFile_->WriteFirewallRules(FIREWALL_CHAIN_DEVICE_IDLE, white_, black_);
}

/**
 * @tc.name: NetPolicyFile001
 * @tc.desc: Test NetPolicyFile ReadFirewallRules.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFile, NetPolicyFile001, TestSize.Level1)
{
    std::set<uint32_t> allowedList;
    std::set<uint32_t> deniedList;
    for (uint32_t i = 0; i <= MAX_LIST_SIZE; i++) {
        allowedList.insert(i);
        deniedList.insert(i);
    }

    netPolicyFile_->WriteFirewallRules(FIREWALL_CHAIN_DEVICE_IDLE, allowedList, deniedList);
    std::set<uint32_t> allowedList1;
    std::set<uint32_t> deniedList1;
    netPolicyFile_->ReadFirewallRules(FIREWALL_CHAIN_DEVICE_IDLE, allowedList1, deniedList1);
    ASSERT_TRUE(allowedList == allowedList1);
    ASSERT_TRUE(deniedList == deniedList1);
}

/**
 * @tc.name: NetPolicyFile002
 * @tc.desc: Test NetPolicyFile WriteFirewallRules.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFile, NetPolicyFile002, TestSize.Level1)
{
    std::set<uint32_t> allowedList;
    std::set<uint32_t> deniedList;
    for (uint32_t i = 0; i <= MAX_LIST_SIZE; i++) {
        allowedList.insert(i);
        deniedList.insert(i);
    }
    netPolicyFile_->WriteFirewallRules(FIREWALL_CHAIN_DEVICE_IDLE, allowedList, deniedList);
    sleep(SLEEP_SECOND_TIME);
    netPolicyFile_->InitPolicy();
    std::set<uint32_t> allowedList1;
    std::set<uint32_t> deniedList1;
    netPolicyFile_->ReadFirewallRules(FIREWALL_CHAIN_DEVICE_IDLE, allowedList1, deniedList1);
    ASSERT_TRUE(allowedList == allowedList1);
    ASSERT_TRUE(deniedList == deniedList1);
}

/**
 * @tc.name: SetDeviceIdleTrustlist001
 * @tc.desc: Test SetDeviceIdleTrustlist Func.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, SetDeviceIdleTrustlist001, TestSize.Level1)
{
    const uint32_t uid = 101;
    std::vector<uint32_t> initialUids(1001);
    for (size_t i = 0; i < initialUids.size(); ++i) {
        initialUids[i] = i;
    }
    for (uint32_t testuid : initialUids) {
        netPolicyFirewall_->powerSaveAllowedList_.insert(testuid);
    }
    auto result = netPolicyFirewall_->SetDeviceIdleTrustlist({uid}, true);
    EXPECT_EQ(result, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: UpdateFirewallPolicyList001
 * @tc.desc: Test UpdateFirewallPolicyList Func.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, UpdateFirewallPolicyList001, TestSize.Level1)
{
    uint32_t chainType = 17;
    std::vector<uint32_t> uids = {1, 2, 3};
    bool isAllowed = true;
    netPolicyFirewall_->powerSaveAllowedList_.clear();
    netPolicyFirewall_->UpdateFirewallPolicyList(chainType, uids, isAllowed);
    EXPECT_EQ(netPolicyFirewall_->powerSaveAllowedList_.size(), 3);
}

/**
 * @tc.name: UpdatePowerSavePolicy001
 * @tc.desc: Test UpdatePowerSavePolicy Func.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, UpdatePowerSavePolicy001, TestSize.Level1)
{
    bool enable = false;
    netPolicyFirewall_->powerSaveMode_ = true;
    auto result = netPolicyFirewall_->UpdatePowerSavePolicy(enable);
    EXPECT_NE(result, NETMANAGER_ERR_STATUS_EXIST);
    EXPECT_NE(result, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

/**
 * @tc.name: ResetPolicies001
 * @tc.desc: Test ResetPolicies ResetPolicies Func.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, ResetPolicies001, TestSize.Level1)
{
    EXPECT_NE(netPolicyFirewall_->powerSaveFirewallRule_, nullptr);
    netPolicyFirewall_->powerSaveFirewallRule_ = nullptr;
    netPolicyFirewall_->ResetPolicies();
}

/**
 * @tc.name: HandleEvent001
 * @tc.desc: Test NetPolicyFirewall HandleEvent Func.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFirewall, HandleEvent001, TestSize.Level1)
{
    const uint32_t uid = 101;
    netPolicyFirewall_->SetDeviceIdleTrustlist({uid}, true);
    std::vector<uint32_t> allowedList;
    netPolicyFirewall_->GetDeviceIdleTrustlist(allowedList);
    ASSERT_TRUE(std::find(allowedList.begin(), allowedList.end(), uid) != allowedList.end());
    auto policyEvent = std::make_shared<PolicyEvent>();
    netPolicyFirewall_->HandleEvent(NetPolicyEventHandler::MSG_POWER_SAVE_MODE_CHANGED, policyEvent);
    netPolicyFirewall_->HandleEvent(NetPolicyEventHandler::MSG_DEVICE_IDLE_MODE_CHANGED, policyEvent);
    netPolicyFirewall_->HandleEvent(NetPolicyEventHandler::MSG_DEVICE_IDLE_LIST_UPDATED, policyEvent);
    ASSERT_FALSE(std::find(allowedList.begin(), allowedList.end(), uid) == allowedList.end());
}
} // namespace NetManagerStandard
} // namespace OHOS
