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
#define protected public
#endif

#include "net_supplier.h"
#include <gtest/gtest.h>
#include <memory>


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


HWTEST_F(NetSupplierTest, ResumeNetworkInfoTest001, TestSize.Level1)
{
    bool ret = supplier->ResumeNetworkInfo();
    EXPECT_FALSE(ret);

    NetDetectionHandler detectionHandler = [](uint32_t supplierId, bool ifValid) {
        std::cout << "supplierId:" << supplierId;
        std::cout << " IfValid:" << ifValid << std::endl;
    };

    std::shared_ptr<Network> network = std::make_shared<Network>(TEST_NETID, TEST_SUPPLIERID, detectionHandler,
        NetBearType::BEARER_ETHERNET, nullptr);
    supplier->SetNetwork(network);
    ret = supplier->ResumeNetworkInfo();
    EXPECT_TRUE(ret);

    supplier->ClearDefault();

    ret = supplier->IsConnecting();
    EXPECT_FALSE(ret);

    ret = supplier->IsConnected();
    EXPECT_FALSE(ret);

    std::string result = supplier->TechToType(NetSlotTech::SLOT_TYPE_GSM);
    EXPECT_TRUE(result == "2G");

    result = supplier->TechToType(NetSlotTech::SLOT_TYPE_LTE);
    EXPECT_TRUE(result == "4G");

    result = supplier->TechToType(NetSlotTech::SLOT_TYPE_LTE_CA);
    EXPECT_TRUE(result == "4G");

    uint32_t invalidValue = 100;
    result = supplier->TechToType(static_cast<NetSlotTech>(invalidValue));
    EXPECT_TRUE(result == "3G");
}
} // namespace NetManagerStandard
} // namespace OHOS
