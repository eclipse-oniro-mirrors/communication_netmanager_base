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

#include <sys/socket.h>

#include <gtest/gtest.h>

#include "net_manager_constants.h"
#include "route_utils.h"
#include "net_conn_types.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
Route GetRoute()
{
    std::string iface("eth0");
    Route route;
    route.iface_ = iface;
    route.rtnType_ = RTN_UNICAST;
    route.hasGateway_ = true;
    route.isDefaultRoute_ = false;
    route.destination_.type_ = INetAddr::IPV4;
    route.destination_.family_ = AF_INET;
    route.destination_.prefixlen_ = 0x18;
    route.destination_.address_ = "192.168.2.10";
    route.destination_.netMask_ = "255.255.255.0";
    route.destination_.hostName_ = "netAddr";
    route.gateway_.type_ = INetAddr::IPV4;
    route.gateway_.family_ = AF_INET;
    route.gateway_.prefixlen_ = 0x18;
    route.gateway_.address_ = "192.168.2.1";
    route.gateway_.netMask_ = "255.255.255.0";
    route.gateway_.hostName_ = "netAddr";
    return route;
}

constexpr uint32_t TEST_NETID = 110;
} // namespace

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
    std::list<Route> rList;
    std::string iface("eth0");
    rList.push_back(GetRoute());
    RouteUtils::AddRoutesToLocal(iface, rList);
    EXPECT_FALSE(rList.empty());
}

HWTEST_F(RouteUtilsTest, RemoveRouteFromLocal01, TestSize.Level1)
{
    std::list<Route> rList;
    rList.push_back(GetRoute());

    EXPECT_EQ(0, RouteUtils::RemoveRoutesFromLocal(rList));
}

HWTEST_F(RouteUtilsTest, AddRoute01, TestSize.Level1)
{
    EXPECT_GE(0, RouteUtils::AddRoute(TEST_NETID, GetRoute()));
}

HWTEST_F(RouteUtilsTest, RemoveRoute01, TestSize.Level1)
{
    EXPECT_GE(0, RouteUtils::RemoveRoute(TEST_NETID, GetRoute()));
}

HWTEST_F(RouteUtilsTest, UpdateRoutes01, TestSize.Level1)
{
    NetLinkInfo nlio;
    NetLinkInfo nlin;
    nlio.routeList_.push_back(GetRoute());

    EXPECT_TRUE(RouteUtils::UpdateRoutes(TEST_NETID, nlin, nlio));
}
} // namespace NetManagerStandard
} // namespace OHOS