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
#define protected public
#endif

#include "net_manager_constants.h"
#include "netlink_socket.cpp"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;

void DealInfoFromKernelTest001()
{
    uint16_t clearThing = RTM_DELROUTE;
    uint32_t table = 0;
    DealInfoFromKernel(nullptr, clearThing, table);
}

void DealInfoFromKernelTest002()
{
    uint16_t clearThing = 0;
    uint32_t table = 0;
    struct nlmsghdr hdr = {0};
    DealInfoFromKernel(&hdr, clearThing, table);
}
} // namespace
class NetlinkSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetlinkSocketTest::SetUpTestCase()
{
}

void NetlinkSocketTest::TearDownTestCase() {}

void NetlinkSocketTest::SetUp() {}

void NetlinkSocketTest::TearDown() {}

HWTEST_F(NetlinkSocketTest, SendNetlinkMsgToKernelTest001, TestSize.Level1)
{
    DealInfoFromKernelTest001();
    DealInfoFromKernelTest002();
    uint32_t table = 0;
    auto ret = SendNetlinkMsgToKernel(nullptr, table);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetlinkSocketTest, ClearRouteInfoTest001, TestSize.Level1)
{
    uint16_t clearThing = RTM_GETROUTE;
    uint32_t table = 0;
    auto ret = ClearRouteInfo(clearThing, table);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetlinkSocketTest, ClearRouteInfoTest002, TestSize.Level1)
{
    uint16_t clearThing = RTM_GETRULE;
    uint32_t table = 0;
    auto ret = ClearRouteInfo(clearThing, table);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetlinkSocketTest, ClearRouteInfoTest003, TestSize.Level1)
{
    uint16_t clearThing = RTM_DELROUTE;
    uint32_t table = 0;
    auto ret = ClearRouteInfo(clearThing, table);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetlinkSocketTest, GetRoutePropertyTest001, TestSize.Level1)
{
    int32_t property = 0;
    auto ret = GetRouteProperty(nullptr, property);
    EXPECT_EQ(ret, -1);
}
} // namespace nmd
} // namespace OHOS