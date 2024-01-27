/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "netlink_define.h"
#include "securec.h"
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <ifaddrs.h>
#include <linux/genetlink.h>
#include <linux/rtnetlink.h>
#include <memory>
#include <net/if.h>
#include <netdb.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "wrapper_decoder.h"
#include <cstring>

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
constexpr int16_t LOCAL_QLOG_NL_EVENT = 112;
constexpr const char TEST_ASCII_MESSAGE[] = {
    "action@msg\0ACTION=add\0ACTION=remove\0ACTION=change\0SEQNUM=111\0SEQNUM=\0SUBSYSTEM=net\0SUBSYSTEM="
    "\0SUBSYSTEM=test\0dfdfcc=ttt\0"};
} // namespace

class WrapperDecoderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void WrapperDecoderTest::SetUpTestCase() {}

void WrapperDecoderTest::TearDownTestCase() {}

void WrapperDecoderTest::SetUp() {}

void WrapperDecoderTest::TearDown() {}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest001, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    std::string buffer = "testMsg@@";
    auto ret = decoder->DecodeAscii(buffer.data(), buffer.length());
    EXPECT_FALSE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest002, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    std::string buffer = "@testMsg";
    auto ret = decoder->DecodeAscii(buffer.data(), 0);
    EXPECT_FALSE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest003, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    std::string buffer = "@testMsg";
    auto ret = decoder->DecodeAscii(buffer.data(), buffer.length());
    EXPECT_FALSE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest004, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    auto ret = decoder->DecodeAscii(TEST_ASCII_MESSAGE, sizeof(TEST_ASCII_MESSAGE));
    EXPECT_TRUE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeBinaryTest001, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    char binarydata[NLMSG_ALIGN(sizeof(struct nlmsghdr)) + NLMSG_ALIGN(sizeof(struct ifinfomsg)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + IFNAMSIZ];
    ASSERT_EQ(memset_s(&binarydata, sizeof(binarydata), 0, sizeof(binarydata)), EOK);
    nlmsghdr *pmsghdr = reinterpret_cast<struct nlmsghdr *>(&binarydata);
    ASSERT_NE(pmsghdr, nullptr);
    ifinfomsg *pifInfomsg = reinterpret_cast<struct ifinfomsg *>(NLMSG_DATA(&binarydata));
    ASSERT_NE(pifInfomsg, nullptr);
    pmsghdr->nlmsg_len = sizeof(binarydata);
    pmsghdr->nlmsg_type = RTM_MAX;
    rtattr *prtattr = IFLA_RTA(pifInfomsg);
    ASSERT_NE(prtattr, nullptr);

    pifInfomsg->ifi_flags = 0;
    auto ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), pmsghdr->nlmsg_len);
    EXPECT_FALSE(ret);

    pmsghdr->nlmsg_type = RTM_NEWLINK;
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    pifInfomsg->ifi_flags = IFF_LOOPBACK;
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    prtattr->rta_type = IFLA_IFNAME;
    prtattr->rta_len = sizeof(struct rtattr) + IFNAMSIZ;
    ASSERT_EQ(strcpy_s(&binarydata[sizeof(binarydata) - IFNAMSIZ], IFNAMSIZ, "ifacename"), 0);
    pifInfomsg->ifi_flags = IFF_LOWER_UP;
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_TRUE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeBinaryTest002, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    char binarydata[NLMSG_ALIGN(sizeof(struct nlmsghdr)) + NLMSG_ALIGN(192)];
    ASSERT_EQ(memset_s(&binarydata, sizeof(binarydata), 0, sizeof(binarydata)), EOK);
    nlmsghdr *pmsghdr = reinterpret_cast<struct nlmsghdr *>(&binarydata);
    ASSERT_NE(pmsghdr, nullptr);

    pmsghdr->nlmsg_len = NLMSG_ALIGN(sizeof(struct nlmsghdr));
    pmsghdr->nlmsg_type = LOCAL_QLOG_NL_EVENT;

    ASSERT_EQ(strcpy_s(&binarydata[NLMSG_ALIGN(sizeof(struct nlmsghdr)) + 28], IFNAMSIZ, "testDevName"), 0);
    auto ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    pmsghdr->nlmsg_len = sizeof(binarydata);
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_TRUE(ret);
}

HWTEST_F(WrapperDecoderTest, InterpreteAddressMsgTest001, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    char binarydata[NLMSG_ALIGN(sizeof(struct nlmsghdr)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct in_addr)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + NLMSG_ALIGN(sizeof(struct ifa_cacheinfo)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + NLMSG_ALIGN(sizeof(uint32_t))];
    ASSERT_EQ(memset_s(&binarydata, sizeof(binarydata), 0, sizeof(binarydata)), EOK);
    nlmsghdr *pmsghdr = reinterpret_cast<struct nlmsghdr *>(&binarydata);
    ASSERT_NE(pmsghdr, nullptr);
    ifaddrmsg *pifaddrmsg = reinterpret_cast<struct ifaddrmsg *>(NLMSG_DATA(&binarydata));
    ASSERT_NE(pifaddrmsg, nullptr);
    rtattr *prtattr = IFA_RTA(pifaddrmsg);
    ASSERT_NE(prtattr, nullptr);
    in_addr *ipv4Addr = reinterpret_cast<struct in_addr *>(RTA_DATA(prtattr));
    ASSERT_NE(ipv4Addr, nullptr);

    pmsghdr->nlmsg_len = NLMSG_ALIGN(sizeof(struct nlmsghdr));
    pmsghdr->nlmsg_type = RTM_NEWADDR;
    EXPECT_FALSE(decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata)));

    pmsghdr->nlmsg_len = sizeof(binarydata);
    EXPECT_FALSE(decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata)));

    prtattr->rta_type = IFLA_IFNAME;
    EXPECT_FALSE(decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata)));

    prtattr->rta_type = IFA_ADDRESS;
    prtattr->rta_len = RTA_ALIGN(sizeof(struct rtattr));
    EXPECT_FALSE(decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata)));

    pifaddrmsg->ifa_family = AF_INET;
    EXPECT_FALSE(decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata)));

    ipv4Addr->s_addr = inet_addr("127.0.0.1");
    prtattr->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct in_addr));
    rtattr *prtattr1 = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(prtattr)) + prtattr->rta_len);
    ASSERT_NE(prtattr1, nullptr);
    prtattr1->rta_type = IFA_CACHEINFO;
    prtattr1->rta_len = RTA_ALIGN(sizeof(struct rtattr));
    EXPECT_TRUE(decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata)));

    prtattr1->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct ifa_cacheinfo));
    rtattr *prtattr2 = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(prtattr1)) + prtattr1->rta_len);
    ASSERT_NE(prtattr2, nullptr);
    prtattr2->rta_type = IFA_FLAGS;
    prtattr2->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(uint32_t));
    EXPECT_TRUE(decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata)));
}

HWTEST_F(WrapperDecoderTest, InterpreteAddressMsgTest002, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    char binarydata[NLMSG_ALIGN(sizeof(struct nlmsghdr)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct in6_addr)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + NLMSG_ALIGN(sizeof(struct ifa_cacheinfo)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + NLMSG_ALIGN(sizeof(uint32_t))];
    ASSERT_EQ(memset_s(&binarydata, sizeof(binarydata), 0, sizeof(binarydata)), EOK);
    nlmsghdr *pmsghdr = reinterpret_cast<struct nlmsghdr *>(&binarydata);
    ASSERT_NE(pmsghdr, nullptr);
    ifaddrmsg *pifaddrmsg = reinterpret_cast<struct ifaddrmsg *>(NLMSG_DATA(&binarydata));
    ASSERT_NE(pifaddrmsg, nullptr);
    rtattr *prtattr = IFA_RTA(pifaddrmsg);
    ASSERT_NE(prtattr, nullptr);
    in6_addr *ipv6Addr = reinterpret_cast<struct in6_addr *>(RTA_DATA(prtattr));
    ASSERT_NE(ipv6Addr, nullptr);

    pmsghdr->nlmsg_len = NLMSG_ALIGN(sizeof(struct nlmsghdr));
    pmsghdr->nlmsg_type = RTM_NEWADDR;

    auto ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    pmsghdr->nlmsg_len = sizeof(binarydata);
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    prtattr->rta_type = IFLA_IFNAME;
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    prtattr->rta_type = IFA_ADDRESS;
    prtattr->rta_len = RTA_ALIGN(sizeof(struct rtattr));
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    pifaddrmsg->ifa_family = AF_INET6;
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    prtattr->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct in6_addr));
    rtattr *prtattr1 = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(prtattr)) + prtattr->rta_len);
    ASSERT_NE(prtattr1, nullptr);
    prtattr1->rta_type = IFA_CACHEINFO;
    prtattr1->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct ifa_cacheinfo));
    rtattr *prtattr2 = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(prtattr1)) + prtattr1->rta_len);
    ASSERT_NE(prtattr2, nullptr);
    prtattr2->rta_type = IFA_FLAGS;
    prtattr2->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(uint32_t));

    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_TRUE(ret);
}

void InterpreteRtMsgTest001ParmaCheck(rtmsg *prtmsg, rtattr *prtattr, rtattr **prtattr1, rtattr **prtattr2,
                                      in_addr *ipv4Addr)
{
    prtmsg->rtm_protocol = RTPROT_KERNEL;
    prtmsg->rtm_family = AF_INET;
    prtmsg->rtm_scope = RT_SCOPE_UNIVERSE;
    prtmsg->rtm_type = RTN_UNICAST;
    prtattr->rta_type = RTA_GATEWAY;
    prtattr->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct in_addr));
    ipv4Addr->s_addr = inet_addr("0.0.0.0");
    *prtattr1 = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(prtattr)) + prtattr->rta_len);
    ASSERT_NE(prtattr1, nullptr);
    (*prtattr1)->rta_type = RTA_DST;
    (*prtattr1)->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct in_addr));
    ipv4Addr = reinterpret_cast<struct in_addr *>(RTA_DATA(*prtattr1));
    ASSERT_NE(ipv4Addr, nullptr);
    ipv4Addr->s_addr = inet_addr("127.0.0.1");
    (*prtattr2) = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(*prtattr1)) + (*prtattr1)->rta_len);
    ASSERT_NE(*prtattr2, nullptr);
    (*prtattr2)->rta_type = RTA_OIF;
    (*prtattr2)->rta_len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(uint32_t));
    prtmsg->rtm_dst_len = 0;
}

HWTEST_F(WrapperDecoderTest, InterpreteRtMsgTest001, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    char binarydata[NLMSG_ALIGN(sizeof(struct nlmsghdr)) + NLMSG_ALIGN(sizeof(struct rtmsg)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(sizeof(struct in_addr)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + NLMSG_ALIGN(sizeof(struct in_addr)) +
                    RTA_ALIGN(sizeof(struct rtattr)) + NLMSG_ALIGN(sizeof(uint32_t))];
    ASSERT_EQ(memset_s(&binarydata, sizeof(binarydata), 0, sizeof(binarydata)), EOK);
    nlmsghdr *pmsghdr = reinterpret_cast<struct nlmsghdr *>(&binarydata);
    ASSERT_NE(pmsghdr, nullptr);
    rtmsg *prtmsg = reinterpret_cast<struct rtmsg *>(NLMSG_DATA(&binarydata));
    ASSERT_NE(prtmsg, nullptr);
    rtattr *prtattr = RTM_RTA(prtmsg);
    ASSERT_NE(prtattr, nullptr);
    in_addr *ipv4Addr = reinterpret_cast<struct in_addr *>(RTA_DATA(prtattr));
    ASSERT_NE(ipv4Addr, nullptr);
    pmsghdr->nlmsg_type = RTM_NEWROUTE;
    pmsghdr->nlmsg_len = NLMSG_ALIGN(sizeof(struct nlmsghdr));
    auto ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);
    pmsghdr->nlmsg_len = sizeof(binarydata);
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);

    rtattr *prtattr1 = nullptr;
    rtattr *prtattr2 = nullptr;

    InterpreteRtMsgTest001ParmaCheck(prtmsg, prtattr, &prtattr1, &prtattr2, ipv4Addr);

    int32_t *pdeviceindex = reinterpret_cast<int32_t *>(RTA_DATA(prtattr2));
    *pdeviceindex = -1;
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    EXPECT_FALSE(ret);
    uint32_t index = if_nametoindex("wlan0");
    if (index == 0) {
        index = if_nametoindex("eth0");
    }
    *pdeviceindex = index;
    ret = decoder->DecodeBinary(reinterpret_cast<char *>(&binarydata), sizeof(binarydata));
    if (index > 0) {
        EXPECT_TRUE(ret);
    } else {
        EXPECT_FALSE(ret);
    }
}

HWTEST_F(WrapperDecoderTest, PushAsciiMessageTest001, TestSize.Level1)
{
    const char *start = TEST_ASCII_MESSAGE;
    const char *end = start + sizeof(TEST_ASCII_MESSAGE);
    std::vector<std::string> recvmsg;
    start += strlen(start) + 1;
    while (start < end) {
        if (start != nullptr) {
            recvmsg.emplace_back(start);
        }
        start += strlen(start) + 1;
    }

    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    auto ret = decoder->PushAsciiMessage(recvmsg);
    EXPECT_TRUE(ret);
}

HWTEST_F(WrapperDecoderTest, WrapperDecoderBranchTest001, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);

    nlmsghdr *hdrMsg = nullptr;
    auto ret = decoder->InterpreteInfoMsg(hdrMsg);
    EXPECT_FALSE(ret);

    ifaddrmsg *addrMsg = nullptr;
    std::string testString = "";
    ifa_cacheinfo *cacheInfo = nullptr;
    ret = decoder->SaveAddressMsg(testString, addrMsg, testString, cacheInfo, testString);
    EXPECT_FALSE(ret);

    uint8_t type = RTM_NEWNEIGH;
    auto result = decoder->CheckRtParam(hdrMsg, type);
    EXPECT_TRUE(result == nullptr);

    int32_t length = 0;
    int32_t family = AF_INET6;
    ret = decoder->SaveRtMsg(testString, testString, testString, length, family);
    EXPECT_FALSE(ret);

    length = 1;
    testString = "test";
    ret = decoder->SaveRtMsg(testString, testString, testString, length, family);
    EXPECT_TRUE(ret);
}
} // namespace nmd
} // namespace OHOS
