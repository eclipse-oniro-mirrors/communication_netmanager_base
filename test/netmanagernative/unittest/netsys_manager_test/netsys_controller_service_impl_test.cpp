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

#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <iostream>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_conn_constants.h"
#include "net_manager_constants.h"
#include "net_stats_constants.h"
#include "netsys_controller.h"
#include "netsys_controller_service_impl.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetsysControllerServiceImplTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();

    static inline std::shared_ptr<NetsysControllerServiceImpl> instance_ = nullptr;
};

void NetsysControllerServiceImplTest::SetUpTestCase()
{
    instance_ = std::make_shared<NetsysControllerServiceImpl>();
    if (instance_) {
        instance_->mockNetsysClient_.mockApi_.clear();
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_NETWORKCREATEPHYSICAL_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_NETWORKDESTROY_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_NETWORKADDINTERFACE_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_NETWORKREMOVEINTERFACE_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_NETWORKADDROUTE_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_NETWORKREMOVEROUTE_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_SETINTERFACEDOWN_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_SETINTERFACEUP_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_INTERFACEGETMTU_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_INTERFACESETMTU_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_INTERFACEADDADDRESS_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_INTERFACEDELADDRESS_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_SETRESOLVERCONFIG_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_GETRESOLVERICONFIG_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_CREATENETWORKCACHE_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_SETDEFAULTNETWORK_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_CLEARDEFAULTNETWORK_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_STARTDHCPCLIENT_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_STOPDHCPCLIENT_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_REGISTERNOTIFYCALLBACK_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_STARTDHCPSERVICE_API);
        instance_->mockNetsysClient_.mockApi_.insert(MOCK_STOPDHCPSERVICE_API);
    }
}

void NetsysControllerServiceImplTest::TearDownTestCase() {}

void NetsysControllerServiceImplTest::SetUp() {}

void NetsysControllerServiceImplTest::TearDown() {}

HWTEST_F(NetsysControllerServiceImplTest, NoRegisterMockApi, TestSize.Level1)
{
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    std::string testName = "eth0";
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    sptr<NetsysControllerCallback> callback = nullptr;

    auto ret = instance_->NetworkCreatePhysical(0, 0);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->NetworkDestroy(0);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->NetworkAddInterface(0, testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->NetworkRemoveInterface(0, testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->NetworkAddRoute(0, testName, testName, testName);
    EXPECT_EQ(ret, -1);

    ret = instance_->NetworkRemoveRoute(0, testName, testName, testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->SetInterfaceDown(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->SetInterfaceUp(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetInterfaceMtu(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->SetInterfaceMtu(testName, 1);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->AddInterfaceAddress(testName, testName, 1);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->DelInterfaceAddress(testName, testName, 1);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->SetResolverConfig(0, 0, 0, servers, domains);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetResolverConfig(0, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->RegisterCallback(callback);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerServiceImplTest, RunRegisterMockApi, TestSize.Level1)
{
    std::string testName = "wlan0";

    auto ret = instance_->GetCellularRxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    instance_->ClearInterfaceAddrs(testName);
    ret = instance_->GetCellularTxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetAllRxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetAllTxBytes();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetUidRxBytes(20010038);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetUidTxBytes(20010038);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetIfaceRxBytes(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetIfaceTxBytes(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetIfaceRxPackets(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->GetIfaceTxPackets(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->CreateNetworkCache(0);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->SetDefaultNetWork(0);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->ClearDefaultNetWorkNetId();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->StartDhcpClient(testName, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->StopDhcpClient(testName, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->StartDhcpService(testName, testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->StopDhcpService(testName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    auto list = instance_->InterfaceGetList();
    EXPECT_GT(list.size(), static_cast<uint32_t>(0));

    list = instance_->UidGetList();
    EXPECT_EQ(list.size(), static_cast<uint32_t>(0));
}

HWTEST_F(NetsysControllerServiceImplTest, ServiceImplTest, TestSize.Level1)
{
    std::vector<UidRange> uidRanges;
    UidRange uidRang(1, 2);
    uidRanges.emplace_back(uidRang);
    int32_t ifaceFd = 5;
    std::string ipAddr = "172.17.5.245";
    NetsysNotifyCallback Callback;
    Callback.NetsysResponseInterfaceAdd = nullptr;
    Callback.NetsysResponseInterfaceRemoved = nullptr;

    auto ret = instance_->NetworkCreateVirtual(5, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->NetworkAddUids(5, uidRanges);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    ret = instance_->NetworkDelUids(5, uidRanges);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    auto ret32 = instance_->BindSocket(1, 2);
    EXPECT_EQ(ret32, NetManagerStandard::NETMANAGER_SUCCESS);

    ret32 = instance_->RegisterNetsysNotifyCallback(Callback);
    EXPECT_EQ(ret32, NetManagerStandard::NETMANAGER_SUCCESS);

    ret32 = instance_->BindNetworkServiceVpn(5);
    EXPECT_EQ(ret32, NetsysContrlResultCode::NETSYS_ERR_VPN);

    ifreq ifRequest;
    ret32 = instance_->EnableVirtualNetIfaceCard(5, ifRequest, ifaceFd);
    EXPECT_EQ(ret32, NetsysContrlResultCode::NETSYS_ERR_VPN);

    ret32 = instance_->SetIpAddress(5, ipAddr, 23, ifRequest);
    EXPECT_EQ(ret32, NetsysContrlResultCode::NETSYS_ERR_VPN);

    ret32 = instance_->SetBlocking(5, false);
    EXPECT_EQ(ret32, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerServiceImplTest, SetInternetPermission, TestSize.Level1)
{
    uint32_t uid = 0;
    uint8_t allow = 0;

    auto ret = instance_->SetInternetPermission(uid, allow);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

} // namespace NetManagerStandard
} // namespace OHOS
