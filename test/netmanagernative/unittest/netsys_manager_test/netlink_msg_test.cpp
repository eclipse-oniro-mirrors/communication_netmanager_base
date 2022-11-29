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

#include "netlink_msg.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;
class NetlinkMsgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetlinkMsgTest::SetUpTestCase() {}

void NetlinkMsgTest::TearDownTestCase() {}

void NetlinkMsgTest::SetUp() {}

void NetlinkMsgTest::TearDown() {}

HWTEST_F(NetlinkMsgTest, AddRouteTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("AddRouteTest001 enter");
    uint16_t flags = 2;
    size_t maxBufLen = 50;
    int32_t pid = 513;
    NetlinkMsg netLinkMsg(flags, maxBufLen, pid);
    rtmsg rtmsg = {
        .rtm_family = 0,
        .rtm_dst_len = 10,
        .rtm_src_len = 10,
    };
    uint16_t action = 2;
    netLinkMsg.AddRoute(action, rtmsg);
    fib_rule_hdr hdr = {
        .family = 0,
        .dst_len = 10,
        .src_len = 10,
    };
    ifaddrmsg addrmsg = {
        .ifa_family = 0,
        .ifa_prefixlen = 10,
        .ifa_flags = 1,
    };
    netLinkMsg.AddRule(action, hdr);
    netLinkMsg.AddAddress(action, addrmsg);
    size_t dataLength100 = 100;
    size_t dataLength10 = 10;
    int32_t ret = netLinkMsg.AddAttr(action, nullptr, dataLength100);
    EXPECT_EQ(ret, -1);
    char temp[10] = "123456789";
    char* data = temp;
    ret = netLinkMsg.AddAttr(action, data, dataLength100);
    EXPECT_EQ(ret, -1);
    ret = netLinkMsg.AddAttr(action, data, dataLength10);
    EXPECT_EQ(ret, 0);
}
} // namespace NetsysNative
} // namespace OHOS
