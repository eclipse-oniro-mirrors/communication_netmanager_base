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

#include "network.h"
#include "netsys_controller.h"
#include "net_mgr_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {
Network::Network(int32_t netId, uint32_t supplierId, NetDetectionHandler handler)
    : netId_(netId), supplierId_(supplierId), netCallback_(handler)
{
    StartDetectionThread();
}

Network::~Network()
{
    if (!ReleaseBasicNetwork()) {
        NETMGR_LOG_E("ReleaseBasicNetwork fail.");
    }
    if (netMonitor_ != nullptr) {
        netMonitor_->StopNetMonitorThread();
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
        NetsysController::GetInstance().NetworkCreatePhysical(netId_, 0);
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
    StartNetDetection();
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

    // Call netsys to add and remove interface
    if (!netLinkInfo.ifaceName_.empty()) {
        NetsysController::GetInstance().NetworkAddInterface(netId_, netLinkInfo.ifaceName_);
    }
    if (!netLinkInfo_.ifaceName_.empty()) {
        NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
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
    NETMGR_LOG_D("UpdateIpAddrs, old ip addrs: [%{public}s]", netLinkInfo_.ToStringAddr("").c_str());
    for (auto it = netLinkInfo_.netAddrList_.begin(); it != netLinkInfo_.netAddrList_.end(); ++it) {
        const struct INetAddr &inetAddr = *it;
        int32_t prefixLen = inetAddr.prefixlen_;
        if (prefixLen == 0) {
            prefixLen = Ipv4PrefixLen(inetAddr.netMask_);
        }
        NetsysController::GetInstance().InterfaceDelAddress(netLinkInfo_.ifaceName_, inetAddr.address_, prefixLen);
    }

    NETMGR_LOG_D("UpdateIpAddrs, new ip addrs: [%{public}s]", netLinkInfo.ToStringAddr("").c_str());
    for (auto it = netLinkInfo.netAddrList_.begin(); it != netLinkInfo.netAddrList_.end(); ++it) {
        const struct INetAddr &inetAddr = *it;
        int32_t prefixLen = inetAddr.prefixlen_;
        if (prefixLen == 0) {
            prefixLen = Ipv4PrefixLen(inetAddr.netMask_);
        }
        NetsysController::GetInstance().InterfaceAddAddress(netLinkInfo.ifaceName_, inetAddr.address_, prefixLen);
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
        NetsysController::GetInstance().NetworkRemoveRoute(netId_, route.iface_, destAddress, route.gateway_.address_);
    }

    NETMGR_LOG_D("UpdateRoutes, new routes: [%{public}s]", netLinkInfo.ToStringRoute("").c_str());
    for (auto it = netLinkInfo.routeList_.begin(); it != netLinkInfo.routeList_.end(); ++it) {
        const struct Route &route = *it;
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        NetsysController::GetInstance().NetworkAddRoute(netId_, route.iface_, destAddress, route.gateway_.address_);
    }
    NETMGR_LOG_D("Network UpdateRoutes out.");
}

void Network::UpdateDnses(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateDnses in.");
    std::vector<std::string> servers;
    std::vector<std::string> doamains;
    for (auto it = netLinkInfo.dnsList_.begin(); it != netLinkInfo.dnsList_.end(); ++it) {
        auto dns = *it;
        servers.push_back(dns.address_);
        doamains.push_back(dns.hostName_);
    }
    // Call netsys to set dns
    NetsysController::GetInstance().SetResolverConfig(netId_, 0, 1, servers, doamains);
    NETMGR_LOG_D("Network UpdateDnses out.");
}

void Network::UpdateMtu(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateMtu in.");
    if (netLinkInfo.mtu_ == netLinkInfo_.mtu_) {
        NETMGR_LOG_D("Network UpdateMtu out. same with before.");
        return;
    }

    NetsysController::GetInstance().InterfaceSetMtu(netLinkInfo.ifaceName_, netLinkInfo.mtu_);
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

void Network::StartNetDetection()
{
    NETMGR_LOG_D("Enter Network::StartNetDetection");
    if (netMonitor_ != nullptr) {
        netMonitor_->SignalNetMonitorThread(netLinkInfo_.ifaceName_);
    }
}

void Network::StopNetDetection()
{
    NETMGR_LOG_D("Enter Network::StopNetDetection");
    if (netMonitor_ != nullptr) {
        netMonitor_->StopNetMonitorThread();
    }
}

void Network::SetExternDetection()
{
    isExternDetection_ = true;
}

void Network::StartDetectionThread()
{
    netDetectionState_ = INVALID_DETECTION_STATE;
    netMonitor_ = std::make_unique<NetMonitor>(
        std::bind(&Network::HandleNetMonitorResult, this, std::placeholders::_1, std::placeholders::_2));
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_E("make_unique NetMonitor failed,netMonitor_ is null!");
        return;
    }
    netMonitor_->InitNetMonitorThread();
}

uint64_t Network::GetNetWorkMonitorResult()
{
    return netDetectionState_;
}

void Network::HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect)
{
    NETMGR_LOG_D("HandleNetMonitorResult, oldState[%{public}d], newState[%{public}d], isExternDetection[%{public}d]",
                 netDetectionState_, netDetectionState, isExternDetection_);
    bool needReport = false;
    if (netDetectionState_ != netDetectionState || isExternDetection_) {
        needReport = true;
        isExternDetection_ = false;
    }
    if (needReport) {
        NETMGR_LOG_D("need to report net detection result.");
        NotifyNetDetectionResult(NetDetectionResultConvert(static_cast<int32_t>(netDetectionState)), urlRedirect);
        if (netCallback_) {
            netCallback_(supplierId_, netDetectionState == VERIFICATION_STATE);
        }
    }
    netDetectionState_ = netDetectionState;
    urlRedirect_ = urlRedirect;
    NETMGR_LOG_D("HandleNetMonitorResult out.");
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
    NetsysController::GetInstance().SetDefaultNetWork(netId_);
}

void Network::ClearDefaultNetWorkNetId()
{
    NetsysController::GetInstance().ClearDefaultNetWorkNetId();
}
} // namespace NetManagerStandard
} // namespace OHOS
