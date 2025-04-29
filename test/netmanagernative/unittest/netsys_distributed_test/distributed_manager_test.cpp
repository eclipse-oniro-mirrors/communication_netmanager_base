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
#include "distributed_manager.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *DISTRIBUTED_TUN_CARD_NAME = "virnic";
} // namespace

class DistributedManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DistributedManagerTest::SetUpTestCase() {}

void DistributedManagerTest::TearDownTestCase() {}

void DistributedManagerTest::SetUp() {}

void DistributedManagerTest::TearDown() {}

HWTEST_F(DistributedManagerTest, CreateDistributedInterface001, TestSize.Level1)
{
    auto result = DistributedManager::GetInstance().CreateDistributedInterface(DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    DistributedManager::GetInstance().DestroyDistributedNic(DISTRIBUTED_TUN_CARD_NAME);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicMtu001, TestSize.Level1)
{
    std::string ifName = "";
    int32_t testNumber = 0;
    auto result = DistributedManager::GetInstance().SetDistributedNicMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    ifName = DISTRIBUTED_TUN_CARD_NAME;
    testNumber = 1;
    result = DistributedManager::GetInstance().SetDistributedNicMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicMtu002, TestSize.Level1)
{
    DistributedManager::GetInstance().CreateDistributedInterface(DISTRIBUTED_TUN_CARD_NAME);
    std::string ifName = DISTRIBUTED_TUN_CARD_NAME;
    int32_t testNumber = 1; // func ioctl will be failed, if mtu is too small
    auto result = DistributedManager::GetInstance().SetDistributedNicMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    ifName = DISTRIBUTED_TUN_CARD_NAME;
    testNumber = 1400;
    result = DistributedManager::GetInstance().SetDistributedNicMtu(ifName, testNumber);
    DistributedManager::GetInstance().DestroyDistributedNic(DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicAddress001, TestSize.Level1)
{
    std::string ifName = "";
    std::string tunAddr = "";
    auto result = DistributedManager::GetInstance().SetDistributedNicAddress(ifName, tunAddr);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    tunAddr = "1.23.45.6";
    DistributedManager::GetInstance().CreateDistributedInterface(DISTRIBUTED_TUN_CARD_NAME);
    result = DistributedManager::GetInstance().SetDistributedNicAddress(DISTRIBUTED_TUN_CARD_NAME, tunAddr);
    DistributedManager::GetInstance().DestroyDistributedNic(DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicUp001, TestSize.Level1)
{
    auto result = DistributedManager::GetInstance().SetDistributedNicUp(DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    result = DistributedManager::GetInstance().SetDistributedNicDown(DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(DistributedManagerTest, InitIfreq001, TestSize.Level1)
{
    ifreq ifr;
    std::string cardName = "";
    auto result = DistributedManager::GetInstance().InitIfreq(ifr, cardName);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicResult001, TestSize.Level1)
{
    std::atomic_int fd = -1;
    unsigned long cmd = 0;
    ifreq ifr;
    auto result = DistributedManager::GetInstance().SetDistributedNicResult(fd, cmd, ifr);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, InitIfreq002, TestSize.Level1)
{
    ifreq ifr;
    std::string cardName = "12345678901234567890";
    auto result = DistributedManager::GetInstance().InitIfreq(ifr, cardName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicMtu003, TestSize.Level1)
{
    std::string ifName = "12345678901234567890";
    int32_t mtu = 1;
    auto result = DistributedManager::GetInstance().SetDistributedNicMtu(ifName, mtu);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicAddress002, TestSize.Level1)
{
    std::string ifName = "12345678901234567890";
    std::string tunAddr = "123";
    auto result = DistributedManager::GetInstance().SetDistributedNicAddress(ifName, tunAddr);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    ifName = "";
    result = DistributedManager::GetInstance().SetDistributedNicAddress(ifName, tunAddr);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicUp002, TestSize.Level1)
{
    std::string ifName = "12345678901234567890";
    auto result = DistributedManager::GetInstance().SetDistributedNicUp(ifName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, SetDistributedNicDown001, TestSize.Level1)
{
    std::string ifName = "12345678901234567890";
    auto result = DistributedManager::GetInstance().SetDistributedNicDown(ifName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, CreateDistributedNic001, TestSize.Level1)
{
    std::string virNicAddr = "";
    std::string ifName = "12345678901234567890";
    auto result = DistributedManager::GetInstance().CreateDistributedNic(virNicAddr, DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    virNicAddr = "1.23.45.6";
    result = DistributedManager::GetInstance().CreateDistributedNic(virNicAddr, DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    result = DistributedManager::GetInstance().CreateDistributedNic(virNicAddr, ifName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(DistributedManagerTest, CloseDistributedTunFd001, TestSize.Level1)
{
    DistributedManager::GetInstance().CloseDistributedTunFd();
    auto result = DistributedManager::GetInstance().CreateDistributedInterface(DISTRIBUTED_TUN_CARD_NAME);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    DistributedManager::GetInstance().CloseDistributedTunFd();
}

HWTEST_F(DistributedManagerTest, ConfigVirnicAndVeth001, TestSize.Level1)
{
    std::string virNicAddr = "";
    std::string virnicName = "";
    std::string virnicVethName = "1";
    auto result = DistributedManager::GetInstance().ConfigVirnicAndVeth(virNicAddr, virnicName, virnicVethName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    virnicName = "1";
    virnicVethName = "";
    result = DistributedManager::GetInstance().ConfigVirnicAndVeth(virNicAddr, virnicName, virnicVethName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    virnicName = "";
    result = DistributedManager::GetInstance().ConfigVirnicAndVeth(virNicAddr, virnicName, virnicVethName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    virnicName = "1";
    virnicVethName = "1";
    result = DistributedManager::GetInstance().ConfigVirnicAndVeth(virNicAddr, virnicName, virnicVethName);
    EXPECT_EQ(result, NETMANAGER_ERROR);
    virNicAddr = "192.168.1.1";
    virnicName = "virnic0";
    virnicVethName = "virnic1";
    result = DistributedManager::GetInstance().ConfigVirnicAndVeth(virNicAddr, virnicName, virnicVethName);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
