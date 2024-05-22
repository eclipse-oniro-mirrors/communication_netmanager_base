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
#endif
#include "bpf_def.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "net_manager_constants.h"
#include "net_stats_constants.h"
#include "netsys_native_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *DESTINATION = "192.168.1.3/24";
static constexpr const char *NEXT_HOP = "192.168.1.1";
static constexpr const char *LOCALIP = "127.0.0.1";
static constexpr const char *IF_NAME = "iface0";
static constexpr const char *ETH0 = "eth0";
static constexpr const char *IFACE = "test0";
static constexpr const char *IP_ADDR = "172.17.5.245";
static constexpr const char *INTERFACE_NAME = "interface_name";
static constexpr const char *REQUESTOR = "requestor";
static constexpr const char *TCP_BUFFER_SIZES = "524288,1048576,2097152,262144,524288,1048576";
static constexpr uint64_t TEST_COOKIE = 1;
static constexpr uint32_t TEST_STATS_TYPE1 = 0;
static constexpr uint32_t TEST_STATS_TYPE2 = 2;
const int32_t MTU = 111;
const int32_t NET_ID = 2;
const int32_t IFACEFD = 5;
const int64_t UID = 1010;
const int32_t APP_ID = 101010;
const int32_t SOCKET_FD = 5;
const int32_t PERMISSION = 5;
const int32_t STATRUID = 1000;
const int32_t ENDUID = 1100;
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

    OHOS::nmd::InterfaceConfigurationParcel parcel;
    ret = nativeClient_.GetInterfaceConfig(parcel);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.SetInterfaceDown(IF_NAME);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.SetInterfaceUp(IF_NAME);
    EXPECT_EQ(ret, 0);

    nativeClient_.ClearInterfaceAddrs(IF_NAME);

    ret = nativeClient_.GetInterfaceMtu(IF_NAME);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.SetInterfaceMtu(IF_NAME, MTU);
    EXPECT_EQ(ret, -1);

    ret = nativeClient_.SetTcpBufferSizes(TCP_BUFFER_SIZES);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest002, TestSize.Level1)
{
    int32_t ret = nativeClient_.AddInterfaceAddress(IF_NAME, IP_ADDR, PREFIX_LENGTH);
    EXPECT_EQ(ret, -19);

    ret = nativeClient_.DelInterfaceAddress(IF_NAME, IP_ADDR, PREFIX_LENGTH);
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
    ret = nativeClient_.FirewallSetUidRule(CHAIN, {NET_ID}, FIREWALL_RULE);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest005, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = nativeClient_.GetTotalStats(stats, 0);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.GetUidStats(stats, 0, APP_ID);
    EXPECT_EQ(ret, NetStatsResultCode::STATS_ERR_READ_BPF_FAIL);

    ret = nativeClient_.GetIfaceStats(stats, 0, IFACE);
    EXPECT_EQ(ret, NetStatsResultCode::STATS_ERR_GET_IFACE_NAME_FAILED);

    std::vector<OHOS::NetManagerStandard::NetStatsInfo> statsInfo;
    ret = nativeClient_.GetAllStatsInfo(statsInfo);
    EXPECT_EQ(ret, 0);

    ret = nativeClient_.GetAllContainerStatsInfo(statsInfo);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest006, TestSize.Level1)
{
    std::vector<UidRange> uidRanges;
    std::vector<int32_t> beginUids;
    std::vector<int32_t> endUids;
    beginUids.push_back(STATRUID);
    endUids.push_back(ENDUID);
    for (size_t i = 0; i < beginUids.size(); i++) {
        uidRanges.emplace_back(UidRange(beginUids[i], endUids[i]));
    }
    nativeClient_.NetworkAddUids(NET_ID, uidRanges);
    nativeClient_.NetworkDelUids(NET_ID, uidRanges);
    int32_t ret = nativeClient_.NetworkCreatePhysical(NET_ID, PERMISSION);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest007, TestSize.Level1)
{
    NetsysNotifyCallback callback;
    struct ifreq ifreq = {};
    int32_t ret = nativeClient_.RegisterNetsysNotifyCallback(callback);
    EXPECT_EQ(ret, 0);
    ret = nativeClient_.BindNetworkServiceVpn(SOCKET_FD);
    int32_t ifacefd1 = 0;
    nativeClient_.EnableVirtualNetIfaceCard(SOCKET_FD, ifreq, ifacefd1);
    int32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ret = nativeClient_.BindNetworkServiceVpn(sockfd);
    int32_t ifacefd2 = 0;
    nativeClient_.EnableVirtualNetIfaceCard(sockfd, ifreq, ifacefd2);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest008, TestSize.Level1)
{
    int32_t ret = nativeClient_.SetBlocking(IFACEFD, true);
    EXPECT_EQ(ret, NETSYS_ERR_VPN);
    struct ifreq ifreq = {};
    int32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ret = nativeClient_.SetIpAddress(sockfd, LOCALIP, PREFIX_LENGTH, ifreq);
    EXPECT_EQ(ret, NETSYS_ERR_VPN);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest009, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClient_);
    std::string ifName = "wlan";
    bool up = true;
    int32_t ret = notifyCallback.OnInterfaceChanged(ifName, up);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest010, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClient_);
    sptr<OHOS::NetsysNative::DhcpResultParcel> dhcpResult = new (std::nothrow) OHOS::NetsysNative::DhcpResultParcel();
    int32_t ret = notifyCallback.OnDhcpSuccess(dhcpResult);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest011, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClient_);
    std::string limitName = "wlan";
    std::string iface = "vpncard";
    int32_t ret = notifyCallback.OnBandwidthReachedLimit(limitName, iface);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest013, TestSize.Level1)
{
    wptr<IRemoteObject> remote = nullptr;
    nativeClient_.OnRemoteDied(remote);
    int handle = 1;
    sptr<IRemoteObject> result = nullptr;
    std::u16string descriptor = std::u16string();
    result = new (std::nothrow) IPCObjectProxy(handle, descriptor);
    IRemoteObject *object = result.GetRefPtr();
    remote = object;
    nativeClient_.OnRemoteDied(remote);
    uint32_t uid = 0;
    uint8_t allow = 0;
    auto ret = nativeClient_.SetInternetPermission(uid, allow);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientTest014, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    auto ret = nativeClient_.AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    ret = nativeClient_.DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, GetCookieStatsTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    BpfMapper<socket_cookie_stats_key, app_cookie_stats_value> appCookieStatsMap(APP_COOKIE_STATS_MAP_PATH, BPF_ANY);
    int32_t ret = nativeClient_.GetCookieStats(stats, TEST_STATS_TYPE1, TEST_COOKIE);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    ret = nativeClient_.GetCookieStats(stats, TEST_STATS_TYPE2, TEST_COOKIE);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, GetNetworkSharingTypeTest001, TestSize.Level1)
{
    std::set<uint32_t> sharingTypeIsOn;
    int32_t ret = nativeClient_.GetNetworkSharingType(sharingTypeIsOn);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, UpdateNetworkSharingTypeTest001, TestSize.Level1)
{
    uint64_t type = 0;
    bool isOpen = true;
    int32_t ret = nativeClient_.UpdateNetworkSharingType(type, isOpen);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, NetsysNativeClientBranchTest001, TestSize.Level1)
{
    uint32_t timeStep = 0;
    sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> callback = nullptr;
    int32_t ret = nativeClient_.RegisterDnsResultCallback(callback, timeStep);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);
    ret = nativeClient_.UnregisterDnsResultCallback(callback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);

    sptr<OHOS::NetsysNative::INetDnsHealthCallback> healthCallback = nullptr;
    ret = nativeClient_.RegisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);
    ret = nativeClient_.UnregisterDnsHealthCallback(healthCallback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetsysNativeClientTest, SetNetworkAccessPolicy001, TestSize.Level1)
{
    uint32_t uid = 0;
    NetworkAccessPolicy netAccessPolicy;
    netAccessPolicy.wifiAllow = false;
    netAccessPolicy.cellularAllow = false;
    bool reconfirmFlag = true;
    int32_t ret = nativeClient_.SetNetworkAccessPolicy(uid, netAccessPolicy, reconfirmFlag);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, NotifyNetBearerTypeChange001, TestSize.Level1)
{
    std::set<NetManagerStandard::NetBearType> bearerTypes;
    bearerTypes.insert(NetManagerStandard::NetBearType::BEARER_CELLULAR);
    int32_t ret = nativeClient_.NotifyNetBearerTypeChange(bearerTypes);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, DeleteNetworkAccessPolicy001, TestSize.Level1)
{
    uint32_t uid = 0;
    int32_t ret = nativeClient_.DeleteNetworkAccessPolicy(uid);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
