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

#if HAS_TELEPHONY
#include "core_service_client.h"
#endif

#include "netconnection.h"
#include "netmanager_base_log.h"

static constexpr const char *NETWORK_NONE = "none";

static constexpr const char *NETWORK_WIFI = "WiFi";

namespace OHOS::NetManagerStandard {
struct NetworkType {
    std::set<NetBearType> bearerTypes;
};

#if HAS_TELEPHONY
static std::string CellularTypeToString(Telephony::SignalInformation::NetworkType type)
{
    switch (type) {
        case Telephony::SignalInformation::NetworkType::GSM:
            return "2g";
        case Telephony::SignalInformation::NetworkType::CDMA:
        case Telephony::SignalInformation::NetworkType::WCDMA:
        case Telephony::SignalInformation::NetworkType::TDSCDMA:
            return "3g";
        case Telephony::SignalInformation::NetworkType::LTE:
            return "4g";
        default:
            break;
    }
    return "5g";
}
#endif

static napi_value MakeNetworkResponse(napi_env env, void *data)
{
    auto deleter = [](NetworkType *t) { delete t; };
    std::unique_ptr<NetworkType, decltype(deleter)> netType(reinterpret_cast<NetworkType *>(data), deleter);

    napi_value obj = NapiUtils::CreateObject(env);
    if (netType->bearerTypes.find(BEARER_WIFI) != netType->bearerTypes.end()) {
        NapiUtils::SetStringPropertyUtf8(env, obj, KEY_TYPE, NETWORK_WIFI);
        NapiUtils::SetBooleanProperty(env, obj, KEY_METERED, false);
        return obj;
    }

#if HAS_TELEPHONY
    if (netType->bearerTypes.find(BEARER_CELLULAR) != netType->bearerTypes.end()) {
        auto vec = DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetSignalInfoList(0);
        if (vec.empty()) {
            NapiUtils::SetStringPropertyUtf8(env, obj, KEY_TYPE, NETWORK_NONE);
            NapiUtils::SetBooleanProperty(env, obj, KEY_METERED, false);
            return obj;
        }

        std::sort(vec.begin(), vec.end(),
                  [](const sptr<Telephony::SignalInformation> &info1, const sptr<Telephony::SignalInformation> &info2)
                      -> bool { return info1->GetSignalLevel() > info2->GetSignalLevel(); });
        NapiUtils::SetStringPropertyUtf8(env, obj, KEY_TYPE, CellularTypeToString(vec[0]->GetNetworkType()));
        NapiUtils::SetBooleanProperty(env, obj, KEY_METERED, true);
        return obj;
    }
#endif

    NapiUtils::SetStringPropertyUtf8(env, obj, KEY_TYPE, NETWORK_NONE);
    NapiUtils::SetBooleanProperty(env, obj, KEY_METERED, false);
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
    NetConnection *netConnection = NetConnection::GetNetConnection(this);
    if (netConnection == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_GET_TYPE)) {
        auto netType = new NetworkType;
        netType->bearerTypes = netAllCap->bearerTypes_;
        netConnection->GetEventManager()->EmitByUv(EVENT_GET_TYPE, netType, CallbackTemplate<MakeNetworkResponse>);
    } else {
        NETMANAGER_BASE_LOGI("NO EVENT_GET_TYPE");
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_SUBSCRIBE)) {
        auto netType = new NetworkType;
        netType->bearerTypes = netAllCap->bearerTypes_;
        netConnection->GetEventManager()->EmitByUv(EVENT_SUBSCRIBE, netType, CallbackTemplate<MakeNetworkResponse>);
    } else {
        NETMANAGER_BASE_LOGI("NO EVENT_SUBSCRIBE");
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
    NetConnection *netConnection = NetConnection::GetNetConnection(this);
    if (netConnection == nullptr) {
        NETMANAGER_BASE_LOGI("can not find netConnection handle");
        return 0;
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_GET_TYPE)) {
        auto netType = new NetworkType;
        netConnection->GetEventManager()->EmitByUv(EVENT_GET_TYPE, netType, CallbackTemplate<MakeNetworkResponse>);
    } else {
        NETMANAGER_BASE_LOGI("NO EVENT_GET_TYPE");
    }
    if (netConnection->GetEventManager()->HasEventListener(EVENT_SUBSCRIBE)) {
        auto netType = new NetworkType;
        netConnection->GetEventManager()->EmitByUv(EVENT_SUBSCRIBE, netType, CallbackTemplate<MakeNetworkResponse>);
    } else {
        NETMANAGER_BASE_LOGI("NO EVENT_SUBSCRIBE");
    }
    return 0;
}

int32_t NetConnCallbackObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    NETMANAGER_BASE_LOGI("NetConnCallbackObserver::NetBlockStatusChange");
    return 0;
}
} // namespace OHOS::NetManagerStandard
