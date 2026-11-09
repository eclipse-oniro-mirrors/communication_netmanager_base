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
#endif
#include "net_http_client.h"
#include "net_http_probe.h"
#include "net_http_probe_result.h"
#include "net_link_info.h"
#include "net_manager_constants.h"
#include "net_proxy_userinfo.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t TEST_NETID = 999;
constexpr const char *TEST_PROXY_HOST = "testHttpProxy";
constexpr const char *TEST_STRING = "testString";
constexpr const char *TEST_HTTP_URL = "http://connectivitycheck.platform.hicloud.com/generate_204";
constexpr const char *TEST_HTTPS_URL = "https://connectivitycheck.platform.hicloud.com/generate_204";

class NetHttpProbeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetHttpProbe> instance_ = nullptr;
};

void NetHttpProbeTest::SetUpTestCase()
{
    instance_ = std::make_shared<NetHttpProbe>(TEST_NETID, NetBearType::BEARER_DEFAULT, NetLinkInfo(), PROBE_HTTP);
}

void NetHttpProbeTest::TearDownTestCase() {}

void NetHttpProbeTest::SetUp() {}

void NetHttpProbeTest::TearDown() {}

HWTEST_F(NetHttpProbeTest, SendProbeTest001, TestSize.Level1)
{
    instance_->GetHttpProbeResult();
    instance_->GetHttpsProbeResult();
    HttpProxy httpProxy = {TEST_PROXY_HOST, 0, {}};
    NetLinkInfo info;
    instance_->netLinkInfo_ = info;
    instance_->UpdateGlobalHttpProxy(httpProxy);
    /* A combined probe type performs no transfer and keeps failing fast. */
    int32_t ret = instance_->SendProbe(PROBE_HTTP_HTTPS, TEST_HTTP_URL, TEST_HTTPS_URL);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetHttpProbeTest, NetHttpProbeBranchTest001, TestSize.Level1)
{
    std::string domain = "";
    std::string result = instance_->GetAddrInfo(domain);
    ASSERT_TRUE(result.empty());

    NetHttp::HttpRequest request;
    bool ret = instance_->AppendResolve(request, domain, TEST_STRING, 0);
    ASSERT_FALSE(ret);

    ret = instance_->AppendResolve(request, TEST_STRING, TEST_STRING, 0);
    ASSERT_TRUE(ret);
    ASSERT_EQ(request.resolves.size(), 1);

    bool useProxy = true;
    ret = instance_->SendDnsProbe(ProbeType::PROBE_HTTPS, TEST_STRING, request, useProxy);
    ASSERT_TRUE(ret);

    useProxy = false;
    ret = instance_->SendDnsProbe(ProbeType::PROBE_HTTPS, TEST_STRING, request, useProxy);
    ASSERT_FALSE(ret);

    instance_->ipAddrList_ = "192.168.1.1";
    ret = instance_->SendDnsProbe(ProbeType::PROBE_HTTPS, TEST_STRING, request, useProxy);
    instance_->ipAddrList_ = "";
    ASSERT_TRUE(ret);
}

HWTEST_F(NetHttpProbeTest, ExtractDomainFormUrl001, TestSize.Level1)
{
    std::string url = "";

    std::string ret = instance_->ExtractDomainFormUrl(url);
    EXPECT_EQ(ret, std::string());
    ret = instance_->ExtractDomainFormUrl(TEST_STRING);
    EXPECT_EQ(ret, TEST_STRING);
}

HWTEST_F(NetHttpProbeTest, BuildHttpRequest001, TestSize.Level1)
{
    ProbeType probeType = ProbeType::PROBE_HTTP_HTTPS;
    NetHttp::HttpRequest request;
    std::string url = "";

    bool ret = instance_->BuildHttpRequest(probeType, url, request);
    ASSERT_FALSE(ret);
    ret = instance_->BuildHttpRequest(ProbeType::PROBE_HTTP, url, request);
    ASSERT_FALSE(ret);

    /* Skip DNS resolution with a preset address list, no network needed. */
    instance_->ipAddrList_ = "192.168.1.1";
    ret = instance_->BuildHttpRequest(ProbeType::PROBE_HTTP, TEST_HTTPS_URL, request);
    instance_->ipAddrList_ = "";
    ASSERT_TRUE(ret);
    EXPECT_EQ(request.url, TEST_HTTPS_URL);
    EXPECT_EQ(request.method, NetHttp::HttpMethod::GET);
    EXPECT_EQ(request.bindInterface, std::string());
    EXPECT_FALSE(request.resolves.empty());
}

HWTEST_F(NetHttpProbeTest, SetProxyOptionTest001, TestSize.Level1)
{
    ProbeType probeType = ProbeType::PROBE_HTTP;
    NetHttp::HttpRequest request;
    HttpProxy httpProxy = {"", 0, {}};
    bool useHttpProxy = true;
    instance_->defaultUseGlobalHttpProxy_ = true;

    instance_->UpdateGlobalHttpProxy(httpProxy);
    bool ret = instance_->SetProxyOption(probeType, request, useHttpProxy);
    ASSERT_TRUE(ret);
    ASSERT_FALSE(useHttpProxy);

    /* Non wifi/ethernet networks never probe through a proxy. */
    ret = instance_->SetProxyOption(ProbeType::PROBE_HTTPS, request, useHttpProxy);
    ASSERT_TRUE(ret);
    ASSERT_FALSE(useHttpProxy);

    /* On wifi an unresolvable proxy host fails the whole probe setup. */
    instance_->netBearType_ = NetBearType::BEARER_WIFI;
    httpProxy = {"httpProxy", 0, {}};
    instance_->UpdateGlobalHttpProxy(httpProxy);
    ret = instance_->SetProxyOption(probeType, request, useHttpProxy);
    instance_->netBearType_ = NetBearType::BEARER_DEFAULT;
    ASSERT_FALSE(ret);
}

HWTEST_F(NetHttpProbeTest, SendDnsProbeTest001, TestSize.Level1)
{
    bool useProxy = false;
    NetHttp::HttpRequest request;
    bool ret = instance_->SendDnsProbe(ProbeType::PROBE_HTTP, "", request, useProxy);
    ASSERT_FALSE(ret);

    ret = instance_->SendDnsProbe(ProbeType::PROBE_HTTPS, TEST_STRING, request, useProxy);
    ASSERT_FALSE(ret);
}

HWTEST_F(NetHttpProbeTest, ExecuteProbe001, TestSize.Level1)
{
    /* Connection refused on the loopback keeps this network independent. */
    instance_->ipAddrList_ = "127.0.0.1";
    NetHttpProbeResult result;
    bool ret = instance_->ExecuteProbe(ProbeType::PROBE_HTTP, "http://127.0.0.1:1/generate_204", result);
    instance_->ipAddrList_ = "";
    ASSERT_TRUE(ret);
    /* A failed transfer historically maps to response code 0. */
    EXPECT_EQ(result.GetCode(), 0);

    PortalResponse resp;
    ret = instance_->NetDetection("", resp);
    ASSERT_FALSE(ret);
    ret = instance_->NetDetection(std::string(MAX_URL_LEN + 1, 'a'), resp);
    ASSERT_FALSE(ret);
    ret = instance_->NetDetection("http://127.0.0.1:1/portal", resp);
    ASSERT_FALSE(ret);
}

HWTEST_F(NetHttpProbeTest, LoadProxyTest001, TestSize.Level1)
{
    instance_->defaultUseGlobalHttpProxy_ = true;
    HttpProxy httpProxy = {"httpProxy", 0, {}};
    std::string proxyHost = "proxyHost";
    int32_t proxyPort = 0;

    instance_->UpdateGlobalHttpProxy(httpProxy);
    bool ret = instance_->LoadProxy(proxyHost, proxyPort);
    ASSERT_TRUE(ret);

    instance_->defaultUseGlobalHttpProxy_ = false;
    NetLinkInfo netLinkInfo;
    netLinkInfo.httpProxy_ = {"127.0.0.1", 80, {"localhost"}};
    instance_->netLinkInfo_ = netLinkInfo;
    ret = instance_->LoadProxy(proxyHost, proxyPort);
    ASSERT_TRUE(ret);
    instance_->netLinkInfo_ = NetLinkInfo();
}

HWTEST_F(NetHttpProbeTest, FillProxyUserInfoTest001, TestSize.Level1)
{
    HttpProxy httpProxy = {"127.0.0.1", 8080, {}};
    SecureData username;
    username.append("testuser", strlen("testuser"));
    SecureData password;
    password.append("testpass", strlen("testpass"));
    httpProxy.SetUserName(username);
    httpProxy.SetPassword(password);
    NetProxyUserinfo::GetInstance().SaveHttpProxyHostPass(httpProxy);
    NetHttp::HttpProxyConfig proxyConfig;
    instance_->FillProxyUserInfo(proxyConfig);
    EXPECT_EQ(proxyConfig.username, "testuser");
    EXPECT_EQ(proxyConfig.password, "testpass");
}

HWTEST_F(NetHttpProbeTest, FillProxyUserInfoTest002, TestSize.Level1)
{
    /* Seed the persistent store with a known row first so this test does not
     * depend on state left by other tests or by previous runs on the device. */
    HttpProxy seedProxy = {"127.0.0.1", 8080, {}};
    SecureData seedUser;
    seedUser.append("seeduser", strlen("seeduser"));
    SecureData seedPass;
    seedPass.append("seedpass", strlen("seedpass"));
    seedProxy.SetUserName(seedUser);
    seedProxy.SetPassword(seedPass);
    NetProxyUserinfo::GetInstance().SaveHttpProxyHostPass(seedProxy);
    /* A partial save (username without password) is specified as a no-op: the
     * previously stored row must survive untouched. */
    HttpProxy httpProxy = {"127.0.0.1", 8080, {}};
    SecureData username;
    username.append("testuser", strlen("testuser"));
    httpProxy.SetUserName(username);
    NetProxyUserinfo::GetInstance().SaveHttpProxyHostPass(httpProxy);
    NetHttp::HttpProxyConfig proxyConfig;
    instance_->FillProxyUserInfo(proxyConfig);
    EXPECT_EQ(proxyConfig.username, "seeduser");
    EXPECT_EQ(proxyConfig.password, "seedpass");
}
} // namespace
} // namespace NetManagerStandard
} // namespace OHOS
