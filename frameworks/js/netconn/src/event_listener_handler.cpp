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

#include "event_listener_handler.h"
#include "inner_event.h"
#include "net_mgr_log_wrapper.h"
#include "net_conn_callback_info.h"

namespace OHOS {
namespace NetManagerStandard {
EventListenerHandler::EventListenerHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    NETMGR_LOG_D("EventListenerHandler");
}

int32_t EventListenerHandler::AddEventListener(EventListener &eventListener)
{
    listenerList.push_back(eventListener);
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        NETMGR_LOG_D("event = [%{public}s]", it->event.c_str());
    }
    NETMGR_LOG_D("listenerList.size = [%{public}d]", static_cast<int32_t>(listenerList.size()));
    return 0;
}

int32_t EventListenerHandler::RemoveEventListener()
{
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        if (it->env != nullptr && it->callbackRef != nullptr) {
            napi_delete_reference(it->env, it->callbackRef);
        }
    }
    listenerList.erase(listenerList.begin(), listenerList.end());
    NETMGR_LOG_D("listenerList.size = [%{puvlic}d]", static_cast<int32_t>(listenerList.size()));
    return 0;
}

void EventListenerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    NetConnEvent ev;
    int eventId = event->GetInnerEventId();
    NETMGR_LOG_D("eventId = %{public}d", eventId);
    std::unique_ptr<NetConnCallbackInfo> info = event->GetUniqueObject<NetConnCallbackInfo>();
    if (info == nullptr) {
        NETMGR_LOG_E("update info nullptr");
        return;
    }
    NETMGR_LOG_D("EventListenerHandler::ProcessEvent() netState_ = [%{public}d], netType_ = [%{public}d]",
        info->netState_, info->netType_);
    ev.netState = info->netState_;
    ev.netType = info->netType_;
    for (auto it = listenerList.begin(); it != listenerList.end(); ++it) {
        EventContext context;
        context.listen.event = it->event;
        context.listen.env = it->env;
        context.listen.callbackRef = it->callbackRef;
        context.ev.netState = ev.netState;
        context.ev.netType = ev.netType;
        NETMGR_LOG_D("netState = [%{public}d], netType = [%{public}d]", context.ev.netState, context.ev.netType);
        NetConnStateUpdated(context);
    }
}

void EventListenerHandler::NetConnStateUpdated(EventContext &context)
{
    NETMGR_LOG_D("NetConnStateUpdated uv_work_t start");
    int32_t netState = context.ev.netState;
    int32_t netType = context.ev.netType;
    NETMGR_LOG_D("netState = [%{public}d], netType = [%{public}d]", netState, netType);
    napi_value info = nullptr;
    napi_create_object(context.listen.env, &info);
    NapiCommon::SetPropertyInt32(context.listen.env, info, "netState", netState);
    NapiCommon::SetPropertyInt32(context.listen.env, info, "netType", netType);
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    if (context.listen.env == nullptr) {
        NETMGR_LOG_E("context.listen.env == null");
    }
    if (context.listen.callbackRef == nullptr) {
        NETMGR_LOG_E("context.listen.callbackRef == null");
    }
    napi_get_undefined(context.listen.env, &recv);
    napi_get_reference_value(context.listen.env, context.listen.callbackRef, &callbackFunc);
    callbackValues[ARGV_INDEX_1] = info;
    napi_call_function(context.listen.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
    NETMGR_LOG_D("NetConnStateUpdated end");
}
} // namespace NetManagerStandard
} // namespace OHOS
