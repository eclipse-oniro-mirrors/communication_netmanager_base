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

#include "iservice_registry.h"
#include "system_ability_definition.h"

#ifdef GTEST_API_
#define private public
#endif

#include "conn_manager.h"
#include "net_conn_manager_test_util.h"
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_proxy.h"
#include "network_permission.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace NetManagerStandard;
using namespace NetConnManagerTestUtil;
constexpr int32_t NETID = 103;
const std::string INTERFACENAME = "wlan0";
constexpr int32_t INTERNAL_NETID = 10;
const std::string INTERNAL_INTERFACENAME = "rmnet0";
constexpr int32_t LOCAL_NET_ID = 99;
constexpr int32_t ERROR_CODE = -101;
constexpr int32_t INVALID_VALUE = -1;

class ConnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<ConnManager> instance_ = nullptr;
};

void ConnManagerTest::SetUpTestCase()
{
    instance_ = std::make_shared<ConnManager>();
}

void ConnManagerTest::TearDownTestCase()
{
    instance_ = nullptr;
}

void ConnManagerTest::SetUp() {}

void ConnManagerTest::TearDown() {}

/**
 * @tc.name: SetInternetPermission001
 * @tc.desc: Test ConnManager SetInternetPermission.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, SetInternetPermission001, TestSize.Level1)
{
    uint32_t uid = 0;
    uint8_t allow = 0;
    uint8_t isBroker = 0;
    int32_t ret = instance_->SetInternetPermission(uid, allow, isBroker);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: SetInternetPermission002
 * @tc.desc: Test ConnManager SetInternetPermission.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, SetInternetPermission002, TestSize.Level1)
{
    uint32_t uid = 0;
    uint8_t allow = 0;
    uint8_t isBroker = 0;
    int32_t ret = instance_->SetInternetPermission(uid, allow, isBroker);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: CreatePhysicalNetworkTest001
 * @tc.desc: Test ConnManager CreatePhysicalNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, CreatePhysicalNetworkTest001, TestSize.Level1)
{
    int32_t ret = instance_->CreatePhysicalNetwork(NETID, PERMISSION_NONE);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: CreatePhysicalNetworkTest002
 * @tc.desc: Test ConnManager CreatePhysicalNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, CreatePhysicalNetworkTest002, TestSize.Level1)
{
    int32_t ret = instance_->ReinitRoute();
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = instance_->CreatePhysicalNetwork(NETID, PERMISSION_NONE);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: CreatePhysicalNetworkTest003
 * @tc.desc: Test ConnManager CreatePhysicalNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, CreatePhysicalNetworkTest003, TestSize.Level1)
{
    auto ret = instance_->CreatePhysicalNetwork(INTERNAL_NETID, PERMISSION_NONE);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: CreateVirtualNetwork001
 * @tc.desc: Test ConnManager CreateVirtualNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, CreateVirtualNetwork001, TestSize.Level1)
{
    uint16_t netId = 1;
    bool hasDns = true;
    int32_t ret = instance_->CreateVirtualNetwork(netId, hasDns);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: DestroyNetworkTest001
 * @tc.desc: Test ConnManager DestroyNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, DestroyNetworkTest001, TestSize.Level1)
{
    auto ret = instance_->DestroyNetwork(LOCAL_NET_ID);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: DestroyNetworkTest002
 * @tc.desc: Test ConnManager DestroyNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, DestroyNetworkTest002, TestSize.Level1)
{
    int32_t ret = instance_->DestroyNetwork(NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = instance_->DestroyNetwork(NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: DestroyNetworkTest003
 * @tc.desc: Test ConnManager DestroyNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, DestroyNetworkTest003, TestSize.Level1)
{
    int32_t netId = 100;
    int32_t ret = instance_->CreatePhysicalNetwork(netId, PERMISSION_NONE);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->DestroyNetwork(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetDefaultNetworkTest001
 * @tc.desc: Test ConnManager SetDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, SetDefaultNetworkTest001, TestSize.Level1)
{
    int32_t ret = instance_->SetDefaultNetwork(NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = instance_->SetDefaultNetwork(NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetDefaultNetworkTest002
 * @tc.desc: Test ConnManager SetDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, SetDefaultNetworkTest002, TestSize.Level1)
{
    int32_t ret = instance_->SetDefaultNetwork(0);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = instance_->SetDefaultNetwork(0);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = instance_->SetDefaultNetwork(NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: ClearDefaultNetwork001
 * @tc.desc: Test ConnManager ClearDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, ClearDefaultNetwork001, TestSize.Level1)
{
    int32_t ret = instance_->ClearDefaultNetwork();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = instance_->ClearDefaultNetwork();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetDefaultNetworkTest001
 * @tc.desc: Test ConnManager GetDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, GetDefaultNetworkTest001, TestSize.Level1)
{
    int32_t ret = instance_->SetDefaultNetwork(NETID);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ret = instance_->GetDefaultNetwork();
    EXPECT_EQ(ret, NETID);
}

/**
 * @tc.name: AddInterfaceToNetworkTest001
 * @tc.desc: Test ConnManager AddInterfaceToNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, AddInterfaceToNetworkTest001, TestSize.Level1)
{
    std::string iface = INTERFACENAME;
    int32_t ret = instance_->AddInterfaceToNetwork(NETID, iface);
    EXPECT_NE(ret, 0);

    iface = INTERNAL_INTERFACENAME;
    ret = instance_->AddInterfaceToNetwork(INTERNAL_NETID, iface);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: AddInterfaceToNetworkTest002
 * @tc.desc: Test ConnManager AddInterfaceToNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, AddInterfaceToNetworkTest002, TestSize.Level1)
{
    std::string testInterfaceName = "testName";
    int32_t ret = instance_->AddInterfaceToNetwork(NETID, testInterfaceName);
    EXPECT_NE(ret, 0);

    ret = instance_->AddInterfaceToNetwork(INTERNAL_NETID, testInterfaceName);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: RemoveInterfaceFromNetworkTest001
 * @tc.desc: Test ConnManager RemoveInterfaceFromNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, RemoveInterfaceFromNetworkTest001, TestSize.Level1)
{
    std::string iface = INTERFACENAME;
    int32_t ret = instance_->RemoveInterfaceFromNetwork(NETID, iface);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);

    iface = INTERNAL_INTERFACENAME;
    ret = instance_->RemoveInterfaceFromNetwork(INTERNAL_NETID, iface);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RemoveInterfaceFromNetworkTest002
 * @tc.desc: Test ConnManager RemoveInterfaceFromNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, RemoveInterfaceFromNetworkTest002, TestSize.Level1)
{
    std::string testInterfaceName = "testName";
    auto ret = instance_->RemoveInterfaceFromNetwork(NETID, testInterfaceName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = instance_->RemoveInterfaceFromNetwork(INTERNAL_NETID, testInterfaceName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: AddRouteTest001
 * @tc.desc: Test ConnManager AddRoute.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, AddRouteTest001, TestSize.Level1)
{
    int32_t ret = instance_->AddRoute(NETID, INTERFACENAME, "0.0.0.0/0", "192.168.113.222");
    EXPECT_LE(ret, 0);
    ret = instance_->AddRoute(NETID, INTERFACENAME, "192.168.113.0/24", "0.0.0.0");
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name: RemoveRouteTest001
 * @tc.desc: Test ConnManager AddRoute.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, RemoveRouteTest001, TestSize.Level1)
{
    int32_t ret = instance_->RemoveRoute(NETID, INTERFACENAME, "0.0.0.0/0", "192.168.113.222");
    EXPECT_LE(ret, 0);
    ret = instance_->RemoveRoute(NETID, INTERFACENAME, "192.168.113.0/24", "0.0.0.0");
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name: UpdateRouteTest001
 * @tc.desc: Test ConnManager AddRoute.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, UpdateRouteTest001, TestSize.Level1)
{
    int32_t ret = instance_->UpdateRoute(NETID, INTERFACENAME, "0.0.0.0/0", "192.168.113.222");
    EXPECT_LE(ret, 0);
    ret = instance_->UpdateRoute(NETID, INTERFACENAME, "192.168.113.0/24", "0.0.0.0");
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name: UpdateRouteTest002
 * @tc.desc: Test ConnManager AddRoute.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, UpdateRouteTest002, TestSize.Level1)
{
    int32_t netId = 99;
    int32_t ret = instance_->UpdateRoute(netId, INTERFACENAME, "0.0.0.0/0", "192.168.113.222");
    EXPECT_EQ(ret, ERROR_CODE);
}

/**
 * @tc.name: SetPermissionForNetwork001
 * @tc.desc: Test ConnManager SetPermissionForNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, SetPermissionForNetwork001, TestSize.Level1)
{
    int32_t netId = 99;
    int32_t ret = instance_->SetPermissionForNetwork(netId, NetworkPermission::PERMISSION_NETWORK);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: AddUidsToNetwork001
 * @tc.desc: Test ConnManager AddUidsToNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, AddUidsToNetwork001, TestSize.Level1)
{
    int32_t netId = 99;
    const std::vector<NetManagerStandard::UidRange> uidRanges;
    int32_t ret = instance_->AddUidsToNetwork(netId, uidRanges);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netId = 1;
    ret = instance_->AddUidsToNetwork(netId, uidRanges);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RemoveUidsFromNetwork001
 * @tc.desc: Test ConnManager RemoveUidsFromNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, RemoveUidsFromNetwork001, TestSize.Level1)
{
    int32_t netId = 99;
    const std::vector<NetManagerStandard::UidRange> uidRanges;
    int32_t ret = instance_->RemoveUidsFromNetwork(netId, uidRanges);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netId = 1;
    ret = instance_->RemoveUidsFromNetwork(netId, uidRanges);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetFwmarkForNetworkTest001
 * @tc.desc: Test ConnManager GetFwmarkForNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, GetFwmarkForNetworkTest001, TestSize.Level1)
{
    int32_t ret = instance_->GetFwmarkForNetwork(NETID);
    EXPECT_LE(ret, 0);
    std::string info;
    instance_->GetDumpInfos(info);
    ASSERT_FALSE(info.empty());
}

/**
 * @tc.name: ConnManagerBranchTest001
 * @tc.desc: Test ConnManager Branch.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, ConnManagerBranchTest001, TestSize.Level1)
{
    std::string testInterfaceName = "testName";
    int32_t ret = instance_->GetNetworkForInterface(100, testInterfaceName);
    EXPECT_EQ(ret, INVALID_VALUE);

    RouteManager::TableType type = instance_->GetTableType(LOCAL_NET_ID);
    EXPECT_EQ(type, RouteManager::TableType::LOCAL_NETWORK);

    type = instance_->GetTableType(LOCAL_NET_ID);
    EXPECT_EQ(type, RouteManager::TableType::LOCAL_NETWORK);

    int32_t netId = 100;
    type = instance_->GetTableType(netId);
    EXPECT_EQ(type, RouteManager::TableType::INTERFACE);

    auto result = instance_->FindVirtualNetwork(NETID);
    EXPECT_EQ(result, nullptr);

    result = instance_->FindVirtualNetwork(netId);
    EXPECT_EQ(result, nullptr);

    netId = 99;
    result = instance_->FindVirtualNetwork(netId);
    EXPECT_EQ(result, nullptr);
}

} // namespace NetsysNative
} // namespace OHOS