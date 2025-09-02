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

#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "netmanager_base_test_security.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "bpf_def.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "common_net_diag_callback_test.h"
#include "common_netsys_controller_callback_test.h"
#include "net_conn_constants.h"
#include "net_diag_callback_stub.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "netsys_controller.h"
#include "netsys_ipc_interface_code.h"
#include "netsys_net_diag_data.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetsysControllerTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();

    static inline std::shared_ptr<NetsysController> instance_ = nullptr;

    sptr<NetsysNative::NetDiagCallbackStubTest> netDiagCallback = new NetsysNative::NetDiagCallbackStubTest();
};

void NetsysControllerTest::SetUpTestCase()
{
    instance_ = std::make_shared<NetsysController>();
    instance_->Init();
}

void NetsysControllerTest::TearDownTestCase() {}

void NetsysControllerTest::SetUp() {}

void NetsysControllerTest::TearDown() {}

HWTEST_F(NetsysControllerTest, EnableDistributedClientNetTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    const std::string virnicAddr = "virnicAddr";
    const std::string iif = "iif";
    int32_t result = netsysController->EnableDistributedClientNet(virnicAddr, iif);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, EnableDistributedServerNetTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    const std::string iif = "iif";
    const std::string devIface = "devIface";
    const std::string dstAddr = "172.0.0.1";
    int32_t result = netsysController->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, DisableDistributedNetTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    bool isServer = true;
    int32_t result = netsysController->DisableDistributedNet(isServer);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetworkAddUidsTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    int32_t netId = 1;
    std::vector<int32_t> beginUids = {0, 1};
    std::vector<int32_t> endUids = {1};
    int32_t result = netsysController->NetworkAddUids(netId, beginUids, endUids);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, NetworkAddUidsTest002, TestSize.Level1)
{
    int32_t netId = 1;
    std::vector<int32_t> beginUids = {0, 1};
    std::vector<int32_t> endUids = {1};
    int32_t result = instance_->NetworkAddUids(netId, beginUids, endUids);
    EXPECT_EQ(result, NETMANAGER_ERR_INTERNAL);
    endUids.push_back(2); // endUids 2
    result = instance_->NetworkAddUids(netId, beginUids, endUids);
    EXPECT_EQ(result, ERR_NATIVESERVICE_NOTFIND);
}

HWTEST_F(NetsysControllerTest, NetworkDelUidsTest001, TestSize.Level1)
{
    int32_t netId = 1;
    std::vector<int32_t> beginUids = {0, 1};
    std::vector<int32_t> endUids = {1};
    int32_t result = instance_->NetworkDelUids(netId, beginUids, endUids);
    EXPECT_EQ(result, NETMANAGER_ERR_INTERNAL);

    endUids.push_back(2); // endUids 2
    result = instance_->NetworkDelUids(netId, beginUids, endUids);
    EXPECT_EQ(result, ERR_NATIVESERVICE_NOTFIND);
}

HWTEST_F(NetsysControllerTest, BindNetworkServiceVpnTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    int32_t socketFd = 0;
    int32_t result = netsysController->BindNetworkServiceVpn(socketFd);
    EXPECT_EQ(result, NETSYS_ERR_VPN);

    socketFd = 1;
    result = netsysController->BindNetworkServiceVpn(socketFd);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, BindNetworkServiceVpnTest002, TestSize.Level1)
{
    int32_t socketFd = 1;
    int32_t result = instance_->BindNetworkServiceVpn(socketFd);
    EXPECT_EQ(result, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, EnableVirtualNetIfaceCardTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    int32_t socketFd = 0;
    struct ifreq ifRequest = {};
    int32_t ifaceFd = 1;
    int32_t result = netsysController->EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
    EXPECT_EQ(result, NETSYS_ERR_VPN);

    socketFd = 1;
    result = netsysController->EnableVirtualNetIfaceCard(socketFd, ifRequest, ifaceFd);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, SetIpAddressTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    int32_t socketFd = 0;
    std::string ipAddress = "";
    int32_t prefixLen = 0;
    struct ifreq ifRequest = {};
    int32_t result = netsysController->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    EXPECT_EQ(result, NETSYS_ERR_VPN);

    socketFd = 1;
    result = netsysController->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    EXPECT_EQ(result, NETSYS_ERR_VPN);

    ipAddress = "172.0.0.1.172.0.0.1.172.0.0.1.172.0.0.1";
    result = netsysController->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    EXPECT_EQ(result, NETSYS_ERR_VPN);

    ipAddress = "172.0.0.1";
    result = netsysController->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    EXPECT_EQ(result, NETSYS_ERR_VPN);

    prefixLen = 35; // prefixLen: 35
    result = netsysController->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    EXPECT_EQ(result, NETSYS_ERR_VPN);

    prefixLen = 1;
    result = netsysController->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, SetIpAddressTest002, TestSize.Level1)
{
    int32_t socketFd = 1;
    std::string ipAddress = "172.0.0.1";
    int32_t prefixLen = 1;
    struct ifreq ifRequest = {};
    int32_t result = instance_->SetIpAddress(socketFd, ipAddress, prefixLen, ifRequest);
    EXPECT_EQ(result, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysControllerTest, SetIptablesCommandForResTest001, TestSize.Level1)
{
    std::string cmd = "";
    std::string respond = "";
    NetsysNative::IptablesType ipType = NetsysNative::IPTYPE_IPV4;
    int32_t result = instance_->SetIptablesCommandForRes(cmd, respond, ipType);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

HWTEST_F(NetsysControllerTest, SetIpCommandForResTest001, TestSize.Level1)
{
    std::string cmd = "";
    std::string respond = "";
    int32_t result = instance_->SetIpCommandForRes(cmd, respond);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

HWTEST_F(NetsysControllerTest, SetNicTrafficAllowedTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    const std::vector<std::string> ifaceNames = {"ifaceNames"};
    bool status = true;
    int32_t result = netsysController->SetNicTrafficAllowed(ifaceNames, status);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, CloseSocketsUidTest001, TestSize.Level1)
{
    auto netsysController = std::make_shared<NetsysController>();
    netsysController->netsysService_ = nullptr;
    const std::string ipAddr = "172.0.0.1";
    uint32_t uid = 1;
    int32_t result = netsysController->CloseSocketsUid(ipAddr, uid);
    EXPECT_EQ(result, NETSYS_NETSYSSERVICE_NULL);
}

HWTEST_F(NetsysControllerTest, CloseSocketsUidTest002, TestSize.Level1)
{
    const std::string ipAddr = "172.0.0.1";
    uint32_t uid = 1;
    int32_t result = instance_->CloseSocketsUid(ipAddr, uid);
    EXPECT_EQ(result, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
