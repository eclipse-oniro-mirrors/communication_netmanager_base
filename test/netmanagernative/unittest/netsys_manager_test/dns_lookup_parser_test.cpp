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
#include "dns_lookup_parse.h"
#undef private

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;

class DNSLookupParserTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DNSLookupParserTest::SetUpTestCase() {}

void DNSLookupParserTest::TearDownTestCase() {}

void DNSLookupParserTest::SetUp() {}

void DNSLookupParserTest::TearDown() {}

void GetNsFromConfTest001()
{
    NETNATIVE_LOGI("GetNsFromConfTest IPV4 enter");
    struct ResolvConf resolvConf;
    resolvConf.ns[0].family = AF_INET;
    int32_t family = 0;
    socklen_t saLen = 0;
    uint32_t nns = 1;
    DnsLookUpParse::GetNsFromConf(&resolvConf, nns, family, saLen);
}

void GetNsFromConfTest002()
{
    NETNATIVE_LOGI("GetNsFromConfTest IPV6 enter");
    struct ResolvConf resolvConf;
    resolvConf.ns[0].family = AF_INET6;
    int32_t family = 0;
    socklen_t saLen = 0;
    uint32_t nns = 1;
    DnsLookUpParse::GetNsFromConf(&resolvConf, nns, family, saLen);
}

void SetSocAddrTest001()
{
    NETNATIVE_LOGI("SetSocAddrTest enter");
    DnsLookUpParse parser;
    int32_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    uint32_t nns = 1;
    parser.SetSocAddr(sock, nns);
}

void SearchNameServerTest001()
{
    NETNATIVE_LOGI("SearchNameServerTest queries zero enter");
    DnsLookUpParse parser;
    struct GetAnswers answer;
    answer.queriesNum = 0;
    int32_t lens[3];
    uint8_t *queries = nullptr;
    int32_t queriesLen = 3;
    parser.SearchNameServer(&answer, lens, &queries, &queriesLen);
}

void SearchNameServerTest002()
{
    NETNATIVE_LOGI("SearchNameServerTest invalid len enter");
    DnsLookUpParse parser;
    struct GetAnswers answer;
    answer.queriesNum = 1;
    int32_t lens[3] = {1, 0, 0};
    uint8_t *queries = nullptr;
    int32_t queriesLen = 3;
    parser.SearchNameServer(&answer, lens, &queries, &queriesLen);
}

void SearchNameServerTest003()
{
    NETNATIVE_LOGI("SearchNameServerTest invalid nns enter");
    DnsLookUpParse parser;
    struct GetAnswers answer;
    answer.queriesNum = 1;
    answer.nns = 0;
    int32_t lens[3] = {0, 0, 0};
    uint8_t *queries = nullptr;
    int32_t queriesLen = 3;
    parser.SearchNameServer(&answer, lens, &queries, &queriesLen);
}

HWTEST_F(DNSLookupParserTest, GetResolvConfTest001, TestSize.Level1)
{
    GetNsFromConfTest001();
    GetNsFromConfTest002();
    NETNATIVE_LOGI("GetResolvConfTest invalid netid enter");
    DnsLookUpParse parser;
    struct ResolvConf resolvConf;
    char search[16] = {0};
    int32_t ret = parser.GetResolvConf(&resolvConf, search, 0, 100);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(DNSLookupParserTest, GetResolvConfTest002, TestSize.Level1)
{
    SetSocAddrTest001();
    NETNATIVE_LOGI("GetResolvConfTest valid netid enter");
    DnsLookUpParse parser;
    struct ResolvConf resolvConf;
    char search[16] = {0};
    int32_t ret = parser.GetResolvConf(&resolvConf, search, 0, 101);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(DNSLookupParserTest, LookupIpLiteralTest001, TestSize.Level1)
{
    SearchNameServerTest001();
    SearchNameServerTest002();
    SearchNameServerTest003();
    NETNATIVE_LOGI("LookupIpLiteralTest IPV6 enter");
    DnsLookUpParse parser;
    struct AddrData addrData;
    int32_t ret = parser.LookupIpLiteral(&addrData, "110.242.68.3", AF_INET6);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupParserTest, LookupIpLiteralTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("LookupIpLiteralTest IPV4 enter");
    DnsLookUpParse parser;
    struct AddrData addrData;
    int32_t ret = parser.LookupIpLiteral(&addrData, "110.242.68.3", AF_INET);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupParserTest, LookupIpLiteralTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("LookupIpLiteralTest invalid host name enter");
    DnsLookUpParse parser;
    struct AddrData addrData;
    int32_t ret = parser.LookupIpLiteral(&addrData, "abcd", AF_INET);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, IsValidHostnameTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsValidHostnameTest invalid nns enter");
    std::string host = "abcd";
    int32_t ret = DnsLookUpParse::IsValidHostname(host);
    EXPECT_EQ(ret, 1);
}

} // namespace NetsysNative
} // namespace OHOS