/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <sys/socket.h>
#include <netinet/in.h>

#include <poll.h>
#include <fcntl.h>

#include "iservice_registry.h"

#include "net_conn_manager_test_util.h"
#include "netsys_native_service_proxy.h"
#include "system_ability_definition.h"
#include "securec.h"
#include "dns_config_client.h"
#include "netnative_log_wrapper.h"

#define private public
#include "dns_proxy_listen.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;
using namespace NetManagerStandard::NetConnManagerTestUtil;
class DnsProxyListenTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DnsProxyListenTest::SetUpTestCase() {}

void DnsProxyListenTest::TearDownTestCase() {}

void DnsProxyListenTest::SetUp() {}

void DnsProxyListenTest::TearDown() {}

HWTEST_F(DnsProxyListenTest, DnsProxyTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("DnsProxyTest001 enter");
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    ASSERT_NE(netsysNativeService, nullptr);
    netsysNativeService->StartDnsProxyListen();
    const int32_t resSize = 512;
    unsigned char rsp[resSize] = {0};
    int proxySockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (proxySockFd < 0) {
        return;
    }
    sockaddr_in proxyAddr;
    socklen_t len;
    (void)memset_s(&proxyAddr, sizeof(proxyAddr), 0, sizeof(proxyAddr));
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    proxyAddr.sin_port = htons(53);

    unsigned char dnsSendData[] = { "\x58\x40\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x03\x77\x77\x77" \
        "\x05\x62\x61\x69\x64\x75\x03\x63\x6f\x6d\x00\x00\x01\x00\x01"};

    if (sendto(proxySockFd, dnsSendData, sizeof(dnsSendData), 0,
                reinterpret_cast<sockaddr *>(&proxyAddr), sizeof(proxyAddr)) < 0) {
        close(proxySockFd);
        return;
    }
    int flags = fcntl(proxySockFd, F_GETFL, 0);
    uint32_t tempFlags = static_cast<uint32_t>(flags) | O_NONBLOCK;
    fcntl(proxySockFd, F_SETFL, tempFlags);
    struct pollfd pfd;
    pfd.fd = proxySockFd;
    pfd.events = POLLIN;
    poll(&pfd, 1, 2000);
    len = sizeof(proxyAddr);
    recvfrom(proxySockFd, rsp, resSize, 0, reinterpret_cast<sockaddr *>(&proxyAddr), &len);
    close(proxySockFd);
}

HWTEST_F(DnsProxyListenTest, StartListenTest, TestSize.Level1)
{
    NETNATIVE_LOGI("StartListenTest enter");
    DnsProxyListen listener;
    listener.StartListen();
    listener.OnListen();
    listener.OffListen();
    listener.SetParseNetId(0);
    EXPECT_EQ(listener.netId_, 0);
}
} // namespace NetsysNative
} // namespace OHOS
