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

#include "net_dns_health_callback_proxy.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "net_manager_constants.h"

namespace OHOS {
namespace NetsysNative {
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

class NetDnsHealthCallbackProxyTest : public testing::Test {
protected:
    RemoteObjectMocker *remoteObjectMocker;
    sptr<NetDnsHealthCallbackProxy> proxy;

    void SetUp() override
    {
        remoteObjectMocker = new RemoteObjectMocker();
        sptr<IRemoteObject> impl(remoteObjectMocker);
        proxy = new (std::nothrow) NetDnsHealthCallbackProxy(impl);
    }

    void TearDown() override
    {
        remoteObjectMocker = nullptr;
        proxy = nullptr;
    }
};

HWTEST_F(NetDnsHealthCallbackProxyTest, NetDnsHealthCallbackProxyTest_NetDnsHealthReport_01, TestSize.Level0)
{
    NetDnsHealthReport dnsResultReport;
    int32_t ret = proxy->OnDnsHealthReport(dnsResultReport);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetsysNative
} // namespace OHOS