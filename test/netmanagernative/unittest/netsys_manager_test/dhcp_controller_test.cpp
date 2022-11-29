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

#include "dhcp_controller.h"
#include "notify_callback_stub.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
using namespace NetsysNative;
class NotifyCallbackTest : public NotifyCallbackStub {
public:
    inline int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                             int scope) override
    {
        return 0;
    }
    inline int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                             int scope) override
    {
        return 0;
    }
    inline int32_t OnInterfaceAdded(const std::string &ifName) override
    {
        return 0;
    }
    inline int32_t OnInterfaceRemoved(const std::string &ifName) override
    {
        return 0;
    }
    inline int32_t OnInterfaceChanged(const std::string &ifName, bool up) override
    {
        return 0;
    }
    inline int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up) override
    {
        return 0;
    }
    inline int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                                  const std::string &ifName) override
    {
        return 0;
    }
    inline int32_t OnDhcpSuccess(sptr<DhcpResultParcel> &dhcpResult) override
    {
        return 0;
    }
    inline int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override
    {
        return 0;
    }
};
} // namespace

class DhcpControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = std::make_shared<DhcpController>();
};

void DhcpControllerTest::SetUpTestCase() {}

void DhcpControllerTest::TearDownTestCase() {}

void DhcpControllerTest::SetUp() {}

void DhcpControllerTest::TearDown() {}

HWTEST_F(DhcpControllerTest, RegisterNotifyCallbackTest001, TestSize.Level1)
{
    sptr<INotifyCallback> callback = new (std::nothrow) NotifyCallbackTest();
    auto ret = instance_->RegisterNotifyCallback(callback);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(DhcpControllerTest, StartDhcpTest001, TestSize.Level1)
{
    std::string testInterfaceName = "eth0";
    std::string testIpv4Addr = "112.254.154.415";
    instance_->StartDhcpClient(testInterfaceName, false);
    instance_->StopDhcpClient(testInterfaceName, false);
    instance_->StartDhcpClient(testInterfaceName, true);
    instance_->StopDhcpClient(testInterfaceName, true);
    auto ret = instance_->StartDhcpService(testInterfaceName, testIpv4Addr);
    ASSERT_FALSE(ret);
    ret = instance_->StopDhcpService(testInterfaceName);
    ASSERT_TRUE(ret);
    ret = instance_->StartDhcpService(testInterfaceName, {});
    ASSERT_FALSE(ret);
    ret = instance_->StopDhcpService(testInterfaceName);
    ASSERT_TRUE(ret);
}
} // namespace nmd
} // namespace OHOS