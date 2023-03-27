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

#include "accesstoken_kit.h"
#include "net_all_capabilities.h"
#include "net_conn_service.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_types.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "token_setproc.h"
#include "net_supplier_callback_stub.h"
#include "net_conn_callback_stub.h"
#include "http_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *TEST_IDENT = "testIdent";
constexpr uint32_t TEST_TIMEOUTMS = 1000;
constexpr const char *TEST_HOST = "testHost";
constexpr int32_t TEST_NETID = 3;
constexpr int32_t TEST_SOCKETFD = 2;
const int32_t NET_ID = 2;
const int32_t SOCKET_FD = 2;

class NetSupplierTestCallback : public NetSupplierCallbackStub {
public:
    inline int32_t RequestNetwork(const std::string &ident, const std::set<NetCap> &netCaps) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t ReleaseNetwork(const std::string &ident, const std::set<NetCap> &netCaps) override
    {
        return NETMANAGER_SUCCESS;
    }
};

class NetConnTestCallback : public NetConnCallbackStub {
public:
    inline int32_t NetAvailable(sptr<NetHandle> &netHandle) override
    {
        return 0;
    }
    inline int32_t NetCapabilitiesChange(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap) override
    {
        return 0;
    }
    inline int32_t NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info) override
    {
        return 0;
    }
    inline int32_t NetLost(sptr<NetHandle> &netHandle) override
    {
        return 0;
    }
    inline int32_t NetUnavailable() override
    {
        return 0;
    }
    inline int32_t NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked) override
    {
        return 0;
    }
};
sptr<INetConnCallback> g_callback = new (std::nothrow) NetConnTestCallback();
uint32_t g_supplierId = 0;
} // namespace

class NetConnServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetConnServiceTest::SetUpTestCase()
{
    std::set<NetCap> netCaps;
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT,
                                                                        netCaps, g_supplierId);
}

void NetConnServiceTest::TearDownTestCase() {}

void NetConnServiceTest::SetUp() {}

void NetConnServiceTest::TearDown() {}

HWTEST_F(NetConnServiceTest, OnStopTest001, TestSize.Level1)
{
    DelayedSingleton<NetConnService>::GetInstance()->OnStop();
}

HWTEST_F(NetConnServiceTest, OnStartTest001, TestSize.Level1)
{
    DelayedSingleton<NetConnService>::GetInstance()->OnStart();
}

HWTEST_F(NetConnServiceTest, SystemReadyTest001, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->SystemReady();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, RegisterNetSupplierCallbackTest001, TestSize.Level1)
{
    sptr<INetSupplierCallback> callback = new (std::nothrow) NetSupplierTestCallback();
    ASSERT_NE(callback, nullptr);
    std::set<NetCap> netCaps;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->RegisterNetSupplierCallback(g_supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, UpdateNetSupplierInfoTest001, TestSize.Level1)
{
    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo();
    ASSERT_NE(netSupplierInfo, nullptr);
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->UpdateNetSupplierInfo(g_supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, UpdateNetLinkInfoTest001, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->UpdateNetLinkInfo(g_supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, RegisterNetConnCallbackTest001, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->RegisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, UnregisterNetConnCallbackTest001, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->UnregisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, RegisterNetConnCallbackTest002, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    ASSERT_NE(netSpecifier, nullptr);
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->RegisterNetConnCallback(netSpecifier, g_callback,
                                                                                        TEST_TIMEOUTMS);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, UnregisterNetConnCallbackTest002, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->UnregisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetAllNetsTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetAllNets(netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetConnectionPropertiesTest001, TestSize.Level1)
{
    NetLinkInfo info;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetConnectionProperties(TEST_NETID, info);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceTest, GetAddressesByNameTest001, TestSize.Level1)
{
    std::vector<INetAddr> addrList;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetAddressesByName(TEST_HOST, TEST_NETID, addrList);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetAddressByNameTest001, TestSize.Level1)
{
    INetAddr addr;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetAddressByName(TEST_HOST, TEST_NETID, addr);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, BindSocketTest001, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->BindSocket(TEST_SOCKETFD, TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, NetDetectionTest001, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->NetDetection(TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetNetIdByIdentifierTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetNetIdByIdentifier(TEST_IDENT, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetDefaultNetTest001, TestSize.Level1)
{
    int32_t netId = 0;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, HasDefaultNetTest001, TestSize.Level1)
{
    bool bFlag = false;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->HasDefaultNet(bFlag);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetNetCapabilitiesTest001, TestSize.Level1)
{
    int32_t netId = 0;
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);

    NetAllCapabilities netAllCap;
    ret = DelayedSingleton<NetConnService>::GetInstance()->GetNetCapabilities(netId, netAllCap);
    ASSERT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceTest, SetAirplaneModeTest001, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetAirplaneMode(true);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetAirplaneModeTest002, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetAirplaneMode(false);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, IsDefaultNetMeteredTest001, TestSize.Level1)
{
    bool bRes = false;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->IsDefaultNetMetered(bRes);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest001, TestSize.Level1)
{
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    HttpProxy httpProxy;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_EQ(ret, NETMANAGER_ERROR);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnService>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    netIdList.push_back(NET_ID);
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->GetSpecificNet(BEARER_CELLULAR, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = DelayedSingleton<NetConnService>::GetInstance()->RestrictBackgroundChanged(true);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    std::vector<std::u16string> args;
    args.emplace_back(u"dummy data");
    ret = DelayedSingleton<NetConnService>::GetInstance()->Dump(SOCKET_FD, args);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    DelayedSingleton<NetConnService>::GetInstance()->OnNetActivateTimeOut(NET_ID);
}
} // namespace NetManagerStandard
} // namespace OHOS
