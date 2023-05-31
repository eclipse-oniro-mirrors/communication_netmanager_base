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
#include "network.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_types.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
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
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
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
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy,if param is invalid,SetGlobalHttpProxy return
 * NET_CONN_ERR_HTTP_PROXY_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest001, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_ERR_HTTP_PROXY_INVALID);
}

/**
 * @tc.name: SetGlobalHttpProxyTest002
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.if param is valid, return NET_CONN_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_IPV4_ADDR, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest003
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.if param is valid, return NET_CONN_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest003, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {TEST_IPV6_ADDR, 8080, {}};
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest004
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.if param is null, return NET_CONN_SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest004, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
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
    HttpProxy validHttpProxy = {TEST_IPV4_ADDR, 8080, {}};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(validHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);

    HttpProxy invalidHttpProxy = {"testHttpProxy", 0, {}};
    ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(invalidHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_ERR_HTTP_PROXY_INVALID);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NET_CONN_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == TEST_IPV4_ADDR);
}

/**
 * @tc.name: GetGlobalHttpProxyTest004
 * @tc.desc: Test NetConnClient::GetGlobalHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest004, TestSize.Level1)
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
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest002
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest002, TestSize.Level1)
{
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
} // namespace NetManagerStandard
} // namespace OHOS
