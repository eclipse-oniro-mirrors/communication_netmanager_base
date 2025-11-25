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
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    std::vector<NetStatsInfoSequence> infos = {};
    uint32_t uid = 1;
    const sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    network->type_ = 1;
    network->startTime_ = 1;
    network->endTime_ = 2;
    int32_t ret = netStatsService->GetTrafficStatsByUidNetwork(infos, uid, *network);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetTrafficStatsByUidNetworkTest002, TestSize.Level1)
{
    auto netStatsService = DelayedSingleton<NetStatsService>::GetInstance();
    std::vector<NetStatsInfoSequence> infos = {};
    uint32_t uid = 1;
    const sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    network->type_ = 1;
    network->startTime_ = 1857600534;
    network->endTime_ = 1867600534;
    int32_t ret = netStatsService->GetTrafficStatsByUidNetwork(infos, uid, *network);
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

} // namespace NetManagerStandard
} // namespace OHOS
