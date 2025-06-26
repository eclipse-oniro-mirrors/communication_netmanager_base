/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "connection_ani.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "cxx.h"
#include "http_proxy.h"
#include "inet_addr.h"
#include "net_conn_client.h"
#include "net_handle.h"
#include "net_link_info.h"
#include "netmanager_secure_data.h"
#include "refbase.h"
#include "tokenid_kit.h"
#include "wrapper.rs.h"
#include <memory>
#include <string>
namespace OHOS {
namespace NetManagerAni {
using namespace Security::AccessToken;

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

// Cxx has to have a &mut argument to return a &mut type
NetManagerStandard::NetConnClient &GetNetConnClient(int32_t &nouse)
{
    return NetManagerStandard::NetConnClient::GetInstance();
}

int32_t IsDefaultNetMetered(bool &isMetered)
{
    return NetManagerStandard::NetConnClient::GetInstance().IsDefaultNetMetered(isMetered);
};

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
    };
}

ConnectionProperties GetConnectionProperties(int32_t net_id, int32_t &ret)
{
    NetManagerStandard::NetHandle netHandle;
    netHandle.SetNetId(net_id);
    NetManagerStandard::NetLinkInfo info;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetConnectionProperties(netHandle, info);
    if (ret != 0) {
        return ConnectionProperties{};
    }

    return ConvertConnectionProperties(info);
}

rust::vec<NetAddress> GetAddressesByName(const std::string &host, int32_t netId, int32_t &ret)
{
    std::vector<NetManagerStandard::INetAddr> addrList;
    rust::vec<NetAddress> addresses;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetAddressesByName(host, netId, addrList);
    if (ret != 0) {
        return addresses;
    }

    for (const auto &addr : addrList) {
        NetAddress address{
            .address = addr.address_,
            .family = addr.family_,
            .port = addr.port_,
        };
        addresses.push_back(address);
    }
    return addresses;
}

NetAddress GetAddressByName(const std::string &host, int32_t netId, int32_t &ret)
{
    NetManagerStandard::INetAddr addr;
    ret = NetManagerStandard::NetConnClient::GetInstance().GetAddressByName(host, netId, addr);
    if (ret != 0) {
        return NetAddress{};
    }
    return NetAddress{
        .address = addr.address_,
        .family = addr.family_,
        .port = addr.port_,
    };
}

void NetDetection(int32_t netId, int32_t &ret)
{
    NetManagerStandard::NetHandle netHandle;
    netHandle.SetNetId(netId);
    ret = NetManagerStandard::NetConnClient::GetInstance().NetDetection(netHandle);
}

bool CheckPermission(uint64_t tokenId, rust::str permission)
{
    auto perm = std::string(permission);
    TypeATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(static_cast<AccessTokenID>(tokenId));
    if (tokenType == TOKEN_INVALID) {
        return false;
    }
    int result = AccessTokenKit::VerifyAccessToken(tokenId, perm);
    if (result != PERMISSION_GRANTED) {
        return false;
    }
    return true;
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
    if (ret != 0) {
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

} // namespace NetManagerAni
} // namespace OHOS