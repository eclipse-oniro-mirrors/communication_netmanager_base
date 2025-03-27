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

#include <algorithm>
#include <gtest/gtest.h>
#include <string>

#include "interface_manager.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "common_notify_callback_test.h"
#include "dns_config_client.h"
#include "net_stats_constants.h"
#include "netsys_native_service.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace NetManagerStandard;
using namespace testing::ext;
static constexpr uint32_t TEST_UID = 1;
static constexpr uint64_t TEST_COOKIE = 1;
static constexpr uint32_t TEST_STATS_TYPE1 = 0;
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
} // namespace

class NetsysNativeServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = std::make_shared<NetsysNativeService>(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
};

void NetsysNativeServiceTest::SetUpTestCase() {}

void NetsysNativeServiceTest::TearDownTestCase() {}

void NetsysNativeServiceTest::SetUp() {}

void NetsysNativeServiceTest::TearDown() {}

HWTEST_F(NetsysNativeServiceTest, EnableDistributedClientNet001, TestSize.Level1)
{
    std::string virnicAddr = "1.189.55.60";
    std::string iif = "lo";
    int32_t ret = instance_->EnableDistributedClientNet(virnicAddr, iif);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    bool isServer = false;
    ret = instance_->DisableDistributedNet(isServer);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, EnableDistributedClientNet002, TestSize.Level1)
{
    std::string virnicAddr = "";
    std::string iif = "";
    int32_t ret = instance_->EnableDistributedClientNet(virnicAddr, iif);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    bool isServer = false;
    ret = instance_->DisableDistributedNet(isServer);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, EnableDistributedServerNet001, TestSize.Level1)
{
    std::string iif = "lo";
    std::string devIface = "lo";
    std::string dstAddr = "1.189.55.61";
    int32_t ret = instance_->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    bool isServer = true;
    ret = instance_->DisableDistributedNet(isServer);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
HWTEST_F(NetsysNativeServiceTest, EnableDistributedServerNet002, TestSize.Level1)
{
    std::string iif = "";
    std::string devIface = "";
    std::string dstAddr = "";
    int32_t ret = instance_->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    bool isServer = true;
    ret = instance_->DisableDistributedNet(isServer);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetsysNative
} // namespace OHOS
