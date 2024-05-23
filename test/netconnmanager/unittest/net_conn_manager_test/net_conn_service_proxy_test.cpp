/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include <memory>

#include "common_net_conn_callback_test.h"
#include "i_net_conn_service.h"
#include "i_net_detection_callback.h"
#include "i_net_factoryreset_callback.h"
#include "net_all_capabilities.h"
#include "net_conn_service_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
constexpr int32_t TEST_UID = 1010;
constexpr const char *TEST_IDENT = "testIdent";
constexpr uint32_t TEST_TIMEOUTMS = 1000;
constexpr const char *TEST_HOST = "testHost";
constexpr int32_t TEST_NETID = 3;
constexpr int32_t TEST_SOCKETFD = 2;
constexpr int32_t TEST_SUPPLIERID = 1021;

uint32_t g_supplierId = 0;
class MockNetIRemoteObject : public IRemoteObject {
public:
    MockNetIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}
    ~MockNetIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        reply.WriteInt32(NETMANAGER_SUCCESS);
        switch (code) {
            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACE_NAMES):
            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_NET):
            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ALL_NETS):
            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESSES_BY_NAME):
            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_ID_BY_IDENTIFIER):
                reply.WriteUint32(NETMANAGER_SUCCESS);
                break;

            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACENAME_BY_TYPE):
                reply.WriteString(TEST_HOST);
                break;

            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GETDEFAULTNETWORK):
                reply.WriteInt32(TEST_NETID);
                break;

            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_HASDEFAULTNET):
            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_IS_DEFAULT_NET_METERED):
                reply.WriteBool(true);
                break;

            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_CONNECTION_PROPERTIES): {
                NetLinkInfo linkInfo;
                linkInfo.ifaceName_ = "ifacename_test";
                linkInfo.Marshalling(reply);
                break;
            }

            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_CAPABILITIES): {
                NetAllCapabilities netCap;
                netCap.Marshalling(reply);
                break;
            }

            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESS_BY_NAME): {
                INetAddr addr;
                addr.Marshalling(reply);
                break;
            }

            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_GLOBAL_HTTP_PROXY):
            case static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_DEFAULT_HTTP_PROXY): {
                HttpProxy httpProxy;
                httpProxy.Marshalling(reply);
                break;
            }

            default:
                reply.WriteUint32(TEST_SUPPLIERID);
                break;
        }

        return eCode;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }

    void SetErrorCode(int errorCode)
    {
        eCode = errorCode;
    }

private:
    int eCode = NETMANAGER_SUCCESS;
};

class NetDetectionTestCallback : public IRemoteStub<INetDetectionCallback> {
public:
    int32_t OnNetDetectionResultChanged(NetDetectionResultCode resultCode, const std::string &urlRedirect) override
    {
        return 0;
    }
};

class NetFactoryResetTestCallback : public IRemoteStub<INetFactoryResetCallback> {
public:
    int32_t OnNetFactoryReset() override
    {
        return 0;
    }
};

class NetConnServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline sptr<MockNetIRemoteObject> remoteObj_ = std::make_unique<MockNetIRemoteObject>().release();
    static inline std::shared_ptr<NetConnServiceProxy> instance_ = std::make_shared<NetConnServiceProxy>(remoteObj_);
    static inline sptr<INetSupplierCallback> supplierCallback_ = new (std::nothrow) NetSupplierCallbackStub();
    static inline sptr<INetConnCallback> netConnCallback_ = new (std::nothrow) NetConnCallbackStubCb();
    static inline sptr<INetDetectionCallback> detectionCallback_ = new (std::nothrow) NetDetectionTestCallback();
    static inline sptr<INetFactoryResetCallback> netFactoryResetCallback_ =
        new (std::nothrow) NetFactoryResetTestCallback();
};

void NetConnServiceProxyTest::SetUpTestCase() {}

void NetConnServiceProxyTest::TearDownTestCase() {}

void NetConnServiceProxyTest::SetUp() {}

void NetConnServiceProxyTest::TearDown() {}

/**
 * @tc.name: SystemReadyTest001
 * @tc.desc: Test NetConnServiceProxy SystemReady.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, SystemReadyTest001, TestSize.Level1)
{
    int32_t ret = instance_->SystemReady();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetInternetPermissionTest001
 * @tc.desc: Test NetConnServiceProxy SetInternetPermission.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, SetInternetPermissionTest001, TestSize.Level1)
{
    int32_t ret = instance_->SetInternetPermission(TEST_UID, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetSupplierTest001
 * @tc.desc: Test NetConnServiceProxy RegisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, RegisterNetSupplierTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    int32_t ret = instance_->RegisterNetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps, g_supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnregisterNetSupplierTest001
 * @tc.desc: Test NetConnServiceProxy UnregisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, UnregisterNetSupplierTest001, TestSize.Level1)
{
    int32_t ret = instance_->UnregisterNetSupplier(g_supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetSupplierCallbackTest001
 * @tc.desc: Test NetConnServiceProxy RegisterNetSupplierCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, RegisterNetSupplierCallbackTest001, TestSize.Level1)
{
    int32_t ret = instance_->RegisterNetSupplierCallback(g_supplierId, supplierCallback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetConnCallbackTest001
 * @tc.desc: Test NetConnServiceProxy RegisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, RegisterNetConnCallbackTest001, TestSize.Level1)
{
    int32_t ret = instance_->RegisterNetConnCallback(netConnCallback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetConnCallbackTest002
 * @tc.desc: Test NetConnServiceProxy RegisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, RegisterNetConnCallbackTest002, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    int32_t ret = instance_->RegisterNetConnCallback(netSpecifier, netConnCallback_, TEST_TIMEOUTMS);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RequestNetConnectionTest001
 * @tc.desc: Test NetConnServiceProxy RequestNetConnection.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, RequestNetConnectionTest001, TestSize.Level1)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    int32_t ret = instance_->RequestNetConnection(netSpecifier, netConnCallback_, TEST_TIMEOUTMS);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnregisterNetConnCallbackTest001
 * @tc.desc: Test NetConnServiceProxy UnregisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, UnregisterNetConnCallbackTest001, TestSize.Level1)
{
    int32_t ret = instance_->UnregisterNetConnCallback(netConnCallback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateNetStateForTest001
 * @tc.desc: Test NetConnServiceProxy UpdateNetStateForTest.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, UpdateNetStateForTest001, TestSize.Level1)
{
    int32_t netState = 0;
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    int32_t ret = instance_->UpdateNetStateForTest(netSpecifier, netState);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateNetSupplierInfoTest001
 * @tc.desc: Test NetConnServiceProxy UpdateNetSupplierInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, UpdateNetSupplierInfoTest001, TestSize.Level1)
{
    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo();
    int32_t ret = instance_->UpdateNetSupplierInfo(g_supplierId, netSupplierInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateNetLinkInfoTest001
 * @tc.desc: Test NetConnServiceProxy UpdateNetLinkInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, UpdateNetLinkInfoTest001, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    int32_t ret = instance_->UpdateNetLinkInfo(g_supplierId, netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetDetectionCallbackTest001
 * @tc.desc: Test NetConnServiceProxy RegisterNetDetectionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, RegisterNetDetectionCallbackTest001, TestSize.Level1)
{
    int32_t ret = instance_->RegisterNetDetectionCallback(TEST_NETID, detectionCallback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnRegisterNetDetectionCallbackTest001
 * @tc.desc: Test NetConnServiceProxy UnRegisterNetDetectionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, UnRegisterNetDetectionCallbackTest001, TestSize.Level1)
{
    int32_t ret = instance_->UnRegisterNetDetectionCallback(TEST_NETID, detectionCallback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetDetectionTest001
 * @tc.desc: Test NetConnServiceProxy NetDetection.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, NetDetectionTest001, TestSize.Level1)
{
    int32_t ret = instance_->NetDetection(TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIfaceNamesTest001
 * @tc.desc: Test NetConnServiceProxy GetIfaceNames.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetIfaceNamesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    int32_t ret = instance_->GetIfaceNames(NetBearType::BEARER_ETHERNET, ifaceNames);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIfaceNameByTypeTest001
 * @tc.desc: Test NetConnServiceProxy GetIfaceNameByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetIfaceNameByTypeTest001, TestSize.Level1)
{
    std::string ifaceName;
    int32_t ret = instance_->GetIfaceNameByType(NetBearType::BEARER_ETHERNET, TEST_IDENT, ifaceName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetDefaultNetTest001
 * @tc.desc: Test NetConnServiceProxy GetDefaultNet.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetDefaultNetTest001, TestSize.Level1)
{
    int32_t netId = 0;
    int32_t ret = instance_->GetDefaultNet(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: HasDefaultNetTest001
 * @tc.desc: Test NetConnServiceProxy HasDefaultNet.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, HasDefaultNetTest001, TestSize.Level1)
{
    bool hasDefaultNet = false;
    int32_t ret = instance_->HasDefaultNet(hasDefaultNet);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(hasDefaultNet, true);
}

/**
 * @tc.name: GetSpecificNetTest001
 * @tc.desc: Test NetConnServiceProxy GetSpecificNet.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetSpecificNetTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    int32_t ret = instance_->GetSpecificNet(NetBearType::BEARER_ETHERNET, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAllNetsTest001
 * @tc.desc: Test NetConnServiceProxy GetAllNets.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetAllNetsTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    int32_t ret = instance_->GetAllNets(netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetSpecificUidNetTest001
 * @tc.desc: Test NetConnServiceProxy GetSpecificUidNet.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetSpecificUidNetTest001, TestSize.Level1)
{
    int32_t netId = 0;
    int32_t ret = instance_->GetSpecificUidNet(TEST_UID, netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetConnectionPropertiesTest001
 * @tc.desc: Test NetConnServiceProxy GetConnectionProperties.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetConnectionPropertiesTest001, TestSize.Level1)
{
    NetLinkInfo info;
    int32_t ret = instance_->GetConnectionProperties(TEST_NETID, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetCapabilitiesTest001
 * @tc.desc: Test NetConnServiceProxy GetNetCapabilities.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetNetCapabilitiesTest001, TestSize.Level1)
{
    NetAllCapabilities netAllCap;
    int32_t ret = instance_->GetNetCapabilities(TEST_NETID, netAllCap);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAddressesByNameTest001
 * @tc.desc: Test NetConnServiceProxy GetAddressesByName.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetAddressesByNameTest001, TestSize.Level1)
{
    std::string host;
    std::vector<INetAddr> addrList;
    int32_t ret = instance_->GetAddressesByName(host, TEST_NETID, addrList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAddressesByNameTest001
 * @tc.desc: Test NetConnServiceProxy GetAddressesByName.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetAddresseByNameTest001, TestSize.Level1)
{
    std::string host;
    INetAddr addr;
    int32_t ret = instance_->GetAddressByName(host, TEST_NETID, addr);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}


/**
 * @tc.name: BindSocketTest001
 * @tc.desc: Test NetConnServiceProxy BindSocket.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, BindSocketTest001, TestSize.Level1)
{
    int32_t ret = instance_->BindSocket(TEST_SOCKETFD, TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetAirplaneModeTest001
 * @tc.desc: Test NetConnServiceProxy SetAirplaneMode.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, SetAirplaneModeTest001, TestSize.Level1)
{
    bool airplaneMode = true;
    int32_t ret = instance_->SetAirplaneMode(airplaneMode);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsDefaultNetMeteredTest001
 * @tc.desc: Test NetConnServiceProxy IsDefaultNetMetered.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, IsDefaultNetMeteredTest001, TestSize.Level1)
{
    bool isMetered;
    int32_t ret = instance_->IsDefaultNetMetered(isMetered);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnServiceProxy SetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, SetGlobalHttpProxyTest001, TestSize.Level1)
{
    HttpProxy proxy;
    proxy.SetHost(TEST_HOST);
    int32_t ret = instance_->SetGlobalHttpProxy(proxy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetGlobalHttpProxyTest001
 * @tc.desc: Test NetConnServiceProxy GetGlobalHttpProxy.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetGlobalHttpProxyTest001, TestSize.Level1)
{
    HttpProxy proxy;
    int32_t ret = instance_->GetGlobalHttpProxy(proxy);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetNetIdByIdentifierTest001
 * @tc.desc: Test NetConnServiceProxy GetNetIdByIdentifier.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, GetNetIdByIdentifierTest001, TestSize.Level1)
{
    std::list<int32_t> netIdList;
    int32_t ret = instance_->GetNetIdByIdentifier(TEST_IDENT, netIdList);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetAppNetTest001
 * @tc.desc: Test NetConnServiceProxy SetAppNet.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, SetAppNetTest001, TestSize.Level1)
{
    int32_t ret = instance_->SetAppNet(TEST_NETID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FactoryResetNetworkTest001
 * @tc.desc: Test NetConnServiceProxy FactoryResetNetwork.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, FactoryResetNetworkTest001, TestSize.Level1)
{
    int32_t ret = instance_->FactoryResetNetwork();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: RegisterNetConnCallbackTest001
 * @tc.desc: Test NetConnServiceProxy RegisterNetFactoryResetCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, RegisterNetFactoryResetCallbackTest001, TestSize.Level1)
{
    int32_t ret = instance_->RegisterNetFactoryResetCallback(netFactoryResetCallback_);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetConnServiceProxyBranchTest001
 * @tc.desc: Test NetConnServiceProxy Branch.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceProxyTest, NetConnServiceProxyBranchTest001, TestSize.Level1)
{
    sptr<INetSupplierCallback> supplierCallback = nullptr;
    int32_t ret = instance_->RegisterNetSupplierCallback(g_supplierId, supplierCallback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<INetConnCallback> connCallback = nullptr;
    ret = instance_->RegisterNetConnCallback(connCallback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<NetSpecifier> netSpecifier = nullptr;
    uint32_t timeoutMS = 0;
    ret = instance_->RegisterNetConnCallback(netSpecifier, connCallback, timeoutMS);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->RequestNetConnection(netSpecifier, connCallback, timeoutMS);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = instance_->UnregisterNetConnCallback(connCallback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    int32_t netState = 0;
    ret = instance_->UpdateNetStateForTest(netSpecifier, netState);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<INetInterfaceStateCallback> stateCallback = nullptr;
    ret = instance_->RegisterNetInterfaceCallback(stateCallback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<INetFactoryResetCallback> netFactoryResetCallback = nullptr;
    ret = instance_->RegisterNetFactoryResetCallback(netFactoryResetCallback);
    EXPECT_EQ(ret, NETMANAGER_ERR_LOCAL_PTR_NULL);
}
}
} // namespace NetManagerStandard
} // namespace OHOS
