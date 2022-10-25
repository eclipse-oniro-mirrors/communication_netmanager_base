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
#include "net_policy_constants.h"
#include "net_policy_inner_define.h"
#include "net_policy_traffic.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string TEST_STRING_PERIODDURATION = "M1";
const std::string ICCID_1 = "sim_abcdefg_1";
const std::string ICCID_2 = "sim_abcdefg_2";
constexpr uint32_t TEST_WARNING_BYTES_1 = 321;
constexpr uint32_t TEST_WARNING_BYTES_2 = 123;
constexpr uint32_t TEST_WARNING_BYTES_3 = 123456;
constexpr uint32_t TEST_LIMIT_BYTES_1 = 4321;
constexpr uint32_t TEST_LIMIT_BYTES_2 = 1234;
constexpr uint32_t TEST_LIMIT_BYTES_3 = 1234567;
constexpr uint32_t TEST_LAST_WARNING_REMIND_1 = 7654321;
constexpr uint32_t TEST_LAST_WARNING_REMIND_2 = 1234567;
constexpr uint32_t TEST_LAST_LIMIT_REMIND_1 = 87654321;
constexpr uint32_t TEST_LAST_LIMIT_REMIND_2 = 12345678;

std::shared_ptr<NetPolicyTraffic> netPolicyTraffic_ = nullptr;

using namespace testing::ext;
class UtNetPolicyTraffic : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void UtNetPolicyTraffic::SetUpTestCase()
{
    netPolicyTraffic_ = std::make_shared<NetPolicyTraffic>();
    netPolicyTraffic_->Init();
}

void UtNetPolicyTraffic::TearDownTestCase()
{
    netPolicyTraffic_.reset();
}

void UtNetPolicyTraffic::SetUp()
{
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.iccid = ICCID_1;
    quotaPolicy1.periodDuration = "M1";
    quotaPolicy1.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy1.warningBytes = TEST_WARNING_BYTES_1;
    quotaPolicy1.limitBytes = TEST_LIMIT_BYTES_1;
    quotaPolicy1.lastWarningRemind = TEST_LAST_WARNING_REMIND_1;
    quotaPolicy1.lastLimitRemind = TEST_LAST_LIMIT_REMIND_1;
    quotaPolicy1.metered = true;
    quotaPolicy1.limitAction = LimitAction::LIMIT_ACTION_AUTO_BILL;

    NetQuotaPolicy quotaPolicy2;
    quotaPolicy2.iccid = ICCID_2;
    quotaPolicy2.periodDuration = "Y1";
    quotaPolicy2.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy2.warningBytes = TEST_WARNING_BYTES_2;
    quotaPolicy2.limitBytes = TEST_LIMIT_BYTES_2;
    quotaPolicy2.lastWarningRemind = TEST_LAST_WARNING_REMIND_2;
    quotaPolicy2.lastLimitRemind = TEST_LAST_LIMIT_REMIND_2;
    quotaPolicy2.metered = true;
    quotaPolicy2.limitAction = LimitAction::LIMIT_ACTION_DISABLE;

    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    quotaPolicies.push_back(quotaPolicy2);
    netPolicyTraffic_->UpdateQuotaPolicies(quotaPolicies);
}

void UtNetPolicyTraffic::TearDown()
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    netPolicyTraffic_->UpdateQuotaPolicies(quotaPolicies);
}

/**
 * @tc.name: NetPolicyTraffic001
 * @tc.desc: Test NetPolicyTraffic UpdateQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, NetPolicyTraffic001, TestSize.Level1)
{
    NetQuotaPolicy quotaPolicy1;
    quotaPolicy1.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy1.iccid = ICCID_1;
    quotaPolicy1.periodDuration = "M1";
    quotaPolicy1.warningBytes = TEST_WARNING_BYTES_3;
    quotaPolicy1.limitBytes = TEST_LIMIT_BYTES_3;
    quotaPolicy1.lastLimitRemind = -1;
    quotaPolicy1.metered = 0;
    quotaPolicy1.source = 0;

    NetQuotaPolicy quotaPolicy2;
    quotaPolicy2.netType = NetBearType::BEARER_CELLULAR;
    quotaPolicy2.iccid = ICCID_2;
    quotaPolicy2.periodDuration = "Y1";
    quotaPolicy2.warningBytes = TEST_WARNING_BYTES_3;
    quotaPolicy2.limitBytes = TEST_LIMIT_BYTES_3;
    quotaPolicy2.lastLimitRemind = -1;
    quotaPolicy2.metered = 0;
    quotaPolicy2.source = 0;
    std::vector<NetQuotaPolicy> quotaPolicies;
    quotaPolicies.push_back(quotaPolicy1);
    quotaPolicies.push_back(quotaPolicy2);
    int32_t result = netPolicyTraffic_->UpdateQuotaPolicies(quotaPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
}

/**
 * @tc.name: NetPolicyTraffic002
 * @tc.desc: Test NetPolicyTraffic GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, NetPolicyTraffic002, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t result = netPolicyTraffic_->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    ASSERT_TRUE(quotaPolicies.size() > 0);
}

/**
 * @tc.name: NetPolicyTraffic003
 * @tc.desc: Test NetPolicyTraffic UpdateRemindPolicy REMIND_TYPE_LIMIT
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, NetPolicyTraffic003, TestSize.Level1)
{
    NETMGR_LOG_E("NetPolicyTraffic003");
    int32_t result = netPolicyTraffic_->UpdateRemindPolicy(
        NetBearType::BEARER_CELLULAR, ICCID_1, RemindType::REMIND_TYPE_LIMIT);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    std::vector<NetQuotaPolicy> quotaPolicies;
    result = netPolicyTraffic_->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    for (auto &quotaPolicy : quotaPolicies) {
        if (quotaPolicy.netType == NetBearType::BEARER_CELLULAR && quotaPolicy.iccid == ICCID_1) {
            if (quotaPolicy.lastLimitRemind < 0) {
                break;
            }
            auto now = time(nullptr);
            ASSERT_TRUE(now - quotaPolicy.lastLimitRemind < 100);
            return;
        }
    }
    ASSERT_TRUE(false);
}

/**
 * @tc.name: NetPolicyTraffic004
 * @tc.desc: Test NetPolicyTraffic UpdateRemindPolicy REMIND_TYPE_WARNING
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, NetPolicyTraffic004, TestSize.Level1)
{
    int32_t result =
        netPolicyTraffic_->UpdateRemindPolicy(NetBearType::BEARER_CELLULAR, ICCID_2, RemindType::REMIND_TYPE_WARNING);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    std::vector<NetQuotaPolicy> quotaPolicies;
    result = netPolicyTraffic_->GetNetQuotaPolicies(quotaPolicies);
    ASSERT_TRUE(result == NetPolicyResultCode::ERR_NONE);
    for (auto &quotaPolicy : quotaPolicies) {
        if (quotaPolicy.netType == NetBearType::BEARER_CELLULAR && quotaPolicy.iccid == ICCID_2) {
            if (quotaPolicy.lastWarningRemind < 0) {
                break;
            }
            auto now = time(nullptr);
            ASSERT_TRUE(now - quotaPolicy.lastWarningRemind < 100);
            return;
        }
    }
    ASSERT_TRUE(false);
}

/**
 * @tc.name: NetPolicyTraffic005
 * @tc.desc: Test NetPolicyTraffic GetMeteredIfaces.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, NetPolicyTraffic005, TestSize.Level1)
{
    auto &ifaces = netPolicyTraffic_->GetMeteredIfaces();
    ASSERT_TRUE(ifaces.size() >= 0);
}

/**
 * @tc.name: NetPolicyTraffic006
 * @tc.desc: Test NetPolicyTraffic ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyTraffic, NetPolicyTraffic006, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    netPolicyTraffic_->ResetPolicies(ICCID_1);
    netPolicyTraffic_->GetNetQuotaPolicies(quotaPolicies);
    for (auto quotaPolicy : quotaPolicies) {
        if (quotaPolicy.iccid == ICCID_1) {
            if (quotaPolicy.periodDuration == "M1"
                    && quotaPolicy.warningBytes == DATA_USAGE_UNKNOWN
                    && quotaPolicy.limitBytes == DATA_USAGE_UNKNOWN
                    && quotaPolicy.lastWarningRemind == REMIND_NEVER
                    && quotaPolicy.lastLimitRemind == REMIND_NEVER
                    && quotaPolicy.metered == false) {
                ASSERT_TRUE(true);
                return;
            }
        }
    }
    ASSERT_TRUE(false);
}
} // namespace NetManagerStandard
} // namespace OHOS
