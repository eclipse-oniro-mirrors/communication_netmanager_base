
/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <net/if.h>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <sys/resource.h>
#include <unistd.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "bpf_loader.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "bpf_stats.h"

#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr uint32_t TEST_UID1 = 10010;
static constexpr uint32_t TEST_UID2 = 10100;
static constexpr uint32_t TEST_UID_IF1 = 11001;
static constexpr uint32_t TEST_UID_IF2 = 11002;
static constexpr uint32_t TEST_BYTES0 = 11;
static constexpr uint32_t STATS_TYPE_INVALID_VALUE = 4;
static constexpr uint64_t TEST_COOKIE1 = 1;
static constexpr const char *TEST_IFACE_NAME_WLAN0 = "wlan0";
static constexpr const char *TEST_IFACE_NAME_LO = "lo";
static constexpr const char *TEST_IFACE_NAME_DUMMY0 = "dummy0";
static constexpr const char *BFP_NAME_NETSYS_PATH = "/system/etc/bpf/netsys.o";
static constexpr const char *TEST_BFP_NAME_NETSYS_PATH = "/data/netsys.o";

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

void NetsysBpfStatsTest::SetUp() {}

void NetsysBpfStatsTest::TearDown() {}

HWTEST_F(NetsysBpfStatsTest, GetTotalStats, TestSize.Level1)
{
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    uint64_t stats = 0;
    bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_RX_BYTES);
    EXPECT_GE(stats, 0);
    bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_RX_PACKETS);
    EXPECT_GE(stats, 0);
    bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_TX_BYTES);
    EXPECT_GE(stats, 0);
    bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_TX_PACKETS);
    EXPECT_GE(stats, 0);
}

HWTEST_F(NetsysBpfStatsTest, GetUidStats, TestSize.Level1)
{
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    uint64_t stats = 0;

    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_UID1);
    EXPECT_GE(stats, 0);
    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_UID1);
    EXPECT_GE(stats, 0);
    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_UID1);
    EXPECT_GE(stats, 0);
    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_UID1);
    EXPECT_GE(stats, 0);

    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_UID2);
    EXPECT_GE(stats, 0);
    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_UID2);
    EXPECT_GE(stats, 0);
    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_UID2);
    EXPECT_GE(stats, 0);
    bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_UID2);
    EXPECT_GE(stats, 0);
}

HWTEST_F(NetsysBpfStatsTest, GetIfaceStats, TestSize.Level1)
{
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    uint64_t stats = 0;
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_IFACE_NAME_WLAN0);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_IFACE_NAME_WLAN0);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_IFACE_NAME_WLAN0);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_IFACE_NAME_WLAN0);
    EXPECT_GE(stats, 0);

    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_IFACE_NAME_LO);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_IFACE_NAME_LO);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_IFACE_NAME_LO);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_IFACE_NAME_LO);
    EXPECT_GE(stats, 0);

    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_IFACE_NAME_DUMMY0);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_IFACE_NAME_DUMMY0);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_IFACE_NAME_DUMMY0);
    EXPECT_GE(stats, 0);
    bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_IFACE_NAME_DUMMY0);
    EXPECT_GE(stats, 0);
}

HWTEST_F(NetsysBpfStatsTest, GetAllStatsInfo, TestSize.Level1)
{
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> stats;
    bpfStats->GetAllStatsInfo(stats);
    EXPECT_GE(stats.size(), 0);
}

HWTEST_F(NetsysBpfStatsTest, LoadElf, TestSize.Level1)
{
    auto ret = OHOS::NetManagerStandard::LoadElf(TEST_BFP_NAME_NETSYS_PATH);
    EXPECT_GE(ret, NETSYS_SUCCESS);

    ret = OHOS::NetManagerStandard::LoadElf(BFP_NAME_NETSYS_PATH);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetsysBpfStatsTest, LoadAndUidStats, TestSize.Level1)
{
    BpfMapper<app_uid_stats_key, app_uid_stats_value> appUidStatsMap(APP_UID_STATS_MAP_PATH, BPF_ANY);
    EXPECT_TRUE(appUidStatsMap.IsValid());
    app_uid_stats_value value;
    value.rxBytes = TEST_BYTES0;
    value.rxPackets = TEST_BYTES0;
    value.txBytes = TEST_BYTES0;
    value.txPackets = TEST_BYTES0;
    auto ret = appUidStatsMap.Write(TEST_UID1, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    uint64_t stats = 0;
    EXPECT_EQ(bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_UID1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_UID1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_UID1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetUidStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_UID1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);

    ret = appUidStatsMap.Delete(TEST_UID1);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::vector<app_uid_stats_key> keys;
    keys.emplace_back(TEST_UID1);
    keys.emplace_back(TEST_UID2);
    ret = appUidStatsMap.Clear(keys);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetsysBpfStatsTest, LoadAndIfaceStats, TestSize.Level1)
{
    BpfMapper<iface_stats_key, iface_stats_value> ifaceStatsMap(IFACE_STATS_MAP_PATH, BPF_ANY);
    EXPECT_TRUE(ifaceStatsMap.IsValid());

    auto ifIndex = if_nametoindex(TEST_IFACE_NAME_WLAN0);

    auto keys = ifaceStatsMap.GetAllKeys();
    auto r = ifaceStatsMap.Clear(keys);
    EXPECT_EQ(r, NETSYS_SUCCESS);

    iface_stats_value ifaceStats = {0};
    ifaceStats.rxBytes = TEST_BYTES0;
    ifaceStats.rxPackets = TEST_BYTES0;
    ifaceStats.txBytes = TEST_BYTES0;
    ifaceStats.txPackets = TEST_BYTES0;
    auto ret = ifaceStatsMap.Write(ifIndex, ifaceStats, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    uint64_t stats = 0;
    EXPECT_EQ(bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_IFACE_NAME_WLAN0), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_IFACE_NAME_WLAN0), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_IFACE_NAME_WLAN0), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetIfaceStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_IFACE_NAME_WLAN0), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);

    stats = 0;
    EXPECT_EQ(bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_RX_BYTES), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_RX_PACKETS), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_TX_BYTES), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetTotalStats(stats, StatsType::STATS_TYPE_TX_PACKETS), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);

    ret = ifaceStatsMap.Delete(ifIndex);
    EXPECT_EQ(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetsysBpfStatsTest, LoadAndUidIfaceStats, TestSize.Level1)
{
    BpfMapper<app_uid_if_stats_key, app_uid_if_stats_value> uidIfaceStatsMap(APP_UID_IF_STATS_MAP_PATH, BPF_ANY);
    EXPECT_TRUE(uidIfaceStatsMap.IsValid());

    app_uid_if_stats_value value = {0};
    value.rxBytes = TEST_BYTES0;
    value.rxPackets = TEST_BYTES0;
    value.txBytes = TEST_BYTES0;
    value.txPackets = TEST_BYTES0;
    app_uid_if_stats_key key1 = {0};
    key1.ifIndex = TEST_UID_IF1;
    key1.uId = TEST_UID1;
    auto ret = uidIfaceStatsMap.Write(key1, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    app_uid_if_stats_key key2 = {0};
    key2.ifIndex = TEST_UID_IF2;
    key2.uId = TEST_UID2;
    ret = uidIfaceStatsMap.Write(key2, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> stats;
    EXPECT_EQ(bpfStats->GetAllStatsInfo(stats), NETSYS_SUCCESS);
}

HWTEST_F(NetsysBpfStatsTest, LoadAndCookieStats, TestSize.Level1)
{
    BpfMapper<socket_cookie_stats_key, app_cookie_stats_value> appCookieStatsMap(APP_COOKIE_STATS_MAP_PATH, BPF_ANY);
    EXPECT_TRUE(appCookieStatsMap.IsValid());
    app_cookie_stats_value value;
    value.rxBytes = TEST_BYTES0;
    value.rxPackets = TEST_BYTES0;
    value.txBytes = TEST_BYTES0;
    value.txPackets = TEST_BYTES0;
    auto ret = appCookieStatsMap.Write(TEST_COOKIE1, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    uint64_t stats = 0;
    EXPECT_EQ(bpfStats->GetCookieStats(stats, StatsType::STATS_TYPE_RX_BYTES, TEST_COOKIE1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetCookieStats(stats, StatsType::STATS_TYPE_RX_PACKETS, TEST_COOKIE1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetCookieStats(stats, StatsType::STATS_TYPE_TX_BYTES, TEST_COOKIE1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);
    EXPECT_EQ(bpfStats->GetCookieStats(stats, StatsType::STATS_TYPE_TX_PACKETS, TEST_COOKIE1), NETSYS_SUCCESS);
    EXPECT_EQ(stats, TEST_BYTES0);

    ret = appCookieStatsMap.Delete(TEST_COOKIE1);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::vector<socket_cookie_stats_key> keys;
    keys.emplace_back(TEST_COOKIE1);
    ret = appCookieStatsMap.Clear(keys);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetsysBpfStatsTest, GetAllStatsInfoTest001, TestSize.Level1)
{
    BpfMapper<app_uid_sim_stats_key, app_uid_sim_stats_value> uidSimStatsMap(APP_UID_SIM_STATS_MAP_PATH, BPF_ANY);
    EXPECT_TRUE(uidSimStatsMap.IsValid());

    app_uid_sim_stats_value value = {0};
    value.rxBytes = TEST_BYTES0;
    value.rxPackets = TEST_BYTES0;
    value.txBytes = TEST_BYTES0;
    value.txPackets = TEST_BYTES0;
    app_uid_sim_stats_key key1 = {0};
    key1.ifIndex = TEST_UID_IF1;
    key1.uId = TEST_UID1;
    auto ret = uidSimStatsMap.Write(key1, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    app_uid_sim_stats_key key2 = {0};
    key2.ifIndex = TEST_UID_IF2;
    key2.uId = TEST_UID2;
    ret = uidSimStatsMap.Write(key2, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> stats;
    EXPECT_EQ(bpfStats->GetAllStatsInfo(stats), NETSYS_SUCCESS);
}

HWTEST_F(NetsysBpfStatsTest, GetAllContainerStatsInfo001, TestSize.Level1)
{
    BpfMapper<app_uid_sim_stats_key, app_uid_sim_stats_value> uidSimStatsMap(APP_UID_SIM_STATS_MAP_PATH, BPF_ANY);
    EXPECT_TRUE(uidSimStatsMap.IsValid());

    app_uid_sim_stats_value value = {0};
    value.rxBytes = TEST_BYTES0;
    value.rxPackets = TEST_BYTES0;
    value.txBytes = TEST_BYTES0;
    value.txPackets = TEST_BYTES0;
    app_uid_sim_stats_key key1 = {0};
    key1.ifIndex = TEST_UID_IF1;
    key1.uId = TEST_UID1;
    auto ret = uidSimStatsMap.Write(key1, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    app_uid_sim_stats_key key2 = {0};
    key2.ifIndex = TEST_UID_IF2;
    key2.uId = TEST_UID2;
    ret = uidSimStatsMap.Write(key2, value, BPF_ANY);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> stats;
    EXPECT_EQ(bpfStats->GetAllContainerStatsInfo(stats), NETSYS_SUCCESS);
    EXPECT_EQ(stats.size(), 2);
}

HWTEST_F(NetsysBpfStatsTest, UnloadElf, TestSize.Level1)
{
    auto ret = OHOS::NetManagerStandard::UnloadElf(BFP_NAME_NETSYS_PATH);
    EXPECT_EQ(ret, NETSYS_SUCCESS);

    ret = OHOS::NetManagerStandard::UnloadElf(TEST_BFP_NAME_NETSYS_PATH);
    EXPECT_GE(ret, NETSYS_SUCCESS);
}

HWTEST_F(NetsysBpfStatsTest, GetNumberFromStatsValue, TestSize.Level1)
{
    uint64_t stats = 0;
    StatsType statsType = static_cast<StatsType>(STATS_TYPE_INVALID_VALUE);
    stats_value value = {};
    std::unique_ptr<NetsysBpfStats> bpfStats = std::make_unique<NetsysBpfStats>();
    auto ret = bpfStats->GetNumberFromStatsValue(stats, statsType, value);
    EXPECT_EQ(ret, NetManagerStandard::NetStatsResultCode::STATS_ERR_READ_BPF_FAIL);
}
} // namespace NetManagerStandard
} // namespace OHOS
