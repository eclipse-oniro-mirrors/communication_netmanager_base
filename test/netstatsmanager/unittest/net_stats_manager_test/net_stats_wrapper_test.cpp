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
    auto result = instance_.GetTotalStats(StatsType::STATS_TYPE_RX_BYTES);
    ASSERT_GE(result, -1);
}

HWTEST_F(NetStatsWrapperTest, GetUidStatsTest001, TestSize.Level1)
{
    auto result = instance_.GetUidStats(StatsType::STATS_TYPE_RX_BYTES, TEST_UID);
    ASSERT_GE(result, -1);
}

HWTEST_F(NetStatsWrapperTest, GetIfaceStatsTest001, TestSize.Level1)
{
    auto result = instance_.GetIfaceStats(StatsType::STATS_TYPE_RX_BYTES, TEST_IFACE);
    ASSERT_GE(result, -1);
}
} // namespace NetManagerStandard
} // namespace OHOS