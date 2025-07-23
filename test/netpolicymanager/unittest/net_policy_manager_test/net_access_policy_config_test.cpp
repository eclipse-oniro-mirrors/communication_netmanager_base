/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <thread>

#include <gtest/gtest.h>

#include "net_access_policy_config.h"

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;
class NetAccessPolicyConfigUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetAccessPolicyConfigUtilsTest::SetUpTestCase() {}

void NetAccessPolicyConfigUtilsTest::TearDownTestCase() {}

void NetAccessPolicyConfigUtilsTest::SetUp() {}

void NetAccessPolicyConfigUtilsTest::TearDown() {}

HWTEST_F(NetAccessPolicyConfigUtilsTest, GetSupplierCallbackTest001, TestSize.Level1)
{
    NetAccessPolicyConfigUtils config;
    std::string content;
    std::string path1 = "etc/netmanager/net_access_policy_config1.json";
    EXPECT_EQ(config.ReadFile(content, path1), false);
    
    std::string path2 = "etc/netmanager/net_access_policy_config.json";
    EXPECT_EQ(config.ReadFile(content, path2), true);
}
}
}