/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "route_utils.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class RouteUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RouteUtilsTest::SetUpTestCase() {}

void RouteUtilsTest::TearDownTestCase() {}

void RouteUtilsTest::SetUp() {}

void RouteUtilsTest::TearDown() {}

HWTEST_F(RouteUtilsTest, AddRouteToLocal01, TestSize.Level1)
{
    std::string iface("eth0");
    std::list<Route> rList;
    Route r;

    r.iface_ = iface;
    r.rtn_type_ = RTN_UNICAST;
    r.hasGateway_ = true;
    r.destination_.type_ = INetAddr::IPV4;
    r.destination_.family_ = AF_INET;
    r.destination_.prefixlen_ = 0x18;
    r.destination_.address_ = "172.17.5.221";
    r.destination_.netMask_ = "255.255.255.0";
    r.destination_.hostName_ = "netAddr";
    r.gateway_.type_ = INetAddr::IPV4;
    r.gateway_.family_ = AF_INET;
    r.gateway_.prefixlen_ = 0x18;
    r.gateway_.address_ = "172.17.5.1";
    r.gateway_.netMask_ = "255.255.255.0";
    r.gateway_.hostName_ = "netAddr";
    rList.push_back(r);

    EXPECT_EQ(-1, RouteUtils::AddRoutesToLocal(iface, rList));
}

HWTEST_F(RouteUtilsTest, RemoveRouteFromLocal01, TestSize.Level1)
{
    std::string iface("eth0");
    std::list<Route> rList;
    Route r;

    r.iface_ = iface;
    r.rtn_type_ = RTN_UNICAST;
    r.hasGateway_ = true;
    r.destination_.type_ = INetAddr::IPV4;
    r.destination_.family_ = AF_INET;
    r.destination_.prefixlen_ = 0x18;
    r.destination_.address_ = "192.168.2.10";
    r.destination_.netMask_ = "255.255.255.0";
    r.destination_.hostName_ = "netAddr";
    r.gateway_.type_ = INetAddr::IPV4;
    r.gateway_.family_ = AF_INET;
    r.gateway_.prefixlen_ = 0x18;
    r.gateway_.address_ = "192.168.2.1";
    r.gateway_.netMask_ = "255.255.255.0";
    r.gateway_.hostName_ = "netAddr";
    rList.push_back(r);

    EXPECT_EQ(0, RouteUtils::RemoveRoutesFromLocal(rList));
}

HWTEST_F(RouteUtilsTest, AddRoute01, TestSize.Level1)
{
    std::string iface("eth0");
    Route r;

    r.iface_ = iface;
    r.rtn_type_ = RTN_UNICAST;
    r.hasGateway_ = true;
    r.destination_.type_ = INetAddr::IPV4;
    r.destination_.family_ = AF_INET;
    r.destination_.prefixlen_ = 0x18;
    r.destination_.address_ = "192.168.2.10";
    r.destination_.netMask_ = "255.255.255.0";
    r.destination_.hostName_ = "netAddr";
    r.gateway_.type_ = INetAddr::IPV4;
    r.gateway_.family_ = AF_INET;
    r.gateway_.prefixlen_ = 0x18;
    r.gateway_.address_ = "192.168.2.1";
    r.gateway_.netMask_ = "255.255.255.0";
    r.gateway_.hostName_ = "netAddr";

    EXPECT_EQ(-1, RouteUtils::AddRoute(1, r));
}

HWTEST_F(RouteUtilsTest, RemoveRoute01, TestSize.Level1)
{
    std::string iface("eth0");
    Route r;

    r.iface_ = iface;
    r.rtn_type_ = RTN_UNICAST;
    r.hasGateway_ = true;
    r.destination_.type_ = INetAddr::IPV4;
    r.destination_.family_ = AF_INET;
    r.destination_.prefixlen_ = 0x18;
    r.destination_.address_ = "192.168.2.10";
    r.destination_.netMask_ = "255.255.255.0";
    r.destination_.hostName_ = "netAddr";
    r.gateway_.type_ = INetAddr::IPV4;
    r.gateway_.family_ = AF_INET;
    r.gateway_.prefixlen_ = 0x18;
    r.gateway_.address_ = "192.168.2.1";
    r.gateway_.netMask_ = "255.255.255.0";
    r.gateway_.hostName_ = "netAddr";

    EXPECT_EQ(0, RouteUtils::RemoveRoute(1, r));
}

HWTEST_F(RouteUtilsTest, UpdateRoutes01, TestSize.Level1)
{
    std::string iface("eth0");
    NetLinkInfo nlio;
    NetLinkInfo nlin;
    Route r;

    r.iface_ = iface;
    r.rtn_type_ = RTN_UNICAST;
    r.hasGateway_ = true;
    r.destination_.type_ = INetAddr::IPV4;
    r.destination_.family_ = AF_INET;
    r.destination_.prefixlen_ = 0x18;
    r.destination_.address_ = "192.168.2.2";
    r.destination_.netMask_ = "255.255.255.0";
    r.destination_.hostName_ = "netAddr";
    r.gateway_.type_ = INetAddr::IPV4;
    r.gateway_.family_ = AF_INET;
    r.gateway_.prefixlen_ = 0x18;
    r.gateway_.address_ = "192.168.2.1";
    r.gateway_.netMask_ = "255.255.255.0";
    r.gateway_.hostName_ = "netAddr";
    nlio.routeList_.push_back(r);

    EXPECT_EQ(1, RouteUtils::UpdateRoutes(1, nlin, nlio));
}
} // namespace NetManagerStandard
} // namespace OHOS