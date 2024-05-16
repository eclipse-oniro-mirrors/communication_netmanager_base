/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "net_stats_info_sequence.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t TEST_UID = 20020142;
constexpr int64_t TEST_RXBYTES = 15453;
constexpr int64_t TEST_TXBYTES = 45115;
constexpr int64_t TEST_RXPACKETS = 5646894;
constexpr int64_t TEST_TXPACKETS = 7894;
constexpr uint64_t TEST_START_TIME = 1;
constexpr uint64_t TEST_END_TIME = 200;
constexpr const char *TEST_IFACE = "eth0";
constexpr const char *TEST_IDENT = "2";
NetStatsInfoSequence GetNetStatsInfoSequenceData()
{
    NetStatsInfoSequence infoSequence;
    infoSequence.startTime_ = TEST_START_TIME;
    infoSequence.endTime_ = TEST_END_TIME;
    NetStatsInfo info;
    info.uid_ = TEST_UID;
    info.iface_ = TEST_IFACE;
    info.ident_ = TEST_IDENT;
    info.date_ = TEST_RXPACKETS;
    info.rxBytes_ = TEST_RXBYTES;
    info.rxPackets_ = TEST_RXPACKETS;
    info.txBytes_ = TEST_TXBYTES;
    info.txPackets_ = TEST_TXPACKETS;
    infoSequence.info_ = info;
    return infoSequence;
}
} // namespace

using namespace testing::ext;
class NetStatsInfoSequenceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    uint32_t GetTestTime();
};

void NetStatsInfoSequenceTest::SetUpTestCase() {}

void NetStatsInfoSequenceTest::TearDownTestCase() {}

void NetStatsInfoSequenceTest::SetUp() {}

void NetStatsInfoSequenceTest::TearDown() {}

/**
* @tc.name: MarshallingUnmarshallingTest001
* @tc.desc: Test NetStatsInfo Marshalling and Unmarshalling
* @tc.type: FUNC
*/
HWTEST_F(NetStatsInfoSequenceTest, MarshallingUnmarshallingTest001, TestSize.Level1)
{
    Parcel parcel;
    NetStatsInfoSequence info = GetNetStatsInfoSequenceData();
    EXPECT_TRUE(info.Marshalling(parcel));
    NetStatsInfoSequence result;
    EXPECT_TRUE(NetStatsInfoSequence::Unmarshalling(parcel, result));
    EXPECT_EQ(result.startTime_, info.startTime_);
    EXPECT_EQ(result.endTime_, info.endTime_);
    EXPECT_EQ(result.info_.uid_, info.info_.uid_);
    EXPECT_EQ(result.info_.iface_, info.info_.iface_);
    EXPECT_EQ(result.info_.ident_, info.info_.ident_);
    EXPECT_EQ(result.info_.date_, info.info_.date_);
    EXPECT_EQ(result.info_.rxBytes_, info.info_.rxBytes_);
    EXPECT_EQ(result.info_.txBytes_, info.info_.txBytes_);
    EXPECT_EQ(result.info_.rxPackets_, info.info_.rxPackets_);
    EXPECT_EQ(result.info_.txPackets_, info.info_.txPackets_);
}

/**
* @tc.name: MarshallingUnmarshallingTest002
* @tc.desc: Test NetStatsInfo Marshalling and Unmarshalling
* @tc.type: FUNC
*/
HWTEST_F(NetStatsInfoSequenceTest, MarshallingUnmarshallingTest002, TestSize.Level1)
{
    Parcel parcel;
    NetStatsInfoSequence info = GetNetStatsInfoSequenceData();
    EXPECT_TRUE(NetStatsInfoSequence::Marshalling(parcel, info));
    NetStatsInfoSequence result;
    EXPECT_TRUE(NetStatsInfoSequence::Unmarshalling(parcel, result));
    EXPECT_EQ(result.startTime_, info.startTime_);
    EXPECT_EQ(result.endTime_, info.endTime_);
    EXPECT_EQ(result.info_.uid_, info.info_.uid_);
    EXPECT_EQ(result.info_.iface_, info.info_.iface_);
    EXPECT_EQ(result.info_.ident_, info.info_.ident_);
    EXPECT_EQ(result.info_.date_, info.info_.date_);
    EXPECT_EQ(result.info_.rxBytes_, info.info_.rxBytes_);
    EXPECT_EQ(result.info_.txBytes_, info.info_.txBytes_);
    EXPECT_EQ(result.info_.rxPackets_, info.info_.rxPackets_);
    EXPECT_EQ(result.info_.txPackets_, info.info_.txPackets_);
}

/**
* @tc.name: MarshallingUnmarshallingTest003
* @tc.desc: Test NetStatsInfo Marshalling and Unmarshalling
* @tc.type: FUNC
*/
HWTEST_F(NetStatsInfoSequenceTest, MarshallingUnmarshallingTest003, TestSize.Level1)
{
    Parcel parcel;
    std::vector<NetStatsInfoSequence> statsInfos;
    NetStatsInfoSequence infoa = GetNetStatsInfoSequenceData();
    statsInfos.push_back(infoa);
    NetStatsInfoSequence infob = GetNetStatsInfoSequenceData();
    statsInfos.push_back(infob);
    EXPECT_TRUE(NetStatsInfoSequence::Marshalling(parcel, statsInfos));
    std::vector<NetStatsInfoSequence> result;
    EXPECT_TRUE(NetStatsInfoSequence::Unmarshalling(parcel, result));
}

/**
* @tc.name: MarshallingUnmarshallingTest004
* @tc.desc: Test NetStatsInfo Equals
* @tc.type: FUNC
*/
HWTEST_F(NetStatsInfoSequenceTest, MarshallingUnmarshallingTest004, TestSize.Level1)
{
    NetStatsInfoSequence infoa = GetNetStatsInfoSequenceData();
    NetStatsInfoSequence infob = GetNetStatsInfoSequenceData();
    EXPECT_TRUE(infoa.Equals(infob));
}
}
}
