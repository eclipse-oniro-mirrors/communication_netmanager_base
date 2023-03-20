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

#include "iservice_registry.h"
#include "system_ability_definition.h"

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
constexpr int32_t NETID = 101;
const std::string INTERFACENAME = "wlan0";
class NetsysNativeServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetsysNativeServiceProxyTest::SetUpTestCase() {}

void NetsysNativeServiceProxyTest::TearDownTestCase() {}

void NetsysNativeServiceProxyTest::SetUp() {}

void NetsysNativeServiceProxyTest::TearDown() {}

/**
 * @tc.name: AddInterfaceToNetworkTest001
 * @tc.desc: Test NetsysNativeServiceProxy AddInterfaceToNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetsysNativeServiceProxyTest, AddInterfaceToNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->NetworkCreatePhysical(NETID, nmd::NetworkPermission::PERMISSION_NONE);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ret = netsysNativeService->NetworkAddInterface(NETID, INTERFACENAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ret = netsysNativeService->InterfaceAddAddress(INTERFACENAME, "192.168.113.209", 24);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: AddRouteTest001
 * @tc.desc: Test NetsysNativeServiceProxy AddRoute.
 * @tc.type: FUNC
 */
HWTEST_F(NetsysNativeServiceProxyTest, AddRouteTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->NetworkAddRoute(NETID, INTERFACENAME, "0.0.0.0/0", "192.168.113.222");
    EXPECT_LE(ret, 0);
    ret = netsysNativeService->NetworkAddRoute(NETID, INTERFACENAME, "192.168.113.0/24", "0.0.0.0");
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name: SetDefaultNetworkTest001
 * @tc.desc: Test NetsysNativeServiceProxy SetDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetsysNativeServiceProxyTest, SetDefaultNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->NetworkSetDefault(NETID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetDefaultNetworkTest001
 * @tc.desc: Test NetsysNativeServiceProxy GetDefaultNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetsysNativeServiceProxyTest, GetDefaultNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->NetworkGetDefault();
    EXPECT_EQ(ret, NETID);
}

/**
 * @tc.name: RemoveInterfaceFromNetworkTest001
 * @tc.desc: Test NetsysNativeServiceProxy RemoveInterfaceFromNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetsysNativeServiceProxyTest, RemoveInterfaceFromNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->InterfaceDelAddress(INTERFACENAME, "192.168.113.209", 24);
    EXPECT_LE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ret = netsysNativeService->NetworkRemoveInterface(NETID, INTERFACENAME);
    EXPECT_LE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: DestroyNetworkTest001
 * @tc.desc: Test NetsysNativeServiceProxy DestroyNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetsysNativeServiceProxyTest, DestroyNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->NetworkDestroy(NETID);
    EXPECT_LE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetsysNative
} // namespace OHOS