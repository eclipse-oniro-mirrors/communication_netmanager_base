/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <iostream>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_conn_callback_stub.h"
#include "net_conn_security.h"
#include "net_conn_service_stub.h"
#include "net_detection_callback_stub.h"
#include "net_interface_callback_stub.h"
#include "net_supplier_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
constexpr int32_t TEST_INT32_VALUE = 1;
constexpr uint32_t TEST_UINT32_VALUE = 1;
constexpr uint64_t OUTOFFRANGECODE = 100;
constexpr const char *TEST_STRING = "test";

class TestNetConnCallback : public NetConnCallbackStub {
public:
    TestNetConnCallback() = default;
    ~TestNetConnCallback() override{};

    int32_t NetAvailable(sptr<NetHandle> &netHandle) override
    {
        return 0;
    }

    int32_t NetCapabilitiesChange(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap) override
    {
        return 0;
    }

    int32_t NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info) override
    {
        return 0;
    }

    int32_t NetLost(sptr<NetHandle> &netHandle) override
    {
        return 0;
    }

    int32_t NetUnavailable() override
    {
        return 0;
    }

    int32_t NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked) override
    {
        return 0;
    }
};

class TestNetDetectionCallback : public NetDetectionCallbackStub {
public:
    TestNetDetectionCallback() = default;
    ~TestNetDetectionCallback() override{};

    int32_t OnNetDetectionResultChanged(NetDetectionResultCode detectionResult, const std::string &urlRedirect) override
    {
        return 0;
    }
};

class TestNetInterfaceStateCallback : public NetInterfaceStateCallbackStub {
public:
    TestNetInterfaceStateCallback() = default;
    ~TestNetInterfaceStateCallback() override{};
};

class TestNetSupplierCallback : public NetSupplierCallbackStub {
public:
    TestNetSupplierCallback() = default;
    ~TestNetSupplierCallback() override{};

    int32_t RequestNetwork(const std::string &ident, const std::set<NetCap> &netCaps) override
    {
        return 0;
    }

    int32_t ReleaseNetwork(const std::string &ident, const std::set<NetCap> &netCaps) override
    {
        return 0;
    }
};

class MockNetConnServiceStub : public NetConnServiceStub {
public:
    MockNetConnServiceStub() = default;
    ~MockNetConnServiceStub() {}

    int32_t SystemReady() override
    {
        return 0;
    }

    int32_t SetInternetPermission(uint32_t uid, uint8_t allow) override
    {
        return 0;
    }

    int32_t RegisterNetSupplier(NetBearType bearerType, const std::string &ident, const std::set<NetCap> &netCaps,
                                uint32_t &supplierId) override
    {
        return 0;
    }

    int32_t UnregisterNetSupplier(uint32_t supplierId) override
    {
        return 0;
    }

    int32_t RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback) override
    {
        return 0;
    }

    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback) override
    {
        return 0;
    }

    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                                    const uint32_t &timeoutMS) override
    {
        return 0;
    }

    int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback) override
    {
        return 0;
    }

    int32_t UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState) override
    {
        return 0;
    }

    int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo) override
    {
        return 0;
    }

    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo) override
    {
        return 0;
    }

    int32_t GetDefaultNet(int32_t &netId) override
    {
        return 0;
    }

    int32_t HasDefaultNet(bool &flag) override
    {
        return 0;
    }

    int32_t GetIfaceNames(NetBearType bearerType, std::list<std::string> &ifaceNames) override
    {
        return 0;
    }

    int32_t GetIfaceNameByType(NetBearType bearerType, const std::string &ident, std::string &ifaceName) override
    {
        return 0;
    }

    int32_t RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override
    {
        return 0;
    }

    int32_t UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override
    {
        return 0;
    }

    int32_t NetDetection(int32_t netId) override
    {
        return 0;
    }

    int32_t GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList) override
    {
        return 0;
    }

    int32_t GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr) override
    {
        return 0;
    }

    int32_t GetSpecificNet(NetBearType bearerType, std::list<int32_t> &netIdList) override
    {
        return 0;
    }

    int32_t GetAllNets(std::list<int32_t> &netIdList) override
    {
        return 0;
    }

    int32_t GetSpecificUidNet(int32_t uid, int32_t &netId) override
    {
        return 0;
    }

    int32_t GetConnectionProperties(int32_t netId, NetLinkInfo &info) override
    {
        return 0;
    }

    int32_t GetNetCapabilities(int32_t netId, NetAllCapabilities &netAllCap) override
    {
        return 0;
    }

    int32_t BindSocket(int32_t socket_fd, int32_t netId) override
    {
        return 0;
    }

    int32_t SetAirplaneMode(bool state) override
    {
        return 0;
    }

    int32_t IsDefaultNetMetered(bool &isMetered) override
    {
        return 0;
    }

    int32_t SetGlobalHttpProxy(const HttpProxy &httpProxy) override
    {
        return 0;
    }

    int32_t GetGlobalHttpProxy(HttpProxy &httpProxy) override
    {
        return 0;
    }

    int32_t GetDefaultHttpProxy(int32_t bindNetId, HttpProxy &httpProxy) override
    {
        return 0;
    }

    int32_t GetNetIdByIdentifier(const std::string &ident, std::list<int32_t> &netIdList) override
    {
        return 0;
    }

    int32_t SetAppNet(int32_t netId) override
    {
        return 0;
    }

    int32_t RegisterNetInterfaceCallback(const sptr<INetInterfaceStateCallback> &callback) override
    {
        return 0;
    }

    int32_t GetNetInterfaceConfiguration(const std::string &iface, NetInterfaceConfiguration &config) override
    {
        return 0;
    }

    int32_t AddNetworkRoute(int32_t netId, const std::string &ifName, const std::string &destination,
                            const std::string &nextHop) override
    {
        return 0;
    }

    int32_t RemoveNetworkRoute(int32_t netId, const std::string &ifName, const std::string &destination,
                               const std::string &nextHop) override
    {
        return 0;
    }

    int32_t AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength) override
    {
        return 0;
    }

    int32_t DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength) override
    {
        return 0;
    }

    int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr, const std::string &ifName) override
    {
        return 0;
    }

    int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr, const std::string &ifName) override
    {
        return 0;
    }

    int32_t RegisterSlotType(uint32_t supplierId, int32_t type) override
    {
        return 0;
    }

    int32_t GetSlotType(std::string &type) override
    {
        return 0;
    }
};

class TestNetConnServiceStub : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<NetConnServiceStub> instance_ = std::make_shared<MockNetConnServiceStub>();
};

void TestNetConnServiceStub::SetUpTestCase() {}

void TestNetConnServiceStub::TearDownTestCase() {}

void TestNetConnServiceStub::SetUp() {}

void TestNetConnServiceStub::TearDown() {}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Test NetConnServiceStub OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRemoteRequestTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(OUTOFFRANGECODE, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnRemoteRequestTest002
 * @tc.desc: Test NetConnServiceStub OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRemoteRequestTest002, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(OUTOFFRANGECODE, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnSystemReadyTest001
 * @tc.desc: Test NetConnServiceStub OnSystemReady.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnSystemReadyTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SYSTEM_READY), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnRegisterNetConnCallbackTest001
 * @tc.desc: Test NetConnServiceStub OnRegisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRegisterNetConnCallbackTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetConnCallback> callback = new (std::nothrow) TestNetConnCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnRegisterNetConnCallbackBySpecifierTest001
 * @tc.desc: Test NetConnServiceStub OnRegisterNetConnCallbackBySpecifier.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRegisterNetConnCallbackBySpecifierTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    sptr<INetConnCallback> callback = new (std::nothrow) TestNetConnCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnUnregisterNetConnCallbackTest001
 * @tc.desc: Test NetConnServiceStub OnUnregisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnUnregisterNetConnCallbackTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    sptr<INetConnCallback> callback = new (std::nothrow) TestNetConnCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_CONN_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

/**
 * @tc.name: OnUpdateNetStateForTest001
 * @tc.desc: Test NetConnServiceStub OnUpdateNetStateForTest.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnUpdateNetStateForTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UPDATE_NET_STATE_FOR_TEST),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnRegisterNetSupplierTest001
 * @tc.desc: Test NetConnServiceStub OnRegisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRegisterNetSupplierTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REG_NET_SUPPLIER), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnUnregisterNetSupplierTest001
 * @tc.desc: Test NetConnServiceStub OnUnregisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnUnregisterNetSupplierTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREG_NETWORK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnUpdateNetSupplierInfoTest001
 * @tc.desc: Test NetConnServiceStub OnUpdateNetSupplierInfo.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnUpdateNetSupplierInfoTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_SUPPLIER_INFO),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnUpdateNetLinkInfoTest001
 * @tc.desc: Test NetConnServiceStub OnUpdateNetLinkInfo.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnUpdateNetLinkInfoTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    if (!netLinkInfo->Marshalling(data)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_LINK_INFO), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnRegisterNetDetectionCallbackTest001
 * @tc.desc: Test NetConnServiceStub OnRegisterNetDetectionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRegisterNetDetectionCallbackTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }
    sptr<INetDetectionCallback> callback = new (std::nothrow) TestNetDetectionCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_DETECTION_RET_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnUnRegisterNetDetectionCallbackTest001
 * @tc.desc: Test NetConnServiceStub OnUnRegisterNetDetectionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnUnRegisterNetDetectionCallbackTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }
    sptr<INetDetectionCallback> callback = new (std::nothrow) TestNetDetectionCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_DETECTION_RET_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnNetDetectionTest001
 * @tc.desc: Test NetConnServiceStub OnNetDetection.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnNetDetectionTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_NET_DETECTION), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetIfaceNamesTest001
 * @tc.desc: Test NetConnServiceStub OnGetIfaceNames.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetIfaceNamesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACE_NAMES), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIfaceNameByTypeTest001
 * @tc.desc: Test NetConnServiceStub GetIfaceNameByType.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, GetIfaceNameByTypeTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACENAME_BY_TYPE),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetDefaultNetTest001
 * @tc.desc: Test NetConnServiceStub OnGetDefaultNetTest001.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, GetDefaultNetTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GETDEFAULTNETWORK), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnHasDefaultNetTest001
 * @tc.desc: Test NetConnServiceStub OnHasDefaultNet.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnHasDefaultNetTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_HASDEFAULTNET), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetSpecificNetTest001
 * @tc.desc: Test NetConnServiceStub OnGetSpecificNet.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetSpecificNetTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_NET), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetAllNetsTest001
 * @tc.desc: Test NetConnServiceStub OnGetAllNets.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetAllNetsTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ALL_NETS), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetSpecificUidNetTest001
 * @tc.desc: Test NetConnServiceStub OnGetSpecificUidNet.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetSpecificUidNetTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_UID_NET),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetConnectionPropertiesTest001
 * @tc.desc: Test NetConnServiceStub OnGetConnectionProperties.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetConnectionPropertiesTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_CONNECTION_PROPERTIES),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetNetCapabilitiesTest001
 * @tc.desc: Test NetConnServiceStub OnGetNetCapabilities.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetNetCapabilitiesTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_CAPABILITIES),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetAddressesByNameTest001
 * @tc.desc: Test NetConnServiceStub OnGetAddressesByName.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetAddressesByNameTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESSES_BY_NAME),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetAddressByNameTest001
 * @tc.desc: Test NetConnServiceStub OnGetAddressByName.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetAddressByNameTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESS_BY_NAME), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnBindSocketTest001
 * @tc.desc: Test NetConnServiceStub OnBindSocket.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnBindSocketTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_BIND_SOCKET), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnSetAirplaneModeTest001
 * @tc.desc: Test NetConnServiceStub OnSetAirplaneMode.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnSetAirplaneModeTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    bool state = false;
    if (!data.WriteBool(state)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_AIRPLANE_MODE), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnSetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnServiceStub OnSetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnSetGlobalHttpProxyTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    HttpProxy httpProxy;
    if (!httpProxy.Marshalling(data)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_GLOBAL_HTTP_PROXY),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnSetAppNetTest001
 * @tc.desc: Test NetConnServiceStub OnSetAppNet.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnSetAppNetTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_APP_NET), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnSetInternetPermissionTest001
 * @tc.desc: Test NetConnServiceStub OnSetInternetPermission.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnSetInternetPermissionTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    uint8_t allow = 0;
    if (!data.WriteUint8(allow)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_INTERNET_PERMISSION),
                                             data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnRegisterNetInterfaceCallbackTest001
 * @tc.desc: Test NetConnServiceStub OnRegisterNetInterfaceCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRegisterNetInterfaceCallbackTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) TestNetInterfaceStateCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_INTERFACE_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnAddNetworkRouteTest001
 * @tc.desc: Test NetConnServiceStub OnAddNetworkRoute.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnAddNetworkRouteTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ROUTE), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnRemoveNetworkRouteTest001
 * @tc.desc: Test NetConnServiceStub OnRemoveNetworkRoute.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRemoveNetworkRouteTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ROUTE), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnAddInterfaceAddressTest001
 * @tc.desc: Test NetConnServiceStub OnAddInterfaceAddress.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnAddInterfaceAddressTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ADDRESS), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnDelInterfaceAddressTest001
 * @tc.desc: Test NetConnServiceStub OnDelInterfaceAddress.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnDelInterfaceAddressTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ADDRESS), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnRegisterNetSupplierCallbackTest001
 * @tc.desc: Test NetConnServiceStub OnRegisterNetSupplierCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRegisterNetSupplierCallbackTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    sptr<INetSupplierCallback> callback = new (std::nothrow) TestNetSupplierCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnAddStaticArpTest001
 * @tc.desc: Test NetConnServiceStub OnAddStaticArp.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnAddStaticArpTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_STATIC_ARP), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnDelStaticArpTest001
 * @tc.desc: Test NetConnServiceStub OnDelStaticArp.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnDelStaticArpTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_DEL_STATIC_ARP), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnIsDefaultNetMeteredTest001
 * @tc.desc: Test NetConnServiceStub OnIsDefaultNetMetered.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnIsDefaultNetMeteredTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_IS_DEFAULT_NET_METERED),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnServiceStub OnGetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetGlobalHttpProxyTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_GLOBAL_HTTP_PROXY),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetDefaultHttpProxyTest001
 * @tc.desc: Test NetConnServiceStub OnGetDefaultHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetDefaultHttpProxyTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_DEFAULT_HTTP_PROXY),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetNetIdByIdentifierTest001
 * @tc.desc: Test NetConnServiceStub OnGetNetIdByIdentifier.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetNetIdByIdentifierTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_ID_BY_IDENTIFIER),
                                             data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnRegisterSlotTypeTest001
 * @tc.desc: Test NetConnServiceStub OnRegisterSlotType.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnRegisterSlotTypeTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    if (!data.WriteInt32(TEST_INT32_VALUE)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_SLOT_TYPE), data,
                                             reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnGetSlotTypeTest001
 * @tc.desc: Test NetConnServiceStub OnGetSlotType.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetConnServiceStub, OnGetSlotTypeTest001, TestSize.Level1)
{
    NetConnManagerAccessToken toekn;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SLOT_TYPE), data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace
} // namespace NetManagerStandard
} // namespace OHOS
