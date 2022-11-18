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

#include "net_stats_info.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int64_t TEST_RXBYTES = 15453;
constexpr int64_t TEST_TXBYTES = 45115;
constexpr int64_t TEST_RXPACKETS = 5646894;
constexpr int64_t TEST_TXPACKETS = 7894;
NetStatsInfo GetNetStatsInfoData()
{
    NetStatsInfo info;
    info.rxBytes_ = TEST_RXBYTES;
    info.rxPackets_ = TEST_RXPACKETS;
    info.txBytes_ = TEST_TXBYTES;
    info.txPackets_ = TEST_TXPACKETS;
    return info;
}
} // namespace

using namespace testing::ext;
class NetStatsInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    uint32_t GetTestTime();
};

void NetStatsInfoTest::SetUpTestCase() {}

void NetStatsInfoTest::TearDownTestCase() {}

void NetStatsInfoTest::SetUp() {}

void NetStatsInfoTest::TearDown() {}

/**
 * @tc.name: MarshallingTest001
 * @tc.desc: Test NetStatsInfo Marshalling.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsInfoTest, MarshallingTest001, TestSize.Level1)
{
    Parcel parcel;
    NetStatsInfo info = GetNetStatsInfoData();
    EXPECT_TRUE(info.Marshalling(parcel));
    NetStatsInfo result;
    EXPECT_TRUE(NetStatsInfo::Unmarshalling(parcel, result));
    EXPECT_EQ(result.rxBytes_, info.rxBytes_);
    EXPECT_EQ(result.txBytes_, info.txBytes_);
    EXPECT_EQ(result.rxPackets_, info.rxPackets_);
    EXPECT_EQ(result.txPackets_, info.txPackets_);
}

/**
 * @tc.name: MarshallingTest002
 * @tc.desc: Test NetStatsInfo Marshalling.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsInfoTest, MarshallingUnmarshallingTest002, TestSize.Level1)
{
    Parcel parcel;
    NetStatsInfo info = GetNetStatsInfoData();
    EXPECT_TRUE(NetStatsInfo::Marshalling(parcel, info));
    NetStatsInfo result;
    EXPECT_TRUE(NetStatsInfo::Unmarshalling(parcel, result));
    EXPECT_EQ(result.rxBytes_, info.rxBytes_);
    EXPECT_EQ(result.txBytes_, info.txBytes_);
    EXPECT_EQ(result.rxPackets_, info.rxPackets_);
    EXPECT_EQ(result.txPackets_, info.txPackets_);
}
} // namespace NetManagerStandard
} // namespace OHOS
