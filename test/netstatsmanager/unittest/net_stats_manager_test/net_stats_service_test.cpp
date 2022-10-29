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

#include "net_mgr_log_wrapper.h"
#include "net_stats_callback_test.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string ETH_IFACE_NAME = "lo";
constexpr int64_t TEST_UID = 1010;

using namespace testing::ext;
class NetStatsServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetStatsCallbackTest> GetINetStatsCallbackSample() const;
    uint32_t GetTestTime();
};

void NetStatsServiceTest::SetUpTestCase() {}

void NetStatsServiceTest::TearDownTestCase() {}

void NetStatsServiceTest::SetUp() {}

void NetStatsServiceTest::TearDown() {}

sptr<NetStatsCallbackTest> NetStatsServiceTest::GetINetStatsCallbackSample() const
{
    sptr<NetStatsCallbackTest> callback = std::make_unique<NetStatsCallbackTest>().release();
    return callback;
}

/**
 * @tc.name: NetStatsServiceTest001
 * @tc.desc: Test NetStatsServiceTest GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager001, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceRxBytes(ETH_IFACE_NAME);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsServiceTest002
 * @tc.desc: Test NetStatsServiceTest GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager002, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceTxBytes(ETH_IFACE_NAME);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsServiceTest003
 * @tc.desc: Test NetStatsServiceTest GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager003, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularRxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsServiceTest004
 * @tc.desc: Test NetStatsServiceTest GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager004, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularTxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsServiceTest005
 * @tc.desc: Test NetStatsServiceTest GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager005, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllRxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsServiceTest006
 * @tc.desc: Test NetStatsServiceTest GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager007, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllTxBytes();
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsServiceTest008
 * @tc.desc: Test NetStatsServiceTest GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager008, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidRxBytes(TEST_UID);
    ASSERT_TRUE(ret >= 0);
}

/**
 * @tc.name: NetStatsServiceTest009
 * @tc.desc: Test NetStatsServiceTest GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(NetStatsServiceTest, NetStatsManager009, TestSize.Level1)
{
    long ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidTxBytes(TEST_UID);
    ASSERT_TRUE(ret >= 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
