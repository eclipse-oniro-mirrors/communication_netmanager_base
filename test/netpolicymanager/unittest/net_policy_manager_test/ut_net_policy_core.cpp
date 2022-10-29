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
constexpr const char *NET_POLICY_WORK_TEST_THREAD = "NET_POLICY_WORK_TEST_THREAD";

std::shared_ptr<NetPolicyCore> g_netPolicyCore;
std::shared_ptr<AppExecFwk::EventRunner> g_runner;
std::shared_ptr<NetPolicyEventHandler> g_handler;

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
    g_runner = AppExecFwk::EventRunner::Create(NET_POLICY_WORK_TEST_THREAD);
    g_netPolicyCore = DelayedSingleton<NetPolicyCore>::GetInstance();
    g_handler = std::make_shared<NetPolicyEventHandler>(g_runner, g_netPolicyCore);
    g_netPolicyCore->Init(g_handler);
}

void UtNetPolicyCore::TearDownTestCase()
{
    g_netPolicyCore.reset();
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
    auto netPolicyTraffic_ = g_netPolicyCore->CreateCore<NetPolicyTraffic>();
    auto netPolicyFirewall_ = g_netPolicyCore->CreateCore<NetPolicyFirewall>();
    auto netPolicyRule_ = g_netPolicyCore->CreateCore<NetPolicyRule>();
    ASSERT_TRUE(netPolicyTraffic_ != nullptr && netPolicyFirewall_ != nullptr && g_netPolicyCore != nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS
