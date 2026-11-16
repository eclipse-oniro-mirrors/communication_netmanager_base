/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "common_net_conn_callback_test.h"
#include "http_proxy.h"
#include "net_all_capabilities.h"
#include "net_conn_callback_stub.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_conn_service.h"
#include "net_conn_types.h"
#include "net_factoryreset_callback_stub.h"
#include "net_interface_callback_stub.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "netsys_controller.h"
#include "parameter.h"
#include "parameters.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetConnServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetConnServiceTest::SetUpTestCase()
{
    NetConnService::GetInstance()->OnStart();
}

void NetConnServiceTest::TearDownTestCase() {}

void NetConnServiceTest::SetUp() {}

void NetConnServiceTest::TearDown() {}

HWTEST_F(NetConnServiceTest, EnableDistributedClientNet001, TestSize.Level1)
{
    std::string virnicAddr = "1.189.55.61";
    std::string virnicName = "virnic";
    std::string iif = "lo";
    int32_t ret = NetConnService::GetInstance()->EnableDistributedClientNet(virnicAddr, virnicName, iif);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    bool isServer = false;
    std::string dstAddr = "1.1.1.1";
    ret = NetConnService::GetInstance()->DisableDistributedNet(isServer, virnicName, dstAddr);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, EnableDistributedServerNet001, TestSize.Level1)
{
    std::string iif = "lo";
    std::string devIface = "lo";
    std::string dstAddr = "1.189.55.61";
    std::string gw = "0.0.0.0";
    int32_t ret = NetConnService::GetInstance()->EnableDistributedServerNet(iif, devIface, dstAddr, gw);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    bool isServer = true;
    std::string virnicName = "virnic";
    ret = NetConnService::GetInstance()->DisableDistributedNet(isServer, virnicName, dstAddr);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceTest, ActiveHttpProxy001, TestSize.Level1)
{
    NetConnService::GetInstance()->ActiveHttpProxy();
    EXPECT_FALSE(NetConnService::GetInstance()->isFallbackProbeWithProxy_);
}

HWTEST_F(NetConnServiceTest, FindBestNetworkForRequestDelay001, TestSize.Level1)
{
    auto netConnService = std::make_shared<NetConnService>();
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    auto netSupplier = sptr<NetSupplier>::MakeSptr(NetBearType::BEARER_WIFI, "delayTest", netCaps);
    netSupplier->supplierId_ = 1;
    auto network = std::make_shared<Network>(100, 1, NetBearType::BEARER_WIFI, nullptr);
    network->UpdateNetConnState(NET_CONN_STATE_CONNECTED);
    netSupplier->SetNetwork(network);
    netConnService->netSuppliers_.emplace(1, netSupplier);
    auto netSpecifier = sptr<NetSpecifier>::MakeSptr();
    netSpecifier->SetCapability(NET_CAPABILITY_INTERNET);
    auto callback = sptr<NetConnCallbackStubCb>::MakeSptr();
    auto active = netConnService->CreateNetActivateRequest(netSpecifier, callback, 0, 0, 1099);
    sptr<NetSupplier> bestSupplier = nullptr;
    netConnService->isDelayHandleFindBestNetwork_ = true;
    netConnService->delaySupplierId_ = 1;
    netConnService->FindBestNetworkForRequest(bestSupplier, active);
    EXPECT_EQ(bestSupplier, nullptr);
    netConnService->isDelayHandleFindBestNetwork_ = true;
    netConnService->delaySupplierId_ = 999;
    netConnService->FindBestNetworkForRequest(bestSupplier, active);
    EXPECT_EQ(bestSupplier, netSupplier);
    netConnService->isDelayHandleFindBestNetwork_ = false;
    netConnService->delaySupplierId_ = 1;
    netConnService->FindBestNetworkForRequest(bestSupplier, active);
    EXPECT_EQ(bestSupplier, netSupplier);
    netConnService->netSuppliers_.clear();
}

HWTEST_F(NetConnServiceTest, FindBestNetworkForRequestDelay002, TestSize.Level1)
{
    auto netConnService = std::make_shared<NetConnService>();
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    auto delaySupplier = sptr<NetSupplier>::MakeSptr(NetBearType::BEARER_WIFI, "delayTest", netCaps);
    delaySupplier->supplierId_ = 1;
    auto delayNetwork = std::make_shared<Network>(100, 1, NetBearType::BEARER_WIFI, nullptr);
    delayNetwork->UpdateNetConnState(NET_CONN_STATE_CONNECTED);
    delaySupplier->SetNetwork(delayNetwork);
    auto normalSupplier = sptr<NetSupplier>::MakeSptr(NetBearType::BEARER_CELLULAR, "normalTest", netCaps);
    normalSupplier->supplierId_ = 2;
    auto normalNetwork = std::make_shared<Network>(101, 2, NetBearType::BEARER_CELLULAR, nullptr);
    normalNetwork->UpdateNetConnState(NET_CONN_STATE_CONNECTED);
    normalSupplier->SetNetwork(normalNetwork);
    netConnService->netSuppliers_.emplace(1, delaySupplier);
    netConnService->netSuppliers_.emplace(2, normalSupplier);
    auto netSpecifier = sptr<NetSpecifier>::MakeSptr();
    netSpecifier->SetCapability(NET_CAPABILITY_INTERNET);
    auto callback = sptr<NetConnCallbackStubCb>::MakeSptr();
    auto active = netConnService->CreateNetActivateRequest(netSpecifier, callback, 0, 0, 1099);
    netConnService->isDelayHandleFindBestNetwork_ = true;
    netConnService->delaySupplierId_ = 1;
    sptr<NetSupplier> bestSupplier = nullptr;
    netConnService->FindBestNetworkForRequest(bestSupplier, active);
    EXPECT_EQ(bestSupplier, normalSupplier);
    netConnService->netSuppliers_.clear();
}

HWTEST_F(NetConnServiceTest, HandleDetectionResultDelay001, TestSize.Level1)
{
    auto netConnService = std::make_shared<NetConnService>();
    std::string ident;
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET};
    netConnService->defaultNetSupplier_ = new NetSupplier(NetBearType::BEARER_CELLULAR, ident, netCaps);
    auto supplier = sptr<NetSupplier>::MakeSptr(NetBearType::BEARER_WIFI, ident, netCaps);
    supplier->supplierId_ = 1;
    netConnService->netSuppliers_.emplace(1, supplier);
    netConnService->netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnService->netConnEventRunner_);
    OHOS::system::SetParameter("persist.booster.enable_wifi_delay_weak_signal", "true");
    netConnService->HandleDetectionResult(1, VERIFICATION_STATE);
    EXPECT_TRUE(netConnService->isDelayHandleFindBestNetwork_);
    EXPECT_EQ(netConnService->delaySupplierId_.load(), 1U);
    netConnService->isDelayHandleFindBestNetwork_ = false;
    netConnService->delaySupplierId_ = 0;
    netConnService->HandleDetectionResult(1, VERIFICATION_STATE);
    EXPECT_FALSE(netConnService->isDelayHandleFindBestNetwork_);
    supplier->SetOnceSuppress();
    netConnService->isDelayHandleFindBestNetwork_ = false;
    netConnService->HandleDetectionResult(1, VERIFICATION_STATE);
    EXPECT_FALSE(netConnService->isDelayHandleFindBestNetwork_);
    OHOS::system::SetParameter("persist.booster.enable_wifi_delay_weak_signal", "false");
    netConnService->netSuppliers_.clear();
}
} // namespace NetManagerStandard
} // namespace OHOS
