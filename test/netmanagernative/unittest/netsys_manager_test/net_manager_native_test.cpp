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

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "net_manager_native.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
}

class NetManagerNativeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetManagerNative> instance_ = nullptr;
};

void NetManagerNativeTest::SetUpTestCase()
{
    instance_ = std::make_shared<NetManagerNative>();
}

void NetManagerNativeTest::TearDownTestCase() {}

void NetManagerNativeTest::SetUp() {}

void NetManagerNativeTest::TearDown() {}

HWTEST_F(NetManagerNativeTest, UpdateInterfaceIndex001, TestSize.Level1)
{
    uint32_t interfaceIndex = 10090;
    instance_->UpdateInterfaceIndex(interfaceIndex);
    auto result = instance_->GetCurrentInterfaceIndex();
    auto findResult = std::find(result.begin(), result.end(), interfaceIndex);
    EXPECT_NE(findResult, result.end());
}

HWTEST_F(NetManagerNativeTest, SetInternetPermission001, TestSize.Level1)
{
    uint32_t uid = 0;
    uint8_t allow = 1;
    uint8_t isBroker = 0;
    auto ret = instance_->SetInternetPermission(uid, allow, isBroker);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerNativeTest, NetworkCreateVirtual001, TestSize.Level1)
{
    int32_t netid = 12235;
    bool hasDns = false;
    auto ret = instance_->NetworkCreateVirtual(netid, hasDns);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerNativeTest, NetworkAddUids001, TestSize.Level1)
{
    int32_t netId = 12235;
    std::vector<UidRange> uidRanges;
    auto ret = instance_->NetworkAddUids(netId, uidRanges);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, NetworkDelUids001, TestSize.Level1)
{
    int32_t netId = 12235;
    std::vector<UidRange> uidRanges;
    auto ret = instance_->NetworkDelUids(netId, uidRanges);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, NetworkSetPermissionForNetwork001, TestSize.Level1)
{
    int32_t netId = 12235;
    auto ret = instance_->NetworkSetPermissionForNetwork(netId, NetworkPermission::PERMISSION_NONE);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerNativeTest, GetCellularRxBytes001, TestSize.Level1)
{
    auto ret = instance_->GetCellularRxBytes();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, GetCellularTxBytes001, TestSize.Level1)
{
    auto ret = instance_->GetCellularTxBytes();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, GetAllRxBytes001, TestSize.Level1)
{
    auto ret = instance_->GetAllRxBytes();
    EXPECT_GE(ret, 0);
}

HWTEST_F(NetManagerNativeTest, GetAllTxBytes001, TestSize.Level1)
{
    auto ret = instance_->GetAllTxBytes();
    EXPECT_GE(ret, 0);
}

HWTEST_F(NetManagerNativeTest, GetUidTxBytes001, TestSize.Level1)
{
    int32_t uid = 10012;
    auto ret = instance_->GetUidTxBytes(uid);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, GetUidRxBytes001, TestSize.Level1)
{
    int32_t uid = 10012;
    auto ret = instance_->GetUidRxBytes(uid);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, GetIfaceRxBytes001, TestSize.Level1)
{
    std::string testIface = "testIface";
    auto ret = instance_->GetIfaceRxBytes(testIface);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, GetIfaceTxBytes001, TestSize.Level1)
{
    std::string testIface = "testIface";
    auto ret = instance_->GetIfaceTxBytes(testIface);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetManagerNativeTest, FirewallSetUidRule001, TestSize.Level1)
{
    uint32_t chain = NetManagerStandard::ChainType::CHAIN_NONE;
    std::vector<uint32_t> uids;
    uint32_t firewallRule = NetManagerStandard::FirewallRule::RULE_ALLOW;
    auto ret = instance_->FirewallSetUidRule(chain, uids, firewallRule);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerNativeTest, DnsGetAddrInfo001, TestSize.Level1)
{
    std::string hostName;
    std::string serverName;
    AddrInfo hints;
    uint16_t netId = 0;
    std::vector<AddrInfo> res;
    auto ret = instance_->DnsGetAddrInfo(hostName, serverName, hints, netId, res);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerNativeTest, AddStaticArpTest001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    auto ret = instance_->AddStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerNativeTest, DelStaticArpTest001, TestSize.Level1)
{
    std::string ipAddr = "192.168.1.100";
    std::string macAddr = "aa:bb:cc:dd:ee:ff";
    std::string ifName = "wlan0";
    auto ret = instance_->DelStaticArp(ipAddr, macAddr, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace nmd
} // namespace OHOS
