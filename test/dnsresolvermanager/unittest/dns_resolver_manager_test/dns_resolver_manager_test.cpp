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
    uint16_t netId = 0;
    uint16_t baseTimeoutMsec = 0x00;
    uint8_t retryCount = 0x01;
    std::vector<std::string> servers = {"8.8.8.8", "114.114.114.114"};
    std::vector<std::string> domains = {"dns.163.", "public1.114dns.com"};
    int32_t ret = DelayedSingleton<DnsResolverClient>::GetInstance()->CreateNetworkCache(netId);
    ret = DelayedSingleton<DnsResolverClient>::GetInstance()->SetResolverConfig(netId,
        baseTimeoutMsec, retryCount, servers, domains);
    ret = DelayedSingleton<DnsResolverClient>::GetInstance()->GetAddressesByName(server, addrInfo);
    std::cout << "GetAddressesByName ret:" << ret << std::endl;
    std::cout << "GetAddressesByName size:" << addrInfo.size() << std::endl;
    for (auto s : addrInfo) {
        std::cout << "dnsResolverService GetAddrInfo ip:";
        std::cout << static_cast<int32_t>(s.family_) << std::endl;
        std::cout << s.address_ << std::endl;
    }
}

/**
 * @tc.name: DnsResolverManagerTest001
 * @tc.desc: Test DnsResolverManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(DnsResolverManagerTest, DnsResolverManagerTest002, TestSize.Level1)
{
    std::string server = "www.163.com";
    std::vector<sptr<DnsAddrInfo>> dnsAddrInfos;
    sptr<DnsAddrInfo> hints = std::make_unique<DnsAddrInfo>().release();
    hints->family_ = AF_INET;
    hints->flags_ = AI_PASSIVE;
    hints->protocol_ = 0;
    hints->sockType_ = SOCK_DGRAM;

    DelayedSingleton<DnsResolverClient>::GetInstance()->GetAddrInfo(server, "", hints, dnsAddrInfos);
    std::cout << "dnsAddrInfos size:" << dnsAddrInfos.size() << std::endl;
    for (std::vector<sptr<DnsAddrInfo>>::iterator it = dnsAddrInfos.begin(); it != dnsAddrInfos.end(); it++) {
        sptr<DnsAddrInfo> &t = *it;
        std::cout << "dnsResolverService GetAddrInfo ip:" << std::endl;
        std::cout << "ip type:" << static_cast<int32_t>(t->family_) << std::endl;
        std::cout << "ip address:" << t->addr_ << std::endl;
    }
}

/**
 * @tc.name: DnsResolverManagerTest002
 * @tc.desc: Test DnsResolverManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(DnsResolverManagerTest, DnsResolverManagerTest003, TestSize.Level1)
{
    int32_t netId = 0;
    int32_t result = DelayedSingleton<DnsResolverClient>::GetInstance()->CreateNetworkCache(netId);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: DnsResolverManagerTest003
 * @tc.desc: Test DnsResolverManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(DnsResolverManagerTest, DnsResolverManagerTest004, TestSize.Level1)
{
    int32_t netId = 0;
    int32_t result = DelayedSingleton<DnsResolverClient>::GetInstance()->DestoryNetworkCache(netId);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: DnsResolverManagerTest004
 * @tc.desc: Test DnsResolverManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(DnsResolverManagerTest, DnsResolverManagerTest005, TestSize.Level1)
{
    int32_t netId = 0;
    int32_t result = DelayedSingleton<DnsResolverClient>::GetInstance()->FlushNetworkCache(netId);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: DnsResolverManagerTest005
 * @tc.desc: Test DnsResolverManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(DnsResolverManagerTest, DnsResolverManagerTest006, TestSize.Level1)
{
    int32_t netId = 0;
    uint16_t baseTimeoutMsec = 0x00;
    uint8_t retryCount = 0x01;
    std::vector<std::string> servers = {"8.8.8.8", "114.114.114.114"};
    std::vector<std::string> domains = {"baidu.com", "163.com"};
    int32_t result = DelayedSingleton<DnsResolverClient>::GetInstance()->SetResolverConfig(netId,
        baseTimeoutMsec, retryCount, servers, domains);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: DnsResolverManagerTest005
 * @tc.desc: Test DnsResolverManager SetIfaceConfig.
 * @tc.type: FUNC
 */
HWTEST_F(DnsResolverManagerTest, DnsResolverManagerTest007, TestSize.Level1)
{
    int32_t netId = 0;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    uint16_t baseTimeoutMsec;
    uint8_t retryCount;
    int32_t result = DelayedSingleton<DnsResolverClient>::GetInstance()->GetResolverInfo(netId,
        servers, domains, baseTimeoutMsec, retryCount);
    ASSERT_TRUE(result == 0);
    for (std::string& s : servers) {
        std::cout << "GetResolverInfo server" << s << std::endl;
    }
    for (std::string& s : domains) {
        std::cout << "GetResolverInfo domains" << s << std::endl;
    }
    std::cout << "GetResolverInfo baseTimeoutMsec" << baseTimeoutMsec << std::endl;
    std::cout << "GetResolverInfo retryCount" << static_cast<int32_t>(retryCount) << std::endl;
}
} // namespace NetManagerStandard
} // namespace OHOS