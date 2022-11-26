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

#include "route_manager.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
} // namespace

class RouteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RouteManagerTest::SetUpTestCase() {}

void RouteManagerTest::TearDownTestCase() {}

void RouteManagerTest::SetUp() {}

void RouteManagerTest::TearDown() {}

HWTEST_F(RouteManagerTest, AddRouteTest001, TestSize.Level1)
{
    uint32_t testRouteType = 6;
    auto ret = RouteManager::AddRoute(static_cast<RouteManager::TableType>(testRouteType), {}, {}, {});
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, AddRouteTest002, TestSize.Level1)
{
    auto ret = RouteManager::AddRoute(RouteManager::TableType::INTERFACE, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddRouteTest003, TestSize.Level1)
{
    auto ret = RouteManager::AddRoute(RouteManager::TableType::LOCAL_NETWORK, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddRouteTest004, TestSize.Level1)
{
    auto ret = RouteManager::AddRoute(RouteManager::TableType::VPN_NETWORK, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveRouteTest001, TestSize.Level1)
{
    uint32_t testRouteType = 6;
    auto ret = RouteManager::RemoveRoute(static_cast<RouteManager::TableType>(testRouteType), {}, {}, {});
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, RemoveRouteTest002, TestSize.Level1)
{
    auto ret = RouteManager::RemoveRoute(RouteManager::TableType::INTERFACE, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveRouteTest003, TestSize.Level1)
{
    auto ret = RouteManager::RemoveRoute(RouteManager::TableType::LOCAL_NETWORK, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveRouteTest004, TestSize.Level1)
{
    auto ret = RouteManager::RemoveRoute(RouteManager::TableType::VPN_NETWORK, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateRouteTest001, TestSize.Level1)
{
    uint32_t testRouteType = 6;
    auto ret = RouteManager::UpdateRoute(static_cast<RouteManager::TableType>(testRouteType), {}, {}, {});
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, UpdateRouteTest002, TestSize.Level1)
{
    auto ret = RouteManager::UpdateRoute(RouteManager::TableType::INTERFACE, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateRouteTest003, TestSize.Level1)
{
    auto ret = RouteManager::UpdateRoute(RouteManager::TableType::LOCAL_NETWORK, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateRouteTest004, TestSize.Level1)
{
    auto ret = RouteManager::UpdateRoute(RouteManager::TableType::VPN_NETWORK, {}, {}, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToDefaultNetworkTest001, TestSize.Level1)
{
    auto ret = RouteManager::AddInterfaceToDefaultNetwork({}, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToDefaultNetworkTest002, TestSize.Level1)
{
    const std::string testInterfaceName = "testInterface";
    auto ret = RouteManager::AddInterfaceToDefaultNetwork(testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToDefaultNetworkTest003, TestSize.Level1)
{
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::AddInterfaceToDefaultNetwork(testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToDefaultNetworkTest004, TestSize.Level1)
{
    const std::string testInterfaceName = "wlan0";
    auto ret = RouteManager::AddInterfaceToDefaultNetwork(testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromDefaultNetworkTest001, TestSize.Level1)
{
    auto ret = RouteManager::RemoveInterfaceFromDefaultNetwork({}, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromDefaultNetworkTest002, TestSize.Level1)
{
    const std::string testInterfaceName = "testInterface";
    auto ret = RouteManager::RemoveInterfaceFromDefaultNetwork(testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromDefaultNetworkTest003, TestSize.Level1)
{
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::RemoveInterfaceFromDefaultNetwork(testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromDefaultNetworkTest004, TestSize.Level1)
{
    const std::string testInterfaceName = "wlan0";
    auto ret = RouteManager::RemoveInterfaceFromDefaultNetwork(testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToPhysicalNetworkTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::AddInterfaceToPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToPhysicalNetworkTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::AddInterfaceToPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NETWORK);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToPhysicalNetworkTest003, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "wlan0";
    auto ret = RouteManager::AddInterfaceToPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_SYSTEM);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToPhysicalNetworkTest004, TestSize.Level1)
{
    uint16_t testNetId = 154;
    auto ret = RouteManager::AddInterfaceToPhysicalNetwork(testNetId, {}, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NETWORK);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest003, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "wlan0";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_SYSTEM);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest004, TestSize.Level1)
{
    uint16_t testNetId = 154;
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, {}, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret =
        RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_NONE, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_NONE,
                                                             PERMISSION_NETWORK);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest003, TestSize.Level1)
{
    uint16_t testNetId = 154;
    auto ret = RouteManager::ModifyPhysicalNetworkPermission(testNetId, {}, PERMISSION_NETWORK, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest004, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret =
        RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_SYSTEM, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest005, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret =
        RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_NONE, PERMISSION_SYSTEM);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest006, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_SYSTEM,
                                                             PERMISSION_SYSTEM);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToLocalNetworkTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::AddInterfaceToLocalNetwork(testNetId, testInterfaceName);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToLocalNetworkTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::AddInterfaceToLocalNetwork(testNetId, testInterfaceName);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToLocalNetworkTest003, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "testInterfaceName";
    auto ret = RouteManager::AddInterfaceToLocalNetwork(testNetId, testInterfaceName);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToLocalNetworkTest004, TestSize.Level1)
{
    uint16_t testNetId = 154;
    auto ret = RouteManager::AddInterfaceToLocalNetwork(testNetId, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromLocalNetworkTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::RemoveInterfaceFromLocalNetwork(testNetId, testInterfaceName);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromLocalNetworkTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::RemoveInterfaceFromLocalNetwork(testNetId, testInterfaceName);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromLocalNetworkTest003, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "testInterfaceName";
    auto ret = RouteManager::RemoveInterfaceFromLocalNetwork(testNetId, testInterfaceName);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromLocalNetworkTest004, TestSize.Level1)
{
    uint16_t testNetId = 154;
    auto ret = RouteManager::RemoveInterfaceFromLocalNetwork(testNetId, {});
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, EnableSharingTest001, TestSize.Level1)
{
    const std::string input;
    const std::string output;
    auto ret = RouteManager::EnableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, EnableSharingTest002, TestSize.Level1)
{
    const std::string input = "eth0";
    const std::string output;
    auto ret = RouteManager::EnableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, EnableSharingTest003, TestSize.Level1)
{
    const std::string input;
    const std::string output = "sla0";
    auto ret = RouteManager::EnableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, EnableSharingTest004, TestSize.Level1)
{
    const std::string input = "test";
    const std::string output = "dds0";
    auto ret = RouteManager::EnableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, EnableSharingTest005, TestSize.Level1)
{
    const std::string input = "wlan0";
    const std::string output = "eth3";
    auto ret = RouteManager::EnableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, DisableSharingTest001, TestSize.Level1)
{
    const std::string input;
    const std::string output;
    auto ret = RouteManager::DisableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, DisableSharingTest002, TestSize.Level1)
{
    const std::string input = "eth0";
    const std::string output;
    auto ret = RouteManager::DisableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, DisableSharingTest003, TestSize.Level1)
{
    const std::string input;
    const std::string output = "sla0";
    auto ret = RouteManager::DisableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, DisableSharingTest004, TestSize.Level1)
{
    const std::string input = "test";
    const std::string output = "dds0";
    auto ret = RouteManager::DisableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, DisableSharingTest005, TestSize.Level1)
{
    const std::string input = "wlan0";
    const std::string output = "eth3";
    auto ret = RouteManager::DisableSharing(input, output);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ReadAddrTest001, TestSize.Level1)
{
    const std::string addr;
    auto ret = RouteManager::ReadAddr(addr, nullptr);
    EXPECT_EQ(ret, -EINVAL);
}

HWTEST_F(RouteManagerTest, ReadAddrTest002, TestSize.Level1)
{
    const std::string addr = "/";
    InetAddr res;
    auto ret = RouteManager::ReadAddr(addr, &res);
    EXPECT_EQ(ret, -EINVAL);
}

HWTEST_F(RouteManagerTest, ReadAddrTest003, TestSize.Level1)
{
    const std::string addr = "48541/451564";
    InetAddr res;
    auto ret = RouteManager::ReadAddr(addr, &res);
    EXPECT_EQ(ret, -EINVAL);
}

HWTEST_F(RouteManagerTest, ReadAddrTest004, TestSize.Level1)
{
    const std::string addr = "48541adfa/451564dfa";
    InetAddr res;
    auto ret = RouteManager::ReadAddr(addr, &res);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ReadAddrTest005, TestSize.Level1)
{
    const std::string addr = "gsga:4557/56445:::df?";
    InetAddr res;
    auto ret = RouteManager::ReadAddr(addr, &res);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ReadAddrGwTest001, TestSize.Level1)
{
    const std::string addr;
    auto ret = RouteManager::ReadAddrGw(addr, nullptr);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ReadAddrGwTest002, TestSize.Level1)
{
    const std::string addr = "/";
    InetAddr res;
    auto ret = RouteManager::ReadAddrGw(addr, &res);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ReadAddrGwTest003, TestSize.Level1)
{
    const std::string addr = "48541/451564";
    InetAddr res;
    auto ret = RouteManager::ReadAddrGw(addr, &res);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ReadAddrGwTest004, TestSize.Level1)
{
    const std::string addr = "48541adfa/451564dfa";
    InetAddr res;
    auto ret = RouteManager::ReadAddrGw(addr, &res);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, ReadAddrGwTest005, TestSize.Level1)
{
    const std::string addr = "gsga:4557/56445:::df?";
    InetAddr res;
    auto ret = RouteManager::ReadAddrGw(addr, &res);
    EXPECT_LE(ret, 0);
}
} // namespace nmd
} // namespace OHOS