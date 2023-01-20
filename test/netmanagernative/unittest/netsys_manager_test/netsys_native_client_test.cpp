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
const int64_t UID = 1010;
const int32_t SOCKET_FD = 5;
const int32_t PERMISSION = 5;
const int32_t PREFIX_LENGTH = 23;
uint16_t BASE_TIMEOUT_MSEC = 200;
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
    static inline NetsysNativeClient nativeClient_;
};

void NetsysNativeClientTest::SetUpTestCase() {}

void NetsysNativeClientTest::TearDownTestCase() {}

void NetsysNativeClientTest::SetUp() {}

void NetsysNativeClientTest::TearDown() {}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest001, TestSize.Level1)
{
    int32_t ret = nativeClient_.NetworkCreatePhysical(NET_ID, PERMISSION);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.NetworkCreatePhysical(NET_ID, PERMISSION);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.NetworkDestroy(NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.NetworkAddInterface(NET_ID, IF_NAME);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.NetworkRemoveInterface(NET_ID, IF_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.NetworkAddRoute(NET_ID, IF_NAME, DESTINATION, NEXT_HOP);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.NetworkRemoveRoute(NET_ID, IF_NAME, DESTINATION, NEXT_HOP);
    EXPECT_EQ(ret, -1);

    OHOS::nmd::InterfaceConfigurationParcel parcel;
    ret = nativeClient_.InterfaceGetConfig(parcel);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.SetInterfaceDown(IF_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.SetInterfaceUp(IF_NAME);
    EXPECT_EQ(ret, 0);

    nativeClient_.InterfaceClearAddrs(IF_NAME);

    ret = nativeClient_.InterfaceGetMtu(IF_NAME);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.InterfaceSetMtu(IF_NAME, MTU);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest002, TestSize.Level1)
{
    int32_t ret = nativeClient_.InterfaceAddAddress(IF_NAME, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, -19);

    ret = nativeClient_.InterfaceDelAddress(IF_NAME, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, -19);

    ret = nativeClient_.SetResolverConfig(NET_ID, BASE_TIMEOUT_MSEC, RETRY_COUNT, {}, {});
    EXPECT_EQ(ret, 0);

    std::vector<std::string> servers;
    std::vector<std::string> domains;
    ret = nativeClient_.GetResolverConfig(NET_ID, servers, domains, BASE_TIMEOUT_MSEC, RETRY_COUNT);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.CreateNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.DestroyNetworkCache(NET_ID);
    EXPECT_EQ(ret, 0);

    nmd::NetworkSharingTraffic traffic;
    ret = nativeClient_.GetNetworkSharingTraffic(ETH0, ETH0, traffic);
    EXPECT_NE(ret, 0);

    ret = nativeClient_.GetCellularRxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetCellularTxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetAllRxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetAllTxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest003, TestSize.Level1)
{
    int32_t ret = nativeClient_.GetUidRxBytes(NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetUidTxBytes(NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetUidOnIfaceRxBytes(NET_ID, INTERFACE_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetUidOnIfaceTxBytes(NET_ID, INTERFACE_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetIfaceRxBytes(INTERFACE_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetIfaceTxBytes(INTERFACE_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    std::vector<std::string> interFaceGetList = nativeClient_.InterfaceGetList();
    EXPECT_NE(interFaceGetList.size(), 0U);

    std::vector<std::string> uidGetList = nativeClient_.UidGetList();
    EXPECT_EQ(uidGetList.size(), 0U);

    ret = nativeClient_.GetIfaceRxPackets(INTERFACE_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.GetIfaceTxPackets(INTERFACE_NAME);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    std::vector<uint32_t> uids;
    uids.push_back(UID);
    ret = nativeClient_.FirewallSetUidsAllowedListChain(CHAIN, uids);
    EXPECT_EQ(ret, -1);
    ret = nativeClient_.FirewallSetUidsDeniedListChain(CHAIN, uids);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest004, TestSize.Level1)
{
    int32_t ret = nativeClient_.SetDefaultNetWork(NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.ClearDefaultNetWorkNetId();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.BindSocket(SOCKET_FD, NET_ID);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = nativeClient_.IpEnableForwarding(REQUESTOR);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.IpDisableForwarding(REQUESTOR);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.EnableNat(ETH0, ETH0);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.DisableNat(ETH0, ETH0);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.IpfwdAddInterfaceForward(ETH0, ETH0);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.IpfwdRemoveInterfaceForward(ETH0, ETH0);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.ShareDnsSet(NET_ID);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.StartDnsProxyListen();
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.StopDnsProxyListen();
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.FirewallEnableChain(CHAIN, true);
    EXPECT_EQ(ret, -1);
    ret = nativeClient_.FirewallSetUidRule(CHAIN, NET_ID, FIREWALL_RULE);
    EXPECT_EQ(ret, -1);
}
} // namespace NetManagerStandard
} // namespace OHOS