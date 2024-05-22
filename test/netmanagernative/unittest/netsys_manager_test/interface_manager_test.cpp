/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <gtest/gtest.h>
#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "interface_manager.h"
#include "netsys_controller.h"
#include "net_manager_constants.h"
namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
using namespace OHOS::NetManagerStandard;
} // namespace

class InterfaceManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void InterfaceManagerTest::SetUpTestCase() {}

void InterfaceManagerTest::TearDownTestCase() {}

void InterfaceManagerTest::SetUp() {}

void InterfaceManagerTest::TearDown() {}

HWTEST_F(InterfaceManagerTest, GetMtuTest001, TestSize.Level1)
{
    auto ret = InterfaceManager::GetMtu(nullptr);
    EXPECT_EQ(ret, -1);

    std::string interfaceName = "IfaceNameIsExtMax16";
    ret = InterfaceManager::GetMtu(interfaceName.data());
    EXPECT_EQ(ret, -1);
}


HWTEST_F(InterfaceManagerTest, SetMtuTest001, TestSize.Level1)
{
    std::string mtuValue = "10";
    std::string interfaceName;
    auto ret = InterfaceManager::SetMtu(interfaceName.data(), mtuValue.data());
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InterfaceManagerTest, SetMtuTest002, TestSize.Level1)
{
    std::string mtuValue = "10";
    std::string interfaceName = "eth0";
    auto ret = InterfaceManager::SetMtu(interfaceName.data(), mtuValue.data());
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InterfaceManagerTest, SetMtuTest003, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    auto ifaceList = NetManagerStandard::NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), interfaceName) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }

    char *mtuValue = nullptr;
    auto ret = InterfaceManager::SetMtu(interfaceName.data(), mtuValue);
    EXPECT_EQ(ret, -1);

    const char *cmtu = "";
    ret = InterfaceManager::SetMtu(interfaceName.data(), cmtu);
    EXPECT_EQ(ret, -1);

    std::string mtu = "1500";
    ret = InterfaceManager::SetMtu(interfaceName.data(), mtu.data());
    EXPECT_EQ(ret, 0);

    mtu = "1500000000000000";
    ret = InterfaceManager::SetMtu(interfaceName.data(), mtu.data());
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InterfaceManagerTest, AddAddressTest001, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    std::string addr = "";
    int32_t prefixLength = 0;
    auto ifaceList = NetManagerStandard::NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), interfaceName) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }
    auto ret = InterfaceManager::AddAddress(interfaceName.data(), addr.data(), prefixLength);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceManagerTest, AddAddressTest002, TestSize.Level1)
{
    std::string addr = "14.4.1.4";
    int32_t prefixLength = 45;
    auto ret = InterfaceManager::AddAddress(nullptr, addr.data(), prefixLength);
    EXPECT_LE(ret, 0);
}

HWTEST_F(InterfaceManagerTest, AddAddressTest003, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    std::string addr;
    int32_t prefixLength = 45;
    auto ifaceList = NetManagerStandard::NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), interfaceName) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }
    auto ret = InterfaceManager::AddAddress(interfaceName.c_str(), addr.data(), prefixLength);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceManagerTest, AddAddressTest004, TestSize.Level1)
{
    std::string interfaceName = "eth";
    std::string addr;
    int32_t prefixLength = 45;
    auto ret = InterfaceManager::AddAddress(interfaceName.c_str(), addr.data(), prefixLength);
    EXPECT_EQ(ret, -errno);
}

HWTEST_F(InterfaceManagerTest, AddAddressTest005, TestSize.Level1)
{
    std::string interfaceName = "eth";
    int32_t prefixLength = 45;
    auto ret = InterfaceManager::AddAddress(interfaceName.c_str(), nullptr, prefixLength);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InterfaceManagerTest, DelAddressTest001, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    std::string addr = "";
    int32_t prefixLength = 0;
    auto ifaceList = NetManagerStandard::NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), interfaceName) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }
    auto ret = InterfaceManager::DelAddress(interfaceName.data(), addr.data(), prefixLength);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceManagerTest, DelAddressTest002, TestSize.Level1)
{
    std::string addr = "14.4.1.4";
    int32_t prefixLength = 45;
    auto ret = InterfaceManager::DelAddress(nullptr, addr.data(), prefixLength);
    EXPECT_LE(ret, 0);
}

HWTEST_F(InterfaceManagerTest, DelAddressTest003, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    int32_t prefixLength = 45;
    std::string addr;
    auto ifaceList = NetManagerStandard::NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), interfaceName) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }
    auto ret = InterfaceManager::DelAddress(interfaceName.data(), addr.data(), prefixLength);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(InterfaceManagerTest, GetInterfaceNamesTest001, TestSize.Level1)
{
    auto ret = InterfaceManager::GetInterfaceNames();
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(InterfaceManagerTest, GetIfaceConfigTest001, TestSize.Level1)
{
    std::string ifaceName = "";
    auto ret = InterfaceManager::GetIfaceConfig(ifaceName);
    EXPECT_TRUE(ret.ifName.empty());
}

HWTEST_F(InterfaceManagerTest, GetIfaceConfigTest002, TestSize.Level1)
{
    std::string ifaceName = "eth0";
    auto ret = InterfaceManager::GetIfaceConfig(ifaceName);
    EXPECT_FALSE(ret.ifName.empty());
}

HWTEST_F(InterfaceManagerTest, SetIfaceConfigTest001, TestSize.Level1)
{
    nmd::InterfaceConfigurationParcel ifaceConfig;
    ifaceConfig.ifName = "test0";
    int32_t ret = InterfaceManager::SetIfaceConfig(ifaceConfig);
    EXPECT_LE(ret, 0);

    ifaceConfig.flags.push_back("up");
    ret = InterfaceManager::SetIfaceConfig(ifaceConfig);
    EXPECT_LE(ret, 0);

    std::string ifaceName = "eth0";
    auto config = InterfaceManager::GetIfaceConfig(ifaceName);
    EXPECT_FALSE(config.ifName.empty());
    ret = InterfaceManager::SetIfaceConfig(config);
    EXPECT_LE(ret, 1);
}

HWTEST_F(InterfaceManagerTest, SetIpAddressTest002, TestSize.Level1)
{
    std::string errName = "test0";
    std::string ipAddr = "172.17.5.245";
    auto ret = InterfaceManager::SetIpAddress(errName, ipAddr);
    EXPECT_LE(ret, 0);
}

HWTEST_F(InterfaceManagerTest, SetIffUpTest002, TestSize.Level1)
{
    std::string errName = "test0";
    auto ret = InterfaceManager::SetIffUp(errName);
    EXPECT_LE(ret, 0);

    std::string ifaceName = "eth1";
    ret = InterfaceManager::SetIffUp(ifaceName);
    EXPECT_LE(ret, 0);
}

HWTEST_F(InterfaceManagerTest, AssembleArp001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    arpreq req = {};
    auto ret = InterfaceManager::AssembleArp(ipAddr, macAddr, ifName, req);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(InterfaceManagerTest, AddStaticArpTest001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    auto ret = InterfaceManager::AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, 0);

    ipAddr = "192.168.1.101";
    macAddr = "aa:bb:cc:dd:00:ff";
    ifName = "wlan0";
    ret = InterfaceManager::AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(InterfaceManagerTest, DelStaticArpTest001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    auto ret = InterfaceManager::DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, 0);

    ipAddr = "192.168.1.101";
    macAddr = "aa:bb:cc:dd:00:ff";
    ifName = "wlan0";
    ret = InterfaceManager::DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, 0);
}
} // namespace nmd
} // namespace OHOS
