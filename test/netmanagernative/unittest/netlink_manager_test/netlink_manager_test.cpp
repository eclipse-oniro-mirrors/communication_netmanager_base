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

#include <iostream>
#include <memory>

#include <gtest/gtest.h>

#include "i_netsys_service.h"
#include "iservice_registry.h"
#include "notify_callback_stub.h"
#include "system_ability_definition.h"

#include "wifi_ap_msg.h"
#include "wifi_hotspot.h"

namespace OHOS::nmd {
using namespace testing::ext;
sptr<OHOS::NetsysNative::INotifyCallback> nativeNotifyCallback_ = nullptr;
sptr<OHOS::NetsysNative::INetsysService> netsysNativeService_ = nullptr;

std::unique_ptr<Wifi::WifiHotspot> wifiHotspot_ = nullptr;
class NetlinkNativeNotifyCallBack : public OHOS::NetsysNative::NotifyCallbackStub {
public:
    int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                      int scope) override;
    int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                      int scope) override;
    int32_t OnInterfaceAdded(const std::string &ifName) override;
    int32_t OnInterfaceRemoved(const std::string &ifName) override;
    int32_t OnInterfaceChanged(const std::string &ifName, bool up) override;
    int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up) override;
    int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                           const std::string &ifName) override;
    int32_t OnDhcpSuccess(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult) override;
    int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override;
};

class NetlinkManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            return;
        }
        auto remote = samgr->GetSystemAbility(OHOS::COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
        if (remote == nullptr) {
            return;
        }
        nativeNotifyCallback_ = new NetlinkNativeNotifyCallBack();
        auto proxy = iface_cast<NetsysNative::INetsysService>(remote);
        netsysNativeService_ = proxy;
        wifiHotspot_ = Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
    }
    static void TearDownTestCase()
    {
        nativeNotifyCallback_ = nullptr;
        netsysNativeService_ = nullptr;
        wifiHotspot_ = nullptr;
    }
};

int32_t NetlinkNativeNotifyCallBack::OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName,
                                                               int flags, int scope)
{
    std::cout << " [OnInterfaceAddressUpdated] " << ifName << " Address: " << addr << std::endl;
    return 0;
}
int32_t NetlinkNativeNotifyCallBack::OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName,
                                                               int flags, int scope)
{
    std::cout << " [OnInterfaceAddressRemoved] " << ifName << " Address: " << addr << std::endl;
    return 0;
}
int32_t NetlinkNativeNotifyCallBack::OnInterfaceAdded(const std::string &ifName)
{
    std::cout << " [OnInterfaceAdded] " << ifName << std::endl;
    return 0;
}
int32_t NetlinkNativeNotifyCallBack::OnInterfaceRemoved(const std::string &ifName)
{
    std::cout << " [OnInterfaceRemoved] " << ifName << std::endl;
    return 0;
}
int32_t NetlinkNativeNotifyCallBack::OnInterfaceChanged(const std::string &ifName, bool up)
{
    std::cout << " [OnInterfaceChanged] " << ifName << " status :" << (up ? "[update]" : "[remove]") << std::endl;
    return 0;
}
int32_t NetlinkNativeNotifyCallBack::OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
{
    std::cout << " [OnInterfaceLinkStateChanged] " << ifName << " Status: " << (up ? "[update]" : "[remove]")
              << std::endl;
    return 0;
}
int32_t NetlinkNativeNotifyCallBack::OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                                                    const std::string &ifName)
{
    std::cout << " [OnRouteChanged] " << ifName << " Route: " << route
              << " State :" << (updated ? "[update]" : "[remove]") << std::endl;
    return 0;
}

int32_t NetlinkNativeNotifyCallBack::OnDhcpSuccess(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult)
{
    std::cout << " [OnDhcpSuccess] " << std::endl;
    return 0;
}
int32_t NetlinkNativeNotifyCallBack::OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface)
{
    std::cout << " [OnBandwidthReachedLimit] " << std::endl;
    return 0;
}

HWTEST_F(NetlinkManagerTest, TestServiceGet001, TestSize.Level1)
{
    EXPECT_NE(netsysNativeService_, nullptr);
}

HWTEST_F(NetlinkManagerTest, RegisterCallbackTest001, TestSize.Level1)
{
    auto result = netsysNativeService_->RegisterNotifyCallback(nativeNotifyCallback_);
    EXPECT_EQ(result, 0);
}

HWTEST_F(NetlinkManagerTest, NotifyAll001, TestSize.Level1)
{
    wifiHotspot_->EnableHotspot(Wifi::ServiceType::DEFAULT);
    // Wait for the callback to be called.
    sleep(10);
}

HWTEST_F(NetlinkManagerTest, NotifyAll002, TestSize.Level1)
{
    wifiHotspot_->DisableHotspot(Wifi::ServiceType::DEFAULT);
    // Wait for the callback to be called.
    sleep(10);
}

HWTEST_F(NetlinkManagerTest, UnRegisterCallbackTest001, TestSize.Level1)
{
    auto result = netsysNativeService_->UnRegisterNotifyCallback(nativeNotifyCallback_);
    EXPECT_EQ(result, 0);
}

// For this it will not recview the interface status change event.
HWTEST_F(NetlinkManagerTest, NotNotifyAnymore001, TestSize.Level1)
{
    wifiHotspot_->EnableHotspot(Wifi::ServiceType::DEFAULT);
    // Wait for the callback to be called.
    sleep(10);
}

// For this it will not recview the interface status change event.
HWTEST_F(NetlinkManagerTest, NotNotifyAnymore002, TestSize.Level1)
{
    wifiHotspot_->DisableHotspot(Wifi::ServiceType::DEFAULT);
    // Wait for the callback to be called.
    sleep(10);
}
} // namespace OHOS::nmd