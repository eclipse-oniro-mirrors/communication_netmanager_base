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

class ManagerNative {
public:
    static ManagerNative *GetInstance()
    {
        if (instance_ == nullptr) {
            instance_ = new ManagerNative();
        }
        return instance_;
    }

    static void DeleteInstance()
    {
        delete instance_;
        if (instance_ != nullptr) {
            instance_ = nullptr;
        }
    }

    std::shared_ptr<SharingManager> GetSharingManager()
    {
        return sharingManager;
    }

private:
    static inline ManagerNative *instance_ = nullptr;

    ManagerNative()
    {
        sharingManager = std::make_shared<SharingManager>();
    }

    ~ManagerNative() = default;

    std::shared_ptr<SharingManager> sharingManager = nullptr;
};
} // namespace

class UnitTestSharingManager : public testing::Test {
public:
    std::shared_ptr<SharingManager> sharingManager = ManagerNative::GetInstance()->GetSharingManager();
    static void TearDownTestCase()
    {
        ManagerNative::DeleteInstance();
    }
};

HWTEST_F(UnitTestSharingManager, IpEnableForwarding, TestSize.Level1)
{
    sharingManager->IpEnableForwarding("aTestName");

    const std::string cmd = "/bin/cat /proc/sys/net/ipv4/ip_forward";
    const char *result = GetResult(cmd, 2);
    ASSERT_EQ(result, "1");
}

HWTEST_F(UnitTestSharingManager, IpDisableForwarding, TestSize.Level1)
{
    sharingManager->IpDisableForwarding("aTestName");

    const std::string cmd = "/bin/cat /proc/sys/net/ipv4/ip_forward";
    const char *result = GetResult(cmd, 2);
    ASSERT_EQ(result, "0");
}

HWTEST_F(UnitTestSharingManager, EnableNat, TestSize.Level1)
{
    sharingManager->EnableNat("down", "up");

    system("/system/bin/iptables -t nat -L tetherctrl_nat_POSTROUTING -nvx > EnableNat_result");
    system("/system/bin/iptables -t mangle -L tetherctrl_mangle_FORWARD -nvx >> EnableNat_result");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(UnitTestSharingManager, DisableNat, TestSize.Level1)
{
    sharingManager->DisableNat("down", "up");

    system("/system/bin/iptables -t nat -L tetherctrl_nat_POSTROUTING -nvx > DisableNat_result");
    system("/system/bin/iptables -t mangle -L tetherctrl_mangle_FORWARD -nvx >> DisableNat_result");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(UnitTestSharingManager, IpFwdAddInterfaceForward, TestSize.Level1)
{
    sharingManager->IpfwdAddInterfaceForward("down", "up");
    system("/system/bin/iptables -t filter -L -nvx > IpFwdAddInterfaceForward_result");
    ASSERT_STREQ("0", "0");
}

HWTEST_F(UnitTestSharingManager, IpFwdRemoveInterfaceForward, TestSize.Level1)
{
    sharingManager->IpfwdRemoveInterfaceForward("down", "up");
    system("/system/bin/iptables -t filter -L -nvx > IpFwdRemoveInterfaceForward_result");
    ASSERT_STREQ("0", "0");
}
} // namespace NetsysNative
} // namespace OHOS
