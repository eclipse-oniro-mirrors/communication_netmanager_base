/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define private public
#include "netsys_native_service_stub.h"
#undef private
#include "notify_callback_stub.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;

class TestNotifyCallback : public NotifyCallbackStub {
public:
    TestNotifyCallback() = default;
    ~TestNotifyCallback() override {};
    int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                      int scope) override
    {
        return 0;
    }

    int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                      int scope) override
    {
        return 0;
    }

    int32_t OnInterfaceAdded(const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnInterfaceRemoved(const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnInterfaceChanged(const std::string &ifName, bool up) override
    {
        return 0;
    }

    int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up) override
    {
        return 0;
    }

    int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                           const std::string &ifName) override
    {
        return 0;
    }

    int32_t OnDhcpSuccess(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult) override
    {
        return 0;
    }

    int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override
    {
        return 0;
    }
};

class TestNetsysNativeServiceStub : public NetsysNativeServiceStub {
public:
    TestNetsysNativeServiceStub() = default;
    ~TestNetsysNativeServiceStub() override {};

    int32_t SetInternetPermission(uint32_t uid, uint8_t allow) override
    {
        return 0;
    }

    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                      const std::vector<std::string> &servers,
                                      const std::vector<std::string> &domains) override
    {
        return 0;
    }

    int32_t GetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
                                      std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                      uint8_t &retryCount) override
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

    int32_t GetProcSysNet(int32_t family, int32_t which, const std::string &ifname,
                                  const std::string &parameter, std::string &value) override
    {
        return 0;
    }

    int32_t SetProcSysNet(int32_t family, int32_t which, const std::string &ifname,
                                  const std::string &parameter, std::string &value) override
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

    int32_t FirewallSetUidRule(uint32_t chain, uint32_t uid, uint32_t firewallRule) override
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

    int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override
    {
        return 0;
    }

    int32_t SetIpTablesCommandForRes(const std::string &cmd, std::string &respond) override
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
};

void NetsysNativeServiceStubTest::SetUpTestCase()
{
    notifyStub_ =  std::make_shared<TestNetsysNativeServiceStub>();
}

void NetsysNativeServiceStubTest::TearDownTestCase() {}

void NetsysNativeServiceStubTest::SetUp() {}

void NetsysNativeServiceStubTest::TearDown() {}

HWTEST_F(NetsysNativeServiceStubTest, CmdSetResolverConfig001, TestSize.Level1)
{
    uint16_t netId = 1001;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t vServerSize = 1;
    std::string strServer = "TestServer";

    int32_t vDomainSize = 1;
    std::string strDomain = "TestDomain";

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint16(netId)) {
        return;
    }
    if (!data.WriteUint16(baseTimeoutMsec)) {
        return;
    }
    if (!data.WriteUint16(retryCount)) {
        return;
    }
    if (!data.WriteUint32(vServerSize)) {
        return;
    }
    if (!data.WriteString(strServer)) {
        return;
    }
    if (!data.WriteUint32(vDomainSize)) {
        return;
    }
    if (!data.WriteString(strDomain)) {
        return;
    }
    MessageParcel reply;
    bool ret = notifyStub_->CmdSetResolverConfig(data, reply);
    EXPECT_NE(ret, ERR_FLATTEN_OBJECT);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetResolverConfig001, TestSize.Level1)
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
    bool ret = notifyStub_->CmdGetResolverConfig(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdCreateNetworkCache001, TestSize.Level1)
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
    bool ret = notifyStub_->CmdCreateNetworkCache(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDestroyNetworkCache001, TestSize.Level1)
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
    bool ret = notifyStub_->CmdDestroyNetworkCache(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetAddrInfo001, TestSize.Level1)
{
    std::string hostName = "TestHostName";
    std::string serverName = "TestServerName";

    struct AddrInfo addrInfo;
    addrInfo.aiFlags = 0;
    addrInfo.aiFamily = 88;

    uint16_t netId = 1001;

    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
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
    bool ret = notifyStub_->CmdGetAddrInfo(data, reply);
    EXPECT_NE(ret, IPC_STUB_INVALID_DATA_ERR);
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
    bool ret = notifyStub_->CmdSetInterfaceMtu(data, reply);
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
    bool ret = notifyStub_->CmdGetInterfaceMtu(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdRegisterNotifyCallback001, TestSize.Level1)
{
    std::string ifName = "ifName";
    sptr<INotifyCallback> callback = new (std::nothrow) TestNotifyCallback();
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }

    MessageParcel reply;
    bool ret = notifyStub_->CmdRegisterNotifyCallback(data, reply);
    EXPECT_TRUE(ret);

    ret = notifyStub_->CmdUnRegisterNotifyCallback(data, reply);
    EXPECT_TRUE(ret);
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
    bool ret = notifyStub_->CmdNetworkAddRouteParcel(data, reply);
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
    bool ret = notifyStub_->CmdNetworkSetDefault(data, reply);
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
    bool ret = notifyStub_->CmdGetProcSysNet(data, reply);
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
    bool ret = notifyStub_->CmdNetworkCreatePhysical(data, reply);
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
    bool ret = notifyStub_->CmdAddInterfaceAddress(data, reply);
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
    bool ret = notifyStub_->CmdInterfaceSetIpAddress(data, reply);
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
    bool ret = notifyStub_->CmdInterfaceSetIffUp(data, reply);
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
    bool ret = notifyStub_->CmdNetworkAddInterface(data, reply);
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
    bool ret = notifyStub_->CmdNetworkDestroy(data, reply);
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
    bool ret = notifyStub_->CmdGetFwmarkForNetwork(data, reply);
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
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(ifName)) {
        return;
    }
    if (!data.WriteString(hwAddr)) {
        return;
    }
    if (!data.WriteString(ipv4Addr)) {
        return;
    }
    if (!data.WriteUint32(prefixLength)) {
        return;
    }
    if (!data.WriteUint32(vServerSize)) {
        return;
    }
    if (!data.WriteString(flag)) {
        return;
    }

    MessageParcel reply;
    bool ret = notifyStub_->CmdSetInterfaceConfig(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    ret = notifyStub_->CmdGetInterfaceConfig(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdInterfaceGetList001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    bool ret = notifyStub_->CmdInterfaceGetList(data, reply);
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
    bool ret = notifyStub_->CmdStartDhcpClient(data, reply);
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
    bool ret = notifyStub_->CmdStartDhcpService(data, reply);
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
    bool ret = notifyStub_->CmdIpEnableForwarding(data, reply);
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
    bool ret = notifyStub_->CmdEnableNat(data, reply);
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
    bool ret = notifyStub_->CmdIpfwdAddInterfaceForward(data, reply);
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
    bool ret = notifyStub_->CmdBandwidthEnableDataSaver(data, reply);
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
    bool ret = notifyStub_->CmdBandwidthSetIfaceQuota(data, reply);
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
    bool ret = notifyStub_->CmdBandwidthAddDeniedList(data, reply);
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
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }
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
    bool ret = notifyStub_->CmdFirewallSetUidsAllowedListChain(data, reply);
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
    bool ret = notifyStub_->CmdFirewallEnableChain(data, reply);
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
    bool ret = notifyStub_->CmdFirewallSetUidRule(data, reply);
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
    bool ret = notifyStub_->CmdShareDnsSet(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdDnsProxyListen001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    bool ret = notifyStub_->CmdStartDnsProxyListen(data, reply);
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
    bool ret = notifyStub_->CmdGetNetworkSharingTraffic(data, reply);
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
    bool ret = notifyStub_->CmdGetTotalStats(data, reply);
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
    bool ret = notifyStub_->CmdGetUidStats(data, reply);
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
    bool ret = notifyStub_->CmdGetIfaceStats(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceStubTest, CmdGetAllStatsInfo001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor())) {
        return;
    }

    MessageParcel reply;
    bool ret = notifyStub_->CmdGetAllStatsInfo(data, reply);
    EXPECT_EQ(ret, ERR_NONE);
}

} // namespace NetsysNative
} // namespace OHOS