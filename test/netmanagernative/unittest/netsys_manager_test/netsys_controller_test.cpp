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

#include "net_manager_constants.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *WLAN = "wlan0";
static constexpr const char *ETH0 = "eth0";
static constexpr const char *DESTINATION = "192.168.1.3/24";
static constexpr const char *NEXT_HOP = "192.168.1.1";
static constexpr const char *PARCEL_IPV4_ADDR = "192.168.55.121";
static constexpr const char *IP_ADDR = "172.17.5.245";
static constexpr const char *INTERFACE_NAME = "";
static constexpr const char *IF_NAME = "iface0";
const int NET_ID = 2;
const int PERMISSION = 5;
const int PREFIX_LENGTH = 23;
const int TEST_MTU = 111;
uint16_t BASE_TIMEOUT_MSEC = 200;
uint8_t RETRY_COUNT = 3;
const int64_t TEST_UID = 1010;
const int32_t SOCKET_FD = 5;
int g_ifaceFd = 5;
const int64_t BYTES = 2097152;
const uint32_t FIREWALL_RULE = 1;
} // namespace

class NetsysControllerTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void NetsysControllerTest::SetUpTestCase() {}

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
    int32_t ret = NetsysController::GetInstance().InterfaceGetConfig(parcel);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest005, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().SetInterfaceDown(ETH0);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().SetInterfaceUp(ETH0);
    EXPECT_EQ(ret, 0);

    NetsysController::GetInstance().InterfaceClearAddrs(ETH0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest006, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().InterfaceSetMtu(ETH0, TEST_MTU);
    EXPECT_EQ(ret, -1);

    ret = NetsysController::GetInstance().InterfaceGetMtu(ETH0);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest007, TestSize.Level1)
{
    auto ifaceList = NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), std::string(ETH0)) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }

    int32_t ret = NetsysController::GetInstance().InterfaceAddAddress(ETH0, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, 0);

    ret = NetsysController::GetInstance().InterfaceDelAddress(ETH0, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysControllerTest, NetsysControllerTest008, TestSize.Level1)
{
    int32_t ret = NetsysController::GetInstance().SetResolverConfig(
            NET_ID, BASE_TIMEOUT_MSEC, RETRY_COUNT, {}, {});
    EXPECT_EQ(ret, 0);

    std::vector<std::string> servers;
    std::vector<std::string> domains;
    ret = NetsysController::GetInstance().GetResolverConfig(
            NET_ID, servers, domains, BASE_TIMEOUT_MSEC, RETRY_COUNT);
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
    EXPECT_GE(getList.size(), 0);

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

    ret = NetsysController::GetInstance().FirewallSetUidRule(TEST_UID, TEST_UID, FIREWALL_RULE);
    EXPECT_NE(ret, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
