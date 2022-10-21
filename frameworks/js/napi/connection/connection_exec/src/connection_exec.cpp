/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "connection_exec.h"

#include "connection_module.h"
#include "constant.h"
#include "net_conn_client.h"
#include "netconnection.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_log.h"
#include "napi_utils.h"
#include "securec.h"

static constexpr const size_t MAX_ARRAY_LENGTH = 64;

static constexpr const size_t MAX_IPV4_STR_LEN = 16;

static constexpr const size_t MAX_IPV6_STR_LEN = 64;

namespace OHOS::NetManagerStandard {
napi_value ConnectionExec::CreateNetHandle(napi_env env, NetHandle *handle)
{
    napi_value netHandle = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, netHandle) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(ConnectionModule::NetHandleInterface::FUNCTION_GET_ADDRESSES_BY_NAME,
                              ConnectionModule::NetHandleInterface::GetAddressesByName),
        DECLARE_NAPI_FUNCTION(ConnectionModule::NetHandleInterface::FUNCTION_GET_ADDRESS_BY_NAME,
                              ConnectionModule::NetHandleInterface::GetAddressByName),
        DECLARE_NAPI_FUNCTION(ConnectionModule::NetHandleInterface::FUNCTION_BIND_SOCKET,
                              ConnectionModule::NetHandleInterface::BindSocket),
    };
    NapiUtils::DefineProperties(env, netHandle, properties);
    NapiUtils::SetUint32Property(env, netHandle, ConnectionModule::NetHandleInterface::PROPERTY_NET_ID,
                                 handle->GetNetId());
    return netHandle;
}

napi_value ConnectionExec::CreateNetCapabilities(napi_env env, NetAllCapabilities *capabilities)
{
    napi_value netCapabilities = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, netCapabilities) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    NapiUtils::SetUint32Property(env, netCapabilities, KEY_LINK_UP_BAND_WIDTH_KPS, capabilities->linkUpBandwidthKbps_);
    NapiUtils::SetUint32Property(env, netCapabilities, KEY_LINK_DOWN_BAND_WIDTH_KPS,
                                 capabilities->linkDownBandwidthKbps_);
    NETMANAGER_BASE_LOGI("capabilities->netCaps_.size() = %{public}zu", capabilities->netCaps_.size());
    if (!capabilities->netCaps_.empty() && capabilities->netCaps_.size() <= MAX_ARRAY_LENGTH) {
        napi_value networkCap = NapiUtils::CreateArray(env, std::min(capabilities->netCaps_.size(), MAX_ARRAY_LENGTH));
        auto it = capabilities->netCaps_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != capabilities->netCaps_.end(); ++index, ++it) {
            NapiUtils::SetArrayElement(env, networkCap, index, NapiUtils::CreateUint32(env, *it));
        }
        NapiUtils::SetNamedProperty(env, netCapabilities, KEY_NETWORK_CAP, networkCap);
    }
    NETMANAGER_BASE_LOGI("capabilities->bearerTypes_.size() = %{public}zu", capabilities->bearerTypes_.size());
    if (!capabilities->bearerTypes_.empty() && capabilities->bearerTypes_.size() <= MAX_ARRAY_LENGTH) {
        napi_value bearerTypes =
            NapiUtils::CreateArray(env, std::min(capabilities->bearerTypes_.size(), MAX_ARRAY_LENGTH));
        auto it = capabilities->bearerTypes_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != capabilities->bearerTypes_.end(); ++index, ++it) {
            NapiUtils::SetArrayElement(env, bearerTypes, index, NapiUtils::CreateUint32(env, *it));
        }
        NapiUtils::SetNamedProperty(env, netCapabilities, KEY_BEARER_TYPE, bearerTypes);
    }
    return netCapabilities;
}

napi_value ConnectionExec::CreateConnectionProperties(napi_env env, NetLinkInfo *linkInfo)
{
    napi_value connectionProperties = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, connectionProperties) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }
    NapiUtils::SetStringPropertyUtf8(env, connectionProperties, KEY_INTERFACE_NAME, linkInfo->ifaceName_);
    NapiUtils::SetStringPropertyUtf8(env, connectionProperties, KEY_DOMAINS, linkInfo->domain_);
    NapiUtils::SetUint32Property(env, connectionProperties, KEY_MTU, linkInfo->mtu_);
    FillLinkAddress(env, connectionProperties, linkInfo);
    FillRouoteList(env, connectionProperties, linkInfo);
    FillDns(env, connectionProperties, linkInfo);
    return connectionProperties;
}

bool ConnectionExec::ExecGetAddressByName(GetAddressByNameContext *context)
{
    return NetHandleExec::ExecGetAddressesByName(context);
}

napi_value ConnectionExec::GetAddressByNameCallback(GetAddressByNameContext *context)
{
    return NetHandleExec::GetAddressesByNameCallback(context);
}

bool ConnectionExec::ExecGetDefaultNet(GetDefaultNetContext *context)
{
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(context->netHandle);
    NETMANAGER_BASE_LOGI("ExecGetDefaultNet ret %{public}d", ret);
    if (ret != NET_CONN_SUCCESS && ret != NET_CONN_ERR_NO_DEFAULT_NET) {
        context->SetErrorCode(ret);
        return false;
    }
    return true;
}

napi_value ConnectionExec::GetDefaultNetCallback(GetDefaultNetContext *context)
{
    return CreateNetHandle(context->GetEnv(), &context->netHandle);
}

bool ConnectionExec::ExecHasDefaultNet(HasDefaultNetContext *context)
{
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(context->hasDefaultNet);
    NETMANAGER_BASE_LOGI("ExecHasDefaultNet ret %{public}d", ret);
    if (ret != NET_CONN_SUCCESS && ret != NET_CONN_ERR_NO_DEFAULT_NET) {
        context->SetErrorCode(ret);
        return false;
    }
    return true;
}

napi_value ConnectionExec::HasDefaultNetCallback(HasDefaultNetContext *context)
{
    return NapiUtils::GetBoolean(context->GetEnv(), context->hasDefaultNet);
}

bool ConnectionExec::ExecGetNetCapabilities(GetNetCapabilitiesContext *context)
{
    auto ret =
        DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(context->netHandle, context->capabilities);
    if (ret != NET_CONN_SUCCESS) {
        context->SetErrorCode(ret);
        return false;
    }
    return true;
}

napi_value ConnectionExec::GetNetCapabilitiesCallback(GetNetCapabilitiesContext *context)
{
    return CreateNetCapabilities(context->GetEnv(), &context->capabilities);
}

bool ConnectionExec::ExecGetConnectionProperties(GetConnectionPropertiesContext *context)
{
    auto ret =
        DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(context->netHandle, context->linkInfo);
    if (ret != NET_CONN_SUCCESS) {
        context->SetErrorCode(ret);
        return false;
    }
    return true;
}

napi_value ConnectionExec::GetConnectionPropertiesCallback(GetConnectionPropertiesContext *context)
{
    return CreateConnectionProperties(context->GetEnv(), &context->linkInfo);
}

bool ConnectionExec::ExecGetAllNets(GetAllNetsContext *context)
{
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(context->netHandleList);
    if (ret != NET_CONN_SUCCESS) {
        context->SetErrorCode(ret);
        return false;
    }
    return true;
}

napi_value ConnectionExec::GetAllNetsCallback(GetAllNetsContext *context)
{
    napi_value array = NapiUtils::CreateArray(context->GetEnv(), context->netHandleList.size());
    uint32_t index = 0;
    std::for_each(context->netHandleList.begin(), context->netHandleList.end(),
                  [array, &index, context](const sptr<NetHandle> &handle) {
                      NapiUtils::SetArrayElement(context->GetEnv(), array, index,
                                                 CreateNetHandle(context->GetEnv(), handle.GetRefPtr()));
                      ++index;
                  });
    return array;
}

bool ConnectionExec::ExecEnableAirplaneMode(EnableAirplaneModeContext *context)
{
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(true);
    if (res != 0) {
        NETMANAGER_BASE_LOGE("ExecEnableAirplaneMode failed %{public}d", res);
        context->SetErrorCode(res);
    }
    NETMANAGER_BASE_LOGE("ExecEnableAirplaneMode OK");
    return res == 0;
}

napi_value ConnectionExec::EnableAirplaneModeCallback(EnableAirplaneModeContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecDisableAirplaneMode(DisableAirplaneModeContext *context)
{
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(false);
    if (res != 0) {
        NETMANAGER_BASE_LOGE("ExecDisableAirplaneMode failed %{public}d", res);
        context->SetErrorCode(res);
    }
    NETMANAGER_BASE_LOGE("ExecDisableAirplaneMode OK");
    return res == 0;
}

napi_value ConnectionExec::DisableAirplaneModeCallback(DisableAirplaneModeContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecReportNetConnected(ReportNetConnectedContext *context)
{
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(context->netHandle);
    if (res != 0) {
        NETMANAGER_BASE_LOGE("ExecReportNetConnected failed %{public}d", res);
        context->SetErrorCode(res);
    }
    NETMANAGER_BASE_LOGE("ExecReportNetConnected OK");
    return res == 0;
}

napi_value ConnectionExec::ReportNetConnectedCallback(ReportNetConnectedContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecReportNetDisconnected(ReportNetConnectedContext *context)
{
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(context->netHandle);
    if (res != 0) {
        NETMANAGER_BASE_LOGE("ExecReportNetDisconnected failed %{public}d", res);
        context->SetErrorCode(res);
    }
    NETMANAGER_BASE_LOGE("ExecReportNetDisconnected OK");
    return res == 0;
}

napi_value ConnectionExec::ReportNetDisconnectedCallback(ReportNetConnectedContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::NetHandleExec::ExecGetAddressesByName(GetAddressByNameContext *context)
{
    addrinfo *res = nullptr;
    int status = getaddrinfo(context->host.c_str(), nullptr, nullptr, &res);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s", errno, strerror(errno));
        context->SetErrorCode(errno);
        return false;
    }

    for (addrinfo *tmp = res; tmp != nullptr; tmp = tmp->ai_next) {
        std::string host;
        if (tmp->ai_family == AF_INET) {
            auto addr = reinterpret_cast<sockaddr_in *>(tmp->ai_addr);
            char ip[MAX_IPV4_STR_LEN] = {0};
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            host = ip;
        } else if (tmp->ai_family == AF_INET6) {
            auto addr = reinterpret_cast<sockaddr_in6 *>(tmp->ai_addr);
            char ip[MAX_IPV6_STR_LEN] = {0};
            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            host = ip;
        }
        NETMANAGER_BASE_LOGI("host ip: %{public}s", CommonUtils::ToAnonymousIp(host).c_str());

        NetAddress address;
        SetAddressInfo(host.c_str(), tmp, address);

        context->addresses.emplace_back(address);
    }
    freeaddrinfo(res);
    return true;
}

napi_value ConnectionExec::NetHandleExec::GetAddressesByNameCallback(GetAddressByNameContext *context)
{
    napi_value addresses = NapiUtils::CreateArray(context->GetEnv(), context->addresses.size());
    for (uint32_t index = 0; index < context->addresses.size(); ++index) {
        napi_value obj = MakeNetAddressJsValue(context->GetEnv(), context->addresses[index]);
        NapiUtils::SetArrayElement(context->GetEnv(), addresses, index, obj);
    }
    return addresses;
}

bool ConnectionExec::NetHandleExec::ExecGetAddressByName(GetAddressByNameContext *context)
{
    addrinfo *res = nullptr;
    int status = getaddrinfo(context->host.c_str(), nullptr, nullptr, &res);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s", errno, strerror(errno));
        context->SetErrorCode(errno);
        return false;
    }

    if (res != nullptr) {
        std::string host;
        if (res->ai_family == AF_INET) {
            auto addr = reinterpret_cast<sockaddr_in *>(res->ai_addr);
            char ip[MAX_IPV4_STR_LEN] = {0};
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            host = ip;
        } else if (res->ai_family == AF_INET6) {
            auto addr = reinterpret_cast<sockaddr_in6 *>(res->ai_addr);
            char ip[MAX_IPV6_STR_LEN] = {0};
            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            host = ip;
        }
        NETMANAGER_BASE_LOGI("host ip: %{public}s", CommonUtils::ToAnonymousIp(host).c_str());

        NetAddress address;
        SetAddressInfo(host.c_str(), res, address);

        context->addresses.emplace_back(address);
    }
    freeaddrinfo(res);
    return true;
}

napi_value ConnectionExec::NetHandleExec::GetAddressByNameCallback(GetAddressByNameContext *context)
{
    if (context->addresses.empty()) {
        return NapiUtils::GetUndefined(context->GetEnv());
    }
    return MakeNetAddressJsValue(context->GetEnv(), context->addresses[0]);
}

napi_value ConnectionExec::NetHandleExec::MakeNetAddressJsValue(napi_env env, const NetAddress &address)
{
    napi_value obj = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, obj) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    NapiUtils::SetStringPropertyUtf8(env, obj, KEY_ADDRESS, address.GetAddress());
    NapiUtils::SetUint32Property(env, obj, KEY_FAMILY, address.GetJsValueFamily());
    NapiUtils::SetUint32Property(env, obj, KEY_PORT, address.GetPort());
    return obj;
}

bool ConnectionExec::NetHandleExec::ExecBindSocket(BindSocketContext *context)
{
    NetHandle handle(context->netId);
    int32_t res = handle.BindSocket(context->socketFd);
    if (res != 0) {
        NETMANAGER_BASE_LOGE("ExecBindSocket failed %{public}d", res);
        context->SetErrorCode(res);
    }
    NETMANAGER_BASE_LOGE("ExecBindSocket OK");
    return res == 0;
}

napi_value ConnectionExec::NetHandleExec::BindSocketCallback(BindSocketContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

void ConnectionExec::NetHandleExec::SetAddressInfo(const char *host, addrinfo *info, NetAddress &address)
{
    address.SetAddress(host);
    address.SetFamilyBySaFamily(info->ai_addr->sa_family);
    if (info->ai_addr->sa_family == AF_INET) {
        auto addr4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        address.SetPort(addr4->sin_port);
    } else if (info->ai_addr->sa_family == AF_INET6) {
        auto addr6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        address.SetPort(addr6->sin6_port);
    }
}

bool ConnectionExec::NetConnectionExec::ExecRegister(RegisterContext *context)
{
    NETMANAGER_BASE_LOGI("ConnectionExec::NetConnectionExec::ExecRegister");

    EventManager *manager = context->GetManager();
    auto conn = static_cast<NetConnection *>(manager->GetData());
    sptr<INetConnCallback> callback = conn->GetObserver();

    if (conn->hasNetSpecifier && conn->hasTimeout) {
        sptr<NetSpecifier> specifier = new NetSpecifier(conn->netSpecifier);
        int32_t ret =
            DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(specifier, callback, conn->timeout);
        NETMANAGER_BASE_LOGI("Register result hasNetSpecifier and hasTimeout %{public}d", ret);
        context->SetErrorCode(ret);
        return ret == 0;
    }

    if (conn->hasNetSpecifier) {
        sptr<NetSpecifier> specifier = new NetSpecifier(conn->netSpecifier);
        int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(specifier, callback, 0);
        NETMANAGER_BASE_LOGI("Register result hasNetSpecifier %{public}d", ret);
        context->SetErrorCode(ret);
        return ret == 0;
    }

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    NETMANAGER_BASE_LOGI("Register result %{public}d", ret);
    context->SetErrorCode(ret);
    return ret == 0;
}

napi_value ConnectionExec::NetConnectionExec::RegisterCallback(RegisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::NetConnectionExec::ExecUnregister(UnregisterContext *context)
{
    NETMANAGER_BASE_LOGI("ConnectionExec::NetConnectionExec::ExecUnregister");

    EventManager *manager = context->GetManager();
    auto conn = static_cast<NetConnection *>(manager->GetData());
    sptr<INetConnCallback> callback = conn->GetObserver();

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
    NETMANAGER_BASE_LOGI("Unregister result %{public}d", ret);
    context->SetErrorCode(ret);
    return ret == 0;
}

napi_value ConnectionExec::NetConnectionExec::UnregisterCallback(RegisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

void ConnectionExec::FillLinkAddress(napi_env env, napi_value connectionProperties, NetLinkInfo *linkInfo)
{
    NETMANAGER_BASE_LOGI("linkInfo->netAddrList_.size() = %{public}zu", linkInfo->netAddrList_.size());
    if (!linkInfo->netAddrList_.empty() && linkInfo->netAddrList_.size() <= MAX_ARRAY_LENGTH) {
        napi_value linkAddresses =
            NapiUtils::CreateArray(env, std::min(linkInfo->netAddrList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->netAddrList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->netAddrList_.end(); ++index, ++it) {
            napi_value netAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, it->address_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_FAMILY, it->family_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_PORT, it->port_);

            napi_value linkAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetNamedProperty(env, linkAddr, KEY_ADDRESS, netAddr);
            NapiUtils::SetUint32Property(env, linkAddr, KEY_PREFIX_LENGTH, it->prefixlen_);
            NapiUtils::SetArrayElement(env, linkAddresses, index, linkAddr);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_LINK_ADDRESSES, linkAddresses);
    }
}

void ConnectionExec::FillRouoteList(napi_env env, napi_value connectionProperties, NetLinkInfo *linkInfo)
{
    NETMANAGER_BASE_LOGI("linkInfo->routeList_.size() = %{public}zu", linkInfo->routeList_.size());
    if (!linkInfo->routeList_.empty() && linkInfo->routeList_.size() <= MAX_ARRAY_LENGTH) {
        napi_value routes = NapiUtils::CreateArray(env, std::min(linkInfo->routeList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->routeList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->routeList_.end(); ++index, ++it) {
            napi_value route = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, route, KEY_INTERFACE, it->iface_);

            napi_value dest = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, dest, KEY_ADDRESS, it->destination_.address_);
            NapiUtils::SetUint32Property(env, dest, KEY_PREFIX_LENGTH, it->destination_.prefixlen_);
            NapiUtils::SetNamedProperty(env, route, KEY_DESTINATION, dest);

            napi_value gateway = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, gateway, KEY_ADDRESS, it->gateway_.address_);
            NapiUtils::SetUint32Property(env, gateway, KEY_PREFIX_LENGTH, it->gateway_.prefixlen_);
            NapiUtils::SetNamedProperty(env, route, KEY_GATE_WAY, gateway);

            NapiUtils::SetBooleanProperty(env, route, KEY_HAS_GET_WAY, it->hasGateway_);
            NapiUtils::SetBooleanProperty(env, route, KEY_IS_DEFAULT_ROUE, it->isDefaultRoute_);

            NapiUtils::SetArrayElement(env, routes, index, route);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_ROUTES, routes);
    }
}

void ConnectionExec::FillDns(napi_env env, napi_value connectionProperties, NetLinkInfo *linkInfo)
{
    NETMANAGER_BASE_LOGI("linkInfo->dnsList_.size() = %{public}zu", linkInfo->dnsList_.size());
    if (!linkInfo->dnsList_.empty() && linkInfo->dnsList_.size() <= MAX_ARRAY_LENGTH) {
        napi_value dnsList = NapiUtils::CreateArray(env, std::min(linkInfo->dnsList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->dnsList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->dnsList_.end(); ++index, ++it) {
            napi_value netAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, it->address_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_FAMILY, it->family_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_PORT, it->port_);
            NapiUtils::SetArrayElement(env, dnsList, index, netAddr);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_DNSES, dnsList);
    }
}
} // namespace OHOS::NetManagerStandard
