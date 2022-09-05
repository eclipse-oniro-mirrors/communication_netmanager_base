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
#include <iostream>
#include <string>
#include <unistd.h>

#include "net_conn_service.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const int32_t TEST_UPDATE_NULL_NET_SUPPLIER_INFO = 1;
const int32_t TEST_UPDATE_NULL_NET_LINK_INFO = 2;
const int32_t TEST_UPDATE_FAULT_NET_INTERFACES = 3;
const int32_t TEST_UPDATE_FAULT_NET_IP_ADDR = 4;
const int32_t TEST_UPDATE_FAULT_NET_ROUTES = 5;
const int32_t TEST_UPDATE_FAULT_NET_DNSES = 6;
const int32_t TEST_UPDATE_FAULT_NET_MTU = 7;
const int32_t TEST_BIND_FAULT_NET_SOCKET = 8;
const int32_t TEST_SET_FAULT_DEFAULT_BETWORK = 9;
const int32_t TEST_CLEAR_FAULT_DEFAULT_BETWORK = 10;
const int32_t TEST_REGISTER_NULL_NET_REQUEST = 11;
} // namespace

using namespace testing::ext;
class NetConnHiEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    void Help();
    void Init();
    void SwitchCase(int32_t case_in);
    void UpdateNullNetSupplierInfo();
    void UpdateNullNetLinkInfo();
    void UpdateFaultInterfaces();
    void UpdateFaultIpAddrs();
    void UpdateFaultRoutes();
    void UpdateFaultDnses();
    void UpdateFaultMtu();
    void BindFaultSocket();
    void SetFaultDefaultNetWork();
    void ClearFaultDefaultNetWork();
    void RegisterNullNetConnCallback();

private:
    void SetNetLinkInfo();
    void HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect);
    void HandleDetectionResult(uint32_t supplierId, bool ifValid);

    // sptr<NetConnService> netConnService_;
    sptr<Network> network_;
    sptr<NetLinkInfo> netLinkInfo_;

    int32_t netId_;
    int32_t supplierId_;
};

void NetConnHiEventTest::SetUpTestCase() {}

void NetConnHiEventTest::TearDownTestCase() {}

void NetConnHiEventTest::SetUp() {}

void NetConnHiEventTest::TearDown() {}

void NetConnHiEventTest::Init()
{
    netId_ = 100;
    supplierId_ = 1001;
    // netConnService_ = std::make_unique<NetConnService>().release();
    network_ = (std::make_unique<Network>(netId_, supplierId_,
        std::bind(&NetConnHiEventTest::HandleDetectionResult, this, std::placeholders::_1, std::placeholders::_2))).release();
    if (network_ == nullptr) {
        std::cout << "network_ is nullptr" << std::endl;
    }

    SetNetLinkInfo();
}

void NetConnHiEventTest::UpdateNullNetSupplierInfo()
{
    DelayedSingleton<NetConnService>::GetInstance()->UpdateNetSupplierInfo(supplierId_, nullptr);
}

void NetConnHiEventTest::UpdateNullNetLinkInfo()
{
    DelayedSingleton<NetConnService>::GetInstance()->UpdateNetLinkInfo(supplierId_, nullptr);
}

void NetConnHiEventTest::UpdateFaultInterfaces()
{
    network_->UpdateInterfaces(*netLinkInfo_);
}

void NetConnHiEventTest::UpdateFaultIpAddrs()
{
    network_->UpdateIpAddrs(*netLinkInfo_);
}

void NetConnHiEventTest::UpdateFaultRoutes()
{
    network_->UpdateRoutes(*netLinkInfo_);
}

void NetConnHiEventTest::UpdateFaultDnses()
{
    network_->UpdateDnses(*netLinkInfo_);
}

void NetConnHiEventTest::UpdateFaultMtu()
{
    network_->UpdateMtu(*netLinkInfo_);
}

void NetConnHiEventTest::BindFaultSocket()
{
    std::unique_ptr<NetMonitor> netMonitor = std::make_unique<NetMonitor>(netId_,
        std::bind(&NetConnHiEventTest::HandleNetMonitorResult, this, std::placeholders::_1, std::placeholders::_2));
    if (netMonitor == nullptr) {
        std::cout << "netMonitor is nullptr" << std::endl;
        return;
    }
    netMonitor->SetSocketParameter(-1);
}

void NetConnHiEventTest::SetFaultDefaultNetWork()
{
    network_->SetDefaultNetWork();
}

void NetConnHiEventTest::ClearFaultDefaultNetWork()
{
    network_->ClearDefaultNetWorkNetId();
}

void NetConnHiEventTest::RegisterNullNetConnCallback()
{
    DelayedSingleton<NetConnService>::GetInstance()->RegisterNetConnCallback(nullptr, nullptr, 0);
}

void NetConnHiEventTest::SetNetLinkInfo()
{
    netLinkInfo_ = (std::make_unique<NetLinkInfo>()).release();
    netLinkInfo_->ifaceName_ = "test";
    netLinkInfo_->domain_ = "test";

    sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
    netAddr->type_ = INetAddr::IPV4;
    netAddr->family_ = 0x10;
    netAddr->prefixlen_ = 0x17;
    netAddr->address_ = "192.168.2.0";
    netAddr->netMask_ = "192.255.255.255";
    netAddr->hostName_ = "netAddr";
    netLinkInfo_->netAddrList_.push_back(*netAddr);

    sptr<INetAddr> dns = (std::make_unique<INetAddr>()).release();
    dns->type_ = INetAddr::IPV4;
    dns->family_ = 0x10;
    dns->prefixlen_ = 0x17;
    dns->address_ = "192.168.2.0";
    dns->netMask_ = "192.255.255.255";
    dns->hostName_ = "netAddr";
    netLinkInfo_->dnsList_.push_back(*dns);

    sptr<Route> route = (std::make_unique<Route>()).release();
    route->iface_ = "iface0";
    route->destination_.type_ = INetAddr::IPV4;
    route->destination_.family_ = 0x10;
    route->destination_.prefixlen_ = 0x17;
    route->destination_.address_ = "192.168.2.0";
    route->destination_.netMask_ = "192.255.255.255";
    route->destination_.hostName_ = "netAddr";
    route->gateway_.type_ = INetAddr::IPV4;
    route->gateway_.family_ = 0x10;
    route->gateway_.prefixlen_ = 0x17;
    route->gateway_.address_ = "192.168.2.0";
    route->gateway_.netMask_ = "192.255.255.255";
    route->gateway_.hostName_ = "netAddr";
    netLinkInfo_->routeList_.push_back(*route);

    netLinkInfo_->mtu_ = 0x5DC;
}

void NetConnHiEventTest::HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect) {}
void NetConnHiEventTest::HandleDetectionResult(uint32_t supplierId, bool ifValid) {}

void NetConnHiEventTest::Help()
{
    std::cout << "************************************************************" << std::endl;
    std::cout << "Welcome to the net_conn_manager hisysevent test demo!" << std::endl;
    std::cout << "1 ::Stop hisysevent test " << std::endl;
    std::cout << "1 ::UpdateNullNetSupplierInfo " << std::endl;
    std::cout << "2 ::UpdateNullNetLinkInfo " << std::endl;
    std::cout << "3 ::UpdateFaultInterfaces " << std::endl;
    std::cout << "4 ::UpdateFaultIpAddrs " << std::endl;
    std::cout << "5 ::UpdateFaultRoutes " << std::endl;
    std::cout << "6 ::UpdateFaultDnses " << std::endl;
    std::cout << "7 ::UpdateFaultMtu " << std::endl;
    std::cout << "8 ::BindFaultSocket " << std::endl;
    std::cout << "9 ::SetFaultDefaultNetWork " << std::endl;
    std::cout << "10 ::ClearFaultDefaultNetWork " << std::endl;
    std::cout << "11::RegisterNullNetConnCallback " << std::endl;
    std::cout << "************************************************************" << std::endl;
}

void NetConnHiEventTest::SwitchCase(int32_t case_in)
{
    switch (case_in) {
        case TEST_UPDATE_NULL_NET_SUPPLIER_INFO:
            UpdateNullNetSupplierInfo();
            break;
        case TEST_UPDATE_NULL_NET_LINK_INFO:
            UpdateNullNetLinkInfo();
            break;
        case TEST_UPDATE_FAULT_NET_INTERFACES:
            UpdateFaultInterfaces();
            break;
        case TEST_UPDATE_FAULT_NET_IP_ADDR:
            UpdateFaultIpAddrs();
            break;
        case TEST_UPDATE_FAULT_NET_ROUTES:
            UpdateFaultRoutes();
            break;
        case TEST_UPDATE_FAULT_NET_DNSES:
            UpdateFaultDnses();
            break;
        case TEST_UPDATE_FAULT_NET_MTU:
            UpdateFaultMtu();
            break;
        case TEST_BIND_FAULT_NET_SOCKET:
            BindFaultSocket();
            break;
        case TEST_SET_FAULT_DEFAULT_BETWORK:
            SetFaultDefaultNetWork();
            break;
        case TEST_CLEAR_FAULT_DEFAULT_BETWORK:
            ClearFaultDefaultNetWork();
            break;
        case TEST_REGISTER_NULL_NET_REQUEST:
            RegisterNullNetConnCallback();
            break;
        default:
            std::cout << "Unknown input case, please re-enter!" << std::endl;
            break;
    }
}

/**
 * @tc.name: NetConnHiEventTest_001
 * @tc.desc: Test NetConnManager fault HiSysEvent
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_001, TestSize.Level1)
{
    std::cout << "NetConnHiEventTest::Start " << std::endl;
    Help();
    Init();
    int32_t in;
    while (true) {
        std::cout << std::endl;
        std::cout << "Enter the case No. : " << std::endl;
        std::cin >> in;
        if (in == 0) {
            break;
        }
        SwitchCase(in);
    }
}
} // NetManagerStandard
} // OHOS