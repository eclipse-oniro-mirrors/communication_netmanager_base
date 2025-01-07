/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#define NETMGR_DEBUG 1

#include "net_supplier_callback_proxy.h"

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;
using ::testing::_;
using ::testing::Return;

class RemoteObjectMocker : public IRemoteObject {
public:
    RemoteObjectMocker() : IRemoteObject{u"RemoteObjectMocker"} {}
    ~RemoteObjectMocker() {}

    MOCK_METHOD(int32_t, GetObjectRefCount, (), (override));
    MOCK_METHOD(int, SendRequest, (uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option),
                (override));
    MOCK_METHOD(bool, IsProxyObject, (), (const, override));
    MOCK_METHOD(bool, IsObjectDead, (), (const, override));
    MOCK_METHOD(std::u16string, GetInterfaceDescriptor, (), (override));
    MOCK_METHOD(bool, CheckObjectLegality, (), (const, override));
    MOCK_METHOD(bool, AddDeathRecipient, (const sptr<DeathRecipient> &recipient), (override));
    MOCK_METHOD(bool, RemoveDeathRecipient, (const sptr<DeathRecipient> &recipient), (override));
    MOCK_METHOD(bool, Marshalling, (OHOS::Parcel & parcel), (const, override));
    MOCK_METHOD(sptr<IRemoteBroker>, AsInterface, (), (override));
    MOCK_METHOD(int, Dump, (int fd, const std::vector<std::u16string> &args), (override));
};

class NetSupplierCallbackProxyTest : public testing::Test {
protected:
    RemoteObjectMocker *remoteObjectMocker;
    sptr<NetSupplierCallbackProxy> proxy;

    void SetUp() override
    {
        remoteObjectMocker = new RemoteObjectMocker();
        sptr<IRemoteObject> impl(remoteObjectMocker);
        proxy = new (std::nothrow) NetSupplierCallbackProxy(impl);
    }

    void TearDown() override
    {
        remoteObjectMocker = nullptr;
        proxy = nullptr;
    }
};

HWTEST_F(NetSupplierCallbackProxyTest, RequestNetwork_001, TestSize.Level1)
{
    // Arrange
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    NetRequest netrequest;

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_TRANSACTION_FAILED));
    int32_t result = proxy->RequestNetwork(ident, netCaps, netrequest);
    ASSERT_EQ(result, ERR_TRANSACTION_FAILED);
}

HWTEST_F(NetSupplierCallbackProxyTest, RequestNetwork_002, TestSize.Level1)
{
    // Arrange
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    NetRequest netrequest;

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_NONE));
    int32_t result = proxy->RequestNetwork(ident, netCaps, netrequest);
    ASSERT_EQ(result, ERR_NONE);
}

HWTEST_F(NetSupplierCallbackProxyTest, RequestNetwork_003, TestSize.Level1)
{
    sptr<NetSupplierCallbackProxy> proxy = new (std::nothrow) NetSupplierCallbackProxy(NULL);
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    NetRequest netrequest;

    int32_t result = proxy->RequestNetwork(ident, netCaps, netrequest);
    ASSERT_EQ(result, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetSupplierCallbackProxyTest, ReleaseNetwork_001, TestSize.Level1)
{
    // Arrange
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    NetRequest netrequest;

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_TRANSACTION_FAILED));
    int32_t result = proxy->ReleaseNetwork(ident, netCaps);
    ASSERT_EQ(result, ERR_TRANSACTION_FAILED);
}

HWTEST_F(NetSupplierCallbackProxyTest, ReleaseNetwork_002, TestSize.Level1)
{
    // Arrange
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    NetRequest netrequest;

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_NONE));
    int32_t result = proxy->ReleaseNetwork(ident, netCaps);
    ASSERT_EQ(result, ERR_NONE);
}

HWTEST_F(NetSupplierCallbackProxyTest, ReleaseNetwork_003, TestSize.Level1)
{
    sptr<NetSupplierCallbackProxy> proxy = new (std::nothrow) NetSupplierCallbackProxy(NULL);
    std::string ident = "testIdent";
    std::set<NetCap> netCaps;
    NetRequest netrequest;

    int32_t result = proxy->ReleaseNetwork(ident, netCaps);
    ASSERT_EQ(result, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetSupplierCallbackProxyTest, AddRequest_001, TestSize.Level1)
{
    NetRequest netRequest;
    netRequest.uid = 1;
    netRequest.requestId = 1;
    netRequest.registerType = 1;
    netRequest.ident = "test";
    netRequest.bearTypes = {BEARER_WIFI, BEARER_BLUETOOTH, BEARER_ETHERNET};
    netRequest.netCaps = {NET_CAPABILITY_SUPL, NET_CAPABILITY_DUN, NET_CAPABILITY_IA};

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_TRANSACTION_FAILED));
    int32_t result = proxy->AddRequest(netRequest);
    ASSERT_EQ(result, ERR_TRANSACTION_FAILED);
}

HWTEST_F(NetSupplierCallbackProxyTest, AddRequest_002, TestSize.Level1)
{
    NetRequest netRequest;
    netRequest.uid = 1;
    netRequest.requestId = 1;
    netRequest.registerType = 1;
    netRequest.ident = "test";
    netRequest.bearTypes = {BEARER_WIFI, BEARER_BLUETOOTH, BEARER_ETHERNET};
    netRequest.netCaps = {NET_CAPABILITY_SUPL, NET_CAPABILITY_DUN, NET_CAPABILITY_IA};

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_NONE));
    int32_t result = proxy->AddRequest(netRequest);
    ASSERT_EQ(result, ERR_NONE);
}

HWTEST_F(NetSupplierCallbackProxyTest, AddRequest_003, TestSize.Level1)
{
    sptr<NetSupplierCallbackProxy> proxy = new (std::nothrow) NetSupplierCallbackProxy(NULL);
    NetRequest netRequest;
    int32_t result = proxy->AddRequest(netRequest);
    ASSERT_EQ(result, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetSupplierCallbackProxyTest, RemoveRequest_001, TestSize.Level1)
{
    NetRequest netRequest;
    netRequest.uid = 1;
    netRequest.requestId = 1;
    netRequest.registerType = 1;
    netRequest.ident = "test";
    netRequest.bearTypes = {BEARER_WIFI, BEARER_BLUETOOTH, BEARER_ETHERNET};
    netRequest.netCaps = {NET_CAPABILITY_SUPL, NET_CAPABILITY_DUN, NET_CAPABILITY_IA};

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_TRANSACTION_FAILED));
    int32_t result = proxy->RemoveRequest(netRequest);
    ASSERT_EQ(result, ERR_TRANSACTION_FAILED);
}

HWTEST_F(NetSupplierCallbackProxyTest, RemoveRequest_002, TestSize.Level1)
{
    NetRequest netRequest;
    netRequest.uid = 1;
    netRequest.requestId = 1;
    netRequest.registerType = 1;
    netRequest.ident = "test";
    netRequest.bearTypes = {BEARER_WIFI, BEARER_BLUETOOTH, BEARER_ETHERNET};
    netRequest.netCaps = {NET_CAPABILITY_SUPL, NET_CAPABILITY_DUN, NET_CAPABILITY_IA};

    EXPECT_CALL(*remoteObjectMocker, SendRequest(_, _, _, _)).WillOnce(Return(ERR_NONE));
    int32_t result = proxy->RemoveRequest(netRequest);
    ASSERT_EQ(result, ERR_NONE);
}

HWTEST_F(NetSupplierCallbackProxyTest, RemoveRequest_003, TestSize.Level1)
{
    sptr<NetSupplierCallbackProxy> proxy = new (std::nothrow) NetSupplierCallbackProxy(NULL);
    NetRequest netRequest;
    int32_t result = proxy->RemoveRequest(netRequest);
    ASSERT_EQ(result, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

} // namespace NetManagerStandard
} // namespace OHOS