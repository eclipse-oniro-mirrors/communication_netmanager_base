/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "common_net_diag_callback_test.h"
#include "common_notify_callback_test.h"
#include "i_netsys_service.h"
#include "net_dns_health_callback_stub.h"
#include "net_dns_result_callback_stub.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_stub.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace testing::ext;
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
} // namespace
static constexpr uint64_t TEST_COOKIE = 1;

class TestNetDnsResultCallback : public NetDnsResultCallbackStub {
public:
    TestNetDnsResultCallback() = default;
    ~TestNetDnsResultCallback() override{};

    int32_t OnDnsResultReport(uint32_t size, const std::list<NetDnsResultReport>) override
    {
        return 0;
    }
};

class TestNetDnsHealthCallback : public NetDnsHealthCallbackStub {
public:
    TestNetDnsHealthCallback() = default;
    ~TestNetDnsHealthCallback() override{};

    int32_t OnDnsHealthReport(const NetDnsHealthReport &dnsHealthReport) override
    {
        return 0;
    }
};

class TestNetsysNativeServiceStub : public NetsysNativeServiceStub {
public:
    TestNetsysNativeServiceStub() = default;
    ~TestNetsysNativeServiceStub() override{};

    int32_t SetInternetPermission(uint32_t uid, uint8_t allow, uint8_t isBroker) override
    {
        return 0;
    }

    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                              const std::vector<std::string> &servers, const std::vector<std::string> &domains) override
    {
        return 0;
    }

    int32_t GetResolverConfig(uint16_t netId, std::vector<std::string> &servers, std::vector<std::string> &domains,
                              uint16_t &baseTimeoutMsec, uint8_t &retryCount) override
    {
        return 0;
    }

    int32_t CreateNetworkCache(uint16_t netId) override
    {
        return 0;
    }

    int32_t DestroyNetworkCache(uint16_t netId) override
    {
        return 0;
    }

    int32_t GetAddrInfo(const std::string &hostName, const std::string &serverName, const AddrInfo &hints,
                        uint16_t netId, std::vector<AddrInfo> &res) override
    {
        return 0;
    }

    int32_t SetInterfaceMtu(const std::string &interfaceName, int mtu) override
    {
        return 0;
    }

    int32_t SetTcpBufferSizes(const std::string &tcpBufferSizes) override
    {
        return 0;
    }

    int32_t GetInterfaceMtu(const std::string &interfaceName) override
    {
        return 0;
    }

    int32_t RegisterNotifyCallback(sptr<INotifyCallback> &callback) override
    {
        return 0;
    }

    int32_t UnRegisterNotifyCallback(sptr<INotifyCallback> &callback) override
    {
        return 0;
    }

    int32_t NetworkAddRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
                            const std::string &nextHop) override
    {
        return 0;
    }

    int32_t NetworkRemoveRoute(int32_t netId, const std::string &interfaceName, const std::string &destination,
                               const std::string &nextHop) override
    {
        return 0;
    }

    int32_t NetworkAddRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) override
    {
        return 0;
    }

    int32_t NetworkRemoveRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo) override
    {
        return 0;
    }

    int32_t NetworkSetDefault(int32_t netId) override
    {
        return 0;
    }

    int32_t NetworkGetDefault() override
    {
        return 0;
    }

    int32_t NetworkClearDefault() override
    {
        return 0;
    }

    int32_t GetProcSysNet(int32_t family, int32_t which, const std::string &ifname, const std::string &parameter,
                          std::string &value) override
    {
        return 0;
    }

    int32_t SetProcSysNet(int32_t family, int32_t which, const std::string &ifname, const std::string &parameter,
                          std::string &value) override
    {
        return 0;
    }

    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) override
    {
        return 0;
    }

    int32_t NetworkCreateVirtual(int32_t netId, bool hasDns) override
    {
        return 0;
    }

    int32_t NetworkAddUids(int32_t netId, const std::vector<UidRange> &uidRanges) override
    {
        return 0;
    }

    int32_t NetworkDelUids(int32_t netId, const std::vector<UidRange> &uidRanges) override
    {
        return 0;
    }

    int32_t AddInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                int32_t prefixLength) override
    {
        return 0;
    }

    int32_t DelInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                int32_t prefixLength) override
    {
        return 0;
    }

    int32_t InterfaceSetIpAddress(const std::string &ifaceName, const std::string &ipAddress) override
    {
        return 0;
    }

    int32_t InterfaceSetIffUp(const std::string &ifaceName) override
    {
        return 0;
    }

    int32_t NetworkAddInterface(int32_t netId, const std::string &iface) override
    {
        return 0;
    }

    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) override
    {
        return 0;
    }

    int32_t NetworkDestroy(int32_t netId) override
    {
        return 0;
    }

    int32_t GetFwmarkForNetwork(int32_t netId, MarkMaskParcel &markMaskParcel) override
    {
        return 0;
    }

    int32_t SetInterfaceConfig(const InterfaceConfigurationParcel &cfg) override
    {
        return 0;
    }

    int32_t GetInterfaceConfig(InterfaceConfigurationParcel &cfg) override
    {
        return 0;
    }

    int32_t InterfaceGetList(std::vector<std::string> &ifaces) override
    {
        return 0;
    }

    int32_t StartDhcpClient(const std::string &iface, bool bIpv6) override
    {
        return 0;
    }

    int32_t StopDhcpClient(const std::string &iface, bool bIpv6) override
    {
        return 0;
    }

    int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr) override
    {
        return 0;
    }

    int32_t StopDhcpService(const std::string &iface) override
    {
        return 0;
    }

    int32_t IpEnableForwarding(const std::string &requestor) override
    {
        return 0;
    }

    int32_t IpDisableForwarding(const std::string &requestor) override
    {
        return 0;
    }

    int32_t EnableNat(const std::string &downstreamIface, const std::string &upstreamIface) override
    {
        return 0;
    }

    int32_t DisableNat(const std::string &downstreamIface, const std::string &upstreamIface) override
    {
        return 0;
    }

    int32_t IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface) override
    {
        return 0;
    }

    int32_t IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface) override
    {
        return 0;
    }

    int32_t BandwidthAddAllowedList(uint32_t uid) override
    {
        return 0;
    }

    int32_t BandwidthRemoveAllowedList(uint32_t uid) override
    {
        return 0;
    }

    int32_t BandwidthEnableDataSaver(bool enable) override
    {
        return 0;
    }

    int32_t BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes) override
    {
        return 0;
    }

    int32_t BandwidthAddDeniedList(uint32_t uid) override
    {
        return 0;
    }

    int32_t BandwidthRemoveDeniedList(uint32_t uid) override
    {
        return 0;
    }

    int32_t BandwidthRemoveIfaceQuota(const std::string &ifName) override
    {
        return 0;
    }

    int32_t FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override
    {
        return 0;
    }

    int32_t FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override
    {
        return 0;
    }

    int32_t FirewallEnableChain(uint32_t chain, bool enable) override
    {
        return 0;
    }

    int32_t FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids, uint32_t firewallRule) override
    {
        return 0;
    }

    int32_t ShareDnsSet(uint16_t netId) override
    {
        return 0;
    }

    int32_t StartDnsProxyListen() override
    {
        return 0;
    }

    int32_t StopDnsProxyListen() override
    {
        return 0;
    }

    int32_t GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                     NetworkSharingTraffic &traffic) override
    {
        return 0;
    }

    int32_t GetTotalStats(uint64_t &stats, uint32_t type) override
    {
        return 0;
    }

    int32_t GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid) override
    {
        return 0;
    }

    int32_t GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName) override
    {
        return 0;
    }

    int32_t GetAllContainerStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override
    {
        return 0;
    }

    int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override
    {
        return 0;
    }

    int32_t SetIptablesCommandForRes(const std::string &cmd, std::string &respond) override
    {
        return 0;
    }

    int32_t NetDiagPingHost(const NetDiagPingOption &pingOption, const sptr<INetDiagCallback> &callback) override
    {
        return 0;
    }

    int32_t NetDiagGetRouteTable(std::list<NetDiagRouteTable> &routeTables) override
    {
        return 0;
    }

    int32_t NetDiagGetSocketsInfo(NetDiagProtocolType socketType, NetDiagSocketsInfo &socketsInfo) override
    {
        return 0;
    }

    int32_t NetDiagGetInterfaceConfig(std::list<NetDiagIfaceConfig> &configs, const std::string &ifaceName) override
    {
        return 0;
    }

    int32_t NetDiagUpdateInterfaceConfig(const NetDiagIfaceConfig &config, const std::string &ifaceName,
                                         bool add) override
    {
        return 0;
    }

    int32_t NetDiagSetInterfaceActiveState(const std::string &ifaceName, bool up) override
    {
        return 0;
    }

    int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr, const std::string &ifName) override
    {
        return 0;
    }

    int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr, const std::string &ifName) override
    {
        return 0;
    }

    int32_t RegisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback, uint32_t delay) override
    {
        return 0;
    }

    int32_t UnregisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback) override
    {
        return 0;
    }

    int32_t RegisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback) override
    {
        return 0;
    }

    int32_t UnregisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback) override
    {
        return 0;
    }

    int32_t GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie) override
    {
        return 0;
    }

    int32_t GetNetworkSharingType(std::set<uint32_t>& sharingTypeIsOn) override
    {
        return 0;
    }
    
    int32_t UpdateNetworkSharingType(uint32_t type, bool isOpen) override
    {
        return 0;
    }

    int32_t SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on) override
    {
        return 0;
    }

    int32_t SetEnableIpv6(const std::string &interfaceName, const uint32_t on) override
    {
        return 0;
    }

    int32_t SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag) override
    {
        return 0;
    }

    int32_t DeleteNetworkAccessPolicy(uint32_t uid) override
    {
        return 0;
    }

    int32_t NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes) override
    {
        return 0;
    }
};

class NetsysNativeServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<NetsysNativeServiceStub> notifyStub_ = nullptr;
    sptr<NetDiagCallbackStubTest> ptrCallback = new NetDiagCallbackStubTest();
};

void NetsysNativeServiceStubTest::SetUpTestCase()
{
    notifyStub_ = std::make_shared<TestNetsysNativeServiceStub>();
}

void NetsysNativeServiceStubTest::TearDownTestCase() {}

void NetsysNativeServiceStubTest::SetUp() {}

void NetsysNativeServiceStubTest::TearDown() {}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetResolverConfig001, TestSize.Level1)
{
    uint16_t netId = 1001;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t vServerSize = 2;
    std::string strServer = "TestServer";

    int32_t vDomainSize = 1;
    std::string strDomain = "TestDomain";

    MessageParcel data;
    EXPECT_TRUE(data.WriteUint16(netId));
    EXPECT_TRUE(data.WriteUint16(baseTimeoutMsec));
    EXPECT_TRUE(data.WriteUint16(retryCount));
    EXPECT_TRUE(data.WriteUint32(vServerSize));
    EXPECT_TRUE(data.WriteString(strServer));
    EXPECT_TRUE(data.WriteUint32(vDomainSize));
    EXPECT_TRUE(data.WriteString(strDomain));
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdSetResolverConfig(data, reply);
    EXPECT_EQ(ret, ERR_FLATTEN_OBJECT);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetResolverConfig002, TestSize.Level1)
{
    uint16_t netId = 1001;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t vServerSize = 0;
    int32_t vDomainSize = 0;
    MessageParcel data;
    EXPECT_TRUE(data.WriteUint16(netId));
    EXPECT_TRUE(data.WriteUint16(baseTimeoutMsec));
    EXPECT_TRUE(data.WriteUint8(retryCount));
    EXPECT_TRUE(data.WriteUint32(vServerSize));
    EXPECT_TRUE(data.WriteUint32(vDomainSize));
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdSetResolverConfig(data, reply);
    DTEST_LOG << "CmdSetResolverConfig002" << ret << std::endl;
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetResolverConfig003, TestSize.Level1)
{
    uint16_t netId = 1001;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t vServerSize = 2;
    std::string server = "testserver";
    int32_t vDomainSize = 2;
    std::string domain = "testdomain";
    MessageParcel data;
    EXPECT_TRUE(data.WriteUint16(netId));
    EXPECT_TRUE(data.WriteUint16(baseTimeoutMsec));
    EXPECT_TRUE(data.WriteUint8(retryCount));
    EXPECT_TRUE(data.WriteUint32(vServerSize));
    for (int32_t i = 0; i < vServerSize; i++) {
        EXPECT_TRUE(data.WriteString(server));
    }

    EXPECT_TRUE(data.WriteUint32(vDomainSize));
    for (int32_t i = 0; i < vDomainSize; i++) {
        EXPECT_TRUE(data.WriteString(domain));
    }
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdSetResolverConfig(data, reply);
    DTEST_LOG << "CmdSetResolverConfig003" << ret << std::endl;
    EXPECT_EQ(ret, ERR_NONE);
}

bool IsDataParemerVaild(MessageParcel &data)
{
    uint16_t netId = 1001;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return false;
    }
    if (!data.WriteUint16(netId)) {
        return false;
    }
    return true;
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetResolverConfig001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    if (!IsDataParemerVaild(data)) {
        return;
    }
    int32_t ret = notifyStub_->CmdGetResolverConfig(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdCreateNetworkCache001, TestSize.Level1)
{
    MessageParcel data;
    if (!IsDataParemerVaild(data)) {
        return;
    }
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdCreateNetworkCache(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDestroyNetworkCache001, TestSize.Level1)
{
    MessageParcel data;
    if (!IsDataParemerVaild(data)) {
        return;
    }
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdDestroyNetworkCache(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetAddrInfo001, TestSize.Level1)
{
    std::string hostName = "TestHostName";
    std::string serverName = "TestServerName";

    struct AddrInfo addrInfo;
    addrInfo.aiFlags = 0;
    addrInfo.aiFamily = AF_INET;

    uint16_t netId = 1001;

    MessageParcel data;
    EXPECT_TRUE(data.WriteString(hostName));
    EXPECT_TRUE(data.WriteString(serverName));
    EXPECT_TRUE(data.WriteRawData(&addrInfo, sizeof(AddrInfo)));
    EXPECT_TRUE(data.WriteUint16(netId));
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetAddrInfo(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetAddrInfo002, TestSize.Level1)
{
    std::string hostName = "TestHostName";
    std::string serverName = "TestServerName";

    struct AddrInfo addrInfo;
    addrInfo.aiFlags = 0;
    addrInfo.aiFamily = 9999;
    uint16_t netId = 1001;

    MessageParcel data;
    if (!data.WriteString(hostName)) {
        return;
    }
    if (!data.WriteString(serverName)) {
        return;
    }
    if (!data.WriteRawData(&addrInfo, sizeof(AddrInfo))) {
        return;
    }
    if (!data.WriteUint16(netId)) {
        return;
    }
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetAddrInfo(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetInterfaceMtu001, TestSize.Level1)
{
    std::string ifName = "ifName";
    int32_t mtu = 0;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(ifName)) {
        return;
    }
    if (!data.WriteUint32(mtu)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdSetInterfaceMtu(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetInterfaceMtu001, TestSize.Level1)
{
    std::string ifName = "ifName";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(ifName)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetInterfaceMtu(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdRegisterNotifyCallback001, TestSize.Level1)
{
    std::string ifName = "ifName";
    sptr<INotifyCallback> callback = new (std::nothrow) NotifyCallbackTest();
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdRegisterNotifyCallback(data, reply);
    EXPECT_EQ(ret, -1);
    MessageParcel data2;
    EXPECT_TRUE(data2.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor()));
    EXPECT_TRUE(data2.WriteRemoteObject(callback->AsObject().GetRefPtr()));
    ret = notifyStub_->CmdUnRegisterNotifyCallback(data2, reply);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdRegisterNotifyCallback002, TestSize.Level1)
{
    std::string ifName = "ifName";
    sptr<INotifyCallback> callback = new (std::nothrow) NotifyCallbackTest();
    MessageParcel data;
    EXPECT_TRUE(data.WriteRemoteObject(callback->AsObject().GetRefPtr()));

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdRegisterNotifyCallback(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
    MessageParcel data2;
    EXPECT_TRUE(data2.WriteRemoteObject(callback->AsObject().GetRefPtr()));
    ret = notifyStub_->CmdUnRegisterNotifyCallback(data2, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkRouteParcel001, TestSize.Level1)
{
    uint16_t netId = 1001;
    std::string ifName = "ifName";
    std::string destination = "destination";
    std::string nextHop = "nextHop";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint16(netId)) {
        return;
    }
    if (!data.WriteString(ifName)) {
        return;
    }
    if (!data.WriteString(destination)) {
        return;
    }
    if (!data.WriteString(nextHop)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdNetworkAddRouteParcel(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdNetworkRemoveRouteParcel(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkDefault001, TestSize.Level1)
{
    uint16_t netId = 1001;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint16(netId)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdNetworkSetDefault(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdNetworkGetDefault(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdNetworkClearDefault(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdProcSysNet001, TestSize.Level1)
{
    int32_t family = 0;
    int32_t which = 0;
    std::string ifName = "TestIfName";
    std::string parameter = "TestParameter";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(family)) {
        return;
    }
    if (!data.WriteUint32(which)) {
        return;
    }
    if (!data.WriteString(ifName)) {
        return;
    }
    if (!data.WriteString(parameter)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetProcSysNet(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    std::string value = "TestValue";
    if (!data.WriteString(value)) {
        return;
    }

    ret = notifyStub_->CmdSetProcSysNet(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkCreatePhysical001, TestSize.Level1)
{
    int32_t netId = 1001;
    int32_t permission = 0;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(netId)) {
        return;
    }
    if (!data.WriteUint32(permission)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdNetworkCreatePhysical(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdInterfaceAddress001, TestSize.Level1)
{
    std::string interfaceName = "testInterfaceName";
    std::string ipAddr = "testIpAddr";
    int32_t prefixLength = 10;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(interfaceName)) {
        return;
    }
    if (!data.WriteString(ipAddr)) {
        return;
    }
    if (!data.WriteUint32(prefixLength)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdAddInterfaceAddress(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdDelInterfaceAddress(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdInterfaceSetIpAddress001, TestSize.Level1)
{
    std::string interfaceName = "testInterfaceName";
    std::string ipAddress = "testIpAddr";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(interfaceName)) {
        return;
    }
    if (!data.WriteString(ipAddress)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdInterfaceSetIpAddress(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdInterfaceSetIffUp001, TestSize.Level1)
{
    std::string interfaceName = "testInterfaceName";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(interfaceName)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdInterfaceSetIffUp(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkInterface001, TestSize.Level1)
{
    int32_t netId = 1001;
    std::string interfaceName = "testInterfaceName";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(interfaceName)) {
        return;
    }
    if (!data.WriteUint32(netId)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdNetworkAddInterface(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    notifyStub_->CmdNetworkRemoveInterface(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkDestroy001, TestSize.Level1)
{
    int32_t netId = 1001;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(netId)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdNetworkDestroy(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetFwmarkForNetwork001, TestSize.Level1)
{
    int32_t netId = 1001;
    int32_t mark = 0;
    int32_t mask = 0;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(netId)) {
        return;
    }
    if (!data.WriteUint32(mark)) {
        return;
    }
    if (!data.WriteUint32(mask)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetFwmarkForNetwork(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdInterfaceConfig001, TestSize.Level1)
{
    std::string ifName = "testIfName";
    std::string hwAddr = "testHwAddr";
    std::string ipv4Addr = "testIpv4Addr";
    uint32_t prefixLength = 0;
    uint32_t vServerSize = 1;
    std::string flag = "testFlag";

    MessageParcel data;
    EXPECT_TRUE(data.WriteString(ifName));
    EXPECT_TRUE(data.WriteString(hwAddr));
    EXPECT_TRUE(data.WriteString(ipv4Addr));
    EXPECT_TRUE(data.WriteUint32(prefixLength));
    EXPECT_TRUE(data.WriteUint32(vServerSize));
    EXPECT_TRUE(data.WriteString(flag));

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdSetInterfaceConfig(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdGetInterfaceConfig(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdInterfaceGetList001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdInterfaceGetList(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDhcpClient001, TestSize.Level1)
{
    std::string iface = "testIface";
    bool bIpv6 = true;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(iface)) {
        return;
    }
    if (!data.WriteBool(bIpv6)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdStartDhcpClient(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdStopDhcpClient(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDhcpService001, TestSize.Level1)
{
    std::string iface = "testIface";
    std::string ipv4addr = "testIpv4addr";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(iface)) {
        return;
    }
    if (!data.WriteString(ipv4addr)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdStartDhcpService(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdStopDhcpService(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdIpForwarding001, TestSize.Level1)
{
    std::string requester = "testRequester";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(requester)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdIpEnableForwarding(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdIpDisableForwarding(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNat001, TestSize.Level1)
{
    std::string downstreamIface = "testDownstreamIface";
    std::string upstreamIface = "testUpstreamIface";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(downstreamIface)) {
        return;
    }
    if (!data.WriteString(upstreamIface)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdEnableNat(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdDisableNat(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdIpfwdInterfaceForward001, TestSize.Level1)
{
    std::string fromIface = "testFromIface";
    std::string toIface = "testToIface";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(fromIface)) {
        return;
    }
    if (!data.WriteString(toIface)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdIpfwdAddInterfaceForward(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdIpfwdRemoveInterfaceForward(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdBandwidthEnableDataSaver001, TestSize.Level1)
{
    bool enable = true;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteBool(enable)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdBandwidthEnableDataSaver(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdBandwidthIfaceQuota001, TestSize.Level1)
{
    std::string ifName = "testIfName";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(ifName)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdBandwidthSetIfaceQuota(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdBandwidthRemoveIfaceQuota(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdBandwidthList001, TestSize.Level1)
{
    uint32_t uid = 1001;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(uid)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdBandwidthAddDeniedList(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdBandwidthRemoveDeniedList(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdBandwidthAddAllowedList(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdBandwidthRemoveAllowedList(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdFirewallSetUidsListChain001, TestSize.Level1)
{
    uint32_t chain = 0;
    uint32_t uidSize = 1;
    uint32_t uid = 1001;

    MessageParcel data;
    if (!data.WriteUint32(chain)) {
        return;
    }
    if (!data.WriteUint32(uidSize)) {
        return;
    }
    if (!data.WriteUint32(uid)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdFirewallSetUidsAllowedListChain(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdFirewallSetUidsDeniedListChain(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdFirewallEnableChain001, TestSize.Level1)
{
    uint32_t chain = 0;
    bool enable = true;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(chain)) {
        return;
    }
    if (!data.WriteBool(enable)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdFirewallEnableChain(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdFirewallSetUidRule001, TestSize.Level1)
{
    uint32_t chain = 0;
    uint32_t uid = 1001;
    uint32_t firewallRule = 1;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(chain)) {
        return;
    }
    if (!data.WriteUint32(uid)) {
        return;
    }
    if (!data.WriteUint32(firewallRule)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdFirewallSetUidRule(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdShareDnsSet001, TestSize.Level1)
{
    uint16_t netId = 0;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint16(netId)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdShareDnsSet(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDnsProxyListen001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdStartDnsProxyListen(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdStopDnsProxyListen(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetNetworkSharingTraffic001, TestSize.Level1)
{
    std::string downIface = "testDownIface";
    std::string upIface = "testUpIface ";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(downIface)) {
        return;
    }
    if (!data.WriteString(upIface)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetNetworkSharingTraffic(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetTotalStats001, TestSize.Level1)
{
    uint32_t type = 0;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(type)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetTotalStats(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetUidStats001, TestSize.Level1)
{
    uint32_t type = 0;
    uint32_t uId = 2020;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(type)) {
        return;
    }
    if (!data.WriteInt32(uId)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetUidStats(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetIfaceStats001, TestSize.Level1)
{
    uint32_t type = 0;
    std::string Iface = "wlan0";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteInt32(type)) {
        return;
    }
    if (!data.WriteString(Iface)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetIfaceStats(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetAllStatsInfo001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetAllStatsInfo(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetAllContainerStatsInfoTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetAllContainerStatsInfo(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}


HWTEST_F(NetsysNativeServiceStubTest, NetsysFreeAddrinfoTest001, TestSize.Level1)
{
    addrinfo *ai = nullptr;
    int32_t ret = notifyStub_->NetsysFreeAddrinfo(ai);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetInternetPermissionTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t uid = 0;
    uint8_t allow = 1;
    ASSERT_TRUE(data.WriteUint32(uid));
    ASSERT_TRUE(data.WriteUint8(allow));
    int32_t ret = notifyStub_->CmdSetInternetPermission(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkCreateVirtualTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    data.WriteBool(false);
    int32_t ret = notifyStub_->CmdNetworkCreateVirtual(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkAddUidsTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    data.WriteInt32(0);
    int32_t ret = notifyStub_->CmdNetworkAddUids(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetworkDelUidsTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    data.WriteInt32(0);
    int32_t ret = notifyStub_->CmdNetworkDelUids(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetIptablesCommandForResTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdSetIptablesCommandForRes(data, reply);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

HWTEST_F(NetsysNativeServiceStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    uint32_t errcode = 9999;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto result = notifyStub_->OnRemoteRequest(errcode, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
    uint32_t code = 10;
    result = notifyStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_INVALID_DATA_ERR);
    auto descriptor = NetsysNativeServiceStub::GetDescriptor();
    data.WriteInterfaceToken(descriptor);
    code = 8;
    result = notifyStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetDiagPingHostCommandForResTest001, TestSize.Level1)
{
    NetDiagPingOption pingOption;
    MessageParcel data;

    pingOption.Marshalling(data);
    MessageParcel reply;

    int32_t ret = notifyStub_->CmdNetDiagPingHost(data, reply);
    EXPECT_EQ(ret, IPC_STUB_ERR);

    pingOption.Marshalling(data);
    data.WriteRemoteObject(ptrCallback->AsObject().GetRefPtr());
    ret = notifyStub_->CmdNetDiagPingHost(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetDiagGetRouteTableCommandForResTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t ret = notifyStub_->CmdNetDiagGetRouteTable(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetDiagGetSocketsInfoCommandForResTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint8(NetDiagProtocolType::PROTOCOL_TYPE_ALL);
    int32_t ret = notifyStub_->CmdNetDiagGetSocketsInfo(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    data.WriteUint8(NetDiagProtocolType::PROTOCOL_TYPE_TCP);
    ret = notifyStub_->CmdNetDiagGetSocketsInfo(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    data.WriteUint8(NetDiagProtocolType::PROTOCOL_TYPE_UDP);
    ret = notifyStub_->CmdNetDiagGetSocketsInfo(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    data.WriteUint8(NetDiagProtocolType::PROTOCOL_TYPE_UNIX);
    ret = notifyStub_->CmdNetDiagGetSocketsInfo(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    data.WriteUint8(NetDiagProtocolType::PROTOCOL_TYPE_RAW);
    ret = notifyStub_->CmdNetDiagGetSocketsInfo(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetDiagSetInterfaceActiveStateCommandForResTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("eth0");
    data.WriteBool(true);
    int32_t ret = notifyStub_->CmdNetDiagSetInterfaceActiveState(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    data.WriteString("eth0");
    data.WriteBool(false);
    ret = notifyStub_->CmdNetDiagSetInterfaceActiveState(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetDiagUpdateInterfaceConfigCommandForResTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    NetDiagIfaceConfig config;
    config.ifaceName_ = "eth0";
    const std::string ifaceName = "eth0";
    config.ipv4Addr_ = "192.168.222.234";
    config.mtu_ = 1000;
    config.ipv4Mask_ = "255.255.255.0";
    config.ipv4Bcast_ = "255.255.255.0";
    config.txQueueLen_ = 1000;
    config.Marshalling(data);
    data.WriteString("eth0");
    data.WriteBool(true);
    int32_t ret = notifyStub_->CmdNetDiagUpdateInterfaceConfig(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNetDiagGetInterfaceConfigCommandForResTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("eth0");
    int32_t ret = notifyStub_->CmdNetDiagGetInterfaceConfig(data, reply);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    MessageParcel data1;
    MessageParcel reply1;
    data1.WriteString("eth1");
    int32_t ret1 = notifyStub_->CmdNetDiagGetInterfaceConfig(data1, reply1);
    EXPECT_EQ(ret1, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdAddStaticArp001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";

    MessageParcel data;
    if (!data.WriteString(ipAddr)) {
        return;
    }
    if (!data.WriteString(macAddr)) {
        return;
    }
    if (!data.WriteString(ifName)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdAddStaticArp(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDelStaticArp001, TestSize.Level1)
{
    std::string ipAddrTest = "192.168.1.100";
    std::string macAddrTest = "aa:bb:cc:dd:ee:ff";
    std::string ifNameTest = "wlan0";

    MessageParcel data;
    if (!data.WriteString(ipAddrTest)) {
        return;
    }
    if (!data.WriteString(macAddrTest)) {
        return;
    }
    if (!data.WriteString(ifNameTest)) {
        return;
    }

    MessageParcel reply;
    auto ret = notifyStub_->CmdDelStaticArp(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetCookieStats001, TestSize.Level1)
{
    uint32_t type = 0;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(type)) {
        return;
    }
    if (!data.WriteUint64(TEST_COOKIE)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetCookieStats(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdRegisterDnsResultListener001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetDnsResultCallback> callback = new (std::nothrow) TestNetDnsResultCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    uint32_t timeStep = 1;
    if (!data.WriteUint32(timeStep)) {
        return;
    }
    MessageParcel reply;
    int32_t ret = notifyStub_->CmdRegisterDnsResultListener(data, reply);
    EXPECT_EQ(ret, IPC_STUB_ERR);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdUnregisterDnsResultListener001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetDnsResultCallback> callback = new (std::nothrow) TestNetDnsResultCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdUnregisterDnsResultListener(data, reply);
    EXPECT_EQ(ret, IPC_STUB_ERR);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdRegisterDnsHealthListener001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetDnsHealthCallback> callback = new (std::nothrow) TestNetDnsHealthCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdRegisterDnsHealthListener(data, reply);
    EXPECT_EQ(ret, IPC_STUB_ERR);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdUnregisterDnsHealthListener001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetDnsHealthCallback> callback = new (std::nothrow) TestNetDnsHealthCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdUnregisterDnsHealthListener(data, reply);
    EXPECT_EQ(ret, IPC_STUB_ERR);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetNetworkSharingType001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdGetNetworkSharingType(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdUpdateNetworkSharingType001, TestSize.Level1)
{
    uint32_t type = 0;
    bool isOpen = true;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(type)) {
        return;
    }
    if (!data.WriteBool(isOpen)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdUpdateNetworkSharingType(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetNetworkAccessPolicy001, TestSize.Level1)
{
    uint32_t uid = 0;
    NetworkAccessPolicy netAccessPolicy;
    netAccessPolicy.wifiAllow = false;
    netAccessPolicy.cellularAllow = false;
    bool reconfirmFlag = true;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(uid)) {
        return;
    }

    if (!data.WriteUint8(netAccessPolicy.wifiAllow)) {
        return;
    }

    if (!data.WriteUint8(netAccessPolicy.cellularAllow)) {
        return;
    }

    if (!data.WriteBool(reconfirmFlag)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdSetNetworkAccessPolicy(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDeleteNetworkAccessPolicy001, TestSize.Level1)
{
    uint32_t uid = 0;
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(uid)) {
        return;
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdDelNetworkAccessPolicy(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdNotifyNetBearerTypeChange001, TestSize.Level1)
{
    std::set<NetManagerStandard::NetBearType> bearerTypes;
    bearerTypes.clear();
    bearerTypes.insert(NetManagerStandard::NetBearType::BEARER_CELLULAR);
    MessageParcel data;

    uint32_t size = static_cast<uint32_t>(bearerTypes.size());
    if (!data.WriteUint32(size)) {
        return;
    }

    for (auto bearerType : bearerTypes) {
        if (!data.WriteUint32(static_cast<uint32_t>(bearerType))) {
            return;
        }
    }

    MessageParcel reply;
    int32_t ret = notifyStub_->CmdNotifyNetBearerTypeChange(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}
} // namespace NetsysNative
} // namespace OHOS
