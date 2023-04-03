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

#include <netdb.h>
#include <gtest/gtest.h>
#include <sys/socket.h>

#include "i_net_monitor_callback.h"
#include "net_manager_constants.h"
#define private public
#include "net_monitor.h"
#undef private

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t TEST_NETID = 999;
constexpr int32_t TEST_SOCKETFD = -1;
constexpr int32_t TEST_NORMAL_FD = 9000;
class TestMonitorCallback : public INetMonitorCallback {
public:
    inline void OnHandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect) override
    {
        (void)netDetectionState;
        (void)urlRedirect;
    }
};
} // namespace

class NetMonitorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<INetMonitorCallback> callback_ = std::make_shared<TestMonitorCallback>();
    static inline std::shared_ptr<NetMonitor> instance_ = std::make_shared<NetMonitor>(TEST_NETID, callback_);
};

void NetMonitorTest::SetUpTestCase()
{
    instance_->Start();
}

void NetMonitorTest::TearDownTestCase()
{
    instance_->Stop();
}

void NetMonitorTest::SetUp() {}

void NetMonitorTest::TearDown() {}

HWTEST_F(NetMonitorTest, SetSocketParameterTest001, TestSize.Level1)
{
    int32_t ret = instance_->SetSocketParameter(TEST_SOCKETFD);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    ret = instance_->SetSocketParameter(TEST_NORMAL_FD);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetMonitorTest, IsDetectingTest001, TestSize.Level1)
{
    bool ret = instance_->IsDetecting();
    EXPECT_TRUE(ret);
    instance_->Detection();
    instance_->Stop();
}

HWTEST_F(NetMonitorTest, GetStatusCodeFromResponse001, TestSize.Level1)
{
    std::string str;
    int32_t ret = instance_->GetStatusCodeFromResponse(str);
    EXPECT_EQ(ret, -1);
    str = "12 34";
    ret = instance_->GetStatusCodeFromResponse(str);
    EXPECT_EQ(ret, -1);
    str = "12 \r\n";
    ret = instance_->GetStatusCodeFromResponse(str);
    EXPECT_EQ(ret, -1);
    str = "12 34 \r\n";
    ret = instance_->GetStatusCodeFromResponse(str);
    EXPECT_EQ(ret, 34);
    str = "12 34\r\n";
    ret = instance_->GetStatusCodeFromResponse(str);
    EXPECT_EQ(ret, -1);
    str = "12 34 56 \r\n";
    ret = instance_->GetStatusCodeFromResponse(str);
    EXPECT_EQ(ret, 34);
}

HWTEST_F(NetMonitorTest, SendParallelHttpProbes001, TestSize.Level1)
{
    instance_->SendParallelHttpProbes();
    SUCCEED();
}

HWTEST_F(NetMonitorTest, SendHttpProbe001, TestSize.Level1)
{
    std::string domain;
    std::string urlPath;
    NetDetectionStatus ret = instance_->SendHttpProbe(domain, urlPath, 80);
    EXPECT_EQ(ret, INVALID_DETECTION_STATE);
    domain = "114.114.114.114";
    urlPath = "www.baidu.com";
    ret = instance_->SendHttpProbe(domain, urlPath, 80);
    EXPECT_EQ(ret, INVALID_DETECTION_STATE);
}

HWTEST_F(NetMonitorTest, ConnectIpv4001, TestSize.Level1)
{
    int32_t sockFd = 9000;
    uint16_t port = 80;
    std::string ipAddr = "192.168.111.11";
    int32_t ret = instance_->ConnectIpv4(sockFd, port, ipAddr);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetMonitorTest, ConnectIpv6001, TestSize.Level1)
{
    int32_t sockFd = 9000;
    uint16_t port = 80;
    std::string ipAddr = "192.168.111.11";
    int32_t ret = instance_->ConnectIpv6(sockFd, port, ipAddr);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetMonitorTest, Send001, TestSize.Level1)
{
    int32_t sockFd = 9000;
    std::string domain = "114.114.114.114";
    std::string url = "www.baidu.com";
    int32_t ret = instance_->Send(sockFd, domain, url);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetMonitorTest, Receive001, TestSize.Level1)
{
    int32_t sockFd = 9000;
    std::string probResult;
    int32_t ret = instance_->Receive(sockFd, probResult);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetMonitorTest, dealRecvResult001, TestSize.Level1)
{
    std::string strResponse = "ok";
    NetDetectionStatus ret = instance_->dealRecvResult(strResponse);
    EXPECT_EQ(ret, INVALID_DETECTION_STATE);
}

HWTEST_F(NetMonitorTest, GetIpAddr001, TestSize.Level1)
{
    std::string domain = "www.baidu.com";
    std::string ip_addr = "192.168.111.111";
    int socketType = 1;
    int32_t ret = instance_->GetIpAddr(domain, ip_addr, socketType);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetMonitorTest, GetDefaultNetDetectionUrlFromCfg001, TestSize.Level1)
{
    std::string strUrl;
    int32_t ret = instance_->GetDefaultNetDetectionUrlFromCfg(strUrl);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetMonitorTest, ParseUrl001, TestSize.Level1)
{
    std::string url = "www.baidu.com";
    std::string domain;
    std::string urlPath;
    int32_t ret = instance_->ParseUrl(url, domain, urlPath);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetMonitorTest, GetUrlRedirectFromResponse001, TestSize.Level1)
{
    std::string tmp;
    std::string empty;
    std::string redirFirst = "Location: baidu.com";
    std::string redirSecond = "http baidu.com";
    int ret = instance_->GetUrlRedirectFromResponse(empty, tmp);
    EXPECT_EQ(ret, -1);
    ret = instance_->GetUrlRedirectFromResponse(redirFirst, tmp);
    EXPECT_NE(ret, -1);
    ret = instance_->GetUrlRedirectFromResponse(redirSecond, tmp);
    EXPECT_NE(ret, -1);
}
} // namespace NetManagerStandard
} // namespace OHOS