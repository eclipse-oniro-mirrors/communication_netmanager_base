/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "http_proxy_callback_proxy.h"
#include "net_conn_constants.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
using ::testing::_;
using ::testing::Return;

class HttpProxyRemoteObjectMock : public IRemoteObject {
public:
    HttpProxyRemoteObjectMock() : IRemoteObject(u"HttpProxyRemoteObjectMock") {}
    MOCK_METHOD(int32_t, GetObjectRefCount, (), (override));
    MOCK_METHOD(int, SendRequest, (uint32_t, MessageParcel &, MessageParcel &, MessageOption &), (override));
    MOCK_METHOD(bool, IsProxyObject, (), (const, override));
    MOCK_METHOD(bool, IsObjectDead, (), (const, override));
    MOCK_METHOD(std::u16string, GetInterfaceDescriptor, (), (override));
    MOCK_METHOD(bool, CheckObjectLegality, (), (const, override));
    MOCK_METHOD(bool, AddDeathRecipient, (const sptr<DeathRecipient> &), (override));
    MOCK_METHOD(bool, RemoveDeathRecipient, (const sptr<DeathRecipient> &), (override));
    MOCK_METHOD(bool, Marshalling, (OHOS::Parcel &), (const, override));
    MOCK_METHOD(sptr<IRemoteBroker>, AsInterface, (), (override));
    MOCK_METHOD(int, Dump, (int, const std::vector<std::u16string> &), (override));
};

/**
 * @tc.name: SendRequestBranches
 * @tc.desc: Test successful, failed, and null remote HTTP proxy change requests.
 * @tc.type: FUNC
 */
HWTEST(HttpProxyCallbackProxyTest, SendRequestBranches, TestSize.Level1)
{
    auto remote = new HttpProxyRemoteObjectMock();
    sptr<HttpProxyCallbackProxy> proxy = new HttpProxyCallbackProxy(sptr<IRemoteObject>(remote));
    EXPECT_CALL(*remote, SendRequest(_, _, _, _)).WillOnce(Return(ERR_NONE));
    EXPECT_EQ(proxy->OnHttpProxyChange(), ERR_NONE);
    EXPECT_CALL(*remote, SendRequest(_, _, _, _)).WillOnce(Return(ERR_TRANSACTION_FAILED));
    EXPECT_EQ(proxy->OnHttpProxyChange(), ERR_TRANSACTION_FAILED);
    sptr<HttpProxyCallbackProxy> nullProxy = new HttpProxyCallbackProxy(nullptr);
    EXPECT_EQ(nullProxy->OnHttpProxyChange(), NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}
} // namespace NetManagerStandard
} // namespace OHOS
