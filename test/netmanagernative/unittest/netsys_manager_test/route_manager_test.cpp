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

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "route_manager.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
using namespace OHOS::NetManagerStandard;
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
    bool flag = false;
    auto ret = RouteManager::AddRoute(static_cast<RouteManager::TableType>(testRouteType), {}, {}, {}, flag);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, AddRouteTest002, TestSize.Level1)
{
    bool flag = false;
    auto ret = RouteManager::AddRoute(RouteManager::TableType::INTERFACE, {}, {}, {}, flag);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddRouteTest003, TestSize.Level1)
{
    bool flag = false;
    auto ret = RouteManager::AddRoute(RouteManager::TableType::LOCAL_NETWORK, {}, {}, {}, flag);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddRouteTest004, TestSize.Level1)
{
    bool flag = false;
    auto ret = RouteManager::AddRoute(RouteManager::TableType::VPN_NETWORK, {}, {}, {}, flag);
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
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromDefaultNetworkTest004, TestSize.Level1)
{
    const std::string testInterfaceName = "wlan0";
    auto ret = RouteManager::RemoveInterfaceFromDefaultNetwork(testInterfaceName, PERMISSION_NONE);
    EXPECT_NE(ret, 0);
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

HWTEST_F(RouteManagerTest, AddInterfaceToPhysicalNetworkTest005, TestSize.Level1)
{
    uint16_t testNetId = 1;
    const std::string testInterfaceName = "rmnet0";
    auto ret = RouteManager::AddInterfaceToPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NETWORK);
    EXPECT_LE(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest003, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "wlan0";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_SYSTEM);
    EXPECT_LE(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest004, TestSize.Level1)
{
    uint16_t testNetId = 154;
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, {}, PERMISSION_NONE);
    EXPECT_LE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest005, TestSize.Level1)
{
    uint16_t testNetId = 1;
    const std::string testInterfaceName = "rmnet0";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, 0);

    ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_LE(ret, -1);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret =
        RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_NONE, PERMISSION_NONE);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_NONE,
                                                             PERMISSION_NETWORK);
    EXPECT_NE(ret, 0);
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
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest005, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret =
        RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_NONE, PERMISSION_SYSTEM);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest006, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret = RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_SYSTEM,
                                                             PERMISSION_SYSTEM);
    EXPECT_NE(ret, 0);
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
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromLocalNetworkTest002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth0";
    auto ret = RouteManager::RemoveInterfaceFromLocalNetwork(testNetId, testInterfaceName);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromLocalNetworkTest003, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "testInterfaceName";
    auto ret = RouteManager::RemoveInterfaceFromLocalNetwork(testNetId, testInterfaceName);
    EXPECT_LE(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromLocalNetworkTest004, TestSize.Level1)
{
    uint16_t testNetId = 154;
    auto ret = RouteManager::RemoveInterfaceFromLocalNetwork(testNetId, {});
    EXPECT_NE(ret, 0);
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
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, DisableSharingTest002, TestSize.Level1)
{
    const std::string input = "eth0";
    const std::string output;
    auto ret = RouteManager::DisableSharing(input, output);
    EXPECT_NE(ret, 0);
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

HWTEST_F(RouteManagerTest, AddInterfaceToVirtualNetwork001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string testInterfaceName = "testName0";
    auto ret = RouteManager::AddInterfaceToVirtualNetwork(testNetId, testInterfaceName);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromVirtualNetwork001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string testInterfaceName = "testName0";
    auto ret = RouteManager::RemoveInterfaceFromVirtualNetwork(testNetId, testInterfaceName);
    EXPECT_NE(ret, 0);

    testInterfaceName = "notexist";
    ret = RouteManager::RemoveInterfaceFromVirtualNetwork(testNetId, testInterfaceName);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, AddUsersToVirtualNetwork001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string testInterfaceName = "testName1";
    std::vector<NetManagerStandard::UidRange> uidRanges;
    auto ret = RouteManager::AddUsersToVirtualNetwork(testNetId, testInterfaceName, uidRanges);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveUsersFromVirtualNetwork001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string testInterfaceName = "testName1";
    std::vector<NetManagerStandard::UidRange> uidRanges;
    auto ret = RouteManager::RemoveUsersFromVirtualNetwork(testNetId, testInterfaceName, uidRanges);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateVnicRoute001, TestSize.Level1)
{
    std::string testInterfaceName = "testName1";
    auto ret = RouteManager::UpdateVnicRoute(testInterfaceName, {}, {}, true);
    EXPECT_EQ(ret, -1);

    ret = RouteManager::UpdateVnicRoute(testInterfaceName, {}, {}, false);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, UpdateVnicUidRangesRule001, TestSize.Level1)
{
    std::vector<NetManagerStandard::UidRange> uidRanges;
    auto ret = RouteManager::UpdateVnicUidRangesRule(uidRanges, true);
    EXPECT_EQ(ret, 0);

    ret = RouteManager::UpdateVnicUidRangesRule(uidRanges, false);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest007, TestSize.Level1)
{
    uint16_t testNetId = 0;
    std::string testInterfaceName = "rmnet0";
    auto ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_NE(ret, 0);
    testNetId = 1;
    ret = RouteManager::RemoveInterfaceFromPhysicalNetwork(testNetId, testInterfaceName, PERMISSION_NONE);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, ModifyPhysicalNetworkPermissionTest007, TestSize.Level1)
{
    uint16_t testNetId = 154;
    const std::string testInterfaceName = "eth1";
    auto ret =
        RouteManager::ModifyPhysicalNetworkPermission(testNetId, testInterfaceName, PERMISSION_NONE, PERMISSION_NONE);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateVirtualNetworkTest002, TestSize.Level1)
{
    NetManagerStandard::UidRange uidRange{};
    std::vector<NetManagerStandard::UidRange> uidRanges;
    uidRanges.push_back(uidRange);
    uint16_t testNetId = 0;
    std::string testInterfaceName = "rmnet0";
    bool add = true;
    auto ret = RouteManager::UpdateVirtualNetwork(testNetId, testInterfaceName, uidRanges, add);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveInterfaceFromPhysicalNetworkTest006, TestSize.Level1)
{
    uint32_t table = 1;
    uid_t uidStart = 1;
    bool add = true;
    int32_t ret = RouteManager::UpdateVpnUidRangeRule(table, uidStart, uidStart, true);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateOutputInterfaceRulesWithUidTest001, TestSize.Level1)
{
    const std::string interface = "interface";
    uint32_t table = 1;
    NetworkPermission permission = PERMISSION_NETWORK;
    uid_t uidStart = 1;
    bool add = true;
    int32_t ret =
        RouteManager::UpdateOutputInterfaceRulesWithUid(interface, table, permission, uidStart, uidStart, add);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToLocalNetworkTest005, TestSize.Level1)
{
    uint16_t testNetId = 1;
    std::string testInterfaceName = "eth0";
    auto ret = RouteManager::AddInterfaceToLocalNetwork(testNetId, testInterfaceName);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, AddClatTunInterfaceTest002, TestSize.Level1)
{
    const std::string interfaceName = "eth0";
    const std::string dstAddr = "127.0.0.1";
    const std::string nxtHop = "nxtHop";
    auto ret = RouteManager::AddClatTunInterface(interfaceName, dstAddr, nxtHop);
    EXPECT_EQ(ret, -1);

    ret = RouteManager::AddClatTunInterface(interfaceName, dstAddr, nxtHop);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, RemoveClatTunInterfaceTest002, TestSize.Level1)
{
    const std::string interfaceName = "eth0";
    std::map<std::string, uint32_t> interfaceToTable;
    interfaceToTable[interfaceName] = RT_TABLE_UNSPEC;
    RouteManager::interfaceToTable_ = interfaceToTable;
    auto ret = RouteManager::RemoveClatTunInterface(interfaceName);
    EXPECT_EQ(ret, -1);

    interfaceToTable[interfaceName] = 1;
    RouteManager::interfaceToTable_ = interfaceToTable;
    ret = RouteManager::RemoveClatTunInterface(interfaceName);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(RouteManagerTest, UpdateClatTunInterfaceTest001, TestSize.Level1)
{
    const std::string interfaceName = "eth0";
    std::map<std::string, uint32_t> interfaceToTable;
    interfaceToTable[interfaceName] = RT_TABLE_UNSPEC;
    RouteManager::interfaceToTable_ = interfaceToTable;
    auto ret = RouteManager::UpdateClatTunInterface(interfaceName, PERMISSION_NONE, true);
    EXPECT_EQ(ret, -1);

    interfaceToTable[interfaceName] = 1;
    RouteManager::interfaceToTable_ = interfaceToTable;
    ret = RouteManager::UpdateClatTunInterface(interfaceName, PERMISSION_NONE, true);
    EXPECT_EQ(ret, 0);
    ret = RouteManager::UpdateClatTunInterface(interfaceName, PERMISSION_NONE, false);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, ClearRoutesTest001, TestSize.Level1)
{
    int32_t netId = 0;
    const std::string interfaceName = "eth0";
    std::map<std::string, uint32_t> interfaceToTable;
    interfaceToTable[interfaceName] = RT_TABLE_UNSPEC;
    RouteManager::interfaceToTable_ = interfaceToTable;
    auto ret = RouteManager::ClearRoutes(interfaceName, netId);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, UpdatePhysicalNetworkTest001, TestSize.Level1)
{
    int32_t netId = 0;
    const std::string interfaceName = "eth0";
    int32_t ret = RouteManager::UpdatePhysicalNetwork(netId, interfaceName, PERMISSION_NONE, true);
    EXPECT_LE(ret, 0);
    ret = RouteManager::UpdatePhysicalNetwork(netId, interfaceName, PERMISSION_NONE, false);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, UpdateIncomingPacketMarkTest001, TestSize.Level1)
{
    int32_t netId = 0;
    const std::string interfaceName = "eth0";

    int32_t ret = RouteManager::UpdateIncomingPacketMark(netId, interfaceName, PERMISSION_NONE, true);
    EXPECT_EQ(ret, 0);
    ret = RouteManager::UpdateIncomingPacketMark(netId, interfaceName, PERMISSION_NONE, false);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateSharingNetworkTest001, TestSize.Level1)
{
    uint16_t netId = 1;
    const std::string inputInterface = "eth0";
    const std::string outputInterface = "eth0";
    int32_t ret = RouteManager::UpdateSharingNetwork(netId, inputInterface, outputInterface);
    EXPECT_EQ(ret, -1);

    std::map<std::string, uint32_t> interfaceToTable;
    interfaceToTable[inputInterface] = 1;
    RouteManager::interfaceToTable_ = interfaceToTable;
    ret = RouteManager::UpdateSharingNetwork(netId, inputInterface, outputInterface);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, UpdateRuleInfoTest001, TestSize.Level1)
{
    uint32_t action = 1;
    uint8_t ruleType = 1;
    RuleInfo ruleInfo{0, 1, 1, 0, "ruleIif", "ruleOif", "ruleSrcIp", "ruleDstIp"};
    uid_t uidStart = 0;
    uid_t uidEnd = 1;

    int32_t ret = RouteManager::UpdateRuleInfo(action, ruleType, ruleInfo, uidStart, uidEnd);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    ruleInfo.ruleMask = 1;
    ret = RouteManager::UpdateRuleInfo(action, ruleType, ruleInfo, uidStart, uidEnd);
    EXPECT_EQ(ret, -ENOTUNIQ); // -76
    uint8_t family = 1;
    ret = RouteManager::SendRuleToKernelEx(action, family, ruleType, ruleInfo, uidStart, uidEnd);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(RouteManagerTest, UpdateDistributedRuleTest001, TestSize.Level1)
{
    uint32_t action = 1;
    uint8_t ruleType = 1;
    RuleInfo ruleInfo{0, 1, 1, 0, "ruleIif", "ruleOif", "ruleSrcIp", "ruleDstIp:"};
    uid_t uidStart = 0;
    uid_t uidEnd = 1;

    int32_t ret = RouteManager::UpdateDistributedRule(action, ruleType, ruleInfo, uidStart, uidEnd);
    EXPECT_EQ(ret, -ENOTUNIQ); // -76
    ruleInfo.ruleTable = 1;
    ret = RouteManager::UpdateDistributedRule(action, ruleType, ruleInfo, uidStart, uidEnd);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(RouteManagerTest, SendRouteToKernelTest005, TestSize.Level1)
{
    uint16_t action = 1;
    uint16_t routeFlag = 1;
    rtmsg msg{};
    RouteInfo routeInfo{1, "", "http/a:6:0:df", ""};
    uint32_t index = 0;

    int32_t ret = RouteManager::SendRouteToKernel(action, routeFlag, msg, routeInfo, index);
    EXPECT_EQ(ret, -1);
    routeInfo.routeNextHop = "http/a:6:0:df";
    ret = RouteManager::SendRouteToKernel(action, routeFlag, msg, routeInfo, index);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, GetRouteTableFromTypeTest005, TestSize.Level1)
{
    RouteManager::TableType tableType = RouteManager::INTERNAL_DEFAULT;
    std::string interfaceName = "eth0";
    uint32_t ret = RouteManager::GetRouteTableFromType(tableType, interfaceName);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, AddInterfaceToVirtualNetwork002, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string testInterfaceName = "vpn";
    auto ret = RouteManager::AddInterfaceToVirtualNetwork(testNetId, testInterfaceName);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(RouteManagerTest, EnableDistributedClientNet001, TestSize.Level1)
{
    std::string virNicAddr;
    std::string iif;
    auto ret = RouteManager::EnableDistributedClientNet(virNicAddr, iif);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(RouteManagerTest, AddClatTunInterface001, TestSize.Level1)
{
    std::string interfaceName;
    std::string dstAddr;
    std::string nxtHop;
    auto ret = RouteManager::AddClatTunInterface(interfaceName, dstAddr, nxtHop);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, RemoveClatTunInterface001, TestSize.Level1)
{
    std::string interfaceName;
    auto ret = RouteManager::RemoveClatTunInterface(interfaceName);
    EXPECT_NE(ret, 0);
}
} // namespace nmd
} // namespace OHOS