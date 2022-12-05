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

#include "netsys_native_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *DESTINATION = "192.168.1.3/24";
static constexpr const char *NEXT_HOP = "192.168.1.1";
static constexpr const char *IF_NAME = "iface0";
static constexpr const char *ETH0 = "eth0";
static constexpr const char *IP_ADDR = "172.17.5.245";
static constexpr const char *INTERFACE_NAME = "interface_name";
static constexpr const char *REQUESTOR = "requestor";
const int32_t MTU = 111;
const int32_t NET_ID = 2;
int32_t g_ifaceFd = 5;
const int64_t UID = 1010;
const int32_t SOCKET_FD = 5;
const int32_t PERMISSION = 5;
const int32_t PREFIX_LENGTH = 23;
uint16_t BASE_TIMEOUT_MSEC = 200;
const int64_t BYTES = 2097152;
const int64_t CHAIN = 1010;
uint8_t RETRY_COUNT = 3;
const uint32_t FIREWALL_RULE = 1;
} // namespace

class NetsysNativeClientTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void NetsysNativeClientTest::SetUpTestCase() {}

void NetsysNativeClientTest::TearDownTestCase() {}

void NetsysNativeClientTest::SetUp() {}

void NetsysNativeClientTest::TearDown() {}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest001, TestSize.Level1)
{
    NetsysNativeClient nativeClient;
    int32_t ret = nativeClient.NetworkCreatePhysical(NET_ID, PERMISSION);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.NetworkCreatePhysical(NET_ID, PERMISSION);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.NetworkDestroy(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.NetworkAddInterface(NET_ID, IF_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.NetworkRemoveInterface(NET_ID, IF_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.NetworkAddRoute(NET_ID, IF_NAME, DESTINATION, NEXT_HOP);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.NetworkRemoveRoute(NET_ID, IF_NAME, DESTINATION, NEXT_HOP);
    EXPECT_EQ(ret, 0);

    OHOS::nmd::InterfaceConfigurationParcel parcel;
    ret = nativeClient.InterfaceGetConfig(parcel);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.SetInterfaceDown(IF_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.SetInterfaceUp(IF_NAME);
    EXPECT_EQ(ret, 0);

    nativeClient.InterfaceClearAddrs(IF_NAME);

    ret = nativeClient.InterfaceGetMtu(IF_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.InterfaceSetMtu(IF_NAME, MTU);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest002, TestSize.Level1)
{
    NetsysNativeClient nativeClient;
    int32_t ret = nativeClient.InterfaceAddAddress(IF_NAME, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.InterfaceDelAddress(IF_NAME, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.SetResolverConfig(NET_ID, BASE_TIMEOUT_MSEC, RETRY_COUNT, {}, {});
    EXPECT_EQ(ret, 0);

    std::vector<std::string> servers;
    std::vector<std::string> domains;
    ret = nativeClient.GetResolverConfig(NET_ID, servers, domains, BASE_TIMEOUT_MSEC, RETRY_COUNT);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.CreateNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.DestroyNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);

    nmd::NetworkSharingTraffic traffic;
    ret = nativeClient.GetNetworkSharingTraffic(ETH0, ETH0, traffic);
    EXPECT_NE(ret, 0);

    ret = nativeClient.GetCellularRxBytes();
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetCellularTxBytes();
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetAllRxBytes();
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetAllTxBytes();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest003, TestSize.Level1)
{
    NetsysNativeClient nativeClient;
    int32_t ret = nativeClient.GetUidRxBytes(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetUidTxBytes(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetUidOnIfaceRxBytes(NET_ID, INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetUidOnIfaceTxBytes(NET_ID, INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetIfaceRxBytes(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetIfaceTxBytes(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    std::vector<std::string> interFaceGetList = nativeClient.InterfaceGetList();
    EXPECT_NE(interFaceGetList.size(), 0);

    std::vector<std::string> uidGetList = nativeClient.UidGetList();
    EXPECT_NE(uidGetList.size(), 0);

    ret = nativeClient.GetIfaceRxPackets(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.GetIfaceTxPackets(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    std::vector<uint32_t> uids;
    uids.push_back(UID);
    ret = nativeClient.FirewallSetUidsAllowedListChain(CHAIN, uids);
    EXPECT_EQ(ret, 0);
    ret = nativeClient.FirewallSetUidsDeniedListChain(CHAIN, uids);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest004, TestSize.Level1)
{
    NetsysNativeClient nativeClient;
    int32_t ret = nativeClient.SetDefaultNetWork(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.ClearDefaultNetWorkNetId();
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BindSocket(SOCKET_FD, NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.IpEnableForwarding(REQUESTOR);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.IpDisableForwarding(REQUESTOR);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.EnableNat(ETH0, ETH0);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.DisableNat(ETH0, ETH0);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.IpfwdAddInterfaceForward(ETH0, ETH0);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.IpfwdRemoveInterfaceForward(ETH0, ETH0);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.ShareDnsSet(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.StartDnsProxyListen();
    EXPECT_EQ(ret, 0);

    ret = nativeClient.StopDnsProxyListen();
    EXPECT_EQ(ret, 0);

    ret = nativeClient.FirewallEnableChain(CHAIN, true);
    EXPECT_EQ(ret, 0);
    ret = nativeClient.FirewallSetUidRule(CHAIN, NET_ID, FIREWALL_RULE);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest005, TestSize.Level1)
{
    NetsysNativeClient nativeClient;
    int32_t ret = nativeClient.BindNetworkServiceVpn(SOCKET_FD);
    EXPECT_EQ(ret, 0);

    struct ifreq ifrequest;
    ret = nativeClient.EnableVirtualNetIfaceCard(SOCKET_FD, ifrequest, g_ifaceFd);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.SetIpAddress(SOCKET_FD, IP_ADDR, PREFIX_LENGTH, ifrequest);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.SetBlocking(g_ifaceFd, true);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.StartDhcpClient(INTERFACE_NAME, false);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.StopDhcpClient(INTERFACE_NAME, false);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.StartDhcpService(INTERFACE_NAME, IP_ADDR);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.StopDhcpService(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BandwidthEnableDataSaver(true);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BandwidthSetIfaceQuota(INTERFACE_NAME, BYTES);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BandwidthRemoveIfaceQuota(INTERFACE_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BandwidthAddDeniedList(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BandwidthRemoveDeniedList(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BandwidthAddAllowedList(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient.BandwidthRemoveAllowedList(NET_ID);
    EXPECT_EQ(ret, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS