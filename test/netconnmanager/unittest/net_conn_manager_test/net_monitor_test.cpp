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

#include <gtest/gtest.h>

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
}

HWTEST_F(NetMonitorTest, IsDetectingTest001, TestSize.Level1)
{
    bool ret = instance_->IsDetecting();
    EXPECT_TRUE(ret);
    instance_->Detection();
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
} // namespace NetManagerStandard
} // namespace OHOS