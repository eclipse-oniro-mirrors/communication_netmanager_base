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
#include <memory>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_supplier.h"
#include "common_net_conn_callback_test.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t TEST_NETID = 12;
constexpr uint32_t TEST_SUPPLIERID = 214;
constexpr const char *TEST_IDENT = "testIdent";
}

class NetSupplierTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline sptr<NetSupplier> supplier = nullptr;
};

void NetSupplierTest::SetUpTestCase()
{
    std::set<NetCap> netCaps;
    netCaps.insert(NET_CAPABILITY_INTERNET);
    supplier = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps);
}

void NetSupplierTest::TearDownTestCase() {}

void NetSupplierTest::SetUp() {}

void NetSupplierTest::TearDown() {}

HWTEST_F(NetSupplierTest, GetSupplierCallbackTest001, TestSize.Level1)
{
    sptr<INetSupplierCallback> callBack = supplier->GetSupplierCallback();
    EXPECT_TRUE(callBack == nullptr);
}

HWTEST_F(NetSupplierTest, GetSupplierCallbackTest002, TestSize.Level1)
{
    sptr<INetSupplierCallback> callback = new (std::nothrow) NetSupplierCallbackStubTestCb();
    ASSERT_NE(callback, nullptr);
    supplier->RegisterSupplierCallback(callback);
    ASSERT_NE(supplier->GetSupplierCallback(), nullptr);
}

HWTEST_F(NetSupplierTest, UpdateNetSupplierInfoTest001, TestSize.Level1)
{
    NetSupplierInfo netSupplierInfo{};
    netSupplierInfo.isAvailable_ = false;
    netSupplierInfo.ident_ = "ident_";
    netSupplierInfo.score_ = 1;
    supplier->network_ = nullptr;
    NetDetectionHandler detectionHandler = [](uint32_t supplierId, bool ifValid) {};
    std::shared_ptr<Network> network = std::make_shared<Network>(TEST_NETID, TEST_SUPPLIERID,
        detectionHandler, NetBearType::BEARER_ETHERNET, nullptr);

    supplier->UpdateNetSupplierInfo(netSupplierInfo);
    EXPECT_FALSE(supplier->netSupplierInfo_.ident_.empty());
    EXPECT_TRUE(supplier->netScore_ == 1);
    EXPECT_FALSE(supplier->netSupplierInfo_.isAvailable_);
    netSupplierInfo.isAvailable_ = true;
    supplier->UpdateNetSupplierInfo(netSupplierInfo);
    EXPECT_TRUE(supplier->netSupplierInfo_.isAvailable_);
    EXPECT_TRUE(supplier->network_ == nullptr);
    EXPECT_TRUE(supplier->netSupplierInfo_.isAvailable_);

    supplier->network_ = network;
    supplier->UpdateNetSupplierInfo(netSupplierInfo);
    EXPECT_TRUE(supplier->network_ != nullptr);
}

HWTEST_F(NetSupplierTest, UpdateNetLinkInfoTest001, TestSize.Level1)
{
    NetLinkInfo netLinkInfo{};
    supplier->netSupplierIdent_ = "simId";

    int32_t ret = supplier->UpdateNetLinkInfo(netLinkInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetSupplierTest, SupplierConnectionTest001, TestSize.Level1)
{
    NetLinkInfo netLinkInfo{};
    std::set<NetCap> netCaps;
    NetRequest netRequest;
    supplier->netSupplierInfo_.isAvailable_ = true;
    supplier->netSupplierIdent_ = "Supplier";
    bool ret = supplier->SupplierConnection(netCaps, netRequest);
    EXPECT_TRUE(ret);

    supplier->netSupplierInfo_.isAvailable_ = false;
    supplier->netSupplierIdent_ = "simId";
    sptr<INetSupplierCallback> callback;
    supplier->netController_ = callback;
    ret = supplier->SupplierConnection(netCaps, netRequest);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetSupplierTest, SupplierDisconnectionTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    supplier->netSupplierInfo_.isAvailable_ = true;
    supplier->netSupplierIdent_ = "simId";
    sptr<INetSupplierCallback> callback;
    supplier->netController_ = callback;
    bool ret = supplier->SupplierDisconnection(netCaps);
    EXPECT_FALSE(ret);

    supplier->netSupplierInfo_.isAvailable_ = false;
    netCaps.insert(NetCap::NET_CAPABILITY_NOT_METERED);
    ret = supplier->SupplierDisconnection(netCaps);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetSupplierTest, IsConnectedTest001, TestSize.Level1)
{
    supplier->network_ = nullptr;
    bool ret = supplier->IsConnected();
    EXPECT_FALSE(ret);
}

HWTEST_F(NetSupplierTest, ReceiveBestScoreTest001, TestSize.Level1)
{
    int32_t bestScore = 1;
    uint32_t supplierId = 0;
    supplier->supplierId_ = 0;
    NetRequest netrequest;
    supplier->InitNetScore();
    supplier->ReceiveBestScore(bestScore, supplierId, netrequest);
    EXPECT_TRUE(supplierId == supplier->supplierId_);

    supplierId = 1;
    supplier->requestList_.insert(1);
    supplier->ReceiveBestScore(bestScore, supplierId, netrequest);
    EXPECT_FALSE(supplier->requestList_.empty());
}

HWTEST_F(NetSupplierTest, SetNetValidTest001, TestSize.Level1)
{
    NetDetectionStatus netState = CAPTIVE_PORTAL_STATE;
    supplier->netCaps_.InsertNetCap(NET_CAPABILITY_VALIDATED);
    supplier->SetNetValid(netState);
    EXPECT_FALSE(supplier->HasNetCap(NET_CAPABILITY_VALIDATED));

    netState = INVALID_DETECTION_STATE;
    supplier->netCaps_.InsertNetCap(NET_CAPABILITY_PORTAL);
    supplier->SetNetValid(netState);
    EXPECT_FALSE(supplier->HasNetCap(NET_CAPABILITY_PORTAL));
}

HWTEST_F(NetSupplierTest, SupplierTypeTest001, TestSize.Level1)
{
    int32_t type = 10; // SLOT_TYPE_LTE_CA = 10
    supplier->SetSupplierType(type);
    EXPECT_EQ(supplier->type_, "4G");
    std::string ret = supplier->GetSupplierType();
    EXPECT_EQ(ret, "4G");
}

HWTEST_F(NetSupplierTest, SetDefaultTest001, TestSize.Level1)
{
    std::shared_ptr<Network> network = nullptr;
    supplier->SetNetwork(network);
    supplier->SetDefault();
    EXPECT_TRUE(supplier->network_ == nullptr);
}

HWTEST_F(NetSupplierTest, InitNetScoreTest001, TestSize.Level1)
{
    supplier->netSupplierType_ = BEARER_DEFAULT;
    auto iter = netTypeScore_.find(supplier->netSupplierType_);
    supplier->InitNetScore();
    EXPECT_TRUE(iter == netTypeScore_.end());
}

HWTEST_F(NetSupplierTest, NetSupplieroperatorTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    netCaps.insert(NET_CAPABILITY_INTERNET);
    std::string netSupplierIdent = "netSupplierIdent";
    NetSupplier netSupplier1(BEARER_CELLULAR, netSupplierIdent, netCaps);
    NetSupplier netSupplier2 = netSupplier1;
    EXPECT_TRUE(netSupplier1 == netSupplier2);
    netSupplier2.netSupplierType_ = BEARER_BLUETOOTH;
    EXPECT_FALSE(netSupplier1 == netSupplier2);
}

HWTEST_F(NetSupplierTest, RemoveBestRequestTest001, TestSize.Level1)
{
    uint32_t reqId = 1;
    supplier->bestReqList_.insert(reqId);
    auto iter1 = supplier->bestReqList_.find(reqId);
    EXPECT_TRUE(iter1 != supplier->bestReqList_.end());
    supplier->RemoveBestRequest(reqId);
    auto iter2 = supplier->bestReqList_.find(reqId);
    EXPECT_TRUE(iter2 == supplier->bestReqList_.end());
}
} // namespace NetManagerStandard
} // namespace OHOS
