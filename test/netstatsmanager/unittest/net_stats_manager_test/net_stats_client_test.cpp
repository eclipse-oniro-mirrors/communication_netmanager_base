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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_callback_test.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
constexpr const char *ETH_IFACE_NAME = "lo";
constexpr int64_t TEST_UID = 1010;
void GetIfaceNamesFromManager(std::list<std::string> &ifaceNames)
{
    NetManagerCenter::GetInstance().GetIfaceNames(BEARER_CELLULAR, ifaceNames);
}
constexpr const char *MOCK_IFACE = "wlan0";
constexpr uint32_t MOCK_UID = 1234;
constexpr uint64_t MOCK_DATE = 115200;
constexpr uint64_t MOCK_RXBYTES = 10000;
constexpr uint64_t MOCK_TXBYTES = 11000;
constexpr uint64_t MOCK_RXPACKETS = 1000;
constexpr uint64_t MOCK_TXPACKETS = 1100;

HapInfoParams testInfoParms = {
    .userID = 1,
    .bundleName = "net_stats_manager_test",
    .instIndex = 0,
    .appIDDesc = "test",
    .isSystemApp = true,
};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.GET_NETWORK_STATS",
    .bundleName = "net_stats_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net stats connectivity internal",
    .descriptionId = 1,
};

PermissionStateFull testState = {
    .permissionName = "ohos.permission.GET_NETWORK_STATS",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef},
    .permStateList = {testState},
};

class AccessToken {
public:
    AccessToken() : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(tokenIdEx.tokenIDEx);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};
} // namespace

class NetStatsClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    uint32_t GetTestTime();
    static inline sptr<NetStatsCallbackTest> callback_ = nullptr;
};

void NetStatsClientTest::SetUpTestCase()
{
    callback_ = new (std::nothrow) NetStatsCallbackTest();
}

void NetStatsClientTest::TearDownTestCase() {}

void NetStatsClientTest::SetUp() {}

void NetStatsClientTest::TearDown() {}

/**
 * @tc.name: RegisterNetStatsCallbackTest001
 * @tc.desc: Test NetStatsClient RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, RegisterNetStatsCallbackTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(callback_);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest001
 * @tc.desc: Test NetStatsClient UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, UnregisterNetStatsCallbackTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(callback_);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetIfaceRxBytesTest001
 * @tc.desc: Test NetStatsClient GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetIfaceRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceRxBytes(stats, ETH_IFACE_NAME);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetIfaceTxBytesTest001
 * @tc.desc: Test NetStatsClient GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetIfaceTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceTxBytes(stats, ETH_IFACE_NAME);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetCellularRxBytesTest001
 * @tc.desc: Test NetStatsClient GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetCellularRxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularRxBytes(stats);
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(stats, static_cast<uint64_t>(0));
}

/**
 * @tc.name: GetCellularTxBytesTest001
 * @tc.desc: Test NetStatsClient GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetCellularTxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularTxBytes(stats);
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(stats, static_cast<uint64_t>(0));
}

/**
 * @tc.name: GetAllRxBytesTest001
 * @tc.desc: Test NetStatsClient GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetAllRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllRxBytes(stats);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetAllTxBytesTest001
 * @tc.desc: Test NetStatsClient GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetAllTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllTxBytes(stats);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetUidRxBytesTest001
 * @tc.desc: Test NetStatsClient GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetUidRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidRxBytes(stats, TEST_UID);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

/**
 * @tc.name: GetUidTxBytesTest001
 * @tc.desc: Test NetStatsClient GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetUidTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidTxBytes(stats, TEST_UID);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

HWTEST_F(NetStatsClientTest, NetStatsClient001, TestSize.Level1)
{
    AccessToken token;
    NetStatsInfo info;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceStatsDetail(MOCK_IFACE, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << info.IfaceData() << std::endl;
}

HWTEST_F(NetStatsClientTest, NetStatsClient002, TestSize.Level1)
{
    AccessToken token;
    NetStatsInfo info;
    int32_t ret =
        DelayedSingleton<NetStatsClient>::GetInstance()->GetUidStatsDetail(MOCK_IFACE, MOCK_UID, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    std::cout << info.UidData() << std::endl;
}

HWTEST_F(NetStatsClientTest, NetStatsClient003, TestSize.Level1)
{
    NETMGR_LOG_I("NetStatsClientTest::NetStatsClient003 enter");
    AccessToken token;
    NetStatsInfo info;
    info.iface_ = MOCK_IFACE;
    info.date_ = MOCK_DATE;
    info.rxBytes_ = MOCK_RXBYTES;
    info.txBytes_ = MOCK_TXBYTES;
    info.rxPackets_ = MOCK_RXPACKETS;
    info.txPackets_ = MOCK_TXPACKETS;
    NETMGR_LOG_I("UpdateIfacesStats enter");
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->UpdateIfacesStats(MOCK_IFACE, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    NETMGR_LOG_I("GetIfaceStatsDetail enter");
    DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceStatsDetail(MOCK_IFACE, 0, UINT32_MAX, info);
    EXPECT_EQ(info.iface_, MOCK_IFACE);
    EXPECT_EQ(info.date_, UINT32_MAX);
    EXPECT_EQ(info.rxBytes_, MOCK_RXBYTES);
    EXPECT_EQ(info.txBytes_, MOCK_TXBYTES);
    EXPECT_EQ(info.rxPackets_, MOCK_RXPACKETS);
    EXPECT_EQ(info.txPackets_, MOCK_TXPACKETS);
}

HWTEST_F(NetStatsClientTest, NetStatsClient004, TestSize.Level1)
{
    NetStatsInfo info;
    info.iface_ = MOCK_IFACE;
    info.date_ = MOCK_DATE;
    info.rxBytes_ = MOCK_RXBYTES;
    info.txBytes_ = MOCK_TXBYTES;
    info.rxPackets_ = MOCK_RXPACKETS;
    info.txPackets_ = MOCK_TXPACKETS;

    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsClientTest, NetStatsClient005, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->UpdateStatsData();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsClientTest, NetStatsClient006, TestSize.Level1)
{
    std::vector<NetStatsInfo> infos;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllStatsInfo(infos);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
