/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <netdb.h>
#include "connection_ani.h"
#include "wrapper.rs.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "http_proxy.h"
#include "inet_addr.h"
#include "net_conn_client.h"
#include "net_handle.h"
#include "net_link_info.h"
#include "netmanager_secure_data.h"
#include "refbase.h"
#include "tokenid_kit.h"
#include "net_manager_constants.h"
#include "netmanager_base_log.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_permission.h"
#include "errorcode_convertor.h"
#include "icu_helper.h"
#include "net_conn_info.h"
#include "net_port_states_info.h"
#include "net_ip_mac_info.h"
#include "net_probe.h"
#include "net_trace_route_info.h"

namespace OHOS {
namespace NetManagerAni {
using namespace Security::AccessToken;
std::mutex g_predefinedHostMtx;

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::string(convertor.ConvertErrorCode(errorCode));
}

NetHandle GetDefaultNetHandle(int32_t &ret)
{
    NetManagerStandard::NetHandle handle;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetDefaultNet(handle);
    return NetHandle{
        .net_id = handle.GetNetId(),
    };
}

rust::vec<NetHandle> GetAllNets(int32_t &ret)
{
    std::list<sptr<NetManagerStandard::NetHandle>> nativeNetLists;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetAllNets(nativeNetLists);
    rust::vec<NetHandle> netLists;
    for (auto handle : nativeNetLists) {
        netLists.push_back(NetHandle{
            .net_id = handle->GetNetId(),
        });
    }
    return netLists;
}

bool HasDefaultNet(int32_t &ret)
{
    auto hasDefaultNet = false;
    ret = NetManagerStandard::NetConnClient::GetInstance().HasDefaultNet(hasDefaultNet);
    return hasDefaultNet;
}

NetCapabilities GetNetCapabilities(NetHandle const &netHandle, int32_t &ret)
{
    NetManagerStandard::NetHandle nativeNethandle;
    nativeNethandle.SetNetId(netHandle.net_id);
    NetManagerStandard::NetAllCapabilities nativeNetCapabilities;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetNetCapabilities(nativeNethandle, nativeNetCapabilities);
    auto networkCap = rust::Vec<NetManagerStandard::NetCap>();
    for (auto &cap : nativeNetCapabilities.netCaps_) {
        networkCap.push_back(cap);
    }
    auto bearerTypes = rust::vec<NetManagerStandard::NetBearType>();
    for (auto &bearerType : nativeNetCapabilities.bearerTypes_) {
        bearerTypes.push_back(bearerType);
    }
    return NetCapabilities{
        .linkUpBandwidthKbps = nativeNetCapabilities.linkUpBandwidthKbps_,
        .linkDownBandwidthKbps = nativeNetCapabilities.linkDownBandwidthKbps_,
        .networkCap = networkCap,
        .bearerTypes = bearerTypes,
    };
}

HttpProxy GetDefaultHttpProxy(int32_t &ret)
{
    NetManagerStandard::HttpProxy nativeHttpProxy;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetDefaultHttpProxy(nativeHttpProxy);
    auto exclusionList = rust::vec<rust::string>();
    for (const auto &s : nativeHttpProxy.GetExclusionList()) {
        exclusionList.push_back(rust::String(s));
    }
    return HttpProxy{
        .host = nativeHttpProxy.GetHost(),
        .port = nativeHttpProxy.GetPort(),
        .username = "Unknown",
        .password = "Unknown",
        .exclusionList = exclusionList,
    };
}

HttpProxy GetGlobalHttpProxy(int32_t &ret)
{
    NetManagerStandard::HttpProxy nativeHttpProxy;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetGlobalHttpProxy(nativeHttpProxy);
    auto exclusionList = rust::vec<rust::string>();
    for (const auto &s : nativeHttpProxy.GetExclusionList()) {
        exclusionList.push_back(rust::String(s));
    }
    return HttpProxy{
        .host = nativeHttpProxy.GetHost(),
        .port = nativeHttpProxy.GetPort(),
        .username = "Unknown",
        .password = "Unknown",
        .exclusionList = exclusionList,
    };
}

NetManagerStandard::HttpProxy ConvertHttpProxy(const HttpProxy &httpProxy)
{
    NetManagerStandard::HttpProxy nativeHttpProxy;
    nativeHttpProxy.SetHost(std::string(httpProxy.host));
    nativeHttpProxy.SetPort(httpProxy.port);
    std::list<std::string> exclusionList;
    for (const auto &s : httpProxy.exclusionList) {
        exclusionList.push_back(std::string(s));
    }
    nativeHttpProxy.SetExclusionList(exclusionList);

    NetManagerStandard::SecureData username;
    username.append(httpProxy.username.data(), httpProxy.username.size());
    nativeHttpProxy.SetUserName(username);

    NetManagerStandard::SecureData password;
    password.append(httpProxy.password.data(), httpProxy.password.size());
    nativeHttpProxy.SetPassword(password);

    return nativeHttpProxy;
}

int32_t SetGlobalHttpProxy(const HttpProxy &httpProxy)
{
    NetManagerStandard::HttpProxy nativeHttpProxy = ConvertHttpProxy(httpProxy);
    return NetManagerStandard::NetConnClient::GetInstance().SetGlobalHttpProxy(nativeHttpProxy);
}

int32_t SetAppHttpProxy(const HttpProxy &httpProxy)
{
    NetManagerStandard::HttpProxy nativeHttpProxy = ConvertHttpProxy(httpProxy);
    return NetManagerStandard::NetConnClient::GetInstance().SetAppHttpProxy(nativeHttpProxy);
}

NetManagerStandard::NetConnClient &GetNetConnClient(int32_t &nouse)
{
    return NetManagerStandard::NetConnClient::GetInstance();
}

int32_t BindSocket(int32_t socketFd, int32_t netId)
{
    if (socketFd <= 0) {
        NETMANAGER_BASE_LOGE("socketFd is invalid.");
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    return NetManagerStandard::NetConnClient::GetInstance().BindSocket(socketFd, netId);
}

int32_t IsDefaultNetMetered(bool &isMetered)
{
    return NetManagerStandard::NetConnClient::GetInstance().IsDefaultNetMetered(isMetered);
}

RouteInfo ConvertRouteInfo(NetManagerStandard::Route &route)
{
    return RouteInfo{
        .interface = route.iface_,
        .destination =
            LinkAddress{
                .address =
                    NetAddress{
                        .address = route.destination_.address_,
                        .family = route.destination_.family_,
                        .port = route.destination_.port_,
                    },
                .prefix_length = route.destination_.prefixlen_,
            },
        .gateway =
            NetAddress{
                .address = route.gateway_.address_,
                .family = route.gateway_.family_,
                .port = route.gateway_.port_,
            },
        .has_gateway = route.hasGateway_,
        .is_default_route = route.isDefaultRoute_,
        .is_excluded_route = route.isExcludedRoute_,
    };
}

ConnectionProperties ConvertConnectionProperties(NetManagerStandard::NetLinkInfo &info)
{
    rust::vec<LinkAddress> linkAddresses;
    for (const auto &addr : info.netAddrList_) {
        LinkAddress linkAddress{
            .address =
                NetAddress{
                    .address = addr.address_,
                    .family = addr.family_,
                    .port = addr.port_,
                },
            .prefix_length = addr.prefixlen_,
        };
        linkAddresses.push_back(linkAddress);
    }
    rust::vec<NetAddress> dnses;
    for (const auto &dns : info.dnsList_) {
        NetAddress dnsAddress{
            .address = dns.address_,
            .family = dns.family_,
            .port = dns.port_,
        };
        dnses.push_back(dnsAddress);
    }

    rust::vec<RouteInfo> routes;
    for (auto &route : info.routeList_) {
        routes.push_back(ConvertRouteInfo(route));
    }

    return ConnectionProperties{
        .interface_name = info.ifaceName_,
        .domains = info.domain_,
        .link_addresses = linkAddresses,
        .dnses = dnses,
        .routes = routes,
        .mtu = info.mtu_,
        .is_ipv6_link_valid = info.isIpv6LinkValid_,
        .is_ipv4_link_valid = info.isIpv4LinkValid_,
    };
}

ConnectionProperties GetConnectionProperties(int32_t net_id, int32_t &ret)
{
    NetManagerStandard::NetHandle netHandle;
    netHandle.SetNetId(net_id);
    NetManagerStandard::NetLinkInfo info;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetConnectionProperties(netHandle, info);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ConnectionProperties{};
    }

    return ConvertConnectionProperties(info);
}

static int32_t TransErrorCode(int32_t error)
{
    switch (error) {
        case NO_PERMISSION_CODE:
            return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
        case PERMISSION_DENIED_CODE:
            return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
        case NET_UNREACHABLE_CODE:
            return NetManagerStandard::NETMANAGER_ERR_INTERNAL;
        default:
            return NetManagerStandard::NETMANAGER_ERR_OPERATION_FAILED;
    }
}

static void SetAddressInfo(const std::string &host, addrinfo *info, NetAddress &address)
{
    address.address = rust::String(host);
    if (info->ai_addr->sa_family == AF_INET) {
        address.family = Family::IPv4;
        auto addr4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        address.port = addr4->sin_port;
    } else if (info->ai_addr->sa_family == AF_INET6) {
        address.family = Family::IPv6;
        auto addr6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        address.port = addr6->sin6_port;
    }
}

rust::vec<NetAddress> GetAddressesByName(const std::string &host, int32_t netId, int32_t &ret)
{
    addrinfo *res = nullptr;
    queryparam param;
    param.qp_type = QEURY_TYPE_NORMAL;
    param.qp_netid = netId;
    rust::vec<NetAddress> addresses;
    if (host.empty()) {
        NETMANAGER_BASE_LOGE("host is empty!");
        ret = NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        return addresses;
    }
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int status = getaddrinfo_ext(host.c_str(), nullptr, &hints, &res, &param);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s,  status: %{public}d", errno, strerror(errno),
                             status);
        ret = TransErrorCode(errno);
        return addresses;
    }

    for (addrinfo *tmp = res; tmp != nullptr; tmp = tmp->ai_next) {
        if (tmp->ai_addr == nullptr) {
            continue;
        }
        std::string addrHost;
        if (tmp->ai_family == AF_INET) {
            auto addr = reinterpret_cast<sockaddr_in *>(tmp->ai_addr);
            char ip[MAX_IPV4_STR_LEN] = {0};
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            addrHost = ip;
        } else if (tmp->ai_family == AF_INET6) {
            auto addr = reinterpret_cast<sockaddr_in6 *>(tmp->ai_addr);
            char ip[MAX_IPV6_STR_LEN] = {0};
            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            addrHost = ip;
        }

        NetAddress address;
        SetAddressInfo(addrHost, tmp, address);
        addresses.push_back(address);
    }
    freeaddrinfo(res);
    return addresses;
}

NetAddress GetAddressByName(const std::string &host, int32_t netId, int32_t &ret)
{
    addrinfo *res = nullptr;
    queryparam param;
    param.qp_type = QEURY_TYPE_NORMAL;
    param.qp_netid = netId;
    if (host.empty()) {
        NETMANAGER_BASE_LOGE("host is empty!");
        ret = NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        return NetAddress{};
    }

    int status = getaddrinfo_ext(host.c_str(), nullptr, nullptr, &res, &param);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s,  status: %{public}d", errno, strerror(errno),
                             status);
        ret = TransErrorCode(errno);
        return NetAddress{};
    }
    if (res == nullptr) {
        NETMANAGER_BASE_LOGE("addrinfo is nullptr!");
        return NetAddress{};
    }

    std::string addrHost;
    if (res->ai_family == AF_INET) {
        auto addr = reinterpret_cast<sockaddr_in *>(res->ai_addr);
        char ip[MAX_IPV4_STR_LEN] = {0};
        inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
        addrHost = ip;
    } else if (res->ai_family == AF_INET6) {
        auto addr = reinterpret_cast<sockaddr_in6 *>(res->ai_addr);
        char ip[MAX_IPV6_STR_LEN] = {0};
        inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
        addrHost = ip;
    }

    NetAddress address;
    SetAddressInfo(addrHost, res, address);

    freeaddrinfo(res);
    return address;
}

void NetDetection(int32_t netId, int32_t &ret)
{
    NetManagerStandard::NetHandle netHandle;
    netHandle.SetNetId(netId);
    ret = NetManagerStandard::NetConnClient::GetInstance().NetDetection(netHandle);
}

NetCoonCallback::NetCoonCallback(rust::Box<ConnCallback> callback) : inner_(std::move(callback)) {}

int32_t NetCoonCallback::NetAvailable(sptr<NetManagerStandard::NetHandle> &netHandle)
{
    NetHandle handle{
        .net_id = netHandle->GetNetId(),
    };
    return inner_->on_net_available(handle);
}

int32_t NetCoonCallback::NetCapabilitiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
                                               const sptr<NetManagerStandard::NetAllCapabilities> &netAllCap)
{
    if (netHandle == nullptr || netAllCap == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }
    rust::vec<NetManagerStandard::NetCap> networkCap;
    for (auto &cap : netAllCap->netCaps_) {
        networkCap.push_back(cap);
    }
    rust::vec<NetManagerStandard::NetBearType> bearerTypes;
    for (auto &bearerType : netAllCap->bearerTypes_) {
        bearerTypes.push_back(bearerType);
    }

    NetCapabilityInfo info{
        .net_handle = NetHandle{.net_id = netHandle->GetNetId()},
        .net_cap =
            NetCapabilities{
                .linkUpBandwidthKbps = netAllCap->linkUpBandwidthKbps_,
                .linkDownBandwidthKbps = netAllCap->linkDownBandwidthKbps_,
                .networkCap = networkCap,
                .bearerTypes = bearerTypes,
            },
    };
    return inner_->on_net_capabilities_change(info);
}

int32_t NetCoonCallback::NetConnectionPropertiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
                                                       const sptr<NetManagerStandard::NetLinkInfo> &linkInfo)
{
    if (netHandle == nullptr || linkInfo == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }
    NetConnectionPropertyInfo info{
        .net_handle = NetHandle{.net_id = netHandle->GetNetId()},
        .connection_properties = ConvertConnectionProperties(*linkInfo),
    };
    return inner_->on_net_connection_properties_change(info);
}

int32_t NetCoonCallback::NetLost(sptr<NetManagerStandard::NetHandle> &netHandle)
{
    return inner_->on_net_lost(NetHandle{.net_id = netHandle->GetNetId()});
}

int32_t NetCoonCallback::NetUnavailable()
{
    return inner_->on_net_unavailable();
}

int32_t NetCoonCallback::NetBlockStatusChange(sptr<NetManagerStandard::NetHandle> &netHandle, bool blocked)
{
    NetBlockStatusInfo info{
        .net_handle = NetHandle{.net_id = netHandle->GetNetId()},
        .blocked = blocked,
    };
    return inner_->on_net_block_status_change(info);
}

std::unique_ptr<UnregisterHandle> RegisterNetConnCallback(rust::Box<ConnCallback> Connection, int32_t &ret)
{
    auto callback = sptr<NetCoonCallback>::MakeSptr(std::move(Connection));
    ret = NetManagerStandard::NetConnClient::GetInstance().RegisterNetConnCallback(callback);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return nullptr;
    }
    auto unregisterHandle = std::make_unique<UnregisterHandle>(callback);
    return unregisterHandle;
}

std::unique_ptr<UnregisterHandle> RegisterNetConnCallbackWithSpecifier(rust::Box<ConnCallback> Connection,
    const NetSpecifier &netSpecifier, int32_t timeout, int32_t &ret)
{
    auto callback = sptr<NetCoonCallback>::MakeSptr(std::move(Connection));
    sptr<NetManagerStandard::NetSpecifier> specifier = new NetManagerStandard::NetSpecifier();
    specifier->ident_ = std::string(netSpecifier.bearer_private_identifier);
    specifier->netCapabilities_.linkUpBandwidthKbps_ =
        static_cast<uint32_t>(netSpecifier.net_capabilities.linkUpBandwidthKbps);
    specifier->netCapabilities_.linkDownBandwidthKbps_ =
        static_cast<uint32_t>(netSpecifier.net_capabilities.linkDownBandwidthKbps);
    for (const auto &cap : netSpecifier.net_capabilities.networkCap) {
        specifier->netCapabilities_.netCaps_.insert(cap);
    }
    for (const auto &type : netSpecifier.net_capabilities.bearerTypes) {
        specifier->netCapabilities_.bearerTypes_.insert(type);
    }
    uint32_t timeoutMs = timeout < 0 ? 0 : static_cast<uint32_t>(timeout);
    ret = NetManagerStandard::NetConnClient::GetInstance().RegisterNetConnCallback(
        specifier, callback, timeoutMs);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return nullptr;
    }
    auto unregisterHandle = std::make_unique<UnregisterHandle>(callback);
    return unregisterHandle;
}

UnregisterHandle::UnregisterHandle(sptr<NetCoonCallback> callback) : callback_(callback) {}

int32_t UnregisterHandle::Unregister()
{
    return NetManagerStandard::NetConnClient::GetInstance().UnregisterNetConnCallback(callback_);
}

int32_t SetCustomDnsRule(const std::string &host, const rust::Vec<rust::String> &ips)
{
    if (!NetManagerStandard::CommonUtils::HasInternetPermission()) {
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    if (host.empty() || ips.empty()) {
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    for (const auto &ip : ips) {
        std::string ipStr(ip);
        if (!NetManagerStandard::CommonUtils::IsValidIPV4(ipStr) &&
            !NetManagerStandard::CommonUtils::IsValidIPV6(ipStr)) {
            return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
        }
    }
    std::string hostIps = host + ",";
    for (size_t i = 0; i < ips.size(); i++) {
        hostIps.append(std::string(ips[i]));
        if (i < ips.size() - 1) {
            hostIps.append(",");
        }
    }
    std::lock_guard<std::mutex> lock(g_predefinedHostMtx);
    return predefined_host_set_hosts(hostIps.c_str());
}

int32_t RemoveCustomDnsRule(const std::string &host)
{
    if (!NetManagerStandard::CommonUtils::HasInternetPermission()) {
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    if (host.empty()) {
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    std::lock_guard<std::mutex> lock(g_predefinedHostMtx);
    return predefined_host_remove_host(host.c_str());
}

int32_t ClearCustomDnsRules()
{
    if (!NetManagerStandard::CommonUtils::HasInternetPermission()) {
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::mutex> lock(g_predefinedHostMtx);
    return predefined_host_clear_all_hosts();
}

HttpProxy RefreshGlobalHttpProxySync(int32_t &ret)
{
    HttpProxy resultProxy{};
    std::mutex mtx;
    std::condition_variable cv;
    bool callbackCalled = false;

    auto callback = [&mtx, &cv, &callbackCalled, &ret, &resultProxy](int32_t resultCode,
                       const NetManagerStandard::HttpProxy &httpProxy) {
        ret = resultCode;
        if (resultCode == NetManagerStandard::NETMANAGER_SUCCESS) {
            auto exclusionList = rust::vec<rust::string>();
            for (const auto &s : httpProxy.GetExclusionList()) {
                exclusionList.push_back(rust::String(s));
            }
            resultProxy = HttpProxy{
                .host = httpProxy.GetHost(),
                .port = httpProxy.GetPort(),
                .username = std::string(httpProxy.GetUsername()),
                .password = std::string(httpProxy.GetPassword()),
                .exclusionList = exclusionList,
            };
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            callbackCalled = true;
        }
        cv.notify_one();
    };

    int32_t refreshRet = NetManagerStandard::NetConnClient::GetInstance().RefreshGlobalHttpProxy(callback);
    if (refreshRet != NetManagerStandard::NETMANAGER_SUCCESS) {
        ret = refreshRet;
        return HttpProxy{};
    }

    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&callbackCalled] { return callbackCalled; });

    return resultProxy;
}

int32_t SetPacUrl(const std::string &pacUrl)
{
    if (pacUrl.empty()) {
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    return NetManagerStandard::NetConnClient::GetInstance().SetPacUrl(pacUrl);
}

int32_t SetPacFileUrl(const std::string &pacUrl)
{
    return NetManagerStandard::NetConnClient::GetInstance().SetPacFileUrl(pacUrl);
}

rust::String FindProxyForURL(const std::string &url, int32_t &ret)
{
    std::string proxy;
    ret = NetManagerStandard::NetConnClient::GetInstance().FindProxyForURL(url, proxy);
    return rust::String(proxy);
}

rust::vec<NetAddress> GetAddressesByNameWithOptions(const std::string &host, int32_t netId, int32_t family,
    int32_t &ret)
{
    addrinfo *res = nullptr;
    queryparam param;
    param.qp_type = QEURY_TYPE_NORMAL;
    param.qp_netid = netId;
    rust::vec<NetAddress> addresses;
    if (host.empty() || family < static_cast<int32_t>(Family::All) ||
        family > static_cast<int32_t>(Family::IPv6)) {
        ret = NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        return addresses;
    }
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    if (family == Family::IPv4) {
        hints.ai_family = AF_INET;
    } else if (family == Family::IPv6) {
        hints.ai_family = AF_INET6;
    }
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int status = getaddrinfo_ext(host.c_str(), nullptr, &hints, &res, &param);
    if (status < 0) {
        ret = TransErrorCode(errno);
        return addresses;
    }
    for (addrinfo *tmp = res; tmp != nullptr; tmp = tmp->ai_next) {
        if (tmp->ai_addr == nullptr) {
            continue;
        }
        std::string addrHost;
        if (tmp->ai_family == AF_INET) {
            auto addr = reinterpret_cast<sockaddr_in *>(tmp->ai_addr);
            char ip[MAX_IPV4_STR_LEN] = {0};
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            addrHost = ip;
        } else if (tmp->ai_family == AF_INET6) {
            auto addr = reinterpret_cast<sockaddr_in6 *>(tmp->ai_addr);
            char ip[MAX_IPV6_STR_LEN] = {0};
            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            addrHost = ip;
        }
        NetAddress address;
        SetAddressInfo(addrHost, tmp, address);
        addresses.push_back(address);
    }
    freeaddrinfo(res);
    return addresses;
}

int32_t CreateVlanInterface(const std::string &ifName, uint32_t vlanId)
{
    return NetManagerStandard::NetConnClient::GetInstance().CreateVlan(ifName, vlanId);
}

int32_t DestroyVlanInterface(const std::string &ifName, uint32_t vlanId)
{
    return NetManagerStandard::NetConnClient::GetInstance().DestroyVlan(ifName, vlanId);
}

int32_t AddVlanIp(const std::string &ifName, uint32_t vlanId, const std::string &ip, uint32_t mask)
{
    return NetManagerStandard::NetConnClient::GetInstance().AddVlanIp(ifName, vlanId, ip, mask);
}

int32_t DeleteVlanIp(const std::string &ifName, uint32_t vlanId, const std::string &ip, uint32_t mask)
{
    return NetManagerStandard::NetConnClient::GetInstance().DeleteVlanIp(ifName, vlanId, ip, mask);
}

AniNetPortStatesInfo GetSystemNetPortStates(int32_t &ret)
{
    NetManagerStandard::NetPortStatesInfo info;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetSystemNetPortStates(info);
    AniNetPortStatesInfo result;
    for (const auto &tcp : info.tcpNetPortStatesInfo_) {
        result.tcpPortStatesInfo.push_back(AniTcpNetPortStatesInfo{
            .tcpLocalIp = tcp.tcpLocalIp_,
            .tcpLocalPort = tcp.tcpLocalPort_,
            .tcpRemoteIp = tcp.tcpRemoteIp_,
            .tcpRemotePort = tcp.tcpRemotePort_,
            .tcpUid = static_cast<int32_t>(tcp.tcpUid_),
            .tcpPid = static_cast<int32_t>(tcp.tcpPid_),
            .tcpState = static_cast<int32_t>(tcp.tcpState_),
        });
    }
    for (const auto &udp : info.udpNetPortStatesInfo_) {
        result.udpPortStatesInfo.push_back(AniUdpNetPortStatesInfo{
            .udpLocalIp = udp.udpLocalIp_,
            .udpLocalPort = udp.udpLocalPort_,
            .udpUid = static_cast<int32_t>(udp.udpUid_),
            .udpPid = static_cast<int32_t>(udp.udpPid_),
        });
    }
    return result;
}

rust::vec<AniNetIpMacInfo> GetIpNeighTable(int32_t &ret)
{
    std::vector<NetManagerStandard::NetIpMacInfo> ipMacInfoList;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetIpNeighTable(ipMacInfoList);
    rust::vec<AniNetIpMacInfo> result;
    for (const auto &info : ipMacInfoList) {
        result.push_back(AniNetIpMacInfo{
            .ipAddress = info.ipAddress_,
            .family = static_cast<int32_t>(info.family_),
            .macAddress = info.macAddress_,
            .iface = info.iface_,
        });
    }
    return result;
}

int32_t GetConnectOwnerUid(int32_t protocolType, const NetAddress &localAddress,
    const NetAddress &remoteAddress, int32_t &ret)
{
    auto normalizeFamily = [](int32_t family) {
        return family == static_cast<int32_t>(NetManagerStandard::NetConnInfo::Family::IPv6)
            ? static_cast<int32_t>(NetManagerStandard::NetConnInfo::Family::IPv6)
            : static_cast<int32_t>(NetManagerStandard::NetConnInfo::Family::IPv4);
    };
    if (normalizeFamily(localAddress.family) != normalizeFamily(remoteAddress.family)) {
        ret = NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        return -1;
    }
    NetManagerStandard::NetConnInfo connInfo;
    connInfo.protocolType_ = protocolType;
    connInfo.family_ = static_cast<NetManagerStandard::NetConnInfo::Family>(
        normalizeFamily(localAddress.family));
    connInfo.localAddress_ = std::string(localAddress.address);
    connInfo.localPort_ = static_cast<uint16_t>(localAddress.port);
    connInfo.remoteAddress_ = std::string(remoteAddress.address);
    connInfo.remotePort_ = static_cast<uint16_t>(remoteAddress.port);
    int32_t ownerUid = -1;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetConnectOwnerUid(connInfo, ownerUid);
    return ownerUid;
}

rust::String GetDnsUnicode(const std::string &host, int32_t conversionProcess, int32_t &ret)
{
    if (host.empty()) {
        ret = NetManagerStandard::NETMANAGER_SUCCESS;
        return rust::String();
    }
    std::string validHost = host;
    if (validHost.length() > MAX_HOST_LENGTH) {
        validHost = validHost.substr(0, MAX_HOST_LENGTH);
    }
    std::string unicode;
    ret = NetManagerStandard::ICUHelper::GetDnsUnicode(
        validHost, static_cast<NetManagerStandard::ConversionProcess>(conversionProcess), unicode);
    return rust::String(unicode);
}

rust::String GetDnsAscii(const std::string &host, int32_t conversionProcess, int32_t &ret)
{
    if (host.empty()) {
        ret = NetManagerStandard::NETMANAGER_SUCCESS;
        return rust::String();
    }
    std::string ascii;
    ret = NetManagerStandard::ICUHelper::GetDnsASCII(
        host, static_cast<NetManagerStandard::ConversionProcess>(conversionProcess), ascii);
    return rust::String(ascii);
}

int32_t SetInterfaceUp(const std::string &iface)
{
    return NetManagerStandard::NetConnClient::GetInstance().SetInterfaceUp(iface);
}

AniProbeResultInfo QueryProbeResult(const std::string &dest, int32_t duration, int32_t &ret)
{
    if (dest.empty() || duration <= 0 || duration > MAX_PING_DURATION) {
        ret = NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        return AniProbeResultInfo{};
    }
    if (!NetManagerStandard::CommonUtils::HasInternetPermission()) {
        ret = NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
        return AniProbeResultInfo{};
    }
    NetManagerStandard::NetProbe netProbe;
    NetManagerStandard::NetConn_ProbeResultInfo probeResult = {0};
    std::string tempDest = dest;
    ret = netProbe.QueryProbeResult(tempDest, duration, probeResult);
    AniProbeResultInfo result;
    if (ret == NetManagerStandard::NETMANAGER_SUCCESS) {
        result.lossRate = static_cast<int32_t>(probeResult.lossRate);
        for (int i = 0; i < NetManagerStandard::NETCONN_MAX_RTT_NUM; i++) {
            result.rtt.push_back(static_cast<int32_t>(probeResult.rtt[i]));
        }
    }
    return result;
}

std::vector<std::string> splitStr(const std::string &str, const char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int32_t Conv2TraceRouteInfoRtt(const std::string &rttStr, rust::Vec<uint32_t> &rtt)
{
    std::vector<std::string> tokens = splitStr(rttStr, ';');
    uint32_t tokensSize = tokens.size();
    std::string max = tokens[0];
    uint8_t maxIdx = 2;
    for (uint32_t i = 0; i < tokensSize && i < NetManagerStandard::NETCONN_MAX_RTT_NUM; ++i) {
        if (i < tokensSize - maxIdx) {
            tokens[i] = tokens[i + 1];
        } else if (i == tokensSize - maxIdx) {
            tokens[i] = max;
        }
        uint32_t num = 0;
        std::istringstream iss(tokens[i]);
        if (iss >> num) {
            rtt.push_back(num);
        }
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t Conv2TraceRouteInfo(const std::string &traceRouteInfoStr,
    rust::Vec<AniTraceRouteInfo> &traceRouteInfo, uint32_t maxJumpNumber)
{
    if (traceRouteInfoStr == "") {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }

    // traceRouteInfo is "1 *.*.*.*;2;3;4 ..." pos is space position
    const uint32_t pos2 = 2;
    const uint32_t pos3 = 3;
    std::vector<std::string> tokens = splitStr(traceRouteInfoStr, ' ');
    uint32_t tokensSize = static_cast<uint32_t>(tokens.size());
    for (uint32_t i = 0; i < tokensSize / pos3 && i < maxJumpNumber; i++) {
        AniTraceRouteInfo info;
        uint32_t num = 0;
        std::istringstream iss(tokens[i * pos3]);
        if (iss >> num) {
            info.jumpNo = static_cast<uint8_t>(num);
        }
        info.address = tokens[i * pos3 + 1];
        if (Conv2TraceRouteInfoRtt(tokens[i * pos3 + pos2], info.rtt) != NetManagerStandard::NETMANAGER_SUCCESS) {
            return NetManagerStandard::NETMANAGER_ERR_INTERNAL;
        }
        traceRouteInfo.push_back(std::move(info));
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

rust::vec<AniTraceRouteInfo> QueryTraceRoute(const std::string &destination, int32_t maxJumpNumber,
    int32_t packetsType, int32_t &ret)
{
    std::string traceRouteInfoStr;
    ret = NetManagerStandard::NetConnClient::GetInstance().QueryTraceRoute(
        destination, maxJumpNumber, packetsType, traceRouteInfoStr, false);
    rust::vec<AniTraceRouteInfo> result;
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return result;
    }
    (void)Conv2TraceRouteInfo(traceRouteInfoStr, result, static_cast<uint32_t>(maxJumpNumber));
    return result;
}

int32_t GetProxyMode(int32_t &mode)
{
    NetManagerStandard::ProxyModeType proxyMode;
    int32_t ret = NetManagerStandard::NetConnClient::GetInstance().GetProxyMode(proxyMode);
    if (ret == NetManagerStandard::NETMANAGER_SUCCESS) {
        mode = static_cast<int32_t>(proxyMode);
    }
    return ret;
}

int32_t SetProxyMode(int32_t mode)
{
    if (mode < 0 || mode > 1) {
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    auto proxyMode = static_cast<NetManagerStandard::ProxyModeType>(mode);
    return NetManagerStandard::NetConnClient::GetInstance().SetProxyMode(proxyMode);
}

rust::String GetPacFileUrl(int32_t &ret)
{
    std::string pacUrl;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetPacFileUrl(pacUrl);
    return rust::String(pacUrl);
}

int32_t SetNetExtAttribute(int32_t netId, const std::string &netExtAttribute)
{
    if (!NetManagerStandard::NetManagerPermission::CheckPermission(
        NetManagerStandard::Permission::SET_NET_EXT_ATTRIBUTE)) {
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    NetManagerStandard::NetHandle netHandle(netId);
    return NetManagerStandard::NetConnClient::GetInstance().SetNetExtAttribute(netHandle, netExtAttribute);
}

rust::String GetNetExtAttribute(int32_t netId, int32_t &ret)
{
    if (!NetManagerStandard::NetManagerPermission::CheckPermission(
        NetManagerStandard::Permission::GET_NETWORK_INFO)) {
        ret = NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
        return rust::String();
    }
    std::string netExtAttribute;
    NetManagerStandard::NetHandle netHandle(netId);
    ret = NetManagerStandard::NetConnClient::GetInstance().GetNetExtAttribute(netHandle, netExtAttribute);
    return rust::String(netExtAttribute);
}

} // namespace NetManagerAni
} // namespace OHOS