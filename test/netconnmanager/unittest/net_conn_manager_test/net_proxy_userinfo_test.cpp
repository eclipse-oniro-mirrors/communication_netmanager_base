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

#include "http_proxy.h"
#include "net_proxy_userinfo.h"

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;

class NetProxyUserinfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline NetProxyUserinfo instance_ = NetProxyUserinfo::GetInstance();
};

void NetProxyUserinfoTest::SetUpTestCase() {}

void NetProxyUserinfoTest::TearDownTestCase() {}

void NetProxyUserinfoTest::SetUp() {}

void NetProxyUserinfoTest::TearDown() {}

HWTEST_F(NetProxyUserinfoTest, SaveHttpProxyHostPassTest001, TestSize.Level1)
{
    HttpProxy httpProxy;
    httpProxy.SetHost("www.xxx.com");
    uint16_t port = 8080;
    httpProxy.SetPort(port);
    NetProxyUserinfoTest::instance_.SaveHttpProxyHostPass(httpProxy);
    EXPECT_EQ(httpProxy.GetUsername().empty(), true);
}

HWTEST_F(NetProxyUserinfoTest, GetHttpProxyHostPassTest001, TestSize.Level1)
{
    HttpProxy httpProxy;
    NetProxyUserinfoTest::instance_.GetHttpProxyHostPass(httpProxy);
    EXPECT_EQ(httpProxy.GetUsername().empty(), true);
}

} // namespace NetManagerStandard
} // namespace OHOS