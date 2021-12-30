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

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t WAIT_TIME_SECOND_LONG = 60;
constexpr int32_t TRIGER_DELAY_US = 100000;
constexpr uint32_t A_WEEK_TIME_MS = 604800;
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
    time_t now;
    time(&now);
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
 * @tc.name: NetStatsManager005
 * @tc.desc: Test NetStatsManagerTest GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager005, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int uid = 1001;
    long ret = flow->GetUidRxBytes(uid);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsManager006
 * @tc.desc: Test NetStatsManagerTest GetUidTxBytes.
 * @tc.type: FUNC
 */

HWTEST_F(NetStatsManagerTest, NetStatsManager006, TestSize.Level1)
{
    std::unique_ptr<DataFlowStatistics> flow = std::make_unique<DataFlowStatistics>();
    int uid = 1001;
    long ret = flow->GetUidTxBytes(uid);
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

void TrigerCallback()
{
    usleep(TRIGER_DELAY_US);
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->UpdateStatsData();
    ASSERT_TRUE(result == NetStatsResultCode::ERR_NONE);
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
    if (result == ERR_NONE) {
        std::thread trigerCallback(TrigerCallback);
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        trigerCallback.join();
        std::cout << "NetStatsClient008 RegisterNetStatsCallback" << std::endl;
    } else {
        std::cout << "NetStatsClient008 RegisterNetStatsCallback return fail" << std::endl;
    }

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
    uint32_t start = GetTestTime() - A_WEEK_TIME_MS;
    uint32_t end = GetTestTime();

    NetStatsInfo statsInfo ;
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceStatsDetail(iface,
        start, end, statsInfo);
    ASSERT_TRUE(result == NetStatsResultCode::ERR_NONE);
}

/**
 * @tc.name: NetStatsManager013
 * @tc.desc: Test NetStatsManagerTest GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager013, TestSize.Level1)
{
    std::string  iface = ETH_IFACE_NAME;
    uint32_t uid = 1001;
    uint32_t start = GetTestTime() - A_WEEK_TIME_MS;
    uint32_t end = GetTestTime();

    NetStatsInfo statsInfo;
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidStatsDetail(
        iface, uid, start, end, statsInfo);

    ASSERT_TRUE(result == NetStatsResultCode::ERR_NONE);
}

/**
 * @tc.name: NetStatsManager014
 * @tc.desc: Test NetStatsManagerTest UpdateIfacesStats.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager014, TestSize.Level1)
{
    std::string  iface =  ETH_IFACE_NAME;
    NetStatsInfo stats;
    stats.rxBytes_ = 2048;
    stats.txBytes_ = 1024;
    uint32_t start = GetTestTime() - A_WEEK_TIME_MS;
    uint32_t end = GetTestTime();
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->UpdateIfacesStats(iface,
        start, end, stats);
    ASSERT_TRUE(result == NetStatsResultCode::ERR_NONE);
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
/**
 * @tc.name: NetStatsManager016
 * @tc.desc: Test NetStatsManagerTest ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsManagerTest, NetStatsManager016, TestSize.Level1)
{
    NetStatsResultCode result = DelayedSingleton<NetStatsClient>::GetInstance()->ResetFactory();
    ASSERT_TRUE(result == NetStatsResultCode::ERR_NONE);
}
} // namespace NetManagerStandard
} // namespace OHOS