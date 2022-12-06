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

#define protected public
#include "net_policy_base.h"
#undef protected
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t EVENT_ID = 101;
constexpr int64_t DELAY_TIME = 10;
} // namespace

class NetPolicyBaseTest : public NetPolicyBase {
public:
    void Init() override {}
    void HandleEvent(int32_t eventId, const std::shared_ptr<PolicyEvent> &policyEvent) override {}
};

class UtNetPolicyBase : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtNetPolicyBase::SetUpTestCase() {}

void UtNetPolicyBase::TearDownTestCase() {}

void UtNetPolicyBase::SetUp() {}

void UtNetPolicyBase::TearDown() {}

/**
 * @tc.name: NetPolicyBaseTest001
 * @tc.desc: Test FirewallRule NetpolicyBase->
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyBase, NetPolicyBaseTest001, TestSize.Level1)
{
    auto policyBase = std::make_shared<NetPolicyBaseTest>();
    EXPECT_TRUE(policyBase->GetCbInst() != nullptr);
    EXPECT_TRUE(policyBase->GetFileInst() != nullptr);
    EXPECT_TRUE(policyBase->GetNetsysInst() != nullptr);
    policyBase->GetNetCenterInst();
    policyBase->SendEvent(EVENT_ID, DELAY_TIME);
}
} // namespace NetManagerStandard
} // namespace OHOS