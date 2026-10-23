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

#include <thread>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "net_conn_client.h"
#include "common_mock_net_conn_service.h"
#include "net_conn_constants.h"

namespace OHOS::NetManagerStandard {
namespace {
using namespace testing;
using namespace testing::ext;

class NetConnProxyCallbackConcurrencyTest : public Test {};

bool IsNotificationLockAvailableFromAnotherThread(NetConnClient &client)
{
    bool available = false;
    std::thread worker([&available, &client]() {
        available = client.httpProxyNotifyMutex_.try_lock();
        if (available) {
            client.httpProxyNotifyMutex_.unlock();
        }
    });
    worker.join();
    return available;
}

/**
 * @tc.name: ProxyCallbacksDoNotHoldNotificationLock
 * @tc.desc: Verify registration replay and change callbacks permit proxy operations on another thread.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnProxyCallbackConcurrencyTest, ProxyCallbacksDoNotHoldNotificationLock, TestSize.Level1)
{
    NetConnClient client;
    uint32_t appId = 0;
    bool appUnlocked = false;
    client.RegisterAppHttpProxyCallback([&appUnlocked, &client](const HttpProxy &) {
        appUnlocked = IsNotificationLockAvailableFromAnotherThread(client);
    }, appId, true);
    EXPECT_TRUE(appUnlocked);
    HttpProxy appProxy;
    appProxy.SetHost("app.example");
    client.SetAppHttpProxy(appProxy);
    EXPECT_TRUE(appUnlocked);
    client.UnregisterAppHttpProxyCallback(appId);
    client.SetAppHttpProxy(HttpProxy());

    auto service = sptr<MockINetConnService>::MakeSptr();
    client.NetConnService_ = service;
    HttpProxy current;
    EXPECT_CALL(*service, RegisterHttpProxyCallback(_)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_CALL(*service, UnregisterHttpProxyCallback(_)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_CALL(*service, GetDefaultHttpProxy(_, _)).WillRepeatedly(Invoke([&current](int32_t, HttpProxy &out) {
        out = current;
        return NETMANAGER_SUCCESS;
    }));
    uint32_t defaultId = 0;
    bool defaultUnlocked = false;
    bool observedDefaultChange = false;
    client.RegisterDefaultHttpProxyCallback(
        [&defaultUnlocked, &observedDefaultChange, &client](const HttpProxy &proxy) {
            defaultUnlocked = IsNotificationLockAvailableFromAnotherThread(client);
            observedDefaultChange = proxy.GetHost() == "default.example";
        },
        defaultId);
    EXPECT_TRUE(defaultUnlocked);
    current.SetHost("default.example");
    client.defaultHttpProxyCallbackStub_->OnHttpProxyChange();
    EXPECT_TRUE(defaultUnlocked);
    EXPECT_TRUE(observedDefaultChange);
    client.UnregisterDefaultHttpProxyCallback(defaultId);
}

/**
 * @tc.name: RestoreHttpProxyCallbackFailureKeepsSubscription
 * @tc.desc: Verify failed recovery keeps the callback and covers retry outcomes.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnProxyCallbackConcurrencyTest, RestoreHttpProxyCallbackFailureKeepsSubscription, TestSize.Level1)
{
    NetConnClient client;
    auto service = sptr<MockINetConnService>::MakeSptr();
    client.NetConnService_ = service;
    EXPECT_CALL(*service, RegisterHttpProxyCallback(_))
        .WillOnce(Return(NETMANAGER_SUCCESS))
        .WillOnce(Return(NETMANAGER_ERR_INTERNAL))
        .WillOnce(Return(NETMANAGER_ERR_INTERNAL))
        .WillOnce(Return(NETMANAGER_ERR_INTERNAL))
        .WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_CALL(*service, UnregisterHttpProxyCallback(_)).WillOnce(Return(NETMANAGER_SUCCESS));
    EXPECT_CALL(*service, GetDefaultHttpProxy(_, _)).WillRepeatedly(Return(NETMANAGER_SUCCESS));
    uint32_t callbackId = 0;
    ASSERT_EQ(client.RegisterDefaultHttpProxyCallback([](const HttpProxy &) {}, callbackId), NETMANAGER_SUCCESS);
    auto stub = client.defaultHttpProxyCallbackStub_;
    client.RecoverCallbackAndGlobalProxy();
    EXPECT_EQ(client.defaultHttpProxyCallbackStub_, stub);
    client.RecoverCallbackAndGlobalProxy();
    EXPECT_EQ(client.defaultHttpProxyCallbackStub_, stub);
    client.UnregisterDefaultHttpProxyCallback(callbackId);
}
} // namespace
} // namespace OHOS::NetManagerStandard
