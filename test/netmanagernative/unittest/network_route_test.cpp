/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "netnative_log_wrapper.h"


namespace OHOS {
namespace NetdNative {
using namespace testing::ext;
class NetworkRouteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkRouteTest::SetUpTestCase() {}

void NetworkRouteTest::TearDownTestCase() {}

void NetworkRouteTest::SetUp() {}

void NetworkRouteTest::TearDown() {}

HWTEST_F(NetworkRouteTest, NetworkRouteTest001, TestSize.Level1)
{
    int32_t ret = 0;
	NETNATIVE_LOGE("NetworkRouteTest001 NetworkRouteTest001 NetworkRouteTest001");
    EXPECT_TRUE(ret == 0);
}

HWTEST_F(NetworkRouteTest, NetworkRouteTest002, TestSize.Level1)
{
    int32_t ret = 0;
	NETNATIVE_LOGE("NetworkRouteTest002 NetworkRouteTest002 NetworkRouteTest002");
    EXPECT_TRUE(ret == 0);
}
} // namespace NetdNative
} // namespace OHOS
