/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <arpa/inet.h>
#include <cstdio>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

#define private public
#include "fwmark_client.h"
#undef private
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace nmd;
namespace {
constexpr int32_t NETID_FIRST = 101;
constexpr int32_t NETID_SECOND = 102;
static constexpr const int32_t ERROR_CODE_SOCKETFD_INVALID = -1;
static constexpr const int32_t ERROR_CODE_CONNECT_FAILED = -2;
static constexpr const int32_t ERROR_CODE_SENDMSG_FAILED = -3;
static constexpr const int32_t ERROR_CODE_READ_FAILED = -4;
class ManagerNative : public std::enable_shared_from_this<ManagerNative> {
    DECLARE_DELAYED_SINGLETON(ManagerNative);

public:
    std::shared_ptr<FwmarkClient> GetFwmarkClient();

private:
    std::shared_ptr<FwmarkClient> fwmarkClient_ = nullptr;
};

ManagerNative::ManagerNative()
{
    fwmarkClient_ = std::make_shared<FwmarkClient>();
}

std::shared_ptr<FwmarkClient> ManagerNative::GetFwmarkClient()
{
    return fwmarkClient_;
}

ManagerNative::~ManagerNative() {}
} // namespace
class UnitTestFwmarkClient : public testing::Test {
public:
    std::shared_ptr<FwmarkClient> fwmarkClient = DelayedSingleton<ManagerNative>::GetInstance()->GetFwmarkClient();
};

/**
 * @tc.name: BindSocketTest001
 * @tc.desc: Test FwmarkClient BindSocket.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestFwmarkClient, BindSocketTest001, TestSize.Level1)
{
    int32_t udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int32_t ret = fwmarkClient->BindSocket(udpSocket, NETID_FIRST);
    NETNATIVE_LOGI("UnitTestFwmarkClient BindSocketTest001 ret=%{public}d", ret);
    close(udpSocket);
    udpSocket = -1;
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: BindSocketTest002
 * @tc.desc: Test FwmarkClient BindSocket.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestFwmarkClient, BindSocketTest002, TestSize.Level1)
{
    int32_t tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int32_t ret = fwmarkClient->BindSocket(tcpSocket, NETID_SECOND);
    NETNATIVE_LOGI("UnitTestFwmarkClient BindSocketTest002 ret=%{public}d", ret);
    close(tcpSocket);
    tcpSocket = -1;
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: BindSocketTest003
 * @tc.desc: Test FwmarkClient BindSocket.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestFwmarkClient, BindSocketTest003, TestSize.Level1)
{
    int32_t tcpSocket = -1;
    int32_t ret = fwmarkClient->BindSocket(tcpSocket, NETID_SECOND);
    NETNATIVE_LOGI("UnitTestFwmarkClient BindSocketTest002 ret=%{public}d", ret);
    close(tcpSocket);
    tcpSocket = -1;
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: HandleErrorTest
 * @tc.desc: Test FwmarkClient BindSocket.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestFwmarkClient, HandleErrorTest, TestSize.Level1)
{
    int32_t ret = -1;
    int32_t errorCode = ERROR_CODE_SOCKETFD_INVALID;
    ret = fwmarkClient->HandleError(ret, errorCode);
    EXPECT_EQ(ret, -1);

    errorCode = ERROR_CODE_CONNECT_FAILED;
    ret = fwmarkClient->HandleError(ret, errorCode);
    EXPECT_EQ(ret, -1);

    errorCode = ERROR_CODE_SENDMSG_FAILED;
    ret = fwmarkClient->HandleError(ret, errorCode);
    EXPECT_EQ(ret, -1);

    errorCode = ERROR_CODE_READ_FAILED;
    ret = fwmarkClient->HandleError(ret, errorCode);
    EXPECT_EQ(ret, -1);

    errorCode = 100;
    ret = fwmarkClient->HandleError(ret, errorCode);
    EXPECT_EQ(ret, -1);
}
} // namespace NetsysNative
} // namespace OHOS
