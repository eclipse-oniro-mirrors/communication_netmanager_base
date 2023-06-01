/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

/**
 * @tc.name: SetPolicyByUid01
 * @tc.desc: Test NetPolicyService SetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetPolicyByUid01, TestSize.Level1)
{
    uint32_t uid = 10000;
    uint32_t policy = 50;
    int32_t ret = instance_->SetPolicyByUid(uid, policy);
    EXPECT_EQ(ret, POLICY_ERR_INVALID_POLICY);
}

/**
 * @tc.name: GetPolicyByUid01
 * @tc.desc: Test NetPolicyService GetPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetPolicyByUid01, TestSize.Level1)
{
    uint32_t uid = 20000;
    uint32_t policy = 50;
    int32_t ret = instance_->GetPolicyByUid(uid, policy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsUidNetAllowed01
 * @tc.desc: Test NetPolicyService IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, IsUidNetAllowed01, TestSize.Level1)
{
    uint32_t uid = 10000;
    uint32_t policy = 50;
    bool isAllowed = false;
    int32_t ret = instance_->IsUidNetAllowed(uid, policy, isAllowed);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsUidNetAllowed02
 * @tc.desc: Test NetPolicyService IsUidNetAllowed.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, IsUidNetAllowed02, TestSize.Level1)
{
    uint32_t uid = 10000;
    std::string ifaceName = "test";
    bool isAllowed = false;
    int32_t ret = instance_->IsUidNetAllowed(uid, ifaceName, isAllowed);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetNetQuotaPolicies01
 * @tc.desc: Test NetPolicyService SetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetNetQuotaPolicies01, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = instance_->SetNetQuotaPolicies(quotaPolicies);
    EXPECT_EQ(ret, POLICY_ERR_INVALID_QUOTA_POLICY);
}

/**
 * @tc.name: GetNetQuotaPolicies01
 * @tc.desc: Test NetPolicyService GetNetQuotaPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetNetQuotaPolicies01, TestSize.Level1)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    int32_t ret = instance_->GetNetQuotaPolicies(quotaPolicies);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetBackgroundPolicy01
 * @tc.desc: Test NetPolicyService SetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, SetBackgroundPolicy01, TestSize.Level1)
{
    instance_->SetBackgroundPolicy(true);
    int32_t ret = instance_->SetBackgroundPolicy(true);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: GetBackgroundPolicy01
 * @tc.desc: Test NetPolicyService GetBackgroundPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetBackgroundPolicy01, TestSize.Level1)
{
    bool backgroundPolicy = false;
    int32_t ret = instance_->GetBackgroundPolicy(backgroundPolicy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetBackgroundPolicyByUid01
 * @tc.desc: Test NetPolicyService GetBackgroundPolicyByUid.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetBackgroundPolicyByUid01, TestSize.Level1)
{
    uint32_t uid = 20000;
    uint32_t backgroundPolicyOfUid = 0;
    int32_t ret = instance_->GetBackgroundPolicyByUid(uid, backgroundPolicyOfUid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetDumpMessage01
 * @tc.desc: Test NetPolicyService GetDumpMessage.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyService, GetDumpMessage01, TestSize.Level1)
{
    std::string message;
    int32_t ret = instance_->GetDumpMessage(message);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS