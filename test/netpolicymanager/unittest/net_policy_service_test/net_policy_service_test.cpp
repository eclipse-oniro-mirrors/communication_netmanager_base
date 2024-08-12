/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "net_policy_firewall.h"
#include "net_policy_rule.h"
#include "net_policy_service.h"
#include "net_policy_traffic.h"
#include "system_ability_definition.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class UtNetPolicyService : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetPolicyService> instance_ = nullptr;
};

void UtNetPolicyService::SetUpTestCase() {}

void UtNetPolicyService::TearDownTestCase() {}

void UtNetPolicyService::SetUp()
{
    instance_ = DelayedSingleton<NetPolicyService>::GetInstance();
    instance_->netPolicyRule_ = std::make_shared<NetPolicyRule>();
    instance_->netPolicyFirewall_ = std::make_shared<NetPolicyFirewall>();
    instance_->netPolicyTraffic_ = std::make_shared<NetPolicyTraffic>();
}

void UtNetPolicyService::TearDown() {}

HWTEST_F(UtNetPolicyService, OnStart001, TestSize.Level1)
{
    instance_->OnStart();
    EXPECT_EQ(instance_->state_, instance_->ServiceRunningState::STATE_STOPPED);
}

HWTEST_F(UtNetPolicyService, FactoryResetPolicies001, TestSize.Level1)
{
    auto ret = instance_->FactoryResetPolicies();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(UtNetPolicyService, RegisterFactoryResetCallback001, TestSize.Level1)
{
    instance_->RegisterFactoryResetCallback();
    instance_->UpdateNetAccessPolicyToMapFromDB();
    EXPECT_NE(instance_->netFactoryResetCallback_, nullptr);
}

HWTEST_F(UtNetPolicyService, NotifyNetAccessPolicyDiag001, TestSize.Level1)
{
    uint32_t uid = 10000;
    auto ret = instance_->NotifyNetAccessPolicyDiag(uid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
