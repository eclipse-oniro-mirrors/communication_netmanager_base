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

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_types.h"
#include "net_interface_callback_stub.h"
#include "net_interface_config.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "network.h"
#include "token_setproc.h"

#include "i_net_conn_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

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

HapInfoParams testInfoParms = {.bundleName = "net_conn_manager_test",
                               .userID = 1,
                               .instIndex = 0,
                               .appIDDesc = "test",
                               .isSystemApp = true};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "net_conn_manager_test",
    .grantMode = 1,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect maneger",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionDef testInternalPermDef = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .bundleName = "net_conn_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect manager internal",
    .descriptionId = 1,
};

PermissionDef testInternetPermDef = {
    .permissionName = "ohos.permission.INTERNET",
    .bundleName = "net_conn_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect manager internet",
    .descriptionId = 1,
};

PermissionStateFull testState = {
    .grantFlags = {2},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .isGeneral = true,
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .resDeviceID = {"local"},
};

PermissionStateFull testInternalState = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

PermissionStateFull testInternetState = {
    .permissionName = "ohos.permission.INTERNET",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef, testInternalPermDef, testInternetPermDef},
    .permStateList = {testState, testInternalState, testInternetState},
};
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
class AccessToken {
public:
    AccessToken() : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(tokenIdEx.tokenIDEx);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
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
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(handle);
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
    AccessToken token;
    NetHandle handle;
    int32_t netId = 0;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(handle);
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
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(bFlag);
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
    AccessToken token;
    bool bFlag = false;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(bFlag);
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
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(handle);
    ASSERT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);

    NetAllCapabilities netAllCap;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(handle, netAllCap);
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
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(handle);
    ASSERT_TRUE(ret == NETMANAGER_ERR_PERMISSION_DENIED);

    AccessToken token;
    NetAllCapabilities netAllCap;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(handle, netAllCap);
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
    AccessToken token;
    NetHandle handle;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(handle);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

    NetAllCapabilities netAllCap;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(handle, netAllCap);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS || ret == NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: SetAirplaneModeTest001
 * @tc.desc: Test NetConnClient::SetAirplaneMode
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAirplaneModeTest001, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(true);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetAirplaneModeTest002
 * @tc.desc: Test NetConnClient::SetAirplaneMode
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAirplaneModeTest002, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(false);
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
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->IsDefaultNetMetered(bRes);
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
    AccessToken token;
    bool bRes = false;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->IsDefaultNetMetered(bRes);
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
    AccessToken token;
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest002
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN1, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest003
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest003, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN2, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest004
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest004, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN3, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest005
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest005, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN4, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest006
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest006, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN5, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest007
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest007, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN6, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest008
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest008, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN7, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest09
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest09, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN8, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest10
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest10, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN9, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest11
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest11, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN10, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest012
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest012, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest013
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest013, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest14
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest14, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_LONG_HOST, 8080, {TEST_LONG_EXCLUSION_LIST}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest015
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest015, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
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
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
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
    AccessToken token;
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
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
    AccessToken token;
    HttpProxy httpProxy = {TEST_DOMAIN2, 8080, {}};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
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
    AccessToken token;
    HttpProxy validHttpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
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
    AccessToken token;
    HttpProxy httpProxy;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
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
    AccessToken token;
    HttpProxy validHttpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy defaultHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultHttpProxy(defaultHttpProxy);
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
    AccessToken token;
    HttpProxy globalHttpProxy;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(globalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy defaultHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: GetDefaultHttpProxyTest003
 * @tc.desc: Test NetConnClient::SetAppNet and NetConnClient::GetDefaultHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetDefaultHttpProxyTest003, TestSize.Level1)
{
    AccessToken token;
    int32_t netId = 102;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    HttpProxy defaultHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultHttpProxy(defaultHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    int32_t cancelNetId = 0;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultHttpProxy(defaultHttpProxy);
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
    auto ret =
        DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(netBearType, ident, netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetSupplier002
 * @tc.desc: Test NetConnClient::RegisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplier002, TestSize.Level1)
{
    AccessToken token;
    uint32_t supplierId = 100;
    NetBearType netBearType = BEARER_WIFI;
    const std::string ident = "";
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    auto ret =
        DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(netBearType, ident, netCaps, supplierId);
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
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetSupplier(supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: UnregisterNetSupplier002
 * @tc.desc: Test NetConnClient::UnregisterNetSupplier
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, UnregisterNetSupplier002, TestSize.Level1)
{
    AccessToken token;
    uint32_t supplierId = 100;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetSupplier(supplierId);
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
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest002, TestSize.Level1)
{
    AccessToken token;
    uint32_t supplierId = 100;
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest003
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest003, TestSize.Level1)
{
    AccessToken token;
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET};
    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result =
        DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    ASSERT_TRUE(result == NETMANAGER_SUCCESS);
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBase();
    ASSERT_NE(callback, nullptr);
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest004
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest004, TestSize.Level1)
{
    AccessToken token;
    uint32_t supplierId = 0;
    sptr<NetSupplierCallbackBase> callback;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: SetAppNetTest001
 * @tc.desc: Test NetConnClient::SetAppNet, if param is invalid, SetAppNet return NET_CONN_ERR_INVALID_NETWORK
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAppNetTest001, TestSize.Level1)
{
    AccessToken token;
    int32_t netId = 99;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(netId);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

/**
 * @tc.name: SetAppNetTest002
 * @tc.desc: Test NetConnClient::SetAppNet, if param is valid, SetAppNet return NETMANAGER_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAppNetTest002, TestSize.Level1)
{
    AccessToken token;
    int32_t netId = 102;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t cancelNetId = 0;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(cancelNetId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAppNetTest001
 * @tc.desc: Test NetConnClient::GetAppNet, return NetId set by SetAppNet
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAppNetTest001, TestSize.Level1)
{
    AccessToken token;
    int32_t netId = 102;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    int32_t getNetId = 0;
    DelayedSingleton<NetConnClient>::GetInstance()->GetAppNet(getNetId);
    EXPECT_EQ(getNetId, netId);

    int32_t cancelNetId = 0;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(cancelNetId);
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
    AccessToken token;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
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
    auto ret =
        DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback, timesOut);
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
    auto ret =
        DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback, timesOut);
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
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
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
    auto client = DelayedSingleton<NetConnClient>::GetInstance();
    uint32_t supplierId = 1;
    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo;
    int32_t ret = client->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
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
    AccessToken token;
    auto client = DelayedSingleton<NetConnClient>::GetInstance();
    uint32_t supplierId = 1;
    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo;
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    int32_t ret = client->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: GetNetInterfaceConfigurationTest001
 * @tc.desc: Test NetConnClient::GetNetInterfaceConfiguration
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetNetInterfaceConfigurationTest001, TestSize.Level1)
{
    AccessToken token;
    NetInterfaceConfiguration config;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetNetInterfaceConfiguration(TEST_IFACE, config);
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
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetNetInterfaceConfiguration(TEST_IFACE, config);
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
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: RegisterNetInterfaceCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetInterfaceCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetInterfaceCallbackTest002, TestSize.Level1)
{
    AccessToken token;
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SystemReadyTest002
 * @tc.desc: Test NetConnClient::SystemReady
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SystemReadyTest002, TestSize.Level1)
{
    AccessToken token;
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
    AccessToken token;
    uint32_t supplierId = 1;
    sptr<NetLinkInfo> netLinkInfo = std::make_unique<NetLinkInfo>().release();
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: GetAllNetsTest002
 * @tc.desc: Test NetConnClient::GetAllNets
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAllNetsTest002, TestSize.Level1)
{
    AccessToken token;
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
    AccessToken token;
    NetHandle netHandle;
    NetLinkInfo info;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(netHandle, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAddressesByNameTest002
 * @tc.desc: Test NetConnClient::GetAddressesByName
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAddressesByNameTest002, TestSize.Level1)
{
    AccessToken token;
    std::string host = "ipaddr";
    int32_t netId = 1;
    std::vector<INetAddr> addrList;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAddressesByName(host, netId,addrList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAddressByNameTest002
 * @tc.desc: Test NetConnClient::GetAddressByName
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetAddressByNameTest002, TestSize.Level1)
{
    AccessToken token;
    std::string host = "ipaddr";
    int32_t netId = 1;
    INetAddr addr;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAddressByName(host, netId,addr);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: BindSocketTest002
 * @tc.desc: Test NetConnClient::BindSocket
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, BindSocketTest002, TestSize.Level1)
{
    AccessToken token;
    int32_t socket_fd = 0;
    int32_t netId =99;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socket_fd, netId);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
    netId =101;
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
    AccessToken token;
    NetHandle netHandle;
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(netHandle);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
