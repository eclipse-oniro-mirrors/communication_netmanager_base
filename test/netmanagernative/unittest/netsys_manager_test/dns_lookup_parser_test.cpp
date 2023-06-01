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

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "dns_lookup_parse.cpp"
#include "dns_param_cache.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace testing::ext;
using namespace OHOS::nmd;
static constexpr const uint32_t MAX_REQUESTDATA_LEN = 512;
static constexpr int32_t RR_CNAME = 5;
void SearchNameServerTest001(const std::shared_ptr<DnsLookUpParse> &ins)
{
    const uint32_t answerNns = 2;
    GetAnswers getAnswers[2] = {};
    getAnswers[0].queriesNum = 1;
    getAnswers[0].nns = answerNns;
    int32_t answersLens[2] = {0};
    uint8_t querieData[2][2] = {{1, 2}, {2, 3}};
    uint8_t *queryData = querieData[0];
    uint8_t *data[1] = {queryData};
    int32_t queriesLens[8] = {0};
    ins->SearchNameServer(getAnswers, answersLens, data, queriesLens);
}

void DnsGetAnswersTest001(const std::shared_ptr<DnsLookUpParse> &ins)
{
    GetAnswers getAnswers;
    uint8_t *queries[ARG_INDEX_2] = {0};
    int32_t queriesLens[ARG_INDEX_2] = {0};
    uint8_t *answers[ARG_INDEX_2] = {0};
    int32_t answersLens[ARG_INDEX_2] = {0};
    int32_t servFailRetry = 0;
    ins->DnsGetAnswers(getAnswers, queries, queriesLens, answers, answersLens, servFailRetry);
}

void SetSocAddrTest001(const std::shared_ptr<DnsLookUpParse> &ins)
{
    uint32_t nns = 0;
    int socketFd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    ASSERT_GE(socketFd, 0);
    ins->SetSocAddr(socketFd, nns);
    close(socketFd);
    socketFd = -1;
}

void SetSocAddrTest002(const std::shared_ptr<DnsLookUpParse> &ins)
{
    uint32_t nns = 2;
    int socketFd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    ASSERT_GE(socketFd, 0);
    ins->SetSocAddr(socketFd, nns);
    close(socketFd);
    socketFd = -1;
}
} // namespace
class DNSLookupParserTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DnsLookUpParse> instance_ = nullptr;
};

void DNSLookupParserTest::SetUpTestCase() {}

void DNSLookupParserTest::TearDownTestCase() {}

void DNSLookupParserTest::SetUp()
{
    instance_ = std::make_shared<DnsLookUpParse>();
}

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
    NETNATIVE_LOGI("SetSocAddrTest enter");
    DnsLookUpParse parser;
    int32_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    uint32_t nns = 1;
    parser.SetSocAddr(sock, nns);
    NETNATIVE_LOGI("GetResolvConfTest valid netid enter");
    struct ResolvConf resolvConf;
    char search[16] = {0};
    int32_t ret = parser.GetResolvConf(&resolvConf, search, 0, 101);
    close(sock);
    sock = -1;
    EXPECT_EQ(ret, -1);
}

HWTEST_F(DNSLookupParserTest, GetResolvConfTest003, TestSize.Level1)
{
    struct ResolvConf resolvConf;
    auto ret = instance_->GetResolvConf(&resolvConf, nullptr, 0, 100);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(DNSLookupParserTest, GetNsFromConfTest001, TestSize.Level1)
{
    ResolvConf cfg;
    AddrData data;
    int32_t family = 0;
    uint32_t nns = 0;
    socklen_t saLen = 0;
    data.family = AF_INET;
    cfg.ns[0] = data;
    cfg.nns = 1;
    instance_->GetNsFromConf(&cfg, nns, family, saLen);
    EXPECT_EQ(nns, cfg.nns);
}

HWTEST_F(DNSLookupParserTest, GetNsFromConfTest002, TestSize.Level1)
{
    ResolvConf cfg;
    AddrData data;
    int32_t family = 0;
    uint32_t nns = 0;
    socklen_t saLen = 0;
    data.family = AF_INET6;
    cfg.ns[0] = data;
    cfg.nns = 1;
    instance_->GetNsFromConf(&cfg, nns, family, saLen);
    EXPECT_EQ(family, AF_INET6);
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

HWTEST_F(DNSLookupParserTest, LookupIpLiteralTest004, TestSize.Level1)
{
    struct AddrData addrData;
    std::string name = "abc";
    name[0] = 0;
    auto ret = instance_->LookupIpLiteral(&addrData, name, AF_INET);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, DnsSendQueriesTest001, TestSize.Level1)
{
    DnsGetAnswersTest001(instance_);
    SearchNameServerTest001(instance_);
    GetAnswers getAnswers;
    getAnswers.timeOut = 0;
    getAnswers.attempts = 0;
    uint8_t *queries[ARG_INDEX_2] = {0};
    int32_t queriesLens[ARG_INDEX_2] = {0};
    uint8_t *answers[ARG_INDEX_2] = {0};
    int32_t answersLens[ARG_INDEX_2] = {0};
    auto ret = instance_->DnsSendQueries(getAnswers, queries, queriesLens, answers, answersLens);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, IsValidHostnameTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsValidHostnameTest invalid nns enter");
    std::string host = "abcd";
    int32_t ret = DnsLookUpParse::IsValidHostname(host);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupParserTest, IsValidHostnameTest002, TestSize.Level1)
{
    SetSocAddrTest001(instance_);
    SetSocAddrTest002(instance_);
    std::string host = "a";
    for (uint32_t i = 0; i <= 255; i++) {
        host.append("a");
    }
    int32_t ret = instance_->IsValidHostname(host);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, ResMSendRcTest001, TestSize.Level1)
{
    int32_t queriesNum = 1;
    uint8_t *queries[ARG_INDEX_2] = {0};
    int32_t queriesLens[ARG_INDEX_2] = {0};
    uint8_t *answers[ARG_INDEX_2] = {0};
    int32_t answersLens[ARG_INDEX_2] = {0};
    int32_t answersSize = 2;
    ResolvConf conf;
    AddrData data;
    data.family = AF_INET6;
    conf.ns[0] = data;
    conf.nns = 1;
    int32_t netId = 100;
    auto ret = instance_->ResMSendRc(queriesNum, queries, queriesLens, answers, answersLens, answersSize, &conf, netId);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, ResMSendRcTest002, TestSize.Level1)
{
    int32_t queriesNum = 1;
    uint8_t *queries[ARG_INDEX_2] = {0};
    int32_t queriesLens[ARG_INDEX_2] = {0};
    uint8_t *answers[ARG_INDEX_2] = {0};
    int32_t answersLens[ARG_INDEX_2] = {0};
    int32_t answersSize = 2;
    ResolvConf conf;
    AddrData data;
    data.family = AF_INET;
    conf.ns[0] = data;
    conf.nns = 1;
    int32_t netId = 100;
    auto ret = instance_->ResMSendRc(queriesNum, queries, queriesLens, answers, answersLens, answersSize, &conf, netId);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, DnsExpand001, TestSize.Level1)
{
    uint8_t base[ARG_INDEX_2] = {0};
    uint8_t end[ARG_INDEX_2] = {0};
    char dest[MAX_REQUESTDATA_LEN] = {0};
    int32_t space = 0;
    int32_t ret = instance_->DnsExpand(base, end, end, dest, space); // src == end
    EXPECT_EQ(ret, INVALID_LENGTH);
}

HWTEST_F(DNSLookupParserTest, DnsExpand002, TestSize.Level1)
{
    uint8_t base[ARG_INDEX_2] = {0};
    uint8_t end[ARG_INDEX_2] = {0};
    uint8_t src[ARG_INDEX_2] = {0, 1};
    char dest[MAX_REQUESTDATA_LEN] = {0};
    int32_t space = 0;
    int32_t ret = instance_->DnsExpand(base, end, src, dest, space);
    EXPECT_EQ(ret, INVALID_LENGTH);
}

HWTEST_F(DNSLookupParserTest, DnsExpand003, TestSize.Level1)
{
    uint8_t base[ARG_INDEX_2] = {0};
    uint8_t end[ARG_INDEX_2] = {0};
    uint8_t src[ARG_INDEX_2] = {0, 1};
    char dest[MAX_REQUESTDATA_LEN] = {0};
    int32_t space = -1;
    int32_t ret = instance_->DnsExpand(base, end, src, dest, space);
    EXPECT_EQ(ret, INVALID_LENGTH);
}

HWTEST_F(DNSLookupParserTest, DnsExpand004, TestSize.Level1)
{
    const int32_t space = 10;
    uint8_t src[] = "a";
    uint32_t srcSize = sizeof("a");
    char dest[space] = {0};
    int32_t ret = instance_->DnsExpand(&src[0], &src[srcSize], src, dest, space);
    EXPECT_EQ(ret, INVALID_LENGTH);
}

HWTEST_F(DNSLookupParserTest, DnsExpand005, TestSize.Level1)
{
    const int32_t space = 10;
    uint8_t src[] = "abcdefghifaddedg";
    uint32_t srcSize = sizeof("abcdefghifaddedg");
    char dest[space] = {0};
    src[0] = src[0] | MASK_HIGH_TWO_BITS;
    src[1] = 0xff;
    int32_t ret = instance_->DnsExpand(&src[0], &src[srcSize], src, dest, space);
    EXPECT_EQ(ret, INVALID_LENGTH);
}

HWTEST_F(DNSLookupParserTest, DnsExpand006, TestSize.Level1)
{
    const int32_t space = 10;
    uint8_t src[] = "abcdefghifaddedg";
    uint32_t srcSize = sizeof("abcdefghifaddedg");
    char dest[space] = {0};
    src[0] = 0x00;
    src[0] = src[0] & (~MASK_HIGH_TWO_BITS);
    src[1] = 0xff;
    src[1] = src[1] & (~MASK_HIGH_TWO_BITS);
    int32_t ret = instance_->DnsExpand(&src[0], &src[srcSize], src, dest, space);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupParserTest, DnsExpand007, TestSize.Level1)
{
    const int32_t space = 10;
    uint8_t src[] = "abcdefghifaddedg";
    uint32_t srcSize = sizeof("abcdefghifaddedg");
    char dest[space] = {0};
    src[0] = 0x00;
    src[0] = src[0] & (~MASK_HIGH_TWO_BITS);
    src[1] = 0x01;
    src[1] = src[1] & (~MASK_HIGH_TWO_BITS);
    int32_t ret = instance_->DnsExpand(&src[0], &src[srcSize], src, dest, space);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupParserTest, DnsExpand008, TestSize.Level1)
{
    const int32_t space = 10;
    uint8_t src[] = "abcdefghifaddedg";
    uint32_t srcSize = sizeof("abcdefghifaddedg");
    char dest[space] = {0};
    src[0] = 0x01;
    src[0] &= ~MASK_HIGH_TWO_BITS;
    int32_t ret = instance_->DnsExpand(&src[0], &src[srcSize], src, dest, space);
    EXPECT_EQ(ret, INVALID_LENGTH);
}

HWTEST_F(DNSLookupParserTest, DnsParseCallback001, TestSize.Level1)
{
    DpcCtx ctx;
    ctx.cnt = MAXADDRS + 1;
    int32_t rr = 0;
    int32_t len = 0;
    auto ret = instance_->DnsParseCallback(&ctx, rr, nullptr, len, nullptr);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, DnsParseCallback002, TestSize.Level1)
{
    DpcCtx ctx;
    ctx.cnt = MAXADDRS - 1;
    int32_t rr = RR_A;
    int32_t len = ADDR_A4_LEN + 1;
    uint8_t data[8] = {0};
    uint8_t packet[8] = {0};
    auto ret = instance_->DnsParseCallback(&ctx, rr, data, len, packet);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, DnsParseCallback003, TestSize.Level1)
{
    DpcCtx ctx;
    ctx.cnt = MAXADDRS - 1;
    AddrData addrData;
    ctx.addrs = &addrData;
    char str[] = "test data";
    ctx.canon = str;
    int32_t rr = RR_A;
    int32_t len = ADDR_A4_LEN;
    uint8_t data[8] = {0};
    uint8_t packet[8] = {0};
    auto ret = instance_->DnsParseCallback(&ctx, rr, &data, len, &packet);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, DnsParseCallback004, TestSize.Level1)
{
    DpcCtx ctx;
    ctx.cnt = MAXADDRS - 1;
    int32_t rr = RR_AAAA;
    int32_t len = ADDR_A6_LEN + 1;
    uint8_t data[8] = {0};
    uint8_t packet[8] = {0};
    auto ret = instance_->DnsParseCallback(&ctx, rr, data, len, packet);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, DnsParseCallback005, TestSize.Level1)
{
    DpcCtx ctx;
    ctx.cnt = MAXADDRS - 1;
    AddrData addrData;
    AddrData addrDatas[2];
    addrDatas[0] = addrData;
    ctx.addrs = addrDatas;
    char str[] = "test data";
    ctx.canon = str;
    ctx.cnt = 0;
    int32_t rr = RR_AAAA;
    int32_t len = ADDR_A6_LEN;
    uint8_t data[8] = {0};
    uint8_t packet[8] = {0};
    auto ret = instance_->DnsParseCallback(&ctx, rr, data, len, packet);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, DnsParseCallback006, TestSize.Level1)
{
    DpcCtx ctx;
    ctx.cnt = MAXADDRS - 1;
    int32_t rr = RR_CNAME;
    int32_t len = ADDR_A6_LEN;
    uint8_t data[8] = {0};
    uint8_t packet[8] = {0};
    auto ret = instance_->DnsParseCallback(&ctx, rr, data, len, packet);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, DnsParse001, TestSize.Level1)
{
    DpcCtx ctx;
    uint8_t *answers = nullptr;
    int32_t answersLen = RLEN_MAXNS - 1;
    auto ret = instance_->DnsParse(answers, answersLen, nullptr, &ctx);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, DnsParse002, TestSize.Level1)
{
    DpcCtx ctx;
    uint8_t answers[] = "abcdefghifaddedg";
    answers[ARG_INDEX_3] = answers[ARG_INDEX_3] | ANSWERS_OPERATION;
    int32_t answersLen = 15;
    auto callback = [](void *, int, const void *, int, const void *) { return 0; };
    auto ret = instance_->DnsParse(answers, answersLen, callback, &ctx);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupParserTest, DnsParse003, TestSize.Level1)
{
    DpcCtx ctx;
    uint8_t answers[] = "abcdefghifaddedg";
    int32_t answersLen = 15;
    answers[ARG_INDEX_3] = answers[ARG_INDEX_3] & (~ANSWERS_OPERATION);
    auto callback = [](void *, int, const void *, int, const void *) { return 0; };
    auto ret = instance_->DnsParse(answers, answersLen, callback, &ctx);
    EXPECT_EQ(ret, INVALID_LENGTH);
}

HWTEST_F(DNSLookupParserTest, ResMkQuery001, TestSize.Level1)
{
    int32_t op = 0;
    std::string dName = "test name";
    dName[dName.length() - 1] = DOT;
    int32_t mineClass = 0;
    int32_t type = 0;
    const uint8_t *data = nullptr;
    int32_t dataLen = 0;
    const uint8_t *newrr = nullptr;
    uint8_t *buf = nullptr;
    int32_t bufLen = 0;
    auto ret = instance_->ResMkQuery(op, dName, mineClass, type, data, dataLen, newrr, buf, bufLen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, ResMkQuery002, TestSize.Level1)
{
    int32_t op = 0;
    std::string dName;
    for (int32_t i = 0; i < HOST_MAX_LEN; i++) {
        dName.append("a");
    }
    int32_t mineClass = 0;
    int32_t type = 0;
    const uint8_t *data = nullptr;
    int32_t dataLen = 0;
    const uint8_t *newrr = nullptr;
    uint8_t *buf = nullptr;
    int32_t bufLen = 0;
    auto ret = instance_->ResMkQuery(op, dName, mineClass, type, data, dataLen, newrr, buf, bufLen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, ResMkQuery003, TestSize.Level1)
{
    int32_t op = 0;
    std::string dName;
    for (int32_t i = 0; i < HOST_MAX_LEN / 2; i++) {
        dName.append("a");
    }
    int32_t mineClass = 0;
    int32_t type = 0;
    const uint8_t *data = nullptr;
    int32_t dataLen = 0;
    const uint8_t *newrr = nullptr;
    uint8_t *buf = nullptr;
    int32_t bufLen = 0;
    auto ret = instance_->ResMkQuery(op, dName, mineClass, type, data, dataLen, newrr, buf, bufLen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, ResMkQuery004, TestSize.Level1)
{
    int32_t op = OP_MAX + 1;
    std::string dName;
    for (int32_t i = 0; i < HOST_MAX_LEN / 2; i++) {
        dName.append("a");
    }
    int32_t mineClass = 0;
    int32_t type = 0;
    const uint8_t *data = nullptr;
    int32_t dataLen = 0;
    const uint8_t *newrr = nullptr;
    uint8_t *buf = nullptr;
    int32_t bufLen = (HOST_MAX_LEN / 2) + 1;
    auto ret = instance_->ResMkQuery(op, dName, mineClass, type, data, dataLen, newrr, buf, bufLen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, ResMkQuery005, TestSize.Level1)
{
    int32_t op = OP_MAX - 1;
    std::string dName;
    for (int32_t i = 0; i < HOST_MAX_LEN / 2; i++) {
        dName.append("a");
    }
    int32_t mineClass = MINE_CLASS_MAX + 1;
    int32_t type = 0;
    const uint8_t *data = nullptr;
    int32_t dataLen = 0;
    const uint8_t *newrr = nullptr;
    uint8_t *buf = nullptr;
    int32_t bufLen = (HOST_MAX_LEN / 2) + 1;
    auto ret = instance_->ResMkQuery(op, dName, mineClass, type, data, dataLen, newrr, buf, bufLen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, ResMkQuery006, TestSize.Level1)
{
    int32_t op = OP_MAX - 1;
    std::string dName;
    for (int32_t i = 0; i < HOST_MAX_LEN / 2; i++) {
        dName.append("a");
    }
    int32_t mineClass = MINE_CLASS_MAX - 1;
    int32_t type = TYPE_MAX + 1;
    const uint8_t *data = nullptr;
    int32_t dataLen = 0;
    const uint8_t *newrr = nullptr;
    uint8_t *buf = nullptr;
    int32_t bufLen = (HOST_MAX_LEN / 2) + 1;
    auto ret = instance_->ResMkQuery(op, dName, mineClass, type, data, dataLen, newrr, buf, bufLen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(DNSLookupParserTest, ResMkQuery007, TestSize.Level1)
{
    int32_t op = OP_MAX - 1;
    std::string dName;
    for (int32_t i = 0; i < HOST_MAX_LEN / 2; i++) {
        dName.append("a");
    }
    int32_t mineClass = MINE_CLASS_MAX - 1;
    int32_t type = TYPE_MAX - 1;
    const uint8_t *data = nullptr;
    int32_t dataLen = 0;
    const uint8_t *newrr = nullptr;
    uint8_t *buf = nullptr;
    int32_t bufLen = (HOST_MAX_LEN / 2) + 1;
    auto ret = instance_->ResMkQuery(op, dName, mineClass, type, data, dataLen, newrr, buf, bufLen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}
} // namespace NetsysNative
} // namespace OHOS