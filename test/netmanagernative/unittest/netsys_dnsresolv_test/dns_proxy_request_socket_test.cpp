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

#define private public

#include "dns_proxy_request_socket.h"
#include "netnative_log_wrapper.h"

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
    int32_t sock = 1;
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    DnsProxyRequestSocket dnsProxyRequestSocket(sock, std::move(clientSock), std::move(recvBuff));
    EXPECT_EQ(dnsProxyRequestSocket.sock, sock);
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
    int32_t sock = 1;
    std::unique_ptr<AlignedSockAddr> clientSock = std::make_unique<AlignedSockAddr>();
    std::unique_ptr<RecvBuff> recvBuff = std::make_unique<RecvBuff>();
    DnsProxyRequestSocket dnsProxyRequestSocket(sock, std::move(clientSock), std::move(recvBuff));
    dnsProxyRequestSocket.~DnsProxyRequestSocket();
    EXPECT_EQ(dnsProxyRequestSocket.sock, sock);
}

}  // namespace OHOS::nmd
