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
#include "napi_net_conn.h"

namespace OHOS {
namespace NetManagerStandard {
static void OnNetAvailableEvent(EventListener &listen, sptr<NetHandle> &netHandle)
{
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    info = NapiNetConn::CreateNetHandle(listen.env, netHandle);
    napi_get_undefined(listen.env, &recv);
    napi_get_reference_value(listen.env, listen.callbackRef, &callbackFunc);
    callbackValues[CALLBACK_ARGV_INDEX_1] = info;
    napi_call_function(listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

static void OnNetCapabilitiesChangeEvent(
    EventListener &listen, sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap)
{
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    napi_create_object(listen.env, &info);
    napi_value handle = NapiNetConn::CreateNetHandle(listen.env, netHandle);
    napi_set_named_property(listen.env, info, "handle", handle);
    std::string netAllCapStr = netAllCap->ToString(" ");
    NapiCommon::SetPropertyString(listen.env, info, "netAllCap", netAllCapStr);
    napi_get_undefined(listen.env, &recv);
    napi_get_reference_value(listen.env, listen.callbackRef, &callbackFunc);
    callbackValues[CALLBACK_ARGV_INDEX_1] = info;
    napi_call_function(listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

static void OnNetConnectionPropertiesChangeEvent(
    EventListener &listen, sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &linkInfo)
{
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    napi_create_object(listen.env, &info);
    napi_value handle = NapiNetConn::CreateNetHandle(listen.env, netHandle);
    napi_set_named_property(listen.env, info, "handle", handle);
    std::string linkInfoStr = linkInfo->ToString(" ");
    NapiCommon::SetPropertyString(listen.env, info, "linkInfo", linkInfoStr);
    napi_get_undefined(listen.env, &recv);
    napi_get_reference_value(listen.env, listen.callbackRef, &callbackFunc);
    callbackValues[CALLBACK_ARGV_INDEX_1] = info;
    napi_call_function(listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

static void OnNetLostEvent(EventListener &listen, sptr<NetHandle> &netHandle)
{
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    info = NapiNetConn::CreateNetHandle(listen.env, netHandle);
    napi_get_undefined(listen.env, &recv);
    napi_get_reference_value(listen.env, listen.callbackRef, &callbackFunc);
    callbackValues[CALLBACK_ARGV_INDEX_1] = info;
    napi_call_function(listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

static void OnNetUnavailableEvent(EventListener &listen)
{
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    info = NapiCommon::CreateUndefined(listen.env);
    napi_get_undefined(listen.env, &recv);
    napi_get_reference_value(listen.env, listen.callbackRef, &callbackFunc);
    callbackValues[CALLBACK_ARGV_INDEX_1] = info;
    napi_call_function(listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

static void OnNetBlockStatusChangeEvent(
    EventListener &listen, sptr<NetHandle> &netHandle, bool blocked)
{
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    napi_create_object(listen.env, &info);
    napi_value handle = NapiNetConn::CreateNetHandle(listen.env, netHandle);
    napi_set_named_property(listen.env, info, "handle", handle);
    napi_value isBlock = nullptr;
    napi_get_boolean(listen.env, blocked, &isBlock);
    napi_set_named_property(listen.env, info, "blocked", isBlock);
    napi_get_undefined(listen.env, &recv);
    napi_get_reference_value(listen.env, listen.callbackRef, &callbackFunc);
    callbackValues[CALLBACK_ARGV_INDEX_1] = info;
    napi_call_function(listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

int32_t NapiNetConnObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    NETMGR_LOG_I("NetAvailable netId [%{public}d]", netHandle->GetNetId());
    EventListener listen;
    listen.eventId = EVENT_NET_AVAILABLE_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnNetAvailableEvent(listen, netHandle);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetCapabilitiesChange(
    sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap)
{
    NETMGR_LOG_I("NetCapabilitiesChange netId [%{public}d]", netHandle->GetNetId());
    EventListener listen;
    listen.eventId = EVENT_NET_CAPABILITIES_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnNetCapabilitiesChangeEvent(listen, netHandle, netAllCap);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetLinkInfo> &info)
{
    NETMGR_LOG_I("NetConnectionPropertiesChange netId [%{public}d], info is [%{public}s]",
        netHandle->GetNetId(), info == nullptr ? "nullptr" : "not nullptr");
    EventListener listen;
    listen.eventId = EVENT_NET_CONNECTION_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnNetConnectionPropertiesChangeEvent(listen, netHandle, info);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetLost(sptr<NetHandle> &netHandle)
{
    NETMGR_LOG_I("NetLost netId [%{public}d]", netHandle->GetNetId());
    EventListener listen;
    listen.eventId = EVENT_NET_LOST_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnNetLostEvent(listen, netHandle);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetUnavailable()
{
    NETMGR_LOG_I("NetUnavailable");
    EventListener listen;
    listen.eventId = EVENT_NET_UNAVAILABLE_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnNetUnavailableEvent(listen);
        EventListenerContext::GetInstance().RemoveCallback(this);
    }
    return 0;
}

int32_t NapiNetConnObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    NETMGR_LOG_I("NapiNetConnObserver NetBlockStatusChange netId [%{public}d]", netHandle->GetNetId());
    EventListener listen;
    listen.eventId = EVENT_NET_BLOCK_STATUS_CHANGE;
    if (EventListenerContext::GetInstance().FindListener(this, listen) != EVENT_NET_UNKNOW_CHANGE) {
        OnNetBlockStatusChangeEvent(listen, netHandle, blocked);
    }
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
