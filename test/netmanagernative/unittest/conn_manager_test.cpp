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

#include "conn_manager.h"
#include "conn_manager_test.h"
#include "iservice_registry.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_proxy.h"
#include "network_permission.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace ConnGetProxy;
constexpr int32_t NETID = 101;
const std::string INTERFACENAME = "wlan0";
class ConnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ConnManagerTest::SetUpTestCase() {}

void ConnManagerTest::TearDownTestCase() {}

void ConnManagerTest::SetUp() {}

void ConnManagerTest::TearDown() {}

/**
 * @tc.name: CreatePhysicalNetworkTest001
 * @tc.desc: Test ConnManager CreatePhysicalNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, CreatePhysicalNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->NetworkCreatePhysical(NETID, PERMISSION_NONE);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: AddInterfaceToNetworkTest001
 * @tc.desc: Test ConnManager AddInterfaceToNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, AddInterfaceToNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->NetworkAddInterface(NETID, INTERFACENAME);
    EXPECT_TRUE(ret == 0);
    ret = netsysNativeService->InterfaceAddAddress(INTERFACENAME, "192.168.113.209", 24);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: AddRouteTest001
 * @tc.desc: Test ConnManager AddRoute.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, AddRouteTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->NetworkAddRoute(NETID, INTERFACENAME, "0.0.0.0/0", "192.168.113.222");
    EXPECT_TRUE(ret == 0);
    ret = netsysNativeService->NetworkAddRoute(NETID, INTERFACENAME, "192.168.113.0/24", "0.0.0.0");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: SetDefaultNetworkTest001
 * @tc.desc: Test ConnManager SetDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, SetDefaultNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->NetworkSetDefault(NETID);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: GetDefaultNetworkTest001
 * @tc.desc: Test ConnManager GetDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, GetDefaultNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->NetworkGetDefault();
    EXPECT_TRUE(ret == NETID);
}

/**
 * @tc.name: RemoveInterfaceFromNetworkTest001
 * @tc.desc: Test ConnManager RemoveInterfaceFromNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, RemoveInterfaceFromNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->InterfaceDelAddress(INTERFACENAME, "192.168.113.209", 24);
    EXPECT_TRUE(ret == 0);
    ret = netsysNativeService->NetworkRemoveInterface(NETID, INTERFACENAME);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: DestroyNetworkTest001
 * @tc.desc: Test ConnManager DestroyNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, DestroyNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->NetworkDestroy(NETID);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: ClearDefaultNetwork001
 * @tc.desc: Test ConnManager ClearDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(ConnManagerTest, ClearDefaultNetwork001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_TRUE(netsysNativeService != nullptr);
    int32_t ret = netsysNativeService->NetworkClearDefault();
    EXPECT_TRUE(ret == 0);
}
} // namespace NetsysNative
} // namespace OHOS