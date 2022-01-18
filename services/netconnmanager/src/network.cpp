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
#include "netd_controller.h"
#include "net_mgr_log_wrapper.h"

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

bool Network::operator==(const Network &network) const
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
        NetdController::GetInstance().NetworkCreatePhysical(netId_, 0);
        NetdController::GetInstance().CreateNetworkCache(netId_);
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
        std::list<INetAddr>().swap(netLinkInfo_.netAddrList_);
        std::list<Route>().swap(netLinkInfo_.routeList_);
        NetdController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
        NetdController::GetInstance().NetworkDestroy(netId_);
        NetdController::GetInstance().DestoryNetworkCache(netId_);
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
    if (netLinkInfo.ifaceName_ == netLinkInfo_.ifaceName_) {
        return;
    }

    // Call netd to add and remove interface
    if (!netLinkInfo.ifaceName_.empty()) {
        NetdController::GetInstance().NetworkAddInterface(netId_, netLinkInfo.ifaceName_);
    }
    if (!netLinkInfo_.ifaceName_.empty()) {
        NetdController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
    }
    netLinkInfo_.ifaceName_ = netLinkInfo.ifaceName_;
}

void Network::UpdateIpAddrs(const NetLinkInfo &netLinkInfo)
{
    for (auto it = netLinkInfo.netAddrList_.begin(); it != netLinkInfo.netAddrList_.end(); ++it) {
        const struct INetAddr &inetAddr = *it;
        if (std::find(netLinkInfo_.netAddrList_.begin(), netLinkInfo_.netAddrList_.end(), *it) ==
            netLinkInfo_.netAddrList_.end()) {
            NetdController::GetInstance().InterfaceAddAddress(netLinkInfo.ifaceName_, inetAddr.address_,
                inetAddr.prefixlen_);
        }
    }

    for (auto it = netLinkInfo_.netAddrList_.begin(); it != netLinkInfo_.netAddrList_.end(); ++it) {
        const struct INetAddr &inetAddr = *it;
        if (std::find(netLinkInfo.netAddrList_.begin(), netLinkInfo.netAddrList_.end(), *it) ==
            netLinkInfo.netAddrList_.end()) {
            NetdController::GetInstance().InterfaceDelAddress(netLinkInfo.ifaceName_, inetAddr.address_,
                inetAddr.prefixlen_);
        }
    }
}

void Network::UpdateRoutes(const NetLinkInfo &netLinkInfo)
{
    for (auto it = netLinkInfo.routeList_.begin(); it != netLinkInfo.routeList_.end(); ++it) {
        const struct Route &route = *it;
        if (std::find(netLinkInfo_.routeList_.begin(), netLinkInfo_.routeList_.end(), *it) ==
            netLinkInfo_.routeList_.end()) {
                NetdController::GetInstance().NetworkAddRoute(
                    netId_, route.iface_, route.destination_.address_, route.gateway_.address_);
        }
    }

    for (auto it = netLinkInfo_.routeList_.begin(); it != netLinkInfo_.routeList_.end(); ++it) {
        const struct Route &route = *it;
        if (std::find(netLinkInfo.routeList_.begin(), netLinkInfo.routeList_.end(), *it) ==
            netLinkInfo.routeList_.end()) {
                NetdController::GetInstance().NetworkRemoveRoute(
                    netId_, route.iface_, route.destination_.address_, route.gateway_.address_);
        }
    }
}

void Network::UpdateDnses(const NetLinkInfo &netLinkInfo)
{
    std::vector<std::string> servers;
    std::vector<std::string> doamains;
    for (auto it = netLinkInfo.dnsList_.begin(); it != netLinkInfo.dnsList_.end(); ++it) {
        auto dns = *it;
        servers.push_back(dns.address_);
        doamains.push_back(dns.hostName_);
    }
    // Call netd to set dns
    NetdController::GetInstance().SetResolverConfig(netId_, 0, 1, servers, doamains);
}

void Network::UpdateMtu(const NetLinkInfo &netLinkInfo)
{
    if (netLinkInfo.mtu_ == netLinkInfo_.mtu_) {
        return;
    }

    NetdController::GetInstance().InterfaceSetMtu(netLinkInfo.ifaceName_, netLinkInfo.mtu_);
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
    bool needReport = false;
    if (netDetectionState_ != netDetectionState || isExternDetection_) {
        needReport = true;
        isExternDetection_ = false;
    }
    if (needReport) {
        NotifyNetDetectionResult(NetDetectionResultConvert(static_cast<int32_t>(netDetectionState)), urlRedirect);
        if (netCallback_) {
            netCallback_(supplierId_, netDetectionState == VERIFICATION_STATE);
        }
    }
    netDetectionState_ = netDetectionState;
    urlRedirect_ = urlRedirect;
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
    NetdController::GetInstance().SetDefaultNetWork(netId_);
}

void Network::ClearDefaultNetWorkNetId()
{
    NetdController::GetInstance().ClearDefaultNetWorkNetId();
}
} // namespace NetManagerStandard
} // namespace OHOS
