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
#include "netsys_native_service_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetsysNative {
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

sptr<INetsysService> GetProxyTest()
{
    NETNATIVE_LOGE("Get samgr >>>>>>>>>>>>>>>>>>>>>>>>>>");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    NETNATIVE_LOGI("Get samgr %{public}p", samgr.GetRefPtr());
    std::cout << "Get samgr  "<< samgr.GetRefPtr() << std::endl;

    auto remote = samgr->GetSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    NETNATIVE_LOGI("Get remote %{public}p", remote.GetRefPtr());
    std::cout << "Get remote "<< remote.GetRefPtr() << std::endl;

    auto proxy = iface_cast<NetsysNative::INetsysService>(remote);
    NETNATIVE_LOGI("Get proxy %{public}p", proxy.GetRefPtr());
    std::cout << "Get proxy "<<proxy.GetRefPtr()<<std::endl;

    return proxy;
}

HWTEST_F(NetworkRouteTest, NetworkRouteTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxyTest();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    NETNATIVE_LOGE("NetworkRouteTest001 NetworkRouteTest001 NetworkRouteTest001");
    EXPECT_TRUE(ret == 0);
}

HWTEST_F(NetworkRouteTest, NetworkRouteTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxyTest();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    NETNATIVE_LOGE("NetworkRouteTest002 NetworkRouteTest002 NetworkRouteTest002");
    EXPECT_TRUE(ret == 0);
}
} // namespace NetsysNative
} // namespace OHOS