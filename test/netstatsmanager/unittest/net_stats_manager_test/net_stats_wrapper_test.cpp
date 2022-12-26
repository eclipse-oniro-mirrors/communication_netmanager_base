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

#include "net_stats_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
using namespace testing::ext;
constexpr uint32_t TEST_UID = 1454;
constexpr const char *TEST_IFACE = "test_iface";
} // namespace

class NetStatsWrapperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline NetStatsWrapper &instance_ = NetStatsWrapper::GetInstance();
};

void NetStatsWrapperTest::SetUpTestCase() {}

void NetStatsWrapperTest::TearDownTestCase() {}

void NetStatsWrapperTest::SetUp() {}

void NetStatsWrapperTest::TearDown() {}

HWTEST_F(NetStatsWrapperTest, GetTotalStatsTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    auto result = instance_.GetTotalStats(stats, StatsType::STATS_TYPE_RX_BYTES);
    ASSERT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << result << std::endl;
}

HWTEST_F(NetStatsWrapperTest, GetUidStatsTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    auto result = instance_.GetUidStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_UID);
    ASSERT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << result << std::endl;
}

HWTEST_F(NetStatsWrapperTest, GetIfaceStatsTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    auto result = instance_.GetIfaceStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_IFACE);
    ASSERT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << result << std::endl;
}
} // namespace NetManagerStandard
} // namespace OHOS