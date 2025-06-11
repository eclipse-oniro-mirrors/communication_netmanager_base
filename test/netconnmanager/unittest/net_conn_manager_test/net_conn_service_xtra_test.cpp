/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "common_net_conn_callback_test.h"
#include "http_proxy.h"
#include "ipc_skeleton.h"
#include "net_all_capabilities.h"
#include "net_conn_callback_stub.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_service.h"
#include "net_conn_types.h"
#include "net_detection_callback_test.h"
#include "net_factoryreset_callback_stub.h"
#include "net_http_proxy_tracker.h"
#include "net_interface_callback_stub.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_test_security.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"
#include "common_mock_net_remote_object_test.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t TEST_TIMEOUTMS = 1000;
constexpr int32_t TEST_NETID = 3;
constexpr int32_t TEST_SOCKETFD = 2;
const int32_t NET_ID = 2;
const int32_t SOCKET_FD = 2;
const int32_t ZERO_VALUE = 0;
const int32_t INVALID_VALUE = 10;
constexpr const char *TEST_IDENT = "testIdent";
constexpr const char *TEST_HOST = "testHost";
constexpr const char *TEST_PROXY_HOST = "testHttpProxy";
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
    "0123456789qwertyuiopasdfghjklzxcvbnm[]:;<>?!@#$%^&()AEFFEqwdqwrtfasfj4897qwe465791qwr87tq4fq7t8qt4654qwr";
constexpr const char *TEST_LONG_EXCLUSION_LIST =
    "www.test0.com,www.test1.com,www.test2.com,www.test3.com,www.test4.com,www.test5.com,www.test6.com,www.test7.com,"
    "www.test8.com,www.test9.com,www.test10.com,www.test11.com,www.test12.com,www.test12.com,www.test12.com,www.test13."
    "com,www.test14.com,www.test15.com,www.test16.com,www.test17.com,www.test18.com,www.test19.com,www.test20.com";
constexpr const char *NET_CONN_MANAGER_WORK_THREAD = "NET_CONN_MANAGER_WORK_THREAD";
constexpr int64_t TEST_UID = 1010;
constexpr uint32_t TEST_NOTEXISTSUPPLIER = 1000;
constexpr int32_t MAIN_USERID = 100;
constexpr int32_t INVALID_USERID = 1;

sptr<INetConnCallback> g_callback = new (std::nothrow) NetConnCallbackStubCb();
sptr<INetDetectionCallback> g_detectionCallback = new (std::nothrow) NetDetectionCallbackTest();
uint32_t g_supplierId = 0;
uint32_t g_vpnSupplierId = 0;
} // namespace

class NetConnServiceExtTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetConnServiceExtTest::SetUpTestCase()
{
    NetConnService::GetInstance()->OnStart();
    if (NetConnService::GetInstance()->state_ != NetConnService::STATE_RUNNING) {
        NetConnService::GetInstance()->netConnEventRunner_ =
            AppExecFwk::EventRunner::Create(NET_CONN_MANAGER_WORK_THREAD);
        ASSERT_NE(NetConnService::GetInstance()->netConnEventRunner_, nullptr);
        NetConnService::GetInstance()->netConnEventHandler_ =
            std::make_shared<NetConnEventHandler>(NetConnService::GetInstance()->netConnEventRunner_);
        NetConnService::GetInstance()->serviceIface_ = std::make_unique<NetConnServiceIface>().release();
        NetManagerCenter::GetInstance().RegisterConnService(NetConnService::GetInstance()->serviceIface_);
        NetHttpProxyTracker httpProxyTracker;
        HttpProxy httpProxy;
        httpProxy.SetPort(0);
        httpProxyTracker.ReadFromSettingsData(httpProxy);
        NetConnService::GetInstance()->SendHttpProxyChangeBroadcast(httpProxy);
    }
}

void NetConnServiceExtTest::TearDownTestCase() {}

void NetConnServiceExtTest::SetUp() {}

void NetConnServiceExtTest::TearDown() {}

HWTEST_F(NetConnServiceExtTest, CheckIfSettingsDataReadyTest001, TestSize.Level1)
{
    NetConnService::GetInstance()->isDataShareReady_ = true;
    auto ret = NetConnService::GetInstance()->CheckIfSettingsDataReady();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetConnServiceExtTest, CheckIfSettingsDataReadyTest002, TestSize.Level1)
{
    NetConnService::GetInstance()->isDataShareReady_ = false;
    auto ret = NetConnService::GetInstance()->CheckIfSettingsDataReady();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetConnServiceExtTest, OnNetSupplierRemoteDiedTest001, TestSize.Level1)
{
    wptr<IRemoteObject> remoteObject = nullptr;
    NetConnService::GetInstance()->netConnEventHandler_ = nullptr;
    EXPECT_FALSE(NetConnService::GetInstance()->registerToService_);
    NetConnService::GetInstance()->OnNetSupplierRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, OnNetSupplierRemoteDiedTest002, TestSize.Level1)
{
    wptr<IRemoteObject> remoteObject = new MockNetIRemoteObject();
    EXPECT_NE(remoteObject, nullptr);
    NetConnService::GetInstance()->netConnEventHandler_ = nullptr;
    NetConnService::GetInstance()->OnNetSupplierRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, OnNetSupplierRemoteDiedTest003, TestSize.Level1)
{
    wptr<IRemoteObject> remoteObject = new MockNetIRemoteObject();
    EXPECT_NE(remoteObject, nullptr);
    NetConnService::GetInstance()->netConnEventHandler_ = nullptr;
    NetConnService::GetInstance()->OnNetSupplierRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, RemoveNetSupplierDeathRecipientTest002, TestSize.Level1)
{
    sptr<INetSupplierCallback> callback = nullptr;
    EXPECT_FALSE(NetConnService::GetInstance()->registerToService_);
    NetConnService::GetInstance()->RemoveNetSupplierDeathRecipient(callback);
}

HWTEST_F(NetConnServiceExtTest, RequestNetConnectionAsyncTest002, TestSize.Level1)
{
    uint32_t callingUid = 1;
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    sptr<INetConnCallback> uidCallback = nullptr;
    int32_t ret = NetConnService::GetInstance()->RequestNetConnectionAsync(netSpecifier, uidCallback, 0, callingUid);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnServiceExtTest, UnregisterNetSupplierAsyncTest001, TestSize.Level1)
{
    uint32_t supplierId = 1;
    int32_t callingUid = 1;
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> netSupplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    netConnService->netSuppliers_[supplierId] = netSupplier;
    auto result = netConnService->FindNetSupplier(supplierId);
    EXPECT_EQ(result, netSupplier);
    netConnService->defaultNetSupplier_ = netSupplier;
    bool ignoreUid = true;
    auto ret = netConnService->UnregisterNetSupplierAsync(supplierId, ignoreUid, callingUid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, UnregisterNetSupplierAsyncTest002, TestSize.Level1)
{
    uint32_t supplierId = 1;
    int32_t callingUid = 1;
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> netSupplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    netConnService->netSuppliers_[supplierId] = netSupplier;
    auto result = netConnService->FindNetSupplier(supplierId);
    EXPECT_EQ(result, netSupplier);
    bool ignoreUid = false;
    netConnService->defaultNetSupplier_ = nullptr;
    EXPECT_EQ(netConnService->defaultNetSupplier_, nullptr);
    auto ret = netConnService->UnregisterNetSupplierAsync(supplierId, ignoreUid, callingUid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, CheckAndCompareUidTest001, TestSize.Level1)
{
    uint32_t supplierId = 1;
    int32_t callingUid = 2;
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> netSupplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    netConnService->netSuppliers_[supplierId] = netSupplier;
    auto supplier = netConnService->FindNetSupplier(supplierId);
    EXPECT_EQ(supplier, netSupplier);
    int32_t uid = netSupplier->GetUid();
    EXPECT_NE(uid, callingUid);
    netConnService->CheckAndCompareUid(supplier, callingUid);
}

HWTEST_F(NetConnServiceExtTest, HandleScreenEventTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->defaultNetSupplier_ = nullptr;
    EXPECT_EQ(netConnService->defaultNetSupplier_, nullptr);
    netConnService->HandleScreenEvent(true);
}

HWTEST_F(NetConnServiceExtTest, UpdateNetCapsAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::set<NetCap> netCaps;
    uint32_t supplierId = 0;
    EXPECT_EQ(netConnService->defaultNetSupplier_, nullptr);
    auto supplier = netConnService->FindNetSupplier(supplierId);
    EXPECT_EQ(supplier, nullptr);
    auto ret = netConnService->UpdateNetCapsAsync(netCaps, supplierId);
    EXPECT_EQ(ret, NET_CONN_ERR_NO_SUPPLIER);
}

HWTEST_F(NetConnServiceExtTest, UpdateNetCapsAsyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::set<NetCap> netCaps;
    uint32_t supplierId = 1;
    std::string netSupplierIdent;
    sptr<NetSupplier> netSupplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    netConnService->netSuppliers_[supplierId] = netSupplier;
    auto result = netConnService->FindNetSupplier(supplierId);
    EXPECT_EQ(result, netSupplier);
    auto supplier = netConnService->FindNetSupplier(supplierId);
    EXPECT_EQ(supplier, netSupplier);
    auto network = supplier->GetNetwork();
    EXPECT_EQ(network, nullptr);
    auto ret = netConnService->UpdateNetCapsAsync(netCaps, supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnServiceExtTest, NetDetectionForDnsHealthSyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    int32_t netId = 0;
    auto iterNetwork = netConnService->networks_.find(netId);
    EXPECT_EQ(iterNetwork, netConnService->networks_.end());
    auto ret = netConnService->NetDetectionForDnsHealthSync(netId, true);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);
}

HWTEST_F(NetConnServiceExtTest, NetDetectionForDnsHealthSyncTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    int32_t netId = 1;
    netConnService->networks_[netId] = nullptr;
    auto iterNetwork = netConnService->networks_.find(netId);
    EXPECT_NE(iterNetwork, netConnService->networks_.end());
    auto ret = netConnService->NetDetectionForDnsHealthSync(netId, true);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);
}

HWTEST_F(NetConnServiceExtTest, RestrictBackgroundChangedAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netSuppliers_.clear();
    netConnService->netSuppliers_[0] = nullptr;
    auto ret = netConnService->RestrictBackgroundChangedAsync(true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, RequestAllNetworkExceptDefaultTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->defaultNetSupplier_ = nullptr;
    EXPECT_FALSE(NetConnService::GetInstance()->registerToService_);
    netConnService->RequestAllNetworkExceptDefault();
}

HWTEST_F(NetConnServiceExtTest, RequestAllNetworkExceptDefaultTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    netConnService->defaultNetSupplier_ = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_NE(netConnService->defaultNetSupplier_, nullptr);
    netConnService->RequestAllNetworkExceptDefault();
}

HWTEST_F(NetConnServiceExtTest, GenerateNetIdTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netIdLastValue_ = MAX_NET_ID;
    netConnService->defaultNetSupplier_ = nullptr;
    EXPECT_EQ(netConnService->defaultNetSupplier_, nullptr);
    auto ret = netConnService->GenerateNetId();
    EXPECT_EQ(ret, MIN_NET_ID);
}

HWTEST_F(NetConnServiceExtTest, GenerateInternalNetIdTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->internalNetIdLastValue_ = MAX_NET_ID;
    auto ret = netConnService->GenerateInternalNetId();
    EXPECT_NE(ret, MIN_INTERNAL_NET_ID);
}

HWTEST_F(NetConnServiceExtTest, NotFindBestSupplierTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    netConnService->NotFindBestSupplier(1, nullptr, nullptr, nullptr);
}

HWTEST_F(NetConnServiceExtTest, NotFindBestSupplierTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_NE(supplier, nullptr);
    netConnService->NotFindBestSupplier(1, nullptr, supplier, nullptr);
}

HWTEST_F(NetConnServiceExtTest, NotFindBestSupplierTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_NE(supplier, nullptr);
    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    netConnService->NotFindBestSupplier(1, nullptr, supplier, callback);
}

HWTEST_F(NetConnServiceExtTest, HandleCallbackTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    CallbackType type = CALL_TYPE_UPDATE_LINK;
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_EQ(supplier->network_, nullptr);
    sptr<NetHandle> netHandle = nullptr;
    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    netConnService->HandleCallback(supplier, netHandle, callback, type);
}

HWTEST_F(NetConnServiceExtTest, HandleCallbackTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    CallbackType type = CALL_TYPE_UNAVAILABLE;
    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    EXPECT_NE(callback, nullptr);
    sptr<NetSupplier> supplier = nullptr;
    sptr<NetHandle> netHandle = nullptr;
    netConnService->HandleCallback(supplier, netHandle, callback, type);
}

HWTEST_F(NetConnServiceExtTest, CallbackForAvailableTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_EQ(supplier->network_, nullptr);
    netConnService->CallbackForAvailable(supplier, nullptr);
}

HWTEST_F(NetConnServiceExtTest, CallbackForAvailableTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_EQ(supplier->network_, nullptr);
    sptr<INetConnCallback> callback = new (std::nothrow) NetConnCallbackStubCb();
    netConnService->CallbackForAvailable(supplier, callback);
}

HWTEST_F(NetConnServiceExtTest, MakeDefaultNetWorkTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> oldSupplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_EQ(oldSupplier->network_, nullptr);
    sptr<NetSupplier> newSupplier = nullptr;
    netConnService->MakeDefaultNetWork(oldSupplier, newSupplier);
}

HWTEST_F(NetConnServiceExtTest, GetNetSupplierFromListTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netSuppliers_[1] = nullptr;
    std::string ident;
    auto ret = netConnService->GetNetSupplierFromList(BEARER_CELLULAR, ident);
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(NetConnServiceExtTest, GetNetSupplierFromListTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netSuppliers_[1] = nullptr;
    std::string ident;
    std::set<NetCap> netCaps;
    auto ret = netConnService->GetNetSupplierFromList(BEARER_CELLULAR, ident, netCaps);
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(NetConnServiceExtTest, GetSpecificNetTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    NetBearType bearerType = static_cast<NetBearType>(-1);
    std::list<int32_t> netIdList;
    auto ret = netConnService->GetSpecificNet(bearerType, netIdList);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_TYPE_NOT_FOUND);
}

HWTEST_F(NetConnServiceExtTest, GetSpecificNetTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netSuppliers_[1] = nullptr;
    NetBearType bearerType = BEARER_CELLULAR;
    std::list<int32_t> netIdList;
    auto ret = netConnService->GetSpecificNet(bearerType, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, GetSpecificNetByIdentTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    NetBearType bearerType = static_cast<NetBearType>(-1);
    std::string ident;
    std::list<int32_t> netIdList;
    auto ret = netConnService->GetSpecificNetByIdent(bearerType, ident, netIdList);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_TYPE_NOT_FOUND);
}

HWTEST_F(NetConnServiceExtTest, GetSpecificNetByIdentTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netSuppliers_[1] = nullptr;
    NetBearType bearerType = BEARER_CELLULAR;
    std::string ident;
    std::list<int32_t> netIdList;
    auto ret = netConnService->GetSpecificNetByIdent(bearerType, ident, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, GetAllNetsTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_EQ(netConnService->netConnEventHandler_, nullptr);
    std::list<int32_t> netIdList;
    auto ret = netConnService->GetAllNets(netIdList);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, GetConnectionPropertiesTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_EQ(netConnService->netConnEventHandler_, nullptr);
    NetLinkInfo info;
    auto ret = netConnService->GetConnectionProperties(0, info);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnServiceExtTest, GetIfaceNamesTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::list<std::string> ifaceNames;
    auto ret = netConnService->GetIfaceNames(static_cast<NetBearType>(-1), ifaceNames);
    EXPECT_EQ(ret, NET_CONN_ERR_NET_TYPE_NOT_FOUND);
}

HWTEST_F(NetConnServiceExtTest, GetNetIdByIdentifierTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::list<int32_t> netIdList;
    netConnService->netSuppliers_[0] = nullptr;
    auto ret = netConnService->GetNetIdByIdentifier(TEST_IDENT, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, GetDumpMessageTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    netConnService->defaultNetSupplier_ = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_NE(netConnService->defaultNetSupplier_, nullptr);
    EXPECT_EQ(netConnService->defaultNetSupplier_->network_, nullptr);
    netConnService->dnsResultCallback_ = new NetDnsResultCallback();
    std::string message;
    netConnService->GetDumpMessage(message);
}

HWTEST_F(NetConnServiceExtTest, IsValidDecValueTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string inputValue = "inputValue test";
    auto ret = netConnService->IsValidDecValue(inputValue);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetConnServiceExtTest, IsValidDecValueTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string inputValue = "input";
    auto ret = netConnService->IsValidDecValue(inputValue);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetConnServiceExtTest, SetAirplaneModeTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->preAirplaneCallbacks_.empty());
    netConnService->preAirplaneCallbacks_[0] = new IPreAirplaneCallbackStubTestCb();
    netConnService->preAirplaneCallbacks_[1] = nullptr;
    auto ret = netConnService->SetAirplaneMode(true);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetConnServiceExtTest, SetCurlOptionsTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    HttpProxy tempProxy;
    netConnService->SetCurlOptions(nullptr, tempProxy);
}

HWTEST_F(NetConnServiceExtTest, GetHttpUrlFromConfigTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    std::string httpUrl;
    netConnService->GetHttpUrlFromConfig(httpUrl);
}

HWTEST_F(NetConnServiceExtTest, SetGlobalHttpProxyOldTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    HttpProxy httpProxy;
    int32_t activeUserId = 1;
    netConnService->currentUserId_ = 0;
    auto ret = netConnService->SetGlobalHttpProxyOld(httpProxy, activeUserId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetGlobalHttpProxyOldTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    HttpProxy httpProxy;
    httpProxy.SetHost("127.0.0.1");
    int32_t activeUserId = 1;
    netConnService->currentUserId_ = 0;
    auto ret = netConnService->SetGlobalHttpProxyOld(httpProxy, activeUserId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, IsValidUserIdTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    auto ret = netConnService->IsValidUserId(-1);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetConnServiceExtTest, GetValidUserIdFromProxyTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    HttpProxy httpProxy;
    httpProxy.SetUserId(NetConnService::ROOT_USER_ID);
    netConnService->GetValidUserIdFromProxy(httpProxy);
}

HWTEST_F(NetConnServiceExtTest, GetValidUserIdFromProxyTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    HttpProxy httpProxy;
    httpProxy.SetUserId(NetConnService::INVALID_USER_ID);
    netConnService->GetValidUserIdFromProxy(httpProxy);
}

HWTEST_F(NetConnServiceExtTest, NetDetectionForDnsHealthTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_NE(netConnService->netConnEventRunner_, nullptr);
    netConnService->netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnService->netConnEventRunner_);
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    auto ret = netConnService->NetDetectionForDnsHealth(1, true);
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, NetDetectionForDnsHealthTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    netConnService->netConnEventHandler_ = nullptr;
    auto ret = netConnService->NetDetectionForDnsHealth(1, true);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, LoadGlobalHttpProxyTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    NetConnService::UserIdType userIdType = NetConnService::UserIdType::LOCAL;
    HttpProxy httpProxy;
    netConnService->LoadGlobalHttpProxy(userIdType, httpProxy);
}

HWTEST_F(NetConnServiceExtTest, LoadGlobalHttpProxyTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    NetConnService::UserIdType userIdType = static_cast<NetConnService::UserIdType>(-1);
    HttpProxy httpProxy;
    netConnService->LoadGlobalHttpProxy(userIdType, httpProxy);
}

HWTEST_F(NetConnServiceExtTest, LoadGlobalHttpProxyTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    NetConnService::UserIdType userIdType = NetConnService::UserIdType::SPECIFY;
    HttpProxy httpProxy;
    httpProxy.SetUserId(1);
    netConnService->LoadGlobalHttpProxy(userIdType, httpProxy);
}

HWTEST_F(NetConnServiceExtTest, UpdateGlobalHttpProxyTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    HttpProxy httpProxy;
    netConnService->netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnService->netConnEventRunner_);
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    netConnService->UpdateGlobalHttpProxy(httpProxy);
}

HWTEST_F(NetConnServiceExtTest, OnInterfaceAddressUpdatedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    std::string testString = "test";
    int testInt = 0;
    auto ret = stateCallback.OnInterfaceAddressUpdated(testString, testString, testInt, testInt);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, OnInterfaceAddressRemovedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    std::string testString = "test";
    int testInt = 0;
    auto ret = stateCallback.OnInterfaceAddressRemoved(testString, testString, testInt, testInt);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, OnInterfaceAddedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    std::string testString = "test";
    auto ret = stateCallback.OnInterfaceAdded(testString);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, OnInterfaceRemovedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    std::string testString = "test";
    auto ret = stateCallback.OnInterfaceRemoved(testString);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, OnInterfaceChangedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    std::string testString = "test";
    auto ret = stateCallback.OnInterfaceChanged(testString, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, OnInterfaceLinkStateChangedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    std::string testString = "test";
    auto ret = stateCallback.OnInterfaceLinkStateChanged(testString, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, OnRouteChangedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    EXPECT_EQ(stateCallback.ifaceStateCallbacks_.size(), 2);
    std::string testString = "test";
    auto ret = stateCallback.OnRouteChanged(false, testString, testString, testString);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, RegisterInterfaceCallbackTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    EXPECT_NE(callback, nullptr);
    auto ret = stateCallback.RegisterInterfaceCallback(callback);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, RegisterInterfaceCallbackTest002, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    stateCallback.ifaceStateCallbacks_.push_back(nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    stateCallback.ifaceStateCallbacks_.push_back(callback);
    EXPECT_EQ(stateCallback.ifaceStateCallbacks_.size(), 2);
    std::string testString = "test";
    int testInt = 0;
    auto ret = stateCallback.RegisterInterfaceCallback(callback);
    EXPECT_EQ(ret, NET_CONN_ERR_SAME_CALLBACK);
}

HWTEST_F(NetConnServiceExtTest, OnNetIfaceStateRemoteDiedTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    EXPECT_TRUE(stateCallback.ifaceStateCallbacks_.empty());
    wptr<IRemoteObject> remoteObject = nullptr;
    stateCallback.OnNetIfaceStateRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, OnNetIfaceStateRemoteDiedTest002, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    wptr<IRemoteObject> remoteObject = new MockNetIRemoteObject();
    EXPECT_NE(remoteObject, nullptr);
    stateCallback.OnNetIfaceStateRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, AddIfaceDeathRecipientTest001, TestSize.Level1)
{
    NetConnService::NetInterfaceStateCallback stateCallback;
    stateCallback.netIfaceStateDeathRecipient_ = new (std::nothrow)
        NetConnService::NetInterfaceStateCallback::NetIfaceStateCallbackDeathRecipient(stateCallback);
    EXPECT_NE(stateCallback.netIfaceStateDeathRecipient_, nullptr);
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackStub();
    EXPECT_NE(callback, nullptr);
    stateCallback.AddIfaceDeathRecipient(callback);
}

HWTEST_F(NetConnServiceExtTest, NetUidPolicyChangeTest001, TestSize.Level1)
{
    std::weak_ptr<NetConnService> netConnService;
    netConnService.reset();
    NetConnService::NetPolicyCallback policyCallback(netConnService);
    auto ret = policyCallback.NetUidPolicyChange(1, 1);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, NetUidPolicyChangeTest002, TestSize.Level1)
{
    NetConnService::NetPolicyCallback policyCallback(NetConnService::GetInstance());
    EXPECT_NE(NetConnService::GetInstance()->defaultNetSupplier_, nullptr);
    EXPECT_NE(NetConnService::GetInstance()->netConnEventHandler_, nullptr);
    auto ret = policyCallback.NetUidPolicyChange(1, 1);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, NetUidPolicyChangeTest003, TestSize.Level1)
{
    NetConnService::NetPolicyCallback policyCallback(NetConnService::GetInstance());
    EXPECT_NE(NetConnService::GetInstance()->defaultNetSupplier_, nullptr);
    NetConnService::GetInstance()->netConnEventHandler_ = nullptr;
    auto ret = policyCallback.NetUidPolicyChange(1, 1);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, NetUidPolicyChangeTest004, TestSize.Level1)
{
    NetConnService::NetPolicyCallback policyCallback(NetConnService::GetInstance());
    EXPECT_NE(NetConnService::GetInstance()->defaultNetSupplier_, nullptr);
    NetConnService::GetInstance()->defaultNetSupplier_ = nullptr;
    auto ret = policyCallback.NetUidPolicyChange(1, 1);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, SendNetPolicyChangeTest001, TestSize.Level1)
{
    std::weak_ptr<NetConnService> netConnService;
    netConnService.reset();
    NetConnService::NetPolicyCallback policyCallback(netConnService);
    EXPECT_EQ(NetConnService::GetInstance()->defaultNetSupplier_, nullptr);
    policyCallback.SendNetPolicyChange(1, 1);
}

HWTEST_F(NetConnServiceExtTest, SendNetPolicyChangeTest002, TestSize.Level1)
{
    NetConnService::NetPolicyCallback policyCallback(NetConnService::GetInstance());
    EXPECT_EQ(NetConnService::GetInstance()->defaultNetSupplier_, nullptr);
    policyCallback.SendNetPolicyChange(1, 1);
}

HWTEST_F(NetConnServiceExtTest, SendNetPolicyChangeTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    netConnService->defaultNetSupplier_ = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    EXPECT_NE(netConnService->defaultNetSupplier_, nullptr);
    NetConnService::NetPolicyCallback policyCallback(netConnService);
    policyCallback.SendNetPolicyChange(1, 1);
}

HWTEST_F(NetConnServiceExtTest, OnAddSystemAbilityTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->hasSARemoved_);
    std::string deviceId = "dev1";
    netConnService->OnAddSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, deviceId);
}

HWTEST_F(NetConnServiceExtTest, OnAddSystemAbilityTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->registerToService_);
    std::string deviceId = "dev1";
    netConnService->OnAddSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID, deviceId);

    EXPECT_TRUE(netConnService->registerToService_);
    netConnService->OnAddSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID, deviceId);
}

HWTEST_F(NetConnServiceExtTest, OnRemoveSystemAbilityTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string deviceId = "dev1";
    netConnService->OnRemoveSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, deviceId);
    EXPECT_FALSE(netConnService->hasSARemoved_);
}

HWTEST_F(NetConnServiceExtTest, IsSupplierMatchRequestAndNetworkTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    netConnService->netActivates_[0] = nullptr;
    bool ret = netConnService->IsSupplierMatchRequestAndNetwork(supplier);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetConnServiceExtTest, IsSupplierMatchRequestAndNetworkTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->defaultNetActivate_ = nullptr;
    netConnService->CreateDefaultRequest();
    bool ret = netConnService->IsSupplierMatchRequestAndNetwork(netConnService->defaultNetSupplier_);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetConnServiceExtTest, RecoverNetSysTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_NE(netConnService->defaultNetSupplier_, nullptr);
    netConnService->netSuppliers_[0] = nullptr;
    netConnService->netSuppliers_[1] = netConnService->defaultNetSupplier_;
    netConnService->RecoverNetSys();
}

HWTEST_F(NetConnServiceExtTest, RecoverNetSysTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_EQ(netConnService->defaultNetSupplier_, nullptr);
    netConnService->netSuppliers_[1] = nullptr;
    netConnService->RecoverNetSys();
}

HWTEST_F(NetConnServiceExtTest, RegisterSlotTypeTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netConnEventHandler_ = nullptr;
    uint32_t supplierId = 10;
    int32_t type = 0;
    auto ret = netConnService->RegisterSlotType(supplierId, type);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    EXPECT_NE(netConnService->netConnEventRunner_, nullptr);
    netConnService->netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnService->netConnEventRunner_);
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    ret = netConnService->RegisterSlotType(supplierId, type);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);

    supplierId = 1;
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    netConnService->netSuppliers_[1] = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    ret = netConnService->RegisterSlotType(supplierId, type);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, GetSlotTypeTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string type;
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    EXPECT_EQ(netConnService->defaultNetSupplier_, nullptr);
    auto ret = netConnService->GetSlotType(type);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    netConnService->defaultNetSupplier_ = netConnService->netSuppliers_[1];
    EXPECT_NE(netConnService->defaultNetSupplier_, nullptr);
    ret = netConnService->GetSlotType(type);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    netConnService->netConnEventHandler_ = nullptr;
    ret = netConnService->GetSlotType(type);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, OnNetSysRestartTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_EQ(netConnService->netConnEventHandler_, nullptr);
    netConnService->OnNetSysRestart();
}

HWTEST_F(NetConnServiceExtTest, IsIfaceNameInUseTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    int32_t netId = 1;
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    std::shared_ptr<Network> network = std::make_shared<Network>(netId, netId, nullptr,
        NetBearType::BEARER_ETHERNET, nullptr);
    supplier->network_ = network;
    supplier->netSupplierInfo_.isAvailable_ = true;
    supplier->network_->netLinkInfo_.ifaceName_ = "rmnet0";
    netConnService->netSuppliers_.clear();
    netConnService->netSuppliers_[1] = supplier;
    auto ret = netConnService->IsIfaceNameInUse("rmnet0", 100);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetConnServiceExtTest, GetNetCapabilitiesAsStringTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netSuppliers_[0] = nullptr;
    uint32_t supplierId = 2;
    auto ret = netConnService->GetNetCapabilitiesAsString(supplierId);
    EXPECT_TRUE(ret.empty());

    supplierId = 0;
    ret = netConnService->GetNetCapabilitiesAsString(supplierId);
    EXPECT_TRUE(ret.empty());

    supplierId = 1;
    ret = netConnService->GetNetCapabilitiesAsString(supplierId);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(NetConnServiceExtTest, FindSupplierWithInternetByBearerTypeTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->netSuppliers_[1]->GetNetCaps().HasNetCap(NET_CAPABILITY_INTERNET));
    auto ret = netConnService->FindSupplierWithInternetByBearerType(NetBearType::BEARER_WIFI, TEST_IDENT);
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(NetConnServiceExtTest, OnRemoteDiedTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_NE(netConnService->defaultNetSupplier_, nullptr);
    wptr<IRemoteObject> remoteObject = nullptr;
    netConnService->OnRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, OnRemoteDiedTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    wptr<IRemoteObject> remoteObject = new MockNetIRemoteObject();
    EXPECT_NE(remoteObject, nullptr);
    netConnService->OnRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, FindSupplierForConnectedTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::vector<sptr<NetSupplier>> suppliers = {nullptr};
    auto ret = netConnService->FindSupplierForConnected(suppliers);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetConnServiceExtTest, OnReceiveEventTest001, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    NetConnService::NetConnListener listener(subscribeInfo, nullptr);
    EXPECT_NE(NetConnService::GetInstance()->defaultNetSupplier_, nullptr);
    EventFwk::CommonEventData eventData;
    listener.OnReceiveEvent(eventData);
}

HWTEST_F(NetConnServiceExtTest, EnableVnicNetworkTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    sptr<NetLinkInfo> netLinkInfo = new NetLinkInfo();
    const std::set<int32_t> uids;
    EXPECT_EQ(netConnService->netConnEventHandler_, nullptr);
    auto ret = netConnService->EnableVnicNetwork(netLinkInfo, uids);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = nullptr;
    ret = netConnService->EnableVnicNetwork(netLinkInfo, uids);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, EnableVnicNetworkAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    sptr<NetLinkInfo> netLinkInfo = new NetLinkInfo();
    const std::set<int32_t> uids;

    NetManagerStandard::INetAddr inetAddr;
    inetAddr.type_ = NetManagerStandard::INetAddr::IpType::IPV4;
    inetAddr.family_ = 0x01;
    inetAddr.address_ = "10.0.0.2.1";
    inetAddr.netMask_ = "255.255.255.0";
    inetAddr.hostName_ = "localhost";
    inetAddr.port_ = 80;
    inetAddr.prefixlen_ = 24;
    netLinkInfo->ifaceName_ = "vnic-tun";
    netLinkInfo->netAddrList_.push_back(inetAddr);
    netLinkInfo->mtu_ = 1500;

    auto ret = netConnService->EnableVnicNetworkAsync(netLinkInfo, uids);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, DisableVnicNetworkTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    auto ret = netConnService->DisableVnicNetwork();
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnService->netConnEventRunner_);
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    ret = netConnService->DisableVnicNetwork();
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedClientNetAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string virnicAddr;
    std::string iif;
    auto ret = netConnService->EnableDistributedClientNetAsync(virnicAddr, iif);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedClientNetAsyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string virnicAddr = "192.168.1.300";
    std::string iif = "eth0";
    auto ret = netConnService->EnableDistributedClientNetAsync(virnicAddr, iif);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedClientNetAsyncTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string virnicAddr = "192.168.1.5";
    std::string iif = "eth0";
    auto ret = netConnService->EnableDistributedClientNetAsync(virnicAddr, iif);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedServerNetTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string iif = "eth0";
    std::string devIface = "bond0";
    std::string dstAddr = "192.168.1.100";
    auto tmpHandler = netConnService->netConnEventHandler_;
    netConnService->netConnEventHandler_ = nullptr;
    auto ret = netConnService->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = tmpHandler;
    ret = netConnService->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedServerNetAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string iif;
    std::string devIface;
    std::string dstAddr;
    auto ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);

    iif = "eth0";
    ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);

    devIface = "bond0";
    dstAddr = "192.168.1.300";
    ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedServerNetAsyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string iif = "eth0";
    std::string devIface = "bond0";
    std::string dstAddr = "192.168.1.100";
    auto ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetConnServiceExtTest, DisableDistributedNetTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    auto tmpHandler = netConnService->netConnEventHandler_;
    netConnService->netConnEventHandler_ = nullptr;
    auto ret = netConnService->DisableDistributedNet(true);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = tmpHandler;
    ret = netConnService->DisableDistributedNet(true);
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, DisableDistributedNetAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    auto ret = netConnService->DisableDistributedNetAsync(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetConnServiceExtTest, CloseSocketsUidAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    int32_t netId = 0;
    uint32_t uid = 1;
    EXPECT_EQ(netConnService->networks_.find(netId), netConnService->networks_.end());
    auto ret = netConnService->CloseSocketsUidAsync(netId, uid);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);

    netId = 1;
    EXPECT_EQ(netConnService->networks_[netId], nullptr);
    ret = netConnService->CloseSocketsUidAsync(netId, uid);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    netConnService->netUidActivates_.clear();
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    bool isFrozened = false;
    std::vector<std::shared_ptr<NetActivate>> activates;
    sptr<NetSpecifier> specifier = nullptr;
    sptr<INetConnCallback> callback = nullptr;
    std::weak_ptr<INetActivateCallback> timeoutCallback;
    std::shared_ptr<AppExecFwk::EventHandler> handler = nullptr;
    auto active = std::make_shared<NetActivate>(specifier, callback, timeoutCallback, 0, handler);
    activates.push_back(active);
    activates[0]->SetIsAppFrozened(isFrozened);
    netConnService->netUidActivates_[uid] = activates;
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    isFrozened = true;
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    isFrozened = false;
    activates[0]->SetIsAppFrozened(true);
    activates[0]->SetLastCallbackType(CALL_TYPE_UNKNOWN);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    bool isFrozened = false;
    auto &activates = netConnService->netUidActivates_[uid];
    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_AVAILABLE);
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_LOST);
    EXPECT_EQ(activates[0]->GetLastServiceSupply(), nullptr);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    activates[0]->SetLastServiceSupply(supplier);
    EXPECT_EQ(activates[0]->GetNetCallback(), nullptr);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    activates[0]->netConnCallback_ = new (std::nothrow) NetConnCallbackStubCb();
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest004, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    bool isFrozened = false;
    auto &activates = netConnService->netUidActivates_[uid];
    activates[0]->SetServiceSupply(activates[0]->GetLastServiceSupply());
    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_AVAILABLE);
    EXPECT_NE(activates[0]->GetServiceSupply(), nullptr);
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_LOST);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, EnableAppFrozenedCallbackLimitationTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netConnEventHandler_ = nullptr;
    auto ret = netConnService->EnableAppFrozenedCallbackLimitation(true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetReuseSupplierIdTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    uint32_t reuseSupplierId = 1;
    netConnService->netSuppliers_.clear();
    netConnService->netSuppliers_[0] = nullptr;
    auto ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetReuseSupplierIdTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    uint32_t reuseSupplierId = 2;
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    supplier->supplierId_ = supplierId;
    netConnService->netSuppliers_[1] = supplier;
    auto ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    netConnService->netSuppliers_[1]->supplierId_ = reuseSupplierId;
    ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    netConnService->netSuppliers_[1]->supplierId_ = 0;
    ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
