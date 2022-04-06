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
#include "constant.h"
#include "netconnection.h"
#include "netmanager_base_log.h"

namespace OHOS::NetManagerStandard {
struct NetworkType {
    std::set<NetBearType> bearerTypes;
};

static napi_value MakeNetworkResponse(napi_env env, void *data)
{
    auto netType = reinterpret_cast<NetworkType *>(data);
    napi_value obj = NapiUtils::CreateObject(env);
    if (netType->bearerTypes.contains(BEARER_WIFI)) {
        NapiUtils::SetStringPropertyUtf8(env, obj, KEY_TYPE, "WiFi");
        NapiUtils::SetBooleanProperty(env, obj, KEY_METERED, false);
    } else if (netType->bearerTypes.contains(BEARER_CELLULAR)) {
        NapiUtils::SetStringPropertyUtf8(env, obj, KEY_TYPE, "cellular");
        NapiUtils::SetBooleanProperty(env, obj, KEY_METERED, true);
    } else {
        NapiUtils::SetStringPropertyUtf8(env, obj, KEY_TYPE, "none");
        NapiUtils::SetBooleanProperty(env, obj, KEY_METERED, false);
    }
    delete netType;
    return obj;
}

int32_t NetConnCallbackObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetAvailable");
    return 0;
}

int32_t NetConnCallbackObserver::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
                                                       const sptr<NetAllCapabilities> &netAllCap)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetCapabilitiesChange");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    if (netConnection == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_GET_TYPE)) {
        auto netType = new NetworkType;
        netType->bearerTypes = netAllCap->bearerTypes_;
        netConnection->GetEventManager()->EmitByUv(EVENT_GET_TYPE, netType, CallbackTemplate<MakeNetworkResponse>);
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_SUBSCRIBE)) {
        auto netType = new NetworkType;
        netType->bearerTypes = netAllCap->bearerTypes_;
        netConnection->GetEventManager()->EmitByUv(EVENT_SUBSCRIBE, netType, CallbackTemplate<MakeNetworkResponse>);
    }
    return 0;
}

int32_t NetConnCallbackObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
                                                               const sptr<NetLinkInfo> &info)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetConnectionPropertiesChange");
    return 0;
}

int32_t NetConnCallbackObserver::NetLost(sptr<NetHandle> &netHandle)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetLost");
    return 0;
}

int32_t NetConnCallbackObserver::NetUnavailable()
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetUnavailable");
    NetConnection *netConnection = NET_CONNECTIONS[this];
    if (netConnection == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_GET_TYPE)) {
        auto netType = new NetworkType;
        netConnection->GetEventManager()->EmitByUv(EVENT_GET_TYPE, netType, CallbackTemplate<MakeNetworkResponse>);
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_SUBSCRIBE)) {
        auto netType = new NetworkType;
        netConnection->GetEventManager()->EmitByUv(EVENT_SUBSCRIBE, netType, CallbackTemplate<MakeNetworkResponse>);
    }
    return 0;
}

int32_t NetConnCallbackObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetBlockStatusChange");
    return 0;
}
} // namespace OHOS::NetManagerStandard
