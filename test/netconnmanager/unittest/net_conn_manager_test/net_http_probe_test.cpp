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

#include "net_http_probe.h"
#include "net_link_info.h"
#include "net_manager_constants.h"
#include "net_http_probe_result.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t TEST_NETID = 999;
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

    instance_ =
        std::make_shared<NetHttpProbe>(TEST_NETID, NetBearType::BEARER_DEFAULT, NetLinkInfo());
}

void NetHttpProbeTest::TearDownTestCase() {}

void NetHttpProbeTest::SetUp() {}

void NetHttpProbeTest::TearDown() {}

HWTEST_F(NetHttpProbeTest, SendProbeTest001, TestSize.Level1)
{
    int32_t ret = instance_->SendProbe(PROBE_HTTP_HTTPS, TEST_HTTP_URL, TEST_HTTPS_URL);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

} // namespace
} // namespace NetManagerStandard
} // namespace OHOS