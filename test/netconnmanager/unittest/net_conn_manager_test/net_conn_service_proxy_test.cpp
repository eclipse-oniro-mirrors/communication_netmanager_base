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
#include <memory>
#include "net_manager_constants.h"
#include "net_conn_service_proxy.h"
#include "net_all_capabilities.h"
#include "net_supplier_callback_stub.h"
#include "net_conn_callback_stub.h"
#include "i_net_detection_callback.h"
#include "i_net_conn_service.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
constexpr uint64_t OUTOFFRANGECODE = 100;
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
            case INetConnService::CMD_NM_GET_IFACE_NAMES:
            case INetConnService::CMD_NM_GET_SPECIFIC_NET:
            case INetConnService::CMD_NM_GET_ALL_NETS:
            case INetConnService::CMD_NM_GET_ADDRESSES_BY_NAME:
            case INetConnService::CMD_NM_GET_NET_ID_BY_IDENTIFIER:
            reply.WriteUint32(NETMANAGER_SUCCESS);
            break;

            case INetConnService::CMD_NM_GET_IFACENAME_BY_TYPE:
            reply.WriteString(TEST_HOST);
            break;

            case INetConnService::CMD_NM_GETDEFAULTNETWORK:
            reply.WriteInt32(TEST_NETID);
            break;

            case INetConnService::CMD_NM_HASDEFAULTNET:
            case INetConnService::CMD_NM_IS_DEFAULT_NET_METERED:
            reply.WriteBool(true);
            break;

            case INetConnService::CMD_NM_GET_CONNECTION_PROPERTIES: {
                NetLinkInfo linkInfo;
                linkInfo.ifaceName_ = "ifacename_test";
                linkInfo.Marshalling(reply);
            }
            break;

            case INetConnService::CMD_NM_GET_NET_CAPABILITIES: {
                NetAllCapabilities netCap;
                netCap.Marshalling(reply);
            }

            case INetConnService::CMD_NM_GET_ADDRESS_BY_NAME: {
                INetAddr addr;
                addr.Marshalling(reply);
            }

            case INetConnService::CMD_NM_GET_HTTP_PROXY: {
                HttpProxy httpProxy;
                httpProxy.Marshalling(reply);
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

class NetDetectionTestCallback : public IRemoteStub<INetDetectionCallback> {
public:
    int32_t OnNetDetectionResultChanged(NetDetectionResultCode detectionResult, const std::string &urlRedirect) override
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
    static inline sptr<INetConnCallback> netConnCallback_ = new (std::nothrow) NetConnTestCallback();
    static inline sptr<INetDetectionCallback> detectionCallback_ = new (std::nothrow) NetDetectionTestCallback();
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
}
} // namespace NetManagerStandard
} // namespace OHOS