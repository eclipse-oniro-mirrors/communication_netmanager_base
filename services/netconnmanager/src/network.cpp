/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "net_conn_service_iface.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_client.h"
#include "netmanager_base_common_utils.h"
#include "netsys_controller.h"
#include "network.h"
#include "route_utils.h"
#include "securec.h"

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
constexpr int32_t DETECTION_RESULT_WAIT_MS = 3 * 1000;
constexpr int32_t LAST_DETECTION_LAPSE_MS = 200;
constexpr int32_t ERRNO_EADDRNOTAVAIL = -99;
constexpr int32_t MAX_IPV4_DNS_NUM = 5;
constexpr int32_t MAX_IPV6_DNS_NUM = 2;
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

Network::~Network()
{
    StopNetDetection();
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
        if (nat464Service_ != nullptr) {
            nat464Service_->UpdateService(NAT464_SERVICE_STOP);
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
        std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
        bool hasDns = netLinkInfo_.dnsList_.size() ? true : false;
        if (NetsysController::GetInstance().NetworkCreateVirtual(netId_, hasDns) != NETMANAGER_SUCCESS) {
            std::string errMsg = std::string(ERROR_MSG_CREATE_VIRTUAL_NETWORK_FAILED).append(std::to_string(netId_));
            SendSupplierFaultHiSysEvent(FAULT_CREATE_VIRTUAL_NETWORK_FAILED, errMsg);
        }
        NetsysController::GetInstance().CreateNetworkCache(netId_, true);
        isVirtualCreated_ = true;
    }
    return true;
}

bool Network::IsIfaceNameInUse()
{
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    return NetConnServiceIface().IsIfaceNameInUse(netLinkInfo_.ifaceName_, netId_);
}

std::string Network::GetNetCapabilitiesAsString(const uint32_t supplierId) const
{
    return NetConnServiceIface().GetNetCapabilitiesAsString(supplierId);
}

bool Network::ReleaseBasicNetwork()
{
    if (!isPhyNetCreated_) {
        NETMGR_LOG_E("physical network has not created");
        return true;
    }
    NETMGR_LOG_D("Destroy physical network");
    StopNetDetection();
    std::string netCapabilities = GetNetCapabilitiesAsString(supplierId_);
    NETMGR_LOG_D("ReleaseBasicNetwork supplierId %{public}u, netId %{public}d, netCapabilities %{public}s",
        supplierId_, netId_, netCapabilities.c_str());
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo netLinkInfoBck = netLinkInfo_;
    lock.unlock();
    if (!IsIfaceNameInUse() || isNeedResume_) {
        for (const auto &inetAddr : netLinkInfoBck.netAddrList_) {
            int32_t prefixLen = inetAddr.prefixlen_ == 0 ? Ipv4PrefixLen(inetAddr.netMask_) : inetAddr.prefixlen_;
            NetsysController::GetInstance().DelInterfaceAddress(netLinkInfoBck.ifaceName_, inetAddr.address_,
                                                                prefixLen);
        }
        for (const auto &route : netLinkInfoBck.routeList_) {
            if (route.destination_.address_ != LOCAL_ROUTE_NEXT_HOP &&
                route.destination_.address_ != LOCAL_ROUTE_IPV6_DESTINATION) {
                auto family = GetAddrFamily(route.destination_.address_);
                std::string nextHop = (family == AF_INET6) ? "" : LOCAL_ROUTE_NEXT_HOP;
                auto destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
                NetsysController::GetInstance().NetworkRemoveRoute(LOCAL_NET_ID, route.iface_, destAddress, nextHop);
            }
        }
        isNeedResume_ = false;
    } else {
        for (const auto &inetAddr : netLinkInfoBck.netAddrList_) {
            int32_t prefixLen = inetAddr.prefixlen_ == 0 ? Ipv4PrefixLen(inetAddr.netMask_) : inetAddr.prefixlen_;
            NetsysController::GetInstance().DelInterfaceAddress(netLinkInfoBck.ifaceName_, inetAddr.address_,
                                                                prefixLen, netCapabilities);
        }
    }
    for (const auto &route : netLinkInfoBck.routeList_) {
        auto destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        NetsysController::GetInstance().NetworkRemoveRoute(netId_, route.iface_, destAddress,
                                                           route.gateway_.address_);
    }
    NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfoBck.ifaceName_);
    NetsysController::GetInstance().NetworkDestroy(netId_);
    NetsysController::GetInstance().DestroyNetworkCache(netId_);
    std::unique_lock<std::shared_mutex> wlock(netLinkInfoMutex_);
    netLinkInfo_.Initialize();
    isPhyNetCreated_ = false;
    return true;
}

bool Network::ReleaseVirtualNetwork()
{
    NETMGR_LOG_D("Enter release virtual network");
    if (isVirtualCreated_) {
        std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
        NetLinkInfo netLinkInfoBck = netLinkInfo_;
        lock.unlock();
        for (const auto &inetAddr : netLinkInfoBck.netAddrList_) {
            int32_t prefixLen = inetAddr.prefixlen_;
            if (prefixLen == 0) {
                prefixLen = Ipv4PrefixLen(inetAddr.netMask_);
            }
            NetsysController::GetInstance().DelInterfaceAddress(
                netLinkInfoBck.ifaceName_, inetAddr.address_, prefixLen);
        }
        NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfoBck.ifaceName_);
        NetsysController::GetInstance().NetworkDestroy(netId_, true);
        NetsysController::GetInstance().DestroyNetworkCache(netId_, true);
        std::unique_lock<std::shared_mutex> wlock(netLinkInfoMutex_);
        netLinkInfo_.Initialize();
        isVirtualCreated_ = false;
    }
    return true;
}

bool Network::UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("update net link information process");
    UpdateStatsCached(netLinkInfo);
    UpdateInterfaces(netLinkInfo);
    bool isIfaceNameInUse = NetConnServiceIface().IsIfaceNameInUse(netLinkInfo.ifaceName_, netId_);
    bool flag = false;
    bool hasSameIpAddr = false;
    {
        std::shared_lock<std::shared_mutex> nlock(netCapsMutex);
        flag = netCaps_.find(NetCap::NET_CAPABILITY_INTERNET) != netCaps_.end();
    }
    if (!isIfaceNameInUse || flag) {
        hasSameIpAddr = UpdateIpAddrs(netLinkInfo);
    }
    UpdateRoutes(netLinkInfo);
    UpdateDns(netLinkInfo);
    UpdateMtu(netLinkInfo);
    UpdateTcpBufferSize(netLinkInfo);
    std::unique_lock<std::shared_mutex> wlock(netLinkInfoMutex_);
    netLinkInfo_ = netLinkInfo;
    wlock.unlock();
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo netLinkInfoBck = netLinkInfo_;
    lock.unlock();
    if (IsNat464Prefered()) {
        if (nat464Service_ == nullptr) {
            nat464Service_ = std::make_unique<Nat464Service>(netId_, netLinkInfoBck.ifaceName_);
        }
        nat464Service_->MaybeUpdateV6Iface(netLinkInfoBck.ifaceName_);
        nat464Service_->UpdateService(NAT464_SERVICE_CONTINUE);
    } else if (nat464Service_ != nullptr) {
        nat464Service_->UpdateService(NAT464_SERVICE_STOP);
    }
    bool find = false;
    {
        std::shared_lock<std::shared_mutex> lock(netCapsMutex);
        if (netSupplierType_ != BEARER_VPN && netCaps_.find(NetCap::NET_CAPABILITY_INTERNET) != netCaps_.end()) {
            find = true;
        }
    }
    if (find) {
        if (netMonitor_) {
            if (DelayStartDetectionForIpUpdate(hasSameIpAddr)) {
                return true;
            }
        }
        StartNetDetection(true);
    }
    return true;
}

bool Network::DelayStartDetectionForIpUpdate(bool hasSameIpAddr)
{
    if (!hasSameIpAddr) {
        return false;
    }
    if ((netSupplierType_ != BEARER_CELLULAR && netSupplierType_ != BEARER_WIFI)) {
        return false;
    }
    if (!netMonitor_->IsDetecting()) {
        return false;
    }
    uint64_t nowTime = CommonUtils::GetCurrentMilliSecond();
    std::string taskName = "DelayStartDetection";
    if ((nowTime - netMonitor_->GetLastDetectTime()) >= LAST_DETECTION_LAPSE_MS) {
        return false;
    }
    NETMGR_LOG_I("UpdateNetLinkInfo: delay start detection");
    if (eventHandler_) {
        eventHandler_->RemoveTask(taskName);
        std::weak_ptr<Network> wp = shared_from_this();
        eventHandler_->PostAsyncTask(
            [wp] {
                auto sp = wp.lock();
                if (sp != nullptr) {
                    sp->StartNetDetection(true);
                }
            }, taskName, DETECTION_RESULT_WAIT_MS);
    }
    return true;
}

NetLinkInfo Network::GetNetLinkInfo() const
{
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo linkInfo = netLinkInfo_;
    if (netSupplierType_ == BEARER_VPN) {
        return linkInfo;
    }
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

HttpProxy Network::GetHttpProxy() const
{
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    return netLinkInfo_.httpProxy_;
}

std::string Network::GetIfaceName() const
{
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    return netLinkInfo_.ifaceName_;
}

std::string Network::GetIdent() const
{
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    return netLinkInfo_.ident_;
}

void Network::UpdateInterfaces(const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_LOG_D("Network UpdateInterfaces in.");
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo netLinkInfoBck = netLinkInfo_;
    lock.unlock();
    if (newNetLinkInfo.ifaceName_ == netLinkInfoBck.ifaceName_) {
        NETMGR_LOG_D("Network UpdateInterfaces out. same with before.");
        return;
    }

    int32_t ret = NETMANAGER_SUCCESS;
    // Call netsys to add and remove interface
    if (!newNetLinkInfo.ifaceName_.empty()) {
        ret = NetsysController::GetInstance().NetworkAddInterface(netId_, newNetLinkInfo.ifaceName_, netSupplierType_);
        if (ret != NETMANAGER_SUCCESS) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_ADD_NET_INTERFACE_FAILED);
        }
    }
    if (!netLinkInfoBck.ifaceName_.empty()) {
        ret = NetsysController::GetInstance().NetworkRemoveInterface(netId_, netLinkInfoBck.ifaceName_);
        if (ret != NETMANAGER_SUCCESS) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_REMOVE_NET_INTERFACE_FAILED);
        }
    }
    std::unique_lock<std::shared_mutex> wlock(netLinkInfoMutex_);
    netLinkInfo_.ifaceName_ = newNetLinkInfo.ifaceName_;
    NETMGR_LOG_D("Network UpdateInterfaces out.");
}

void Network::RemoveRouteByFamily(INetAddr::IpType addrFamily)
{
    auto route = netLinkInfo_.routeList_.begin();
    while (route != netLinkInfo_.routeList_.end()) {
        if (route->destination_.type_ != addrFamily) {
            route++;
            continue;
        }
        std::string destAddress =
            route->destination_.address_ + "/" + std::to_string(route->destination_.prefixlen_);
        NetsysController::GetInstance().NetworkRemoveRoute(netId_, route->iface_, destAddress,
            route->gateway_.address_);
        route = netLinkInfo_.routeList_.erase(route);
    }
}

bool Network::UpdateIpAddrs(const NetLinkInfo &newNetLinkInfo)
{
    // netLinkInfo_ represents the old, netLinkInfo represents the new
    // Update: remove old Ips first, then add the new Ips
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo netLinkInfoBck = netLinkInfo_;
    lock.unlock();
    bool hasSameIpAddr = false;
    NETMGR_LOG_I("UpdateIpAddrs, old ip addrs size: [%{public}zu]", netLinkInfoBck.netAddrList_.size());
    for (const auto &inetAddr : netLinkInfoBck.netAddrList_) {
        if (newNetLinkInfo.HasNetAddr(inetAddr)) {
            hasSameIpAddr = true;
            NETMGR_LOG_W("Same ip address:[%{public}s], there is not need to be deleted",
                CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        auto family = GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : Ipv4PrefixLen(inetAddr.netMask_));
        int32_t ret =
            NetsysController::GetInstance().DelInterfaceAddress(
                netLinkInfoBck.ifaceName_, inetAddr.address_, prefixLen);
        if (NETMANAGER_SUCCESS != ret) {
            SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_DELETE_NET_IP_ADDR_FAILED);
        }

        if ((ret == ERRNO_EADDRNOTAVAIL) || (ret == 0)) {
            NETMGR_LOG_W("remove route info of ip address:[%{public}s]",
                CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            std::unique_lock<std::shared_mutex> lock(netLinkInfoMutex_);
            INetAddr::IpType addrFamily = family == AF_INET ? INetAddr::IpType::IPV4 : INetAddr::IpType::IPV6;
            RemoveRouteByFamily(addrFamily);
        }
    }

    HandleUpdateIpAddrs(newNetLinkInfo);
    return hasSameIpAddr;
}

void Network::HandleUpdateIpAddrs(const NetLinkInfo &newNetLinkInfo)
{
    NETMGR_LOG_I("HandleUpdateIpAddrs, new ip addrs size: [%{public}zu]", newNetLinkInfo.netAddrList_.size());
    for (const auto &inetAddr : newNetLinkInfo.netAddrList_) {
        std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
        if (netLinkInfo_.HasNetAddr(inetAddr)) {
            NETMGR_LOG_W("Same ip address:[%{public}s], there is no need to add it again",
                         CommonUtils::ToAnonymousIp(inetAddr.address_).c_str());
            continue;
        }
        lock.unlock();
        auto family = GetAddrFamily(inetAddr.address_);
        auto prefixLen = inetAddr.prefixlen_ ? static_cast<int32_t>(inetAddr.prefixlen_)
                                             : ((family == AF_INET6) ? Ipv6PrefixLen(inetAddr.netMask_)
                                                                     : Ipv4PrefixLen(inetAddr.netMask_));
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
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo netLinkInfoBck = netLinkInfo_;
    lock.unlock();
    NETMGR_LOG_D("UpdateRoutes, old routes: [%{public}s]", netLinkInfoBck.ToStringRoute("").c_str());
    for (const auto &route : netLinkInfoBck.routeList_) {
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
        if (netLinkInfoBck.HasRoute(route)) {
            NETMGR_LOG_W("Same route:[%{public}s]  ifo, there is no need to add it again",
                         CommonUtils::ToAnonymousIp(route.destination_.address_).c_str());
            continue;
        }

        std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
        auto ret = NetsysController::GetInstance().NetworkAddRoute(
            netId_, route.iface_, destAddress, route.gateway_.address_, route.isExcludedRoute_);
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
    if (newNetLinkInfo.routeList_.empty()) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_UPDATE_NET_ROUTES_FAILED);
    }
}

void Network::UpdateDns(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateDns in.");
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    std::stringstream ss;
    int32_t ipv4DnsCnt = 0;
    int32_t ipv6DnsCnt = 0;
    for (const auto &dns : netLinkInfo.dnsList_) {
        if(dns.address_ == ""){
            continue;
        }
        domains.emplace_back(dns.hostName_);
        auto dnsFamily = GetAddrFamily(dns.address_);
        if (dns.type_ == NetManagerStandard::INetAddr::IPV4 || dnsFamily == AF_INET) {
            if (ipv4DnsCnt++ < MAX_IPV4_DNS_NUM) {
                servers.emplace_back(dns.address_);
                ss << '[' << CommonUtils::ToAnonymousIp(dns.address_).c_str() << ']';
            }
        } else if (dns.type_ == NetManagerStandard::INetAddr::IPV6 || dnsFamily == AF_INET6) {
            if (ipv6DnsCnt++ < MAX_IPV6_DNS_NUM) {
                servers.emplace_back(dns.address_);
                ss << '[' << CommonUtils::ToAnonymousIp(dns.address_).c_str() << ']';
            }
        } else {
            servers.emplace_back(dns.address_);
            ss << '[' << CommonUtils::ToAnonymousIp(dns.address_).c_str() << ']';
            NETMGR_LOG_W("unknown dns.type_");
        }
    }
    NETMGR_LOG_I("update dns server: %{public}s", ss.str().c_str());
    // Call netsys to set dns, use default timeout and retry
    int32_t ret = NetsysController::GetInstance().SetResolverConfig(netId_, 0, 0, servers, domains);
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_SET_NET_RESOLVER_FAILED);
    }
    NETMGR_LOG_I("SetUserDefinedServerFlag: netId:[%{public}d], flag:[%{public}d]", netId_,
        netLinkInfo.isUserDefinedDnsServer_);
    ret = NetsysController::GetInstance().SetUserDefinedServerFlag(netId_, netLinkInfo.isUserDefinedDnsServer_);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("SetUserDefinedServerFlag failed");
    }
    NETMGR_LOG_D("Network UpdateDns out.");
    if (netLinkInfo.dnsList_.empty()) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_UPDATE_NET_DNSES_FAILED);
    }
}

void Network::UpdateMtu(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateMtu in.");
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    if (netLinkInfo.mtu_ == netLinkInfo_.mtu_) {
        NETMGR_LOG_D("Network UpdateMtu out. same with before.");
        return;
    }
    lock.unlock();

    int32_t ret = NetsysController::GetInstance().SetInterfaceMtu(netLinkInfo.ifaceName_, netLinkInfo.mtu_);
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_SET_NET_MTU_FAILED);
    }
    NETMGR_LOG_D("Network UpdateMtu out.");
}

void Network::UpdateTcpBufferSize(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateTcpBufferSize in.");
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    if (netLinkInfo.tcpBufferSizes_ == netLinkInfo_.tcpBufferSizes_) {
        NETMGR_LOG_D("Network UpdateTcpBufferSize out. same with before.");
        return;
    }
    lock.unlock();
    int32_t ret = NetsysController::GetInstance().SetTcpBufferSizes(netLinkInfo.tcpBufferSizes_);
    if (ret != NETMANAGER_SUCCESS) {
        SendSupplierFaultHiSysEvent(FAULT_UPDATE_NETLINK_INFO_FAILED, ERROR_MSG_SET_NET_TCP_BUFFER_SIZE_FAILED);
    }
    NETMGR_LOG_D("Network UpdateTcpBufferSize out.");
}

void Network::UpdateStatsCached(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOG_D("Network UpdateStatsCached in.");
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    if (netLinkInfo.ifaceName_ == netLinkInfo_.ifaceName_ && netLinkInfo.ident_ == netLinkInfo_.ident_) {
        NETMGR_LOG_D("Network UpdateStatsCached out. same with before");
        return;
    }
    lock.unlock();
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
    NETMGR_LOG_D("Enter StartNetDetection");
#ifdef FEATURE_SUPPORT_POWERMANAGER
    if (forbidDetectionFlag_) {
        NETMGR_LOG_W("Sleep status, forbid detection");
        return;
    }
#endif
    if (needReport && netMonitor_) {
        StopNetDetection();
        InitNetMonitor();
        return;
    }
    if (!netMonitor_) {
        NETMGR_LOG_I("netMonitor_ is null.");
        InitNetMonitor();
        return;
    } else {
        netMonitor_->Start();
    }
}

#ifdef FEATURE_SUPPORT_POWERMANAGER
void Network::UpdateForbidDetectionFlag(bool forbidDetectionFlag)
{
    forbidDetectionFlag_ = forbidDetectionFlag;
}
#endif

void Network::SetNetCaps(const std::set<NetCap> &netCaps)
{
    std::unique_lock<std::shared_mutex> lock(netCapsMutex);
    netCaps_ = netCaps;
}

void Network::NetDetectionForDnsHealth(bool dnsHealthSuccess)
{
    NETMGR_LOG_D("Enter NetDetectionForDnsHealthSync");
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_D("netMonitor_ is nullptr");
        return;
    }
    NetDetectionStatus lastDetectResult = detectResult_;
    {
        static NetDetectionStatus preStatus = UNKNOWN_STATE;
        if (preStatus != lastDetectResult) {
            NETMGR_LOG_I("Last netDetectionState: [%{public}d->%{public}d]", preStatus, lastDetectResult);
            preStatus = lastDetectResult;
        }
    }
    if (IsDetectionForDnsSuccess(lastDetectResult, dnsHealthSuccess)) {
        NETMGR_LOG_I("Dns report success, so restart detection.");
        isDetectingForDns_ = true;
        netMonitor_->Start();
    } else if (IsDetectionForDnsFail(lastDetectResult, dnsHealthSuccess)) {
        NETMGR_LOG_D("Dns report fail, start net detection");
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
        netMonitor_->StopDualStackProbe();
        lastDetectTime_ = netMonitor_->GetLastDetectTime();
        netMonitor_ = nullptr;
    }
}

void Network::InitNetMonitor()
{
    NETMGR_LOG_D("Enter InitNetMonitor");
    std::weak_ptr<INetMonitorCallback> monitorCallback = shared_from_this();
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetMonitorInfo netMonitorInfo;
    netMonitorInfo.isScreenOn = isScreenOn_;
    netMonitorInfo.isSleep = isSleep_;
    netMonitorInfo.lastDetectTime = lastDetectTime_;
    netMonitor_ = std::make_shared<NetMonitor>(
        netId_, netSupplierType_, netLinkInfo_, monitorCallback, netMonitorInfo);
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_E("new NetMonitor failed,netMonitor_ is null!");
        return;
    }
    netMonitor_->UpdateDualStackProbeTime(dualStackProbeTime_);
    netMonitor_->Start();
}

void Network::HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect)
{
    NETMGR_LOG_D("HNMR, [%{public}d]", netDetectionState);
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
#ifdef FEATURE_SUPPORT_POWERMANAGER
    StartNetDetection(false);
#endif
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
    if (IsNat464Prefered()) {
        std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
        NetLinkInfo netLinkInfoBck = netLinkInfo_;
        lock.unlock();
        if (nat464Service_ == nullptr) {
            nat464Service_ = std::make_unique<Nat464Service>(netId_, netLinkInfoBck.ifaceName_);
        }
        nat464Service_->MaybeUpdateV6Iface(netLinkInfoBck.ifaceName_);
        nat464Service_->UpdateService(NAT464_SERVICE_CONTINUE);
    } else if (nat464Service_ != nullptr) {
        nat464Service_->UpdateService(NAT464_SERVICE_STOP);
    }
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
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    struct EventInfo eventInfo = {.netlinkInfo = netLinkInfo_.ToString(" "),
                                  .supplierId = static_cast<int32_t>(supplierId_),
                                  .errorType = static_cast<int32_t>(errorType),
                                  .errorMsg = errMsg};
    EventReport::SendSupplierFaultEvent(eventInfo);
}

void Network::ResetNetlinkInfo()
{
    std::unique_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    netLinkInfo_.Initialize();
    detectResult_ = UNKNOWN_STATE;
}

void Network::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    if (netMonitor_ == nullptr) {
        NETMGR_LOG_D("netMonitor_ is nullptr");
        return;
    }
    netMonitor_->UpdateGlobalHttpProxy(httpProxy);
    StartNetDetection(true);
}

void Network::OnHandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect)
{
    if (eventHandler_) {
        auto network = shared_from_this();
        eventHandler_->PostAsyncTask([netDetectionState, urlRedirect,
                                      network]() { network->HandleNetMonitorResult(netDetectionState, urlRedirect); },
                                     0);
    }
}

void Network::OnHandleDualStackProbeResult(DualStackProbeResultCode dualStackProbeResultCode)
{
    if (eventHandler_) {
        auto network = shared_from_this();
        eventHandler_->PostAsyncTask([dualStackProbeResultCode,
            network]() { network->HandleNetProbeResult(dualStackProbeResultCode); }, 0);
    }
}

bool Network::ResumeNetworkInfo()
{
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo nli = netLinkInfo_;
    lock.unlock();
    {
        std::shared_lock<std::shared_mutex> lock(netCapsMutex);
        if (netCaps_.find(NetCap::NET_CAPABILITY_INTERNET) != netCaps_.end()) {
            isNeedResume_ = true;
        }
    }
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

bool Network::IsNat464Prefered()
{
    if (netSupplierType_ != BEARER_CELLULAR && netSupplierType_ != BEARER_WIFI && netSupplierType_ != BEARER_ETHERNET) {
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    NetLinkInfo netLinkInfoBck = netLinkInfo_;
    lock.unlock();
    if (std::any_of(netLinkInfoBck.netAddrList_.begin(), netLinkInfoBck.netAddrList_.end(),
                    [](const INetAddr &i) { return i.type_ != INetAddr::IPV6; })) {
        return false;
    }
    if (netLinkInfoBck.ifaceName_.empty() || !IsConnected()) {
        return false;
    }
    return true;
}


void Network::CloseSocketsUid(uint32_t uid)
{
    std::shared_lock<std::shared_mutex> lock(netLinkInfoMutex_);
    for (const auto &inetAddr : netLinkInfo_.netAddrList_) {
        NetsysController::GetInstance().CloseSocketsUid(inetAddr.address_, uid);
    }
}

void Network::SetScreenState(bool isScreenOn)
{
    isScreenOn_ = isScreenOn;
    if (netMonitor_ == nullptr) {
        return;
    }
    netMonitor_->SetScreenState(isScreenOn);
}

void Network::SetSleepMode(bool isSleep)
{
    isSleep_ = isSleep;
    if (netMonitor_ == nullptr) {
        return;
    }
    netMonitor_->SetSleepMode(isSleep);
}

int32_t Network::StartDualStackProbeThread()
{
    if (netMonitor_) {
        return netMonitor_->StartDualStackProbeThread();
    }
    return NETMANAGER_ERR_INTERNAL;
}

int32_t Network::RegisterDualStackProbeCallback(std::shared_ptr<IDualStackProbeCallback>& callback)
{
    NETMGR_LOG_I("Enter RNPCB");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    for (const auto &iter : dualStackProbeCallback_) {
        if (callback == iter) {
            NETMGR_LOG_D("dualStackProbeCallback_ had this callback");
            return NETMANAGER_SUCCESS;
        }
    }

    dualStackProbeCallback_.emplace_back(callback);
    return NETMANAGER_SUCCESS;
}

int32_t Network::UnRegisterDualStackProbeCallback(std::shared_ptr<IDualStackProbeCallback>& callback)
{
    NETMGR_LOG_I("Enter URNPCB");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    for (auto iter = dualStackProbeCallback_.begin(); iter != dualStackProbeCallback_.end(); ++iter) {
        if (callback == *iter) {
            dualStackProbeCallback_.erase(iter);
            return NETMANAGER_SUCCESS;
        }
    }

    return NETMANAGER_SUCCESS;
}

void Network::HandleNetProbeResult(DualStackProbeResultCode DualStackProbeResultCode)
{
    for (const auto &callback : dualStackProbeCallback_) {
        NETMGR_LOG_D("start DualStackProbe callback!");
        if (callback) {
            callback->OnHandleDualStackProbeResult(DualStackProbeResultCode);
        }
    }
}

void Network::UpdateDualStackProbeTime(int32_t dualStackProbeTime)
{
    dualStackProbeTime_ = dualStackProbeTime;
    if (netMonitor_) {
        netMonitor_->UpdateDualStackProbeTime(dualStackProbeTime);
    }
}

} // namespace NetManagerStandard
} // namespace OHOS
