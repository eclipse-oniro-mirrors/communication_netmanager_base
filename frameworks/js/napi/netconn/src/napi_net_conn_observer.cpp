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

#include "napi_net_conn_observer.h"
#include "net_mgr_log_wrapper.h"
#include "napi_net_conn.h"
#include "napi_common.h"
#include "event_listener_context.h"

namespace OHOS {
namespace NetManagerStandard {
static void OnEvent(EventListener & listen)
{
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    napi_create_int32(listen.env, listen.eventId, &info);
    napi_get_undefined(listen.env, &recv);
    napi_get_reference_value(listen.env, listen.callbackRef, &callbackFunc);
    callbackValues[CALLBACK_ARGV_INDEX_1] = info;
    napi_call_function(listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

int32_t NapiNetConnObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    NETMGR_LOG_D("NetAvailable netId [%{public}d]", netHandle->GetNetId());
    std::unique_ptr<int32_t> id = std::make_unique<int32_t>(netHandle->GetNetId());
    EventListener listen;
    listen.eventId = EVENT_NET_AVAILABLE_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnEvent(listen);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetCapabilitiesChange(
    sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap)
{
    NETMGR_LOG_D("NetCapabilitiesChange netId [%{public}d]", netHandle->GetNetId());
    std::unique_ptr<CapabilitiesEvent> capabilities = std::make_unique<CapabilitiesEvent>(netHandle->GetNetId(),
        netAllCap);
    EventListener listen;
    listen.eventId = EVENT_NET_CAPABILITIES_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnEvent(listen);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetLinkInfo> &info)
{
    NETMGR_LOG_D("NetConnectionPropertiesChange netId [%{public}d], info is [%{public}s]",
        netHandle->GetNetId(), info == nullptr ? "nullptr" : "not nullptr");
    std::unique_ptr<ConnectionEvent> connection = std::make_unique<ConnectionEvent>(netHandle->GetNetId(), info);
    EventListener listen;
    listen.eventId = EVENT_NET_CONNECTION_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnEvent(listen);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetLost(sptr<NetHandle> &netHandle)
{
    NETMGR_LOG_D("NetLost netId [%{public}d]", netHandle->GetNetId());
    std::unique_ptr<int32_t> id = std::make_unique<int32_t>(netHandle->GetNetId());
    EventListener listen;
    listen.eventId = EVENT_NET_LOST_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnEvent(listen);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetUnavailable()
{
    EventListener listen;
    listen.eventId = EVENT_NET_UNAVAILABLE_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnEvent(listen);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    NETMGR_LOG_D("NapiNetConnObserver NetBlockStatusChange netId [%{public}d]", netHandle->GetNetId());
    EventListener listen;
    listen.eventId = EVENT_NET_BLOCK_STATUS_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnEvent(listen);
    }
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
