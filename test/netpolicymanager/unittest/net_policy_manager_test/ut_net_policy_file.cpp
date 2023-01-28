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

#define private public
#include "net_policy_file.h"
#undef private
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class UtNetPolicyFile : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UtNetPolicyFile::SetUpTestCase() {}

void UtNetPolicyFile::TearDownTestCase() {}

void UtNetPolicyFile::SetUp() {}

void UtNetPolicyFile::TearDown() {}

/**
 * @tc.name: NetPolicyFileTest001
 * @tc.desc: Test NetPolicyFile NetpolicyFile->
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFile, NetPolicyFileTest001, TestSize.Level1)
{
    auto policyFile = DelayedSingleton<NetPolicyFile>::GetInstance();
    std::string fileName;
    std::string fileContent;
    EXPECT_FALSE(policyFile->CreateFile(fileName));
    EXPECT_FALSE(policyFile->ReadFile(fileName, fileContent));
    std::remove(POLICY_FILE_NAME);
    EXPECT_TRUE(policyFile->CreateFile(POLICY_FILE_NAME));
    EXPECT_TRUE(policyFile->ReadFile(POLICY_FILE_NAME, fileContent));
    policyFile->GetNetPolicies();
    std::string content;
    NetPolicy netPolicy;
    EXPECT_FALSE(policyFile->Json2Obj(content, netPolicy));
}

/**
 * @tc.name: WriteFileTest
 * @tc.desc: Test NetPolicyFile WriteFile.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFile, WriteFileTest, TestSize.Level1)
{
    auto policyFile = DelayedSingleton<NetPolicyFile>::GetInstance();
    uint32_t netUidPolicyOpType = NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE;
    uint32_t uid = 100;
    uint32_t policy = 1;
    bool ret = policyFile->WriteFile(netUidPolicyOpType, uid, policy);
    netUidPolicyOpType = NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE;
    ret = policyFile->WriteFile(netUidPolicyOpType, uid, policy);
    netUidPolicyOpType = NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_ADD;
    ret = policyFile->WriteFile(netUidPolicyOpType, uid, policy);
    policyFile->netPolicy_.netQuotaPolicies.clear();
    NetQuotaPolicy quotaPolicy;
    ret = policyFile->UpdateQuotaPolicyExist(quotaPolicy);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ResetPoliciesTest
 * @tc.desc: Test NetPolicyFile ResetPolicies.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetPolicyFile, ResetPoliciesTest, TestSize.Level1)
{
    auto policyFile = DelayedSingleton<NetPolicyFile>::GetInstance();
    std::string iccid;
    int32_t ret = policyFile->ResetPolicies(iccid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS