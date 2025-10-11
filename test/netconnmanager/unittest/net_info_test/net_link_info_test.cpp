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

#include "message_parcel.h"
#include "net_link_info.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *LOCAL_ROUTE_NEXT_HOP = "0.0.0.0";
constexpr const char *LOCAL_ROUTE_IPV6_DESTINATION = "::";
constexpr const char *TEST_IPV4_ADDR = "127.0.0.1";
} // namespace
class NetLinkInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetLinkInfoTest::SetUpTestCase() {}

void NetLinkInfoTest::TearDownTestCase() {}

void NetLinkInfoTest::SetUp() {}

void NetLinkInfoTest::TearDown() {}

sptr<NetLinkInfo> GetNetLinkInfo()
{
    sptr<NetLinkInfo> netLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    netLinkInfo->ifaceName_ = "test";
    netLinkInfo->domain_ = "test";

    sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
    netAddr->type_ = INetAddr::IPV4;
    netAddr->family_ = 0x10;
    netAddr->prefixlen_ = 0x17;
    netAddr->address_ = "0.0.0.0";
    netAddr->netMask_ = "0.0.0.0";
    netAddr->hostName_ = "netAddr";
    netLinkInfo->netAddrList_.push_back(*netAddr);

    sptr<Route> route = (std::make_unique<Route>()).release();
    route->iface_ = "iface0";
    route->destination_.type_ = INetAddr::IPV4;
    route->destination_.family_ = 0x10;
    route->destination_.prefixlen_ = 0x17;
    route->destination_.address_ = "0.0.0.0";
    route->destination_.netMask_ = "0.0.0.0";
    route->destination_.hostName_ = "netAddr";
    route->gateway_.type_ = INetAddr::IPV4;
    route->gateway_.family_ = 0x10;
    route->gateway_.prefixlen_ = 0x17;
    route->gateway_.address_ = "0.0.0.0";
    route->gateway_.netMask_ = "0.0.0.0";
    route->gateway_.hostName_ = "netAddr";
    netLinkInfo->routeList_.push_back(*route);

    netLinkInfo->mtu_ = 0x5DC;

    netLinkInfo->httpProxy_ = {TEST_IPV4_ADDR, 80, {"localhost"}};
    return netLinkInfo;
}

sptr<NetLinkInfo> GetOverSizeNetLinkInfo()
{
    sptr<NetLinkInfo> netLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    netLinkInfo->ifaceName_ = "test";
    netLinkInfo->domain_ = "test";

    int addrSize = 20;
    for (int i = 0; i < addrSize; i++) {
        sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
        netAddr->type_ = INetAddr::IPV4;
        netAddr->family_ = 0x10;
        netAddr->prefixlen_ = 0x17;
        netAddr->address_ = "0.0.0." + std::to_string(i);
        netAddr->netMask_ = "0.0.0.0";
        netAddr->hostName_ = "netAddr";
        netLinkInfo->netAddrList_.push_back(*netAddr);
    }

    for (int i = 0; i < addrSize; i++) {
        sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
        netAddr->type_ = INetAddr::IPV4;
        netAddr->family_ = 0x10;
        netAddr->prefixlen_ = 0x17;
        netAddr->address_ = "0.0.0." + std::to_string(i);
        netAddr->netMask_ = "0.0.0.0";
        netAddr->hostName_ = "netAddr";
        netLinkInfo->dnsList_.push_back(*netAddr);
    }

    int routeSize = 1080;
    for (int i = 0; i < routeSize; i++) {
        sptr<Route> route = (std::make_unique<Route>()).release();
        route->iface_ = "iface0";
        route->destination_.type_ = INetAddr::IPV4;
        route->destination_.family_ = 0x10;
        route->destination_.prefixlen_ = 0x17;
        route->destination_.address_ = "0.0.0." + std::to_string(i);
        route->destination_.netMask_ = "0.0.0.0";
        route->destination_.hostName_ = "netAddr";
        route->gateway_.type_ = INetAddr::IPV4;
        route->gateway_.family_ = 0x10;
        route->gateway_.prefixlen_ = 0x17;
        route->gateway_.address_ = "0.0.0.0";
        route->gateway_.netMask_ = "0.0.0.0";
        route->gateway_.hostName_ = "netAddr";
        netLinkInfo->routeList_.push_back(*route);
    }

    netLinkInfo->mtu_ = 0x5DC;

    netLinkInfo->httpProxy_ = {TEST_IPV4_ADDR, 80, {"localhost"}};
    return netLinkInfo;
}
/**
 * @tc.name: UnmarshallingTest
 * @tc.desc: Test NetLinkInfo::Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, UnmarshallingTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_TRUE(netLinkInfo != nullptr);

    MessageParcel data;
    sptr<NetLinkInfo> netLinkInfo_ptr = nullptr;
    bool bRet = NetLinkInfo::Marshalling(data, netLinkInfo);
    ASSERT_TRUE(bRet == true);

    netLinkInfo_ptr = NetLinkInfo::Unmarshalling(data);
    ASSERT_TRUE(netLinkInfo_ptr != nullptr);
}

/**
 * @tc.name: InitializeTest
 * @tc.desc: Test NetLinkInfo::Initialize
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, InitializeTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_TRUE(netLinkInfo != nullptr);
    netLinkInfo->Initialize();
}

/**
 * @tc.name: ToStringTest
 * @tc.desc: Test NetLinkInfo::ToString
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, ToStringTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    std::string str = netLinkInfo->ToString("testTab");
    int32_t ret = 0;
    NETMGR_LOG_D("netLinkInfo.ToString string is : [%{public}s]", str.c_str());
    if (str.c_str() != nullptr) {
        ret = 1;
    }
}

/**
 * @tc.name: ToStringAddrTest
 * @tc.desc: Test NetLinkInfo::ToStringAddr
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, ToStringAddrTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    std::string str = netLinkInfo->ToStringAddr("testAddrTab");
    int32_t ret = 0;
    NETMGR_LOG_D("netLinkInfo.ToString string is : [%{public}s]", str.c_str());
    if (str.c_str() != nullptr) {
        ret = 1;
    }
}

/**
 * @tc.name: ToStringDnsTest
 * @tc.desc: Test NetLinkInfo::ToStringDns
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, ToStringDnsTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    std::string str = netLinkInfo->ToStringDns("testDnsTab");
    int32_t ret = 0;
    NETMGR_LOG_D("netLinkInfo.ToString string is : [%{public}s]", str.c_str());
    if (str.c_str() != nullptr) {
        ret = 1;
    }
}

/**
 * @tc.name: ToStringRouteTest
 * @tc.desc: Test NetLinkInfo::ToStringRoute
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, ToStringRouteTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    std::string str = netLinkInfo->ToStringRoute("testRouteTab");
    int32_t ret = 0;
    NETMGR_LOG_D("netLinkInfo.ToString string is : [%{public}s]", str.c_str());
    if (str.c_str() != nullptr) {
        ret = 1;
    }
}

/**
 * @tc.name: operatorAndMarshalling
 * @tc.desc: Test NetLinkInfo::operatorAndMarshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, operatorAndMarshallingTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_TRUE(netLinkInfo != nullptr);
    NetLinkInfo netLinkInfoa = *netLinkInfo;
    ASSERT_EQ(netLinkInfoa.domain_, "test");
    Parcel data;
    bool bRet = netLinkInfo->Marshalling(data);
    ASSERT_TRUE(bRet == true);
}

/**
 * @tc.name: OversizeNetLinkinfoMarshallingTest
 * @tc.desc: Test NetLinkInfo::Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, OversizeNetLinkinfoMarshallingTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetOverSizeNetLinkInfo();
    ASSERT_TRUE(netLinkInfo != nullptr);
    ASSERT_TRUE(netLinkInfo->netAddrList_.size() == 20);
    ASSERT_TRUE(netLinkInfo->dnsList_.size() == 20);
    ASSERT_TRUE(netLinkInfo->routeList_.size() == 1080);

    MessageParcel data;
    sptr<NetLinkInfo> netLinkInfo_ptr = nullptr;
    bool bRet = netLinkInfo->Marshalling(data);
    ASSERT_TRUE(bRet == true);

    netLinkInfo_ptr = netLinkInfo->Unmarshalling(data);
    ASSERT_TRUE(netLinkInfo_ptr != nullptr);
    ASSERT_TRUE(netLinkInfo_ptr->netAddrList_.size() == 16);
    ASSERT_TRUE(netLinkInfo_ptr->dnsList_.size() == 16);
    ASSERT_TRUE(netLinkInfo_ptr->routeList_.size() == 1024);
}

/**
 * @tc.name: OversizeNetLinkinfoMarshallingTest02
 * @tc.desc: Test NetLinkInfo::Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, OversizeNetLinkinfoMarshallingTest02, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetOverSizeNetLinkInfo();
    ASSERT_TRUE(netLinkInfo != nullptr);
    ASSERT_TRUE(netLinkInfo->netAddrList_.size() == 20);
    ASSERT_TRUE(netLinkInfo->dnsList_.size() == 20);
    ASSERT_TRUE(netLinkInfo->routeList_.size() == 1080);

    MessageParcel data;
    sptr<NetLinkInfo> netLinkInfo_ptr = nullptr;
    bool bRet = NetLinkInfo::Marshalling(data, netLinkInfo);
    ASSERT_TRUE(bRet == true);

    netLinkInfo_ptr = NetLinkInfo::Unmarshalling(data);
    ASSERT_TRUE(netLinkInfo_ptr != nullptr);
    ASSERT_TRUE(netLinkInfo_ptr->netAddrList_.size() == 16);
    ASSERT_TRUE(netLinkInfo_ptr->dnsList_.size() == 16);
    ASSERT_TRUE(netLinkInfo_ptr->routeList_.size() == 1024);
}

/**
 * @tc.name: HasIpv6DefaultRoute
 * @tc.desc: Test NetLinkInfo::HasIpv6DefaultRoute
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, HasIpv6DefaultRouteTest001, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_IPV6_DESTINATION;
    route.destination_.type_ = INetAddr::IPV6;
    netLinkInfo->routeList_.push_back(route);
    ASSERT_FALSE(netLinkInfo->HasIpv4DefaultRoute());
    ASSERT_TRUE(netLinkInfo->HasIpv6DefaultRoute());
}

/**
 * @tc.name: HasIpv4DefaultRoute
 * @tc.desc: Test NetLinkInfo::HasIpv4DefaultRoute
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, HasIpv6DefaultRouteTest002, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    Route route;
    route.destination_.address_ = "192.168.0.1";
    route.destination_.type_ = INetAddr::IPV4;
    netLinkInfo->routeList_.push_back(route);
    ASSERT_TRUE(netLinkInfo->HasIpv4DefaultRoute());
    ASSERT_FALSE(netLinkInfo->HasIpv6DefaultRoute());
}

/**
 * @tc.name: HasIpv4Address
 * @tc.desc: Test NetLinkInfo::HasIpv4Address
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, HasIpv4AddressTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    INetAddr addr;
    addr.address_ = "192.168.0.1";
    addr.type_ = INetAddr::IPV4;
    netLinkInfo->netAddrList_.push_back(addr);
    ASSERT_TRUE(netLinkInfo->HasIpv4Address());
}

/**
 * @tc.name: HasIpv4DnsServer
 * @tc.desc: Test NetLinkInfo::HasIpv4DnsServer
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, HasIpv4DnsServerTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    INetAddr addr;
    addr.address_ = "192.168.0.1";
    addr.type_ = INetAddr::IPV4;
    netLinkInfo->dnsList_.push_back(addr);
    ASSERT_TRUE(netLinkInfo->HasIpv4DnsServer());
}

/**
 * @tc.name: IsIpv4Provisioned
 * @tc.desc: Test NetLinkInfo::IsIpv4Provisioned
 * @tc.type: FUNC
 */
HWTEST_F(NetLinkInfoTest, IsIpv4ProvisionedTest, TestSize.Level1)
{
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    INetAddr addr;
    addr.address_ = "192.168.0.1";
    addr.type_ = INetAddr::IPV4;
    netLinkInfo->dnsList_.push_back(addr);
    netLinkInfo->netAddrList_.push_back(addr);
    Route route;
    route.destination_.address_ = "192.168.0.1";
    route.destination_.type_ = INetAddr::IPV4;
    netLinkInfo->routeList_.push_back(route);
    ASSERT_TRUE(netLinkInfo->IsIpv4Provisioned());
}
} // namespace NetManagerStandard
} // namespace OHOS
