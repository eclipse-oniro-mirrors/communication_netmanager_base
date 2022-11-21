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
using namespace testing::ext;
using namespace nmd;
constexpr int RES_LEN = 2;

namespace {
const std::string GetResult(const std::string &cmd, int size)
{
    char res[RES_LEN];
    FILE *fp = popen(cmd.c_str(), "r");
    char *result = fgets(res, size, fp);
    pclose(fp);
    return result;
}
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
    sharingManager->IpEnableForwarding("aTestName");

    const std::string cmd = "/bin/cat /proc/sys/net/ipv4/ip_forward";
    const std::string result = GetResult(cmd, 2);
    ASSERT_EQ(result, "1");
}

HWTEST_F(SharingManagerTest, IpDisableForwarding, TestSize.Level1)
{
    sharingManager->IpDisableForwarding("aTestName");

    const std::string cmd = "/bin/cat /proc/sys/net/ipv4/ip_forward";
    const std::string result = GetResult(cmd, 2);
    ASSERT_EQ(result, "0");
}

HWTEST_F(SharingManagerTest, EnableNat, TestSize.Level1)
{
    sharingManager->EnableNat("down", "up");

    system("/system/bin/iptables -t nat -L tetherctrl_nat_POSTROUTING -nvx > EnableNat_result");
    system("/system/bin/iptables -t mangle -L tetherctrl_mangle_FORWARD -nvx >> EnableNat_result");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(SharingManagerTest, DisableNat, TestSize.Level1)
{
    sharingManager->DisableNat("down", "up");

    system("/system/bin/iptables -t nat -L tetherctrl_nat_POSTROUTING -nvx > DisableNat_result");
    system("/system/bin/iptables -t mangle -L tetherctrl_mangle_FORWARD -nvx >> DisableNat_result");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(SharingManagerTest, IpFwdAddInterfaceForward, TestSize.Level1)
{
    sharingManager->IpfwdAddInterfaceForward("down", "up");
    system("/system/bin/iptables -t filter -L -nvx > IpFwdAddInterfaceForward_result");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(SharingManagerTest, IpFwdRemoveInterfaceForward, TestSize.Level1)
{
    sharingManager->IpfwdRemoveInterfaceForward("down", "up");
    system("/system/bin/iptables -t filter -L -nvx > IpFwdRemoveInterfaceForward_result");
    ASSERT_STREQ("0", "0");
}
} // namespace NetsysNative
} // namespace OHOS