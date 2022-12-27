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

#include <memory>

#include <gtest/gtest.h>

#include "net_stats_database_defines.h"
#include "net_stats_history.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;
namespace {
constexpr uint64_t TIME_CYCLE = 60;
} // namespace

using namespace testing::ext;
class NetStatsHistoryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetStatsHistoryTest::SetUpTestCase() {}

void NetStatsHistoryTest::TearDownTestCase() {}

void NetStatsHistoryTest::SetUp() {}

void NetStatsHistoryTest::TearDown() {}

HWTEST_F(NetStatsHistoryTest, HistoryTest001, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    int32_t ret = history->GetHistory(infos);
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });

    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsHistoryTest, HistoryTest002, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    uint64_t currentTime = CommonUtils::GetCurrentSecond();
    int32_t ret = history->GetHistory(infos, currentTime - TIME_CYCLE, currentTime + TIME_CYCLE);
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsHistoryTest, HistoryTest003, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    int32_t ret = history->GetHistory(infos, 1152, 0, LONG_MAX);
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsHistoryTest, HistoryTest004, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    uint64_t currentTime = CommonUtils::GetCurrentSecond();
    int32_t ret = history->GetHistory(infos, 1152, currentTime - TIME_CYCLE, currentTime + TIME_CYCLE);
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsHistoryTest, HistoryTest005, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    int32_t ret = history->GetHistory(infos, "wlan0");
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsHistoryTest, HistoryTest006, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    uint64_t currentTime = CommonUtils::GetCurrentSecond();
    int32_t ret = history->GetHistory(infos, "wlan0", currentTime - TIME_CYCLE, currentTime + TIME_CYCLE);
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsHistoryTest, HistoryTest007, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    int32_t ret = history->GetHistory(infos, "wlan0", 1152, 0, LONG_MAX);
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsHistoryTest, HistoryTest008, TestSize.Level1)
{
    auto history = std::make_unique<NetStatsHistory>();
    std::vector<NetStatsInfo> infos;
    uint64_t currentTime = CommonUtils::GetCurrentSecond();
    int32_t ret = history->GetHistory(infos, "wlan0", 1152, currentTime - TIME_CYCLE, currentTime + TIME_CYCLE);
    std::for_each(infos.begin(), infos.end(), [](const auto &info) { std::cout << info.UidData() << std::endl; });
    EXPECT_EQ(ret, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS