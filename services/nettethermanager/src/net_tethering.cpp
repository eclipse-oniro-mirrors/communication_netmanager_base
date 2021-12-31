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

#include <algorithm>

#include "net_mgr_log_wrapper.h"
#include "net_tether_recv_broadcast.h"
#include "net_tether_controller_factory.h"
#include "net_tether_netd_utils.h"
#include "net_tethering.h"

namespace OHOS {
namespace NetManagerStandard {
static NetTethering* instance = nullptr;

NetTethering::NetTethering() : currentRequestedNetId_(0)
{
    currentTether_.clear();
    NetdResponseCallback netdResponseCallback = {
        std::bind(&NetTethering::CallbackNetdResponseInterfaceAdd, this, std::placeholders::_1),
        std::bind(&NetTethering::CallbackNetdResponseInterfaceRemoved, this, std::placeholders::_1),
    };
    NetTetherNetdUtils::GetInstance()->RegisterNetdResponseCallback(netdResponseCallback);
    netTetherRequestNetwork_ = std::make_unique<NetTetherRequestNetwork>();
    RequestNetworkCallback requestCallback = {
        std::bind(&NetTethering::CallbackRequestNetworLost, this, std::placeholders::_1),
    };
    netTetherRequestNetwork_->RegisterNetRequestCallback(requestCallback);
    NetTetherRecvBroadcast::GetInstance()->AddApStateChangeSubscribe(std::bind(&NetTethering::HandleApEvent, this,
        std::placeholders::_1));
    NetTetherRecvBroadcast::GetInstance()->AddUsbStateChangeSubscribe(std::bind(&NetTethering::HandleUsbEvent, this,
        std::placeholders::_1));
}

NetTethering* NetTethering::GetInstance()
{
    if (instance == nullptr) {
        instance = new NetTethering();
    }
    return instance;
}

void NetTethering::ReleaseInstance()
{
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
    return;
}

NetTethering::~NetTethering()
{
    RequestNetworkCallback Requescallback;
    netTetherRequestNetwork_->RegisterNetRequestCallback(Requescallback);
    NetdResponseCallback netdResponseCallback;
    NetTetherNetdUtils::GetInstance()->RegisterNetdResponseCallback(netdResponseCallback);
    NetTetherRecvBroadcast::GetInstance()->RemoveApStateChangeSubscribe();
    NetTetherRecvBroadcast::GetInstance()->RemoveUsbStateChangeSubscribe();
}

int32_t NetTethering::TetherByType(TetheringType type)
{
    NETMGR_LOG_D("NetTethering::StartTethering, type: [%{public}d]", static_cast<int32_t>(type));
    if (type == TETHERING_INVALID) {
        return TETHERING_TYPE_ERR;
    }
    auto iter = find(currentTether_.begin(), currentTether_.end(), type);
    if (iter != currentTether_.end()) {
        return TETHERING_TETHER_ALREADY_OPEN;
    }
    currentTether_.push_back(type);
    return ChooseTetherType(type, true);
}

int32_t NetTethering::UntetherByType(TetheringType type)
{
    NETMGR_LOG_D("NetTethering::StopTethering, type: [%{public}d]", static_cast<int32_t>(type));
    if (type == TETHERING_INVALID) {
        return TETHERING_TYPE_ERR;
    }
    auto iter = find(currentTether_.begin(), currentTether_.end(), type);
    if (iter == currentTether_.end()) {
        return TETHERING_TETHER_NOT_OPEN;
    }
    currentTether_.erase(iter);
    return ChooseTetherType(type, false);
}

int32_t NetTethering::ChooseTetherType(TetheringType type, bool tether)
{
    int32_t ret = TETHERING_NO_ERR;
    switch (type) {
        case TETHERING_WIFI: {
            ret = TryWifiTethering(tether);
            break;
        }
        case TETHERING_USB: {
            ret = TryUsbTethering(tether);
            break;
        }
        case TETHERING_BLUETOOTH: {
            ret = TryBluetoothTethering(tether);
            break;
        }
        default:
            return TETHERING_TYPE_ERR;
    }
    return ret;
}

int32_t NetTethering::TryWifiTethering(bool enable)
{
    NETMGR_LOG_D("NetTethering::TryWifiTethering");
    int32_t ret = TETHERING_NO_ERR;
    if (enable) {
        sptr<NetTetherController> netInst =
            DelayedSingleton<NetTetherControllerFactory>::GetInstance()->MakeNetTetherController(WIFI_SA_ID);
        if (netInst == nullptr) {
            return TETHERING_GET_SA_ERR;
        }
        if (netInst->OpenTether() != 0) {
            ret = TETHERING_INTERNAL_ERROR;
        }
    } else {
        sptr<NetTetherController> netInst =
            DelayedSingleton<NetTetherControllerFactory>::GetInstance()->MakeNetTetherController(WIFI_SA_ID);
        if (netInst == nullptr) {
            return TETHERING_GET_SA_ERR;
        }
        if (netInst->CloseTether() != 0) {
            ret = TETHERING_INTERNAL_ERROR;
        }
    }
    return ret;
}

int32_t NetTethering::TryUsbTethering(bool enable)
{
    NETMGR_LOG_D("NetTethering::TryUsbTethering");
    int32_t ret = TETHERING_NO_ERR;
    if (enable) {
        NETMGR_LOG_D("Try to start usb tethering");
    } else {
        NETMGR_LOG_D("Try to stop usb tethering");
    }
    return ret;
}

int32_t NetTethering::TryBluetoothTethering(bool enable)
{
    NETMGR_LOG_D("NetTethering::TryBluetoothTethering");
    int32_t ret = TETHERING_NO_ERR;
    if (enable) {
        NETMGR_LOG_D("Try to start bluetooth tethering");
    } else {
        NETMGR_LOG_D("Try to stop bluetooth tethering");
    }
    return ret;
}

void NetTethering::HandleApEvent(int32_t state)
{
    NETMGR_LOG_D("Ap changed state: [%{public}d]", state);
    const int32_t AP_STATE_NONE = 0;
    const int32_t AP_STATE_IDLE = 1;
    const int32_t AP_STATE_STARTING = 2;
    const int32_t AP_STATE_STARTED = 3;
    const int32_t AP_STATE_CLOSING = 4;
    const int32_t AP_STATE_CLOSED = 5;

    switch (state) {
        case AP_STATE_NONE:
        case AP_STATE_STARTING:
        case AP_STATE_CLOSING: {
            break;
        }
        case AP_STATE_IDLE:
        case AP_STATE_CLOSED: {
            DisableWifiTether(TETHER_AP_IFACE);
            break;
        }
        case AP_STATE_STARTED: {
            EnableWifiTether(TETHER_AP_IFACE);
            break;
        }
        default: {
            NETMGR_LOG_D("Error ap changed state.");
            break;
        }
    }
    return;
}

void NetTethering::HandleUsbEvent(bool isRndis)
{
    NETMGR_LOG_D("Usb is rndis mode: [%{public}s]", isRndis ? "true" : "false");
    return;
}

void NetTethering::EnableWifiTether(const std::string &ifName)
{
    NETMGR_LOG_D("NetTethering::EnableWifiTether, ifName: [%{public}s]", ifName.c_str());
    if (ifName.empty()) {
        NETMGR_LOG_E("Empty ifName!");
        return;
    }
    return;
}

void NetTethering::DisableWifiTether(const std::string &ifName)
{
    NETMGR_LOG_D("NetTethering::DisableWifiTether, ifName: [%{public}s]", ifName.c_str());
    if (ifName.empty()) {
        NETMGR_LOG_E("Empty ifName!");
        return;
    }
    if (netTetherCallback_ != nullptr) {
        netTetherCallback_->TetherFailed(TETHERING_WIFI, "", TETHERING_ERR_OPENAP_FAIL);
    }
    auto iter = find(currentTether_.begin(), currentTether_.end(), TETHERING_WIFI);
    if (iter != currentTether_.end()) {
        currentTether_.erase(iter);
    }
}

void NetTethering::TrackNewInterface(const std::string &ifName)
{
    NETMGR_LOG_D("NetTethering::TrackNewInterface, ifName: [%{public}s]", ifName.c_str());
    TetheringType ifType = IfnameToType(ifName);
    if (ifType == TetheringType::TETHERING_INVALID) {
        NETMGR_LOG_E("Invalid interface type with ifName: [%{public}s]", ifName.c_str());
        return;
    }
    auto iter = ifaceMap_.find(ifName);
    if (iter != ifaceMap_.end()) {
        NETMGR_LOG_D("[%{public}s] ifName is already tracked.", ifName.c_str());
        return;
    }
    IfaceMgrCallback cb = {
        std::bind(&NetTethering::IfaceStateChange, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&NetTethering::RequestTethering, this, std::placeholders::_1, std::placeholders::_2),
    };
    int32_t netId = netTetherRequestNetwork_->GetUpstreamNetId();
    std::unique_ptr<NetTetherIfaceManager> ifManager =
        std::make_unique<NetTetherIfaceManager>(ifName, ifType, cb, netId);
    ifaceMap_.insert(std::make_pair(ifName, ifManager.release()));
    ifManager->Init();
    return;
}

void NetTethering::UntrackInterface(const std::string &ifName)
{
    NETMGR_LOG_D("NetTethering::UntrackInterface, ifName: [%{public}s]", ifName.c_str());
    auto iter = ifaceMap_.find(ifName);
    if (iter != ifaceMap_.end()) {
        NETMGR_LOG_D("[%{public}s] ifName not found, untrack failed.", ifName.c_str());
        return;
    }
    iter->second.reset();
    ifaceMap_.erase(iter);
    return;
}

void NetTethering::ChangeInterfaceState(const std::string &ifName, bool startTether)
{
    NETMGR_LOG_D("NetTethering::ChangeInterfaceState, ifName: [%{public}s]", ifName.c_str());
    int32_t ret = TETHERING_NO_ERR;
    if (startTether) {
        ret = TetherByIface(ifName);
    } else {
        ret = UntetherByIface(ifName);
    }
    if (ret != TETHERING_NO_ERR) {
        NETMGR_LOG_E("start tether [%{public}s] iface failed.", ifName.c_str());
    }
    return;
}

int32_t NetTethering::TetherByIface(const std::string &ifName)
{
    NETMGR_LOG_D("NetTethering::TetherByIface, ifName: [%{public}s]", ifName.c_str());
    auto iter = ifaceMap_.find(ifName);
    if (iter == ifaceMap_.end()) {
        NETMGR_LOG_E("Tether unknown [%{public}s] iface, failed.", ifName.c_str());
        return TETHERING_UNKNOWN_IFACE_ERROR;
    }
    if (iter->second->GetLastState() != STATE_AVAILABLE) {
        NETMGR_LOG_E("Tether unavailable [%{public}s] iface, failed.", ifName.c_str());
        return TETHERING_UNAVAIL_IFACE_ERROR;
    }
    if (!iter->second->RequestedTether()) {
        NETMGR_LOG_E("RequestedTether [%{public}s] iface failed.", ifName.c_str());
        if (netTetherCallback_ != nullptr) {
            netTetherCallback_->TetherFailed(IfnameToType(ifName), ifName, TETHERING_ERR_IFACE_SET);
        }
        return TETHERING_INTERNAL_ERROR;
    }
    return TETHERING_NO_ERR;
}

int32_t NetTethering::UntetherByIface(const std::string &ifName)
{
    NETMGR_LOG_D("NetTethering::UntetherByIface, ifName: [%{public}s]", ifName.c_str());
    auto iter = ifaceMap_.find(ifName);
    if (iter == ifaceMap_.end()) {
        NETMGR_LOG_E("Untether unknown [%{public}s] iface, failed.", ifName.c_str());
        return TETHERING_UNKNOWN_IFACE_ERROR;
    }
    if (iter->second->GetLastState() != STATE_TETHERED) {
        NETMGR_LOG_E("Untether untethered [%{public}s] iface, failed.", ifName.c_str());
        return TETHERING_UNAVAIL_IFACE_ERROR;
    }
    iter->second->UnrequestedTether();
    return TETHERING_NO_ERR;
}

TetheringType NetTethering::IfnameToType(const std::string &ifName)
{
    if (ifName == "wlan1") {
        return TetheringType::TETHERING_WIFI;
    } else if (ifName == "bt-pan") {
        return TetheringType::TETHERING_BLUETOOTH;
    } else if (ifName == "rndis") {
        return TetheringType::TETHERING_USB;
    } else {
        return TetheringType::TETHERING_INVALID;
    }
}

void NetTethering::IfaceStateChange(const std::string& iface, int32_t state)
{
    if (state == STATE_TETHERED) {
        InterfaceTetheringActive(iface);
    } else if (state == STATE_AVAILABLE) {
        InterfaceTetheringInactive(iface);
    }
    return;
}

void NetTethering::RequestTethering(TetheringType type, bool enable)
{
    ChooseTetherType(type, enable);
}

void NetTethering::InterfaceTetheringActive(const std::string &iface)
{
    auto iter = ifaceMap_.find(iface);
    if (iter == ifaceMap_.end()) {
        NETMGR_LOG_E("Cannot found iface [%{public}s].", iface.c_str());
        return;
    }
    if (!NetTetherNetdUtils::GetInstance()->IpEnableForwarding(iface)) {
        iter->second->UnconfigAndUntetherIface();
        if (netTetherCallback_ != nullptr) {
            netTetherCallback_->TetherFailed(IfnameToType(iface), iface, TETHERING_ERR_IFACE_TETHER);
        }
        NETMGR_LOG_E("SetIfaceTether iface [%{public}s] failed!", iface.c_str());
        return;
    }
    if (!SetReqestedNetwork(iface)) {
        if (netTetherCallback_ != nullptr) {
            netTetherCallback_->TetherFailed(IfnameToType(iface), iface, TETHERING_ERR_IFACE_TETHER);
        }
        NETMGR_LOG_E("SetReqestedNetwork iface [%{public}s] failed!", iface.c_str());
    } else {
        if (netTetherCallback_ != nullptr) {
            netTetherCallback_->TetherSuccess(IfnameToType(iface), iface);
        }
        NETMGR_LOG_E("SetReqestedNetwork iface [%{public}s] success!", iface.c_str());
    }
}

void NetTethering::InterfaceTetheringInactive(const std::string &iface)
{
    if (!NetTetherNetdUtils::GetInstance()->IpDisableForwarding(iface)) {
        NETMGR_LOG_E("Disable ip forwarding failed!");
        return;
    }
    auto iter = ifaceMap_.find(iface);
    if (iter == ifaceMap_.end()) {
        NETMGR_LOG_E("Cannot found iface [%{public}s], RequestedNetworkChange failed.", iface.c_str());
        return;
    }
    iter->second->ClearUpstream();
    return;
}

bool NetTethering::SetReqestedNetwork(const std::string &iface)
{
    auto iter = ifaceMap_.find(iface);
    if (iter == ifaceMap_.end()) {
        NETMGR_LOG_E("Cannot found iface [%{public}s], SetReqestedNetwork failed.", iface.c_str());
        return false;
    }
    int32_t netId = netTetherRequestNetwork_->GetUpstreamNetId();
    if (!SetDnsForward(netId)) {
        NETMGR_LOG_E("SetDnsForward failed! netId: [%{public}d]", netId);
        NetTetherNetdUtils::GetInstance()->IpDisableForwarding(iface);
        iter->second->UnconfigAndUntetherIface();
        if (netTetherCallback_ != nullptr) {
            netTetherCallback_->TetherFailed(IfnameToType(iface), iface, TETHERING_ERR_IFACE_TETHER);
        }
        return false;
    }
    if (!RequestedNetworkChange(netId, iface)) {
        NETMGR_LOG_E("RequestedNetworkChange failed! netId: [%{public}d], iface: [%{public}s]", netId, iface.c_str());
        if (netTetherCallback_ != nullptr) {
            netTetherCallback_->TetherFailed(IfnameToType(iface), iface, TETHERING_ERR_IFACE_TETHER);
        }
        return false;
    }
    return true;
}

bool NetTethering::SetDnsForward(uint32_t netId)
{
    NetLinkInfo info = netTetherRequestNetwork_->GetUpstreamLinkInfo();
    return NetTetherNetdUtils::GetInstance()->TetherDnsSet(netId, info.dnsList_);
}

bool NetTethering::RequestedNetworkChange(uint32_t netId, const std::string &iface)
{
    currentRequestedNetId_ = netId;
    const NetLinkInfo &info = netTetherRequestNetwork_->GetUpstreamLinkInfo();
    auto iter = ifaceMap_.find(iface);
    if (iter == ifaceMap_.end()) {
        NETMGR_LOG_E("Cannot found iface [%{public}s], RequestedNetworkChange failed.", iface.c_str());
        return false;
    }
    return iter->second->UpstreamForward(info.ifaceName_);
}

int32_t NetTethering::RegisterTetheringEventCallback(const sptr<INetTetherCallback> &callback)
{
    netTetherCallback_ = callback;
    return TETHERING_NO_ERR;
}

void NetTethering::CallbackRequestNetworLost(int32_t netId)
{
    netTetherRequestNetwork_->RerequestNetwork();
}

void NetTethering::CallbackNetdResponseInterfaceAdd(const std::string &iface)
{
    if (iface.empty()) {
        NETMGR_LOG_E("Empty ifName!");
        return;
    }

    TrackNewInterface(iface);
    TetheringType ifType = IfnameToType(iface);
    auto iter = find(currentTether_.begin(), currentTether_.end(), ifType);
    if (iter != currentTether_.end()) {
        ChangeInterfaceState(iface, true);
    }
}

void NetTethering::CallbackNetdResponseInterfaceRemoved(const std::string &iface)
{
    TetheringType ifType = IfnameToType(iface);
    if (ifType == TetheringType::TETHERING_INVALID) {
        NETMGR_LOG_E("Invalid interface type with ifName: [%{public}s]", iface.c_str());
        return;
    }
    ChangeInterfaceState(iface, false);
    UntrackInterface(iface);
}
} // namespace NetManagerStandard
} // namespace OHOS