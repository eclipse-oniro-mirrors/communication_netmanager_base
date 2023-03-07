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
#include "message_parcel.h"
#include "network.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_types.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

HapInfoParams testInfoParms = {.bundleName = "net_conn_manager_test",
                               .userID = 1,
                               .instIndex = 0,
                               .appIDDesc = "test"};

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

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef, testInternalPermDef},
    .permStateList = {testState, testInternalState},
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
        SetSelfTokenID(accessID_);
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
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
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
 * @tc.name: SetAirplaneModeTest
 * @tc.desc: Test NetConnClient::SetAirplaneMode
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetAirplaneModeTest, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(true);
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
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ASSERT_EQ(bRes, true);
}

/**
 * @tc.name: SetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy,if param is not null,SetGlobalHttpProxy return NET_CONN_SUCCESS
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
 * @tc.desc: Test NetConnClient::SetGlobalHttpProxy.if param is null, return NET_CONN_ERR_INTERNAL_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy;
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnClient::GetHttpProxy
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

    HttpProxy getGlobalHttpProxy;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(getGlobalHttpProxy);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
    ASSERT_TRUE(getGlobalHttpProxy.GetHost() == "testHttpProxy");
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest001
 * @tc.desc: Test NetConnClient::RegisterNetSupplierCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnClientTest, RegisterNetSupplierCallbackTest001, TestSize.Level1)
{
    uint32_t supplierId = 100;
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBaseTest();
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
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBaseTest();
    ASSERT_NE(callback, nullptr);
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplierCallback(supplierId, callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
