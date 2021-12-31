/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NET_TETHER_IFACE_MANAGER_H
#define NET_TETHER_IFACE_MANAGER_H

#include <string>
#include <memory>

#include "net_tether_constants.h"
#include "net_tether_define.h"
#include "net_tether_ip_address.h"
#include "net_tether_netd_utils.h"
#include "i_dhcp_result_notify.h"
#include "dhcp_service.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTetherIfaceManager {
public:
    class TetherIDhcpResultNotify : public OHOS::Wifi::IDhcpResultNotify {
    public:
        explicit TetherIDhcpResultNotify(NetTetherIfaceManager &netTetherIfaceManager);
        ~TetherIDhcpResultNotify() override;
        void OnSuccess(int32_t status, const std::string &ifname, OHOS::Wifi::DhcpResult &result) override;
        void OnFailed(int32_t status, const std::string &ifname, const std::string &reason) override;
        void OnSerExitNotify(const std::string &ifname) override;
    private:
        NetTetherIfaceManager &netTetherIfaceManager_;
    };

public:
    NetTetherIfaceManager(const std::string &ifaceName, TetheringType ifaceType, const IfaceMgrCallback &callback,
        int32_t netId);
    NetTetherIfaceManager() = delete;
    ~NetTetherIfaceManager();
    void Init();
    const std::string &GetIfaceName();
    TetheringType GetIfaceType();
    int32_t GetLastError();
    int32_t GetLastState();
    bool RequestedTether();
    bool UnrequestedTether();
    bool UpstreamForward(const std::string &upstreamIface);
    void ClearUpstream();
    void UnconfigAndUntetherIface();

private:
    void SendInterfaceState(int32_t newState);
    void ConfigAndTetherIface();
    bool ConfigureIPv4();
    bool UnconfigureIPv4();
    void RequestIpv4Addr(NetTetherIpAddress &ipAddr);
    bool TetherInterface(int32_t netId, const std::string &ifaceName, const NetTetherIpAddress &ipAddr);
    bool UntetherInterface(int32_t netId, const std::string &ifaceName, const NetTetherIpAddress &ipAddr);
    bool EnableDhcp(bool enable);
    bool StartTetherDhcpService();
    bool StopTetherDhcpService();

private:
    std::string ifaceName_;
    TetheringType ifaceType_;
    int32_t lastError_;
    int32_t lastState_;
    NetTetherIpAddress ipv4Addr_;
    IfaceMgrCallback callback_;
    std::string upstreamIface_;
    int32_t upstreamNetId_;
    std::unique_ptr<OHOS::Wifi::IDhcpService> dhcpService_ = nullptr;
    std::unique_ptr<TetherIDhcpResultNotify> dhcpResultNotify_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_IFACE_MANAGER_H