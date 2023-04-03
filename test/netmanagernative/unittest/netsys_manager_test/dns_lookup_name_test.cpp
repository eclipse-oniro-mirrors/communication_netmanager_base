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
#include "dns_lookup_name.h"
#undef private

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;

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

HWTEST_F(DNSLookupNameTest, NameFromDnsSearchTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsSearchTest invalid host name enter");
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

HWTEST_F(DNSLookupNameTest, NameFromDnsSearchTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("NameFromDnsSearchTest enter");
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

HWTEST_F(DNSLookupNameTest, LookUpNameParamTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("LookUpNameParamTest count zero enter");
    DnsLookUpName name;
    struct AddrData sddrData = {};
    int32_t cnt = 0;
    name.LookUpNameParam(&sddrData, cnt, 0);
}

HWTEST_F(DNSLookupNameTest, LookUpNameParamTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("LookUpNameParamTest enter");
    DnsLookUpName name;
    struct AddrData sddrData = {};
    int32_t cnt = 1;
    name.LookUpNameParam(&sddrData, cnt, 0);
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

} // namespace NetsysNative
} // namespace OHOS