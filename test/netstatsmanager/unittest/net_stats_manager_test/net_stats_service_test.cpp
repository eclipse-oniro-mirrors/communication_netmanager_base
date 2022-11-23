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
constexpr const char *ETH_IFACE_NAME = "lo";
constexpr int64_t TEST_UID = 1010;
constexpr int32_t TEST_FD = 2;
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
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: RegisterNetStatsCallbackTest002
 * @tc.desc: Test NetStatsService RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, RegisterNetStatsCallbackTest002, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->RegisterNetStatsCallback(nullptr);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest001
 * @tc.desc: Test NetStatsService UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, UnregisterNetStatsCallbackTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->UnregisterNetStatsCallback(callback_);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest002
 * @tc.desc: Test NetStatsService UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, UnregisterNetStatsCallbackTest002, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->UnregisterNetStatsCallback(nullptr);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: GetIfaceRxBytesTest001
 * @tc.desc: Test NetStatsService GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetIfaceRxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetIfaceRxBytes(ETH_IFACE_NAME);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetIfaceTxBytesTest001
 * @tc.desc: Test NetStatsService GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetIfaceTxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetIfaceTxBytes(ETH_IFACE_NAME);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetCellularRxBytesTest001
 * @tc.desc: Test NetStatsService GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetCellularRxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetCellularRxBytes();
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetCellularTxBytesTest001
 * @tc.desc: Test NetStatsService GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetCellularTxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetCellularTxBytes();
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetAllRxBytesTest001
 * @tc.desc: Test NetStatsService GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetAllRxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetAllRxBytes();
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetAllTxBytesTest001
 * @tc.desc: Test NetStatsService GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetAllTxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetAllTxBytes();
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetUidRxBytesTest001
 * @tc.desc: Test NetStatsService GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetUidRxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetUidRxBytes(TEST_UID);
    EXPECT_GE(ret, 0);
}

/**
 * @tc.name: GetUidTxBytesTest001
 * @tc.desc: Test NetStatsService GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, GetUidTxBytesTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->GetUidTxBytes(TEST_UID);
    EXPECT_GE(ret, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
