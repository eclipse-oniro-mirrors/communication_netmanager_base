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

#include "dns_lookup_name.cpp"
#include "dns_manager.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;
namespace {
const uint16_t NET_ID = 3;
uint16_t BASE_TIMEOUT_MILLIS = 2000;
uint8_t RETRY_COUNT = 3;
constexpr int32_t TEST_SOCKET_FD = 7;

void SockAddrCopy001()
{
    DnsLookUpName name;
    ScokAddrCopy addrBuff;
    addrBuff.family = AF_INET;
    addrBuff.lookUpNameFd = TEST_SOCKET_FD;
    sockaddr sockaddr1;
    sockaddr sockaddr2;
    int32_t dScope = 0;
    int32_t preFixLen = 0;
    uint32_t key = 0;
    name.SockAddrCopy(addrBuff, &sockaddr1, &sockaddr2, dScope, preFixLen, key);
}

void LookUpNameParamTest001()
{
    NETNATIVE_LOGI("LookUpNameParamTest enter");
    DnsLookUpName name;
    const size_t addrDataSize = 5;
    int32_t cnt = addrDataSize;
    std::unique_ptr<AddrData[]> addrBuf = std::make_unique<AddrData[]>(addrDataSize);
    memset_s(addrBuf.get(), sizeof(AddrData) * addrDataSize, 0, sizeof(AddrData) * addrDataSize);
    addrBuf[0].family = AF_INET6;
    addrBuf[1].family = AF_INET;
    name.LookUpNameParam(addrBuf.get(), cnt, 0);
}
} // namespace
class DNSLookupNameTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DNSLookupNameTest::SetUpTestCase() {}

void DNSLookupNameTest::TearDownTestCase() {}

void DNSLookupNameTest::SetUp() {}

void DNSLookupNameTest::TearDown() {}

HWTEST_F(DNSLookupNameTest, NameFromNullTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromNullTest invalid nns enter");
    SockAddrCopy001();
    DnsLookUpName name;
    struct AddrData buf[2] = {};
    std::string host = "a";
    int32_t family = 0;
    int32_t flag = 0;
    int32_t ret = name.NameFromNull(buf, host, family, flag);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, NameFromNullTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromNullTest IPv4 AI_Passive enter");
    DnsLookUpName name;
    struct AddrData buf[2] = {};
    std::string host = "";
    int32_t family = AF_INET;
    int32_t flag = AI_PASSIVE;
    int32_t ret = name.NameFromNull(buf, host, family, flag);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupNameTest, NameFromNullTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromNullTest IPv6 AI_Passive enter");
    DnsLookUpName name;
    struct AddrData buf[2] = {};
    std::string host = "";
    int32_t family = AF_INET6;
    int32_t flag = AI_PASSIVE;
    int32_t ret = name.NameFromNull(buf, host, family, flag);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupNameTest, NameFromNullTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromNullTest IPv4 enter");
    DnsLookUpName name;
    struct AddrData buf[2] = {};
    std::string host = "";
    int32_t family = AF_INET;
    int32_t flag = 0;
    int32_t ret = name.NameFromNull(buf, host, family, flag);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupNameTest, NameFromNullTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromNullTest IPv4 enter");
    DnsLookUpName name;
    struct AddrData buf[2] = {};
    std::string host = "";
    int32_t family = AF_INET6;
    int32_t flag = 0;
    int32_t ret = name.NameFromNull(buf, host, family, flag);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsTest enter");
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "abcd";
    int32_t family = AF_INET;
    struct ResolvConf resolvConf = {};
    int32_t netId = 101;
    int32_t ret = name.NameFromDns(addrData, canon, host, family, &resolvConf, netId);
    EXPECT_EQ(ret, EAI_AGAIN);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsTest invalid host name enter");
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "";
    int32_t family = AF_INET;
    struct ResolvConf resolvConf = {};
    int32_t netId = 101;
    int32_t ret = name.NameFromDns(addrData, canon, host, family, &resolvConf, netId);
    EXPECT_EQ(ret, EAI_AGAIN);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsTest enter");
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "abcd";
    int32_t family = AF_INET;
    struct ResolvConf resolvConf = {};
    int32_t netId = 101;
    int32_t ret = name.NameFromDns(addrData, canon, host, family, &resolvConf, netId);
    EXPECT_EQ(ret, EAI_AGAIN);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsTest enter");
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "abcd";
    int32_t family = 266;
    struct ResolvConf resolvConf = {};
    int32_t netId = 101;
    int32_t ret = name.NameFromDns(addrData, canon, host, family, &resolvConf, netId);
    EXPECT_EQ(ret, EAI_AGAIN);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsTest005, TestSize.Level1)
{
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "abcd";
    for (int32_t i = 0; i < HOST_MAX_LEN; i++) {
        host.append("a");
    }
    int32_t family = 266;
    struct ResolvConf resolvConf = {};
    int32_t netId = 101;
    int32_t ret = name.NameFromDns(addrData, canon, host, family, &resolvConf, netId);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsTest006, TestSize.Level1)
{
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "abcd";
    for (int32_t i = 0; i < HOST_MAX_LEN / 2; i++) {
        host.append("a");
    }
    int32_t family = AF_INET + 255;
    struct ResolvConf resolvConf = {};
    int32_t netId = 101;
    int32_t ret = name.NameFromDns(addrData, canon, host, family, &resolvConf, netId);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsTest007, TestSize.Level1)
{
    DnsLookUpName name;
    AddrData addrData[48] = {};
    for (int i = 0; i < 48; i++) {
        addrData[i].family = AF_INET6;
    }
    char canon[256] = {0};
    std::string host = "abcd";
    for (int32_t i = 0; i < HOST_MAX_LEN / 2; i++) {
        host.append("a");
    }
    int32_t family = AF_INET6;
    struct ResolvConf resolvConf = {};
    int32_t netId = 100;
    int32_t ret = name.NameFromDns(addrData, canon, host, family, &resolvConf, netId);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsSearchTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsSearchTest001 host is null enter");
    DnsManager dnsManager;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    dnsManager.SetResolverConfig(NET_ID, BASE_TIMEOUT_MILLIS, RETRY_COUNT, servers, domains);
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "";
    int32_t family = AF_INET;
    int32_t ret = name.NameFromDnsSearch(addrData, canon, host, family, NET_ID);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, NameFromDnsSearchTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsSearchTest001 host is baidu enter");
    DnsManager dnsManager;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    dnsManager.SetResolverConfig(NET_ID, BASE_TIMEOUT_MILLIS, RETRY_COUNT, servers, domains);
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "www.baidu.com";
    int32_t family = AF_INET;
    int32_t ret = name.NameFromDnsSearch(addrData, canon, host, family, NET_ID);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, LabelOfTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("LabelOfTest001 enter");
    DnsLookUpName name;
    sockaddr_in6 da6 = {
        .sin6_family = AF_INET6,
        .sin6_port = PORT_NUM,
        .sin6_scope_id = 1,
    };
    int32_t ret = name.LabelOf(&da6.sin6_addr);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupNameTest, ScopeOfTest001, TestSize.Level1)
{
    DnsLookUpName instance;
    in6_addr addr;
    addr.s6_addr[0] = htonl (0xffc00000);
    addr.s6_addr[ARG_INDEX_1] = 0x01;
    auto ret = instance.ScopeOf(&addr);
    EXPECT_EQ(ret, 0x01);
}

HWTEST_F(DNSLookupNameTest, ScopeOfTest002, TestSize.Level1)
{
    DnsLookUpName instance;
    in6_addr addr;
    addr.s6_addr32[0] |= htonl(0xfe800000);
    auto ret = instance.ScopeOf(&addr);
    EXPECT_EQ(ret, SCOPEOF_RESULT_2);
}

HWTEST_F(DNSLookupNameTest, ScopeOfTest003, TestSize.Level1)
{
    DnsLookUpName instance;
    in6_addr addr;
    addr.s6_addr32[0] = 0;
    addr.s6_addr32[1] = 0;
    addr.s6_addr32[2] = 0;
    addr.s6_addr32[3] = htonl(1);
    auto ret = instance.ScopeOf(&addr);
    EXPECT_EQ(ret, SCOPEOF_RESULT_2);
}

HWTEST_F(DNSLookupNameTest, ScopeOfTest004, TestSize.Level1)
{
    DnsLookUpName instance;
    in6_addr addr;
    addr.s6_addr32[0] = htonl (0xfec00000);
    auto ret = instance.ScopeOf(&addr);
    EXPECT_EQ(ret, SCOPEOF_RESULT_5);
}

HWTEST_F(DNSLookupNameTest, ScopeOfTest005, TestSize.Level1)
{
    DnsLookUpName instance;
    in6_addr addr;
    addr.s6_addr[0] = 0xFF;
    addr.s6_addr[1] = 0xFE;
    addr.s6_addr[2] = 0x80;
    addr.s6_addr[15] = 0x01;
    auto ret = instance.ScopeOf(&addr);
    EXPECT_EQ(ret, SCOPEOF_RESULT_14);
}

HWTEST_F(DNSLookupNameTest, PreFixMatchTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("PreFixMatchTest001 enter");
    DnsLookUpName name;
    sockaddr_in6 da6 = {
        .sin6_family = AF_INET6,
        .sin6_port = PORT_NUM,
        .sin6_scope_id = 1,
    };
    int32_t ret = name.PreFixMatch(&da6.sin6_addr, &da6.sin6_addr);
    EXPECT_EQ(ret, PREFIX_SIZE);
}

HWTEST_F(DNSLookupNameTest, CheckNameParamTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckNameParamTest empty host name enter");
    DnsLookUpName name;
    std::string host = "";
    int32_t family = AF_INET;
    int32_t flag = 0xFFFF;
    char *canon = nullptr;
    int32_t ret = name.CheckNameParam(host, flag, family, canon);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, CheckNameParamTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckNameParamTest IPv4 enter");
    DnsLookUpName name;
    std::string host = "abcd";
    int32_t family = AF_INET;
    int32_t flag = 0xFFFF;
    char *canon = nullptr;
    int32_t ret = name.CheckNameParam(host, flag, family, canon);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, CheckNameParamTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckNameParamTest IPv6 enter");
    DnsLookUpName name;
    std::string host = "abcd";
    int32_t family = AF_INET6;
    int32_t flag = 0xFFFF;
    char *canon = nullptr;
    int32_t ret = name.CheckNameParam(host, flag, family, canon);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, CheckNameParamTest004, TestSize.Level1)
{
    DnsLookUpName name;
    std::string host = "a";
    for (uint32_t i = 0; i <= 255; i++) {
        host.append("a");
    }
    int32_t family = AF_INET;
    int32_t flag = 0xFFFF;
    char *canon = nullptr;
    int32_t ret = name.CheckNameParam(host, flag, family, canon);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, UpdateBufTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("UpdateBufTest IPv4 enter");
    DnsLookUpName name;
    struct AddrData sddrData = {};
    int32_t family = AF_INET;
    int32_t flag = 0xFFFF;
    int32_t cnt = 1;
    bool ret = name.UpdateBuf(flag, family, &sddrData, cnt);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DNSLookupNameTest, UpdateBufTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("UpdateBufTest IPv6 enter");
    DnsLookUpName name;
    struct AddrData sddrData = {};
    int32_t family = AF_INET;
    int32_t flag = 0xFFFF;
    int32_t cnt = 2;
    bool ret = name.UpdateBuf(flag, family, &sddrData, cnt);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DNSLookupNameTest, UpdateBufTest003, TestSize.Level1)
{
    LookUpNameParamTest001();
    DnsLookUpName name;
    int32_t flags = AI_ALL;
    int32_t family = AF_INET6;
    const size_t addrDataSize = 5;
    std::unique_ptr<AddrData[]> addrBuf = std::make_unique<AddrData[]>(addrDataSize);
    addrBuf[0].family = AF_INET;
    addrBuf[1].family = AF_INET6;
    int32_t cnt = addrDataSize - 1;
    auto ret = name.UpdateBuf(flags, family, addrBuf.get(), cnt);
    EXPECT_TRUE(ret);
}

HWTEST_F(DNSLookupNameTest, LookUpNameTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("LookUpNameTest empty host name enter");
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "";
    int32_t family = AF_INET;
    int32_t flag = 0xFFFF;
    int32_t netId = 101;
    int32_t ret = name.LookUpName(addrData, canon, host, family, flag, netId);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupNameTest, LookUpNameTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("LookUpNameTest IPv4 enter");
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "abcd";
    int32_t family = AF_INET;
    int32_t flag = 0xFFFF;
    int32_t netId = 101;
    int32_t ret = name.LookUpName(addrData, canon, host, family, flag, netId);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, LookUpNameTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("LookUpNameTest IPv6 enter");
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "abcd";
    int32_t family = AF_INET6;
    int32_t flag = 0xFFFF;
    int32_t netId = 101;
    int32_t ret = name.LookUpName(addrData, canon, host, family, flag, netId);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, LookUpNameTest004, TestSize.Level1)
{
    DnsLookUpName name;
    struct AddrData addrData[48] = {};
    char canon[256] = {0};
    std::string host = "a";
    for (uint32_t i = 0; i <= 255; i++) {
        host.append("a");
    }
    int32_t family = AF_INET6;
    int32_t flag = 0xFFFF;
    int32_t netId = 101;
    int32_t ret = name.LookUpName(addrData, canon, host, family, flag, netId);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest stream 0 enter");
    DnsLookUpName name;
    int32_t sockType = SOCK_STREAM;
    int32_t proto = 0;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest stream TCP enter");
    DnsLookUpName name;
    int32_t sockType = SOCK_STREAM;
    int32_t proto = IPPROTO_TCP;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest stream default enter");
    DnsLookUpName name;
    int32_t sockType = SOCK_STREAM;
    int32_t proto = 88;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, EAI_SERVICE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest dgram 0 enter");
    DnsLookUpName name;
    int32_t sockType = SOCK_DGRAM;
    int32_t proto = 0;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest dgram udp enter");
    DnsLookUpName name;
    int32_t sockType = SOCK_DGRAM;
    int32_t proto = IPPROTO_UDP;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest006, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest dgram default enter");
    DnsLookUpName name;
    int32_t sockType = SOCK_DGRAM;
    int32_t proto = 88;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, EAI_SERVICE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest007, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest default enter");
    DnsLookUpName name;
    int32_t sockType = 0;
    int32_t proto = 0;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest008, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest host name empty enter");
    DnsLookUpName name;
    int32_t sockType = 88;
    int32_t proto = 88;
    std::string host = "";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSLookupNameTest, SwitchSocketTypeTest009, TestSize.Level1)
{
    NETNATIVE_LOGI("SwitchSocketTypeTest host name empty enter");
    DnsLookUpName name;
    int32_t sockType = 88;
    int32_t proto = 88;
    std::string host = "abcd";
    struct ServData buf = {};
    int32_t ret = name.SwitchSocketType(sockType, host, proto, &buf);
    EXPECT_EQ(ret, EAI_SERVICE);
}

HWTEST_F(DNSLookupNameTest, LookUpServerTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("LookUpServerTest switch error enter");
    DnsLookUpName name;
    struct ServData servData[2] = {};
    std::string host = "";
    int32_t proto = 88;
    int32_t sockType = SOCK_STREAM;
    int32_t flag = 0;
    int32_t ret = name.LookUpServer(servData, host, proto, sockType, flag);
    EXPECT_EQ(ret, EAI_SERVICE);
}

HWTEST_F(DNSLookupNameTest, LookUpServerTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("LookUpServerTest host name empty enter");
    DnsLookUpName name;
    struct ServData servData[2] = {};
    std::string host = "";
    int32_t proto = IPPROTO_TCP;
    int32_t sockType = SOCK_STREAM;
    int32_t flag = 0;
    int32_t ret = name.LookUpServer(servData, host, proto, sockType, flag);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(DNSLookupNameTest, AddrCmp001, TestSize.Level1)
{
    DnsLookUpName name;
    AddrData addrData1;
    addrData1.sortKey = 2;
    AddrData addrData2;
    addrData2.sortKey = 2;
    int ret = name.AddrCmp(&addrData1, &addrData2);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSLookupNameTest, AddrCmp002, TestSize.Level1)
{
    DnsLookUpName name;
    AddrData addrData1;
    addrData1.sortKey = 0;
    AddrData addrData2;
    addrData2.sortKey = 1;
    int ret = name.AddrCmp(&addrData1, &addrData2);
    EXPECT_EQ(ret, addrData2.sortKey - addrData1.sortKey);
}

HWTEST_F(DNSLookupNameTest, AddrCmp003, TestSize.Level1)
{
    DnsLookUpName name;
    AddrData addrData1;
    addrData1.sortKey = 1;
    AddrData addrData2;
    addrData2.sortKey = 0;
    int ret = name.AddrCmp(&addrData1, &addrData2);
    EXPECT_EQ(ret, addrData2.sortKey - addrData1.sortKey);
}

HWTEST_F(DNSLookupNameTest, RefreshBuf001, TestSize.Level1)
{
    DnsLookUpName name;
    int32_t cnt = 2;
    const size_t bufSize = 2;
    AddrData buf[bufSize] = {};
    buf[0].family = AF_INET6;
    int32_t num = 0;
    name.RefreshBuf(buf, num, cnt);
    EXPECT_EQ(cnt, 1);
}
} // namespace NetsysNative
} // namespace OHOS