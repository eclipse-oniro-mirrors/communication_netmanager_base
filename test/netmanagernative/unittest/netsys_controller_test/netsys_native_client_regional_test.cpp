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
#endif
#include "bpf_def.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "net_manager_constants.h"
#include "net_stats_constants.h"
#include "netsys_native_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
static constexpr const char *IFACE = "test0";
static constexpr const char *IP_ADDR = "172.17.5.245";
static constexpr const char *INTERFACE_NAME = "interface_name";
const int32_t NET_ID = 2;
} // namespace

class NetsysNativeClientTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
    static inline std::shared_ptr<NetsysNativeClient> nativeClientInstance_ = std::make_shared<NetsysNativeClient>();
    static inline NetsysNativeClient &nativeClient_ = *nativeClientInstance_;
};

void NetsysNativeClientTest::SetUpTestCase() {}

void NetsysNativeClientTest::TearDownTestCase() {}

void NetsysNativeClientTest::SetUp() {}

void NetsysNativeClientTest::TearDown() {}

HWTEST_F(NetsysNativeClientTest, OnInterfaceAddressUpdatedTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClientInstance_);
    const std::string addr = IP_ADDR;
    const std::string ifName = INTERFACE_NAME;
    int flags = 1;
    int scope = 1;
    notifyCallback.netsysNativeClient_.cbObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnInterfaceAddressRemovedTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClientInstance_);
    const std::string addr = IP_ADDR;
    const std::string ifName = INTERFACE_NAME;
    int flags = 1;
    int scope = 1;
    notifyCallback.netsysNativeClient_.cbObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnInterfaceAddedTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClientInstance_);
    const std::string ifName = INTERFACE_NAME;
    notifyCallback.netsysNativeClient_.cbObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnInterfaceAdded(ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnInterfaceRemovedTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClientInstance_);
    const std::string ifName = INTERFACE_NAME;
    notifyCallback.netsysNativeClient_.cbObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnInterfaceRemoved(ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnInterfaceChangedTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClientInstance_);
    const std::string ifName = INTERFACE_NAME;
    notifyCallback.netsysNativeClient_.cbObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnInterfaceChanged(ifName, true);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnInterfaceLinkStateChangedTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClientInstance_);
    const std::string ifName = INTERFACE_NAME;
    notifyCallback.netsysNativeClient_.cbObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnInterfaceLinkStateChanged(ifName, true);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnRouteChangedTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNotifyCallback notifyCallback(nativeClientInstance_);
    bool updated = true;
    const std::string route = "route";
    const std::string gateway = "gateway";
    const std::string ifName = INTERFACE_NAME;
    notifyCallback.netsysNativeClient_.cbObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnRouteChanged(updated, route, gateway, ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnDnsResultReportTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNetDnsResultCallback notifyCallback(nativeClientInstance_);
    uint32_t size = 1;
    OHOS::NetsysNative::NetDnsResultReport netDnsResultRepor{};
    std::list<OHOS::NetsysNative::NetDnsResultReport> res = {netDnsResultRepor};
    notifyCallback.netsysNativeClient_.cbDnsReportObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnDnsResultReport(size, res);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnDnsQueryResultReportTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNetDnsResultCallback notifyCallback(nativeClientInstance_);
    uint32_t size = 1;
    OHOS::NetsysNative::NetDnsQueryResultReport netDnsResultRepor{};
    std::list<OHOS::NetsysNative::NetDnsQueryResultReport> res = {netDnsResultRepor};
    notifyCallback.netsysNativeClient_.cbDnsQueryReportObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnDnsQueryResultReport(size, res);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, OnDnsQueryAbnormalReportTest001, TestSize.Level1)
{
    NetsysNativeClient::NativeNetDnsResultCallback notifyCallback(nativeClientInstance_);
    uint32_t eventfailcause = 1;
    OHOS::NetsysNative::NetDnsQueryResultReport netDnsResultRepor{};
    notifyCallback.netsysNativeClient_.cbDnsQueryReportObjects_ = {nullptr};
    int32_t ret = notifyCallback.OnDnsQueryAbnormalReport(eventfailcause, netDnsResultRepor);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, DelInterfaceAddressTest001, TestSize.Level1)
{
    const std::string ifName = INTERFACE_NAME;
    const std::string ipAddr = IP_ADDR;
    int32_t prefixLength = 1;
    const std::string netCapabilities = "netCapabilities";
    int32_t ret = nativeClient_.DelInterfaceAddress(ifName, ipAddr, prefixLength, netCapabilities);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

HWTEST_F(NetsysNativeClientTest, InterfaceSetIffUpTest001, TestSize.Level1)
{
    const std::string ifName = INTERFACE_NAME;
    int32_t ret = nativeClient_.InterfaceSetIffUp(ifName);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}

HWTEST_F(NetsysNativeClientTest, ProcessDhcpResultTest001, TestSize.Level1)
{
    sptr<OHOS::NetsysNative::DhcpResultParcel> dhcpResult = new (std::nothrow) OHOS::NetsysNative::DhcpResultParcel();
    sptr<NetsysControllerCallback> netsysCallback = nullptr;
    nativeClient_.cbObjects_.push_back(netsysCallback);
    nativeClient_.ProcessDhcpResult(dhcpResult);
    EXPECT_TRUE(nativeClient_.cbObjects_.empty());
}

HWTEST_F(NetsysNativeClientTest, ProcessBandwidthReachedLimitTest001, TestSize.Level1)
{
    const std::string limitName = "limitName";
    const std::string iface = INTERFACE_NAME;
    sptr<NetsysControllerCallback> netsysCallback = nullptr;
    nativeClient_.cbObjects_.push_back(netsysCallback);
    nativeClient_.ProcessBandwidthReachedLimit(limitName, iface);
    EXPECT_TRUE(nativeClient_.cbObjects_.empty());
}
} // namespace NetManagerStandard
} // namespace OHOS
