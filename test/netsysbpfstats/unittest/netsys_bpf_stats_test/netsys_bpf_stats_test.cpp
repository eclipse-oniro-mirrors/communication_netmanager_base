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

#include <ctime>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <sys/resource.h>
#include <unistd.h>

#include "netsys_bpf_map.h"
#include "netsys_bpf_stats.h"
namespace OHOS {
namespace NetManagerStandard {
static constexpr uint32_t TEST_MAP_SIZE = 10;
static constexpr uint32_t TEST_UID1 = 10010;
static constexpr uint32_t TEST_UID2 = 10100;
static constexpr uint32_t TEST_IFACE_INDEX_1 = 1;
static constexpr uint32_t TEST_IFACE_INDEX_2 = 2;
static constexpr uint32_t TEST_IFACE_INDEX_3 = 3;
static constexpr uint32_t TEST_IFACE_INDEX_4 = 4;
static constexpr uint32_t TEST_BYTES0 = 1000;
static constexpr uint32_t TEST_BYTES1 = 2000;
static constexpr uint32_t TEST_PACKET0 = 100;
static constexpr uint32_t TEST_PACKET1 = 200;
static constexpr const char *TEST_IFACE_NAME_WLAN0 = "wlan0";
static constexpr const char *TEST_IFACE_NAME_LO = "lo";
static constexpr const char *TEST_IFACE_NAME_DUMMY0 = "dummy0";
static constexpr const char *TEST_IFACENAME_MAP_PATH = "/sys/fs/bpf/test_netsys_iface_name_map";
static constexpr const char *TEST_IFACESTATS_MAP_PATH = "/sys/fs/bpf/test_netsys_iface_stats_map";
static constexpr const char *TEST_APP_UID_STATS_MAP_PATH = "/sys/fs/bpf/test_netsys_app_uid_stats_map";
static constexpr const char *MOUNT_BPF_FS = "mount -t bpf /sys/fs/bpf /sys/fs/bpf";
using namespace testing::ext;

class NetsysBpfStatsTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();

protected:
    NetsysBpfStatsTest() = default;
};
void NetsysBpfStatsTest::SetUpTestCase() {}

void NetsysBpfStatsTest::TearDownTestCase() {}

void NetsysBpfStatsTest::SetUp()
{
    system(MOUNT_BPF_FS);
}

void NetsysBpfStatsTest::TearDown() {}

void SetStatsValue(StatsValue &value1, StatsValue &value2)
{
    value1 = {
        .rxPackets = TEST_PACKET0 * 2,
        .rxBytes = TEST_BYTES0 * 2,
        .txPackets = TEST_PACKET1 * 2,
        .txBytes = TEST_BYTES1 * 2,
    };
    value2 = {
        .rxPackets = TEST_PACKET1,
        .rxBytes = TEST_BYTES1,
        .txPackets = TEST_PACKET0,
        .txBytes = TEST_BYTES0,
    };
}

HWTEST_F(NetsysBpfStatsTest, NetsysBpfStats001, TestSize.Level1)
{
    struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
    setrlimit(RLIMIT_MEMLOCK, &r);
    NetsysBpfMap<uint32_t, IfaceName> fakeIfaceNameMap(BPF_MAP_TYPE_HASH, TEST_MAP_SIZE, 0);
    NetsysBpfMap<uint32_t, StatsValue> fakeIfaceStatsMap(BPF_MAP_TYPE_HASH, TEST_MAP_SIZE, 0);
    NetsysBpfMap<uint32_t, StatsValue> fakeAppUidStatsMap(BPF_MAP_TYPE_HASH, TEST_MAP_SIZE, 0);
    ASSERT_TRUE(fakeIfaceNameMap.BpfMapFdPin(TEST_IFACENAME_MAP_PATH));
    ASSERT_TRUE(fakeIfaceStatsMap.BpfMapFdPin(TEST_IFACESTATS_MAP_PATH));
    ASSERT_TRUE(fakeAppUidStatsMap.BpfMapFdPin(TEST_APP_UID_STATS_MAP_PATH));
    ASSERT_TRUE(fakeIfaceNameMap.IsValid());
    ASSERT_TRUE(fakeIfaceStatsMap.IsValid());
    ASSERT_TRUE(fakeAppUidStatsMap.IsValid());
}

HWTEST_F(NetsysBpfStatsTest, NetsysBpfStats002, TestSize.Level1)
{
    NetsysBpfMap<uint32_t, IfaceName> fakeIfaceNameMap(BPF_MAP_TYPE_HASH, TEST_MAP_SIZE, 0);
    NetsysBpfMap<uint32_t, StatsValue> fakeIfaceStatsMap(BPF_MAP_TYPE_HASH, TEST_MAP_SIZE, 0);
    IfaceName ifName1;
    IfaceName ifName2;
    IfaceName ifName3;
    IfaceName ifName4;
    ifName1.name = TEST_IFACE_NAME_WLAN0;
    ifName2.name = TEST_IFACE_NAME_LO;
    ifName3.name = TEST_IFACE_NAME_WLAN0;
    ifName4.name = TEST_IFACE_NAME_DUMMY0;
    ASSERT_TRUE(fakeIfaceNameMap.WriteValue(TEST_IFACE_INDEX_1, ifName1, BPF_ANY));
    ASSERT_TRUE(fakeIfaceNameMap.WriteValue(TEST_IFACE_INDEX_2, ifName2, BPF_ANY));
    ASSERT_TRUE(fakeIfaceNameMap.WriteValue(TEST_IFACE_INDEX_3, ifName3, BPF_ANY));
    ASSERT_TRUE(fakeIfaceNameMap.WriteValue(TEST_IFACE_INDEX_4, ifName4, BPF_ANY));
    StatsValue value1;
    StatsValue value2;
    SetStatsValue(value1, value2);
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_1, value1, BPF_ANY));
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_2, value2, BPF_ANY));
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_3, value2, BPF_ANY));
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_4, value1, BPF_ANY));
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    ASSERT_EQ(bpfStats->BpfGetIfaceStats(StatsType::STATS_TYPE_RX_PACKETS, TEST_IFACE_NAME_WLAN0, fakeIfaceNameMap,
                                         fakeIfaceStatsMap),
              TEST_PACKET0 * 2 + TEST_PACKET1);
    ASSERT_EQ(bpfStats->BpfGetIfaceStats(StatsType::STATS_TYPE_RX_BYTES, TEST_IFACE_NAME_WLAN0, fakeIfaceNameMap,
                                         fakeIfaceStatsMap),
              TEST_BYTES0 * 2 + TEST_BYTES1);
    ASSERT_EQ(bpfStats->BpfGetIfaceStats(StatsType::STATS_TYPE_TX_PACKETS, TEST_IFACE_NAME_DUMMY0, fakeIfaceNameMap,
                                         fakeIfaceStatsMap),
              TEST_PACKET1 * 2);
    ASSERT_EQ(bpfStats->BpfGetIfaceStats(StatsType::STATS_TYPE_TX_BYTES, TEST_IFACE_NAME_DUMMY0, fakeIfaceNameMap,
                                         fakeIfaceStatsMap),
              TEST_BYTES1 * 2);
    ASSERT_EQ(fakeIfaceNameMap.ReadValueFromMap(TEST_IFACE_INDEX_2).name, TEST_IFACE_NAME_LO);
    ASSERT_EQ(fakeIfaceNameMap.ReadValueFromMap(TEST_IFACE_INDEX_4).name, TEST_IFACE_NAME_DUMMY0);
    ASSERT_EQ(bpfStats->GetIfaceStats(StatsType::STATS_TYPE_TX_PACKETS, TEST_IFACE_NAME_DUMMY0), 0);
}

HWTEST_F(NetsysBpfStatsTest, NetsysBpfStats003, TestSize.Level1)
{
    NetsysBpfMap<uint32_t, StatsValue> fakeAppUidStatsMap(BPF_MAP_TYPE_HASH, TEST_MAP_SIZE, 0);
    StatsValue value1 = {
        .rxPackets = TEST_PACKET0,
        .rxBytes = TEST_BYTES0,
        .txPackets = TEST_PACKET1,
        .txBytes = TEST_BYTES1,
    };
    StatsValue value2 = {
        .rxPackets = TEST_PACKET0 * 2,
        .rxBytes = TEST_BYTES0 * 2,
        .txPackets = TEST_PACKET1 * 2,
        .txBytes = TEST_BYTES1 * 2,
    };
    ASSERT_EQ(fakeAppUidStatsMap.WriteValue(TEST_UID1, value1, BPF_ANY), true);
    ASSERT_EQ(fakeAppUidStatsMap.WriteValue(TEST_UID2, value2, BPF_ANY), true);
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_RX_BYTES, TEST_UID1, fakeAppUidStatsMap), TEST_BYTES0);
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_TX_BYTES, TEST_UID1, fakeAppUidStatsMap), TEST_BYTES1);
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_RX_PACKETS, TEST_UID1, fakeAppUidStatsMap),
              TEST_PACKET0);
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_TX_PACKETS, TEST_UID1, fakeAppUidStatsMap),
              TEST_PACKET1);
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_RX_BYTES, TEST_UID2, fakeAppUidStatsMap),
              TEST_BYTES0 * 2);
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_TX_BYTES, TEST_UID2, fakeAppUidStatsMap),
              TEST_BYTES1 * 2);
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_RX_PACKETS, TEST_UID2, fakeAppUidStatsMap),
              TEST_PACKET0 * 2);
    ASSERT_EQ(bpfStats->BpfGetUidStats(StatsType::STATS_TYPE_TX_PACKETS, TEST_UID2, fakeAppUidStatsMap),
              TEST_PACKET1 * 2);
    ASSERT_EQ(bpfStats->GetUidStats(StatsType::STATS_TYPE_TX_PACKETS, TEST_UID2), 0);
}

HWTEST_F(NetsysBpfStatsTest, NetsysBpfStats004, TestSize.Level1)
{
    NetsysBpfMap<uint32_t, StatsValue> fakeIfaceStatsMap(BPF_MAP_TYPE_HASH, TEST_MAP_SIZE, 0);
    StatsValue value1;
    StatsValue value2;
    SetStatsValue(value1, value2);
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_1, value1, BPF_ANY));
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_2, value2, BPF_ANY));
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_3, value1, BPF_ANY));
    ASSERT_TRUE(fakeIfaceStatsMap.WriteValue(TEST_IFACE_INDEX_4, value2, BPF_ANY));
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    ASSERT_EQ(bpfStats->BpfGetTotalStats(StatsType::STATS_TYPE_RX_PACKETS, fakeIfaceStatsMap), TEST_PACKET0 * 8);
    ASSERT_EQ(bpfStats->BpfGetTotalStats(StatsType::STATS_TYPE_RX_BYTES, fakeIfaceStatsMap), TEST_BYTES0 * 8);
    ASSERT_EQ(bpfStats->BpfGetTotalStats(StatsType::STATS_TYPE_TX_PACKETS, fakeIfaceStatsMap), TEST_PACKET1 * 5);
    ASSERT_EQ(bpfStats->BpfGetTotalStats(StatsType::STATS_TYPE_TX_BYTES, fakeIfaceStatsMap), TEST_BYTES1 * 5);
    ASSERT_TRUE(fakeIfaceStatsMap.DeleteEntryFromMap(TEST_IFACE_INDEX_1));
    ASSERT_TRUE(fakeIfaceStatsMap.DeleteEntryFromMap(TEST_IFACE_INDEX_3));
    ASSERT_EQ(bpfStats->GetTotalStats(StatsType::STATS_TYPE_RX_PACKETS), 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
