/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "wrapper_distributor.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
constexpr int32_t TEST_SOCKET = 112;
constexpr int32_t TEST_FORMAT = NetlinkDefine::NETLINK_FORMAT_BINARY_UNICAST;
} // namespace

class WrapperDistributorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<WrapperDistributor> instance_ =
        std::make_shared<WrapperDistributor>(TEST_SOCKET, TEST_FORMAT);
};

void WrapperDistributorTest::SetUpTestCase() {}

void WrapperDistributorTest::TearDownTestCase() {}

void WrapperDistributorTest::SetUp() {}

void WrapperDistributorTest::TearDown() {}

HWTEST_F(WrapperDistributorTest, SocketErrorTest001, TestSize.Level1)
{
    int32_t testSocket = -1;
    std::unique_ptr<WrapperDistributor> receiver = std::make_unique<WrapperDistributor>(testSocket, TEST_FORMAT);
    ASSERT_NE(receiver, nullptr);
}

HWTEST_F(WrapperDistributorTest, FormatErrorTest001, TestSize.Level1)
{
    int32_t testFormat = 6;
    std::unique_ptr<WrapperDistributor> distributor = std::make_unique<WrapperDistributor>(TEST_SOCKET, testFormat);
    ASSERT_NE(distributor, nullptr);
}

HWTEST_F(WrapperDistributorTest, StartTest001, TestSize.Level1)
{
    int32_t ret = instance_->Start();
    EXPECT_EQ(ret, NetlinkResult::OK);
}

HWTEST_F(WrapperDistributorTest, StopTest001, TestSize.Level1)
{
    int32_t ret = instance_->Stop();
    EXPECT_EQ(ret, NetlinkResult::OK);
}

HWTEST_F(WrapperDistributorTest, RegisterNetlinkCallbacksTest001, TestSize.Level1)
{
    int32_t ret = instance_->RegisterNetlinkCallbacks(nullptr);
    EXPECT_EQ(ret, NetlinkResult::ERR_NULL_PTR);
}

HWTEST_F(WrapperDistributorTest, RegisterNetlinkCallbacksTest002, TestSize.Level1)
{
    auto callbacks_ = std::make_shared<std::vector<sptr<NetsysNative::INotifyCallback>>>();
    int32_t ret = instance_->RegisterNetlinkCallbacks(callbacks_);
    EXPECT_EQ(ret, NetlinkResult::OK);
}
} // namespace nmd
} // namespace OHOS