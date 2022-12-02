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

#include "interface_manager.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
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
    std::string interfaceName;
    auto ret = InterfaceManager::GetMtu(interfaceName.data());
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InterfaceManagerTest, GetMtuTest002, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    auto ret = InterfaceManager::GetMtu(interfaceName.c_str());
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
    std::string mtuValue;
    auto ret = InterfaceManager::SetMtu(interfaceName.data(), mtuValue.data());
    EXPECT_LE(ret, 0);
}

HWTEST_F(InterfaceManagerTest, AddAddressTest001, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    std::string addr = "";
    int32_t prefixLength = 0;
    auto ret = InterfaceManager::AddAddress(interfaceName.data(), addr.data(), prefixLength);
    EXPECT_LE(ret, 0);
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
    auto ret = InterfaceManager::AddAddress(interfaceName.c_str(), addr.data(), prefixLength);
    EXPECT_LE(ret, 0);
}

HWTEST_F(InterfaceManagerTest, DelAddressTest001, TestSize.Level1)
{
    std::string interfaceName = "eth0";
    std::string addr = "";
    int32_t prefixLength = 0;
    auto ret = InterfaceManager::DelAddress(interfaceName.data(), addr.data(), prefixLength);
    EXPECT_LE(ret, 0);
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
    auto ret = InterfaceManager::DelAddress(interfaceName.data(), addr.data(), prefixLength);
    EXPECT_LE(ret, 0);
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
    std::string ifaceName = "eth0";
    auto config = InterfaceManager::GetIfaceConfig(ifaceName);
    EXPECT_FALSE(config.ifName.empty());
    auto ret = InterfaceManager::SetIfaceConfig(config);
    EXPECT_LE(ret, 1);
}
} // namespace nmd
} // namespace OHOS