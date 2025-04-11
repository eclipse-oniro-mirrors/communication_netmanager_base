/*
Copyright (c) 2022-2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
network->nat464Service_ = std::make_unique(netId, "ifaceName");
network->UpdateBasicNetwork(false);
}

HWTEST_F(NetworkTest, ReleaseVirtualNetworkTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
EXPECT_FALSE(network->isVirtualCreated_);
auto ret = network->ReleaseVirtualNetwork();
EXPECT_TRUE(ret);
}

HWTEST_F(NetworkTest, ReleaseVirtualNetworkTest002, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
EXPECT_EQ(network->netSupplierType_, NetBearType::BEARER_VPN);
network->GetNetLinkInfo();
}

HWTEST_F(NetworkTest, GetNetLinkInfoTest002, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
network->netLinkInfo_.ifaceName_ = "test";
NetLinkInfo newNetLinkInfo;
EXPECT_TRUE(newNetLinkInfo.ifaceName_.empty());
network->UpdateInterfaces(newNetLinkInfo);
EXPECT_TRUE(network->netLinkInfo_.ifaceName_.empty());
}

HWTEST_F(NetworkTest, UpdateIpAddrsTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_VPN, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
NetLinkInfo netLinkInfo;
EXPECT_TRUE(netLinkInfo.dnsList_.empty());
network->UpdateDns(netLinkInfo);
}

HWTEST_F(NetworkTest, UpdateTcpBufferSizeTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
NetLinkInfo netLinkInfo;
netLinkInfo.tcpBufferSizes_ = "4096";
EXPECT_NE(netLinkInfo.tcpBufferSizes_, network->netLinkInfo_.tcpBufferSizes_);
network->UpdateTcpBufferSize(netLinkInfo);
}

HWTEST_F(NetworkTest, NetDetectionForDnsHealthTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
network->InitNetMonitor();
EXPECT_NE(network->netMonitor_, nullptr);
network->detectResult_ = INVALID_DETECTION_STATE;
network->NetDetectionForDnsHealth(false);
}

HWTEST_F(NetworkTest, HandleNetMonitorResultTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
NetDetectionStatus state = UNKNOWN_STATE;
std::string urlRedirect = "test";
network->netCallback_ = nullptr;
network->HandleNetMonitorResult(state, urlRedirect);
}

HWTEST_F(NetworkTest, HandleNetMonitorResultTest003, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
NetDetectionResultCode detectionResult = NET_DETECTION_FAIL;
std::string urlRedirect = "test";
network->netDetectionRetCallback_.push_back(nullptr);
network->NotifyNetDetectionResult(detectionResult, urlRedirect);
}

HWTEST_F(NetworkTest, NetDetectionResultConvertTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
int32_t internalRet = static_cast<int32_t>(INVALID_DETECTION_STATE);
auto ret = network->NetDetectionResultConvert(internalRet);
EXPECT_EQ(ret, NET_DETECTION_FAIL);
}

HWTEST_F(NetworkTest, UpdateNetConnStateTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
network->netLinkInfo_.ifaceName_ = "test";
network->state_ = NET_CONN_STATE_CONNECTED;
network->nat464Service_ = std::make_unique(netId, "test");
EXPECT_TRUE(network->netLinkInfo_.netAddrList_.empty());
NetConnState netConnState = NET_CONN_STATE_IDLE;
network->UpdateNetConnState(netConnState);
}

HWTEST_F(NetworkTest, IsNat464PreferedTest001, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_NE(network, nullptr);
network->netLinkInfo_.ifaceName_ = "test";
EXPECT_TRUE(network->netLinkInfo_.netAddrList_.empty());
auto ret = network->IsNat464Prefered();
EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, IsNat464PreferedTest002, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
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
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
EXPECT_EQ(network->netMonitor_, nullptr);
network->SetScreenState(false);
}

HWTEST_F(NetworkTest, SetScreenStateTest002, TestSize.Level1)
{
int32_t netId = 1;
auto network = std::make_shared(netId, netId, nullptr, NetBearType::BEARER_ETHERNET, nullptr);
network->InitNetMonitor();
EXPECT_NE(network->netMonitor_, nullptr);
network->SetScreenState(false);
}

HWTEST_F(NetworkTest, MaybeUpdateV6IfaceTest001, TestSize.Level1)
{
int32_t netId = 1;
std::string v6Iface;
auto service = std::make_shared(netId, v6Iface);
EXPECT_NE(service, nullptr);
service->MaybeUpdateV6Iface(v6Iface);

service->serviceState_ = NAT464_SERVICE_STATE_DISCOVERING;
service->MaybeUpdateV6Iface(v6Iface);
}

HWTEST_F(NetworkTest, UpdateServiceStateTest001, TestSize.Level1)
{
int32_t netId = 1;
std::string v6Iface;
auto service = std::make_shared(netId, v6Iface);
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
auto service = std::make_shared(netId, v6Iface);
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
auto service = std::make_shared(netId, v6Iface);
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
auto service = std::make_shared(netId, v6Iface);
EXPECT_NE(service, nullptr);
service->tryStopDiscovery_ = true;
service->DiscoverPrefix();
EXPECT_FALSE(service->tryStopDiscovery_);
}

HWTEST_F(NetworkTest, DiscoverPrefixTest002, TestSize.Level1)
{
int32_t netId = 1;
std::string v6Iface;
auto service = std::make_shared(netId, v6Iface);
EXPECT_NE(service, nullptr);
EXPECT_FALSE(service->tryStopDiscovery_);
EXPECT_FALSE(service->GetPrefixFromDns64());
service->DiscoverPrefix();
}

HWTEST_F(NetworkTest, GetPrefixFromDns64Test001, TestSize.Level1)
{
int32_t netId = 1;
std::string v6Iface;
auto service = std::make_shared(netId, v6Iface);
EXPECT_NE(service, nullptr);
auto ret = service->GetPrefixFromDns64();
EXPECT_FALSE(ret);
}

HWTEST_F(NetworkTest, StartServiceTest001, TestSize.Level1)
{
int32_t netId = 1;
std::string v6Iface;
auto service = std::make_shared(netId, v6Iface);
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
auto latch = std::make_shared(0);
auto probeThread = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo,
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
auto probeThread = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo,
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
auto probeThread = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo,
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
auto probeThread = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo,
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
auto probeThread = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo,
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
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
EXPECT_NE(probe, nullptr);
probe->useCurlCount_ = 0;
probe->CurlGlobalCleanup();
}

HWTEST_F(NetworkTest, CleanHttpCurlTest001, TestSize.Level1)
{
uint32_t netId = 1;
NetLinkInfo netLinkInfo;
ProbeType probeType = PROBE_HTTP;
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
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
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
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
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
std::string url = "http://example.com";
auto ret = probe->ExtractDomainFormUrl(url);
EXPECT_EQ(ret, "example.com");
}

HWTEST_F(NetworkTest, HeaderCallbackTest001, TestSize.Level1)
{
uint32_t netId = 1;
NetLinkInfo netLinkInfo;
ProbeType probeType = PROBE_HTTP;
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
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
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
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
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
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
auto probe = std::make_shared(netId, BEARER_CELLULAR, netLinkInfo, probeType);
std::string url;
CURL *curl = curl_easy_init();
auto ret = probe->SetHttpOptions(probeType, curl, url);
EXPECT_FALSE(ret);

url = "http://example.com";
ret = probe->SetHttpOptions(probeType, curl, url);
EXPECT_FALSE(ret);
}

} // namespace NetManagerStandard
} // namespace OHOS