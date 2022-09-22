/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <vector>
#include <thread>

#include <gtest/gtest.h>
#include <ctime>

#include "net_stats_callback_test.h"
#include "data_flow_statistics.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"
#include "net_stats_client.h"
#include "net_stats_csv.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t TEST_UID = 1001;
constexpr int64_t TEST_BYTES = 4096;
const std::string ETH_IFACE_NAME = "eth0";

using namespace testing::ext;
class NetStatsManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetStatsCallbackTest> GetINetStatsCallbackSample() const;
    uint32_t GetTestTime();
};

void NetStatsManagerTest::SetUpTestCase() {}

void NetStatsManagerTest::TearDownTestCase() {}

void NetStatsManagerTest::SetUp() {}

void NetStatsManagerTest::TearDown() {}

sptr<NetStatsCallbackTest> NetStatsManagerTest::GetINetStatsCallbackSample() const
{
    sptr<NetStatsCallbackTest> callback = std::make_unique<NetStatsCallbackTest>().release();
    return callback;
}

uint32_t NetStatsManagerTest::GetTestTime()
{
    std::time_t now = std::time(nullptr);
    if (now < 0) {
        std::cout << ("NetStatsManagerTest GetTestTime failed") << std::endl;
    }
    std::stringstream ss;
    ss << now;
    return static_cast<uint32_t>(std::stoi(ss.str()));
}

/**
 * @tc.name: NetStatsManager001
 * @tc.desc: Test NetStatsManagerTest GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager001, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    long ret = flow->GetCellularRxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager002
 * @tc.desc: Test NetStatsManagerTest GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager002, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    long ret = flow->GetCellularTxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager003
 * @tc.desc: Test NetStatsManagerTest GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager003, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    long ret = flow->GetAllRxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager004
 * @tc.desc: Test NetStatsManagerTest GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager004, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    long ret = flow->GetAllTxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager007
 * @tc.desc: Test NetStatsManagerTest GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager007, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    long ret = flow->GetIfaceRxBytes(iface);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager008
 * @tc.desc: Test NetStatsManagerTest GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager008, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    long ret = flow->GetIfaceTxBytes(iface);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager009
 * @tc.desc: Test NetStatsManagerTest GetIfaceRxPackets.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager009, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    long ret = flow->GetIfaceRxPackets(iface);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager010
 * @tc.desc: Test NetStatsManagerTest GetIfaceTxPackets.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager010, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    std::string iface = ETH_IFACE_NAME;
    long ret = flow->GetIfaceTxPackets(iface);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager011
 * @tc.desc: Test NetStatsManagerTest RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager011, TestSize.Level1)
{
    sptr<NetStatsCallbackTest> callback = GetINetStatsCallbackSample();
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(callback);
    ASSERT_TRUE(result == ERR_NONE);
    result = DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(callback);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetStatsManager012
 * @tc.desc: Test NetStatsManagerTest GetIfaceStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager012, TestSize.Level1)
{
    std::string  iface =  ETH_IFACE_NAME;
    uint32_t start = GetTestTime();
    uint32_t end = GetTestTime();

    NetStatsInfo statsInfo ;
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceStatsDetail(iface,
        start, end, statsInfo);
    ASSERT_TRUE(result == NetStatsResultCode::ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NetStatsManager013
 * @tc.desc: Test NetStatsManagerTest GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager013, TestSize.Level1)
{
    std::string  iface = ETH_IFACE_NAME;
    uint32_t uid = TEST_UID;
    uint32_t start = GetTestTime();
    uint32_t end = GetTestTime();

    NetStatsInfo statsInfo;
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidStatsDetail(
        iface, uid, start, end, statsInfo);
    ASSERT_TRUE(result == NetStatsResultCode::ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: NetStatsManager014
 * @tc.desc: Test NetStatsManagerTest UpdateIfacesStats.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager014, TestSize.Level1)
{
    std::string iface;
    NetStatsInfo stats;
    stats.rxBytes_ = TEST_BYTES;
    stats.txBytes_ = TEST_BYTES;
    uint32_t start = GetTestTime();
    uint32_t end = GetTestTime();
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->UpdateIfacesStats(iface,
        start, end, stats);
    ASSERT_TRUE(result == NetStatsResultCode::ERR_INVALID_PARAMETER);
}
/**
 * @tc.name: NetStatsManager015
 * @tc.desc: Test NetStatsManagerTest UpdateStatsData.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager015, TestSize.Level1)
{
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->UpdateStatsData();
    ASSERT_TRUE(result == NetStatsResultCode::ERR_NONE);
}
} // namespace NetManagerStandard
} // namespace OHOS