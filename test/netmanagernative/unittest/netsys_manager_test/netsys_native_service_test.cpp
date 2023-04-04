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

#include "system_ability_definition.h"

#include "dns_config_client.h"
#include "net_manager_constants.h"
#define private public
#include "netsys_native_service.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace NetManagerStandard;
using namespace testing::ext;
} // namespace

class NetsysNativeServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = std::make_shared<NetsysNativeService>(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
};

void NetsysNativeServiceTest::SetUpTestCase() {}

void NetsysNativeServiceTest::TearDownTestCase() {}

void NetsysNativeServiceTest::SetUp() {}

void NetsysNativeServiceTest::TearDown() {}

HWTEST_F(NetsysNativeServiceTest, DumpTest001, TestSize.Level1)
{
    instance_->Init();
    int32_t testFd = 11;
    int32_t ret = instance_->Dump(testFd, {});
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, SetResolverConfigTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    uint16_t baseTimeoutMsec = 200;
    uint8_t retryCount = 3;
    int32_t ret = instance_->SetResolverConfig(testNetId, baseTimeoutMsec, retryCount, {}, {});
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, GetResolverConfigTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    uint16_t baseTimeoutMsec = 200;
    uint8_t retryCount = 3;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    int32_t ret = instance_->GetResolverConfig(testNetId, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, CreateNetworkCacheTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    int32_t ret = instance_->CreateNetworkCache(testNetId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, DestroyNetworkCacheTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    int32_t ret = instance_->DestroyNetworkCache(testNetId);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkAddRouteTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string interfaceName = "eth1";
    std::string destination = "";
    std::string nextHop = "";
    int32_t ret = instance_->NetworkAddRoute(testNetId, interfaceName, destination, nextHop);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkAddRouteParcelTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    RouteInfoParcel routeInfo;
    int32_t ret = instance_->NetworkAddRouteParcel(testNetId, routeInfo);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkRemoveRouteParcelTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    RouteInfoParcel routeInfo;
    int32_t ret = instance_->NetworkRemoveRouteParcel(testNetId, routeInfo);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkSetDefaultTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    int32_t ret = instance_->NetworkSetDefault(testNetId);
    EXPECT_GE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkGetDefaultTest001, TestSize.Level1)
{
    int32_t ret = instance_->NetworkGetDefault();
    EXPECT_LE(ret, 154);
}

HWTEST_F(NetsysNativeServiceTest, NetworkClearDefaultTest001, TestSize.Level1)
{
    int32_t ret = instance_->NetworkClearDefault();
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, GetProcSysNetTest001, TestSize.Level1)
{
    int32_t ipversion = 45;
    int32_t which = 14;
    std::string ifname = "testifname";
    std::string paramete = "testparamete";
    std::string value = "testvalue";
    int32_t ret = instance_->GetProcSysNet(ipversion, which, ifname, paramete, value);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, SetProcSysNetTest001, TestSize.Level1)
{
    int32_t ipversion = 45;
    int32_t which = 14;
    std::string ifname = "testifname";
    std::string paramete = "testparamete";
    std::string value = "testvalue";
    int32_t ret = instance_->SetProcSysNet(ipversion, which, ifname, paramete, value);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}
} // namespace NetsysNative
} // namespace OHOS