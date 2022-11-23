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
#include <memory>

#include "net_activate.h"
#include "net_conn_callback_stub.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t TEST_TIMEOUT_MS = 1000;
constexpr uint32_t TEST_REQUEST_ID = 54656;
constexpr const char *TEST_IDENT = "testIdent";
class ConnCallbackTest : public NetConnCallbackStub {
    inline int32_t NetAvailable(sptr<NetHandle> &netHandle) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t NetCapabilitiesChange(sptr<NetHandle> &netHandle,
                                         const sptr<NetAllCapabilities> &netAllCap) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t NetLost(sptr<NetHandle> &netHandle) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t NetUnavailable() override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked) override
    {
        return NETMANAGER_SUCCESS;
    }
};

class NetActivateCallbackTest : public INetActivateCallback {
    void OnNetActivateTimeOut(uint32_t reqId) override
    {
        std::cout << "Activate network request " << reqId << " timeout." << std::endl;
    }
};
} // namespace

class NetActivateTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::unique_ptr<NetActivate> instance_ = nullptr;
    static inline sptr<INetConnCallback> callback_ = nullptr;
    static inline sptr<NetSpecifier> specifier_ = nullptr;
    static inline std::shared_ptr<INetActivateCallback> timeoutCallback_ = nullptr;
};

void NetActivateTest::SetUpTestCase()
{
    callback_ = new (std::nothrow) ConnCallbackTest();
    specifier_ = new (std::nothrow) NetSpecifier();
    timeoutCallback_ = std::make_shared<NetActivateCallbackTest>();
    instance_ = std::make_unique<NetActivate>(specifier_, callback_, timeoutCallback_, TEST_TIMEOUT_MS);
}

void NetActivateTest::TearDownTestCase() {}

void NetActivateTest::SetUp() {}

void NetActivateTest::TearDown() {}

HWTEST_F(NetActivateTest, MatchRequestAndNetworkTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps);
    bool ret = instance_->MatchRequestAndNetwork(supplier);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetActivateTest, SetGetRequestIdTest001, TestSize.Level1)
{
    instance_->SetRequestId(TEST_REQUEST_ID);
    uint32_t requestId = instance_->GetRequestId();
    EXPECT_EQ(requestId, TEST_REQUEST_ID);
}

HWTEST_F(NetActivateTest, SetGetServiceSupplyTest001, TestSize.Level1)
{
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new (std::nothrow) NetSupplier(NetBearType::BEARER_ETHERNET, TEST_IDENT, netCaps);
    instance_->SetServiceSupply(supplier);
    auto result = instance_->GetServiceSupply();
    EXPECT_EQ(result, supplier);
}

HWTEST_F(NetActivateTest, GetNetCallbackTest001, TestSize.Level1)
{
    auto result = instance_->GetNetCallback();
    EXPECT_EQ(result, callback_);
}

HWTEST_F(NetActivateTest, GetNetSpecifierTest001, TestSize.Level1)
{
    auto result = instance_->GetNetSpecifier();
    EXPECT_EQ(result, specifier_);
}
} // namespace NetManagerStandard
} // namespace OHOS