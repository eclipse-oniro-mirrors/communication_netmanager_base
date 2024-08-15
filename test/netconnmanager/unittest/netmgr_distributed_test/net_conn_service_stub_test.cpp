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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "common_net_conn_callback_test.h"
#include "net_conn_service_stub_test.h"
#include "net_interface_callback_stub.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetConnServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetConnServiceStub> instance_ = std::make_shared<MockNetConnServiceStub>();
    static int32_t SendRemoteRequest(MessageParcel &data, ConnInterfaceCode code);
};

void NetConnServiceStubTest::SetUpTestCase() {}

void NetConnServiceStubTest::TearDownTestCase() {}

void NetConnServiceStubTest::SetUp() {}

void NetConnServiceStubTest::TearDown() {}

int32_t NetConnServiceStubTest::SendRemoteRequest(MessageParcel &data, ConnInterfaceCode code)
{
    MessageParcel reply;
    MessageOption option;
    return instance_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
}

/**
 * @tc.name: OnEnableDistributedClientNet001
 * @tc.desc: Test OnEnableDistributedClientNet Branch.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceStubTest, OnEnableDistributedClientNet001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    std::string virnicAddr = "1.189.55.61";
    if (!data.WriteString(virnicAddr)) {
        return;
    }
    std::string iif = "wlan0";
    if (!data.WriteString(iif)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, ConnInterfaceCode::CMD_NM_ENABLE_DISTRIBUTE_CLIENT_NET);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    MessageParcel data1;
    if (!data1.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    bool isServer = false;
    if (!data1.WriteBool(isServer)) {
        return;
    }
    ret = SendRemoteRequest(data1, ConnInterfaceCode::CMD_NM_DISABLE_DISTRIBUTE_NET);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: OnEnableDistributedServerNet001
 * @tc.desc: Test OnEnableDistributedServerNet Branch.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnServiceStubTest, OnEnableDistributedServerNet001, TestSize.Level1)
{
    NetManagerBaseAccessToken token;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    std::string iif = "lo";
    std::string devIface = "lo";
    std::string dstAddr = "1.189.55.61";
    if (!data.WriteString(iif)) {
        return;
    }
    if (!data.WriteString(devIface)) {
        return;
    }
    if (!data.WriteString(dstAddr)) {
        return;
    }
    int32_t ret = SendRemoteRequest(data, ConnInterfaceCode::CMD_NM_ENABLE_DISTRIBUTE_SERVER_NET);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    MessageParcel data1;
    if (!data1.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        return;
    }
    bool isServer = true;
    if (!data1.WriteBool(isServer)) {
        return;
    }
    ret = SendRemoteRequest(data1, ConnInterfaceCode::CMD_NM_DISABLE_DISTRIBUTE_NET);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS