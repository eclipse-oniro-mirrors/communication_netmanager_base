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

#include "netmanager_base_permission.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetManagerPermissionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetManagerPermissionTest::SetUpTestCase() {}

void NetManagerPermissionTest::TearDownTestCase() {}

void NetManagerPermissionTest::SetUp() {}

void NetManagerPermissionTest::TearDown() {}

HWTEST_F(NetManagerPermissionTest, CheckPermissionTest001, TestSize.Level1)
{
    auto ret = NetManagerPermission::CheckPermission({});
    EXPECT_FALSE(ret);
}

HWTEST_F(NetManagerPermissionTest, CheckPermissionWithCacheTest001, TestSize.Level1)
{
    auto ret = NetManagerPermission::CheckPermissionWithCache({});
    EXPECT_FALSE(ret);
}
} // namespace NetManagerStandard
} // namespace OHOS