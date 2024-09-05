/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "common_notify_callback_test.h"
#include "dhcp_controller.h"
#include "notify_callback_stub.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
using namespace NetsysNative;
} // namespace

class DhcpControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = std::make_shared<DhcpController>();
};

void DhcpControllerTest::SetUpTestCase() {}

void DhcpControllerTest::TearDownTestCase() {}

void DhcpControllerTest::SetUp() {}

void DhcpControllerTest::TearDown() {}

HWTEST_F(DhcpControllerTest, RegisterNotifyCallbackTest001, TestSize.Level1)
{
    sptr<INotifyCallback> callback = new (std::nothrow) NotifyCallbackTest();
    auto ret = instance_->RegisterNotifyCallback(callback);
    ASSERT_EQ(ret, 0);
}
} // namespace nmd
} // namespace OHOS
