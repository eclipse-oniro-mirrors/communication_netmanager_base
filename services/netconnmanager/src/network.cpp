/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "network.h"

#include "common_event_support.h"
#include "event_report.h"
#include "netsys_controller.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "securec.h"
#include "broadcast_manager.h"

namespace OHOS {
namespace NetManagerStandard {
Network::Network(int32_t netId, uint32_t supplierId, NetDetectionHandler handler, NetBearType bearerType)
    : netId_(netId), supplierId_(supplierId), netCallback_(handler), netSupplierType_(bearerType)
{
    InitNetMonitor();
}

Network::~Network()
{
    if (!ReleaseBasicNetwork()) {
        NETMGR_LOG_E("ReleaseBasicNetwork fail.");
    }
}

int32_t Network::GetNetId() const
{
    return netId_;
}

bool Network::operator == (const Network &network) const
{
    return netId_ == network.netId_;
}

bool Network::UpdateBasicNetwork(bool isAvailable_)
{
    NETMGR_LOG_D("Enter UpdateBasicNetwork");
    if (isAvailable_) {
        return CreateBasicNetwork();
    } else {
        return ReleaseBasicNetwork();
    }
}

bool Network::CreateBasicNetwork()
{
    NETMGR_LOG_D("Enter CreateBasicNetwork");
    if (!isPhyNetCreated_) {
        NETMGR_LOG_D("Create physical network");
        // Create a physical network
        if (NetsysController::GetInstance().NetworkCreatePhysical(netId_, 0) != NETMANAGER_SUCCESS) {
            std::string errMsg = std::string("Create physical network failed, net id:").append(std::to_string(netId_));
            SendSupplierFaultHiSysEvent(FAULT_CREATE_PHYSICAL_NETWORK_FAILED, errMsg);
        }
        NetsysController::GetInstance().CreateNetworkCache(netId_);
        isPhyNetCreated_ = true;
    }
    return true;
}

bool Network::ReleaseBasicNetwork()
{
    NETMGR_LOG_D("Enter ReleaseBasicNetwork");
    if (isPhyNetCreated_) {
        NETMGR_LOG_D("Destroy physical network");
        StopNetDetection();
        for (auto it = netLinkInfo_.netAddrList_.begin(); it != netLinkInfo_.netAddrList_.end(); ++it) {
            const struct INetAddr &inetAddr = *it;
            int32_t prefixLen = inetAddr.prefixlen_;
            if (prefixLen == 0) {
                prefixLen = Ipv4PrefixLen(inetAddr.netMask_);
            }
            NetsysController::GetInstance().InterfaceDelAddress(netLinkInfo_.ifaceName_, inetAddr.address_, prefixLen);
        }
        NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
        NetsysController::GetInstance().NetworkDestroy(netId_);
        NetsysController::GetInstance().DestroyNetworkCache(netId_);
        netLinkInfo_.Initialize();
        isPhyNetCreated_ = false;
    }
    return true;
}

bool Network::UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("update net link information process");
    UpdateInterfaces(netLinkInfo);
    UpdateIpAddrs(netLinkInfo);
    UpdateRoutes(netLinkInfo);
    UpdateDnses(netLinkInfo);
    UpdateMtu(netLinkInfo);
    netLinkInfo_ = netLinkInfo;
    StartNetDetection(false);
    return true;
}

NetLinkInfo Network::GetNetLinkInfo() const
{
    return netLinkInfo_;
}

void Network::UpdateInterfaces(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateInterfaces in.");
    if (netLinkInfo.ifaceName_ == netLinkInfo_.ifaceName_) {
        NETMGR_LOG_D("Network UpdateInterfaces out. same with before.");
        return;
    }

    int32_t ret = NETMANAGER_SUCCESS;
    // Call netsys to add and remove interface
    if (!netLinkInfo.ifaceName_.empty()) {
        ret = NetsysController::GetInstance().NetworkAddInterface(netId_, netLinkInfo.ifaceName_);
        if (ret != NETMANAGER_SUCCESS) {
            std::string errMsg = "Add network interface failed";
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
        }
    }
    if (!netLinkInfo_.ifaceName_.empty()) {
        ret = NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
        if (ret != NETMANAGER_SUCCESS) {
            std::string errMsg = "Remove network interface failed";
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
        }
    }
    netLinkInfo_.ifaceName_ = netLinkInfo.ifaceName_;
    NETMGR_LOG_D("Network UpdateInterfaces out.");
}

int32_t Network::Ipv4PrefixLen(const std::string &ip)
{
    constexpr int32_t BIT32 = 32;
    constexpr int32_t BIT24 = 24;
    constexpr int32_t BIT16 = 16;
    constexpr int32_t BIT8 = 8;
    if (ip.empty()) {
        return 0;
    }
    int32_t ret = 0;
    uint32_t ipNum = 0;
    uint8_t c1 = 0;
    uint8_t c2 = 0;
    uint8_t c3 = 0;
    uint8_t c4 = 0;
    int32_t cnt = 0;
    ret = sscanf_s(ip.c_str(), "%hhu.%hhu.%hhu.%hhu", &c1, &c2, &c3, &c4);
    if (ret != sizeof(int32_t)) {
        return 0;
    }
    ipNum = (c1 << static_cast<uint32_t>(BIT24)) | (c2 << static_cast<uint32_t>(BIT16)) |
            (c3 << static_cast<uint32_t>(BIT8)) | c4;
    if (ipNum == 0xFFFFFFFF) {
        return BIT32;
    }
    if (ipNum == 0xFFFFFF00) {
        return BIT24;
    }
    if (ipNum == 0xFFFF0000) {
        return BIT16;
    }
    if (ipNum == 0xFF000000) {
        return BIT8;
    }
    for (int32_t i = 0; i < BIT32; i++) {
        if ((ipNum << i) & 0x80000000) {
            cnt++;
        } else {
            break;
        }
    }
    return cnt;
}

void Network::UpdateIpAddrs(const NetLinkInfo &netLinkInfo)
{
    // netLinkInfo_ represents the old, netLinkInfo represents the new
    // Update: remove old Ips first, then add the new Ips
    NETMGR_LOG_D("UpdateIpAddrs, old ip addrs: ...");
    for (auto it = netLinkInfo_.netAddrList_.begin(); it != netLinkInfo_.netAddrList_.end(); ++it) {
        const struct INetAddr &inetAddr = *it;
        int32_t prefixLen = inetAddr.prefixlen_;
        if (prefixLen == 0) {
            prefixLen = Ipv4PrefixLen(inetAddr.netMask_);
        }
        int32_t ret =  NetsysController::GetInstance().InterfaceDelAddress(
            netLinkInfo_.ifaceName_, inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            std::string errMsg = "Delete network ip address failed";
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
        }
    }

    NETMGR_LOG_D("UpdateIpAddrs, new ip addrs: ...");
    for (auto it = netLinkInfo.netAddrList_.begin(); it != netLinkInfo.netAddrList_.end(); ++it) {
        const struct INetAddr &inetAddr = *it;
        int32_t prefixLen = inetAddr.prefixlen_;
        if (prefixLen == 0) {
            prefixLen = Ipv4PrefixLen(inetAddr.netMask_);
        }
        int32_t ret =  NetsysController::GetInstance().InterfaceAddAddress(
            netLinkInfo.ifaceName_, inetAddr.address_, prefixLen);
        if (ret != NETMANAGER_SUCCESS) {
            std::string errMsg = "Add network ip address failed";
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
        }
    }
    NETMGR_LOG_D("Network UpdateIpAddrs out.");
}

void Network::UpdateRoutes(const NetLinkInfo &netLinkInfo)
{
    // netLinkInfo_ contains the old routes info, netLinkInfo contains the new routes info
    // Update: remove old routes first, then add the new routes
    NETMGR_LOG_D("UpdateRoutes, old routes: [%{public}s]", netLinkInfo_.ToStringRoute("").c_str());
    for (auto it = netLinkInfo_.routeList_.begin(); it != netLinkInfo_.routeList_.end(); ++it) {
        const struct Route &route = *it;
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        int32_t ret = NetsysController::GetInstance().NetworkRemoveRoute(
            netId_, route.iface_, destAddress, route.gateway_.address_);
        if (ret != NETMANAGER_SUCCESS) {
            std::string errMsg = "Remove network routes failed";
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
        }
    }

    NETMGR_LOG_D("UpdateRoutes, new routes: [%{public}s]", netLinkInfo.ToStringRoute("").c_str());
    for (auto it = netLinkInfo.routeList_.begin(); it != netLinkInfo.routeList_.end(); ++it) {
        const struct Route &route = *it;
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        int32_t ret = NetsysController::GetInstance().NetworkAddRoute(
            netId_, route.iface_, destAddress, route.gateway_.address_);
        if (ret != NETMANAGER_SUCCESS) {
            std::string errMsg = "Network add routes failed";
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
        }
    }
    NETMGR_LOG_D("Network UpdateRoutes out.");
    if (netLinkInfo.routeList_.size() == 0) {
        std::string errMsg = "Update netlink routes failed,routes list is empty";
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
    }
}

void Network::UpdateDnses(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateDnses in.");
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    for (const auto &dns : netLinkInfo.dnsList_) {
        servers.emplace_back(dns.address_);
        domains.emplace_back(dns.hostName_);
    }
    // Call netsys to set dns, use default timeout and retry
    int32_t ret = NetsysController::GetInstance().SetResolverConfig(netId_, 0, 0, servers, domains);
    if (ret != NETMANAGER_SUCCESS) {
        std::string errMsg = "Set network resolver config failed";
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
    }
    NETMGR_LOG_D("Network UpdateDnses out.");
    if (netLinkInfo.dnsList_.size() == 0) {
        std::string errMsg = "Update netlink dns failed,dns list is empty";
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
    }
}

void Network::UpdateMtu(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateMtu in.");
    if (netLinkInfo.mtu_ == netLinkInfo_.mtu_) {
        NETMGR_LOG_D("Network UpdateMtu out. same with before.");
        return;
    }

    int32_t ret = NetsysController::GetInstance().InterfaceSetMtu(netLinkInfo.ifaceName_, netLinkInfo.mtu_);
    if (ret != NETMANAGER_SUCCESS) {
        std::string errMsg = "Update network mtu failed";
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, errMsg);
    }
    NETMGR_LOG_D("Network UpdateMtu out.");
}

void Network::RegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter RegisterNetDetectionCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return;
    }

    for (auto iter = netDetectionRetCallback_.begin(); iter != netDetectionRetCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            NETMGR_LOG_D("netDetectionRetCallback_ had this callback");
            return;
        }
    }

    netDetectionRetCallback_.emplace_back(callback);
}

int32_t Network::UnRegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter UnRegisterNetDetectionCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    for (auto iter = netDetectionRetCallback_.begin(); iter != netDetectionRetCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            netDetectionRetCallback_.erase(iter);
            break;
        }
    }

    return ERR_NONE;
}

void Network::StartNetDetection(bool needReport)
{
    NETMGR_LOG_D("Enter Network::StartNetDetection");
    if (netMonitor_ != nullptr) {
        netMonitor_->Start(needReport);
    }
}

void Network::StopNetDetection()
{
    NETMGR_LOG_D("Enter Network::StopNetDetection");
    if (netMonitor_ != nullptr) {
        netMonitor_->Stop();
    }
}

void Network::InitNetMonitor()
{
    netMonitor_ = std::make_unique<NetMonitor>(netId_,
        std::bind(&Network::HandleNetMonitorResult, this, std::placeholders::_1, std::placeholders::_2));
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_E("make_unique NetMonitor failed,netMonitor_ is null!");
        return;
    }
}

uint64_t Network::GetNetWorkMonitorResult()
{
    return netMonitor_->GetDetectionResult();
}

void Network::HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect)
{
    NETMGR_LOG_D("HandleNetMonitorResult, netDetectionState[%{public}d]", netDetectionState);
    NotifyNetDetectionResult(NetDetectionResultConvert(static_cast<int32_t>(netDetectionState)), urlRedirect);
    if (netCallback_) {
        netCallback_(supplierId_, netDetectionState == VERIFICATION_STATE);
    }
}

void Network::NotifyNetDetectionResult(NetDetectionResultCode detectionResult, const std::string &urlRedirect)
{
    for (auto callback : netDetectionRetCallback_) {
        NETMGR_LOG_D("start callback!");
        callback->OnNetDetectionResultChanged(detectionResult, urlRedirect);
    }
}

NetDetectionResultCode Network::NetDetectionResultConvert(int32_t internalRet)
{
    switch (internalRet) {
        case static_cast<int32_t>(INVALID_DETECTION_STATE):
            return NET_DETECTION_FAIL;
        case static_cast<int32_t>(VERIFICATION_STATE):
            return NET_DETECTION_SUCCESS;
        case static_cast<int32_t>(CAPTIVE_PORTAL_STATE):
            return NET_DETECTION_CAPTIVE_PORTAL;
        default:
            break;
    }
    return NET_DETECTION_FAIL;
}

void Network::SetDefaultNetWork()
{
    int32_t ret = NetsysController::GetInstance().SetDefaultNetWork(netId_);
    if (ret != NETMANAGER_SUCCESS) {
        std::string errMsg = "Set default network failed";
        SendSupplierFaultHiSysEvent(FAULT_SET_DEFAULT_NETWORK_FAILED, errMsg);
    }
}

void Network::ClearDefaultNetWorkNetId()
{
    int32_t ret = NetsysController::GetInstance().ClearDefaultNetWorkNetId();
    if (ret != NETMANAGER_SUCCESS) {
        std::string errMsg = "Clear default network failed";
        SendSupplierFaultHiSysEvent(FAULT_CLEAR_DEFAULT_NETWORK_FAILED, errMsg);
    }
}

bool Network::IsConnecting() const
{
    return state_ == NET_CONN_STATE_CONNECTING;
}

bool Network::IsConnected() const
{
    return state_ == NET_CONN_STATE_CONNECTED;
}

void Network::UpdateNetConnState(NetConnState netConnState)
{
    if (state_ == netConnState) {
        NETMGR_LOG_E("Ignore same network state changed.");
        return;
    }
    NetConnState oldState = state_;
    switch (netConnState) {
        case NET_CONN_STATE_IDLE:
        case NET_CONN_STATE_CONNECTING:
        case NET_CONN_STATE_CONNECTED:
        case NET_CONN_STATE_DISCONNECTING:
        case NET_CONN_STATE_DISCONNECTED:
            state_ = netConnState;
            break;
        default:
            state_ = NET_CONN_STATE_UNKNOWN;
            break;
    }

    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE;
    info.data = "Net Manager Connection State Changed";
    info.code = static_cast<int32_t>(netConnState);
    info.ordered = true;
    std::map<std::string, int32_t> param = {{"NetType", static_cast<int32_t>(netSupplierType_)}};
    DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);
    NETMGR_LOG_D("Network[%{public}d] state changed, from [%{public}d] to [%{public}d]", netId_, oldState, state_);
}

void Network::SendSupplierFaultHiSysEvent(NetConnSupplerFault errorType, const std::string &errMsg)
{
    struct EventInfo eventInfo = {
        .netlinkInfo = netLinkInfo_.ToString(" "),
        .supplierId = static_cast<int32_t>(supplierId_),
        .errorType = static_cast<int32_t>(errorType),
        .errorMsg = errMsg
    };
    EventReport::SendSupplierFaultEvent(eventInfo);
}
} // namespace NetManagerStandard
} // namespace OHOS
