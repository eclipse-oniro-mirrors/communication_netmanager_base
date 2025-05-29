/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "netnative_log_wrapper.h"
#include "vpn_manager.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class VpnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void VpnManagerTest::SetUpTestCase() {}

void VpnManagerTest::TearDownTestCase() {}

void VpnManagerTest::SetUp() {}

void VpnManagerTest::TearDown() {}

HWTEST_F(VpnManagerTest, VpnManagerBranchTest001, TestSize.Level1)
{
    VpnManager::GetInstance().StartUnixSocketListen();
    VpnManager::GetInstance().StartVpnInterfaceFdListen();

    auto result = VpnManager::GetInstance().CreateVpnInterface();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    VpnManager::GetInstance().DestroyVpnInterface();

    std::string ifName = "";
    int32_t testNumber = 0;
    result = VpnManager::GetInstance().SetVpnMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    testNumber = 1;
    result = VpnManager::GetInstance().SetVpnMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    std::string tunAddr = "";
    result = VpnManager::GetInstance().SetVpnAddress(ifName, tunAddr, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    result = VpnManager::GetInstance().SetVpnUp();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    result = VpnManager::GetInstance().SetVpnDown();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    ifreq ifr;
    std::string cardName = "";
    result = VpnManager::GetInstance().InitIfreq(ifr, cardName);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    result = VpnManager::GetInstance().SendVpnInterfaceFdToClient(testNumber, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, VpnManagerBranchTest002, TestSize.Level1)
{
    VpnManager::GetInstance().tunFd_ = 1;
    auto ret = VpnManager::GetInstance().CreateVpnInterface();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(VpnManagerTest, VpnManagerBranchTest003, TestSize.Level1)
{
    auto ret = VpnManager::GetInstance().SetVpnMtu("eth0", 1);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(VpnManagerTest, VpnManagerBranchTest004, TestSize.Level1)
{
    std::string ifName;
    std::string tunAddr = "fe80::af71:b0c7:e3f7:3c0f%5";
    auto ret = VpnManager::GetInstance().SetVpnAddress(ifName, tunAddr, 0);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, VpnManagerBranchTest005, TestSize.Level1)
{
    std::string ifName;
    std::string tunAddr = "127.0.0.1";
    auto ret = VpnManager::GetInstance().SetVpnAddress(ifName, tunAddr, 0);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, VpnManagerBranchTest006, TestSize.Level1)
{
    VpnManager::GetInstance().net4Sock_ = -1;
    VpnManager::GetInstance().net6Sock_ = -1;
    auto ret = VpnManager::GetInstance().SetVpnUp();
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, VpnManagerBranchTest007, TestSize.Level1)
{
    VpnManager::GetInstance().net4Sock_ = -1;
    VpnManager::GetInstance().net6Sock_ = -1;
    auto ret = VpnManager::GetInstance().SetVpnDown();
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, CreateVpnInterfaceTest001, TestSize.Level1)
{
    VpnManager vpnmanager;
    vpnmanager.tunFd_ = 1;
    vpnmanager.listeningFlag_ = 1;
    auto result = vpnmanager.CreateVpnInterface();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(VpnManagerTest, SetVpnMtuTest001, TestSize.Level1)
{
    VpnManager vpnmanager;
    std::string ifName = "12345678901234567890";
    int32_t mtu = 1;
    auto result = vpnmanager.SetVpnMtu(ifName, mtu);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, SetVpnMtuTest002, TestSize.Level1)
{
    VpnManager vpnmanager;
    vpnmanager.net4Sock_ = -1;
    vpnmanager.net6Sock_ = -1;
    std::string ifName = "123456789abcdef";
    int32_t mtu = 1;
    auto result = vpnmanager.SetVpnMtu(ifName, mtu);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, SetVpnAddressTest001, TestSize.Level1)
{
    VpnManager vpnmanager;
    std::string ifName = "12345678901234567890";
    std::string tunAddr = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    int32_t prefix = 1;
    auto result = vpnmanager.SetVpnAddress(ifName, tunAddr, prefix);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, SetVpnAddressTest002, TestSize.Level1)
{
    VpnManager vpnmanager;
    std::string ifName = "12345678901234567890";
    std::string tunAddr = "192.168.1.1";
    int32_t prefix = 1;
    auto result = vpnmanager.SetVpnAddress(ifName, tunAddr, prefix);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, SetVpnAddressTest003, TestSize.Level1)
{
    VpnManager vpnmanager;
    std::string ifName = "12345678901234567890";
    std::string tunAddr = "";
    int32_t prefix = 1;
    auto result = vpnmanager.SetVpnAddress(ifName, tunAddr, prefix);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, SetVpnUpTest001, TestSize.Level1)
{
    VpnManager vpnmanager;
    vpnmanager.net4Sock_ = -1;
    vpnmanager.net6Sock_ = -1;
    auto result = vpnmanager.SetVpnUp();
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, SetVpnDownTest001, TestSize.Level1)
{
    VpnManager vpnmanager;
    vpnmanager.net4Sock_ = -1;
    vpnmanager.net6Sock_ = -1;
    auto result = vpnmanager.SetVpnDown();
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VpnManagerTest, InitIfreqTest001, TestSize.Level1)
{
    VpnManager vpnmanager;
    ifreq ifr;
    std::string cardName = "12345678901234567890";
    auto result = vpnmanager.InitIfreq(ifr, cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}
} // namespace NetManagerStandard
} // namespace OHOS
