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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "vnic_manager.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class VnicManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void VnicManagerTest::SetUpTestCase() {}

void VnicManagerTest::TearDownTestCase() {}

void VnicManagerTest::SetUp() {}

void VnicManagerTest::TearDown() {}

HWTEST_F(VnicManagerTest, CreateVnicInterface001, TestSize.Level1)
{
    auto result = VnicManager::GetInstance().CreateVnicInterface();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);

    VnicManager::GetInstance().DestroyVnicInterface();
}

HWTEST_F(VnicManagerTest, SetVnicMtu001, TestSize.Level1)
{
    std::string ifName = "";
    int32_t testNumber = 0;
    auto result = VnicManager::GetInstance().SetVnicMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);

    testNumber = 1;
    result = VnicManager::GetInstance().SetVnicMtu(ifName, testNumber);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(VnicManagerTest, SetVnicAddress001, TestSize.Level1)
{
    std::string ifName = "";
    std::string tunAddr = "";
    int32_t testNumber = 0;
    auto result = VnicManager::GetInstance().SetVnicAddress(ifName, tunAddr, testNumber);
    EXPECT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(VnicManagerTest, SetVnicUp001, TestSize.Level1)
{
    auto result = VnicManager::GetInstance().SetVnicUp();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(VnicManagerTest, SetVnicDown001, TestSize.Level1)
{
    auto result = VnicManager::GetInstance().SetVnicDown();
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(VnicManagerTest, InitIfreq001, TestSize.Level1)
{
    ifreq ifr;
    std::string cardName = "";
    auto result = VnicManager::GetInstance().InitIfreq(ifr, cardName);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
