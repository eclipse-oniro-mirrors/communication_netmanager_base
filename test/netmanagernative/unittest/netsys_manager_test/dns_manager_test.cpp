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

#include "dns_manager.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
std::string INTERFACE_NAME = "interface_name";
std::string INFO = "info";
const uint16_t NET_ID = 2;
uint16_t BASE_TIMEOUT_MILLIS = 2000;
uint8_t RETRY_COUNT = 3;
} // namespace

class DnsManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DnsManagerTest::SetUpTestCase() {}

void DnsManagerTest::TearDownTestCase() {}

void DnsManagerTest::SetUp() {}

void DnsManagerTest::TearDown() {}

HWTEST_F(DnsManagerTest, InterfaceTest001, TestSize.Level1)
{
    DnsManager dnsManager;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    int32_t ret = dnsManager.SetResolverConfig(NET_ID, BASE_TIMEOUT_MILLIS,
                                                RETRY_COUNT, servers, domains);
    EXPECT_NE(ret, 0);

    ret = dnsManager.GetResolverConfig(NET_ID, servers, domains, BASE_TIMEOUT_MILLIS, RETRY_COUNT);
    EXPECT_NE(ret, 0);

    ret = dnsManager.CreateNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);

    dnsManager.SetDefaultNetwork(NET_ID);

    dnsManager.ShareDnsSet(NET_ID);

    dnsManager.StartDnsProxyListen();

    dnsManager.StopDnsProxyListen();

    dnsManager.GetDumpInfo(INFO);

    ret = dnsManager.DestroyNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);
}
} // namespace nmd
} // namespace OHOS
