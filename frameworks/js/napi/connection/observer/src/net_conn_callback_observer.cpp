/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "connection_exec.h"
#include "constant.h"
#include "netconnection.h"
#include "netmanager_base_log.h"

namespace OHOS::NetManagerStandard {
static constexpr const int NETAVAILABLE_NOLISTENER_FLAG = 0;
static constexpr const int NETUNAVAILABLE_NOLISTENER_FLAG = 1;
static constexpr const int CAPABILITIES_NOLISTENER_FLAG = 2;
static constexpr const int PROPERTIES_NOLISTENER_FLAG = 3;

int32_t NetConnCallbackObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        return 0;
    }
    std::shared_lock<std::shared_mutex> lock(g_netConnectionsMutex);
    auto iter = NET_CONNECTIONS.find(this);
    if (iter == NET_CONNECTIONS.end()) {
        NETMANAGER_BASE_LOGI("can not find netConnection key");
        return 0;
    }
    auto netConnection = *iter;
    lock.unlock();
    if (netConnection.second == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    auto manager = netConnection.second->GetEventManager();
    if (manager == nullptr) {
        return 0;
    }
    SetListenerState(EVENT_NET_UNAVAILABLE, false, nullptr, nullptr, nullptr);
    if (!manager->HasEventListener(EVENT_NET_AVAILABLE)) {
        NETMANAGER_BASE_LOGD("no %{public}s listener", EVENT_NET_AVAILABLE);
        SetListenerState(EVENT_NET_AVAILABLE, true, netHandle, nullptr, nullptr);
        return 0;
    }
    auto network = *netHandle;
    auto handler = [network, manager](napi_env env) {
        auto obj = CreateNetAvailableParam(env, const_cast<NetHandle &>(network));
        std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), obj};
        manager->Emit(EVENT_NET_AVAILABLE, arg);
    };
    manager->EmitByUvWithModuleId(EVENT_NET_AVAILABLE, handler, netConnection.second->moduleId_);
    return 0;
}

int32_t NetConnCallbackObserver::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
                                                       const sptr<NetAllCapabilities> &netAllCap)
{
    if (netHandle == nullptr || netAllCap == nullptr) {
        return 0;
    }
    std::shared_lock<std::shared_mutex> lock(g_netConnectionsMutex);
    auto iter = NET_CONNECTIONS.find(this);
    if (iter == NET_CONNECTIONS.end()) {
        NETMANAGER_BASE_LOGI("can not find netConnection key");
        return 0;
    }
    auto netConnection = *iter;
    lock.unlock();
    if (netConnection.second == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    auto manager = netConnection.second->GetEventManager();
    if (manager == nullptr) {
        return 0;
    }
    if (!manager->HasEventListener(EVENT_NET_CAPABILITIES_CHANGE)) {
        NETMANAGER_BASE_LOGD("no %{public}s listener", EVENT_NET_CAPABILITIES_CHANGE);
        SetListenerState(EVENT_NET_CAPABILITIES_CHANGE, true, netHandle, nullptr, netAllCap);
        return 0;
    }
    auto network = *netHandle;
    auto caps = *netAllCap;
    auto handler = [network, caps, manager](napi_env env) {
        auto obj = CreateNetCapabilitiesChangeParam(env, const_cast<NetHandle &>(network),
                                                    const_cast<NetAllCapabilities &>(caps));
        std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), obj};
        manager->Emit(EVENT_NET_CAPABILITIES_CHANGE, arg);
    };
    manager->EmitByUvWithModuleId(EVENT_NET_CAPABILITIES_CHANGE, handler, netConnection.second->moduleId_);
    return 0;
}

int32_t NetConnCallbackObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
                                                               const sptr<NetLinkInfo> &info)
{
    if (netHandle == nullptr || info == nullptr) {
        return 0;
    }
    std::shared_lock<std::shared_mutex> lock(g_netConnectionsMutex);
    auto iter = NET_CONNECTIONS.find(this);
    if (iter == NET_CONNECTIONS.end()) {
        NETMANAGER_BASE_LOGI("can not find netConnection key");
        return 0;
    }
    auto netConnection = *iter;
    lock.unlock();
    if (netConnection.second == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    auto manager = netConnection.second->GetEventManager();
    if (manager == nullptr) {
        return 0;
    }
    if (!manager->HasEventListener(EVENT_NET_CONNECTION_PROPERTIES_CHANGE)) {
        NETMANAGER_BASE_LOGD("no %{public}s listener", EVENT_NET_CONNECTION_PROPERTIES_CHANGE);
        SetListenerState(EVENT_NET_CONNECTION_PROPERTIES_CHANGE, true, netHandle, info, nullptr);
        return 0;
    }
    auto network = *netHandle;
    auto linkInfo = *info;
    auto handler = [network, linkInfo, manager](napi_env env) {
        auto obj = CreateNetConnectionPropertiesChangeParam(env, const_cast<NetHandle &>(network),
                                                            const_cast<NetLinkInfo &>(linkInfo));
        std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), obj};
        manager->Emit(EVENT_NET_CONNECTION_PROPERTIES_CHANGE, arg);
    };
    manager->EmitByUvWithModuleId(EVENT_NET_CONNECTION_PROPERTIES_CHANGE, handler, netConnection.second->moduleId_);
    return 0;
}

int32_t NetConnCallbackObserver::NetLost(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        return 0;
    }
    std::shared_lock<std::shared_mutex> lock(g_netConnectionsMutex);
    auto iter = NET_CONNECTIONS.find(this);
    if (iter == NET_CONNECTIONS.end()) {
        NETMANAGER_BASE_LOGI("can not find netConnection key");
        return 0;
    }
    auto netConnection = *iter;
    lock.unlock();
    if (netConnection.second == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    auto manager = netConnection.second->GetEventManager();
    if (manager == nullptr) {
        return 0;
    }
    if (listenState_.propertyState.first && netHandle->GetNetId() == listenState_.propertyState.first->GetNetId()) {
        SetListenerState(EVENT_NET_CONNECTION_PROPERTIES_CHANGE, false, nullptr, nullptr, nullptr);
    }
    if (listenState_.capState.first && netHandle->GetNetId() == listenState_.capState.first->GetNetId()) {
        SetListenerState(EVENT_NET_CAPABILITIES_CHANGE, false, nullptr, nullptr, nullptr);
    }
    if (listenState_.availState && netHandle->GetNetId() == listenState_.availState->GetNetId()) {
        SetListenerState(EVENT_NET_AVAILABLE, false, nullptr, nullptr, nullptr);
    }
    if (!manager->HasEventListener(EVENT_NET_LOST)) {
        NETMANAGER_BASE_LOGI("no event listener find %{public}s", EVENT_NET_LOST);
        return 0;
    }
    auto network = *netHandle;
    auto handler = [network, manager](napi_env env) {
        auto obj = CreateNetLostParam(env, const_cast<NetHandle &>(network));
        std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), obj};
        manager->Emit(EVENT_NET_LOST, arg);
    };
    manager->EmitByUvWithModuleId(EVENT_NET_LOST, handler, netConnection.second->moduleId_);
    return 0;
}

int32_t NetConnCallbackObserver::NetUnavailable()
{
    std::shared_lock<std::shared_mutex> lock(g_netConnectionsMutex);
    auto iter = NET_CONNECTIONS.find(this);
    if (iter == NET_CONNECTIONS.end()) {
        NETMANAGER_BASE_LOGI("can not find netConnection key");
        return 0;
    }
    auto netConnection = *iter;
    lock.unlock();
    if (netConnection.second == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    auto manager = netConnection.second->GetEventManager();
    if (manager == nullptr) {
        return 0;
    }
    SetListenerState(EVENT_NET_CONNECTION_PROPERTIES_CHANGE, false, nullptr, nullptr, nullptr);
    SetListenerState(EVENT_NET_CAPABILITIES_CHANGE, false, nullptr, nullptr, nullptr);
    SetListenerState(EVENT_NET_AVAILABLE, false, nullptr, nullptr, nullptr);
    if (!manager->HasEventListener(EVENT_NET_UNAVAILABLE)) {
        NETMANAGER_BASE_LOGD("no event listener find %{public}s", EVENT_NET_UNAVAILABLE);
        SetListenerState(EVENT_NET_UNAVAILABLE, true, nullptr, nullptr, nullptr);
        return 0;
    }
    auto handler = [manager](napi_env env) {
        auto obj = CreateNetUnavailableParam(env);
        std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), obj};
        manager->Emit(EVENT_NET_UNAVAILABLE, arg);
    };
    manager->EmitByUvWithModuleId(EVENT_NET_UNAVAILABLE, handler, netConnection.second->moduleId_);
    return 0;
}

int32_t NetConnCallbackObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    if (netHandle == nullptr) {
        return 0;
    }
    std::shared_lock<std::shared_mutex> lock(g_netConnectionsMutex);
    auto iter = NET_CONNECTIONS.find(this);
    if (iter == NET_CONNECTIONS.end()) {
        NETMANAGER_BASE_LOGI("can not find netConnection key");
        return 0;
    }
    auto netConnection = *iter;
    lock.unlock();
    if (netConnection.second == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    auto manager = netConnection.second->GetEventManager();
    if (manager == nullptr) {
        return 0;
    }
    if (!manager->HasEventListener(EVENT_NET_BLOCK_STATUS_CHANGE)) {
        NETMANAGER_BASE_LOGI("no event listener find %{public}s", EVENT_NET_BLOCK_STATUS_CHANGE);
        return 0;
    }
    auto network = *netHandle;
    auto handler = [network, blocked, manager](napi_env env) {
        auto obj = CreateNetBlockStatusChangeParam(env, const_cast<NetHandle &>(network), blocked);
        std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), obj};
        manager->Emit(EVENT_NET_BLOCK_STATUS_CHANGE, arg);
    };
    manager->EmitByUvWithModuleId(EVENT_NET_BLOCK_STATUS_CHANGE, handler, netConnection.second->moduleId_);
    return 0;
}

napi_value NetConnCallbackObserver::CreateNetHandle(napi_env env, NetHandle &handle)
{
    napi_value netHandle = ConnectionExec::CreateNetHandle(env, &handle);
    return netHandle;
}

napi_value NetConnCallbackObserver::CreateNetCapabilities(napi_env env, NetAllCapabilities &capabilities)
{
    napi_value netCapabilities = ConnectionExec::CreateNetCapabilities(env, &capabilities);
    return netCapabilities;
}

napi_value NetConnCallbackObserver::CreateConnectionProperties(napi_env env, NetLinkInfo &linkInfo)
{
    napi_value connectionProperties = ConnectionExec::CreateConnectionProperties(env, &linkInfo);
    return connectionProperties;
}

napi_value NetConnCallbackObserver::CreateNetAvailableParam(napi_env env, NetHandle &netHandle)
{
    return CreateNetHandle(env, netHandle);
}

napi_value NetConnCallbackObserver::CreateNetCapabilitiesChangeParam(napi_env env, NetHandle &handle,
                                                                     NetAllCapabilities &caps)
{
    napi_value netHandle = CreateNetHandle(env, handle);
    napi_value capabilities = CreateNetCapabilities(env, caps);
    napi_value obj = NapiUtils::CreateObject(env);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_HANDLE, netHandle);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_CAP, capabilities);
    return obj;
}

napi_value NetConnCallbackObserver::CreateNetConnectionPropertiesChangeParam(napi_env env, NetHandle &handle,
                                                                             NetLinkInfo &linkInfo)
{
    napi_value netHandle = CreateNetHandle(env, handle);
    napi_value properties = CreateConnectionProperties(env, linkInfo);
    napi_value obj = NapiUtils::CreateObject(env);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_HANDLE, netHandle);
    NapiUtils::SetNamedProperty(env, obj, KEY_CONNECTION_PROPERTIES, properties);
    return obj;
}

napi_value NetConnCallbackObserver::CreateNetLostParam(napi_env env, NetHandle &netHandle)
{
    return CreateNetHandle(env, netHandle);
}

napi_value NetConnCallbackObserver::CreateNetUnavailableParam(napi_env env)
{
    return NapiUtils::GetUndefined(env);
}

napi_value NetConnCallbackObserver::CreateNetBlockStatusChangeParam(napi_env env, NetHandle &handle, bool blocked)
{
    napi_value netHandle = CreateNetHandle(env, handle);
    napi_value obj = NapiUtils::CreateObject(env);
    NapiUtils::SetNamedProperty(env, obj, KEY_NET_HANDLE, netHandle);
    NapiUtils::SetBooleanProperty(env, obj, KEY_BLOCKED, blocked);
    return obj;
}

void NetConnCallbackObserver::OnAddListener(const std::string &type)
{
    if (type == EVENT_NET_AVAILABLE) {
        if (listenState_.callbackFlag.test(NETAVAILABLE_NOLISTENER_FLAG)) {
            listenState_.callbackFlag.set(NETAVAILABLE_NOLISTENER_FLAG, false);
            NETMANAGER_BASE_LOGI("re-callback netAvailable, netHandle: %{public}d",
                listenState_.availState != nullptr ? listenState_.availState->GetNetId() : 0);
            NetAvailable(listenState_.availState);
            return;
        }
    } else if (type == EVENT_NET_UNAVAILABLE) {
        if (listenState_.callbackFlag.test(NETUNAVAILABLE_NOLISTENER_FLAG)) {
            listenState_.callbackFlag.set(NETUNAVAILABLE_NOLISTENER_FLAG, false);
            NETMANAGER_BASE_LOGI("re-callback netUnavailable");
            NetUnavailable();
            return;
        }
    } else if (type == EVENT_NET_CAPABILITIES_CHANGE) {
        if (listenState_.callbackFlag.test(CAPABILITIES_NOLISTENER_FLAG)) {
            listenState_.callbackFlag.set(CAPABILITIES_NOLISTENER_FLAG, false);
            NETMANAGER_BASE_LOGI("re-callback netCapabilitiesChange, netHandle: %{public}d",
                listenState_.capState.first != nullptr ? listenState_.capState.first->GetNetId() : 0);
            NetCapabilitiesChange(listenState_.capState.first, listenState_.capState.second);
            return;
        }
    } else if (type == EVENT_NET_CONNECTION_PROPERTIES_CHANGE) {
        if (listenState_.callbackFlag.test(PROPERTIES_NOLISTENER_FLAG)) {
            listenState_.callbackFlag.set(PROPERTIES_NOLISTENER_FLAG, false);
            NETMANAGER_BASE_LOGI("re-callback netConnectionPropertiesChange, netHandle: %{public}d",
                listenState_.propertyState.first != nullptr ? listenState_.propertyState.first->GetNetId() : 0);
            NetConnectionPropertiesChange(listenState_.propertyState.first,
                listenState_.propertyState.second);
            return;
        }
    }
}

void NetConnCallbackObserver::SetListenerState(const std::string &event, const bool &flag,
    const sptr<NetHandle> &network, const sptr<NetLinkInfo> &info, const sptr<NetAllCapabilities> &netCap)
{
    if (event == EVENT_NET_AVAILABLE) {
        listenState_.callbackFlag.set(NETAVAILABLE_NOLISTENER_FLAG, flag);
        if (network != nullptr) {
            listenState_.availState = network;
            NETMANAGER_BASE_LOGI("set netAvailable callback state %{public}d,"
                "netHandle: %{public}d", flag, network->GetNetId());
        } else {
            listenState_.availState = nullptr;
        }
    } else if (event == EVENT_NET_UNAVAILABLE) {
        listenState_.callbackFlag.set(NETUNAVAILABLE_NOLISTENER_FLAG, flag);
        NETMANAGER_BASE_LOGD("set netUnavailable callback state %{public}d", flag);
    } else if (event == EVENT_NET_CAPABILITIES_CHANGE) {
        listenState_.callbackFlag.set(CAPABILITIES_NOLISTENER_FLAG, flag);
        if (netCap != nullptr && network != nullptr) {
            listenState_.capState = std::make_pair(network, netCap);
            NETMANAGER_BASE_LOGD("set netCapabilitiesChange callback state %{public}d"
                ", netHandle: %{public}d", flag, network->GetNetId());
        } else {
            listenState_.capState = {nullptr, nullptr};
        }
    } else if (event == EVENT_NET_CONNECTION_PROPERTIES_CHANGE) {
        listenState_.callbackFlag.set(PROPERTIES_NOLISTENER_FLAG, flag);
        if (info != nullptr && network != nullptr) {
            NETMANAGER_BASE_LOGD("set netConnectionPropertiesChange callback state %{public}d"
                ", netHandle: %{public}d", flag, network->GetNetId());
            listenState_.propertyState = std::make_pair(network, info);
        } else {
            listenState_.propertyState = {nullptr, nullptr};
        }
    }
}
} // namespace OHOS::NetManagerStandard
