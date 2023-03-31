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

#include <algorithm>
#include <gtest/gtest.h>
#include <string>

#include "system_ability_definition.h"
#include "netsys_controller.h"
#include "interface_manager.h"

#include "dns_config_client.h"
#include "net_manager_constants.h"
#define private public
#include "netsys_native_service.h"
#undef private
#include "notify_callback_stub.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace NetManagerStandard;
using namespace testing::ext;
class TestNotifyCallback : public NotifyCallbackStub {
public:
    TestNotifyCallback() = default;
    ~TestNotifyCallback() override{};
    int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags, int scope) override
    {
        return 0;
    }

    int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags, int scope) override
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
} // namespace

class NetsysNativeServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline auto instance_ = std::make_shared<NetsysNativeService>(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
};

void NetsysNativeServiceTest::SetUpTestCase() {}

void NetsysNativeServiceTest::TearDownTestCase() {}

void NetsysNativeServiceTest::SetUp() {}

void NetsysNativeServiceTest::TearDown() {}

HWTEST_F(NetsysNativeServiceTest, DumpTest001, TestSize.Level1)
{
    instance_->Init();
    int32_t testFd = 11;
    int32_t ret = instance_->Dump(testFd, {});
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, SetResolverConfigTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    uint16_t baseTimeoutMsec = 200;
    uint8_t retryCount = 3;
    int32_t ret = instance_->SetResolverConfig(testNetId, baseTimeoutMsec, retryCount, {}, {});
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, GetResolverConfigTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    uint16_t baseTimeoutMsec = 200;
    uint8_t retryCount = 3;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    int32_t ret = instance_->GetResolverConfig(testNetId, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, CreateNetworkCacheTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    int32_t ret = instance_->CreateNetworkCache(testNetId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, DestroyNetworkCacheTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    int32_t ret = instance_->DestroyNetworkCache(testNetId);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkAddRouteTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string interfaceName = "eth1";
    std::string destination = "";
    std::string nextHop = "";
    int32_t ret = instance_->NetworkAddRoute(testNetId, interfaceName, destination, nextHop);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkAddRouteParcelTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    RouteInfoParcel routeInfo;
    int32_t ret = instance_->NetworkAddRouteParcel(testNetId, routeInfo);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkRemoveRouteParcelTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    RouteInfoParcel routeInfo;
    int32_t ret = instance_->NetworkRemoveRouteParcel(testNetId, routeInfo);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkSetDefaultTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    int32_t ret = instance_->NetworkSetDefault(testNetId);
    EXPECT_GE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkGetDefaultTest001, TestSize.Level1)
{
    int32_t ret = instance_->NetworkGetDefault();
    EXPECT_LE(ret, 154);
}

HWTEST_F(NetsysNativeServiceTest, NetworkClearDefaultTest001, TestSize.Level1)
{
    int32_t ret = instance_->NetworkClearDefault();
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, GetProcSysNetTest001, TestSize.Level1)
{
    int32_t ipversion = 45;
    int32_t which = 14;
    std::string ifname = "testifname";
    std::string paramete = "testparamete";
    std::string value = "testvalue";
    int32_t ret = instance_->GetProcSysNet(ipversion, which, ifname, paramete, value);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, SetProcSysNetTest001, TestSize.Level1)
{
    int32_t ipversion = 45;
    int32_t which = 14;
    std::string ifname = "testifname";
    std::string paramete = "testparamete";
    std::string value = "testvalue";
    int32_t ret = instance_->SetProcSysNet(ipversion, which, ifname, paramete, value);
    EXPECT_LE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, SetInterfaceMtu001, TestSize.Level1)
{
    std::string testName = "test0";
    int32_t mtu = 1500;
    int32_t ret = instance_->SetInterfaceMtu(testName, mtu);
    EXPECT_NE(ret, 0);
    std::string eth0Name = "eth0";
    auto ifaceList = NetsysController::GetInstance().InterfaceGetList();
    bool eth0NotExist = std::find(ifaceList.begin(), ifaceList.end(), eth0Name) == ifaceList.end();
    if (eth0NotExist) {
        return;
    }
    ret = instance_->SetInterfaceMtu(eth0Name, mtu);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, GetInterfaceMtu001, TestSize.Level1)
{
    std::string testName = "test0";
    int32_t ret = instance_->GetInterfaceMtu(testName);
    EXPECT_NE(ret, 0);
    std::string eth0Name = "eth0";
    ret = instance_->GetInterfaceMtu(eth0Name);
    EXPECT_NE(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, RegisterNotifyCallback001, TestSize.Level1)
{
    sptr<INotifyCallback> callback = new (std::nothrow) TestNotifyCallback();
    int32_t ret = instance_->RegisterNotifyCallback(callback);
    EXPECT_EQ(ret, 0);

    ret = instance_->UnRegisterNotifyCallback(callback);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, NetworkRemoveRouteTest001, TestSize.Level1)
{
    uint16_t testNetId = 154;
    std::string interfaceName = "eth1";
    std::string destination = "";
    std::string nextHop = "";
    int32_t ret = instance_->NetworkRemoveRoute(testNetId, interfaceName, destination, nextHop);
    EXPECT_NE(ret, 0);
}

HWTEST_F(NetsysNativeServiceTest, NetworkCreatePhysicalTest001, TestSize.Level1)
{
    int32_t netId = 1000;
    int32_t permission = 0;
    int32_t ret = instance_->NetworkCreatePhysical(netId, permission);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, AddInterfaceAddressTest001, TestSize.Level1)
{
    std::string iFName = "test0";
    std::string addrStr = "192.168.22.33";
    int32_t prefixLength = 24;
    int32_t ret = instance_->AddInterfaceAddress(iFName, addrStr, prefixLength);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, DelInterfaceAddressTest001, TestSize.Level1)
{
    std::string iFName = "test0";
    std::string addrStr = "192.168.22.33";
    int32_t prefixLength = 24;
    int32_t ret = instance_->DelInterfaceAddress(iFName, addrStr, prefixLength);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, InterfaceSetIpAddressTest001, TestSize.Level1)
{
    std::string iFName = "test0";
    std::string addrStr = "192.168.22.33";
    int32_t ret = instance_->InterfaceSetIpAddress(iFName, addrStr);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, InterfaceSetIffUpTest001, TestSize.Level1)
{
    std::string iFName = "test0";
    int32_t ret = instance_->InterfaceSetIffUp(iFName);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkAddInterfaceTest001, TestSize.Level1)
{
    int32_t netId = 1000;
    std::string iFName = "test0";
    int32_t ret = instance_->NetworkAddInterface(netId, iFName);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkRemoveInterfaceTest001, TestSize.Level1)
{
    int32_t netId = 1000;
    std::string iFName = "test0";
    int32_t ret = instance_->NetworkRemoveInterface(netId, iFName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, NetworkDestroyTest001, TestSize.Level1)
{
    int32_t netId = 1000;
    int32_t ret = instance_->NetworkDestroy(netId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeServiceTest, GetFwmarkForNetworkTest001, TestSize.Level1)
{
    int32_t netId = 1000;
    MarkMaskParcel markMaskParcel;
    int32_t ret = instance_->GetFwmarkForNetwork(netId, markMaskParcel);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceTest, SetInterfaceConfigTest001, TestSize.Level1)
{
    InterfaceConfigurationParcel cfg;
    cfg.ifName = "test0";
    cfg.hwAddr = "0b:2c:43:d7:22:1s";
    cfg.ipv4Addr = "192.168.133.12";
    cfg.prefixLength = 24;
    cfg.flags.push_back("up");
    int32_t ret = instance_->SetInterfaceConfig(cfg);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceTest, GetInterfaceConfigTest001, TestSize.Level1)
{
    InterfaceConfigurationParcel cfg;
    cfg.ifName = "eth0";
    int32_t ret = instance_->GetInterfaceConfig(cfg);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceTest, InterfaceGetListTest001, TestSize.Level1)
{
    std::vector<std::string> ifaces;
    int32_t ret = instance_->InterfaceGetList(ifaces);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceTest, StartDhcpClientTest001, TestSize.Level1)
{
    std::string iface = "test0";
    bool bIpv6 = false;
    int32_t ret = instance_->StartDhcpClient(iface, bIpv6);
    EXPECT_EQ(ret, ERR_NONE);

    ret = instance_->StopDhcpClient(iface, bIpv6);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceTest, StartDhcpServiceTest001, TestSize.Level1)
{
    std::string iface = "test0";
    std::string ipv4addr = "192.168.133.12";
    int32_t ret = instance_->StartDhcpService(iface, ipv4addr);
    EXPECT_EQ(ret, ERR_NONE);

    ret = instance_->StopDhcpService(iface);
    EXPECT_EQ(ret, ERR_NONE);
}

HWTEST_F(NetsysNativeServiceTest, IpEnableForwardingTest001, TestSize.Level1)
{
    std::string iface = "";
    std::string eth0 = "eth0";
    int32_t ret = instance_->IpEnableForwarding(iface);
    EXPECT_EQ(ret, 0);
    ret = instance_->IpDisableForwarding(iface);
    EXPECT_EQ(ret, 0);

    ret = instance_->EnableNat(eth0, eth0);
    EXPECT_NE(ret, 0);
    ret = instance_->DisableNat(eth0, eth0);
    EXPECT_NE(ret, 0);

    ret = instance_->IpfwdAddInterfaceForward(eth0, eth0);
    EXPECT_NE(ret, 0);
    ret = instance_->IpfwdRemoveInterfaceForward(eth0, eth0);
    EXPECT_NE(ret, 0);
    instance_->OnNetManagerRestart();
}
} // namespace NetsysNative
} // namespace OHOS
