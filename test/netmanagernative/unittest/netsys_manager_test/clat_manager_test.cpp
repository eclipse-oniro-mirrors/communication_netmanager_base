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

#include "iservice_registry.h"
#include "system_ability_definition.h"

#ifdef GTEST_API_
#define private public
#endif

#include "clat_constants.h"
#include "clat_manager.h"
#include "net_conn_manager_test_util.h"
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_proxy.h"
#include "network_permission.h"
#include "physical_network.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace NetManagerStandard;
using namespace NetConnManagerTestUtil;

class ClatManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<ClatManager> instance_ = nullptr;
};

void ClatManagerTest::SetUpTestCase()
{
    instance_ = std::make_shared<ClatManager>();
}

void ClatManagerTest::TearDownTestCase()
{
    instance_ = nullptr;
}

void ClatManagerTest::SetUp() {}

void ClatManagerTest::TearDown() {}

/**
 * @tc.name: ClatStartTest001
 * @tc.desc: Test ConnManager ClatStart.
 * @tc.type: FUNC
 */
HWTEST_F(ClatManagerTest, ClatStartTest001, TestSize.Level1)
{
    const std::string v6Iface = "";
    int32_t netId = 1;
    const std::string nat64PrefixStr;
    NetManagerNative *netsysService = nullptr;

    int32_t ret = instance_->ClatStart(v6Iface, netId, nat64PrefixStr, netsysService);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
    netsysService = new NetManagerNative();
    ret = instance_->ClatStart(v6Iface, netId, nat64PrefixStr, netsysService);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: ClatStopTest001
 * @tc.desc: Test ConnManager ClatStop.
 * @tc.type: FUNC
 */
HWTEST_F(ClatManagerTest, ClatStopTest001, TestSize.Level1)
{
    const std::string v6Iface = "";
    NetManagerNative *netsysService = nullptr;
    netsysService = new NetManagerNative();
    int32_t ret = instance_->ClatStop(v6Iface, netsysService);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: GenerateClatSrcAddrTest001
 * @tc.desc: Test ConnManager GenerateClatSrcAddr.
 * @tc.type: FUNC
 */
HWTEST_F(ClatManagerTest, GenerateClatSrcAddrTest001, TestSize.Level1)
{
    const std::string v6Iface = "";
    const std::string nat64PrefixStr = "";
    uint32_t fwmark = 1;
    INetAddr v4Addr;
    INetAddr v6Addr;
    int32_t ret = instance_->GenerateClatSrcAddr(v6Iface, fwmark, nat64PrefixStr, v4Addr, v6Addr);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: CreateAndConfigureTunIfaceTest001
 * @tc.desc: Test ConnManager CreateAndConfigureTunIface.
 * @tc.type: FUNC
 */
HWTEST_F(ClatManagerTest, CreateAndConfigureTunIfaceTest001, TestSize.Level1)
{
    const std::string v6Iface = "";
    const std::string tunIface = "";
    INetAddr v4Addr;
    NetManagerNative *netsysService = nullptr;
    int tunFd = 1;

    int32_t ret = instance_->CreateAndConfigureTunIface(v6Iface, tunIface, v4Addr, netsysService, tunFd);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: CreateAndConfigureClatSocketTest001
 * @tc.desc: Test ConnManager CreateAndConfigureClatSocket.
 * @tc.type: FUNC
 */
HWTEST_F(ClatManagerTest, CreateAndConfigureClatSocketTest001, TestSize.Level1)
{
    const std::string v6Iface = "";
    uint32_t fwmark = 1;
    INetAddr v6Addr;
    int readSock6 = 1;
    int writeSock6 = 1;

    int32_t ret = instance_->CreateAndConfigureClatSocket(v6Iface, v6Addr, fwmark, readSock6, writeSock6);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

/**
 * @tc.name: AddNatBypassRules
 * @tc.desc: Test ConnManager AddNatBypassRules.
 * @tc.type: FUNC
 */
HWTEST_F(ClatManagerTest, AddNatBypassRulesTest001, TestSize.Level1)
{
    const std::string v6Iface = "rmnet0";
    const std::string v6Ip = "240e:46e:b900:27ab:1532:b318:192b:2841";
    int32_t ret = instance_->AddNatBypassRules(v6Iface, v6Ip);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.name: DeleteNatBypassRules
 * @tc.desc: Test ConnManager DeleteNatBypassRules.
 * @tc.type: FUNC
 */
HWTEST_F(ClatManagerTest, DeleteNatBypassRulesTest001, TestSize.Level1)
{
    const std::string v6Iface = "rmnet0";
    int32_t ret = instance_->DeleteNatBypassRules(v6Iface);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}
} // namespace NetsysNative
} // namespace OHOS