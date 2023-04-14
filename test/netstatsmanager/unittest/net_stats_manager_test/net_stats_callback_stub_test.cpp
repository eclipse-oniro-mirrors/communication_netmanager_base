/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "net_stats_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
class MockNetStatsCallbackStubTest : public NetStatsCallbackStub {
public:
    MockNetStatsCallbackStubTest() = default;
    ~MockNetStatsCallbackStubTest() override {}
    int32_t NetIfaceStatsChanged(const std::string &iface) override
    {
        std::cout << std::endl;
        std::cout << "Stub NetIfaceStatsChanged::iface: " << iface << std::endl;
        return 0;
    }
    int32_t NetUidStatsChanged(const std::string &iface, uint32_t uid) override
    {
        std::cout << std::endl;
        std::cout << "Stub NetUidStatsChanged::iface: " << iface << ", uid:" << uid << std::endl;
        return 0;
    }
};
} // namespace

using namespace testing::ext;
class NetStatsCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetStatsCallbackStub> instance_ = std::make_shared<MockNetStatsCallbackStubTest>();
};

void NetStatsCallbackStubTest::SetUpTestCase() {}

void NetStatsCallbackStubTest::TearDownTestCase() {}

void NetStatsCallbackStubTest::SetUp() {}

void NetStatsCallbackStubTest::TearDown() {}

HWTEST_F(NetStatsCallbackStubTest, OnRemoteRequest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(100, data, reply, option);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsCallbackStubTest, OnRemoteRequest002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int32_t ret = instance_->OnRemoteRequest(0, data, reply, option);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsCallbackStubTest, OnRemoteRequest003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int32_t ret = instance_->OnRemoteRequest(1, data, reply, option);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsCallbackStubTest, OnNetIfaceStatsChanged001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnNetIfaceStatsChanged(data, reply);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsCallbackStubTest, OnNetUidStatsChanged001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = instance_->OnNetUidStatsChanged(data, reply);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS