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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "net_conn_service.h"
#include "net_conn_security.h"
#include "net_all_capabilities.h"
#include "iservice_registry.h"
#include "i_net_monitor_callback.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

HapInfoParams testInfoParms = {
    .userID = 1,
    .bundleName = "net_conn_hievent_test",
    .instIndex = 0,
    .appIDDesc = "test",
};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "net_conn_hievent_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect maneger HiSysEvent",
    .descriptionId = 1,
};

PermissionStateFull testState = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef},
    .permStateList = {testState},
};
} // namespace

class NetConnHiEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<Network> GetNetwork();
    sptr<NetLinkInfo> GetNetLinkInfo() const;
    void HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect);
    void HandleDetectionResult(uint32_t supplierId, bool ifValid);
};

void NetConnHiEventTest::SetUpTestCase() {}

void NetConnHiEventTest::TearDownTestCase() {}

void NetConnHiEventTest::SetUp() {}

void NetConnHiEventTest::TearDown() {}

sptr<Network> NetConnHiEventTest::GetNetwork()
{
    int32_t netId = 100;
    int32_t supplierId = 1001;
    sptr<Network> network = (std::make_unique<Network>(netId, supplierId,
                                                       std::bind(&NetConnHiEventTest::HandleDetectionResult, this,
                                                                 std::placeholders::_1, std::placeholders::_2),
                                                       BEARER_CELLULAR, nullptr))
                                .release();
    return network;
}

sptr<NetLinkInfo> NetConnHiEventTest::GetNetLinkInfo() const
{
    sptr<NetLinkInfo> netLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    netLinkInfo->ifaceName_ = "test";
    netLinkInfo->domain_ = "test";

    sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
    netAddr->type_ = INetAddr::IPV4;
    netAddr->family_ = 0x10;
    netAddr->prefixlen_ = 0x17;
    netAddr->address_ = "192.168.2.0";
    netAddr->netMask_ = "192.255.255.255";
    netAddr->hostName_ = "netAddr";
    netLinkInfo->netAddrList_.push_back(*netAddr);

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
    netLinkInfo->routeList_.push_back(*route);

    netLinkInfo->mtu_ = 0x5DC;
    return netLinkInfo;
}

void NetConnHiEventTest::HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect) {}
void NetConnHiEventTest::HandleDetectionResult(uint32_t supplierId, bool ifValid) {}

/**
 * @tc.name: NetConnHiEventTest_001
 * @tc.desc: Test NetConnManager HiSysEvent:UpdateNetSupplierInfo
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_001, TestSize.Level1)
{
    int32_t supplierId = 1001;
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->UpdateNetSupplierInfo(supplierId, nullptr);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetConnHiEventTest_002
 * @tc.desc: Test NetConnManager HiSysEvent:UpdateNetLinkInfo
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_002, TestSize.Level1)
{
    int32_t supplierId = 1001;
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->UpdateNetLinkInfo(supplierId, nullptr);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetConnHiEventTest_003
 * @tc.desc: Test NetConnManager HiSysEvent:UpdateInterfaces
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_003, TestSize.Level1)
{
    sptr<Network> network = GetNetwork();
    ASSERT_NE(network, nullptr);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    network->UpdateInterfaces(*netLinkInfo);
}

/**
 * @tc.name: NetConnHiEventTest_004
 * @tc.desc: Test NetConnManager HiSysEvent:UpdateIpAddrs
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_004, TestSize.Level1)
{
    sptr<Network> network = GetNetwork();
    ASSERT_NE(network, nullptr);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    network->UpdateIpAddrs(*netLinkInfo);
}

/**
 * @tc.name: NetConnHiEventTest_005
 * @tc.desc: Test NetConnManager HiSysEvent:UpdateRoutes
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_005, TestSize.Level1)
{
    sptr<Network> network = GetNetwork();
    ASSERT_NE(network, nullptr);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    network->UpdateRoutes(*netLinkInfo);
}

/**
 * @tc.name: NetConnHiEventTest_006
 * @tc.desc: Test NetConnManager HiSysEvent:UpdateDns
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_006, TestSize.Level1)
{
    sptr<Network> network = GetNetwork();
    ASSERT_NE(network, nullptr);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    network->UpdateDns(*netLinkInfo);
}

/**
 * @tc.name: NetConnHiEventTest_007
 * @tc.desc: Test NetConnManager HiSysEvent:UpdateMtu
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_007, TestSize.Level1)
{
    sptr<Network> network = GetNetwork();
    ASSERT_NE(network, nullptr);
    sptr<NetLinkInfo> netLinkInfo = GetNetLinkInfo();
    ASSERT_NE(netLinkInfo, nullptr);
    network->UpdateMtu(*netLinkInfo);
}

/**
 * @tc.name: NetConnHiEventTest_008
 * @tc.desc: Test NetConnManager HiSysEvent:SetSocketParameter
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_008, TestSize.Level1)
{
    int32_t netId = 1000;
    std::weak_ptr<INetMonitorCallback> callback;
    callback.reset();
    sptr<NetMonitor> netMonitor = new (std::nothrow) NetMonitor(netId, callback);
    int32_t ret = netMonitor->SetSocketParameter(-1);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: NetConnHiEventTest_009
 * @tc.desc: Test NetConnManager HiSysEvent:SetDefaultNetWork
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_009, TestSize.Level1)
{
    sptr<Network> network = GetNetwork();
    ASSERT_NE(network, nullptr);
    network->SetDefaultNetWork();
}

/**
 * @tc.name: NetConnHiEventTest_010
 * @tc.desc: Test NetConnManager HiSysEvent:ClearDefaultNetWorkNetId
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_010, TestSize.Level1)
{
    sptr<Network> network = GetNetwork();
    ASSERT_NE(network, nullptr);
    network->ClearDefaultNetWorkNetId();
}

/**
 * @tc.name: NetConnHiEventTest_011
 * @tc.desc: Test NetConnManager HiSysEvent:RegisterNetConnCallback
 * @tc.type: FUNC
 */
HWTEST_F(NetConnHiEventTest, NetConnHiEventTest_011, TestSize.Level1)
{
    OHOS::NetManagerStandard::AccessToken token(testInfoParms, testPolicyPrams);
    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->RegisterNetConnCallback(nullptr, nullptr, 0);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
