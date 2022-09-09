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

#include <gtest/gtest.h>

#include "net_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string ICCID_1 = "sim_abcdefg_1";
using namespace testing::ext;
class UtNetQuotaPolicy : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtNetQuotaPolicy::SetUpTestCase() {}

void UtNetQuotaPolicy::TearDownTestCase() {}

void UtNetQuotaPolicy::SetUp() {}

void UtNetQuotaPolicy::TearDown() {}

/**
 * @tc.name: NetPolicyQuota001
 * @tc.desc: Test NetPolicyQuota GetPeriodStart.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetQuotaPolicy, NetPolicyQuota001, TestSize.Level1)
{
    NetQuotaPolicy netQuotaPolicy1;
    netQuotaPolicy1.iccid = ICCID_1;
    netQuotaPolicy1.periodDuration = "M1";
    auto result = netQuotaPolicy1.GetPeriodStart();
    std::cout << "result1:" << result << std::endl;

    NetQuotaPolicy netQuotaPolicy2;
    netQuotaPolicy2.iccid = ICCID_1;
    netQuotaPolicy2.periodDuration = "Y1";
    auto result2 = netQuotaPolicy2.GetPeriodStart();
    std::cout << "result2:" << result2 << std::endl;

    NetQuotaPolicy netQuotaPolicy3;
    netQuotaPolicy3.iccid = ICCID_1;
    netQuotaPolicy3.periodDuration = "D1";
    auto result3 = netQuotaPolicy3.GetPeriodStart();
    std::cout << "result3:" << result3 << std::endl;

    ASSERT_TRUE(result > 0);
    ASSERT_TRUE(result2 > 0);
    ASSERT_TRUE(result3 > 0);
}
} // namespace NetManagerStandard
} // namespace OHOS