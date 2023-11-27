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
#include <iostream>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "i_net_stats_callback.h"
#include "net_all_capabilities.h"
#include "net_manager_constants.h"

#include "net_stats_service_stub.h"
#include "net_stats_callback_stub.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
namespace {
constexpr uint64_t OUTOFFRANGECODE = 100;
constexpr uint32_t TEST_UINT32_VALUE = 100;
constexpr uint64_t TEST_UINT64_VALUE = 100;
constexpr const char *TEST_STRING = "test";

class TestNetStatsCallback : public NetStatsCallbackStub {
public:
    TestNetStatsCallback() = default;
    ~TestNetStatsCallback() override{};

    int32_t NetIfaceStatsChanged(const std::string &iface) override
    {
        return 0;
    }

    int32_t NetUidStatsChanged(const std::string &iface, uint32_t uid) override
    {
        return 0;
    }
};

class MockNetStatsServiceStub : public NetStatsServiceStub {
public:
    MockNetStatsServiceStub() = default;
    ~MockNetStatsServiceStub() {}

    int32_t GetIfaceRxBytes(uint64_t &stats, const std::string &interfaceName) override
    {
        return 0;
    }

    int32_t GetIfaceTxBytes(uint64_t &stats, const std::string &interfaceName) override
    {
        return 0;
    }

    int32_t GetCellularRxBytes(uint64_t &stats) override
    {
        return 0;
    }

    int32_t GetCellularTxBytes(uint64_t &stats) override
    {
        return 0;
    }

    int32_t GetAllRxBytes(uint64_t &stats) override
    {
        return 0;
    }

    int32_t GetAllTxBytes(uint64_t &stats) override
    {
        return 0;
    }

    int32_t GetUidRxBytes(uint64_t &stats, uint32_t uid) override
    {
        return 0;
    }

    int32_t GetUidTxBytes(uint64_t &stats, uint32_t uid) override
    {
        return 0;
    }

    int32_t GetAllStatsInfo(std::vector<NetStatsInfo> &info) override
    {
        return 0;
    }

    int32_t RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override
    {
        return 0;
    }

    int32_t UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override
    {
        return 0;
    }

    int32_t GetIfaceStatsDetail(const std::string &iface, uint64_t start, uint64_t end,
                                NetStatsInfo &statsInfo) override
    {
        return 0;
    }

    int32_t GetUidStatsDetail(const std::string &iface, uint32_t uid, uint64_t start, uint64_t end,
                              NetStatsInfo &statsInfo) override
    {
        return 0;
    }

    int32_t UpdateIfacesStats(const std::string &iface, uint64_t start, uint64_t end,
                                      const NetStatsInfo &stats) override
    {
        return 0;
    }

    int32_t UpdateStatsData() override
    {
        return 0;
    }

    int32_t ResetFactory() override
    {
        return 0;
    }

    int32_t GetCookieRxBytes(uint64_t &stats, uint64_t cookie) override
    {
        return 0;
    }

    int32_t GetCookieTxBytes(uint64_t &stats, uint64_t cookie) override
    {
        return 0;
    }
};

class TestNetStatsServiceStub : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<NetStatsServiceStub> instance_ = std::make_shared<MockNetStatsServiceStub>();
};

void TestNetStatsServiceStub::SetUpTestCase() {}

void TestNetStatsServiceStub::TearDownTestCase() {}

void TestNetStatsServiceStub::SetUp() {}

void TestNetStatsServiceStub::TearDown() {}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Test NetConnCallbackStub OnRemoteRequest.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, OnRemoteRequestTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(OUTOFFRANGECODE, data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERR_DESCRIPTOR_MISMATCH);
}

/**
 * @tc.name: RegisterNetStatsCallbackTest001
 * @tc.desc: Test NetConnCallbackStub RegisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, RegisterNetStatsCallbackTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetStatsCallback> callback = new (std::nothrow) TestNetStatsCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_REGISTER_NET_STATS_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UnregisterNetStatsCallbackTest001
 * @tc.desc: Test NetConnCallbackStub UnregisterNetStatsCallback.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, UnregisterNetStatsCallbackTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    sptr<INetStatsCallback> callback = new (std::nothrow) TestNetStatsCallback();
    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(
        static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_UNREGISTER_NET_STATS_CALLBACK), data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIfaceRxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetIfaceRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetIfaceRxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_RXBYTES), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIfaceTxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetIfaceTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetIfaceTxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_TXBYTES), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetCellularRxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetCellularRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetCellularRxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_CELLULAR_RXBYTES), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetCellularTxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetCellularTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetCellularTxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_CELLULAR_TXBYTES), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAllRxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetAllRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetAllRxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_RXBYTES), data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAllTxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetAllTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetAllTxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_TXBYTES), data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetUidRxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetUidRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetUidRxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_RXBYTES), data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetUidTxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetUidTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetUidTxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret =
        instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_TXBYTES), data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetIfaceStatsDetailTest001
 * @tc.desc: Test NetConnCallbackStub GetIfaceStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetIfaceStatsDetailTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_STATS_DETAIL),
                                             data, reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetUidStatsDetailTest001
 * @tc.desc: Test NetConnCallbackStub GetUidStatsDetail.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetUidStatsDetailTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteUint32(TEST_UINT32_VALUE)) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_STATS_DETAIL), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateIfacesStatsTest001
 * @tc.desc: Test NetConnCallbackStub UpdateIfacesStats.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, UpdateIfacesStatsTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteString(TEST_STRING)) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    NetStatsInfo stats;
    stats.Marshalling(data);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_UPDATE_IFACES_STATS), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateStatsDataTest001
 * @tc.desc: Test NetConnCallbackStub UpdateStatsData.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, UpdateStatsDataTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_UPDATE_STATS_DATA), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: ResetFactoryTest001
 * @tc.desc: Test NetConnCallbackStub ResetFactory.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, ResetFactoryTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_RESET_FACTORY), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetAllStatsInfoTest001
 * @tc.desc: Test NetConnCallbackStub GetAllStatsInfo.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetAllStatsInfoTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_STATS_INFO), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetCookieRxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetCookieRxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetCookieRxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_COOKIE_RXBYTES), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: GetCookieTxBytesTest001
 * @tc.desc: Test NetConnCallbackStub GetCookieTxBytes.
 * @tc.type: FUNC
 */
HWTEST_F(TestNetStatsServiceStub, GetCookieTxBytesTest001, TestSize.Level1)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return;
    }
    if (!data.WriteUint64(TEST_UINT64_VALUE)) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = instance_->OnRemoteRequest(static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_COOKIE_TXBYTES), data,
                                             reply, option);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}
} // namespace
} // namespace NetManagerStandard
} // namespace OHOS