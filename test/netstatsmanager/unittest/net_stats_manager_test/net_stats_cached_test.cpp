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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_stats_cached.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetStatsCachedTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = std::make_shared<NetStatsCached>();
};

void NetStatsCachedTest::SetUpTestCase() {}

void NetStatsCachedTest::TearDownTestCase() {}

void NetStatsCachedTest::SetUp() {}

void NetStatsCachedTest::TearDown() {}

HWTEST_F(NetStatsCachedTest, CacheUidStatsTest001, TestSize.Level1)
{
    instance_->CacheUidStats();
    auto ret = instance_->CheckUidStor();
    EXPECT_FALSE(ret);
    ret = instance_->CheckIfaceStor();
    EXPECT_FALSE(ret);
    NetStatsInfo info;
    instance_->stats_.PushUidStats(info);
    instance_->stats_.PushIfaceStats(info);
    EXPECT_TRUE(instance_->stats_.GetUidStatsInfo().empty());
    EXPECT_TRUE(instance_->stats_.GetIfaceStatsInfo().empty());
    EXPECT_EQ(instance_->stats_.GetCurrentUidStats(), static_cast<uint64_t>(0));
    EXPECT_EQ(instance_->stats_.GetCurrentIfaceStats(), static_cast<uint64_t>(0));
    instance_->stats_.ResetUidStats();
    instance_->stats_.ResetIfaceStats();
    EXPECT_TRUE(instance_->stats_.GetUidStatsInfo().empty());
    EXPECT_TRUE(instance_->stats_.GetIfaceStatsInfo().empty());
    EXPECT_EQ(instance_->stats_.GetCurrentUidStats(), static_cast<uint64_t>(0));
    EXPECT_EQ(instance_->stats_.GetCurrentIfaceStats(), static_cast<uint64_t>(0));
    instance_->CacheStats();
}
} // namespace NetManagerStandard
} // namespace OHOS