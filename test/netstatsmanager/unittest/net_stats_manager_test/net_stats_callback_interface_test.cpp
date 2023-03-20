/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "net_stats_callback_proxy.h"
#include "net_stats_callback_stub.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *TEST_IFACE = "TEST_IFACE";
constexpr uint32_t TEST_UID = 4454;

class NetStatsCallbackStubTest : public NetStatsCallbackStub {
public:
    NetStatsCallbackStubTest() = default;
    ~NetStatsCallbackStubTest() override {}
    int32_t NetIfaceStatsChanged(const std::string &iface) override
    {
        std::cout << std::endl;
        std::cout << "Stub NetIfaceStatsChanged::iface: " << iface << std::endl;
        return 0;
    }
    int32_t NetUidStatsChanged(const std::string &iface, uint32_t uid) override
    {
        std::cout << std::endl;
        std::cout << "Stub NetUidStatsChanged::iface: " << iface << ", uid:" << uid << std::endl;
        return 0;
    }
};

class MockNetIRemoteObject : public IRemoteObject {
public:
    MockNetIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}
    ~MockNetIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

class NetStatsCallbackNotifyTest {
public:
    NetStatsCallbackNotifyTest() = default;
    ~NetStatsCallbackNotifyTest() = default;

    void RegisterNetStatsCbChanged(sptr<NetStatsCallbackStub> &callback)
    {
        netStatsCb_ = callback;
    }

    int32_t NotifyNetIfaceStatsChangedNoString()
    {
        MessageParcel dataErr;
        if (!dataErr.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
            return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
        }
        MessageParcel reply;
        MessageOption option;
        int32_t ret = netStatsCb_->OnRemoteRequest(INetStatsCallback::NET_STATS_IFACE_CHANGED, dataErr, reply, option);
        std::cout << "thread HandleIfaceChangedThread 001 ret:" << ret << std::endl;
        return ret;
    }

    int32_t NotifyNetIfaceStatsChanged(const std::string &iface)
    {
        MessageParcel data;
        if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
            return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
        }
        if (!data.WriteString(iface)) {
            return NETMANAGER_ERR_WRITE_DATA_FAIL;
        }
        MessageParcel reply;
        MessageOption option;
        int32_t ret = netStatsCb_->OnRemoteRequest(INetStatsCallback::NET_STATS_IFACE_CHANGED, data, reply, option);
        std::cout << "thread HandleIfaceChangedThread 002 ret:" << ret << std::endl;
        return ret;
    }

    int32_t NotifyNetUidStatsChangedNoString()
    {
        MessageParcel dataNoStr;
        if (!dataNoStr.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
            return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
        }
        MessageParcel reply;
        MessageOption option;
        int32_t ret = netStatsCb_->OnRemoteRequest(INetStatsCallback::NET_STATS_UID_CHANGED, dataNoStr, reply, option);
        std::cout << "thread NetUidStatsChanged 001 ret:" << ret << std::endl;
        return ret;
    }

    int32_t NotifyNetUidStatsChangedNoInt(const std::string &iface)
    {
        MessageParcel dataNoInt;
        if (!dataNoInt.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
            return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
        }
        if (!dataNoInt.WriteString(iface)) {
            return NETMANAGER_ERR_WRITE_DATA_FAIL;
        }
        MessageParcel reply;
        MessageOption option;
        int32_t ret = netStatsCb_->OnRemoteRequest(INetStatsCallback::NET_STATS_UID_CHANGED, dataNoInt, reply, option);
        std::cout << "thread NetUidStatsChanged 002 ret:" << ret << std::endl;
        return ret;
    }

    int32_t NotifyNetUidStatsChanged(const std::string &iface, uint32_t uid)
    {
        MessageParcel data;
        if (!data.WriteInterfaceToken(NetStatsCallbackStub::GetDescriptor())) {
            return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
        }
        if (!data.WriteString(iface)) {
            return NETMANAGER_ERR_WRITE_DATA_FAIL;
        }
        if (!data.WriteUint32(uid)) {
            return NETMANAGER_ERR_WRITE_DATA_FAIL;
        }
        MessageParcel reply;
        MessageOption option;
        int32_t ret = netStatsCb_->OnRemoteRequest(INetStatsCallback::NET_STATS_UID_CHANGED, data, reply, option);
        std::cout << "thread NetUidStatsChanged 003 ret:" << ret << std::endl;
        return ret;
    }

    sptr<NetStatsCallbackStub> netStatsCb_ = nullptr;
};
} // namespace

class NetStatsCallbackInterfaceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<NetStatsCallbackNotifyTest> instance_ = nullptr;
};

void NetStatsCallbackInterfaceTest::SetUpTestCase()
{
    instance_ = std::make_shared<NetStatsCallbackNotifyTest>();
    sptr<NetStatsCallbackStub> callback = new (std::nothrow) NetStatsCallbackStubTest();
    instance_->RegisterNetStatsCbChanged(callback);
}

void NetStatsCallbackInterfaceTest::TearDownTestCase() {}

void NetStatsCallbackInterfaceTest::SetUp() {}

void NetStatsCallbackInterfaceTest::TearDown() {}

HWTEST_F(NetStatsCallbackInterfaceTest, NotifyNetIfaceStatsChangedTest001, TestSize.Level1)
{
    int32_t ret = instance_->NotifyNetIfaceStatsChangedNoString();
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    ret = instance_->NotifyNetIfaceStatsChanged(TEST_IFACE);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsCallbackInterfaceTest, NotifyNetUidStatsChangedTest001, TestSize.Level1)
{
    int32_t ret = instance_->NotifyNetUidStatsChangedNoString();
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    ret = instance_->NotifyNetUidStatsChangedNoInt(TEST_IFACE);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
    ret = instance_->NotifyNetUidStatsChanged(TEST_IFACE, TEST_UID);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsCallbackInterfaceTest, NetIfaceStatsChangedTest001, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new (std::nothrow) MockNetIRemoteObject();
    sptr<NetStatsCallbackProxy> netCbProxy = new (std::nothrow) NetStatsCallbackProxy(impl);
    int32_t ret = netCbProxy->NetIfaceStatsChanged(TEST_IFACE);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);

    ret = netCbProxy->NetUidStatsChanged(TEST_IFACE, TEST_UID);
    EXPECT_NE(ret, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS