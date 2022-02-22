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

#include "net_conn_callback_observer.h"
#include "connection_module.h"
#include "constant.h"
#include "netconnection.h"
#include "netmanager_base_log.h"

namespace OHOS::NetManagerStandard {
static constexpr const size_t MAX_ARRAY_LENGTH = 64;

int32_t NetConnCallbackObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetAvailable");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    netConnection->GetEventManager()->EmitByUv(EVENT_NET_AVAILABLE, netHandle.GetRefPtr(), NetAvailableCallback);
    return 0;
}

int32_t NetConnCallbackObserver::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
                                                       const sptr<NetAllCapabilities> &netAllCap)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetCapabilitiesChange");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    auto pair = new std::pair<NetHandle *, NetAllCapabilities *>;
    pair->first = netHandle.GetRefPtr();
    pair->second = netAllCap.GetRefPtr();
    netConnection->GetEventManager()->EmitByUv(EVENT_NET_CAPABILITIES_CHANGE, pair, NetCapabilitiesChangeCallback);
    return 0;
}

int32_t NetConnCallbackObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
                                                               const sptr<NetLinkInfo> &info)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetConnectionPropertiesChange");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    auto pair = new std::pair<NetHandle *, NetLinkInfo *>;
    pair->first = netHandle.GetRefPtr();
    pair->second = info.GetRefPtr();
    netConnection->GetEventManager()->EmitByUv(EVENT_NET_CONNECTION_PROPERTIES_CHANGE, pair,
                                               NetConnectionPropertiesChangeCallback);
    return 0;
}

int32_t NetConnCallbackObserver::NetLost(sptr<NetHandle> &netHandle)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetLost");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    netConnection->GetEventManager()->EmitByUv(EVENT_NET_LOST, netHandle.GetRefPtr(), NetLostCallback);
    return 0;
}

int32_t NetConnCallbackObserver::NetUnavailable()
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetUnavailable");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    netConnection->GetEventManager()->EmitByUv(EVENT_NET_UNAVAILABLE, nullptr, NetUnavailableCallback);
    return 0;
}

int32_t NetConnCallbackObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetBlockStatusChange");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    auto pair = new std::pair<NetHandle *, bool>;
    pair->first = netHandle.GetRefPtr();
    pair->second = blocked;
    netConnection->GetEventManager()->EmitByUv(EVENT_NET_BLOCK_STATUS_CHANGE, pair, NetBlockStatusChangeCallback);
    return 0;
}

napi_value NetConnCallbackObserver::CreateNetHandle(napi_env env, NetHandle *handle)
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
    };
    NapiUtils::DefineProperties(env, netHandle, properties);
    NapiUtils::SetUint32Property(env, netHandle, ConnectionModule::NetHandleInterface::PROPERTY_NET_ID,
                                 handle->GetNetId());
    return netHandle;
}

napi_value NetConnCallbackObserver::CreateNetCapabilities(napi_env env, NetAllCapabilities *capabilities)
{
    napi_value netCapabilities = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, netCapabilities) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    NapiUtils::SetUint32Property(env, netCapabilities, KEY_LINK_UP_BAND_WIDTH_KPS, capabilities->linkUpBandwidthKbps_);
    NapiUtils::SetUint32Property(env, netCapabilities, KEY_LINK_DOWN_BAND_WIDTH_KPS,
                                 capabilities->linkDownBandwidthKbps_);
    NETMANAGER_BASE_LOGI("capabilities->netCaps_.size() = %{public}zu", capabilities->netCaps_.size());
    if (!capabilities->netCaps_.empty()) {
        napi_value networkCap = NapiUtils::CreateArray(env, std::min(capabilities->netCaps_.size(), MAX_ARRAY_LENGTH));
        auto it = capabilities->netCaps_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != capabilities->netCaps_.end(); ++index, ++it) {
            NapiUtils::SetArrayElement(env, networkCap, index, NapiUtils::CreateUint32(env, *it));
        }
        NapiUtils::SetNamedProperty(env, netCapabilities, KEY_NETWORK_CAP, networkCap);
    }
    NETMANAGER_BASE_LOGI("capabilities->bearerTypes_.size() = %{public}zu", capabilities->bearerTypes_.size());
    if (!capabilities->bearerTypes_.empty()) {
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

napi_value NetConnCallbackObserver::CreateConnectionProperties(napi_env env, NetLinkInfo *linkInfo)
{
    napi_value connectionProperties = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, connectionProperties) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }
    NapiUtils::SetStringPropertyUtf8(env, connectionProperties, KEY_INTERFACE_NAME, linkInfo->ifaceName_);
    NapiUtils::SetStringPropertyUtf8(env, connectionProperties, KEY_DOMAINS, linkInfo->domain_);
    NapiUtils::SetUint32Property(env, connectionProperties, KEY_MTU, linkInfo->mtu_);
    NETMANAGER_BASE_LOGI("linkInfo->netAddrList_.size() = %{public}zu", linkInfo->netAddrList_.size());
    if (!linkInfo->netAddrList_.empty()) {
        napi_value linkAddresses =
            NapiUtils::CreateArray(env, std::min(linkInfo->netAddrList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->netAddrList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->netAddrList_.end(); ++index, ++it) {
            napi_value netAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, it->address_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_PREFIX_LENGTH, it->prefixlen_);
            NapiUtils::SetArrayElement(env, linkAddresses, index, netAddr);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_LINK_ADDRESSES, linkAddresses);
    }
    NETMANAGER_BASE_LOGI("linkInfo->routeList_.size() = %{public}zu", linkInfo->routeList_.size());
    if (!linkInfo->routeList_.empty()) {
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
    NETMANAGER_BASE_LOGI("linkInfo->dnsList_.size() = %{public}zu", linkInfo->dnsList_.size());
    if (!linkInfo->dnsList_.empty()) {
        napi_value dnsList = NapiUtils::CreateArray(env, std::min(linkInfo->dnsList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->dnsList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->dnsList_.end(); ++index, ++it) {
            napi_value netAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, it->address_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_FAMILY, it->family_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_PORT, it->port_);
            NapiUtils::SetArrayElement(env, dnsList, index, netAddr);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_LINK_ADDRESSES, dnsList);
    }
    return connectionProperties;
}

napi_value NetConnCallbackObserver::CreateNetAvailableParam(napi_env env, void *data)
{
    return CreateNetHandle(env, static_cast<NetHandle *>(data));
}

napi_value NetConnCallbackObserver::CreateNetCapabilitiesChangeParam(napi_env env, void *data)
{
    auto pair = static_cast<std::pair<NetHandle *, NetAllCapabilities *> *>(data);
    napi_value netHandle = CreateNetHandle(env, pair->first);
    napi_value capabilities = CreateNetCapabilities(env, pair->second);
    napi_value obj = NapiUtils::CreateObject(env);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_HANDLE, netHandle);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_CAP, capabilities);
    delete pair;
    return obj;
}

napi_value NetConnCallbackObserver::CreateNetConnectionPropertiesChangeParam(napi_env env, void *data)
{
    auto pair = static_cast<std::pair<NetHandle *, NetLinkInfo *> *>(data);
    napi_value netHandle = CreateNetHandle(env, pair->first);
    napi_value properties = CreateConnectionProperties(env, pair->second);
    napi_value obj = NapiUtils::CreateObject(env);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_HANDLE, netHandle);
    NapiUtils::SetNamedProperty(env, obj, KEY_CONNECTION_PROPERTIES, properties);
    delete pair;
    return obj;
}

napi_value NetConnCallbackObserver::CreateNetLostParam(napi_env env, void *data)
{
    return CreateNetHandle(env, static_cast<NetHandle *>(data));
}

napi_value NetConnCallbackObserver::CreateNetUnavailableParam(napi_env env, void *data)
{
    (void)data;

    return NapiUtils::GetUndefined(env);
}

napi_value NetConnCallbackObserver::CreateNetBlockStatusChangeParam(napi_env env, void *data)
{
    auto pair = static_cast<std::pair<NetHandle *, bool> *>(data);
    napi_value netHandle = CreateNetHandle(env, pair->first);
    napi_value obj = NapiUtils::CreateObject(env);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_HANDLE, netHandle);
    NapiUtils::SetBooleanProperty(env, obj, KEY_BLOCKED, pair->second);
    delete pair;
    return obj;
}

void NetConnCallbackObserver::NetAvailableCallback(uv_work_t *work, int status)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetAvailableCallback");

    CallbackTemplate<CreateNetAvailableParam>(work, status);
}

void NetConnCallbackObserver::NetCapabilitiesChangeCallback(uv_work_t *work, int status)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetCapabilitiesChangeCallback");

    CallbackTemplate<CreateNetCapabilitiesChangeParam>(work, status);
}

void NetConnCallbackObserver::NetConnectionPropertiesChangeCallback(uv_work_t *work, int status)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetConnectionPropertiesChangeCallback");

    CallbackTemplate<CreateNetConnectionPropertiesChangeParam>(work, status);
}

void NetConnCallbackObserver::NetLostCallback(uv_work_t *work, int status)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetLostCallback");

    CallbackTemplate<CreateNetLostParam>(work, status);
}

void NetConnCallbackObserver::NetUnavailableCallback(uv_work_t *work, int status)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetUnavailableCallback");

    CallbackTemplate<CreateNetUnavailableParam>(work, status);
}

void NetConnCallbackObserver::NetBlockStatusChangeCallback(uv_work_t *work, int status)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetBlockStatusChangeCallback");

    CallbackTemplate<CreateNetBlockStatusChangeParam>(work, status);
}
} // namespace OHOS::NetManagerStandard
