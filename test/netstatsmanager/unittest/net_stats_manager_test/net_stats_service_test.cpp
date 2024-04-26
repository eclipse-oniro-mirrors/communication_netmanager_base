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

#include "net_manager_center.h"
#include "net_stats_callback_test.h"
#include "net_stats_constants.h"
#include "net_stats_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
constexpr const char *ETH_IFACE_NAME = "lo";
constexpr int64_t TEST_UID = 1010;
constexpr int32_t TEST_FD = 2;
static constexpr uint64_t TEST_COOKIE = 1;
void GetIfaceNamesFromManager(std::list<std::string> &ifaceNames)
{
    NetManagerCenter::GetInstance().GetIfaceNames(BEARER_CELLULAR, ifaceNames);
}
} // namespace

using namespace testing::ext;
class NetStatsServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    uint32_t GetTestTime();
    static inline sptr<NetStatsCallbackTest> callback_ = nullptr;
};

void NetStatsServiceTest::SetUpTestCase()
{
    callback_ = new (std::nothrow) NetStatsCallbackTest();
    DelayedSingleton<NetStatsService>::GetInstance()->OnStart();
}

void NetStatsServiceTest::TearDownTestCase()
{
    DelayedSingleton<NetStatsService>::GetInstance()->OnStop();
}

void NetStatsServiceTest::SetUp() {}

void NetStatsServiceTest::TearDown() {}

/**
 * @tc.name: DumpTest001
 * @tc.desc: Test NetStatsService RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, DumpTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->Dump(TEST_FD, {});
    EXPECT_GE(ret, -1);
}

/**
 * @tc.name: RegisterNetStatsCallbackTest001
 * @tc.desc: Test NetStatsService RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, RegisterNetStatsCallbackTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->RegisterNetStatsCallback(callback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetStatsCallbackTest002
 * @tc.desc: Test NetStatsService RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, RegisterNetStatsCallbackTest002, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->RegisterNetStatsCallback(nullptr);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest001
 * @tc.desc: Test NetStatsService UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, UnregisterNetStatsCallbackTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->UnregisterNetStatsCallback(callback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest002
 * @tc.desc: Test NetStatsService UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, UnregisterNetStatsCallbackTest002, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->UnregisterNetStatsCallback(nullptr);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIfaceRxBytesTest001
 * @tc.desc: Test NetStatsService GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetIfaceRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetIfaceRxBytes(stats, ETH_IFACE_NAME);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetIfaceTxBytesTest001
 * @tc.desc: Test NetStatsService GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetIfaceTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetIfaceTxBytes(stats, ETH_IFACE_NAME);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetCellularRxBytesTest001
 * @tc.desc: Test NetStatsService GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetCellularRxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetCellularRxBytes(stats);
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(stats, static_cast<uint64_t>(0));
}

/**
 * @tc.name: GetCellularTxBytesTest001
 * @tc.desc: Test NetStatsService GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetCellularTxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetCellularTxBytes(stats);
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(stats, static_cast<uint64_t>(0));
}

/**
 * @tc.name: GetAllRxBytesTest001
 * @tc.desc: Test NetStatsService GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetAllRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetAllRxBytes(stats);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetAllTxBytesTest001
 * @tc.desc: Test NetStatsService GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetAllTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetAllTxBytes(stats);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetUidRxBytesTest001
 * @tc.desc: Test NetStatsService GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetUidRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetUidRxBytes(stats, TEST_UID);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetUidTxBytesTest001
 * @tc.desc: Test NetStatsService GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetUidTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetUidTxBytes(stats, TEST_UID);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetIfaceStatsDetail001
 * @tc.desc: Test NetStatsService GetIfaceStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetIfaceStatsDetail001, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetIfaceStatsDetail(iface, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetUidStatsDetail001
 * @tc.desc: Test NetStatsService GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetUidStatsDetail001, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    uint32_t uid = 1234;
    int32_t ret =
        DelayedSingleton<NetStatsService>::GetInstance()->GetUidStatsDetail(iface, uid, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateIfacesStats
 * @tc.desc: Test NetStatsService UpdateIfacesStats.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, UpdateIfacesStats, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->UpdateIfacesStats(iface, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: ResetFactory001
 * @tc.desc: Test NetStatsService ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, ResetFactory001, TestSize.Level1)
{
    NetStatsInfo info;
    info.iface_ = "wlan0";
    info.date_ = 115200;
    info.rxBytes_ = 10000;
    info.txBytes_ = 11000;
    info.rxPackets_ = 1000;
    info.txPackets_ = 1100;

    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateStatsData001
 * @tc.desc: Test NetStatsService UpdateStatsData.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, UpdateStatsData001, TestSize.Level1)
{
    NetStatsInfo info;
    info.iface_ = "wlan0";
    info.date_ = 115200;
    info.rxBytes_ = 10000;
    info.txBytes_ = 11000;
    info.rxPackets_ = 1000;
    info.txPackets_ = 1100;

    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->UpdateStatsData();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetCookieRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetCookieRxBytes(stats, TEST_COOKIE);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetCookieTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetCookieTxBytes(stats, TEST_COOKIE);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetAllContainerStatsInfoTest001, TestSize.Level1)
{
    std::vector<NetStatsInfo> infos;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetAllContainerStatsInfo(infos);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetTrafficStatsByNetworkTest001, TestSize.Level1)
{
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetTrafficStatsByNetwork(infos, network);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, GetTrafficStatsByUidNetworkTest001, TestSize.Level1)
{
    std::vector<NetStatsInfoSequence> infos;
    uint32_t uid = 1;
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork();
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetTrafficStatsByUidNetwork(infos, uid, network);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceTest, SetAppStats001, TestSize.Level1)
{
    PushStatsInfo info;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->SetAppStats(info);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
