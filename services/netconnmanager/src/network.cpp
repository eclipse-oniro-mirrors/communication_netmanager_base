/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "common_event_support.h"

#include "broadcast_manager.h"
#include "event_report.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_client.h"
#include "netmanager_base_common_utils.h"
#include "netsys_controller.h"
#include "network.h"
#include "route_utils.h"
#include "securec.h"
#include "net_conn_service_iface.h"

using namespace OHOS::NetManagerStandard::CommonUtils;

namespace OHOS {
namespace NetManagerStandard {
namespace {
// hisysevent error messgae
constexpr const char *ERROR_MSG_CREATE_PHYSICAL_NETWORK_FAILED = "Create physical network failed, net id:";
constexpr const char *ERROR_MSG_CREATE_VIRTUAL_NETWORK_FAILED = "Create virtual network failed, net id:";
constexpr const char *ERROR_MSG_ADD_NET_INTERFACE_FAILED = "Add network interface failed";
constexpr const char *ERROR_MSG_REMOVE_NET_INTERFACE_FAILED = "Remove network interface failed";
constexpr const char *ERROR_MSG_DELETE_NET_IP_ADDR_FAILED = "Delete network ip address failed";
constexpr const char *ERROR_MSG_ADD_NET_IP_ADDR_FAILED = "Add network ip address failed";
constexpr const char *ERROR_MSG_REMOVE_NET_ROUTES_FAILED = "Remove network routes failed";
constexpr const char *ERROR_MSG_ADD_NET_ROUTES_FAILED = "Add network routes failed";
constexpr const char *ERROR_MSG_UPDATE_NET_ROUTES_FAILED = "Update netlink routes failed,routes list is empty";
constexpr const char *ERROR_MSG_SET_NET_RESOLVER_FAILED = "Set network resolver config failed";
constexpr const char *ERROR_MSG_UPDATE_NET_DNSES_FAILED = "Update netlink dns failed,dns list is empty";
constexpr const char *ERROR_MSG_SET_NET_MTU_FAILED = "Set netlink interface mtu failed";
constexpr const char *ERROR_MSG_SET_NET_TCP_BUFFER_SIZE_FAILED = "Set netlink tcp buffer size failed";
constexpr const char *ERROR_MSG_UPDATE_STATS_CACHED = "force update kernel map stats cached failed";
constexpr const char *ERROR_MSG_SET_DEFAULT_NETWORK_FAILED = "Set default network failed";
constexpr const char *ERROR_MSG_CLEAR_DEFAULT_NETWORK_FAILED = "Clear default network failed";
constexpr const char *LOCAL_ROUTE_NEXT_HOP = "0.0.0.0";
constexpr const char *LOCAL_ROUTE_IPV6_DESTINATION = "::";
constexpr int32_t ERRNO_EADDRNOTAVAIL = -99;
} // namespace

Network::Network(int32_t netId, uint32_t supplierId, const NetDetectionHandler &handler, NetBearType bearerType,
                 const std::shared_ptr<NetConnEventHandler> &eventHandler)
    : netId_(netId),
      supplierId_(supplierId),
      netCallback_(handler),
      netSupplierType_(bearerType),
      eventHandler_(eventHandler)
{
}

int32_t Network::GetNetId() const
{
    return netId_;
}

uint32_t Network::GetSupplierId() const
{
    return supplierId_;
}

bool Network::operator==(const Network &network) const
{
    return netId_ == network.netId_;
}

bool Network::UpdateBasicNetwork(bool isAvailable_)
{
    NETMGR_LOG_D("Enter UpdateBasicNetwork");
    if (isAvailable_) {
        if (netSupplierType_ == BEARER_VPN) {
            return CreateVirtualNetwork();
        }
        return CreateBasicNetwork();
    } else {
        if (netSupplierType_ == BEARER_VPN) {
            return ReleaseVirtualNetwork();
        }
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
            std::string errMsg = std::string(ERROR_MSG_CREATE_PHYSICAL_NETWORK_FAILED).append(std::to_string(netId_));
            SendSupplierFaultHiSysEvent(FAULT_CREATE_PHYSICAL_NETWORK_FAILED, errMsg);
        }
        NetsysController::GetInstance().CreateNetworkCache(netId_);
        isPhyNetCreated_ = true;
    }
    return true;
}

bool Network::CreateVirtualNetwork()
{
    NETMGR_LOG_D("Enter create virtual network");
    if (!isVirtualCreated_) {
        // Create a virtual network here
        bool hasDns = netLinkInfo_.dnsList_.size() ? true : false;
        if (NetsysController::GetInstance().NetworkCreateVirtual(netId_, hasDns) != NETMANAGER_SUCCESS) {
            std::string errMsg = std::string(ERROR_MSG_CREATE_VIRTUAL_NETWORK_FAILED).append(std::to_string(netId_));
            SendSupplierFaultHiSysEvent(FAULT_CREATE_VIRTUAL_NETWORK_FAILED, errMsg);
        }
        NetsysController::GetInstance().CreateNetworkCache(netId_);
        isVirtualCreated_ = true;
    }
    return true;
}

bool Network::IsAddrInOtherNetwork(const INetAddr &netAddr)
{
    return NetConnServiceIface().IsAddrInOtherNetwork(netLinkInfo_.ifaceName_, netId_, netAddr);
}

bool Network::IsIfaceNameInUse()
{
    return NetConnServiceIface().IsIfaceNameInUse(netLinkInfo_.ifaceName_, netId_);
}

bool Network::ReleaseBasicNetwork()
{
    NETMGR_LOG_D("Enter ReleaseBasicNetwork");
    if (isPhyNetCreated_) {
        NETMGR_LOG_D("Destroy physical network");
        StopNetDetection();
        if (!IsIfaceNameInUse()) {
            for (const auto &inetAddr : netLinkInfo_.netAddrList_) {
                int32_t prefixLen = inetAddr.prefixlen_ == 0 ? Ipv4PrefixLen(inetAddr.netMask_) : inetAddr.prefixlen_;
                NetsysController::GetInstance().DelInterfaceAddress(
                    netLinkInfo_.ifaceName_, inetAddr.address_, prefixLen);
            }
        }
        for (const auto &route : netLinkInfo_.routeList_) {
            std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
            NetsysController::GetInstance().NetworkRemoveRoute(netId_, route.iface_, destAddress,
                                                               route.gateway_.address_);
            if (route.destination_.address_ != LOCAL_ROUTE_NEXT_HOP &&
                route.destination_.address_ != LOCAL_ROUTE_IPV6_DESTINATION) {
                auto family = GetAddrFamily(route.destination_.address_);
                std::string nextHop = (family == AF_INET6) ? "" : LOCAL_ROUTE_NEXT_HOP;
                NetsysController::GetInstance().NetworkRemoveRoute(LOCAL_NET_ID, route.iface_, destAddress, nextHop);
            }
        }
        NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
        NetsysController::GetInstance().NetworkDestroy(netId_);
        NetsysController::GetInstance().DestroyNetworkCache(netId_);
        netLinkInfo_.Initialize();
        isPhyNetCreated_ = false;
    }
    return true;
}

bool Network::ReleaseVirtualNetwork()
{
    NETMGR_LOG_D("Enter release virtual network");
    if (isVirtualCreated_) {
        for (const auto &inetAddr : netLinkInfo_.netAddrList_) {
            int32_t prefixLen = inetAddr.prefixlen_;
            if (prefixLen == 0) {
                prefixLen = Ipv4PrefixLen(inetAddr.netMask_);
            }
            NetsysController::GetInstance().DelInterfaceAddress(netLinkInfo_.ifaceName_, inetAddr.address_, prefixLen);
        }
        NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
        NetsysController::GetInstance().NetworkDestroy(netId_);
        NetsysController::GetInstance().DestroyNetworkCache(netId_);
        netLinkInfo_.Initialize();
        isVirtualCreated_ = false;
    }
    return true;
}

bool Network::UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("update net link information process");
    UpdateInterfaces(netLinkInfo);
    UpdateIpAddrs(netLinkInfo);
    UpdateRoutes(netLinkInfo);
    UpdateDns(netLinkInfo);
    UpdateMtu(netLinkInfo);
    UpdateTcpBufferSize(netLinkInfo);
    UpdateStatsCached(netLinkInfo);

    netLinkInfo_ = netLinkInfo;
    if (netSupplierType_ != BEARER_VPN &&
        netCaps_.find(NetCap::NET_CAPABILITY_INTERNET) != netCaps_.end()) {
        StartNetDetection(false);
    }
    return true;
}

NetLinkInfo Network::GetNetLinkInfo() const
{
    NetLinkInfo linkInfo = netLinkInfo_;
    for (auto iter = linkInfo.routeList_.begin(); iter != linkInfo.routeList_.end();) {
        if (iter->destination_.address_ == LOCAL_ROUTE_NEXT_HOP ||
            iter->destination_.address_ == LOCAL_ROUTE_IPV6_DESTINATION) {
            ++iter;
            continue;
        }
        iter = linkInfo.routeList_.erase(iter);
    }
    return linkInfo;
}

void Network::UpdateInterfaces(const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_LOG_D("Network UpdateInterfaces in.");
    if (newNetLinkInfo.ifaceName_ == netLinkInfo_.ifaceName_) {
        NETMGR_LOG_D("Network UpdateInterfaces out. same with before.");
        return;
    }

    int32_t ret = NETMANAGER_SUCCESS;
    // Call netsys to add and remove interface
    if (!newNetLinkInfo.ifaceName_.empty()) {
        ret = NetsysController::GetInstance().NetworkAddInterface(netId_, newNetLinkInfo.ifaceName_);
        if (ret != NETMANAGER_SUCCESS) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_ADD_NET_INTERFACE_FAILED);
        }
    }
    if (!netLinkInfo_.ifaceName_.empty()) {
        ret = NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
        if (ret != NETMANAGER_SUCCESS) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_REMOVE_NET_INTERFACE_FAILED);
        }
    }
    netLinkInfo_.ifaceName_ = newNetLinkInfo.ifaceName_;
    NETMGR_LOG_D("Network UpdateInterfaces out.");
}

void Network::UpdateIpAddrs(const NetLinkInfo &newNetLinkInfo)
{
    // netLinkInfo_ represents the old, netLinkInfo represents the new
    // Update: remove old Ips first, then add the new Ips
    NETMGR_LOG_I("UpdateIpAddrs, old ip addrs size: [%{public}zu]", netLinkInfo_.netAddrList_.size());
    for (const auto &inetAddr : netLinkInfo_.netAddrList_) {
        if (IsAddrInOtherNetwork(inetAddr)) {
            continue;
        }
        if (newNetLinkInfo.HasNetAddr(inetAddr)) {
            NETMGR_LOG_W("Same ip address:[%{public}s], there is not need to be deleted",
                         CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        auto family = GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_) :
            ((family == AF_INET6) ? Ipv6PrefixLen(inetAddr.netMask_) : Ipv4PrefixLen(inetAddr.netMask_));
        int32_t ret = NetsysController::GetInstance().DelInterfaceAddress(netLinkInfo_.ifaceName_,
            inetAddr.address_, prefixLen);
        if (NETMANAGER_SUCCESS != ret) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_DELETE_NET_IP_ADDR_FAILED);
        }

        if ((ret == ERRNO_EADDRNOTAVAIL) || (ret == 0)) {
            NETMGR_LOG_W("remove route info of ip address:[%{public}s]",
                CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            netLinkInfo_.routeList_.remove_if([family](const Route &route) {
                INetAddr::IpType addrFamily = INetAddr::IpType::UNKNOWN;
                if (family == AF_INET) {
                    addrFamily = INetAddr::IpType::IPV4;
                } else if (family == AF_INET6) {
                    addrFamily = INetAddr::IpType::IPV6;
                }
                return route.destination_.type_ == addrFamily;
            });
        }
    }

    HandleUpdateIpAddrs(newNetLinkInfo);
}

void Network::HandleUpdateIpAddrs(const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_LOG_I("HandleUpdateIpAddrs, new ip addrs size: [%{public}zu]", newNetLinkInfo.netAddrList_.size());
    for (const auto &inetAddr : newNetLinkInfo.netAddrList_) {
        if (IsAddrInOtherNetwork(inetAddr)) {
            continue;
        }
        if (netLinkInfo_.HasNetAddr(inetAddr)) {
            NETMGR_LOG_W("Same ip address:[%{public}s], there is no need to add it again",
                         CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        auto family = GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_) :
            ((family == AF_INET6) ? Ipv6PrefixLen(inetAddr.netMask_) : Ipv4PrefixLen(inetAddr.netMask_));
        if (NETMANAGER_SUCCESS != NetsysController::GetInstance().AddInterfaceAddress(newNetLinkInfo.ifaceName_,
                                                                                      inetAddr.address_, prefixLen)) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_ADD_NET_IP_ADDR_FAILED);
        }
    }
}

void Network::UpdateRoutes(const NetLinkInfo &newNetLinkInfo)
{
    // netLinkInfo_ contains the old routes info, netLinkInfo contains the new routes info
    // Update: remove old routes first, then add the new routes
    NETMGR_LOG_D("UpdateRoutes, old routes: [%{public}s]", netLinkInfo_.ToStringRoute("").c_str());
    for (const auto &route : netLinkInfo_.routeList_) {
        if (newNetLinkInfo.HasRoute(route)) {
            NETMGR_LOG_W("Same route:[%{public}s]  ifo, there is not need to be deleted",
                         CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }
        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetsysController::GetInstance().NetworkRemoveRoute(netId_, route.iface_, destAddress,
                                                                      route.gateway_.address_);
        int32_t res = NETMANAGER_SUCCESS;
        if (netSupplierType_ != BEARER_VPN && route.destination_.address_ != LOCAL_ROUTE_NEXT_HOP &&
            route.destination_.address_ != LOCAL_ROUTE_IPV6_DESTINATION) {
            auto family = GetAddrFamily(route.destination_.address_);
            std::string nextHop = (family == AF_INET6) ? "" : LOCAL_ROUTE_NEXT_HOP;
            res = NetsysController::GetInstance().NetworkRemoveRoute(LOCAL_NET_ID, route.iface_, destAddress, nextHop);
        }
        if (ret != NETMANAGER_SUCCESS || res != NETMANAGER_SUCCESS) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_REMOVE_NET_ROUTES_FAILED);
        }
    }

    NETMGR_LOG_D("UpdateRoutes, new routes: [%{public}s]", newNetLinkInfo.ToStringRoute("").c_str());
    for (const auto &route : newNetLinkInfo.routeList_) {
        if (netLinkInfo_.HasRoute(route)) {
            NETMGR_LOG_W("Same route:[%{public}s]  ifo, there is no need to add it again",
                         CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }

        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret =
            NetsysController::GetInstance().NetworkAddRoute(netId_, route.iface_, destAddress, route.gateway_.address_);
        int32_t res = NETMANAGER_SUCCESS;
        if (netSupplierType_ != BEARER_VPN && route.destination_.address_ != LOCAL_ROUTE_NEXT_HOP &&
            route.destination_.address_ != LOCAL_ROUTE_IPV6_DESTINATION) {
            auto family = GetAddrFamily(route.destination_.address_);
            std::string nextHop = (family == AF_INET6) ? "" : LOCAL_ROUTE_NEXT_HOP;
            res = NetsysController::GetInstance().NetworkAddRoute(LOCAL_NET_ID, route.iface_, destAddress, nextHop);
        }
        if (ret != NETMANAGER_SUCCESS || res != NETMANAGER_SUCCESS) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_ADD_NET_ROUTES_FAILED);
        }
    }
    NETMGR_LOG_D("Network UpdateRoutes out.");
    if (newNetLinkInfo.routeList_.empty()) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_UPDATE_NET_ROUTES_FAILED);
    }
}

void Network::UpdateDns(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateDns in.");
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    for (const auto &dns : netLinkInfo.dnsList_) {
        servers.emplace_back(dns.address_);
        domains.emplace_back(dns.hostName_);
    }
    // Call netsys to set dns, use default timeout and retry
    int32_t ret = NetsysController::GetInstance().SetResolverConfig(netId_, 0, 0, servers, domains);
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_SET_NET_RESOLVER_FAILED);
    }
    NETMGR_LOG_D("Network UpdateDns out.");
    if (netLinkInfo.dnsList_.empty()) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_UPDATE_NET_DNSES_FAILED);
    }
}

void Network::UpdateMtu(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateMtu in.");
    if (netLinkInfo.mtu_ == netLinkInfo_.mtu_) {
        NETMGR_LOG_D("Network UpdateMtu out. same with before.");
        return;
    }

    int32_t ret = NetsysController::GetInstance().SetInterfaceMtu(netLinkInfo.ifaceName_, netLinkInfo.mtu_);
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_SET_NET_MTU_FAILED);
    }
    NETMGR_LOG_D("Network UpdateMtu out.");
}

void Network::UpdateTcpBufferSize(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateTcpBufferSize in.");
    if (netLinkInfo.tcpBufferSizes_ == netLinkInfo_.tcpBufferSizes_) {
        NETMGR_LOG_D("Network UpdateTcpBufferSize out. same with before.");
        return;
    }
    int32_t ret = NetsysController::GetInstance().SetTcpBufferSizes(netLinkInfo.tcpBufferSizes_);
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_SET_NET_TCP_BUFFER_SIZE_FAILED);
    }
    NETMGR_LOG_D("Network UpdateTcpBufferSize out.");
}

void Network::UpdateStatsCached(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateStatsCached in.");
    if (netLinkInfo.ifaceName_ == netLinkInfo_.ifaceName_ && netLinkInfo.ident_ == netLinkInfo_.ident_) {
        NETMGR_LOG_D("Network UpdateStatsCached out. same with before");
        return;
    }
    int32_t ret = NetStatsClient::GetInstance().UpdateStatsData();
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_UPDATE_STATS_CACHED);
    }
    NETMGR_LOG_D("Network UpdateStatsCached out.");
}

void Network::RegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_I("Enter RNDCB");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return;
    }

    for (const auto &iter : netDetectionRetCallback_) {
        if (callback->AsObject().GetRefPtr() == iter->AsObject().GetRefPtr()) {
            NETMGR_LOG_D("netDetectionRetCallback_ had this callback");
            return;
        }
    }

    netDetectionRetCallback_.emplace_back(callback);
}

int32_t Network::UnRegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_I("Enter URNDCB");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    for (auto iter = netDetectionRetCallback_.begin(); iter != netDetectionRetCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            netDetectionRetCallback_.erase(iter);
            return NETMANAGER_SUCCESS;
        }
    }

    return NETMANAGER_SUCCESS;
}

void Network::StartNetDetection(bool needReport)
{
    NETMGR_LOG_I("Enter StartNetDetection");
    if (needReport || netMonitor_) {
        StopNetDetection();
        InitNetMonitor();
        return;
    }
    if (!netMonitor_) {
        NETMGR_LOG_I("netMonitor_ is null.");
        InitNetMonitor();
        return;
    }
}

void Network::SetNetCaps(const std::set<NetCap> &netCaps)
{
    netCaps_ = netCaps;
}

void Network::NetDetectionForDnsHealth(bool dnsHealthSuccess)
{
    NETMGR_LOG_D("Enter NetDetectionForDnsHealthSync");
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_E("netMonitor_ is nullptr");
        return;
    }
    NetDetectionStatus lastDetectResult = detectResult_;
    NETMGR_LOG_I("Last netDetectionState: [%{public}d]", lastDetectResult);
    if (IsDetectionForDnsSuccess(lastDetectResult, dnsHealthSuccess)) {
        NETMGR_LOG_I("Dns report success, so restart detection.");
        isDetectingForDns_ = true;
        StopNetDetection();
        InitNetMonitor();
    } else if (IsDetectionForDnsFail(lastDetectResult, dnsHealthSuccess)) {
        NETMGR_LOG_I("Dns report fail, start net detection");
        netMonitor_->Start();
    } else {
        NETMGR_LOG_D("Not match, no need to restart.");
    }
}

void Network::StopNetDetection()
{
    NETMGR_LOG_D("Enter StopNetDetection");
    if (netMonitor_ != nullptr) {
        netMonitor_->Stop();
        netMonitor_ = nullptr;
    }
}

void Network::InitNetMonitor()
{
    NETMGR_LOG_D("Enter InitNetMonitor");
    std::weak_ptr<INetMonitorCallback> monitorCallback = shared_from_this();
    netMonitor_ = std::make_shared<NetMonitor>(netId_, netSupplierType_, netLinkInfo_, monitorCallback);
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_E("new NetMonitor failed,netMonitor_ is null!");
        return;
    }
    netMonitor_->Start();
}

void Network::HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect)
{
    NETMGR_LOG_I("HNMR, [%{public}d]", netDetectionState);
    isDetectingForDns_ = false;
    NotifyNetDetectionResult(NetDetectionResultConvert(static_cast<int32_t>(netDetectionState)), urlRedirect);
    if (netCallback_ && (detectResult_ != netDetectionState)) {
        detectResult_ = netDetectionState;
        netCallback_(supplierId_, netDetectionState);
    }
}

void Network::NotifyNetDetectionResult(NetDetectionResultCode detectionResult, const std::string &urlRedirect)
{
    for (const auto &callback : netDetectionRetCallback_) {
        NETMGR_LOG_D("start callback!");
        if (callback) {
            callback->OnNetDetectionResultChanged(detectionResult, urlRedirect);
        }
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
        SendSupplierFaultHiSysEvent(FAULT_SET_DEFAULT_NETWORK_FAILED, ERROR_MSG_SET_DEFAULT_NETWORK_FAILED);
    }
}

void Network::ClearDefaultNetWorkNetId()
{
    int32_t ret = NetsysController::GetInstance().ClearDefaultNetWorkNetId();
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_CLEAR_DEFAULT_NETWORK_FAILED, ERROR_MSG_CLEAR_DEFAULT_NETWORK_FAILED);
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
        NETMGR_LOG_D("Ignore same network state changed.");
        return;
    }
    NetConnState oldState = state_;
    switch (netConnState) {
        case NET_CONN_STATE_IDLE:
        case NET_CONN_STATE_CONNECTING:
        case NET_CONN_STATE_CONNECTED:
        case NET_CONN_STATE_DISCONNECTING:
            state_ = netConnState;
            break;
        case NET_CONN_STATE_DISCONNECTED:
            state_ = netConnState;
            ResetNetlinkInfo();
            break;
        default:
            state_ = NET_CONN_STATE_UNKNOWN;
            break;
    }

    SendConnectionChangedBroadcast(netConnState);
    NETMGR_LOG_I("Network[%{public}d] state changed, from [%{public}d] to [%{public}d]", netId_, oldState, state_);
}

void Network::SendConnectionChangedBroadcast(const NetConnState &netConnState) const
{
    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE;
    info.data = "Net Manager Connection State Changed";
    info.code = static_cast<int32_t>(netConnState);
    info.ordered = false;
    std::map<std::string, int32_t> param = {{"NetType", static_cast<int32_t>(netSupplierType_)}};
    BroadcastManager::GetInstance().SendBroadcast(info, param);
}

void Network::SendSupplierFaultHiSysEvent(NetConnSupplerFault errorType, const std::string &errMsg)
{
    struct EventInfo eventInfo = {.netlinkInfo = netLinkInfo_.ToString(" "),
                                  .supplierId = static_cast<int32_t>(supplierId_),
                                  .errorType = static_cast<int32_t>(errorType),
                                  .errorMsg = errMsg};
    EventReport::SendSupplierFaultEvent(eventInfo);
}

void Network::ResetNetlinkInfo()
{
    netLinkInfo_.Initialize();
}

void Network::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_E("netMonitor_ is nullptr");
        return;
    }
    netMonitor_->UpdateGlobalHttpProxy(httpProxy);
}

void Network::OnHandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect)
{
    if (eventHandler_) {
        eventHandler_->PostAsyncTask(
            [netDetectionState, urlRedirect, this]() { this->HandleNetMonitorResult(netDetectionState, urlRedirect); },
            0);
    }
}

bool Network::ResumeNetworkInfo()
{
    NetLinkInfo nli = netLinkInfo_;

    NETMGR_LOG_D("ResumeNetworkInfo UpdateBasicNetwork false");
    if (!UpdateBasicNetwork(false)) {
        NETMGR_LOG_E("%s release existed basic network failed", __FUNCTION__);
        return false;
    }

    NETMGR_LOG_D("ResumeNetworkInfo UpdateBasicNetwork true");
    if (!UpdateBasicNetwork(true)) {
        NETMGR_LOG_E("%s create basic network failed", __FUNCTION__);
        return false;
    }

    NETMGR_LOG_D("ResumeNetworkInfo UpdateNetLinkInfo");
    return UpdateNetLinkInfo(nli);
}

bool Network::IsDetectionForDnsSuccess(NetDetectionStatus netDetectionState, bool dnsHealthSuccess)
{
    return ((netDetectionState == INVALID_DETECTION_STATE) && dnsHealthSuccess && !isDetectingForDns_);
}

bool Network::IsDetectionForDnsFail(NetDetectionStatus netDetectionState, bool dnsHealthSuccess)
{
    return ((netDetectionState == VERIFICATION_STATE) && !dnsHealthSuccess && !(netMonitor_->IsDetecting()));
}
} // namespace NetManagerStandard
} // namespace OHOS
