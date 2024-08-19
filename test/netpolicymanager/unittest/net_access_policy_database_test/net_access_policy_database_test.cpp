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

#include "net_access_policy_rdb.h"
#include "net_manager_constants.h"
#include "system_ability_definition.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class NetAccessPolicyRDBTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetAccessPolicyRDBTest::SetUpTestCase() {}

void NetAccessPolicyRDBTest::TearDownTestCase() {}

void NetAccessPolicyRDBTest::SetUp() {}

void NetAccessPolicyRDBTest::TearDown() {}

HWTEST_F(NetAccessPolicyRDBTest, InitRdbStore001, TestSize.Level1)
{
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.InitRdbStore();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, InsertData001, TestSize.Level1)
{
    NetAccessPolicyData policy;
    policy.uid = 10000;
    policy.wifiPolicy = 64;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.InsertData(policy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, UpdateByUid001, TestSize.Level1)
{
    uint32_t uid = 10000;
    NetAccessPolicyData policy;
    policy.uid = uid;
    policy.wifiPolicy = 64;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.UpdateByUid(uid, policy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, QueryAll001, TestSize.Level1)
{
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.QueryAll();
    ASSERT_FALSE(ret.empty());
}

HWTEST_F(NetAccessPolicyRDBTest, QueryByUid001, TestSize.Level1)
{
    uint32_t uid = 10000;
    NetAccessPolicyData policy;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.QueryByUid(uid, policy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, QueryByUid_ShouldReturnError_WhenRdbStoreIsNull, TestSize.Level0)
{
    int32_t uid = 123;
    NetAccessPolicyData uidPolicy;
    NetAccessPolicyRDB netAccessPolicyRDB;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.QueryByUid(uid, uidPolicy);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

// Scenario5: Test when uidPolicy.uid is equal to uid then QueryByUid returns NETMANAGER_SUCCESS.
HWTEST_F(NetAccessPolicyRDBTest, QueryByUid_ShouldReturnSuccess_WhenUidPolicyUidIsEqualToUid, TestSize.Level0)
{
    int32_t uid = 123;
    NetAccessPolicyData uidPolicy;
    uidPolicy.uid = uid;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.QueryByUid(uid, uidPolicy);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetAccessPolicyRDBTest, DeleteByUid001, TestSize.Level1)
{
    uint32_t uid = 10000;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.DeleteByUid(uid);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(NetAccessPolicyRDBTest, GetRdbStore001, TestSize.Level1)
{
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.GetRdbStore();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, UpgradeDbVersionTo001, TestSize.Level1)
{
    int newVersion = 0;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.GetRdbStore();
    NetAccessPolicyRDB::RdbDataOpenCallback rdbDataOpenCallback;
    rdbDataOpenCallback.UpgradeDbVersionTo(*netAccessPolicy.rdbStore_, newVersion);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, UpgradeDbVersionTo002, TestSize.Level1)
{
    int newVersion = 1;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.GetRdbStore();
    NetAccessPolicyRDB::RdbDataOpenCallback rdbDataOpenCallback;
    rdbDataOpenCallback.UpgradeDbVersionTo(*netAccessPolicy.rdbStore_, newVersion);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, UpgradeDbVersionTo003, TestSize.Level1)
{
    int newVersion = 2;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.GetRdbStore();
    NetAccessPolicyRDB::RdbDataOpenCallback rdbDataOpenCallback;
    rdbDataOpenCallback.UpgradeDbVersionTo(*netAccessPolicy.rdbStore_, newVersion);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, UpgradeDbVersionTo004, TestSize.Level1)
{
    int newVersion = 3;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.GetRdbStore();
    NetAccessPolicyRDB::RdbDataOpenCallback rdbDataOpenCallback;
    rdbDataOpenCallback.UpgradeDbVersionTo(*netAccessPolicy.rdbStore_, newVersion);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, AddIsBroker001, TestSize.Level1)
{
    int newVersion = 2;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.GetRdbStore();
    NetAccessPolicyRDB::RdbDataOpenCallback rdbDataOpenCallback;
    rdbDataOpenCallback.AddIsBroker(*netAccessPolicy.rdbStore_, newVersion);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetAccessPolicyRDBTest, AddIsBroker002, TestSize.Level1)
{
    int newVersion = 0;
    NetAccessPolicyRDB netAccessPolicy;
    auto ret = netAccessPolicy.GetRdbStore();
    NetAccessPolicyRDB::RdbDataOpenCallback rdbDataOpenCallback;
    rdbDataOpenCallback.AddIsBroker(*netAccessPolicy.rdbStore_, newVersion);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
