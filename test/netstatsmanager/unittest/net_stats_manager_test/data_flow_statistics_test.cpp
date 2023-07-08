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

#include "data_flow_statistics.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_callback_test.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
constexpr uint32_t TEST_UID = 1001;
const std::string ETH_IFACE_NAME = "eth0";
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
class DataFlowStatisticsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetStatsCallbackTest> GetINetStatsCallbackSample() const;
};

void DataFlowStatisticsTest::SetUpTestCase() {}

void DataFlowStatisticsTest::TearDownTestCase() {}

void DataFlowStatisticsTest::SetUp() {}

void DataFlowStatisticsTest::TearDown() {}

sptr<NetStatsCallbackTest> DataFlowStatisticsTest::GetINetStatsCallbackSample() const
{
    sptr<NetStatsCallbackTest> callback = new (std::nothrow) NetStatsCallbackTest();
    return callback;
}

/**
 * @tc.name: NetStatsManager001
 * @tc.desc: Test DataFlowStatisticsTest GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager001, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetCellularRxBytes();
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager002
 * @tc.desc: Test DataFlowStatisticsTest GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager002, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetCellularTxBytes();
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager003
 * @tc.desc: Test DataFlowStatisticsTest GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager003, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetAllRxBytes();
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager004
 * @tc.desc: Test DataFlowStatisticsTest GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager004, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetAllTxBytes();
    ASSERT_GE(ret, 0);
}

HWTEST_F(DataFlowStatisticsTest, NetStatsManager005, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int64_t ret = flow->GetUidTxBytes(TEST_UID);
    ASSERT_GE(ret, -1);
}

/**
 * @tc.name: NetStatsManager007
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager007, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceRxBytes(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager008
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager008, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceTxBytes(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager009
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceRxPackets.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager009, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceRxPackets(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager010
 * @tc.desc: Test DataFlowStatisticsTest GetIfaceTxPackets.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager010, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    int64_t ret = flow->GetIfaceTxPackets(iface);
    ASSERT_GE(ret, 0);
}

/**
 * @tc.name: NetStatsManager011
 * @tc.desc: Test DataFlowStatisticsTest RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(DataFlowStatisticsTest, NetStatsManager011, TestSize.Level1)
{
    AccessToken token;
    sptr<NetStatsCallbackTest> callback = GetINetStatsCallbackSample();
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
    result = DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(callback);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
