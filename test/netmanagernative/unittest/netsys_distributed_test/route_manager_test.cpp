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

#include "net_manager_constants.h"
#include "route_manager.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
using namespace OHOS::NetManagerStandard;
} // namespace

class RouteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RouteManagerTest::SetUpTestCase() {}

void RouteManagerTest::TearDownTestCase() {}

void RouteManagerTest::SetUp() {}

void RouteManagerTest::TearDown() {}

HWTEST_F(RouteManagerTest, EnableDistributedClientNet001, TestSize.Level1)
{
    std::string virnicAddr = "1.189.55.61";
    std::string iif = "lo";
    auto ret = RouteManager::EnableDistributedClientNet(virnicAddr, iif);
    EXPECT_EQ(ret, 0);
    bool isServer = false;
    ret = RouteManager::DisableDistributedNet(isServer);
    EXPECT_NE(ret, 0);
}

HWTEST_F(RouteManagerTest, EnableDistributedServerNet001, TestSize.Level1)
{
    std::string iif = "lo";
    std::string devIface = "lo";
    std::string dstAddr = "1.189.55.61";
    auto ret = RouteManager::EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_EQ(ret, 0);
    bool isServer = true;
    ret = RouteManager::DisableDistributedNet(isServer);
    EXPECT_NE(ret, 0);
}
} // namespace nmd
} // namespace OHOS