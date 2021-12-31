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

#include "net_score.h"

#include "net_mgr_log_wrapper.h"
#include "net_conn_types.h"

#include <gtest/gtest.h>

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetScoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<NetScore> netScore_ = nullptr;
};

void NetScoreTest::SetUpTestCase()
{}

void NetScoreTest::TearDownTestCase() {}

void NetScoreTest::SetUp()
{
    netScore_ = std::make_unique<NetScore>();
    if (netScore_ == nullptr) {
        NETMGR_LOG_E("Make NetScore failed");
        return;
    }
}

void NetScoreTest::TearDown() {}

HWTEST_F(NetScoreTest, GetServiceScore, TestSize.Level1)
{
    uint64_t netCapabilities = 0x03;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident";
    NetworkType netType = NET_TYPE_CELLULAR;
    sptr<NetSupplier> supplier = (std::make_unique<NetSupplier>(netType, ident, netCapabilities)).release();

    // mock Failed to detect network
    supplier->SetNetValid(false);

    bool result = netScore_->GetServiceScore(supplier);
    ASSERT_TRUE(result == true);
    ASSERT_TRUE(supplier->GetNetScore() == static_cast<int32_t>(NetTypeScoreValue::CELLULAR_VALUE));
    ASSERT_TRUE(supplier->GetRealScore() ==
        (static_cast<int32_t>(NetTypeScoreValue::CELLULAR_VALUE) - NET_VALID_SCORE));

    // mock successed to detect network
    supplier->SetNetValid(true);

    result = netScore_->GetServiceScore(supplier);
    ASSERT_TRUE(result == true);
    ASSERT_TRUE(supplier->GetNetScore() == static_cast<int32_t>(NetTypeScoreValue::CELLULAR_VALUE));
    ASSERT_TRUE(supplier->GetRealScore() == static_cast<int32_t>(NetTypeScoreValue::CELLULAR_VALUE));
}
} // namespace NetManagerStandard
} // namespace OHOS