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

#include "message_parcel.h"
#ifdef GTEST_API_
#define private public
#endif
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_security.h"
#include "net_conn_types.h"
#include "net_interface_callback_stub.h"
#include "net_interface_config.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "network.h"

#include "i_net_conn_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr const char *TEST_IPV4_ADDR = "127.0.0.1";
constexpr const char *TEST_IPV6_ADDR = "240C:1:1:1::1";
constexpr const char *TEST_DOMAIN1 = ".com";
constexpr const char *TEST_DOMAIN2 = "test.com";
constexpr const char *TEST_DOMAIN3 = "testcom";
constexpr const char *TEST_DOMAIN4 = "com.test";
constexpr const char *TEST_DOMAIN5 = "test.co.uk";
constexpr const char *TEST_DOMAIN6 = "test.com.com";
constexpr const char *TEST_DOMAIN7 = "test1.test2.test3.test4.test5.com";
constexpr const char *TEST_DOMAIN8 = "http://www.example.com";
constexpr const char *TEST_DOMAIN9 = "https://www.example.com";
constexpr const char *TEST_DOMAIN10 = "httpd://www.example.com";
constexpr const char *TEST_LONG_HOST =
    "0123456789qwertyuiopasdfghjklzxcvbnm[]:;<>?!@#$%^&*()qwdqwrtfasfj4897qwe465791qwr87tq4fq7t8qt4654qwr";
constexpr const char *TEST_LONG_EXCLUSION_LIST =
    "www.test0.com,www.test1.com,www.test2.com,www.test3.com,www.test4.com,www.test5.com,www.test6.com,www.test7.com,"
    "www.test8.com,www.test9.com,www.test10.com,www.test11.com,www.test12.com,www.test12.com,www.test12.com,www.test13."
    "com,www.test14.com,www.test15.com,www.test16.com,www.test17.com,www.test18.com,www.test19.com,www.test20.com";
constexpr const char *TEST_IFACE = "eth0";
} // namespace

class NetSupplierCallbackBaseTest : public NetSupplierCallbackBase {
public:
    virtual ~NetSupplierCallbackBaseTest() = default;

    int32_t RequestNetwork(const std::string &ident, const std::set<NetCap> &netCaps) override
    {
        return NETMANAGER_SUCCESS;
    };

    int32_t ReleaseNetwork(const std::string &ident, const std::set<NetCap> &netCaps) override
    {
        return NETMANAGER_SUCCESS;
    };
};

class NetConnClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetConnClientTest::SetUpTestCase() {}

void NetConnClientTest::TearDownTestCase() {}

void NetConnClientTest::SetUp() {}

void NetConnClientTest::TearDown() {}

class INetConnCallbackTest : public IRemoteStub<INetConnCallback> {
public:
    int32_t NetAvailable(sptr<NetHandle> &netHandle)
    {
        return 0;
    }

    int32_t NetCapabilitiesChange(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap)
    {
        return 0;
    }

    int32_t NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info)
    {
        return 0;
    }

    int32_t NetLost(sptr<NetHandle> &netHandle)
    {
        return 0;
    }

    int32_t NetUnavailable()
    {
        return 0;
    }

    int32_t NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
    {
        return 0;
    }
};

/**
 * @tc.name: GetDefaultNetTest001
 * @tc.desc: Test NetConnClient::GetDefaultNet, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultNetTest001, TestSize.Level1)
{
    std::cout << "GetDefaultNetTest001 In" << std::endl;
    NetHandle handle;
    auto ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetDefaultNetTest002
 * @tc.desc: Test NetConnClient::GetDefaultNet, not applying for
 * permission,return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultNetTest002, TestSize.Level1)
{
    std::cout << "GetDefaultNetTest002 In" << std::endl;
    NetConnManagerAccessToken token;
    NetHandle handle;
    int32_t netId = 0;
    auto ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    netId = handle.GetNetId();
    if (netId == 0) {
        std::cout << "No network" << std::endl;
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    } else if (netId >= 100 && netId <= MAX_NET_ID) {
        std::cout << "Get default network id:" << netId << std::endl;
        ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    } else {
        ASSERT_FALSE(ret == NETMANAGER_SUCCESS);
    }
}

/**
 * @tc.name: HasDefaultNetTest001
 * @tc.desc: Test NetConnClient::HasDefaultNet,not applying for
 * permission, return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, HasDefaultNetTest001, TestSize.Level1)
{
    bool bFlag = false;
    auto ret = NetConnClient::GetInstance().HasDefaultNet(bFlag);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: HasDefaultNetTest002
 * @tc.desc: Test NetConnClient::HasDefaultNet, applying for
 * permission, return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, HasDefaultNetTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    bool bFlag = false;
    auto ret = NetConnClient::GetInstance().HasDefaultNet(bFlag);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetCapabilitiesTest001
 * @tc.desc: Test NetConnClient::GetNetCapabilities, In the absence of
 * permission, GetDefaultNet return NETMANAGER_ERR_PERMISSION_DENIED and
 * GetNetCapabilities return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetCapabilitiesTest001, TestSize.Level1)
{
    NetHandle handle;
    int32_t ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);

    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetNetCapabilitiesTest002
 * @tc.desc: Test NetConnClient::GetNetCapabilities:In the absence of
 * permission, GetDefaultNet return NETMANAGER_ERR_PERMISSION_DENIED, and
 * after add permission GetNetCapabilities return NET_CONN_ERR_INVALID_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetCapabilitiesTest002, TestSize.Level1)
{
    NetHandle handle;
    int32_t ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_TRUE(ret == NETMANAGER_ERR_PERMISSION_DENIED);

    NetConnManagerAccessToken token;
    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    ASSERT_TRUE(ret == NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: GetNetCapabilitiesTest003
 * @tc.desc: Test NetConnClient::GetNetCapabilities:Apply for permission at
 * first, when net is connected,return NET_CONN_SUCCESS, or net is not connected,return
 * NET_CONN_ERR_INVALID_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetCapabilitiesTest003, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    NetHandle handle;
    int32_t ret = NetConnClient::GetInstance().GetDefaultNet(handle);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS || ret == NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: SetAirplaneModeTest001
 * @tc.desc: Test NetConnClient::SetAirplaneMode
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAirplaneModeTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    auto ret = NetConnClient::GetInstance().SetAirplaneMode(true);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetAirplaneModeTest002
 * @tc.desc: Test NetConnClient::SetAirplaneMode
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAirplaneModeTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    auto ret = NetConnClient::GetInstance().SetAirplaneMode(false);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsDefaultNetMeteredTest001
 * @tc.desc: if no permission,NetConnClient::IsDefaultNetMetered return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, IsDefaultNetMeteredTest001, TestSize.Level1)
{
    bool bRes = false;
    auto ret = NetConnClient::GetInstance().IsDefaultNetMetered(bRes);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
    ASSERT_TRUE(bRes == false);
}

/**
 * @tc.name: IsDefaultNetMeteredTest002
 * @tc.desc: Test NetConnClient::IsDefaultNetMetered
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, IsDefaultNetMeteredTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    bool bRes = false;
    auto ret = NetConnClient::GetInstance().IsDefaultNetMetered(bRes);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    ASSERT_TRUE(bRes == true);
}

/**
 * @tc.name: SetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest002
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN1, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest003
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest003, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN2, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest004
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest004, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN3, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest005
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest005, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN4, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest006
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest006, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN5, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest007
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest007, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN6, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest008
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest008, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN7, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest09
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest09, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN8, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest10
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest10, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN9, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest11
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest11, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN10, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest012
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest012, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest013
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest013, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest14
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest14, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_LONG_HOST, 8080, {TEST_LONG_EXCLUSION_LIST}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest015
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest015, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy;
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest016
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.not applying for permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest016, TestSize.Level1)
{
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    auto ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_IPV4_ADDR);
}

/**
 * @tc.name: GetGlobalHttpProxyTest002
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_IPV6_ADDR);
}

/**
 * @tc.name: GetGlobalHttpProxyTest003
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest003, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN2, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_DOMAIN2);
}

/**
 * @tc.name: GetGlobalHttpProxyTest004
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest004, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy validHttpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_IPV4_ADDR);
}

/**
 * @tc.name: GetGlobalHttpProxyTest005
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest005, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy httpProxy;
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = NetConnClient::GetInstance().GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost().empty());
}

/**
 * @tc.name: GetDefaultHttpProxyTest001
 * @tc.desc: Test NetConnClient::GetDefaultHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultHttpProxyTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy validHttpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy defaultHttpProxy;
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(defaultHttpProxy.GetHost() == TEST_IPV4_ADDR);
}

/**
 * @tc.name: GetDefaultHttpProxyTest002
 * @tc.desc: Test NetConnClient::GetDefaultHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultHttpProxyTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    HttpProxy globalHttpProxy;
    int32_t ret = NetConnClient::GetInstance().SetGlobalHttpProxy(globalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy defaultHttpProxy;
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: GetDefaultHttpProxyTest003
 * @tc.desc: Test NetConnClient::SetAppNet and NetConnClient::GetDefaultHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultHttpProxyTest003, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    int32_t netId = 102;
    int32_t ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    HttpProxy defaultHttpProxy;
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    int32_t cancelNetId = 0;
    ret = NetConnClient::GetInstance().SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = NetConnClient::GetInstance().GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: RegisterNetSupplier001
 * @tc.desc: Test NetConnClient::RegisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplier001, TestSize.Level1)
{
    uint32_t supplierId = 100;
    NetBearType netBearType = BEARER_WIFI;
    const std::string ident = "";
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    auto ret = NetConnClient::GetInstance().RegisterNetSupplier(netBearType, ident, netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetSupplier002
 * @tc.desc: Test NetConnClient::RegisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplier002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    uint32_t supplierId = 100;
    NetBearType netBearType = BEARER_WIFI;
    const std::string ident = "";
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    auto ret = NetConnClient::GetInstance().RegisterNetSupplier(netBearType, ident, netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnregisterNetSupplier001
 * @tc.desc: Test NetConnClient::UnregisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UnregisterNetSupplier001, TestSize.Level1)
{
    uint32_t supplierId = 100;
    auto ret = NetConnClient::GetInstance().UnregisterNetSupplier(supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnregisterNetSupplier002
 * @tc.desc: Test NetConnClient::UnregisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UnregisterNetSupplier002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    uint32_t supplierId = 100;
    auto ret = NetConnClient::GetInstance().UnregisterNetSupplier(supplierId);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest001
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest001, TestSize.Level1)
{
    uint32_t supplierId = 100;
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    uint32_t supplierId = 100;
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest003
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest003, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET};
    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result = NetConnClient::GetInstance().RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    ASSERT_TRUE(result == NETMANAGER_SUCCESS);
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest004
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest004, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    uint32_t supplierId = 0;
    sptr<NetSupplierCallbackBase> callback;
    auto ret = NetConnClient::GetInstance().RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: SetAppNetTest001
 * @tc.desc: Test NetConnClient::SetAppNet, if param is invalid, SetAppNet return NET_CONN_ERR_INVALID_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAppNetTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    int32_t netId = 99;
    auto ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: SetAppNetTest002
 * @tc.desc: Test NetConnClient::SetAppNet, if param is valid, SetAppNet return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAppNetTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    int32_t netId = 102;
    auto ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t cancelNetId = 0;
    ret = NetConnClient::GetInstance().SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAppNetTest001
 * @tc.desc: Test NetConnClient::GetAppNet, return NetId set by SetAppNet
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAppNetTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    int32_t netId = 102;
    auto ret = NetConnClient::GetInstance().SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t getNetId = 0;
    NetConnClient::GetInstance().GetAppNet(getNetId);
    EXPECT_EQ(getNetId, netId);

    int32_t cancelNetId = 0;
    ret = NetConnClient::GetInstance().SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetConnCallback001
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetConnCallback001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    int32_t ret = NetConnClient::GetInstance().RegisterNetConnCallback(callback);
    ret = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetConnCallback002
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetConnCallback002, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = nullptr;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 1;
    auto ret = NetConnClient::GetInstance().RegisterNetConnCallback(netSpecifier, callback, timesOut);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: RegisterNetConnCallback002
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetConnCallback003, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    uint32_t timesOut = 1;
    auto ret = NetConnClient::GetInstance().RegisterNetConnCallback(netSpecifier, callback, timesOut);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name: RegisterNetConnCallback001
 * @tc.desc: Test NetConnClient::RegisterNetConnCallback, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UnRegisterNetConnCallback001, TestSize.Level1)
{
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    int32_t ret = NetConnClient::GetInstance().RegisterNetConnCallback(callback);
    ret = NetConnClient::GetInstance().UnregisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UpdateNetSupplierInfo001
 * @tc.desc: Test NetConnClient::UpdateNetSupplierInfo, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UpdateNetSupplierInfo001, TestSize.Level1)
{
    auto &client = NetConnClient::GetInstance();
    uint32_t supplierId = 1;
    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo;
    int32_t ret = client.UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UpdateNetSupplierInfo002
 * @tc.desc: Test NetConnClient::UpdateNetSupplierInfo, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UpdateNetSupplierInfo002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    auto &client = NetConnClient::GetInstance();
    uint32_t supplierId = 1;
    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo;
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    int32_t ret = client.UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: GetNetInterfaceConfigurationTest001
 * @tc.desc: Test NetConnClient::GetNetInterfaceConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetInterfaceConfigurationTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    NetInterfaceConfiguration config;
    auto ret = NetConnClient::GetInstance().GetNetInterfaceConfiguration(TEST_IFACE, config);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetInterfaceConfigurationTest001
 * @tc.desc: Test NetConnClient::GetNetInterfaceConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetInterfaceConfigurationTest002, TestSize.Level1)
{
    NetInterfaceConfiguration config;
    auto ret = NetConnClient::GetInstance().GetNetInterfaceConfiguration(TEST_IFACE, config);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetInterfaceCallbackTest001
 * @tc.desc: Test NetConnClient::RegisterNetInterfaceCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetInterfaceCallbackTest001, TestSize.Level1)
{
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    int32_t ret = NetConnClient::GetInstance().RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetInterfaceCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetInterfaceCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetInterfaceCallbackTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    int32_t ret = NetConnClient::GetInstance().RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SystemReadyTest002
 * @tc.desc: Test NetConnClient::SystemReady
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SystemReadyTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SystemReady();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateNetLinkInfoTest002
 * @tc.desc: Test NetConnClient::UpdateNetLinkInfo
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UpdateNetLinkInfoTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    uint32_t supplierId = 1;
    sptr<NetLinkInfo> netLinkInfo = std::make_unique<NetLinkInfo>().release();
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: GetAllNetsTest002
 * @tc.desc: Test NetConnClient::GetAllNets
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAllNetsTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    std::list<sptr<NetHandle>> netList;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(netList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetConnectionPropertiesTest002
 * @tc.desc: Test NetConnClient::GetConnectionProperties
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetConnectionPropertiesTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    NetHandle netHandle;
    NetLinkInfo info;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(netHandle, info);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: GetAddressesByNameTest002
 * @tc.desc: Test NetConnClient::GetAddressesByName
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAddressesByNameTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    const std::string host = "ipaddr";
    int32_t netId = 1;
    std::vector<INetAddr> addrList = {};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAddressesByName(host, netId, addrList);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: GetAddressByNameTest002
 * @tc.desc: Test NetConnClient::GetAddressByName
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAddressByNameTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    std::string host = "ipaddr";
    int32_t netId = 1;
    INetAddr addr;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAddressByName(host, netId, addr);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: BindSocketTest002
 * @tc.desc: Test NetConnClient::BindSocket
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, BindSocketTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    NetConnClient::NetConnDeathRecipient deathRecipient(*DelayedSingleton<NetConnClient>::GetInstance());
    sptr<IRemoteObject> remote = nullptr;
    deathRecipient.OnRemoteDied(remote);
    int32_t socket_fd = 0;
    int32_t netId = 99;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socket_fd, netId);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
    netId = 101;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socket_fd, netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetDetectionTest002
 * @tc.desc: Test NetConnClient::NetDetection
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, NetDetectionTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    NetHandle netHandle;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(netHandle);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);
}

HWTEST_F(NetConnClientTest, NetworkRouteTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    int32_t netId = 10;
    std::string ifName = "wlan0";
    std::string destination = "0.0.0.0/0";
    std::string nextHop = "0.0.0.1234";

    int32_t ret = NetConnClient::GetInstance().AddNetworkRoute(netId, ifName, destination, nextHop);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    ret = NetConnClient::GetInstance().RemoveNetworkRoute(netId, ifName, destination, nextHop);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnClientTest, InterfaceAddressTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    std::string ifName = "wlan0";
    std::string ipAddr = "0.0.0.1";
    int32_t prefixLength = 23;

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->AddInterfaceAddress(ifName, ipAddr, prefixLength);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelInterfaceAddress(ifName, ipAddr, prefixLength);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnClientTest, StaticArpTest001, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    std::string ifName = "wlan0";
    std::string ipAddr = "123.12.12.123";
    std::string macAddr = "12:23:34:12:12:11";
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ipAddr = "1234";
    macAddr = "12:23:34:12:12:11";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ipAddr = "123.12.12.123";
    macAddr = "12:234:34";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetConnClientTest, StaticArpTest002, TestSize.Level1)
{
    NetConnManagerAccessToken token;
    std::string ipAddr = "123.12.12.123";
    std::string macAddr = "12:23:34:12:12:11";
    std::string ifName = "wlan0";
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ipAddr = "123.12.12.123";
    macAddr = "12:23:34:12:12:11";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);

    ipAddr = "123.12.12.1235678";
    macAddr = "12:23:34:12:12:11";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ipAddr = "123.12.12.123";
    macAddr = "12:23:34:12:12";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ipAddr = "123.12.12.123";
    macAddr = "12:23:34:12:12:11";
    ifName = "";
    ret = DelayedSingleton<NetConnClient>::GetInstance()->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetConnClientTest, NetConnClientBranchTest001, TestSize.Level1)
{
    int32_t uid = 0;
    uint8_t allow = 0;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetInternetPermission(uid, allow);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);

    uint32_t supplierId = 0;
    sptr<NetSupplierInfo> netSupplierInfo = nullptr;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<NetLinkInfo> netLinkInfo = nullptr;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}
} // namespace NetManagerStandard
} // namespace OHOS
