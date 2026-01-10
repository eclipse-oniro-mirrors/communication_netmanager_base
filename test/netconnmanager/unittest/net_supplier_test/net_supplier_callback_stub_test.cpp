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
#define private public
#include "net_manager_constants.h"
#include "net_supplier_callback_stub.h"
#include "net_supplier_callback_proxy.h"
#include "common_net_conn_callback_test.h"
namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing;
using namespace testing::ext;
static constexpr uint32_t MAX_NET_CAP_NUM = 32;
static constexpr uint32_t MAX_NET_BEARTYPE_NUM = 7;
} // namespace

class NetSupplierCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<NetSupplierCallbackStubTestCb> supplierCbStub_ = nullptr;
};

void NetSupplierCallbackStubTest::SetUpTestCase()
{
    supplierCbStub_ = std::make_shared<NetSupplierCallbackStubTestCb>();
    sptr<NetSupplierCallbackBase> callback = new (std::nothrow) NetSupplierCallbackBaseTestCb();
    supplierCbStub_->RegisterSupplierCallbackImpl(callback);
}

void NetSupplierCallbackStubTest::TearDownTestCase() {}

void NetSupplierCallbackStubTest::SetUp() {}

void NetSupplierCallbackStubTest::TearDown() {}

HWTEST_F(NetSupplierCallbackStubTest, RequestNetwork001, TestSize.Level1)
{
    std::string ident = "testsupid";
    std::set<NetCap> netCaps;
    netCaps.insert(NetCap::NET_CAPABILITY_NOT_METERED);
    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetSupplierCallbackStub::GetDescriptor()), false);
    if (!data.WriteString(ident)) {
        return;
    }
    uint32_t size = static_cast<uint32_t>(netCaps.size());
    if (!data.WriteUint32(size)) {
        return;
    }
    for (auto netCap : netCaps) {
        data.WriteInt32(static_cast<uint32_t>(netCap));
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = supplierCbStub_->OnRemoteRequest(100, data, reply, option);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    ret = supplierCbStub_->OnRemoteRequest(
        static_cast<uint32_t>(SupplierInterfaceCode::NET_SUPPLIER_REQUEST_NETWORK), data, reply, option);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    MessageParcel dataOk;
    if (!dataOk.WriteInterfaceToken(NetSupplierCallbackStub::GetDescriptor())) {
        return;
    }
    if (!dataOk.WriteString(ident)) {
        return;
    }
    size = static_cast<uint32_t>(netCaps.size());
    if (!dataOk.WriteUint32(size)) {
        return;
    }
    for (auto netCap : netCaps) {
        dataOk.WriteInt32(static_cast<uint32_t>(netCap));
    }
    ret = supplierCbStub_->OnRemoteRequest(
        static_cast<uint32_t>(SupplierInterfaceCode::NET_SUPPLIER_REQUEST_NETWORK), dataOk, reply, option);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetSupplierCallbackStubTest, ReleaseNetwork001, TestSize.Level1)
{
    NetRequest netrequest;
    netrequest.ident = "testsupid";
    netrequest.netCaps.insert(NetCap::NET_CAPABILITY_NOT_METERED);

    MessageParcel data;
    ASSERT_NE(data.WriteInterfaceToken(NetSupplierCallbackStub::GetDescriptor()), false);
    bool result = data.WriteUint32(netrequest.uid) && data.WriteUint32(netrequest.requestId) &&
                  data.WriteUint32(netrequest.registerType) && data.WriteString(netrequest.ident);
    if (!result) {
        return;
    }

    uint32_t size = static_cast<uint32_t>(netrequest.bearTypes.size());
    if (!data.WriteUint32(size)) {
        return;
    }
    for (auto netBearType : netrequest.bearTypes) {
        if (!data.WriteInt32(netBearType)) {
            return;
        }
    }

    size = static_cast<uint32_t>(netrequest.netCaps.size());
    if (!data.WriteUint32(size)) {
        return;
    }
    for (auto netCap : netrequest.netCaps) {
        data.WriteInt32(static_cast<uint32_t>(netCap));
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = supplierCbStub_->OnRemoteRequest(
        static_cast<uint32_t>(SupplierInterfaceCode::NET_SUPPLIER_RELEASE_NETWORK), data, reply, option);
    EXPECT_GE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetSupplierCallbackStubTest, OnAddRequest001, TestSize.Level1)
{
    MessageParcel data;

    int32_t uid = 0;
    ASSERT_TRUE(data.WriteUint32(uid));
    int32_t requestId = 0;
    ASSERT_TRUE(data.WriteUint32(requestId));
    int32_t registerType = 0;
    ASSERT_TRUE(data.WriteUint32(registerType));
    std::string ident = "testsupid";
    ASSERT_TRUE(data.WriteString(ident));
    std::vector<uint32_t> netBearTypes = {NetBearType::BEARER_CELLULAR, NetBearType::BEARER_DEFAULT};
    ASSERT_TRUE(data.WriteUInt32Vector(netBearTypes));
    std::vector<uint32_t> netCaps = {NetCap::NET_CAPABILITY_NOT_METERED, NetCap::NET_CAPABILITY_END};
    ASSERT_TRUE(data.WriteUInt32Vector(netCaps));
    MessageParcel reply;
    auto netSupplierCallbackStub = std::make_shared<NetSupplierCallbackStub>();
    EXPECT_EQ(netSupplierCallbackStub->OnAddRequest(data, reply), NETMANAGER_SUCCESS);
}

HWTEST_F(NetSupplierCallbackStubTest, OnAddRequest002, TestSize.Level1)
{
    MessageParcel data;

    int32_t uid = 0;
    ASSERT_TRUE(data.WriteUint32(uid));
    int32_t requestId = 0;
    ASSERT_TRUE(data.WriteUint32(requestId));
    int32_t registerType = 0;
    ASSERT_TRUE(data.WriteUint32(registerType));
    std::string ident = "testsupid";
    ASSERT_TRUE(data.WriteString(ident));
    ASSERT_TRUE(data.WriteUint32(MAX_NET_BEARTYPE_NUM + 1));
    ASSERT_TRUE(data.WriteUint32(0));
    MessageParcel reply;
    auto netSupplierCallbackStub = std::make_shared<NetSupplierCallbackStub>();
    EXPECT_EQ(netSupplierCallbackStub->OnAddRequest(data, reply), NETMANAGER_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetSupplierCallbackStubTest, OnAddRequest003, TestSize.Level1)
{
    MessageParcel data;
    int32_t uid = 0;
    ASSERT_TRUE(data.WriteUint32(uid));
    int32_t requestId = 0;
    ASSERT_TRUE(data.WriteUint32(requestId));
    int32_t registerType = 0;
    ASSERT_TRUE(data.WriteUint32(registerType));
    std::string ident = "testsupid";
    ASSERT_TRUE(data.WriteString(ident));
    ASSERT_TRUE(data.WriteUint32(0));
    ASSERT_TRUE(data.WriteUint32(MAX_NET_CAP_NUM + 1));
    MessageParcel reply;
    auto netSupplierCallbackStub = std::make_shared<NetSupplierCallbackStub>();
    EXPECT_EQ(netSupplierCallbackStub->OnAddRequest(data, reply), NETMANAGER_ERR_INVALID_PARAMETER);
}

} // namespace NetManagerStandard
} // namespace OHOS