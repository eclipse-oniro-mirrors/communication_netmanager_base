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

#include "net_tether_iface_manager.h"
#include "net_tether_define.h"
#include "net_tether_ip_coordinator.h"
#include "net_tether_request_network.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherIfaceManager::TetherIDhcpResultNotify::TetherIDhcpResultNotify(NetTetherIfaceManager &netTetherIfaceManager)
    : netTetherIfaceManager_(netTetherIfaceManager) {}

NetTetherIfaceManager::TetherIDhcpResultNotify::~TetherIDhcpResultNotify() {}

void NetTetherIfaceManager::TetherIDhcpResultNotify::OnSuccess(int32_t status, const std::string &ifname,
    OHOS::Wifi::DhcpResult &result)
{
    NETMGR_LOG_D("Enter NetTetherIfaceManager::TetherIDhcpResultNotify::OnSuccess "
        "ifname=[%{public}s], iptype=[%{public}d], strYourCli=[%{public}s], "
        "strServer=[%{public}s], strSubnet=[%{public}s], strDns1=[%{public}s], "
        "strDns2=[%{public}s] strRouter1=[%{public}s] strRouter2=[%{public}s]",
        ifname.c_str(), result.iptype, result.strYourCli.c_str(), result.strServer.c_str(), result.strSubnet.c_str(),
        result.strDns1.c_str(), result.strDns2.c_str(), result.strRouter1.c_str(), result.strRouter2.c_str());
        netTetherIfaceManager_.GetIfaceType();
    return;
}

void NetTetherIfaceManager::TetherIDhcpResultNotify::OnFailed(int32_t status, const std::string &ifname,
    const std::string &reason)
{
    NETMGR_LOG_D("Enter EthernetManagement::EthDhcpResultNotify::OnFailed");
    return;
}

void NetTetherIfaceManager::TetherIDhcpResultNotify::OnSerExitNotify(const std::string &ifname)
{
    NETMGR_LOG_D("EthernetManagement::EthDhcpResultNotify::OnSerExitNotify");
    return;
}

NetTetherIfaceManager::NetTetherIfaceManager(const std::string &ifaceName, TetheringType ifaceType,
    const IfaceMgrCallback &callback, int32_t netId)
    : ifaceName_(ifaceName), ifaceType_(ifaceType), lastError_(TETHER_ERROR_NO_ERROR), lastState_(STATE_AVAILABLE),
      callback_(callback), upstreamNetId_(netId)
{
    dhcpService_ = std::make_unique<OHOS::Wifi::DhcpService>();
    dhcpResultNotify_ = std::make_unique<TetherIDhcpResultNotify>(*this);
}

NetTetherIfaceManager::~NetTetherIfaceManager()
{
}

void NetTetherIfaceManager::Init()
{
}

const std::string &NetTetherIfaceManager::GetIfaceName()
{
    return ifaceName_;
}

TetheringType NetTetherIfaceManager::GetIfaceType()
{
    return ifaceType_;
}

int32_t NetTetherIfaceManager::GetLastError()
{
    return lastError_;
}

int32_t NetTetherIfaceManager::GetLastState()
{
    return lastState_;
}

bool NetTetherIfaceManager::RequestedTether()
{
    ConfigAndTetherIface();
    if (lastError_ != TETHER_ERROR_NO_ERROR) {
        UnconfigAndUntetherIface();
        return false;
    }
    SendInterfaceState(STATE_TETHERED);
    return true;
}

bool NetTetherIfaceManager::UnrequestedTether()
{
    if (lastState_ != STATE_TETHERED) {
        NETMGR_LOG_E("[%{public}s] iface is not tether state, UnrequestedTether failed", ifaceName_.c_str());
        return false;
    }
    UnconfigAndUntetherIface();
    SendInterfaceState(STATE_AVAILABLE);
    return true;
}

void NetTetherIfaceManager::SendInterfaceState(int32_t newState)
{
    lastState_ = newState;
    callback_.OnIfaceStateChange(ifaceName_, newState);
    return;
}

void NetTetherIfaceManager::ConfigAndTetherIface()
{
    lastError_ = TETHER_ERROR_NO_ERROR;
    if (!ConfigureIPv4()) {
        lastError_ = TETHER_ERROR_IFACE_CFG_ERROR;
        return;
    }
    if (!TetherInterface(upstreamNetId_, ifaceName_, ipv4Addr_)) {
        lastError_ = TETHER_ERROR_TETHER_IFACE_ERROR;
    }
    return;
}

bool NetTetherIfaceManager::ConfigureIPv4()
{
    if (ifaceType_ == TETHERING_WIFI) {
        return true;
    }
    RequestIpv4Addr(ipv4Addr_);
    if (ipv4Addr_.InvalidAddr()) {
        NETMGR_LOG_E("Request ipv4 address failed.");
        return false;
    }
    if (ifaceType_ == TETHERING_BLUETOOTH) {
        return EnableDhcp(true);
    }

    if (!NetTetherNetdUtils::GetInstance()->AddIfaceConfigAndUp(ifaceName_, ipv4Addr_)) {
        NETMGR_LOG_E("Set interface config failed.");
        return false;
    }
    return EnableDhcp(true);
}

void NetTetherIfaceManager::RequestIpv4Addr(NetTetherIpAddress &ipAddr)
{
    if (ifaceType_ == TetheringType::TETHERING_BLUETOOTH) {
        ipAddr = NetTetherIpAddress(BLUETOOTH_IFACE_ADDR, BULETOOTH_PREFIX_LEN, true);
        return;
    }
    NetTetherIpCoordinator::GetInstance()->RequestIpv4Addr(ipAddr);
    return;
}

bool NetTetherIfaceManager::EnableDhcp(bool enable)
{
    if (enable) {
        return StartTetherDhcpService();
    } else {
        return StopTetherDhcpService();
    }
}

bool NetTetherIfaceManager::StartTetherDhcpService()
{
    if (dhcpService_ == nullptr) {
        NETMGR_LOG_E("dhcpService_ is nullptr, failed.");
        return false;
    }
    std::string ipAddr = ipv4Addr_.GetAddress();
    std::string::size_type pos = ipAddr.rfind(".");
    if (pos == std::string::npos) {
        NETMGR_LOG_E("Error ipAddr: [%{public}s].", ipAddr.c_str());
        return false;
    }
    std::string ipHead = ipAddr.substr(0, pos);
    OHOS::Wifi::DhcpRange range;
    constexpr int32_t IP_V4 = 0;
    range.iptype = IP_V4;
    range.strStartip = ipHead + ".3";
    range.strEndip = ipHead + ".254";
    range.strSubnet = "255.255.255.0";
    range.strTagName = ifaceName_;
    if (dhcpService_->SetDhcpRange(ifaceName_, range) != 0) {
        return false;
    }
    NETMGR_LOG_D("Set dhcp range : ifaceName[%{public}s] TagName[%{public}s] start ip[%{public}s] end ip[%{public}s]",
        ifaceName_.c_str(),
        range.strTagName.c_str(),
        range.strStartip.c_str(),
        range.strEndip.c_str());
    if (dhcpService_->StartDhcpServer(ifaceName_) != 0) {
        return false;
    }
    if (dhcpService_->GetDhcpSerProExit(ifaceName_, dhcpResultNotify_.get())) {
        return false;
    }
    return true;
}

bool NetTetherIfaceManager::StopTetherDhcpService()
{
    if (dhcpService_->RemoveAllDhcpRange(ifaceName_) != 0) {
        NETMGR_LOG_D("failed to remove [%{public}s] dhcp range.", ifaceName_.c_str());
    }
    if (dhcpService_->StopDhcpServer(ifaceName_) != 0) {
        NETMGR_LOG_D("Stop dhcp server failed!");
        return false;
    }
    return true;
}

bool NetTetherIfaceManager::TetherInterface(int32_t netId, const std::string &ifaceName,
    const NetTetherIpAddress &ipAddr)
{
    if (!NetTetherNetdUtils::GetInstance()->NetworkAddInterface(netId, ifaceName) ||
        !NetTetherNetdUtils::GetInstance()->SetTetherRoute(netId, ifaceName, ipAddr.GetAddress(),
            ipAddr.GetPrefixLength())) {
        return false;
    } else {
        return true;
    }
}

void NetTetherIfaceManager::UnconfigAndUntetherIface()
{
    if (!UntetherInterface(upstreamNetId_, ifaceName_, ipv4Addr_)) {
        lastError_ = TETHER_ERROR_UNTETHER_IFACE_ERROR;
        NETMGR_LOG_E("Untether interface [%{public}s] failed.", ifaceName_.c_str());
    }
    UnconfigureIPv4();
    return;
}

bool NetTetherIfaceManager::UntetherInterface(int32_t netId, const std::string &ifaceName,
    const NetTetherIpAddress &ipAddr)
{
    if (!NetTetherNetdUtils::GetInstance()->NetworkRemoveInterface(netId, ifaceName) ||
        !NetTetherNetdUtils::GetInstance()->DelTetherRoute(netId, ifaceName, ipAddr.GetAddress(),
            ipAddr.GetPrefixLength())) {
        return false;
    } else {
        return true;
    }
}

bool NetTetherIfaceManager::UnconfigureIPv4()
{
    if (ifaceType_ == TETHERING_WIFI) {
        return true;
    }
    if (ifaceType_ == TETHERING_BLUETOOTH) {
        return EnableDhcp(false);
    }
    ipv4Addr_ = NetTetherIpAddress();
    NetTetherNetdUtils::GetInstance()->DelIfaceConfigAndDown(ifaceName_, ipv4Addr_);
    return EnableDhcp(false);
}

bool NetTetherIfaceManager::UpstreamForward(const std::string &upstreamIface)
{
    if (upstreamIface_ == upstreamIface) {
        NETMGR_LOG_D("Current upstream interface has been forward.");
        return true;
    }
    if (upstreamIface.empty()) {
        ClearUpstream();
        return true;
    }
    upstreamIface_ = upstreamIface;
    if (!NetTetherNetdUtils::GetInstance()->TetherAddForward(ifaceName_, upstreamIface_) ||
        !NetTetherNetdUtils::GetInstance()->IpfwdAddInterfaceForward(ifaceName_, upstreamIface_)) {
        NETMGR_LOG_E("Upstream forward failed, upstreamIface_: [%{public}s]", upstreamIface_.c_str());
        UnconfigAndUntetherIface();
        SendInterfaceState(STATE_AVAILABLE);
        return false;
    } else {
        return true;
    }
}

void NetTetherIfaceManager::ClearUpstream()
{
    if (upstreamIface_.empty()) {
        return;
    }
    if (!NetTetherNetdUtils::GetInstance()->IpfwdRemoveInterfaceForward(ifaceName_, upstreamIface_)) {
        NETMGR_LOG_E("ipfwdRemoveInterfaceForward failed! ifaceName_: %s, upstreamIface_: [%{public}s]",
            ifaceName_.c_str(), upstreamIface_.c_str());
    }
    if (!NetTetherNetdUtils::GetInstance()->TetherRemoveForward(ifaceName_, upstreamIface_)) {
        NETMGR_LOG_E("tetherRemoveForward failed! ifaceName_: %s, upstreamIface_: [%{public}s]",
            ifaceName_.c_str(), upstreamIface_.c_str());
    }
    upstreamIface_.clear();
    return;
}
} // namespace NetManagerStandard
} // namespace OHOS