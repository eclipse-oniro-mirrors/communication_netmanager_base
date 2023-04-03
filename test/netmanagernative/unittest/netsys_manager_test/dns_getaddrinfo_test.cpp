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

class DNSGetaddrinfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DNSGetaddrinfoTest::SetUpTestCase() {}

void DNSGetaddrinfoTest::TearDownTestCase() {}

void DNSGetaddrinfoTest::SetUp() {}

void DNSGetaddrinfoTest::TearDown() {}

HWTEST_F(DNSGetaddrinfoTest, CheckHintsTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckHintsTest enter");
    DnsGetAddrInfo getInfo;
    struct AddrInfo addrInfo;
    addrInfo.aiFlags = 0;
    addrInfo.aiFamily = AF_UNSPEC;
    int32_t ret = getInfo.CheckHints(addrInfo);
    EXPECT_EQ(ret, DNS_ERR_NONE);
}

HWTEST_F(DNSGetaddrinfoTest, CheckHintsTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("CheckHintsTest error family enter");
    DnsGetAddrInfo getInfo;
    struct AddrInfo addrInfo;
    addrInfo.aiFlags = 0;
    addrInfo.aiFamily = 88;
    int32_t ret = getInfo.CheckHints(addrInfo);
    EXPECT_EQ(ret, EAI_FAMILY);
}

HWTEST_F(DNSGetaddrinfoTest, ParseAddrTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest addr number zero enter");
    DnsGetAddrInfo getInfo;
    int32_t addrNum = 0;
    int32_t servNum = 0;
    struct ServData servData[2] = {};
    struct AddrData addrData[48] = {};
    char canon[16] = {0};
    int32_t len = 16;
    std::vector<AddrInfo> out;
    getInfo.ParseAddr(addrNum, servNum, servData, addrData, canon, len, out);
}

HWTEST_F(DNSGetaddrinfoTest, ParseAddrTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest server number zero enter");
    DnsGetAddrInfo getInfo;
    int32_t addrNum = 1;
    int32_t servNum = 0;
    struct ServData servData[2] = {};
    struct AddrData addrData[48] = {};
    char canon[16] = {0};
    int32_t len = 16;
    std::vector<AddrInfo> out;
    getInfo.ParseAddr(addrNum, servNum, servData, addrData, canon, len, out);
}

HWTEST_F(DNSGetaddrinfoTest, ParseAddrTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest canon len zero enter");
    DnsGetAddrInfo getInfo;
    int32_t addrNum = 1;
    int32_t servNum = 1;
    struct ServData servData[2] = {};
    struct AddrData addrData[48] = {};
    char canon[16] = {0};
    int32_t len = 0;
    std::vector<AddrInfo> out;
    getInfo.ParseAddr(addrNum, servNum, servData, addrData, canon, len, out);
}

HWTEST_F(DNSGetaddrinfoTest, ParseAddrTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest IPv4 enter");
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
}

HWTEST_F(DNSGetaddrinfoTest, ParseAddrTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest IPv6 enter");
    DnsGetAddrInfo getInfo;
    int32_t addrNum = 1;
    int32_t servNum = 1;
    struct ServData servData[2] = {};
    struct AddrData addrData[48] = {};
    addrData[0].family = AF_INET6;
    char canon[16] = {0};
    int32_t len = 16;
    std::vector<AddrInfo> out;
    getInfo.ParseAddr(addrNum, servNum, servData, addrData, canon, len, out);
}

HWTEST_F(DNSGetaddrinfoTest, ParseAddrTest006, TestSize.Level1)
{
    NETNATIVE_LOGI("ParseAddrTest default enter");
    DnsGetAddrInfo getInfo;
    int32_t addrNum = 1;
    int32_t servNum = 1;
    struct ServData servData[2] = {};
    struct AddrData addrData[48] = {};
    addrData[0].family = 0;
    char canon[16] = {0};
    int32_t len = 16;
    std::vector<AddrInfo> out;
    getInfo.ParseAddr(addrNum, servNum, servData, addrData, canon, len, out);
}

HWTEST_F(DNSGetaddrinfoTest, GetAddrInfoTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetAddrInfoTest host name empty enter");
    DnsGetAddrInfo getInfo;
    std::string host = "";
    std::string server = "";
    struct AddrInfo addrInfo = {};
    uint16_t netId = 101;
    std::vector<AddrInfo> out;
    int32_t ret = getInfo.GetAddrInfo(host, server, addrInfo, netId, out);
    EXPECT_EQ(ret, EAI_NONAME);
}

HWTEST_F(DNSGetaddrinfoTest, GetAddrInfoTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("GetAddrInfoTest AddrInfo error enter");
    DnsGetAddrInfo getInfo;
    std::string host = "abcd";
    std::string server = "dcba";
    struct AddrInfo addrInfo = {};
    addrInfo.aiFlags = 0;
    addrInfo.aiFamily = 88;
    uint16_t netId = 101;
    std::vector<AddrInfo> out;
    int32_t ret = getInfo.GetAddrInfo(host, server, addrInfo, netId, out);
    EXPECT_EQ(ret, EAI_FAMILY);
}

} // namespace NetsysNative
} // namespace OHOS