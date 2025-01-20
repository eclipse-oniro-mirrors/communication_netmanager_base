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
    static inline std::shared_ptr<NetMonitor> instance_ =
        std::make_shared<NetMonitor>(TEST_NETID, BEARER_DEFAULT, NetLinkInfo(), callback_, true, false);
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

HWTEST_F(NetMonitorTest, IsDetectingTest001, TestSize.Level1)
{
    bool ret = instance_->IsDetecting();
    EXPECT_TRUE(ret);
    instance_->Detection();
    instance_->Stop();
}

HWTEST_F(NetMonitorTest, SendHttpProbe001, TestSize.Level1)
{
    std::string domain;
    std::string urlPath;
    NetHttpProbeResult probeResult = instance_->SendProbe();
    EXPECT_EQ(probeResult.IsFailed(), true);
}

HWTEST_F(NetMonitorTest, SendHttpProbe002, TestSize.Level1)
{
    std::string domain;
    std::string urlPath;
    instance_->isFallbackProbeWithProxy_ = false;
    NetHttpProbeResult probeResult = instance_->SendProbe();
    EXPECT_EQ(probeResult.IsFailed(), true);
    instance_->isFallbackProbeWithProxy_ = true;
    probeResult = instance_->SendProbe();
    EXPECT_EQ(probeResult.IsFailed(), true);
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult001, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {200, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {200, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {200, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsNeedPortal());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult002, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {0, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {200, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {200, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsNeedPortal());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult003, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {0, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {0, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {200, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsNeedPortal());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult004, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {0, url};
    NetHttpProbeResult httpsResult = {204, url};
    NetHttpProbeResult fallbackHttpResult = {0, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsSuccessful());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult005, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {0, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {0, url};
    NetHttpProbeResult fallbackHttpsResult = {204, url};
    NetHttpProbeResult fallbackProxyHttpResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsSuccessful());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult006, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {0, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {0, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {204, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsSuccessful());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult007, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {204, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {204, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsSuccessful());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult008, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {0, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {204, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {204, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_TRUE(result.IsSuccessful());
}

HWTEST_F(NetMonitorTest, ProcessThreadDetectResult009, TestSize.Level1)
{
    std::string url = "test";
    NetHttpProbeResult httpResult = {0, url};
    NetHttpProbeResult httpsResult = {0, url};
    NetHttpProbeResult fallbackHttpResult = {0, url};
    NetHttpProbeResult fallbackHttpsResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpResult = {0, url};
    NetHttpProbeResult fallbackProxyHttpsResult = {0, url};
    NetHttpProbeResult result = instance_->ProcessThreadDetectResult(httpResult, httpsResult, fallbackHttpResult,
        fallbackHttpsResult, fallbackProxyHttpResult, fallbackProxyHttpsResult);
    EXPECT_FALSE(result.IsSuccessful());
}

HWTEST_F(NetMonitorTest, GetHttpProbeUrlFromConfig001, TestSize.Level1)
{
    instance_->GetHttpProbeUrlFromConfig();
    EXPECT_FALSE(instance_->httpUrl_.empty());
    EXPECT_FALSE(instance_->httpsUrl_.empty());
    EXPECT_FALSE(instance_->fallbackHttpUrl_.empty());
    EXPECT_FALSE(instance_->fallbackHttpsUrl_.empty());
}

} // namespace NetManagerStandard
} // namespace OHOS