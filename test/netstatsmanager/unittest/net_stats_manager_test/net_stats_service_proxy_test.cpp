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

#include <vector>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "iremote_proxy.h"
#include "net_manager_center.h"
#include "net_stats_callback.h"
#include "net_stats_callback_test.h"
#include "net_stats_constants.h"
#include "net_stats_service.h"
#include "net_stats_service_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
constexpr const char *ETH_IFACE_NAME = "lo";
constexpr int64_t TEST_UID = 1010;
void GetIfaceNamesFromManager(std::list<std::string> &ifaceNames)
{
    NetManagerCenter::GetInstance().GetIfaceNames(BEARER_CELLULAR, ifaceNames);
}
} // namespace

using namespace testing::ext;
class NetStatsServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetStatsServiceProxy> instance_ = nullptr;
    static inline sptr<INetStatsCallback> callback_ = nullptr;
};

void NetStatsServiceProxyTest::SetUpTestCase()
{
    instance_ = std::make_shared<NetStatsServiceProxy>(nullptr);
    callback_ = new (std::nothrow) NetStatsCallbackTest();
}

void NetStatsServiceProxyTest::TearDownTestCase()
{
    instance_ = nullptr;
}

void NetStatsServiceProxyTest::SetUp() {}

void NetStatsServiceProxyTest::TearDown() {}

HWTEST_F(NetStatsServiceProxyTest, RegisterNetStatsCallback, TestSize.Level1)
{
    int32_t ret;
    instance_->RegisterNetStatsCallback(callback_);
    instance_->RegisterNetStatsCallback(callback_);
    instance_->RegisterNetStatsCallback(nullptr);
    instance_->UnregisterNetStatsCallback(callback_);
    instance_->UnregisterNetStatsCallback(callback_);
    ret = instance_->UnregisterNetStatsCallback(nullptr);
    EXPECT_EQ(ret, NETMANAGER_ERR_PARAMETER_ERROR);
    for (int16_t i = 0; i < LIMIT_STATS_CALLBACK_NUM; i++) {
        sptr<INetStatsCallback> callback = new (std::nothrow) NetStatsCallbackTest();
        instance_->RegisterNetStatsCallback(callback);
    }
    ret = instance_->RegisterNetStatsCallback(callback_);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsServiceProxyTest, GetIfaceRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = instance_->GetIfaceRxBytes(stats, ETH_IFACE_NAME);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

HWTEST_F(NetStatsServiceProxyTest, GetIfaceTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = instance_->GetIfaceTxBytes(stats, ETH_IFACE_NAME);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

HWTEST_F(NetStatsServiceProxyTest, GetCellularRxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    uint64_t stats = 0;
    int32_t ret = instance_->GetCellularRxBytes(stats);
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(stats, static_cast<uint64_t>(0));
}

HWTEST_F(NetStatsServiceProxyTest, GetCellularTxBytesTest001, TestSize.Level1)
{
    std::list<std::string> ifaceNames;
    uint64_t stats = 0;
    int32_t ret = instance_->GetCellularTxBytes(stats);
    GetIfaceNamesFromManager(ifaceNames);
    if (ifaceNames.empty()) {
        EXPECT_GE(ret, -1);
        return;
    }
    EXPECT_GE(stats, static_cast<uint64_t>(0));
}

HWTEST_F(NetStatsServiceProxyTest, GetAllRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = instance_->GetAllRxBytes(stats);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

HWTEST_F(NetStatsServiceProxyTest, GetAllTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = instance_->GetAllTxBytes(stats);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

HWTEST_F(NetStatsServiceProxyTest, GetUidRxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = instance_->GetUidRxBytes(stats, TEST_UID);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

HWTEST_F(NetStatsServiceProxyTest, GetUidTxBytesTest001, TestSize.Level1)
{
    uint64_t stats = 0;
    int32_t ret = instance_->GetUidTxBytes(stats, TEST_UID);
    EXPECT_GE(stats, static_cast<uint64_t>(0));
    DTEST_LOG << "Ret" << ret << std::endl;
}

HWTEST_F(NetStatsServiceProxyTest, GetIfaceStatsDetail001, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    int32_t ret = instance_->GetIfaceStatsDetail(iface, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetStatsServiceProxyTest, GetUidStatsDetail001, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    uint32_t uid = 1234;
    int32_t ret = instance_->GetUidStatsDetail(iface, uid, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetStatsServiceProxyTest, UpdateIfacesStats, TestSize.Level1)
{
    NetStatsInfo info;
    std::string iface = "wlan0";
    int32_t ret = instance_->UpdateIfacesStats(iface, 0, UINT32_MAX, info);
    EXPECT_EQ(ret, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetStatsServiceProxyTest, ResetFactory001, TestSize.Level1)
{
    NetStatsInfo info;
    info.iface_ = "wlan0";
    info.date_ = 115200;
    info.rxBytes_ = 10000;
    info.txBytes_ = 11000;
    info.rxPackets_ = 1000;
    info.txPackets_ = 1100;

    int32_t ret = instance_->ResetFactory();
    EXPECT_EQ(ret, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}

HWTEST_F(NetStatsServiceProxyTest, UpdateStatsData001, TestSize.Level1)
{
    NetStatsInfo info;
    info.iface_ = "wlan0";
    info.date_ = 115200;
    info.rxBytes_ = 10000;
    info.txBytes_ = 11000;
    info.rxPackets_ = 1000;
    info.txPackets_ = 1100;

    int32_t ret = instance_->UpdateStatsData();
    EXPECT_EQ(ret, NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
}
} // namespace NetManagerStandard
} // namespace OHOS