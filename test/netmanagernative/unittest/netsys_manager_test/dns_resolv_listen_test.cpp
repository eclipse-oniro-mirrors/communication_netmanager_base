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

#include "dns_config_client.h"
#include "dns_param_cache.h"
#include "dns_resolv_listen.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;
class DnsResolvListenTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DnsResolvListenTest::SetUpTestCase() {}

void DnsResolvListenTest::TearDownTestCase() {}

void DnsResolvListenTest::SetUp() {}

void DnsResolvListenTest::TearDown() {}

HWTEST_F(DnsResolvListenTest, SetResolverConfigTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetResolverConfigTest001 enter");
    DnsParamCache dnsParCache;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    servers.resize(MAX_SERVER_NUM + 1);
    uint16_t netId = 1;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t ret = dnsParCache.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    EXPECT_EQ(ret, -ENOENT);
}
} // namespace NetsysNative
} // namespace OHOS
