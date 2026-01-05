/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <ctime>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_center.h"
#include "net_stats_callback_test.h"
#include "net_stats_constants.h"
#include "net_stats_service.h"
#include "net_stats_cached.h"
#include "net_stats_database_defines.h"
#include "system_ability_definition.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;

using namespace testing::ext;
class NetStatsServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetStatsServiceTest::SetUpTestCase() {}

void NetStatsServiceTest::TearDownTestCase() {}

void NetStatsServiceTest::SetUp() {}

void NetStatsServiceTest::TearDown() {}

HWTEST_F(NetStatsServiceTest, GetTrafficStatsByUidNetworkTest001, TestSize.Level1)
{
    NetStatsService netStatsService;
    std::vector<NetStatsInfoSequence> infos = {};
    uint32_t uid = 1;
    const sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    network->type_ = 1;
    network->startTime_ = 1;
    network->endTime_ = 2;
    int32_t ret = netStatsService.GetTrafficStatsByUidNetwork(infos, uid, *network);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetTrafficStatsByUidNetworkTest002, TestSize.Level1)
{
    NetStatsService netStatsService;
    std::vector<NetStatsInfoSequence> infos = {};
    uint32_t uid = 1;
    const sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    network->type_ = 1;
    network->startTime_ = 1857600534;
    network->endTime_ = 1867600534;
    int32_t ret = netStatsService.GetTrafficStatsByUidNetwork(infos, uid, *network);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetTrafficStatsByUidNetworkTest003, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    std::vector<NetStatsInfoSequence> infos = {};
    uint32_t uid = 1;
    const sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    network->type_ = 1;
    network->startTime_ = 1757600034;
    network->endTime_ = 1867600534;
    int32_t ret = netStatsService->GetTrafficStatsByUidNetwork(infos, uid, *network);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, ProcessOsAccountChangedTest001, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    AccountSA::OsAccountState state = AccountSA::OsAccountState::CREATED;
    int32_t userId = 101;
    int32_t ret = netStatsService->ProcessOsAccountChanged(userId, state);
    EXPECT_EQ(ret, 0);
    state = AccountSA::OsAccountState::STOPPING;
    ret = netStatsService->ProcessOsAccountChanged(888, state);
    EXPECT_EQ(ret, 0);
    netStatsService->netStatsCached_->SetCurPrivateUserId(101);
    EXPECT_EQ(netStatsService->netStatsCached_->GetCurPrivateUserId(), 101);
    state = AccountSA::OsAccountState::INVALID_TYPE;
    ret = netStatsService->ProcessOsAccountChanged(888, state);
}

HWTEST_F(NetStatsServiceTest, ProcessOsAccountChangedTest002, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t userId = 101;
    netStatsService->netStatsCached_->SetCurPrivateUserId(userId);
    AccountSA::OsAccountState state = AccountSA::OsAccountState::STOPPED;
    int32_t ret = netStatsService->ProcessOsAccountChanged(userId, state);

    int32_t curPrivateUserId = netStatsService->netStatsCached_->GetCurPrivateUserId();
    EXPECT_EQ(curPrivateUserId, -1);
}

HWTEST_F(NetStatsServiceTest, ProcessOsAccountChangedTest003, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t userId = 101;
    netStatsService->netStatsCached_->SetCurPrivateUserId(userId);
    AccountSA::OsAccountState state = AccountSA::OsAccountState::SWITCHED;
    int32_t ret = netStatsService->ProcessOsAccountChanged(userId, state);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsServiceTest, ProcessOsAccountChangedTest004, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t userId = 101;
    netStatsService->netStatsCached_->SetCurPrivateUserId(userId);
    AccountSA::OsAccountState state = AccountSA::OsAccountState::LOCKED;
    int32_t ret = netStatsService->ProcessOsAccountChanged(userId, state);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsServiceTest, ProcessOsAccountChangedTest005, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t userId = 101;
    netStatsService->netStatsCached_->SetCurPrivateUserId(userId);
    AccountSA::OsAccountState state = AccountSA::OsAccountState::SWITCHED;
    int32_t ret = netStatsService->ProcessOsAccountChanged(userId + 1, state);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsServiceTest, MergeTrafficStatsByAccountTest001, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    
    int32_t curUserId = -1;
    int32_t ret1 = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(curUserId);
    int32_t defaultUserId = -1;
    int32_t ret2 = AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    if (ret1 != 0 || ret2 != 0) {
        return;
    }

    std::vector<NetStatsInfo> infos;
    NetStatsInfo info1;
    info1.userId_ = curUserId;
    NetStatsInfo info2;
    info2.userId_ = 101;
    infos.push_back(info1);
    infos.push_back(info2);
    netStatsService->netStatsCached_->SetCurPrivateUserId(101);

    if (curUserId == defaultUserId) {
        netStatsService->MergeTrafficStatsByAccount(infos);
        EXPECT_EQ(infos[1].userId_, 101);
    }
}

HWTEST_F(NetStatsServiceTest, MergeTrafficStatsByAccountTest002, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    
    int32_t curUserId = -1;
    int32_t ret1 = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(curUserId);
    int32_t defaultUserId = -1;
    int32_t ret2 = AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    if (ret1 != 0 || ret2 != 0) {
        return;
    }
    NETMGR_LOG_E("curUserId:%{public}d, defaultUserId:%{public}d", curUserId, defaultUserId);
    std::vector<NetStatsInfo> infos;
    NetStatsInfo info1;
    info1.userId_ = curUserId;
    NetStatsInfo info2;
    info2.userId_ = 101;
    NetStatsInfo info3;
    info3.userId_ = SIM_PRIVATE_USERID;
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    netStatsService->netStatsCached_->SetCurDefaultUserId(109);
    netStatsService->netStatsCached_->SetCurPrivateUserId(curUserId);
    netStatsService->MergeTrafficStatsByAccount(infos);
    if (curUserId == defaultUserId) {
        EXPECT_EQ(infos[1].uid_, DEFAULT_ACCOUNT_UID);
    }
}

HWTEST_F(NetStatsServiceTest, MergeTrafficStatsByAccountTest003, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    
    int32_t curUserId = -1;
    int32_t ret1 = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(curUserId);
    int32_t defaultUserId = -1;
    int32_t ret2 = AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    if (ret1 != 0 || ret2 != 0) {
        return;
    }

    std::vector<NetStatsInfo> infos;
    NetStatsInfo info1;
    info1.userId_ = curUserId;
    NetStatsInfo info2;
    info2.userId_ = 101;
    NetStatsInfo info3;
    info3.userId_ = SIM_PRIVATE_USERID;
    NetStatsInfo info4;
    info4.userId_ = 108;
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->netStatsCached_->SetCurDefaultUserId(curUserId);
    netStatsService->netStatsCached_->SetCurPrivateUserId(108);
    netStatsService->MergeTrafficStatsByAccount(infos);

    if (curUserId == defaultUserId) {
        EXPECT_EQ(infos[3].uid_, OTHER_ACCOUNT_UID);
    }
}

HWTEST_F(NetStatsServiceTest, DeleteTrafficStatsByAccountTest001, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t defaultUserId = -1;
    AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    std::vector<NetStatsInfoSequence> infos;
    NetStatsInfoSequence info1;
    info1.info_.userId_ = defaultUserId;
    NetStatsInfoSequence info2;
    info2.info_.userId_ = defaultUserId + 1;
    NetStatsInfoSequence info3;
    info3.info_.userId_ = 12345612;
    NetStatsInfoSequence info4;
    info3.info_.userId_ = 0;
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, DEFAULT_ACCOUNT_UID);
    EXPECT_EQ(infos.size(), 3);
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, OTHER_ACCOUNT_UID);
    EXPECT_EQ(infos.size(), 1);
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, 888);
    EXPECT_EQ(infos.size(), 5);
}

HWTEST_F(NetStatsServiceTest, DeleteTrafficStatsByAccountTest003, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t defaultUserId = -1;
    AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    netStatsService->netStatsCached_->SetCurDefaultUserId(defaultUserId);
    std::vector<NetStatsInfoSequence> infos;
    NetStatsInfoSequence info1;
    info1.info_.userId_ = defaultUserId;
    NetStatsInfoSequence info2;
    info2.info_.userId_ = defaultUserId + 1;
    NetStatsInfoSequence info3;
    info3.info_.userId_ = 12345612;
    NetStatsInfoSequence info4;
    info3.info_.userId_ = 0;
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, Sim_UID);
    EXPECT_EQ(infos.size(), 2);
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, SIM2_UID);
    EXPECT_EQ(infos.size(), 4);
}

HWTEST_F(NetStatsServiceTest, DeleteTrafficStatsByAccountTest004, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t defaultUserId = -1;
    AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    netStatsService->netStatsCached_->SetCurDefaultUserId(defaultUserId);
    std::vector<NetStatsInfoSequence> infos;
    NetStatsInfoSequence info1;
    info1.info_.userId_ = defaultUserId;
    NetStatsInfoSequence info2;
    info2.info_.userId_ = defaultUserId + 1;
    NetStatsInfoSequence info3;
    info3.info_.userId_ = 12345612;
    NetStatsInfoSequence info4;
    info3.info_.userId_ = 0;
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, DEFAULT_ACCOUNT_UID);
    EXPECT_EQ(infos.size(), 3);
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, OTHER_ACCOUNT_UID);
    EXPECT_EQ(infos.size(), 1);
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, 888);
    EXPECT_EQ(infos.size(), 5);
}

HWTEST_F(NetStatsServiceTest, DeleteTrafficStatsByAccountTest002, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int32_t defaultUserId = -1;
    AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    std::vector<NetStatsInfoSequence> infos;
    NetStatsInfoSequence info1;
    info1.info_.userId_ = defaultUserId;
    NetStatsInfoSequence info2;
    info2.info_.userId_ = defaultUserId + 1;
    NetStatsInfoSequence info3;
    info3.info_.userId_ = 12345612;
    NetStatsInfoSequence info4;
    info3.info_.userId_ = 0;
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, Sim_UID);
    EXPECT_EQ(infos.size(), 2);
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->DeleteTrafficStatsByAccount(infos, SIM2_UID);
    EXPECT_EQ(infos.size(), 4);
}

HWTEST_F(NetStatsServiceTest, EraseNetStatsInfoByUserIdTest001, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    int defaultUserId = 100;
    std::vector<NetStatsInfoSequence> infos;
    NetStatsInfoSequence info1;
    info1.info_.userId_ = defaultUserId;
    NetStatsInfoSequence info2;
    info2.info_.userId_ = defaultUserId + 1;
    NetStatsInfoSequence info3;
    info3.info_.userId_ = 12345612;
    NetStatsInfoSequence info4;
    info3.info_.userId_ = 0;
    infos.push_back(info1);
    infos.push_back(info2);
    infos.push_back(info3);
    infos.push_back(info4);
    netStatsService->EraseNetStatsInfoByUserId(infos, 0);
    EXPECT_EQ(infos.size(), 2);
}

HWTEST_F(NetStatsServiceTest, AddUidStatsFlagTest001, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    netStatsService->AddUidStatsFlag(0);
    sleep(1);
    EXPECT_EQ(netStatsService->isUpdate_, true);
}

HWTEST_F(NetStatsServiceTest, CommonEventPackageRemovedTest001, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    bool ret = netStatsService->CommonEventPackageRemoved(123);
    EXPECT_EQ(ret, false);
}

HWTEST_F(NetStatsServiceTest, CommonEventPackageRemovedTest003, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    bool ret = netStatsService->CommonEventPackageRemoved(SYSTEM_DEFAULT_USERID + USER_ID_DIVIDOR);
    EXPECT_EQ(ret, true);
}

HWTEST_F(NetStatsServiceTest, CommonEventPackageRemovedTest004, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    bool ret = netStatsService->CommonEventPackageRemoved(SIM_PRIVATE_USERID * 1000 + 1);
    EXPECT_EQ(ret, false);
}
} // namespace NetManagerStandard
} // namespace OHOS
