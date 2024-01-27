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
#include "net_http_probe.h"
#include "net_http_probe_result.h"
#include "net_link_info.h"
#include "net_manager_constants.h"

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
    instance_ = std::make_shared<NetHttpProbe>(TEST_NETID, NetBearType::BEARER_DEFAULT, NetLinkInfo());
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
    instance_->UpdateNetLinkInfo(info);
    instance_->UpdateGlobalHttpProxy(httpProxy);
    int32_t ret = instance_->SendProbe(PROBE_HTTP_HTTPS, TEST_HTTP_URL, TEST_HTTPS_URL);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetHttpProbeTest, HasProbeType001, TestSize.Level1)
{
    bool ret = instance_->HasProbeType(ProbeType::PROBE_HTTP, ProbeType::PROBE_HTTP_HTTPS);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetHttpProbeTest, NetHttpProbeBranchTest001, TestSize.Level1)
{
    instance_->SendHttpProbeRequest();
    instance_->RecvHttpProbeResponse();

    std::string domain = "";
    std::string result = instance_->GetAddrInfo(domain);
    ASSERT_TRUE(result.empty());

    int32_t port = 0;
    bool ret = instance_->SetResolveOption(ProbeType::PROBE_HTTP, domain, TEST_STRING, port);
    ASSERT_FALSE(ret);

    ret = instance_->SetResolveOption(ProbeType::PROBE_HTTP, TEST_STRING, TEST_STRING, port);
    ASSERT_FALSE(ret);

    ret = instance_->SetResolveOption(ProbeType::PROBE_HTTPS, TEST_STRING, TEST_STRING, port);
    ASSERT_FALSE(ret);

    bool useProxy = true;
    ret = instance_->SendDnsProbe(ProbeType::PROBE_HTTPS, TEST_STRING, TEST_STRING, useProxy);
    ASSERT_TRUE(ret);

    ret = instance_->SendDnsProbe(ProbeType::PROBE_HTTPS, TEST_STRING, TEST_STRING, useProxy);
    ASSERT_TRUE(ret);
}
} // namespace
} // namespace NetManagerStandard
} // namespace OHOS
