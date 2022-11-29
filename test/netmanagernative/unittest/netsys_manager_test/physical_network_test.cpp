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

#include "netnative_log_wrapper.h"
#define private public
#define protected public
#include "physical_network.h"
#undef private
#undef protected

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;
class PhysicalNetworkTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void PhysicalNetworkTest::SetUpTestCase() {}

void PhysicalNetworkTest::TearDownTestCase() {}

void PhysicalNetworkTest::SetUp() {}

void PhysicalNetworkTest::TearDown() {}

HWTEST_F(PhysicalNetworkTest, AddInterfaceTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("AddInterfaceTest001 enter");
    PhysicalNetwork physicNetwork(2, NetworkPermission::PERMISSION_NETWORK);
    std::string interfaceName1 = "eth0";
    int32_t ret = physicNetwork.AddInterface(interfaceName1);
    EXPECT_EQ(ret, 0);
    physicNetwork.AddDefault();
    physicNetwork.interfaces_.insert(interfaceName1);
    ret = physicNetwork.AddInterface(interfaceName1);
    EXPECT_EQ(ret, 0);
    std::string interfaceName2 = "eth1";
    ret = physicNetwork.AddInterface(interfaceName2);
    EXPECT_EQ(ret, 0);

    ret = physicNetwork.RemoveInterface(interfaceName1);
    physicNetwork.RemoveDefault();
    physicNetwork.RemoveInterface(interfaceName2);
    std::string interfaceName3 = "eth2";
    physicNetwork.RemoveInterface(interfaceName3);
    physicNetwork.IsPhysical();
    physicNetwork.GetPermission();
    physicNetwork.GetNetworkType();
}
} // namespace NetsysNative
} // namespace OHOS
