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

    NetManagerBaseAccessToken access;
    NetHandle handle;
    NetConnClient::GetInstance().GetDefaultNet(handle);
    NetAllCapabilities netAllCap;
    NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    if (netAllCap.bearerTypes_.count(NetManagerStandard::BEARER_CELLULAR) > 0 ||
        netAllCap.bearerTypes_.count(NetManagerStandard::BEARER_WIFI) > 0) {
        return;
    }

    ret = netsysNativeService->NetworkAddInterface(NETID, INTERFACENAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ret = netsysNativeService->AddInterfaceAddress(INTERFACENAME, "192.168.113.209", 24);
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
 * @tc.name: GetAllContainerStatsInfoTest001
 * @tc.desc: Test NetsysNativeServiceProxy GetAllContainerStatsInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetsysNativeServiceProxyTest, GetAllContainerStatsInfoTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> stats;
    int32_t ret = netsysNativeService->GetAllContainerStatsInfo(stats);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
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
    int32_t ret = netsysNativeService->DelInterfaceAddress(INTERFACENAME, "192.168.113.209", 24);
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

HWTEST_F(NetsysNativeServiceProxyTest, NetworkAddRouteParcelTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    RouteInfoParcel routeInfo;
    routeInfo.destination = "destination";
    routeInfo.ifName = INTERFACENAME;
    routeInfo.nextHop = "nextHop";
    routeInfo.mtu = MTU;
    int32_t ret = netsysNativeService->NetworkAddRouteParcel(NETID, routeInfo);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysNativeServiceProxyTest, NetworkRemoveRouteParcelTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    RouteInfoParcel routeInfo;
    routeInfo.destination = "";
    routeInfo.ifName = INTERFACENAME;
    routeInfo.nextHop = "";
    routeInfo.mtu = MTU;
    int32_t ret = netsysNativeService->NetworkRemoveRouteParcel(NETID, routeInfo);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysNativeServiceProxyTest, NetworkClearDefaultTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->NetworkClearDefault();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, GetSetProcSysNetTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    std::string parameter = "TestParameter";
    std::string value = "Testvalue";
    int32_t ret = netsysNativeService->SetProcSysNet(AF_INET, WHICH, INTERFACENAME, parameter, value);
    ret = netsysNativeService->GetProcSysNet(AF_INET, WHICH, INTERFACENAME, parameter, value);
    EXPECT_GE(ret, ERR_FLATTEN_OBJECT);
}

HWTEST_F(NetsysNativeServiceProxyTest, GetProcSysNetTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->SetInternetPermission(UID, true, false);
    ret = netsysNativeService->NetworkCreateVirtual(NETID, true);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, AddStaticArp001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, DelStaticArp001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    int32_t ret = netsysNativeService->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, GetFwmarkForNetworkTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    MarkMaskParcel markMaskParcel;
    markMaskParcel.mark = 1;
    markMaskParcel.mask = 0XFFFF;
    int32_t ret = netsysNativeService->GetFwmarkForNetwork(NETID, markMaskParcel);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}


HWTEST_F(NetsysNativeServiceProxyTest, NetsysNativeServiceProxyBranchTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    sptr<OHOS::NetsysNative::INetDnsResultCallback> resultCallback = nullptr;
    uint32_t timeStep = 0;
    int32_t ret = netsysNativeService->RegisterDnsResultCallback(resultCallback, timeStep);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = netsysNativeService->UnregisterDnsResultCallback(resultCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<OHOS::NetsysNative::INetDnsHealthCallback> healthCallback = nullptr;
    ret = netsysNativeService->RegisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = netsysNativeService->UnregisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<INotifyCallback> notifyCallback = nullptr;
    ret = netsysNativeService->RegisterNotifyCallback(notifyCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = netsysNativeService->UnRegisterNotifyCallback(notifyCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    uint64_t stats = 0;
    uint32_t type = 0;
    uint64_t cookie = 0;
    ret = netsysNativeService->GetCookieStats(stats, type, cookie);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetsysNativeServiceProxyTest, GetNetworkSharingTypeTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    std::set<uint32_t> sharingTypeIsOn;
    int32_t ret = netsysNativeService->GetNetworkSharingType(sharingTypeIsOn);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, UpdateNetworkSharingTypeTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    uint32_t type = 0;
    int32_t ret = netsysNativeService->UpdateNetworkSharingType(type, true);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = netsysNativeService->UpdateNetworkSharingType(type, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}


HWTEST_F(NetsysNativeServiceProxyTest, SetNetworkAccessPolicy001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    uint32_t uid = 0;
    NetworkAccessPolicy netAccessPolicy;
    netAccessPolicy.wifiAllow = false;
    netAccessPolicy.cellularAllow = false;
    bool reconfirmFlag = true;
    int32_t ret = netsysNativeService->SetNetworkAccessPolicy(uid, netAccessPolicy, reconfirmFlag);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, NotifyNetBearerTypeChange001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    std::set<NetManagerStandard::NetBearType> bearerTypes;
    bearerTypes.insert(NetManagerStandard::NetBearType::BEARER_CELLULAR);
    int32_t ret = netsysNativeService->NotifyNetBearerTypeChange(bearerTypes);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceProxyTest, DeleteNetworkAccessPolicy001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);

    uint32_t uid = 0;
    int32_t ret = netsysNativeService->DeleteNetworkAccessPolicy(uid);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetsysNative
} // namespace OHOS
