/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "network.h"
#include "nat464_service.h"
#include "net_http_probe_result.h"
#include "probe_thread.h"
#include "net_connection.h"
#include "net_connection_adapter.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *LOCAL_ROUTE_NEXT_HOP = "0.0.0.0";
constexpr const char *LOCAL_ROUTE_IPV6_DESTINATION = "::";
constexpr int32_t SUCCESS_CODE = 204;
constexpr int32_t PORTAL_CODE_MIN = 200;
constexpr int32_t PORTAL_CODE_MAX = 399;
constexpr size_t CURL_MAX_SIZE = 1024;
constexpr size_t CURL_MAX_NITEMS = 100;
constexpr int32_t HTTP_OK_CODE = 200;
constexpr int32_t DEFAULT_CONTENT_LENGTH_VALUE = -1;
constexpr int32_t MIN_VALID_CONTENT_LENGTH_VALUE = 5;
constexpr int32_t FAIL_CODE = 599;
constexpr int32_t PORTAL_CODE = 302;
constexpr int32_t HTTP_RES_CODE_BAD_REQUEST = 400;
constexpr int32_t HTTP_RES_CODE_CLIENT_ERRORS_MAX = 499;
const std::string CONNECTION_CLOSE_VALUE = "close";
const std::string CONNECTION_KEY = "Connection:";
const std::string CONTENT_LENGTH_KEY = "Content-Length:";
const std::string KEY_WORDS_REDIRECTION = "location.replace";
const std::string HTML_TITLE_HTTP_EN = "http://";
const std::string HTML_TITLE_HTTPS_EN = "https://";
constexpr int32_t VALID_NETID_START = 100;
constexpr int32_t PAC_URL_MAX_LEN = 1024;
constexpr int32_t DNS_NUM_TEST = 5;
} // namespace

class NetworkTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}

    void SetUp() {}

    void TearDown() {}
};

HWTEST_F(NetworkTest, UpdateBasicNetworkTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    network->nat464Service_ = std::make_unique<Nat464Service>(netId, "ifaceName");
    auto ret = network->UpdateBasicNetwork(false);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, ReleaseVirtualNetworkTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    EXPECT_FALSE(network->isVirtualCreated_);
    auto ret = network->ReleaseVirtualNetwork();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, ReleaseVirtualNetworkTest002, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    network->isVirtualCreated_ = true;
    INetAddr addr;
    network->netLinkInfo_.netAddrList_.push_back(addr);
    auto ret = network->ReleaseVirtualNetwork();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, ReleaseVirtualNetworkTest003, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    network->isVirtualCreated_ = true;
    INetAddr addr;
    addr.prefixlen_ = 24;
    network->netLinkInfo_.netAddrList_.push_back(addr);
    auto ret = network->ReleaseVirtualNetwork();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, GetNetLinkInfoTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
    EXPECT_EQ(network->netSupplierType_, NetBearType::BEARER_VPN);
    network->GetNetLinkInfo();
}

HWTEST_F(NetworkTest, GetNetLinkInfoTest002, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    Route route1;
    route1.destination_.address_ = LOCAL_ROUTE_NEXT_HOP;
    Route route2;
    route2.destination_.address_ = LOCAL_ROUTE_IPV6_DESTINATION;
    Route route3;
    route3.destination_.address_ = "192.168.1.1";
    network->netLinkInfo_.routeList_.push_back(route1);
    network->netLinkInfo_.routeList_.push_back(route2);
    network->netLinkInfo_.routeList_.push_back(route3);
    auto ret = network->GetNetLinkInfo();
    EXPECT_EQ(ret.routeList_.size(), 2);
}

HWTEST_F(NetworkTest, UpdateInterfacesTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
    network->netLinkInfo_.ifaceName_ = "test";
    NetLinkInfo newNetLinkInfo;
    EXPECT_TRUE(newNetLinkInfo.ifaceName_.empty());
    network->UpdateInterfaces(newNetLinkInfo);
    EXPECT_TRUE(network->netLinkInfo_.ifaceName_.empty());
}

HWTEST_F(NetworkTest, UpdateIpAddrsTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
    EXPECT_NE(network, nullptr);
    INetAddr addr1;
    INetAddr addr2;
    addr2.prefixlen_ = 24;
    network->netLinkInfo_.netAddrList_.push_back(addr1);
    network->netLinkInfo_.netAddrList_.push_back(addr2);
    NetLinkInfo newNetLinkInfo;
    newNetLinkInfo.netAddrList_.push_back(addr1);
    network->UpdateIpAddrs(newNetLinkInfo);
}

HWTEST_F(NetworkTest, HandleUpdateIpAddrsTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
    EXPECT_NE(network, nullptr);
    INetAddr addr1;
    INetAddr addr2;
    addr2.prefixlen_ = 24;
    network->netLinkInfo_.netAddrList_.push_back(addr1);
    network->netLinkInfo_.netAddrList_.push_back(addr2);
    NetLinkInfo newNetLinkInfo;
    newNetLinkInfo.netAddrList_.push_back(addr1);
    network->HandleUpdateIpAddrs(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
    EXPECT_NE(network, nullptr);
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_NEXT_HOP;
    network->netLinkInfo_.routeList_.push_back(route);
    NetLinkInfo newNetLinkInfo;
    newNetLinkInfo.routeList_.push_back(route);
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest002, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
    EXPECT_NE(network, nullptr);
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_NEXT_HOP;
    network->netLinkInfo_.routeList_.push_back(route);
    NetLinkInfo newNetLinkInfo;
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest003, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_NEXT_HOP;
    network->netLinkInfo_.routeList_.push_back(route);
    NetLinkInfo newNetLinkInfo;
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest004, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_IPV6_DESTINATION;
    network->netLinkInfo_.routeList_.push_back(route);
    NetLinkInfo newNetLinkInfo;
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest005, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    Route route;
    route.destination_.address_ = "192.168.1.1";
    network->netLinkInfo_.routeList_.push_back(route);
    NetLinkInfo newNetLinkInfo;
    EXPECT_TRUE(newNetLinkInfo.routeList_.empty());
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest006, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
    EXPECT_NE(network, nullptr);
    EXPECT_TRUE(network->netLinkInfo_.routeList_.empty());
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_NEXT_HOP;
    NetLinkInfo newNetLinkInfo;
    newNetLinkInfo.routeList_.push_back(route);
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest007, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    EXPECT_TRUE(network->netLinkInfo_.routeList_.empty());
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_NEXT_HOP;
    NetLinkInfo newNetLinkInfo;
    newNetLinkInfo.routeList_.push_back(route);
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest008, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    EXPECT_TRUE(network->netLinkInfo_.routeList_.empty());
    Route route;
    route.destination_.address_ = LOCAL_ROUTE_IPV6_DESTINATION;
    NetLinkInfo newNetLinkInfo;
    newNetLinkInfo.routeList_.push_back(route);
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateRoutesTest009, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    EXPECT_TRUE(network->netLinkInfo_.routeList_.empty());
    Route route;
    route.destination_.address_ = "192.168.1.1";
    NetLinkInfo newNetLinkInfo;
    newNetLinkInfo.routeList_.push_back(route);
    network->UpdateRoutes(newNetLinkInfo);
}

HWTEST_F(NetworkTest, UpdateDnsTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    NetLinkInfo netLinkInfo;
    EXPECT_TRUE(netLinkInfo.dnsList_.empty());
    network->UpdateDns(netLinkInfo);
    NetManagerStandard::INetAddr dns;
    dns.type_ = NetManagerStandard::INetAddr::IPV4;
    NetManagerStandard::INetAddr ipv6Dns;
    ipv6Dns.type_ = NetManagerStandard::INetAddr::IPV6;
    for (int32_t i = 0 ; i < DNS_NUM_TEST; i++) {
        dns.address_ = "99.99.99.99";
        ipv6Dns.address_ = "fe80::99:99:99:99";
        netLinkInfo.dnsList_.push_back(dns);
        netLinkInfo.dnsList_.push_back(ipv6Dns);
    }
    network->UpdateDns(netLinkInfo);
    dns.type_ = NetManagerStandard::INetAddr::UNKNOWN;
    netLinkInfo.dnsList_.push_back(dns);
    network->UpdateDns(netLinkInfo);
}

HWTEST_F(NetworkTest, UpdateTcpBufferSizeTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    NetLinkInfo netLinkInfo;
    netLinkInfo.tcpBufferSizes_ = "4096";
    EXPECT_NE(netLinkInfo.tcpBufferSizes_, network->netLinkInfo_.tcpBufferSizes_);
    network->UpdateTcpBufferSize(netLinkInfo);
}

HWTEST_F(NetworkTest, NetDetectionForDnsHealthTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    network->InitNetMonitor();
    EXPECT_NE(network->netMonitor_, nullptr);
    network->detectResult_ = INVALID_DETECTION_STATE;
    network->NetDetectionForDnsHealth(false);
}

HWTEST_F(NetworkTest, HandleNetMonitorResultTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    NetDetectionStatus state = UNKNOWN_STATE;
    std::string urlRedirect = "test";
    network->netCallback_ = nullptr;
    network->HandleNetMonitorResult(state, urlRedirect);
}

HWTEST_F(NetworkTest, HandleNetMonitorResultTest003, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    NetDetectionStatus state = UNKNOWN_STATE;
    std::string urlRedirect = "test";
    network->netCallback_ = [](uint32_t supplierId, NetDetectionStatus netState) {};
    network->detectResult_ = state;
    network->HandleNetMonitorResult(state, urlRedirect);
}

HWTEST_F(NetworkTest, HandleNetMonitorResultTest002, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    NetDetectionStatus state = UNKNOWN_STATE;
    std::string urlRedirect = "test";
    network->netCallback_ = [](uint32_t supplierId, NetDetectionStatus netState) {};
    network->detectResult_ = INVALID_DETECTION_STATE;
    network->HandleNetMonitorResult(state, urlRedirect);
}

HWTEST_F(NetworkTest, NotifyNetDetectionResultTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    NetDetectionResultCode detectionResult = NET_DETECTION_FAIL;
    std::string urlRedirect = "test";
    network->netDetectionRetCallback_.push_back(nullptr);
    network->NotifyNetDetectionResult(detectionResult, urlRedirect);
}

HWTEST_F(NetworkTest, NetDetectionResultConvertTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    int32_t internalRet = static_cast<int32_t>(INVALID_DETECTION_STATE);
    auto ret = network->NetDetectionResultConvert(internalRet);
    EXPECT_EQ(ret, NET_DETECTION_FAIL);
}

HWTEST_F(NetworkTest, UpdateNetConnStateTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    network->netLinkInfo_.ifaceName_ = "test";
    network->state_ = NET_CONN_STATE_CONNECTED;
    network->nat464Service_ = nullptr;
    EXPECT_TRUE(network->netLinkInfo_.netAddrList_.empty());
    NetConnState netConnState = NET_CONN_STATE_IDLE;
    network->UpdateNetConnState(netConnState);
}

HWTEST_F(NetworkTest, UpdateNetConnStateTest002, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    network->netLinkInfo_.ifaceName_ = "test";
    network->state_ = NET_CONN_STATE_CONNECTED;
    network->nat464Service_ = std::make_unique<Nat464Service>(netId, "test");
    EXPECT_TRUE(network->netLinkInfo_.netAddrList_.empty());
    NetConnState netConnState = NET_CONN_STATE_IDLE;
    network->UpdateNetConnState(netConnState);
}

HWTEST_F(NetworkTest, IsNat464PreferedTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    network->netLinkInfo_.ifaceName_ = "test";
    EXPECT_TRUE(network->netLinkInfo_.netAddrList_.empty());
    auto ret = network->IsNat464Prefered();
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, IsNat464PreferedTest002, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_NE(network, nullptr);
    network->netLinkInfo_.ifaceName_ = "test";
    network->state_ = NET_CONN_STATE_CONNECTED;
    EXPECT_TRUE(network->netLinkInfo_.netAddrList_.empty());
    auto ret = network->IsNat464Prefered();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, SetScreenStateTest001, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    EXPECT_EQ(network->netMonitor_, nullptr);
    network->SetScreenState(false);
}

HWTEST_F(NetworkTest, SetScreenStateTest002, TestSize.Level1)
{
    int32_t netId = 1;
    auto network = std::make_shared<Network>(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
    network->InitNetMonitor();
    EXPECT_NE(network->netMonitor_, nullptr);
    network->SetScreenState(false);
}

HWTEST_F(NetworkTest, MaybeUpdateV6IfaceTest001, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    service->MaybeUpdateV6Iface(v6Iface);

    service->serviceState_ = NAT464_SERVICE_STATE_DISCOVERING;
    service->MaybeUpdateV6Iface(v6Iface);
}

HWTEST_F(NetworkTest, UpdateServiceStateTest001, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    Nat464UpdateFlag updateFlag = NAT464_SERVICE_STOP;
    service->UpdateServiceState(updateFlag);
    EXPECT_EQ(service->serviceState_, NAT464_SERVICE_STATE_IDLE);

    updateFlag = NAT464_SERVICE_CONTINUE;
    service->UpdateServiceState(updateFlag);
    EXPECT_EQ(service->serviceState_, NAT464_SERVICE_STATE_DISCOVERING);
}

HWTEST_F(NetworkTest, UpdateServiceStateTest002, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    service->serviceState_ = NAT464_SERVICE_STATE_DISCOVERING;
    Nat464UpdateFlag updateFlag = NAT464_SERVICE_STOP;
    service->UpdateServiceState(updateFlag);
    EXPECT_EQ(service->serviceState_, NAT464_SERVICE_STATE_IDLE);

    service->serviceState_ = NAT464_SERVICE_STATE_DISCOVERING;
    updateFlag = NAT464_SERVICE_CONTINUE;
    EXPECT_TRUE(service->nat64PrefixFromDns_.address_.empty());
    service->UpdateServiceState(updateFlag);
    EXPECT_EQ(service->serviceState_, NAT464_SERVICE_STATE_DISCOVERING);

    service->nat64PrefixFromDns_.address_ = "test";
    service->UpdateServiceState(updateFlag);
    EXPECT_EQ(service->serviceState_, NAT464_SERVICE_STATE_RUNNING);
}

HWTEST_F(NetworkTest, UpdateServiceStateTest003, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    service->serviceState_ = NAT464_SERVICE_STATE_RUNNING;
    Nat464UpdateFlag updateFlag = NAT464_SERVICE_CONTINUE;
    service->UpdateServiceState(updateFlag);

    updateFlag = NAT464_SERVICE_STOP;
    service->UpdateServiceState(updateFlag);
    EXPECT_EQ(service->serviceState_, NAT464_SERVICE_STATE_IDLE);

    service->serviceState_ = static_cast<Nat464ServiceState>(-1);
    service->UpdateServiceState(updateFlag);
}

HWTEST_F(NetworkTest, DiscoverPrefixTest001, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    service->tryStopDiscovery_ = true;
    service->DiscoverPrefix();
    EXPECT_FALSE(service->tryStopDiscovery_);
}

HWTEST_F(NetworkTest, DiscoverPrefixTest002, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    EXPECT_FALSE(service->tryStopDiscovery_);
    EXPECT_FALSE(service->GetPrefixFromDns64());
    service->DiscoverPrefix();
}

HWTEST_F(NetworkTest, GetPrefixFromDns64Test001, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    auto ret = service->GetPrefixFromDns64();
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, StartServiceTest001, TestSize.Level1)
{
    int32_t netId = 1;
    std::string v6Iface;
    auto service = std::make_shared<Nat464Service>(netId, v6Iface);
    EXPECT_NE(service, nullptr);
    service->serviceState_ = NAT464_SERVICE_STATE_RUNNING;
    service->StartService();

    service->serviceState_ = NAT464_SERVICE_STATE_DISCOVERING;
    service->StartService();
}

HWTEST_F(NetworkTest, NetHttpProbeResultTest001, TestSize.Level1)
{
    NetHttpProbeResult result1;
    NetHttpProbeResult result2;
    result1.responseCode_ = SUCCESS_CODE;
    auto ret = result1 == result2;
    EXPECT_FALSE(ret);

    result2.responseCode_ = SUCCESS_CODE;
    ret = result1 == result2;
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, NetHttpProbeResultTest002, TestSize.Level1)
{
    NetHttpProbeResult result1;
    NetHttpProbeResult result2;
    result1.responseCode_ = PORTAL_CODE_MIN;
    auto ret = result1 == result2;
    EXPECT_FALSE(ret);

    result2.responseCode_ = PORTAL_CODE_MIN;
    EXPECT_EQ(result1.redirectUrl_, result2.redirectUrl_);
    ret = result1 == result2;
    EXPECT_TRUE(ret);

    result1.redirectUrl_ = "test";
    ret = result1 == result2;
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, NetHttpProbeResultTest003, TestSize.Level1)
{
    NetHttpProbeResult result1;
    NetHttpProbeResult result2;
    auto ret = result1 == result2;
    EXPECT_TRUE(ret);

    result2.responseCode_ = SUCCESS_CODE;
    ret = result1 == result2;
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, SendHttpProbeTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    std::string httpUrl = "test";
    std::string httpsUrl = "test";
    ProbeType probeType = PROBE_HTTP;
    auto latch = std::make_shared<TinyCountDownLatch>(0);
    auto probeThread = std::make_shared<ProbeThread>(netId, BEARER_CELLULAR, netLinkInfo,
        latch, latch, probeType, httpUrl, httpsUrl);
    probeThread->httpProbe_ = nullptr;
    probeThread->SendHttpProbe(probeType);
    EXPECT_FALSE(probeThread->isDetecting_);
}

HWTEST_F(NetworkTest, IsConclusiveResultTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    std::string httpUrl = "test";
    std::string httpsUrl = "test";
    ProbeType probeType = PROBE_HTTP_HTTPS;
    auto probeThread = std::make_shared<ProbeThread>(netId, BEARER_CELLULAR, netLinkInfo,
        nullptr, nullptr, probeType, httpUrl, httpsUrl);
    auto ret = probeThread->IsConclusiveResult();
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, IsConclusiveResultTest002, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    std::string httpUrl = "test";
    std::string httpsUrl = "test";
    ProbeType probeType = PROBE_HTTP;
    auto probeThread = std::make_shared<ProbeThread>(netId, BEARER_CELLULAR, netLinkInfo,
        nullptr, nullptr, probeType, httpUrl, httpsUrl);
    probeThread->httpProbe_->httpProbeResult_.responseCode_ = SUCCESS_CODE;
    auto ret = probeThread->IsConclusiveResult();
    EXPECT_FALSE(ret);

    probeThread->probeType_ = PROBE_HTTP_FALLBACK;
    ret = probeThread->IsConclusiveResult();
    EXPECT_FALSE(ret);

    probeThread->httpProbe_->httpProbeResult_.responseCode_ = PORTAL_CODE_MIN;
    ret = probeThread->IsConclusiveResult();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, IsConclusiveResultTest003, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    std::string httpUrl = "test";
    std::string httpsUrl = "test";
    ProbeType probeType = PROBE_HTTPS;
    auto probeThread = std::make_shared<ProbeThread>(netId, BEARER_CELLULAR, netLinkInfo,
        nullptr, nullptr, probeType, httpUrl, httpsUrl);
    probeThread->httpProbe_->httpsProbeResult_.responseCode_ = PORTAL_CODE_MIN;
    auto ret = probeThread->IsConclusiveResult();
    EXPECT_FALSE(ret);

    probeThread->probeType_ = PROBE_HTTPS_FALLBACK;
    ret = probeThread->IsConclusiveResult();
    EXPECT_FALSE(ret);

    probeThread->httpProbe_->httpsProbeResult_.responseCode_ = SUCCESS_CODE;
    ret = probeThread->IsConclusiveResult();
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, UpdateGlobalHttpProxyTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    std::string httpUrl = "test";
    std::string httpsUrl = "test";
    ProbeType probeType = PROBE_HTTP;
    auto probeThread = std::make_shared<ProbeThread>(netId, BEARER_CELLULAR, netLinkInfo,
        nullptr, nullptr, probeType, httpUrl, httpsUrl);
    EXPECT_NE(probeThread->httpProbe_, nullptr);
    probeThread->httpProbe_ = nullptr;
    HttpProxy httpProxy;
    probeThread->UpdateGlobalHttpProxy(httpProxy);
}

HWTEST_F(NetworkTest, CurlGlobalCleanupTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    EXPECT_NE(probe, nullptr);
    probe->useCurlCount_ = 0;
    probe->CurlGlobalCleanup();
}

HWTEST_F(NetworkTest, CleanHttpCurlTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    probe->httpCurl_ = curl_easy_init();
    EXPECT_NE(probe->httpCurl_, nullptr);
    probe->curlMulti_ = nullptr;
    probe->CleanHttpCurl();
}

HWTEST_F(NetworkTest, CleanHttpCurlTest002, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    probe->httpsCurl_ = curl_easy_init();
    EXPECT_NE(probe->httpsCurl_, nullptr);
    probe->curlMulti_ = nullptr;
    probe->CleanHttpCurl();
}

HWTEST_F(NetworkTest, ExtractDomainFormUrlTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    std::string url = "http://example.com";
    auto ret = probe->ExtractDomainFormUrl(url);
    EXPECT_EQ(ret, "example.com");
}

HWTEST_F(NetworkTest, HeaderCallbackTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    char* buffer = nullptr;
    size_t size = CURL_MAX_SIZE;
    size_t nitems = CURL_MAX_NITEMS + 1;
    void* userdata = nullptr;
    auto ret = probe->HeaderCallback(buffer, size, nitems, userdata);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetworkTest, HeaderCallbackTest002, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    char* buffer = nullptr;
    size_t size = 1;
    size_t nitems = 1;
    void* userdata = nullptr;
    auto ret = probe->HeaderCallback(buffer, size, nitems, userdata);
    EXPECT_EQ(ret, size * nitems);
}

HWTEST_F(NetworkTest, HeaderCallbackTest003, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    char* buffer = nullptr;
    size_t size = 1;
    size_t nitems = 1;
    std::string data;
    void* userdata = static_cast<void*>(&data);
    auto ret = probe->HeaderCallback(buffer, size, nitems, userdata);
    EXPECT_EQ(ret, size * nitems);

    char buf[10] = "1";
    ret = probe->HeaderCallback(buf, size, nitems, userdata);
    EXPECT_EQ(ret, size * nitems);
    EXPECT_EQ(data, "1");
}

HWTEST_F(NetworkTest, SetHttpOptionsTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTPS;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_CELLULAR, netLinkInfo, probeType);
    std::string url;
    CURL *curl = curl_easy_init();
    auto ret = probe->SetHttpOptions(probeType, curl, url);
    EXPECT_FALSE(ret);

    url = "http://example.com";
    ret = probe->SetHttpOptions(probeType, curl, url);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, SetProxyOptionTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTPS;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    std::string url = "http://";
    bool useHttpProxy = true;
    probe->globalHttpProxy_.host_ = url;
    probe->globalHttpProxy_.port_ = 1;
    EXPECT_TRUE(probe->defaultUseGlobalHttpProxy_);
    auto ret = probe->SetProxyOption(probeType, useHttpProxy);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, SetProxyOptionTest002, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    std::string url = "http://192.168.1.1";
    bool useHttpProxy = true;
    probe->globalHttpProxy_.host_ = url;
    probe->globalHttpProxy_.port_ = 1;
    EXPECT_EQ(probe->httpCurl_, nullptr);
    EXPECT_TRUE(probe->defaultUseGlobalHttpProxy_);
    auto ret = probe->SetProxyOption(probeType, useHttpProxy);
    EXPECT_FALSE(ret);

    probe->httpCurl_ = curl_easy_init();
    EXPECT_NE(probe->httpCurl_, nullptr);
    ret = probe->SetProxyOption(probeType, useHttpProxy);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, SetProxyOptionTest003, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTPS;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    std::string url = "http://192.168.1.1";
    bool useHttpProxy = true;
    probe->globalHttpProxy_.host_ = url;
    probe->globalHttpProxy_.port_ = 1;
    EXPECT_EQ(probe->httpCurl_, nullptr);
    EXPECT_TRUE(probe->defaultUseGlobalHttpProxy_);
    auto ret = probe->SetProxyOption(probeType, useHttpProxy);
    EXPECT_FALSE(ret);

    probe->httpCurl_ = curl_easy_init();
    EXPECT_NE(probe->httpCurl_, nullptr);
    ret = probe->SetProxyOption(probeType, useHttpProxy);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, SetProxyInfoTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    CURL *curlHandler = nullptr;
    std::string proxyHost = "http://192.168.1.1";
    int32_t proxyPort = 1;
    auto ret = probe->SetProxyInfo(curlHandler, proxyHost, proxyPort);
    EXPECT_FALSE(ret);

    probe->httpCurl_ = curl_easy_init();
    EXPECT_NE(probe->httpCurl_, nullptr);
    ret = probe->SetProxyInfo(curlHandler, proxyHost, proxyPort);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, SetResolveOptionTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP_HTTPS;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    std::string domain = "test";
    std::string ipAddress = "test";
    int32_t port = 1;
    auto ret = probe->SetResolveOption(probeType, domain, ipAddress, port);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, GetHeaderFieldTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    std::string key = "test";
    auto ret = probe->GetHeaderField(key);
    EXPECT_EQ(ret, "");
}

HWTEST_F(NetworkTest, GetHeaderFieldTest002, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    probe->respHeader_ = "test";
    std::string key = "1";
    auto ret = probe->GetHeaderField(key);
    EXPECT_EQ(ret, "");

    key = "t";
    ret = probe->GetHeaderField(key);
    EXPECT_NE(ret, "");
}

HWTEST_F(NetworkTest, CheckRespCodeTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    int32_t respCode = HTTP_RES_CODE_CLIENT_ERRORS_MAX + 1;
    auto ret = probe->CheckRespCode(respCode);
    EXPECT_EQ(ret, respCode);
}

HWTEST_F(NetworkTest, CheckRespCodeTest002, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTPS;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    int32_t respCode = HTTP_OK_CODE;
    auto ret = probe->CheckRespCode(respCode);
    EXPECT_EQ(ret, FAIL_CODE);

    probe->respHeader_ = "test";
    ret = probe->CheckRespCode(respCode);
    EXPECT_EQ(ret, HTTP_OK_CODE);

    probe->respHeader_ = CONTENT_LENGTH_KEY + "1\r\n";
    ret = probe->CheckRespCode(respCode);
    EXPECT_EQ(ret, FAIL_CODE);
}

HWTEST_F(NetworkTest, CheckRespCodeTest003, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTPS;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    int32_t respCode = HTTP_OK_CODE;
    probe->respHeader_ =  CONTENT_LENGTH_KEY + "123\r\n";
    probe->respHeader_ += CONNECTION_KEY + CONNECTION_CLOSE_VALUE + "\r\n";
    auto ret = probe->CheckRespCode(respCode);
    EXPECT_EQ(ret, HTTP_OK_CODE);

    probe->probeType_ = PROBE_HTTP;
    ret = probe->CheckRespCode(respCode);
    EXPECT_EQ(ret, FAIL_CODE);
}

HWTEST_F(NetworkTest, CheckClientErrorRespCodeTest001, TestSize.Level1)
{
    uint32_t netId = 1;
    NetLinkInfo netLinkInfo;
    ProbeType probeType = PROBE_HTTP;
    auto probe = std::make_shared<NetHttpProbe>(netId, BEARER_WIFI, netLinkInfo, probeType);
    int32_t respCode = HTTP_RES_CODE_BAD_REQUEST;
    auto ret = probe->CheckClientErrorRespCode(respCode);
    EXPECT_EQ(ret, HTTP_RES_CODE_BAD_REQUEST);

    strcpy_s(probe->errBuffer, CURL_ERROR_SIZE, HTML_TITLE_HTTP_EN.c_str());
    ret = probe->CheckClientErrorRespCode(respCode);
    EXPECT_EQ(ret, HTTP_RES_CODE_BAD_REQUEST);

    std::string errMsg = HTML_TITLE_HTTPS_EN + KEY_WORDS_REDIRECTION;
    strcpy_s(probe->errBuffer, CURL_ERROR_SIZE, errMsg.c_str());
    ret = probe->CheckClientErrorRespCode(respCode);
    EXPECT_EQ(ret, PORTAL_CODE);
}

HWTEST_F(NetworkTest, OH_NetConn_GetAddrInfoTest001, TestSize.Level1)
{
    char *host = nullptr;
    char *serv = nullptr;
    struct addrinfo *hint = nullptr;
    struct addrinfo **res = nullptr;
    int32_t netId = 1;
    auto ret = OH_NetConn_GetAddrInfo(host, serv, hint, res, netId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    char host1[] = "192.168.1.1";
    ret = OH_NetConn_GetAddrInfo(host1, serv, hint, res, netId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_GetAddrInfoTest002, TestSize.Level1)
{
    char host[] = "";
    char *serv = nullptr;
    struct addrinfo info;
    struct addrinfo *hint = &info;
    struct addrinfo **res = &hint;
    int32_t netId = 1;
    auto ret = OH_NetConn_GetAddrInfo(host, serv, hint, res, netId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    char host1[] = "192.168.1.1";
    ret = OH_NetConn_GetAddrInfo(host1, serv, hint, res, netId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    netId = -1;
    OH_NetConn_GetAddrInfo(host, serv, hint, res, netId);

    netId = VALID_NETID_START;
    OH_NetConn_GetAddrInfo(host, serv, hint, res, netId);
}

HWTEST_F(NetworkTest, OH_NetConn_FreeDnsResultTest001, TestSize.Level1)
{
    struct addrinfo *res = nullptr;
    auto ret = OH_NetConn_FreeDnsResult(res);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_GetAllNetsTest001, TestSize.Level1)
{
    auto ret = OH_NetConn_GetAllNets(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    NetConn_NetHandleList netHandleList;
    OH_NetConn_GetAllNets(&netHandleList);
}

HWTEST_F(NetworkTest, OH_NetConn_HasDefaultNetTest001, TestSize.Level1)
{
    auto ret = OH_NetConn_HasDefaultNet(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    int32_t hasDefaultNet;
    OH_NetConn_HasDefaultNet(&hasDefaultNet);
}

HWTEST_F(NetworkTest, OH_NetConn_GetDefaultNetTest001, TestSize.Level1)
{
    auto ret = OH_NetConn_GetDefaultNet(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    NetConn_NetHandle netHandle;
    OH_NetConn_GetDefaultNet(&netHandle);
}

HWTEST_F(NetworkTest, OH_NetConn_IsDefaultNetMeteredTest001, TestSize.Level1)
{
    auto ret = OH_NetConn_IsDefaultNetMetered(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    int32_t isMetered;
    OH_NetConn_IsDefaultNetMetered(&isMetered);
}

HWTEST_F(NetworkTest, OH_NetConn_GetConnectionPropertiesTest001, TestSize.Level1)
{
    auto ret = OH_NetConn_GetConnectionProperties(nullptr, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    NetConn_NetHandle netHandle; NetConn_ConnectionProperties prop;
    ret = OH_NetConn_GetConnectionProperties(&netHandle, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    OH_NetConn_GetConnectionProperties(&netHandle, &prop);
}

HWTEST_F(NetworkTest, OH_NetConn_GetNetCapabilitiesTest001, TestSize.Level1)
{
    auto ret = OH_NetConn_GetNetCapabilities(nullptr, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    NetConn_NetHandle netHandle;
    NetConn_NetCapabilities netAllCapabilities;
    ret = OH_NetConn_GetNetCapabilities(&netHandle, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    OH_NetConn_GetNetCapabilities(&netHandle, &netAllCapabilities);
}

HWTEST_F(NetworkTest, OH_NetConn_GetDefaultHttpProxyTest001, TestSize.Level1)
{
    auto ret = OH_NetConn_GetDefaultHttpProxy(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    NetConn_HttpProxy httpProxy;
    OH_NetConn_GetDefaultHttpProxy(&httpProxy);
}

HWTEST_F(NetworkTest, OHOS_NetConn_RegisterDnsResolverTest001, TestSize.Level1)
{
    OH_NetConn_CustomDnsResolver resolver = nullptr;
    auto ret = OHOS_NetConn_RegisterDnsResolver(resolver);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    resolver = [](const char *host, const char *serv,
        const struct addrinfo *hint, struct addrinfo **res) -> int {
            return NETMANAGER_ERR_PARAMETER_ERROR;
    };
    ret = OHOS_NetConn_RegisterDnsResolver(resolver);
    EXPECT_NE(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_RegisterDnsResolverTest001, TestSize.Level1)
{
    OH_NetConn_CustomDnsResolver resolver = nullptr;
    auto ret = OH_NetConn_RegisterDnsResolver(resolver);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    resolver = [](const char *host, const char *serv,
        const struct addrinfo *hint, struct addrinfo **res) -> int {
            return NETMANAGER_ERR_PARAMETER_ERROR;
    };
    ret = OH_NetConn_RegisterDnsResolver(resolver);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_BindSocketTest001, TestSize.Level1)
{
    int32_t socketFd = -1;
    NetConn_NetHandle netHandle;
    auto ret = OH_NetConn_BindSocket(socketFd, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_BindSocket(socketFd, &netHandle);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_BindSocketTest002, TestSize.Level1)
{
    int32_t socketFd = 1;
    NetConn_NetHandle netHandle = {1};
    auto ret = OH_NetConn_BindSocket(socketFd, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    netHandle.netId = VALID_NETID_START;
    ret = OH_NetConn_BindSocket(socketFd, &netHandle);
    EXPECT_NE(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_RegisterNetConnCallbackTest001, TestSize.Level1)
{
    NetConn_NetSpecifier specifier;
    NetConn_NetConnCallback netConnCallback;
    uint32_t timeout = 1;
    uint32_t callbackId = 1;
    auto ret = OH_NetConn_RegisterNetConnCallback(nullptr, nullptr, timeout, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_RegisterNetConnCallback(&specifier, nullptr, timeout, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_RegisterNetConnCallbackTest002, TestSize.Level1)
{
    NetConn_NetSpecifier specifier;
    NetConn_NetConnCallback netConnCallback;
    uint32_t timeout = 1;
    uint32_t callbackId = 1;
    auto ret = OH_NetConn_RegisterNetConnCallback(&specifier, &netConnCallback, timeout, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_RegisterNetConnCallback(&specifier, &netConnCallback, timeout, &callbackId);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_RegisterDefaultNetConnCallbackTest001, TestSize.Level1)
{
    NetConn_NetConnCallback netConnCallback;
    uint32_t callbackId = 1;
    auto ret = OH_NetConn_RegisterDefaultNetConnCallback(nullptr, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_RegisterDefaultNetConnCallback(&netConnCallback, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_RegisterDefaultNetConnCallback(&netConnCallback, &callbackId);
    EXPECT_NE(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_SetAppHttpProxyTest001, TestSize.Level1)
{
    NetConn_HttpProxy httpProxy;
    auto ret = OH_NetConn_SetAppHttpProxy(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_SetAppHttpProxy(&httpProxy);
    EXPECT_NE(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_RegisterAppHttpProxyCallbackTest001, TestSize.Level1)
{
    OH_NetConn_AppHttpProxyChange appHttpProxyChange = nullptr;
    uint32_t callbackId = 1;
    auto ret = OH_NetConn_RegisterAppHttpProxyCallback(appHttpProxyChange, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    appHttpProxyChange = [](NetConn_HttpProxy *proxy) {};
    ret = OH_NetConn_RegisterAppHttpProxyCallback(appHttpProxyChange, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_RegisterAppHttpProxyCallback(appHttpProxyChange, &callbackId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetworkTest, OH_NetConn_SetPacUrlTest001, TestSize.Level1)
{
    const char *pacUrl = "test";
    auto ret = OH_NetConn_SetPacUrl(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_SetPacUrl(pacUrl);
    EXPECT_NE(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_GetPacUrlTest001, TestSize.Level1)
{
    char pacUrl[PAC_URL_MAX_LEN] = {0};
    auto ret = OH_NetConn_GetPacUrl(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    ret = OH_NetConn_GetPacUrl(pacUrl);
    EXPECT_NE(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryProbeResult001, TestSize.Level1)
{
    struct NetConn_ProbeResultInfo result;
    auto ret = OH_NetConn_QueryProbeResult("www.baidu.com", 10, &result);
    EXPECT_EQ(ret, 0);

    ret = OH_NetConn_QueryProbeResult(nullptr, 10, &result);
    EXPECT_NE(ret, 0);

    ret = OH_NetConn_QueryProbeResult("www.baidu.com", 0, &result);
    EXPECT_NE(ret, 0);

    ret = OH_NetConn_QueryProbeResult("www.baidu.com", -1, &result);
    EXPECT_NE(ret, 0);

    ret = OH_NetConn_QueryProbeResult("www.baidu.com", 1001, &result);
    EXPECT_NE(ret, 0);

    ret = OH_NetConn_QueryProbeResult("8.8.8.8", 10, &result);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetworkTest, OH_NetConn_GetAddrInfoTest003, TestSize.Level1)
{
    char host[] = "192.168.1.1";
    char *serv = nullptr;
    struct addrinfo info;
    struct addrinfo *hint = &info;
    struct addrinfo **res = &hint;
    int32_t netId = -1;
    auto ret = OH_NetConn_GetAddrInfo(host, serv, hint, res, netId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest001, TestSize.Level1)
{
    const char *destination = "www.example.com";
    NetConn_TraceRouteInfo traceRouteInfo[1] = {};
    auto ret = OH_NetConn_QueryTraceRoute(destination, nullptr, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
    ret = OH_NetConn_QueryTraceRoute(nullptr, nullptr, traceRouteInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest002, TestSize.Level1)
{
    std::string traceRouteInfoStr = "1 192.168.1.1 50 2 192.168.1.2 100 3 192.168.1.3 150";
    NetConn_TraceRouteInfo traceRouteInfo[3];
    int32_t maxJumpNumber = 3;

    EXPECT_EQ(Conv2TraceRouteInfo(traceRouteInfoStr, traceRouteInfo, maxJumpNumber), NETMANAGER_SUCCESS);
    EXPECT_EQ(traceRouteInfo[0].jumpNo, 1);
    EXPECT_STREQ(traceRouteInfo[0].address, "192.168.1.1");
    EXPECT_EQ(traceRouteInfo[0].rtt[0], 50);
    EXPECT_EQ(traceRouteInfo[1].jumpNo, 2);
    EXPECT_STREQ(traceRouteInfo[1].address, "192.168.1.2");
    EXPECT_EQ(traceRouteInfo[1].rtt[0], 100);
    EXPECT_EQ(traceRouteInfo[2].jumpNo, 3);
    EXPECT_STREQ(traceRouteInfo[2].address, "192.168.1.3");
    EXPECT_EQ(traceRouteInfo[2].rtt[0], 150);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest003, TestSize.Level1)
{
    std::string traceRouteInfoStr = "1 192.168.1.1 50 2 192.168.1.2 100 3 192.168.1.3 150";
    NetConn_TraceRouteInfo traceRouteInfo[2];
    int32_t maxJumpNumber = 2;

    EXPECT_EQ(Conv2TraceRouteInfo(traceRouteInfoStr, traceRouteInfo, maxJumpNumber), NETMANAGER_SUCCESS);
    EXPECT_EQ(traceRouteInfo[0].jumpNo, 1);
    EXPECT_STREQ(traceRouteInfo[0].address, "192.168.1.1");
    EXPECT_EQ(traceRouteInfo[0].rtt[0], 50);
    EXPECT_EQ(traceRouteInfo[1].jumpNo, 2);
    EXPECT_STREQ(traceRouteInfo[1].address, "192.168.1.2");
    EXPECT_EQ(traceRouteInfo[1].rtt[0], 100);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest004, TestSize.Level1)
{
    std::string traceRouteInfoStr = "1 192.168.1.1 50 2 192.168.1.2 100 3 192.168.1.3 150";
    int32_t maxJumpNumber = 3;

    EXPECT_EQ(Conv2TraceRouteInfo(traceRouteInfoStr, nullptr, maxJumpNumber), NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest005, TestSize.Level1)
{
    std::string traceRouteInfoStr = "1 192.168.1.1 50 2 192.168.1.2 invalid 3 192.168.1.3 150";
    NetConn_TraceRouteInfo traceRouteInfo[3];
    int32_t maxJumpNumber = 3;

    EXPECT_EQ(Conv2TraceRouteInfo(traceRouteInfoStr, traceRouteInfo, maxJumpNumber), NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest006, TestSize.Level1)
{
    std::string rttStr = "100;200;300";
    uint32_t rtt[NETCONN_MAX_RTT_NUM] = {0};

    int32_t result = Conv2TraceRouteInfoRtt(rttStr, &rtt);

    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    EXPECT_EQ(rtt[0], 100);
    EXPECT_EQ(rtt[1], 200);
    EXPECT_EQ(rtt[2], 300);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest007, TestSize.Level1)
{
    std::string rttStr;
    for (int i = 0; i < NETCONN_MAX_RTT_NUM; ++i) {
        rttStr += std::to_string(i) + ";";
    }
    uint32_t rtt[NETCONN_MAX_RTT_NUM] = {0};

    int32_t result = Conv2TraceRouteInfoRtt(rttStr, &rtt);

    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    EXPECT_EQ(rtt[0], 0);
    EXPECT_EQ(rtt[1], 1);
    EXPECT_EQ(rtt[2], 2);
    EXPECT_EQ(rtt[3], 3);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest008, TestSize.Level1)
{
    std::string rttStr = "100;abc;300";
    uint32_t rtt[NETCONN_MAX_RTT_NUM] = {0};

    int32_t result = Conv2TraceRouteInfoRtt(rttStr, &rtt);

    EXPECT_EQ(result, NETMANAGER_SUCCESS);
    EXPECT_EQ(rtt[0], 100);
    EXPECT_EQ(rtt[1], 0);
    EXPECT_EQ(rtt[2], 300);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest09, TestSize.Level1)
{
    const std::string traceRouteInfoStr = "1 192.168.2.1 788;889;998;110 2 10.111.120.189 1334;1445;1667;1678";
    NetConn_TraceRouteInfo traceRouteInfo[2] = {};
    auto ret = Conv2TraceRouteInfo(traceRouteInfoStr, traceRouteInfo, 2);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(1678, traceRouteInfo[1].rtt[3]);
}

HWTEST_F(NetworkTest, OH_NetConn_QueryTraceRouteTest10, TestSize.Level1)
{
    const char *destination = "www.text.com";
    NetConn_TraceRouteInfo traceRouteInfo[30] = {};
    NetConn_TraceRouteOption Option = {30, NETCONN_PACKETS_ICMP};
    OH_NetConn_QueryTraceRoute(destination, &Option, traceRouteInfo);
    Option = {31, NETCONN_PACKETS_ICMP};
    auto ret = OH_NetConn_QueryTraceRoute(destination, &Option, traceRouteInfo);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetworkTest, OH_NetConn_BindSocketTest003, TestSize.Level1)
{
    int32_t socketFd = 1;
    NetConn_NetHandle netHandle = {1};
    auto ret = OH_NetConn_BindSocket(socketFd, nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);

    netHandle.netId = 99;
    ret = OH_NetConn_BindSocket(socketFd, &netHandle);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
}

} // namespace NetManagerStandard
} // namespace OHOS