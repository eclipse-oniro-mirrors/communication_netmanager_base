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
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

HapInfoParams testInfoParms = {.bundleName = "net_conn_service_test", .userID = 1, .instIndex = 0, .appIDDesc = "test"};

PermissionDef testPermDef = {.permissionName = "ohos.permission.GET_NETWORK_INFO",
                             .bundleName = "net_conn_service_test",
                             .grantMode = 1,
                             .label = "label",
                             .labelId = 1,
                             .description = "Test net connect maneger",
                             .descriptionId = 1,
                             .availableLevel = APL_SYSTEM_BASIC};

PermissionStateFull testState = {.grantFlags = {2},
                                 .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                 .isGeneral = true,
                                 .permissionName = "ohos.permission.GET_NETWORK_INFO",
                                 .resDeviceID = {"local"}};

PermissionDef testPermDef2 = {.permissionName = "ohos.permission.INTERNET",
                              .bundleName = "net_conn_service_test",
                              .grantMode = 1,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net connect maneger",
                              .descriptionId = 1,
                              .availableLevel = APL_SYSTEM_BASIC};

PermissionStateFull testState2 = {.grantFlags = {2},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .isGeneral = true,
                                  .permissionName = "ohos.permission.INTERNET",
                                  .resDeviceID = {"local"}};
PermissionDef testPermDef3 = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                              .bundleName = "net_conn_service_test",
                              .grantMode = 1,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net connect maneger",
                              .descriptionId = 1,
                              .availableLevel = APL_SYSTEM_BASIC};

PermissionStateFull testState3 = {.grantFlags = {2},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .isGeneral = true,
                                  .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                  .resDeviceID = {"local"}};

HapPolicyParams testPolicyPrams = {.apl = APL_SYSTEM_BASIC,
                                   .domain = "test.domain",
                                   .permList = {testPermDef, testPermDef2, testPermDef3},
                                   .permStateList = {testState, testState2, testState3}};

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

class AccessToken {
public:
    AccessToken()
    {
        currentID_ = GetSelfTokenID();
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
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};

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
    sptr<INetConnCallback> callback = new (std::nothrow) NetConnTestCallback();
    ASSERT_NE(callback, nullptr);
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->RegisterNetConnCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, RegisterNetConnCallbackTest002, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->RegisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, RegisterNetConnCallbackTest004, TestSize.Level1)
{
    AccessToken token;
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    ASSERT_NE(netSpecifier, nullptr);
    sptr<INetConnCallback> callback = new (std::nothrow) NetConnTestCallback();
    ASSERT_NE(callback, nullptr);
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback,
                                                                                        TEST_TIMEOUTMS);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, UnregisterNetConnCallbackTest002, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->UnregisterNetConnCallback(g_callback);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetAllNetsTest002, TestSize.Level1)
{
    AccessToken token;
    std::list<int32_t> netIdList;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetAllNets(netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetConnectionPropertiesTest002, TestSize.Level1)
{  
    AccessToken token;
    NetLinkInfo info;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetConnectionProperties(TEST_NETID, info);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceTest, GetAddressesByNameTest002, TestSize.Level1)
{
    AccessToken token;
    std::vector<INetAddr> addrList;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetAddressesByName(TEST_HOST, TEST_NETID, addrList);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetAddressByNameTest002, TestSize.Level1)
{
    AccessToken token;
    INetAddr addr;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetAddressByName(TEST_HOST, TEST_NETID, addr);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, BindSocketTest001, TestSize.Level1)
{
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->BindSocket(TEST_SOCKETFD, TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, NetDetectionTest002, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->NetDetection(TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, GetNetIdByIdentifierTest001, TestSize.Level1)
{
    int32_t netId = 0;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetNetIdByIdentifier(TEST_IDENT, netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetDefaultNetTest002, TestSize.Level1)
{
    AccessToken token;
    int32_t netId = 0;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, HasDefaultNetTest002, TestSize.Level1)
{
    AccessToken token;
    bool bFlag = false;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->HasDefaultNet(bFlag);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetNetCapabilitiesTest003, TestSize.Level1)
{
    NETMGR_LOG_D("GetNetCapabilitiesTest003 In");
    AccessToken token;
    int32_t netId = 0;
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->GetDefaultNet(netId);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);

    NetAllCapabilities netAllCap;
    ret = DelayedSingleton<NetConnService>::GetInstance()->GetNetCapabilities(netId, netAllCap);
    ASSERT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceTest, SetAirplaneModeTest001, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetAirplaneMode(true);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, SetAirplaneModeTest002, TestSize.Level1)
{
    AccessToken token;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetAirplaneMode(false);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, IsDefaultNetMeteredTest002, TestSize.Level1)
{
    AccessToken token;
    bool bRes = false;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->IsDefaultNetMetered(bRes);
    ASSERT_EQ(ret, NETMANAGER_SUCCESS);
    ASSERT_TRUE(bRes);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest001, TestSize.Level1)
{
    AccessToken token;
    HttpProxy httpProxy = {"testHttpProxy", 0, {}};
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceTest, SetGlobalHttpProxyTest002, TestSize.Level1)
{
    HttpProxy httpProxy;
    auto ret = DelayedSingleton<NetConnService>::GetInstance()->SetGlobalHttpProxy(httpProxy);
    ASSERT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    AccessToken token;
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
