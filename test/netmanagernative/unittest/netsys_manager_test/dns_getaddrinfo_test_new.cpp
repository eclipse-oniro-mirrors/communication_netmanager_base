/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define private public
#include "dns_getaddrinfo.h"
#undef private

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;

class DNSGetaddrinNewTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DNSGetaddrinNewTest::SetUpTestCase() {}

void DNSGetaddrinNewTest::TearDownTestCase() {}

void DNSGetaddrinNewTest::SetUp() {}

void DNSGetaddrinNewTest::TearDown() {}

HWTEST_F(DNSGetaddrinNewTest, CheckHintsNewTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckHintsTest aiFamily = AF_UNSPEC");
    DnsGetAddrInfo getInfo;
    struct AddrInfo addrInfo;
    addrInfo.aiFlags = AI_ALL;
    addrInfo.aiFamily = AF_UNSPEC;
    int32_t ret = getInfo.CheckHints(addrInfo);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSGetaddrinNewTest, CheckHintsNewTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckHintsTest aiFamily = AF_INET");
    DnsGetAddrInfo getInfo;
    struct AddrInfo addrInfo;
    addrInfo.aiFlags = AI_ALL;
    addrInfo.aiFamily = AF_INET;
    int32_t ret = getInfo.CheckHints(addrInfo);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSGetaddrinNewTest, CheckHintsNewTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckHintsTest aiFamily = AF_INET6");
    DnsGetAddrInfo getInfo;
    struct AddrInfo addrInfo;
    addrInfo.aiFlags = AI_ALL;
    addrInfo.aiFamily = AF_INET6;
    int32_t ret = getInfo.CheckHints(addrInfo);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSGetaddrinNewTest, CheckHintsNewTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckHintsTest error family enter");
    DnsGetAddrInfo getInfo;
    struct AddrInfo addrInfo;
    addrInfo.aiFlags = AI_ALL;
    addrInfo.aiFamily = 88;
    int32_t ret = getInfo.CheckHints(addrInfo);
    EXPECT_EQ(ret, EAI_FAMILY);
}

HWTEST_F(DNSGetaddrinNewTest, ParseAddrNewTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest family = AF_INET enter");
    DnsGetAddrInfo getInfo;
    int32_t addrNum = 1;
    int32_t servNum = 1;
    struct ServData servData[2] = {};
    struct AddrData addrData[48] = {};
    addrData[0].family = AF_INET;
    char canon[16] = {0};
    int32_t len = 16;
    std::vector<AddrInfo> out;
    getInfo.ParseAddr(addrNum, servNum, servData, addrData, canon, len, out);
    int32_t res = out.empty();
    EXPECT_EQ(res, false);
}

HWTEST_F(DNSGetaddrinNewTest, ParseAddrNewTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest family = AF_INET6 enter");
    DnsGetAddrInfo getInfo;
    int32_t addrNum = 1;
    int32_t servNum = 1;
    struct ServData servData[2] = {};
    struct AddrData addrData[48] = {};
    addrData[0].family = AF_INET6;
    char canon[16] = {0};
    int32_t len = 0;
    std::vector<AddrInfo> out;
    getInfo.ParseAddr(addrNum, servNum, servData, addrData, canon, len, out);
    int32_t res = out.empty();
    EXPECT_EQ(res, false);
}

HWTEST_F(DNSGetaddrinNewTest, GetAddrInfoNewTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetAddrInfoTest host name empty enter");
    DnsGetAddrInfo getInfo;
    std::string host = "";
    std::string server = "";
    struct AddrInfo addrInfo = {};
    uint16_t netId = 103;
    std::vector<AddrInfo> out;
    int32_t ret = getInfo.GetAddrInfo(host, server, addrInfo, netId, out);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSGetaddrinNewTest, GetAddrInfoNewTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("GetAddrInfoTest hints is ok enter");
    DnsGetAddrInfo getInfo;
    AddrInfo hints = {
        .aiFamily = AF_INET6,
        .aiFlags = AI_ALL,
        .aiProtocol = IPPROTO_UDP,
        .aiSockType = SOCK_DGRAM,
    };
    std::string host = "www.baidu.com";
    std::string server;
    uint16_t netId = 0;
    std::vector<AddrInfo> out;
    int32_t ret = getInfo.GetAddrInfo(host, server, hints, netId, out);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSGetaddrinNewTest, GetAddrInfoNewTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("GetAddrInfoTest server is null enter");
    DnsGetAddrInfo getInfo;
    std::string host = "www.baidu.com";
    std::string server;
    struct AddrInfo addrInfo = {};
    uint16_t netId = 101;
    std::vector<AddrInfo> out;
    int32_t ret = getInfo.GetAddrInfo(host, server, addrInfo, netId, out);
    EXPECT_EQ(ret, EAI_NONAME);
}

} // namespace NetsysNative
} // namespace OHOS