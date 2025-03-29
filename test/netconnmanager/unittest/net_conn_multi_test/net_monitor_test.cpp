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
        std::make_shared<NetMonitor>(TEST_NETID, BEARER_DEFAULT, NetLinkInfo(), callback_, true);
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

HWTEST_F(NetMonitorTest, GetHttpProbeUrlFromConfig001, TestSize.Level1)
{
    instance_->GetHttpProbeUrlFromConfig();
    EXPECT_FALSE(instance_->httpUrl_.empty());
    EXPECT_FALSE(instance_->httpsUrl_.empty());
    EXPECT_FALSE(instance_->fallbackHttpUrl_.empty());
    EXPECT_FALSE(instance_->fallbackHttpsUrl_.empty());
}

HWTEST_F(NetMonitorTest, StartTest001, TestSize.Level1)
{
    bool ret = instance_->IsDetecting();
    EXPECT_FALSE(ret);
    instance_->Start();
    instance_->Stop();
    ret = instance_->IsDetecting();
    EXPECT_FALSE(ret);
}

HWTEST_F(NetMonitorTest, ProcessDetectionTest001, TestSize.Level1)
{
    NetHttpProbeResult probeResult;
    probeResult.responseCode_ = 204;
    NetDetectionStatus result;
    instance_->ProcessDetection(probeResult, result);
    EXPECT_EQ(result, VERIFICATION_STATE);
    probeResult.responseCode_ = 302;
    instance_->netBearType_ = BEARER_CELLULAR;
    instance_->ProcessDetection(probeResult, result);
    EXPECT_EQ(result, CAPTIVE_PORTAL_STATE);
    probeResult.responseCode_ = 200;
    instance_->ProcessDetection(probeResult, result);
    EXPECT_EQ(result, CAPTIVE_PORTAL_STATE);
    probeResult.responseCode_ = 302;
    instance_->netBearType_ = BEARER_WIFI;
    instance_->detectionDelay_ = 0;
    instance_->ProcessDetection(probeResult, result);
    EXPECT_NE(result, INVALID_DETECTION_STATE);
    instance_->ProcessDetection(probeResult, result);
    EXPECT_NE(result, INVALID_DETECTION_STATE);
    instance_->detectionDelay_ = 10 * 60 * 1000;
    instance_->ProcessDetection(probeResult, result);
    EXPECT_NE(result, INVALID_DETECTION_STATE);
    instance_->isDetecting_ = true;
    instance_->ProcessDetection(probeResult, result);
    EXPECT_NE(result, INVALID_DETECTION_STATE);
    instance_->isDetecting_ = false;
    instance_->ProcessDetection(probeResult, result);
    EXPECT_NE(result, INVALID_DETECTION_STATE);
}

HWTEST_F(NetMonitorTest, DetectionTest001, TestSize.Level1)
{
    instance_->isDetecting_ = true;
    instance_->Detection();
    instance_->isDetecting_ = false;
    instance_->Detection();
    instance_->Stop();
    bool ret = instance_->IsDetecting();
    EXPECT_FALSE(ret);
}

} // namespace NetManagerStandard
} // namespace OHOS