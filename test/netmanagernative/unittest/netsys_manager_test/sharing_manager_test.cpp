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

#include "sharing_manager.h"

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
}
} // namespace NetsysNative
} // namespace OHOS