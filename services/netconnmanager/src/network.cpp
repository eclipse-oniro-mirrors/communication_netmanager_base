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
#include <cstdio>
#include <securec.h>
#include <fcntl.h>
#include <unistd.h>
#include "netsys_controller.h"
#include "net_conn_types.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "network.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t MIN_NET_ID = 100;
constexpr int32_t MAX_NET_ID = 0xFFFF - 0x400;
static int32_t g_netId = MIN_NET_ID;

Network::Network() : id_(g_netId++)
{
    if (id_ > MAX_NET_ID) {
        id_ = MIN_NET_ID;
    }
}

Network::~Network()
{
    DestroyPhy();
}

uint32_t Network::GetId() const
{
    return id_;
}

int32_t Network::CreatePhy()
{
    int32_t err;
    if (!phyCreated_) {
        err = NetsysController::GetInstance().NetworkCreatePhysical(id_, 0);
        if (err < 0) {
            NETMGR_LOG_W("NetworkCreatePhysical failed, [%{public}d]", err);
            return err;
        }

        err = NetsysController::GetInstance().CreateNetworkCache(id_);
        if (err < 0) {
            NETMGR_LOG_W("CreateNetworkCache failed, [%{public}d]", err);
            NetsysController::GetInstance().NetworkDestroy(id_);
            return err;
        }
        phyCreated_ = true;
    }
    return ERR_NONE;
}

int32_t Network::DestroyPhy()
{
    int32_t err;
    if (phyCreated_) {
        for (auto it = netAddrList_.begin(); it != netAddrList_.end(); ++it) {
            const struct INetAddr &inetAddr = *it;
            int32_t prefixLen = inetAddr.prefixlen_;
            if (prefixLen == 0) {
                prefixLen = CommonUtils::GetMaskLength(inetAddr.netMask_);
            }
            NetsysController::GetInstance().InterfaceDelAddress(ifaceName_, inetAddr.address_, prefixLen);
        }
        err = NetsysController::GetInstance().NetworkRemoveInterface(id_, ifaceName_);
        if (err < 0) {
            NETMGR_LOG_W("NetworkRemoveInterface failed, [%{public}d]", err);
        }
        err = NetsysController::GetInstance().NetworkDestroy(id_);
        if (err < 0) {
            NETMGR_LOG_W("NetworkDestroy failed, [%{public}d]", err);
        }
        err = NetsysController::GetInstance().DestroyNetworkCache(id_);
        if (err < 0) {
            NETMGR_LOG_W("DestroyNetworkCache failed, [%{public}d]", err);
        }
        ClearData();
        phyCreated_ = false;
    }
    return ERR_NONE;
}

int32_t Network::SetIfaceName(const std::string &ifaceName)
{
    int32_t err;
    if (ifaceName_ != ifaceName) {
        // Call netsys to add and remove interface
        if (!ifaceName_.empty()) {
            err = NetsysController::GetInstance().NetworkRemoveInterface(id_, ifaceName_);
            if (err) {
                NETMGR_LOG_W("NetworkRemoveInterface failed, [%{public}d]", err);
            }
        }
        if (!ifaceName.empty()) {
            err = NetsysController::GetInstance().NetworkAddInterface(id_, ifaceName);
            if (err) {
                NETMGR_LOG_W("NetworkAddInterface failed, [%{public}d]", err);
            }
        }
        ClearData();
        ifaceName_ = ifaceName;
    }
    return ERR_NONE;
}

int32_t Network::SetDomain(const std::string &domain)
{
    domain_ = domain;
    return ERR_NONE;
}

int32_t Network::SetNetAddrList(const std::list<INetAddr> &netAddrList)
{
    int32_t err;
    if (netAddrList_ != netAddrList) {
        for (auto it = netAddrList_.begin(); it != netAddrList_.end(); ++it) {
            const struct INetAddr &inetAddr = *it;
            int32_t prefixLen = inetAddr.prefixlen_;
            if (prefixLen == 0) {
                prefixLen = CommonUtils::GetMaskLength(inetAddr.netMask_);
            }
            err = NetsysController::GetInstance().InterfaceDelAddress(ifaceName_, inetAddr.address_, prefixLen);
            if (err) {
                NETMGR_LOG_W("InterfaceDelAddress failed, [%{public}d]", err);
            }
        }

        for (auto it = netAddrList.begin(); it != netAddrList.end(); ++it) {
            const struct INetAddr &inetAddr = *it;
            int32_t prefixLen = inetAddr.prefixlen_;
            if (prefixLen == 0) {
                prefixLen = CommonUtils::GetMaskLength(inetAddr.netMask_);
            }
            err = NetsysController::GetInstance().InterfaceAddAddress(ifaceName_, inetAddr.address_, prefixLen);
            if (err) {
                NETMGR_LOG_W("InterfaceAddAddress failed, [%{public}d]", err);
            }
        }
        netAddrList_ = netAddrList;
    }
    return ERR_NONE;
}

int32_t Network::SetRouteList(const std::list<Route> &routeList)
{
    int32_t err;
    if (routeList_ != routeList) {
        for (auto it = routeList_.begin(); it != routeList_.end(); ++it) {
            const struct Route &route = *it;
            std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
            err = NetsysController::GetInstance().NetworkRemoveRoute(id_, route.iface_, destAddress,
                                                                     route.gateway_.address_);
            if (err) {
                NETMGR_LOG_W("NetworkRemoveRoute failed, [%{public}d]", err);
            }
        }

        for (auto it = routeList.begin(); it != routeList.end(); ++it) {
            const struct Route &route = *it;
            std::string destAddress = route.destination_.address_ + "/" + std::to_string(route.destination_.prefixlen_);
            err = NetsysController::GetInstance().NetworkAddRoute(id_, route.iface_, destAddress,
                                                                  route.gateway_.address_);
            if (err) {
                NETMGR_LOG_W("NetworkAddRoute failed, [%{public}d]", err);
            }
        }
        routeList_ = routeList;
    }
    return ERR_NONE;
}

int32_t Network::SetDnsList(const std::list<INetAddr> &dnsAddrList)
{
    int32_t err;
    if (dnsList_ == dnsAddrList) {
        NETMGR_LOG_W("dns list is same, do not update");
        return ERR_NONE;
    }

    std::vector<std::string> servers;
    std::vector<std::string> domains;
    for (const auto &dns : dnsAddrList) {
        servers.push_back(dns.address_);
        domains.push_back(dns.hostName_);
    }

    err = NetsysController::GetInstance().SetResolverConfig(static_cast<uint16_t>(id_), 0, 1, servers, domains);
    if (err) {
        NETMGR_LOG_W("SetResolverConfig failed, [%{public}d]", err);
    }
    dnsList_ = dnsAddrList;
    return ERR_NONE;
}

int32_t Network::SetMtu(const uint16_t &mtu)
{
    int32_t err;
    if (mtu_ != mtu) {
        err = NetsysController::GetInstance().InterfaceSetMtu(ifaceName_, mtu);
        if (err) {
            NETMGR_LOG_W("InterfaceSetMtu failed, [%{public}d]", err);
        }
        mtu_ = mtu;
    }
    return ERR_NONE;
}

int32_t Network::SetTcpBufferSizes(const std::string &tcpBufferSizes)
{
    tcpBufferSizes_ = tcpBufferSizes;
    return ERR_NONE;
}

int32_t Network::SetDefault()
{
    return NetsysController::GetInstance().SetDefaultNetWork(id_);
}

int32_t Network::CreateSocket(int32_t domain, int32_t type, int32_t protocol)
{
    int32_t sockFd = socket(domain, type, protocol);
    if (sockFd > 0) {
        int32_t err = NetsysController::GetInstance().BindSocket(sockFd, id_);
        if (err < 0) {
            NETMGR_LOG_W("Bind socket failed in network[%{public}d], err[%{public}d]", id_, err);
        }
    } else {
        NETMGR_LOG_W("Create socket failed in network[%{public}d], err[%{public}d]", id_, sockFd);
    }
    return sockFd;
}

void Network::DestroySocket(int32_t sockFd)
{
    bool sockFdValid = (fcntl(sockFd, F_GETFD) != -1 || errno != EBADF);
    if (sockFdValid) {
        close(sockFd);
    }
}

void Network::ClearData()
{
    ifaceName_ = "";
    domain_ = "";
    netAddrList_.clear();
    dnsList_.clear();
    routeList_.clear();
    mtu_ = 0;
    tcpBufferSizes_ = "";
}
} // namespace NetManagerStandard
} // namespace OHOS
