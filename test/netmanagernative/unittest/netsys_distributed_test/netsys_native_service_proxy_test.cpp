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

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "bpf_def.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "conn_manager.h"
#include "net_manager_constants.h"
#include "net_stats_constants.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_proxy.h"
#include "network_permission.h"

#include "net_all_capabilities.h"
#include "net_conn_client.h"
#include "net_handle.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace NetManagerStandard;
constexpr int32_t NETID = 101;
constexpr int32_t UID = 1000;
constexpr int32_t MTU = 1500;
constexpr int32_t WHICH = 14;
const std::string INTERFACENAME = "wlan0";
static constexpr uint64_t TEST_COOKIE = 1;
static constexpr uint32_t TEST_STATS_TYPE1 = 0;
static constexpr uint32_t TEST_STATS_TYPE2 = 2;
namespace {
sptr<NetsysNative::INetsysService> ConnManagerGetProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        return nullptr;
    }

    auto remote = samgr->GetSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    if (remote == nullptr) {
        return nullptr;
    }

    auto proxy = iface_cast<NetsysNative::INetsysService>(remote);
    if (proxy == nullptr) {
        return nullptr;
    }
    return proxy;
}
} // namespace
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

HWTEST_F(NetsysNativeServiceProxyTest, EnableDistributedClientNet001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    std::string virnicAddr = "1.189.55.60";
    std::string iif = "lo";
    int32_t ret = netsysNativeService->EnableDistributedClientNet(virnicAddr, iif);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    bool isServer = false;
    ret = netsysNativeService->DisableDistributedNet(isServer);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, EnableDistributedClientNet002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    std::string virnicAddr = "";
    std::string iif = "";
    int32_t ret = netsysNativeService->EnableDistributedClientNet(virnicAddr, iif);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, EnableDistributedServerNet001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    std::string dstAddr = "1.189.55.61";
    std::string devIface = "lo";
    std::string iif = "lo";
    int32_t ret = netsysNativeService->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    bool isServer = true;
    ret = netsysNativeService->DisableDistributedNet(isServer);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, EnableDistributedServerNet002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    std::string dstAddr = "";
    std::string devIface = "";
    std::string iif = "";
    int32_t ret = netsysNativeService->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetsysNative
} // namespace OHOS
