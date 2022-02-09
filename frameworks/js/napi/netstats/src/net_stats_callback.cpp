/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "net_stats_callback.h"
#include "i_net_stats_callback.h"
#include "net_stats_event_listener_manager.h"

namespace OHOS {
namespace NetManagerStandard {
void OnNetStatsChangeEvent(napi_env env, napi_ref callbackRef, napi_value callbackValue)
{
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    napi_get_undefined(env, &recv);
    napi_get_reference_value(env, callbackRef, &callbackFunc);
    napi_value callbackValues[] = {callbackValue};
    napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

int32_t NetStatsCallback::NetIfaceStatsChanged(const std::string &iface)
{
    NETMGR_LOG_I("NetStatsCallback NetIfaceStatsChanged iface = %{public}s", iface.c_str());
    EventListener listen;
    listen.eventId = EVENT_NET_STATS_CHANGE;
    if (NetStatsEventListenerManager::GetInstance().FindListener(listen) != EVENT_NET_UNKNOW_CHANGE) {
        napi_value callbackValue = nullptr;
        napi_create_object(listen.env, &callbackValue);
        NapiCommon::SetPropertyString(listen.env, callbackValue, "iface", iface);
        OnNetStatsChangeEvent(listen.env, listen.callbackRef, callbackValue);
    }
    return 0;
};

int32_t NetStatsCallback::NetUidStatsChanged(const std::string &iface, uint32_t uid)
{
    NETMGR_LOG_I("NetStatsCallback NetUidStatsChanged iface = %{public}s, uid = %{public}d", iface.c_str(), uid);
    EventListener listen;
    listen.eventId = EVENT_NET_STATS_CHANGE;
    if (NetStatsEventListenerManager::GetInstance().FindListener(listen) != EVENT_NET_UNKNOW_CHANGE) {
        napi_value callbackValue = nullptr;
        napi_create_object(listen.env, &callbackValue);
        NapiCommon::SetPropertyString(listen.env, callbackValue, "iface", iface);
        NapiCommon::SetPropertyInt32(listen.env, callbackValue, "uid", static_cast<int32_t>(uid));
        OnNetStatsChangeEvent(listen.env, listen.callbackRef, callbackValue);
    }
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS