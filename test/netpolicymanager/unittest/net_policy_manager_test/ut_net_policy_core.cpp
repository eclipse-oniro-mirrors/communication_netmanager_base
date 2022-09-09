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
#include "net_policy_core.h"
#include "net_policy_firewall.h"
#include "net_policy_inner_define.h"
#include "net_policy_rule.h"
#include "net_policy_traffic.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string NET_POLICY_WORK_TEST_THREAD = "NET_POLICY_WORK_TEST_THREAD";

std::shared_ptr<NetPolicyCore> netPolicyCore_;
std::vector<NetQuotaPolicy> quotaPolicies;
std::shared_ptr<AppExecFwk::EventRunner> runner_;
std::shared_ptr<NetPolicyEventHandler> handler_;

using namespace testing::ext;
class UtNetPolicyCore : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtNetPolicyCore::SetUpTestCase()
{
    runner_ = AppExecFwk::EventRunner::Create(NET_POLICY_WORK_TEST_THREAD);
    netPolicyCore_ = DelayedSingleton<NetPolicyCore>::GetInstance();
    handler_ = std::make_shared<NetPolicyEventHandler>(runner_, netPolicyCore_);
    netPolicyCore_->Init(handler_);
}

void UtNetPolicyCore::TearDownTestCase()
{
    netPolicyCore_.reset();
}

void UtNetPolicyCore::SetUp() {}

void UtNetPolicyCore::TearDown() {}

/**
 * @tc.name: NetPolicyCore001
 * @tc.desc: Test NetPolicyCore CreateCore.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyCore, NetPolicyCore001, TestSize.Level1)
{
    auto netPolicyTraffic_ = netPolicyCore_->CreateCore<NetPolicyTraffic>();
    auto netPolicyFirewall_ = netPolicyCore_->CreateCore<NetPolicyFirewall>();
    auto netPolicyRule_ = netPolicyCore_->CreateCore<NetPolicyRule>();
    ASSERT_TRUE(netPolicyTraffic_ != nullptr && netPolicyFirewall_ != nullptr && netPolicyCore_ != nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS
