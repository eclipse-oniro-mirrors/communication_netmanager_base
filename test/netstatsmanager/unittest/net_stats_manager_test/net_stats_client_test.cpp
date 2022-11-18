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

#include "net_stats_callback_test.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"
#include "net_manager_center.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *ETH_IFACE_NAME = "lo";
constexpr int64_t TEST_UID = 1010;
void GetIfaceNamesFromManager(std::list<std::string> &ifaceNames)
{
    NetManagerCenter::GetInstance().GetIfaceNames(BEARER_CELLULAR, ifaceNames);
}
} // namespace

using namespace testing::ext;
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
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceRxBytes(ETH_IFACE_NAME);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetIfaceTxBytesTest001
 * @tc.desc: Test NetStatsClient GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetIfaceTxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceTxBytes(ETH_IFACE_NAME);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetCellularRxBytesTest001
 * @tc.desc: Test NetStatsClient GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetCellularRxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularRxBytes();
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetCellularTxBytesTest001
 * @tc.desc: Test NetStatsClient GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetCellularTxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularTxBytes();
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetAllRxBytesTest001
 * @tc.desc: Test NetStatsClient GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetAllRxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllRxBytes();
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetAllTxBytesTest001
 * @tc.desc: Test NetStatsClient GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetAllTxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllTxBytes();
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetUidRxBytesTest001
 * @tc.desc: Test NetStatsClient GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetUidRxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidRxBytes(TEST_UID);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetUidTxBytesTest001
 * @tc.desc: Test NetStatsClient GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsClientTest, GetUidTxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidTxBytes(TEST_UID);
    EXPECT_GE(ret, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
