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

#include <cstdio>

#include <gtest/gtest.h>

#include "net_manager_constants.h"
#define private public
#include "sharing_manager.h"
#undef private

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace testing::ext;
using namespace nmd;
} // namespace

class SharingManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<SharingManager> sharingManager = nullptr;
};

void SharingManagerTest::SetUpTestCase()
{
    sharingManager = std::make_shared<SharingManager>();
}

void SharingManagerTest::TearDownTestCase() {}

void SharingManagerTest::SetUp() {}

void SharingManagerTest::TearDown() {}

HWTEST_F(SharingManagerTest, IpEnableForwardingTest, TestSize.Level1)
{
    auto result = sharingManager->IpEnableForwarding("aTestName");
    ASSERT_EQ(result, 0);
}

HWTEST_F(SharingManagerTest, IpDisableForwarding, TestSize.Level1)
{
    auto result = sharingManager->IpDisableForwarding("aTestName");
    ASSERT_EQ(result, 0);
}

HWTEST_F(SharingManagerTest, EnableNat001, TestSize.Level1)
{
    auto result = sharingManager->EnableNat("down", "up");
    ASSERT_EQ(result, 0);
}

HWTEST_F(SharingManagerTest, EnableNat002, TestSize.Level1)
{
    const std::string enableAction = "down";
    int32_t ret = sharingManager->EnableNat(enableAction, enableAction);
    ASSERT_EQ(ret, -1);

    const std::string upstreamIface = "_test0";
    ret = sharingManager->EnableNat(enableAction, upstreamIface);
    ASSERT_EQ(ret, -1);

    const std::string nullIface;
    ret = sharingManager->EnableNat(enableAction, nullIface);
    ASSERT_EQ(ret, -1);
}

HWTEST_F(SharingManagerTest, DisableNat001, TestSize.Level1)
{
    sharingManager->DisableNat("down", "up");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(SharingManagerTest, DisableNat002, TestSize.Level1)
{
    const std::string enableAction = "down";
    int32_t ret = sharingManager->DisableNat(enableAction, enableAction);
    ASSERT_EQ(ret, -1);

    const std::string upstreamIface = "_test0";
    ret = sharingManager->DisableNat(enableAction, upstreamIface);
    ASSERT_EQ(ret, -1);

    const std::string nullIface;
    ret = sharingManager->DisableNat(enableAction, nullIface);
    ASSERT_EQ(ret, -1);
}

HWTEST_F(SharingManagerTest, IpFwdAddInterfaceForward001, TestSize.Level1)
{
    sharingManager->IpfwdAddInterfaceForward("down", "up");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(SharingManagerTest, IpFwdAddInterfaceForward002, TestSize.Level1)
{
    const std::string enableAction = "down";
    int32_t ret = sharingManager->IpfwdAddInterfaceForward(enableAction, enableAction);
    ASSERT_EQ(ret, -1);
    const std::string fromIface = "_err";
    ret = sharingManager->IpfwdAddInterfaceForward(fromIface, enableAction);
    EXPECT_EQ(ret, -1);
    const std::string upstreamIface = "_test0";
    ret = sharingManager->IpfwdAddInterfaceForward(enableAction, upstreamIface);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(SharingManagerTest, IpFwdRemoveInterfaceForward001, TestSize.Level1)
{
    sharingManager->IpfwdRemoveInterfaceForward("down", "up");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(SharingManagerTest, IpFwdRemoveInterfaceForward002, TestSize.Level1)
{
    const std::string enableAction = "down";
    int32_t ret = sharingManager->IpfwdRemoveInterfaceForward(enableAction, enableAction);
    ASSERT_EQ(ret, -1);
    const std::string fromIface = "_err";
    ret = sharingManager->IpfwdRemoveInterfaceForward(fromIface, enableAction);
    EXPECT_EQ(ret, -1);
    const std::string upstreamIface = "_test0";
    ret = sharingManager->IpfwdRemoveInterfaceForward(enableAction, upstreamIface);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(SharingManagerTest, GetNetworkSharingTraffic001, TestSize.Level1)
{
    std::string downIface = "down0";
    std::string upIface = "up0";
    NetworkSharingTraffic traffic;
    int32_t ret = sharingManager->GetNetworkSharingTraffic(downIface, upIface, traffic);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(SharingManagerTest, GetNetworkSharingTraffic002, TestSize.Level1)
{
    std::string downIface = "eth0";
    std::string upIface = "wlan0";
    NetworkSharingTraffic traffic;
    int32_t ret = sharingManager->GetNetworkSharingTraffic(downIface, upIface, traffic);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(SharingManagerTest, SetIpFwdEnable001, TestSize.Level1)
{
    int32_t ret = sharingManager->SetIpFwdEnable();
    EXPECT_EQ(ret, 0);
    ret = sharingManager->SetForwardRules(false, " tetherctrl_FORWARD -j DROP");
    EXPECT_EQ(ret, 0);
}
} // namespace NetsysNative
} // namespace OHOS
