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

#include "net_policy_event_listener_handler.h"
#include "inner_event.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyEventListenerHandler::NetPolicyEventListenerHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    NETMGR_LOG_D("NetPolicyEventListenerHandler");
}

int32_t NetPolicyEventListenerHandler::AddEventListener(EventListener &eventListener)
{
    listenerList.push_back(eventListener);
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        NETMGR_LOG_D("eventId = [%{public}d]", it->eventId);
    }
    NETMGR_LOG_D("listenerList.size = [%{public}zd]", listenerList.size());
    return 0;
}

int32_t NetPolicyEventListenerHandler::RemoveEventListener(EventListener &eventListener)
{
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        if (it->env != nullptr && it->callbackRef != nullptr) {
            napi_delete_reference(it->env, it->callbackRef);
        }
    }
    listenerList.erase(listenerList.begin(), listenerList.end());
    NETMGR_LOG_D("listenerList.size = [%{puvlic}zd]", listenerList.size());
    return 0;
}

void NetPolicyEventListenerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    int32_t eventId = event->GetInnerEventId();
    NETMGR_LOG_D("eventId = %{public}d", eventId);
    std::unique_ptr<UidPolicyEvent> uidPolicyEvent = nullptr;
    if (eventId == EVENT_NET_UID_POLICY_CHANGE) {
        uidPolicyEvent = event->GetUniqueObject<UidPolicyEvent>();
    } else {
        NETMGR_LOG_E("UNKONOW_EVENT");
    }
    for (auto it = listenerList.begin(); it != listenerList.end(); ++it) {
        if (it->eventId == EVENT_NET_UID_POLICY_CHANGE) {
            NetUidPolicyUpdated(*it, uidPolicyEvent);
        } else {
            NETMGR_LOG_E("listener.eventId = [%{public}d] unknown", eventId);
        }
    }
}

void NetPolicyEventListenerHandler::NetUidPolicyUpdated(EventListener &eventListener,
    std::unique_ptr<UidPolicyEvent> &event)
{
    NETMGR_LOG_I("NetUidPolicyUpdated, event->uid = [%{public}d], event->policy = [%{public}u]", event->uid,
        event->policy);
    napi_value info = nullptr;
    napi_create_object(eventListener.env, &info);
    NapiCommon::SetPropertyInt32(eventListener.env, info, "uid", event->uid);
    NapiCommon::SetPropertyInt32(eventListener.env, info, "policy", static_cast<int32_t>(event->policy));
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    if ((eventListener.env == nullptr) || (eventListener.callbackRef == nullptr)) {
        NETMGR_LOG_E("eventListener.env = nullptr || eventListener.callbackRef =nullptr");
        return;
    }
    napi_get_undefined(eventListener.env, &recv);
    napi_get_reference_value(eventListener.env, eventListener.callbackRef, &callbackFunc);
    callbackValues[ARGV_INDEX_1] = info;
    napi_call_function(eventListener.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

void NetPolicyEventListenerHandler::NetCellularPolicyUpdated(EventListener &eventListener,
    std::unique_ptr<CellularPolicyEvent> &event)
{
    NETMGR_LOG_I("NetCellularPolicyUpdated cellularPolicys.size= [%{public}zd]", event->cellularPolicys.size());
}

void NetPolicyEventListenerHandler::NetStrategySwitchUpdated(EventListener &eventListener,
    std::unique_ptr<StrategySwitchEvent> &event)
{
    NETMGR_LOG_I("NetStrategySwitchUpdated, event->slotId = [%{public}d], event->enable = [%{public}d]",
        event->slotId, event->enable);
}
} // namespace NetManagerStandard
} // namespace OHOS
