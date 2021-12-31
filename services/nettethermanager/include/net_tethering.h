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

#ifndef NET_TETHERING_H
#define NET_TETHERING_H

#include <string>
#include <list>
#include <map>
#include <memory>

#include "net_tether_constants.h"
#include "net_tether_define.h"
#include "i_net_tether_callback.h"
#include "net_tether_iface_manager.h"
#include "net_tether_request_network.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTethering {
public:
    ~NetTethering();
    int32_t TetherByIface(const std::string &ifName);
    int32_t UntetherByIface(const std::string &ifName);
    int32_t TetherByType(TetheringType type);
    int32_t UntetherByType(TetheringType type);
    int32_t RegisterTetheringEventCallback(const sptr<INetTetherCallback> &callback);
    static NetTethering* GetInstance();
    static void ReleaseInstance();

private:
    int32_t ChooseTetherType(TetheringType type, bool tether);
    int32_t TryWifiTethering(bool enable);
    int32_t TryUsbTethering(bool enable);
    int32_t TryBluetoothTethering(bool enable);
    void HandleApEvent(int32_t state);
    void HandleUsbEvent(bool isRndis);
    void EnableWifiTether(const std::string &ifName);
    void DisableWifiTether(const std::string &ifName);
    void TrackNewInterface(const std::string &ifName);
    void UntrackInterface(const std::string &ifName);
    void ChangeInterfaceState(const std::string &ifName, bool startTether);
    TetheringType IfnameToType(const std::string &ifName);
    void IfaceStateChange(const std::string &iface, int32_t state);
    void RequestTethering(TetheringType type, bool enable);
    void InterfaceTetheringActive(const std::string &iface);
    void InterfaceTetheringInactive(const std::string &iface);
    bool SetReqestedNetwork(const std::string &iface);
    bool SetDnsForward(uint32_t netId);
    bool RequestedNetworkChange(uint32_t netId, const std::string &iface);
    void CallbackRequestNetworLost(int32_t netId);
    void CallbackNetdResponseInterfaceAdd(const std::string &iface);
    void CallbackNetdResponseInterfaceRemoved(const std::string &iface);
    NetTethering();

private:
    std::list<TetheringType> currentTether_;
    std::map<std::string, std::unique_ptr<NetTetherIfaceManager>> ifaceMap_;
    std::unique_ptr<NetTetherRequestNetwork> netTetherRequestNetwork_;
    uint32_t currentRequestedNetId_;
    sptr<INetTetherCallback> netTetherCallback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHERING_H