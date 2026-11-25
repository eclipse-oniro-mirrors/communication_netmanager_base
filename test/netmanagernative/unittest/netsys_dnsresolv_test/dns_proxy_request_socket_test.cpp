/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <unistd.h>
#include <sys/socket.h>

#define private public

#include "dns_proxy_request_socket.h"
#include "netnative_log_wrapper.h"
#include "dns_proxy_listen.h"

namespace OHOS::nmd {
    
using namespace testing;
using namespace testing::ext;

class DnsProxyRequestSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DnsProxyRequestSocketTest::SetUpTestCase() {}
void DnsProxyRequestSocketTest::TearDownTestCase() {}
void DnsProxyRequestSocketTest::SetUp() {}
void DnsProxyRequestSocketTest::TearDown() {}

HWTEST_F(DnsProxyRequestSocketTest, Create_01, TestSize.Level0)
{
    int32_t sock = socket(10000, SOCK_CLOEXEC, 0);
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    DnsProxyRequestSocket dnsProxyRequestSocket(sock, std::move(clientSock), std::move(recvBuff), AF_INET6);
    EXPECT_EQ(dnsProxyRequestSocket.sock, sock);
    EXPECT_EQ(dnsProxyRequestSocket.family, AF_INET6);
    EXPECT_EQ(dnsProxyRequestSocket.GetFamily(), AF_INET6);
    EXPECT_EQ(dnsProxyRequestSocket.event.data.fd, sock);
    EXPECT_EQ(dnsProxyRequestSocket.event.events, EPOLLIN);
    EXPECT_NE(dnsProxyRequestSocket.clientSock, nullptr);
    EXPECT_NE(dnsProxyRequestSocket.recvBuff, nullptr);
}

HWTEST_F(DnsProxyRequestSocketTest, Release_01, TestSize.Level0)
{
    int32_t sock = -1;
    std::unique_ptr<AlignedSockAddr> clientSock = nullptr;
    std::unique_ptr<RecvBuff> recvBuff = nullptr;
    DnsProxyRequestSocket dnsProxyRequestSocket(sock, std::move(clientSock), std::move(recvBuff));
    dnsProxyRequestSocket.~DnsProxyRequestSocket();
    EXPECT_EQ(dnsProxyRequestSocket.sock, sock);
}

HWTEST_F(DnsProxyRequestSocketTest, Release_02, TestSize.Level0)
{
    int32_t sock = socket(10000, SOCK_CLOEXEC, 0);
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    DnsProxyRequestSocket dnsProxyRequestSocket(sock, std::move(clientSock), std::move(recvBuff));
    dnsProxyRequestSocket.~DnsProxyRequestSocket();
    EXPECT_EQ(dnsProxyRequestSocket.sock, sock);
}

HWTEST_F(DnsProxyRequestSocketTest, DnsParseBySocket001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    clientSock->sa.sa_family = AF_INET;
    dnsproxylisten.DnsParseBySocket(recvBuff, clientSock);
    EXPECT_FALSE(dnsproxylisten.proxyListenSwitch_);
}

HWTEST_F(DnsProxyRequestSocketTest, DnsParseBySocket002, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    clientSock->sa.sa_family = 10;
    dnsproxylisten.DnsParseBySocket(recvBuff, clientSock);
    EXPECT_FALSE(dnsproxylisten.proxyListenSwitch_);
}

HWTEST_F(DnsProxyRequestSocketTest, DnsParseBySocket003, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    clientSock->sa.sa_family = 3;
    dnsproxylisten.DnsParseBySocket(recvBuff, clientSock);
    EXPECT_FALSE(dnsproxylisten.proxyListenSwitch_);
}

HWTEST_F(DnsProxyRequestSocketTest, GetDnsProxyServers001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    std::vector<std::string> servers = {"1", "2"};
    size_t serverIdx = 1;
    EXPECT_TRUE(dnsproxylisten.GetDnsProxyServers(servers, serverIdx, AF_UNSPEC));
}

HWTEST_F(DnsProxyRequestSocketTest, GetDnsProxyServers002, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    std::vector<std::string> servers = {"1"};
    size_t serverIdx = 1;
    EXPECT_FALSE(dnsproxylisten.GetDnsProxyServers(servers, serverIdx, AF_UNSPEC));
}

HWTEST_F(DnsProxyRequestSocketTest, MakeAddrInfo001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    AlignedSockAddr addrParse{};
    EXPECT_FALSE(dnsproxylisten.MakeAddrInfo("1", addrParse));
    EXPECT_FALSE(dnsproxylisten.MakeAddrInfo(".", addrParse));
    EXPECT_TRUE(dnsproxylisten.MakeAddrInfo("127.0.0.1", addrParse));
    EXPECT_EQ(addrParse.sin.sin_family, AF_INET);
    EXPECT_EQ(addrParse.sin.sin_port, htons(53));
}

HWTEST_F(DnsProxyRequestSocketTest, MakeAddrInfo002, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    AlignedSockAddr addrParse{};
    EXPECT_FALSE(dnsproxylisten.MakeAddrInfo("2", addrParse));
    EXPECT_FALSE(dnsproxylisten.MakeAddrInfo(":", addrParse));
    EXPECT_TRUE(dnsproxylisten.MakeAddrInfo("2400:c600:152a::1", addrParse));
    EXPECT_EQ(addrParse.sin6.sin6_family, AF_INET6);
    EXPECT_EQ(addrParse.sin6.sin6_port, htons(53));
}

/**
 * @tc.name: MakeAddrInfo003
 * @tc.desc: A server string that carries no recognizable address family is rejected.
 */
HWTEST_F(DnsProxyRequestSocketTest, MakeAddrInfo003, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    AlignedSockAddr addrParse{};
    EXPECT_FALSE(dnsproxylisten.MakeAddrInfo("", addrParse));
    EXPECT_FALSE(dnsproxylisten.MakeAddrInfo("invalid", addrParse));
    EXPECT_FALSE(dnsproxylisten.MakeAddrInfo("999.999.999.999", addrParse));
}

/**
 * @tc.name: MakeAddrInfo004
 * @tc.desc: The parsed address family comes from the server, so an IPv6 upstream is usable
 *           even when the client is IPv4. This is the regression case of the hotspot defect.
 */
HWTEST_F(DnsProxyRequestSocketTest, MakeAddrInfo004, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    AlignedSockAddr addrParse{};
    EXPECT_TRUE(dnsproxylisten.MakeAddrInfo("2400:c600:152a::1", addrParse));
    EXPECT_EQ(addrParse.sin6.sin6_family, AF_INET6);
    EXPECT_TRUE(dnsproxylisten.MakeAddrInfo("192.168.1.1", addrParse));
    EXPECT_EQ(addrParse.sin.sin_family, AF_INET);
}

/**
 * @tc.name: GetUpstreamFamily001
 * @tc.desc: An IPv4 client with an IPv6-only upstream list must get an IPv6 outbound socket.
 */
HWTEST_F(DnsProxyRequestSocketTest, GetUpstreamFamily001, TestSize.Level0)
{
    std::vector<std::string> servers = {"2400:c600:152a::1", "2400:c600:162a::1"};
    EXPECT_EQ(DnsProxyListen::GetUpstreamFamily(servers, AF_INET), AF_INET6);
    EXPECT_EQ(DnsProxyListen::GetUpstreamFamily(servers, AF_INET6), AF_INET6);
    std::vector<std::string> servers4 = {"192.168.1.1", "223.5.5.5"};
    EXPECT_EQ(DnsProxyListen::GetUpstreamFamily(servers4, AF_INET6), AF_INET);
}

/**
 * @tc.name: GetUpstreamFamily002
 * @tc.desc: A dual-stack upstream list keeps the same-family server, which preserves the
 *           existing behaviour for every non-hotspot scenario.
 */
HWTEST_F(DnsProxyRequestSocketTest, GetUpstreamFamily002, TestSize.Level0)
{
    std::vector<std::string> servers = {"192.168.1.1", "2400:c600:152a::1"};
    EXPECT_EQ(DnsProxyListen::GetUpstreamFamily(servers, AF_INET), AF_INET);
    EXPECT_EQ(DnsProxyListen::GetUpstreamFamily(servers, AF_INET6), AF_INET6);
    std::vector<std::string> empty;
    EXPECT_EQ(DnsProxyListen::GetUpstreamFamily(empty, AF_INET), AF_INET);
    EXPECT_EQ(DnsProxyListen::GetUpstreamFamily(empty, AF_INET6), AF_INET6);
}

/**
 * @tc.name: KeepServersOfFamily001
 * @tc.desc: Only the servers of the socket family survive the filter.
 */
HWTEST_F(DnsProxyRequestSocketTest, KeepServersOfFamily001, TestSize.Level0)
{
    std::vector<std::string> servers = {"2400:c600:152a::1", "192.168.1.1", "2400:c600:162a::1", "bad"};
    DnsProxyListen::KeepServersOfFamily(servers, AF_INET6);
    ASSERT_EQ(servers.size(), static_cast<size_t>(2));
    EXPECT_EQ(servers[0], "2400:c600:152a::1");
    EXPECT_EQ(servers[1], "2400:c600:162a::1");
    std::vector<std::string> servers4 = {"2400:c600:152a::1", "192.168.1.1"};
    DnsProxyListen::KeepServersOfFamily(servers4, AF_INET);
    ASSERT_EQ(servers4.size(), static_cast<size_t>(1));
    EXPECT_EQ(servers4[0], "192.168.1.1");
}

HWTEST_F(DnsProxyRequestSocketTest, SendRequest2Server001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    int32_t socketFd = 300;
    dnsproxylisten.SendRequest2Server(socketFd);
    EXPECT_EQ(socketFd, 300);
}

HWTEST_F(DnsProxyRequestSocketTest, SendRequest2Server002, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    int32_t socketFd = 300;
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    int32_t family = AF_INET;
    clientSock->sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    dnsproxylisten.serverIdxOfSocket.emplace(std::piecewise_construct, std::forward_as_tuple(socketFd),
                              std::forward_as_tuple(socketFd, std::move(clientSock), std::move(recvBuff),
                                                    family));;
    dnsproxylisten.SendRequest2Server(socketFd);
    EXPECT_EQ(socketFd, 300);
}

HWTEST_F(DnsProxyRequestSocketTest, SendDnsBack2Client001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    int32_t socketFd = 300;
    dnsproxylisten.SendDnsBack2Client(socketFd);
    EXPECT_EQ(socketFd, 300);
}

HWTEST_F(DnsProxyRequestSocketTest, GetRequestAndTransmit001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    int32_t family = AF_INET;
    dnsproxylisten.GetRequestAndTransmit(family);
    family = AF_INET6;
    dnsproxylisten.GetRequestAndTransmit(family);
    EXPECT_EQ(family, 10);
}

HWTEST_F(DnsProxyRequestSocketTest, CollectSocks001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    dnsproxylisten.CollectSocks();
    EXPECT_FALSE(dnsproxylisten.proxyListenSwitch_);
}

HWTEST_F(DnsProxyRequestSocketTest, EpollTimeout001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    dnsproxylisten.EpollTimeout();
    EXPECT_FALSE(dnsproxylisten.proxyListenSwitch_);
}

HWTEST_F(DnsProxyRequestSocketTest, CheckDnsQuestion001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    char data[100];
    char *recBuff = data;
    size_t recLen = 10;
    dnsproxylisten.CheckDnsQuestion(recBuff, recLen);
    recLen = 12;
    EXPECT_TRUE(dnsproxylisten.CheckDnsQuestion(recBuff, recLen));
}

HWTEST_F(DnsProxyRequestSocketTest, CheckDnsResponse001, TestSize.Level0)
{
    DnsProxyListen dnsproxylisten;
    char data[100];
    char *recBuff = data;
    size_t recLen = 2;
    dnsproxylisten.CheckDnsResponse(recBuff, recLen);
    recLen = 3;
    EXPECT_FALSE(dnsproxylisten.CheckDnsResponse(recBuff, recLen));
}
}  // namespace OHOS::nmd
