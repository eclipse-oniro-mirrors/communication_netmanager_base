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

#include "netlink_define.h"
#include "netlink_manager.h"
#include "notify_callback_stub.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
class TestNotifyCallback : public NetsysNative::NotifyCallbackStub {
public:
    TestNotifyCallback() = default;
    ~TestNotifyCallback() override {};
    int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                      int scope) override
    {
        return 0;
    }

    int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                      int scope) override
    {
        return 0;
    }

    int32_t OnInterfaceAdded(const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnInterfaceRemoved(const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnInterfaceChanged(const std::string &ifName, bool up) override
    {
        return 0;
    }

    int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up) override
    {
        return 0;
    }

    int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                           const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnDhcpSuccess(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult) override
    {
        return 0;
    }

    int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override
    {
        return 0;
    }
};
} // namespace

class NetlinkManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::unique_ptr<NetlinkManager> manager_ = nullptr;
};

void NetlinkManagerTest::SetUpTestCase()
{
    manager_ = std::make_unique<NetlinkManager>();
}

void NetlinkManagerTest::TearDownTestCase() {}

void NetlinkManagerTest::SetUp() {}

void NetlinkManagerTest::TearDown() {}

HWTEST_F(NetlinkManagerTest, StartListenerTest001, TestSize.Level1)
{
    int32_t ret = manager_->StartListener();
    EXPECT_EQ(ret, NetlinkResult::OK);
}

HWTEST_F(NetlinkManagerTest, RegisterNetlinkCallbackTest002, TestSize.Level1)
{
    sptr<NetsysNative::INotifyCallback> callback = nullptr;
    int32_t ret = manager_->RegisterNetlinkCallback(callback);
    EXPECT_NE(ret, NetlinkResult::OK);
    ret = manager_->UnregisterNetlinkCallback(callback);
    EXPECT_NE(ret, NetlinkResult::OK);

    callback = new (std::nothrow) TestNotifyCallback();
    ret = manager_->RegisterNetlinkCallback(callback);
    EXPECT_EQ(ret, NetlinkResult::OK);
    ret = manager_->UnregisterNetlinkCallback(callback);
    EXPECT_EQ(ret, NetlinkResult::OK);
}

HWTEST_F(NetlinkManagerTest, StopListenerTest003, TestSize.Level1)
{
    int32_t ret = manager_->StopListener();
    EXPECT_EQ(ret, NetlinkResult::OK);
}
} // namespace nmd
} // namespace OHOS
