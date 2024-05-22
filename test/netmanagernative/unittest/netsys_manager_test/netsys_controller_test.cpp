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

#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "netmanager_base_test_security.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "bpf_def.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "common_net_diag_callback_test.h"
#include "common_netsys_controller_callback_test.h"
#include "net_conn_constants.h"
#include "net_diag_callback_stub.h"
#include "netnative_log_wrapper.h"
#include "netsys_controller.h"
#include "netsys_ipc_interface_code.h"
#include "netsys_net_diag_data.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *IFACE = "test0";
static constexpr const char *WLAN = "wlan0";
static constexpr const char *ETH0 = "eth0";
static constexpr const char *DESTINATION = "192.168.1.3/24";
static constexpr const char *NEXT_HOP = "192.168.1.1";
static constexpr const char *PARCEL_IPV4_ADDR = "192.168.55.121";
static constexpr const char *IP_ADDR = "172.17.5.245";
static constexpr const char *INTERFACE_NAME = "";
static constexpr const char *IF_NAME = "iface0";
static constexpr const char *TCP_BUFFER_SIZES = "524288,1048576,2097152,262144,524288,1048576";
static constexpr uint64_t TEST_COOKIE = 1;
static constexpr uint32_t TEST_STATS_TYPE1 = 0;
static constexpr uint32_t TEST_STATS_TYPE2 = 2;
static constexpr uint32_t IPC_ERR_FLATTEN_OBJECT = 3;
const int NET_ID = 2;
const int PERMISSION = 5;
const int PREFIX_LENGTH = 23;
const int TEST_MTU = 111;
uint16_t g_baseTimeoutMsec = 200;
uint8_t g_retryCount = 3;
const int64_t TEST_UID = 1010;
const int32_t SOCKET_FD = 5;
const int32_t TEST_STATS_UID = 11111;
int g_ifaceFd = 5;
const int64_t BYTES = 2097152;
const uint32_t FIREWALL_RULE = 1;
bool g_isWaitAsync = false;
const int32_t ERR_INVALID_DATA = 5;
} // namespace

class NetsysControllerTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();

    static inline std::shared_ptr<NetsysController> instance_ = nullptr;

    sptr<NetsysNative::NetDiagCallbackStubTest> netDiagCallback = new NetsysNative::NetDiagCallbackStubTest();
};

void NetsysControllerTest::SetUpTestCase()
{
    instance_ = std::make_shared<NetsysController>();
}

void NetsysControllerTest::TearDownTestCase() {}

void NetsysControllerTest::SetUp() {}

void NetsysControllerTest::TearDown() {}

HWTEST_F(NetsysControllerTest, NetsysControllerTest001, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().NetworkCreatePhysical(NET_ID, PERMISSION);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().NetworkDestroy(NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest002, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().NetworkAddInterface(NET_ID, WLAN);
    EXPECT_EQ(ret, -1);

    ret = NetsysController::GetInstance().NetworkRemoveInterface(NET_ID, WLAN);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest003, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().NetworkAddRoute(NET_ID, ETH0, DESTINATION, NEXT_HOP);
    EXPECT_LE(ret, 0);

    ret = NetsysController::GetInstance().NetworkRemoveRoute(NET_ID, ETH0, DESTINATION, NEXT_HOP);
    EXPECT_LE(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest004, TestSize.Level1)
{
    OHOS::nmd::InterfaceConfigurationParcel parcel;
    parcel.ifName = ETH0;
    parcel.ipv4Addr = PARCEL_IPV4_ADDR;
    int32_t ret = NetsysController::GetInstance().SetInterfaceConfig(parcel);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().GetInterfaceConfig(parcel);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest005, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().SetInterfaceDown(ETH0);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().SetInterfaceUp(ETH0);
    EXPECT_EQ(ret, 0);

    NetsysController::GetInstance().ClearInterfaceAddrs(ETH0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest006, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().SetInterfaceMtu(ETH0, TEST_MTU);
    EXPECT_EQ(ret, -1);

    ret = NetsysController::GetInstance().GetInterfaceMtu(ETH0);
    EXPECT_EQ(ret, -1);

    ret = NetsysController::GetInstance().SetTcpBufferSizes(TCP_BUFFER_SIZES);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest007, TestSize.Level1)
{
    auto ifaceList = NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), std::string(ETH0)) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }

    int32_t ret = NetsysController::GetInstance().AddInterfaceAddress(ETH0, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().DelInterfaceAddress(ETH0, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest008, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().SetResolverConfig(NET_ID, g_baseTimeoutMsec, g_retryCount, {}, {});
    EXPECT_EQ(ret, 0);

    std::vector<std::string> servers;
    std::vector<std::string> domains;
    ret = NetsysController::GetInstance().GetResolverConfig(NET_ID, servers, domains, g_baseTimeoutMsec, g_retryCount);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest009, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().CreateNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().DestroyNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest010, TestSize.Level1)
{
    nmd::NetworkSharingTraffic traffic;
    int32_t ret = NetsysController::GetInstance().GetNetworkSharingTraffic(ETH0, ETH0, traffic);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest011, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().GetCellularRxBytes();
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().GetCellularTxBytes();
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().GetAllRxBytes();
    EXPECT_GE(ret, 0);

    ret = NetsysController::GetInstance().GetAllTxBytes();
    EXPECT_GE(ret, 0);

    ret = NetsysController::GetInstance().GetUidRxBytes(TEST_UID);
    EXPECT_EQ(ret, -1);

    ret = NetsysController::GetInstance().GetUidTxBytes(TEST_UID);
    EXPECT_EQ(ret, -1);

    ret = NetsysController::GetInstance().GetUidOnIfaceRxBytes(TEST_UID, INTERFACE_NAME);
    EXPECT_GE(ret, 0);

    ret = NetsysController::GetInstance().GetUidOnIfaceTxBytes(TEST_UID, INTERFACE_NAME);
    EXPECT_GE(ret, 0);

    ret = NetsysController::GetInstance().GetIfaceRxBytes(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().GetIfaceTxBytes(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest012, TestSize.Level1)
{
    std::vector<std::string> getList = NetsysController::GetInstance().InterfaceGetList();

    getList.clear();
    getList = NetsysController::GetInstance().UidGetList();
    EXPECT_EQ(getList.size(), 0);

    int64_t ret = NetsysController::GetInstance().GetIfaceRxPackets(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().GetIfaceTxPackets(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().SetDefaultNetWork(NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().ClearDefaultNetWorkNetId();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest013, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().BindSocket(SOCKET_FD, NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().IpEnableForwarding(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().IpDisableForwarding(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().EnableNat(ETH0, ETH0);
    EXPECT_NE(ret, 0);

    ret = NetsysController::GetInstance().DisableNat(ETH0, ETH0);
    EXPECT_NE(ret, 0);

    ret = NetsysController::GetInstance().IpfwdAddInterfaceForward(ETH0, ETH0);
    EXPECT_NE(ret, 0);

    ret = NetsysController::GetInstance().IpfwdRemoveInterfaceForward(ETH0, ETH0);
    EXPECT_NE(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest014, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().ShareDnsSet(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StartDnsProxyListen();
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StopDnsProxyListen();
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().BindNetworkServiceVpn(SOCKET_FD);
    EXPECT_EQ(ret, 0);

    ifreq ifRequest;
    ret = NetsysController::GetInstance().EnableVirtualNetIfaceCard(SOCKET_FD, ifRequest, g_ifaceFd);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().SetIpAddress(SOCKET_FD, IP_ADDR, PREFIX_LENGTH, ifRequest);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().SetBlocking(g_ifaceFd, true);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().SetBlocking(g_ifaceFd, false);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StartDhcpClient(INTERFACE_NAME, true);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StartDhcpClient(INTERFACE_NAME, false);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StopDhcpClient(INTERFACE_NAME, true);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StopDhcpClient(INTERFACE_NAME, false);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StartDhcpService(INTERFACE_NAME, IP_ADDR);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().StopDhcpService(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest015, TestSize.Level1)
{
    NetsysController::GetInstance().BandwidthEnableDataSaver(false);
    int32_t ret = NetsysController::GetInstance().BandwidthEnableDataSaver(true);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthSetIfaceQuota(IF_NAME, BYTES);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthSetIfaceQuota(WLAN, BYTES);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthRemoveIfaceQuota(IF_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthRemoveIfaceQuota(WLAN);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthAddDeniedList(TEST_UID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthAddAllowedList(TEST_UID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthRemoveDeniedList(TEST_UID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().BandwidthRemoveAllowedList(TEST_UID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    std::vector<uint32_t> uids;
    uids.push_back(TEST_UID);
    ret = NetsysController::GetInstance().FirewallSetUidsAllowedListChain(TEST_UID, uids);
    EXPECT_NE(ret, 0);
    ret = NetsysController::GetInstance().FirewallSetUidsDeniedListChain(TEST_UID, uids);
    EXPECT_NE(ret, 0);

    ret = NetsysController::GetInstance().FirewallEnableChain(TEST_UID, true);
    EXPECT_NE(ret, 0);

    ret = NetsysController::GetInstance().FirewallSetUidRule(TEST_UID, {TEST_UID}, FIREWALL_RULE);
    EXPECT_NE(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest016, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().InterfaceSetIpAddress("ifaceName", "192.168.x.x");
    EXPECT_NE(ret, 0);

    ret = NetsysController::GetInstance().InterfaceSetIpAddress("ifaceName", "192.168.2.0");
    EXPECT_EQ(ret, -1);

    ret = NetsysController::GetInstance().InterfaceSetIffUp("");
    EXPECT_NE(ret, 0);

    ret = NetsysController::GetInstance().InterfaceSetIffUp("ifaceName");
    EXPECT_EQ(ret, -1);

    std::string hostName = "";
    std::string serverName = "";
    AddrInfo hints;
    uint16_t netId = 0;
    std::vector<AddrInfo> res;

    ret = NetsysController::GetInstance().GetAddrInfo(hostName, serverName, hints, netId, res);
    EXPECT_NE(ret, 0);

    auto callback = new NetsysControllerCallbackTestCb();
    ret = NetsysController::GetInstance().RegisterCallback(callback);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest017, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = NetsysController::GetInstance().GetTotalStats(stats, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    stats = 0;
    ret = NetsysController::GetInstance().GetUidStats(stats, 0, TEST_STATS_UID);
    EXPECT_EQ(ret, NetStatsResultCode::STATS_ERR_READ_BPF_FAIL);

    stats = 0;
    ret = NetsysController::GetInstance().GetIfaceStats(stats, 0, IFACE);
    EXPECT_EQ(ret, NetStatsResultCode::STATS_ERR_GET_IFACE_NAME_FAILED);

    stats = 0;
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> statsInfo;
    ret = NetsysController::GetInstance().GetAllStatsInfo(statsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().GetAllContainerStatsInfo(statsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest018, TestSize.Level1)
{
    std::string respond;
    int32_t ret = NetsysController::GetInstance().SetIptablesCommandForRes("-L", respond);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED);

    NetManagerBaseAccessToken token;
    ret = NetsysController::GetInstance().SetIptablesCommandForRes("abc", respond);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED);

    ret = NetsysController::GetInstance().SetIptablesCommandForRes("-L", respond);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetsysControllerTest, NetsysControllerErr001, TestSize.Level1)
{
    std::vector<int32_t> beginUids;
    std::vector<int32_t> endUids;
    std::string iface = "test";
    OHOS::nmd::InterfaceConfigurationParcel Parcel;

    int32_t ret = instance_->SetInternetPermission(0, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkCreateVirtual(0, false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkDestroy(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkAddUids(0, beginUids, endUids);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkDelUids(0, beginUids, endUids);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkAddInterface(0, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkRemoveInterface(0, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkAddRoute(0, iface, iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkRemoveRoute(0, iface, iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetInterfaceConfig(Parcel);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetInterfaceConfig(Parcel);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetInterfaceDown(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetInterfaceUp(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    instance_->ClearInterfaceAddrs(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetInterfaceMtu(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetInterfaceMtu(iface, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerErr002, TestSize.Level1)
{
    std::string iface = "test";
    std::vector<std::string> servers;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    AddrInfo hints = {0};
    std::vector<AddrInfo> res;
    nmd::NetworkSharingTraffic traffic;
    addrinfo *aihead = static_cast<addrinfo *>(malloc(sizeof(addrinfo)));
    if (aihead != nullptr) {
        aihead->ai_next = nullptr;
        aihead->ai_addr = static_cast<sockaddr *>(malloc(sizeof(sockaddr)));
    }
    if (aihead != nullptr) {
        aihead->ai_canonname = static_cast<char *>(malloc(10));
    }

    int32_t ret = instance_->AddInterfaceAddress(iface, iface, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->DelInterfaceAddress(iface, iface, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->InterfaceSetIpAddress(iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->InterfaceSetIffUp(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetResolverConfig(0, 0, 0, servers, servers);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetResolverConfig(0, servers, servers, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->CreateNetworkCache(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->DestroyNetworkCache(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    instance_->FreeAddrInfo(aihead);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetAddrInfo(iface, iface, hints, 0, res);
    EXPECT_EQ(ret, NetManagerStandard::NetConnResultCode::NET_CONN_ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL);

    ret = instance_->GetNetworkSharingTraffic(iface, iface, traffic);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerErr003, TestSize.Level1)
{
    std::string iface = "test";

    auto ret = instance_->GetCellularRxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetCellularTxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetAllRxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetAllTxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetUidRxBytes(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetUidTxBytes(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetUidOnIfaceRxBytes(0, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetUidOnIfaceTxBytes(0, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetIfaceRxBytes(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetIfaceTxBytes(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetIfaceRxPackets(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetIfaceTxPackets(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerErr004, TestSize.Level1)
{
    std::string iface = "test";
    NetsysNotifyCallback callback;

    auto faceList = instance_->InterfaceGetList();
    EXPECT_EQ(faceList.size(), 0);

    auto uidList = instance_->UidGetList();
    EXPECT_EQ(uidList.size(), 0);

    auto ret = instance_->SetDefaultNetWork(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->ClearDefaultNetWorkNetId();
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BindSocket(0, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->IpEnableForwarding(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->IpDisableForwarding(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->EnableNat(iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->DisableNat(iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->IpfwdAddInterfaceForward(iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->IpfwdRemoveInterfaceForward(iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->ShareDnsSet(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->StartDnsProxyListen();
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->StopDnsProxyListen();
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->RegisterNetsysNotifyCallback(callback);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerErr005, TestSize.Level1)
{
    std::string iface = "test";
    struct ifreq ifRequest;
    int32_t ifaceFd = 0;
    sptr<NetsysControllerCallback> callback;
    auto ret = instance_->BindNetworkServiceVpn(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_ERR_VPN);

    ret = instance_->BindNetworkServiceVpn(1);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->EnableVirtualNetIfaceCard(0, ifRequest, ifaceFd);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_ERR_VPN);

    ret = instance_->EnableVirtualNetIfaceCard(1, ifRequest, ifaceFd);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetIpAddress(0, iface, 0, ifRequest);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_ERR_VPN);

    ret = instance_->SetIpAddress(1, iface, 1, ifRequest);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetBlocking(0, false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->StartDhcpClient(iface, false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->StopDhcpClient(iface, false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->StartDhcpService(iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->StopDhcpService(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BandwidthEnableDataSaver(false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BandwidthSetIfaceQuota(iface, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BandwidthRemoveIfaceQuota(iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BandwidthAddDeniedList(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BandwidthRemoveDeniedList(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BandwidthAddAllowedList(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->BandwidthRemoveAllowedList(0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerErr006, TestSize.Level1)
{
    std::string iface = "test";
    std::vector<uint32_t> uids;
    uint64_t stats = 0;
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> statsInfo;

    auto ret = instance_->FirewallSetUidsAllowedListChain(0, uids);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->FirewallSetUidsDeniedListChain(0, uids);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->FirewallEnableChain(0, false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->FirewallSetUidRule(0, uids, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetTotalStats(stats, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetUidStats(stats, 0, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetIfaceStats(stats, 0, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->GetAllStatsInfo(statsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetIptablesCommandForRes(iface, iface);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->SetTcpBufferSizes("");
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetDiagGetRouteTable001, TestSize.Level1)
{
    std::list<OHOS::NetsysNative::NetDiagRouteTable> diagrouteTable;
    auto ret = NetsysController::GetInstance().NetDiagGetRouteTable(diagrouteTable);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    for (const auto &lt : diagrouteTable) {
        NETNATIVE_LOGI(
            "show NetDiagRouteTable destination_:%{public}s gateway_:%{public}s"
            "mask_:%{public}s iface_:%{public}s flags_:%{public}s metric_:%{public}d"
            "ref_:%{public}d use_:%{public}d",
            lt.destination_.c_str(), lt.gateway_.c_str(), lt.mask_.c_str(), lt.iface_.c_str(), lt.flags_.c_str(),
            lt.metric_, lt.ref_, lt.use_);
    }
}

void ShowSocketInfo(NetsysNative::NetDiagSocketsInfo &info)
{
    for (const auto &lt : info.netProtoSocketsInfo_) {
        NETNATIVE_LOGI(
            "ShowSocketInfo NeyDiagNetProtoSocketInfo protocol_:%{public}s localAddr_:%{public}s"
            "foreignAddr_:%{public}s state_:%{public}s user_:%{public}s programName_:%{public}s recvQueue_:%{public}d"
            "sendQueue_:%{public}d inode_:%{public}d ",
            lt.protocol_.c_str(), lt.localAddr_.c_str(), lt.foreignAddr_.c_str(), lt.state_.c_str(), lt.user_.c_str(),
            lt.programName_.c_str(), lt.recvQueue_, lt.sendQueue_, lt.inode_);
    }

    for (const auto &lt : info.unixSocketsInfo_) {
        NETNATIVE_LOGI(
            "ShowSocketInfo  unixSocketsInfo_ refCnt_:%{public}d inode_:%{public}d protocol_:%{public}s"
            "flags_:%{public}s type_:%{public}s state_:%{public}s path_:%{public}s",
            lt.refCnt_, lt.inode_, lt.protocol_.c_str(), lt.flags_.c_str(), lt.type_.c_str(), lt.state_.c_str(),
            lt.path_.c_str());
    }
}

HWTEST_F(NetsysControllerTest, NetDiagGetSocketsInfo001, TestSize.Level1)
{
    OHOS::NetsysNative::NetDiagProtocolType socketType = OHOS::NetsysNative::NetDiagProtocolType::PROTOCOL_TYPE_ALL;
    OHOS::NetsysNative::NetDiagSocketsInfo socketsInfo;
    auto ret = NetsysController::GetInstance().NetDiagGetSocketsInfo(socketType, socketsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ShowSocketInfo(socketsInfo);

    socketsInfo.unixSocketsInfo_.clear();
    socketsInfo.netProtoSocketsInfo_.clear();
    socketType = OHOS::NetsysNative::NetDiagProtocolType::PROTOCOL_TYPE_RAW;
    ret = NetsysController::GetInstance().NetDiagGetSocketsInfo(socketType, socketsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ShowSocketInfo(socketsInfo);

    socketsInfo.unixSocketsInfo_.clear();
    socketsInfo.netProtoSocketsInfo_.clear();
    socketType = OHOS::NetsysNative::NetDiagProtocolType::PROTOCOL_TYPE_TCP;
    ret = NetsysController::GetInstance().NetDiagGetSocketsInfo(socketType, socketsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ShowSocketInfo(socketsInfo);

    socketsInfo.unixSocketsInfo_.clear();
    socketsInfo.netProtoSocketsInfo_.clear();
    socketType = OHOS::NetsysNative::NetDiagProtocolType::PROTOCOL_TYPE_UDP;
    ret = NetsysController::GetInstance().NetDiagGetSocketsInfo(socketType, socketsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ShowSocketInfo(socketsInfo);

    socketsInfo.unixSocketsInfo_.clear();
    socketsInfo.netProtoSocketsInfo_.clear();
    socketType = OHOS::NetsysNative::NetDiagProtocolType::PROTOCOL_TYPE_UNIX;
    ret = NetsysController::GetInstance().NetDiagGetSocketsInfo(socketType, socketsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ShowSocketInfo(socketsInfo);
}

HWTEST_F(NetsysControllerTest, NetDiagGetInterfaceConfig001, TestSize.Level1)
{
    std::list<OHOS::NetsysNative::NetDiagIfaceConfig> configs;
    std::string ifaceName = "eth0";

    auto ret = NetsysController::GetInstance().NetDiagGetInterfaceConfig(configs, ifaceName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    for (const OHOS::NetsysNative::NetDiagIfaceConfig &lt : configs) {
        NETNATIVE_LOGI(
            "ShowSocketInfo  DiagGetInterfaceConfig  ifaceName_:%{public}s linkEncap_:%{public}s  macAddr_:%{public}s"
            "ipv4Addr_:%{public}s ipv4Bcast_:%{public}s ipv4Mask_:%{public}s mtu_:%{public}d txQueueLen_:%{public}d"
            "rxBytes_:%{public}d txBytes_:%{public}d isUp_:%{public}d",
            lt.ifaceName_.c_str(), lt.linkEncap_.c_str(), lt.macAddr_.c_str(), lt.ipv4Addr_.c_str(),
            lt.ipv4Bcast_.c_str(), lt.ipv4Mask_.c_str(), lt.mtu_, lt.txQueueLen_, lt.rxBytes_, lt.txBytes_, lt.isUp_);
    }

    configs.clear();
    ifaceName = "eth1";
    ret = NetsysController::GetInstance().NetDiagGetInterfaceConfig(configs, ifaceName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    for (const OHOS::NetsysNative::NetDiagIfaceConfig &lt : configs) {
        NETNATIVE_LOGI(
            "ShowSocketInfo  DiagGetInterfaceConfig ifaceName_:%{public}s linkEncap_:%{public}s  macAddr_:%{public}s"
            "ipv4Addr_:%{public}s ipv4Bcast_:%{public}s ipv4Mask_:%{public}s mtu_:%{public}d txQueueLen_:%{public}d"
            "rxBytes_:%{public}d txBytes_:%{public}d isUp_:%{public}d ",
            lt.ifaceName_.c_str(), lt.linkEncap_.c_str(), lt.macAddr_.c_str(), lt.ipv4Addr_.c_str(),
            lt.ipv4Bcast_.c_str(), lt.ipv4Mask_.c_str(), lt.mtu_, lt.txQueueLen_, lt.rxBytes_, lt.txBytes_, lt.isUp_);
    }
}

HWTEST_F(NetsysControllerTest, NetDiagSetInterfaceActiveState001, TestSize.Level1)
{
    std::list<OHOS::NetsysNative::NetDiagIfaceConfig> configs;
    std::string ifaceName = "eth0";

    auto ret = NetsysController::GetInstance().NetDiagSetInterfaceActiveState(ifaceName, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    configs.clear();
    ifaceName = "eth1";
    ret = NetsysController::GetInstance().NetDiagSetInterfaceActiveState(ifaceName, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, NetDiagUpdateInterfaceConfig001, TestSize.Level1)
{
    std::string ifaceName = "eth0";
    OHOS::NetsysNative::NetDiagIfaceConfig config;
    config.ifaceName_ = ifaceName;
    config.ipv4Addr_ = "192.168.222.234";
    config.ipv4Mask_ = "255.255.255.0";
    config.ipv4Bcast_ = "255.255.255.0";
    bool add = true;
    auto ret = NetsysController::GetInstance().NetDiagUpdateInterfaceConfig(config, ifaceName, add);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ifaceName = "eth1";
    add = false;
    ret = NetsysController::GetInstance().NetDiagUpdateInterfaceConfig(config, ifaceName, add);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, NetDiagPing001, TestSize.Level1)
{
    OHOS::NetsysNative::NetDiagPingOption pingOption;
    pingOption.destination_ = "127.0.0.1";
    const int maxWaitSecond = 10;
    g_isWaitAsync = true;
    auto ret = NetsysController::GetInstance().NetDiagPingHost(pingOption, netDiagCallback);
    EXPECT_EQ(ret, IPC_ERR_FLATTEN_OBJECT);
    std::chrono::steady_clock::time_point tp1 = std::chrono::steady_clock::now();
    while (g_isWaitAsync) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::chrono::steady_clock::time_point tp2 = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(tp2 - tp1).count() > maxWaitSecond) {
            break;
        }
    }
}

HWTEST_F(NetsysControllerTest, NetsysControllerErr007, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";

    auto ret = instance_->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetworkCreatePhysical(NET_ID, PERMISSION);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    std::string cmd = "";
    std::string respond = "";
    ret = instance_->SetIptablesCommandForRes(cmd, respond);
    EXPECT_EQ(ret, NetManagerStandard::ERR_INVALID_DATA);

    OHOS::NetsysNative::NetDiagPingOption pingOption = {};
    ret = instance_->NetDiagPingHost(pingOption, netDiagCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    std::list<OHOS::NetsysNative::NetDiagRouteTable> diagrouteTable;
    ret = instance_->NetDiagGetRouteTable(diagrouteTable);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    OHOS::NetsysNative::NetDiagProtocolType socketType = OHOS::NetsysNative::NetDiagProtocolType::PROTOCOL_TYPE_ALL;
    OHOS::NetsysNative::NetDiagSocketsInfo socketsInfo = {};
    ret = instance_->NetDiagGetSocketsInfo(socketType, socketsInfo);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    std::list<OHOS::NetsysNative::NetDiagIfaceConfig> configs;
    std::string ifaceName = "eth0";
    ret = instance_->NetDiagGetInterfaceConfig(configs, ifaceName);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    OHOS::NetsysNative::NetDiagIfaceConfig config;
    ret = instance_->NetDiagUpdateInterfaceConfig(config, ifaceName, false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = instance_->NetDiagSetInterfaceActiveState(ifaceName, false);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerBranchTest001, TestSize.Level1)
{
    instance_->initFlag_ = true;
    instance_->Init();

    instance_->initFlag_ = false;
    instance_->Init();

    std::vector<int32_t> beginUids = {1};
    std::vector<int32_t> endUids = {1};
    int32_t netId = 0;

    NetsysController::GetInstance().NetworkCreateVirtual(netId, false);

    auto ret = instance_->NetworkAddUids(netId, beginUids, endUids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->NetworkDelUids(netId, beginUids, endUids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    endUids = {1, 2};
    ret = instance_->NetworkAddUids(netId, beginUids, endUids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_INTERNAL);

    ret = instance_->NetworkDelUids(netId, beginUids, endUids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerBranchTest002, TestSize.Level1)
{
    uint32_t uid = 0;
    uint8_t allow = 0;
    auto ret = NetsysController::GetInstance().SetInternetPermission(uid, allow);
    EXPECT_NE(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    ret = NetsysController::GetInstance().AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    NetsysNotifyCallback callback;
    ret = NetsysController::GetInstance().RegisterNetsysNotifyCallback(callback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    int32_t netId = 0;
    int32_t permission = 0;
    ret = NetsysController::GetInstance().NetworkCreatePhysical(netId, permission);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = NetsysController::GetInstance().NetworkCreateVirtual(netId, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}


HWTEST_F(NetsysControllerTest, GetNetworkSharingTypeTest001, TestSize.Level1)
{
    std::set<uint32_t> sharingTypeIsOn;
    int32_t ret = NetsysController::GetInstance().GetNetworkSharingType(sharingTypeIsOn);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, UpdateNetworkSharingTypeTest001, TestSize.Level1)
{
    uint64_t type = 0;
    bool isOpen = true;
    int32_t ret = NetsysController::GetInstance().UpdateNetworkSharingType(type, isOpen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, NetsysControllerBranchTest003, TestSize.Level1)
{
    uint32_t timeStep = 0;
    sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> callback = nullptr;
    int32_t ret = NetsysController::GetInstance().RegisterDnsResultCallback(callback, timeStep);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = NetsysController::GetInstance().UnregisterDnsResultCallback(callback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<OHOS::NetsysNative::INetDnsHealthCallback> healthCallback = nullptr;
    ret = NetsysController::GetInstance().RegisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    ret = NetsysController::GetInstance().UnregisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetsysControllerTest, NetsysControllerBranchTest004, TestSize.Level1)
{
    NetsysController::GetInstance().netsysService_ = nullptr;
    uint32_t timeStep = 0;
    sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> callback = nullptr;
    int32_t ret = NetsysController::GetInstance().RegisterDnsResultCallback(callback, timeStep);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = NetsysController::GetInstance().UnregisterDnsResultCallback(callback);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    sptr<OHOS::NetsysNative::INetDnsHealthCallback> healthCallback = nullptr;
    ret = NetsysController::GetInstance().RegisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);

    ret = NetsysController::GetInstance().UnregisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, SetNetworkAccessPolicy001, TestSize.Level1)
{
    uint32_t uid = 0;
    NetworkAccessPolicy netAccessPolicy;
    netAccessPolicy.wifiAllow = false;
    netAccessPolicy.cellularAllow = false;
    bool reconfirmFlag = true;
    int32_t ret = NetsysController::GetInstance().SetNetworkAccessPolicy(uid, netAccessPolicy, reconfirmFlag);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NotifyNetBearerTypeChange001, TestSize.Level1)
{
    std::set<NetManagerStandard::NetBearType> bearTypes;
    bearTypes.insert(NetManagerStandard::NetBearType::BEARER_CELLULAR);
    int32_t ret = NetsysController::GetInstance().NotifyNetBearerTypeChange(bearTypes);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, DeleteNetworkAccessPolicy001, TestSize.Level1)
{
    uint32_t uid = 0;
    int32_t ret = NetsysController::GetInstance().DeleteNetworkAccessPolicy(uid);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_NETSYSSERVICE_NULL);
}
} // namespace NetManagerStandard
} // namespace OHOS
