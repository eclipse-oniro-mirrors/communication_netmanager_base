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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "dns_resolver_client.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class DnsResolverManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DnsResolverManagerTest::SetUpTestCase() {}

void DnsResolverManagerTest::TearDownTestCase() {}

void DnsResolverManagerTest::SetUp() {}

void DnsResolverManagerTest::TearDown() {}

/**
 * @tc.name: DnsResolverManagerTest001
 * @tc.desc: Test DnsResolverManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(DnsResolverManagerTest, DnsResolverManagerTest001, TestSize.Level1)
{
    std::string server = "www.163.com";
    std::vector<INetAddr> addrInfo;
    int32_t ret = DelayedSingleton<DnsResolverClient>::GetInstance()->GetAddressesByName(server, addrInfo);
    std::cout << "GetAddressesByName ret:" << ret << std::endl;
    std::cout << "GetAddressesByName size:" << addrInfo.size() << std::endl;
    for (auto s : addrInfo) {
        std::cout << "dnsResolverService GetAddrInfo ip:";
        std::cout << static_cast<int32_t>(s.family_) << std::endl;
        std::cout << s.address_ << std::endl;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS