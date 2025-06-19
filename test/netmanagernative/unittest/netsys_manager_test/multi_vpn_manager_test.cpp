/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "multi_vpn_manager.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *TEST_XFRM_CARD_NAME = "xfrm-vpn1";
constexpr const char *TEST_PPP_CARD_NAME = "ppp-vpn";
} // namespace

class MultiVpnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MultiVpnManagerTest::SetUpTestCase() {}

void MultiVpnManagerTest::TearDownTestCase() {}

void MultiVpnManagerTest::SetUp() {}

void MultiVpnManagerTest::TearDown() {}

HWTEST_F(MultiVpnManagerTest, VpnManagerBranchTest001, TestSize.Level1)
{
    MultiVpnManager::GetInstance().StartPppInterfaceFdListen(TEST_XFRM_CARD_NAME);
    MultiVpnManager::GetInstance().pppListeningFlag_ = false;
    MultiVpnManager::GetInstance().StartPppInterfaceFdListen(TEST_PPP_CARD_NAME);

    MultiVpnManager::GetInstance().StartPppInterfaceFdListen(TEST_PPP_CARD_NAME);

    MultiVpnManager::GetInstance().StartPppSocketListen(TEST_XFRM_CARD_NAME);
    MultiVpnManager::GetInstance().StartPppSocketListen(TEST_PPP_CARD_NAME);

    MultiVpnManager::GetInstance().SetXfrmPhyIfName("eth0");
    MultiVpnManager::GetInstance().DestroyVpnInterface(TEST_XFRM_CARD_NAME);

    auto result = MultiVpnManager::GetInstance().CreateVpnInterface(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    result = MultiVpnManager::GetInstance().CreateVpnInterface(TEST_PPP_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    result = MultiVpnManager::GetInstance().CreateVpnInterface("eth0");
    EXPECT_EQ(result, NETMANAGER_ERROR);

    MultiVpnManager::GetInstance().SetVpnRemoteAddress("192.168.1.1");

    std::string ifName = "";
    int32_t testNumber = 0;
    result = MultiVpnManager::GetInstance().SetVpnMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    testNumber = 1500;
    result = MultiVpnManager::GetInstance().SetVpnMtu(TEST_XFRM_CARD_NAME, testNumber);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    std::string ipAddr = "";
    result = MultiVpnManager::GetInstance().SetVpnAddress(ifName, ipAddr, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    result = MultiVpnManager::GetInstance().SetVpnUp(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    result = MultiVpnManager::GetInstance().SetVpnDown(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    ifreq ifr;
    std::string cardName = "xfrm-vpn2";
    result = MultiVpnManager::GetInstance().InitIfreq(ifr, cardName);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    uint32_t cmd = 0;
    std::atomic_int fd = 1;
    result = MultiVpnManager::GetInstance().SetVpnResult(fd, cmd, ifr);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    result = MultiVpnManager::GetInstance().DestroyVpnInterface(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    result = MultiVpnManager::GetInstance().SendVpnInterfaceFdToClient(testNumber, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, VpnManagerBranchTest002, TestSize.Level1)
{
    auto ret = MultiVpnManager::GetInstance().CreateVpnInterface(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, VpnManagerBranchTest003, TestSize.Level1)
{
    auto ret = MultiVpnManager::GetInstance().SetVpnMtu(TEST_XFRM_CARD_NAME, 1500);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, VpnManagerBranchTest004, TestSize.Level1)
{
    std::string ipAddr = "127.0.0.1";
    auto ret = MultiVpnManager::GetInstance().SetVpnAddress(TEST_XFRM_CARD_NAME, ipAddr, 0);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, VpnManagerBranchTest005, TestSize.Level1)
{
    ifreq ifr;
    uint32_t cmd = 0;
    std::atomic_int fd = 1;
    auto result = MultiVpnManager::GetInstance().SetVpnResult(fd, cmd, ifr);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetVpnMtuTest001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string ifName = "12345678901234567890";
    int32_t mtu = 1;
    auto result = multiVpnManager.SetVpnMtu(ifName, mtu);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetVpnMtuTest002, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string ifName = TEST_XFRM_CARD_NAME;
    int32_t mtu = 1500;
    auto result = multiVpnManager.SetVpnMtu(ifName, mtu);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, SetVpnAddressTest001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string ifName = "12345678901234567890";
    std::string ipAddr = "192.168.1.1";
    int32_t prefix = 1;
    auto result = multiVpnManager.SetVpnAddress(ifName, ipAddr, prefix);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetVpnAddressTest002, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string ifName = "12345678901234567890";
    std::string ipAddr = "";
    int32_t prefix = 1;
    auto result = multiVpnManager.SetVpnAddress(ifName, ipAddr, prefix);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetVpnAddressTest003, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string ifName = TEST_XFRM_CARD_NAME;
    std::string ipAddr = "127.0.0.1";
    int32_t prefix = 32;
    auto result = multiVpnManager.SetVpnAddress(ifName, ipAddr, prefix);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    ifName = TEST_PPP_CARD_NAME;
    multiVpnManager.remoteIpv4Addr_ = "";
    result = multiVpnManager.SetVpnAddress(ifName, ipAddr, prefix);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, InitIfreqTest001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    ifreq ifr;
    std::string cardName = "12345678901234567890";
    auto result = multiVpnManager.InitIfreq(ifr, cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetMultiVpnDown001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = TEST_XFRM_CARD_NAME;
    auto result = multiVpnManager.SetVpnDown(cardName);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, SetMultiVpnDown002, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = "12345678901234567890";
    auto result = multiVpnManager.SetVpnDown(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetMultiVpnUp001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = "12345678901234567890";
    auto result = multiVpnManager.SetVpnUp(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetMultiVpnUp002, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = TEST_XFRM_CARD_NAME;
    auto result = multiVpnManager.SetVpnUp(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, CreatePppInterface001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = "12345678901234567890";
    auto result = multiVpnManager.CreatePppInterface(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, CreatePppInterface002, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    multiVpnManager.pppFdMap_[TEST_PPP_CARD_NAME] = 1;
    std::string cardName = TEST_PPP_CARD_NAME;
    auto result = multiVpnManager.CreatePppInterface(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, CreatePppFd001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = "12345678901234567890";
    auto result = multiVpnManager.CreatePppFd(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    result = multiVpnManager.CreatePppFd(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    result = MultiVpnManager::GetInstance().CreatePppFd(TEST_PPP_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, DestroyPppFd001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = "12345678901234567890";
    auto result = multiVpnManager.DestroyPppFd(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, DestroyPppFd002, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = TEST_PPP_CARD_NAME;
    auto result = multiVpnManager.DestroyPppFd(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, DestroyPppFd003, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = TEST_PPP_CARD_NAME;
    multiVpnManager.pppFdMap_[TEST_PPP_CARD_NAME] = 2;
    auto result = multiVpnManager.DestroyPppFd(cardName);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, AddVpnRemoteAddress001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    ifreq ifr = {};
    std::atomic_int net4Sock = 1;
    std::string cardName = TEST_PPP_CARD_NAME;
    auto result = multiVpnManager.AddVpnRemoteAddress(cardName, net4Sock, ifr);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    multiVpnManager.remoteIpv4Addr_= "192.168.1.20";
    result = multiVpnManager.AddVpnRemoteAddress(cardName, net4Sock, ifr);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SendVpnInterfaceFdToClient001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    int32_t clientFd = 0;
    std::atomic_int net4Sock = 1;
    auto result = multiVpnManager.SendVpnInterfaceFdToClient(clientFd, net4Sock);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, DestroyVpnInterface001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    auto result = multiVpnManager.DestroyVpnInterface(TEST_XFRM_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    result = multiVpnManager.DestroyVpnInterface(TEST_PPP_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(MultiVpnManagerTest, DestroyVpnInterface002, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string cardName = "12345678901234567890";
    auto result = multiVpnManager.DestroyVpnInterface(cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(MultiVpnManagerTest, SetVpnCallMode001, TestSize.Level1)
{
    MultiVpnManager multiVpnManager;
    std::string message = "";
    auto result = multiVpnManager.SetVpnCallMode(message);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    message = "1";
    result = multiVpnManager.SetVpnCallMode(message);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    message = "0";
    result = multiVpnManager.SetVpnCallMode(message);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    message = "2";
    result = multiVpnManager.SetVpnCallMode(message);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

} // namespace NetManagerStandard
} // namespace OHOS
