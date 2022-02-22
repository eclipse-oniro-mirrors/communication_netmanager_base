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
    if (!capabilities->netCaps_.empty()) {
        napi_value networkCap = NapiUtils::CreateArray(env, capabilities->netCaps_.size());
        uint32_t index = 0;
        std::for_each(capabilities->netCaps_.begin(), capabilities->netCaps_.end(),
                      [env, &index, networkCap](NetCap cap) {
                          NapiUtils::SetArrayElement(env, networkCap, index, NapiUtils::CreateUint32(env, cap));
                          ++index;
                      });
        NapiUtils::SetNamedProperty(env, netCapabilities, KEY_NETWORK_CAP, networkCap);
    }
    if (!capabilities->bearerTypes_.empty()) {
        napi_value bearerTypes = NapiUtils::CreateArray(env, capabilities->bearerTypes_.size());
        uint32_t index = 0;
        std::for_each(capabilities->bearerTypes_.begin(), capabilities->bearerTypes_.end(),
                      [env, &index, bearerTypes](NetBearType bearType) {
                          NapiUtils::SetArrayElement(env, bearerTypes, index, NapiUtils::CreateUint32(env, bearType));
                          ++index;
                      });
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
    if (!linkInfo->netAddrList_.empty()) {
        napi_value linkAddresses = NapiUtils::CreateArray(env, linkInfo->netAddrList_.size());
        uint32_t index = 0;
        std::for_each(linkInfo->netAddrList_.begin(), linkInfo->netAddrList_.end(),
                      [env, &index, linkAddresses](const INetAddr &addr) {
                          napi_value netAddr = NapiUtils::CreateObject(env);
                          if (NapiUtils::GetValueType(env, netAddr) != napi_object) {
                              return;
                          }
                          NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, addr.address_);
                          NapiUtils::SetUint32Property(env, netAddr, KEY_PREFIX_LENGTH, addr.prefixlen_);
                          NapiUtils::SetArrayElement(env, linkAddresses, index, netAddr);
                          ++index;
                      });
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_LINK_ADDRESSES, linkAddresses);
    }
    if (!linkInfo->routeList_.empty()) {
        napi_value routes = NapiUtils::CreateArray(env, linkInfo->routeList_.size());
        uint32_t index = 0;
        std::for_each(linkInfo->routeList_.begin(), linkInfo->routeList_.end(), [env, &index, routes](const Route &rt) {
            napi_value route = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, route, KEY_INTERFACE, rt.iface_);

            napi_value dest = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, dest, KEY_ADDRESS, rt.destination_.address_);
            NapiUtils::SetUint32Property(env, dest, KEY_PREFIX_LENGTH, rt.destination_.prefixlen_);
            NapiUtils::SetNamedProperty(env, route, KEY_DESTINATION, dest);

            napi_value gateway = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, gateway, KEY_ADDRESS, rt.gateway_.address_);
            NapiUtils::SetUint32Property(env, gateway, KEY_PREFIX_LENGTH, rt.gateway_.prefixlen_);
            NapiUtils::SetNamedProperty(env, route, KEY_GATE_WAY, gateway);

            NapiUtils::SetBooleanProperty(env, route, KEY_HAS_GET_WAY, rt.hasGateway_);
            NapiUtils::SetBooleanProperty(env, route, KEY_IS_DEFAULT_ROUE, rt.isDefaultRoute_);

            NapiUtils::SetArrayElement(env, routes, index, route);
            ++index;
        });
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_ROUTES, routes);
    }
    if (!linkInfo->dnsList_.empty()) {
        napi_value dnses = NapiUtils::CreateArray(env, linkInfo->dnsList_.size());
        uint32_t index = 0;
        std::for_each(linkInfo->dnsList_.begin(), linkInfo->dnsList_.end(), [env, &index, dnses](const INetAddr &addr) {
            napi_value netAddr = NapiUtils::CreateObject(env);
            if (NapiUtils::GetValueType(env, netAddr) != napi_object) {
                return;
            }
            NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, addr.address_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_FAMILY, addr.family_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_PORT, addr.port_);
            NapiUtils::SetArrayElement(env, dnses, index, netAddr);
            ++index;
        });
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_LINK_ADDRESSES, dnses);
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
