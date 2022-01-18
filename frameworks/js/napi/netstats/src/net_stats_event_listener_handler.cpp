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

#include "net_stats_event_listener_handler.h"
#include "inner_event.h"
#include "net_mgr_log_wrapper.h"
#include "i_net_stats_callback.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsEventListenerHandler::NetStatsEventListenerHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    NETMGR_LOG_I("NetStatsEventListenerHandler");
}

void NetStatsEventListenerHandler::NetIfaceStatsUpdated(
    EventListener &eventListener, std::unique_ptr<IfaceStatsEvent> &event)
{
    NETMGR_LOG_I("event->iface = [%{public}s],  ", event->iface.c_str());
}

void NetStatsEventListenerHandler::NetUidStatsUpdated(
    EventListener &eventListener, std::unique_ptr<UidStatsEvent> &event)
{
    NETMGR_LOG_I("event->iface = [%{public}s],  event->uid = [%{public}d]", event->iface.c_str(), event->uid);
}

int32_t NetStatsEventListenerHandler::AddEventListener(EventListener &eventListener)
{
    listenerList.push_back(eventListener);
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        NETMGR_LOG_I("eventId = [%{public}d]", it->eventId);
    }
    NETMGR_LOG_I("listenerList.size = [%{public}zu]", listenerList.size());
    return 0;
}

int32_t NetStatsEventListenerHandler::RemoveEventListener(EventListener &eventListener)
{
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        if (it->env != nullptr && it->callbackRef != nullptr) {
            napi_delete_reference(it->env, it->callbackRef);
        }
    }
    listenerList.erase(listenerList.begin(), listenerList.end());
    NETMGR_LOG_I("listenerList.size = [%{puvlic}zu]", listenerList.size());
    return 0;
}

void NetStatsEventListenerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    int32_t eventId = event->GetInnerEventId();
    NETMGR_LOG_I("eventId = %{public}d", eventId);
    std::unique_ptr<IfaceStatsEvent> ifaceStatsEvent = nullptr;
    if (eventId == INetStatsCallback::NET_STATS_IFACE_CHANGED) {
        ifaceStatsEvent = event->GetUniqueObject<IfaceStatsEvent>();
    } else {
        NETMGR_LOG_E("UNKONOW_EVENT");
    }
    for (auto it = listenerList.begin(); it != listenerList.end(); ++it) {
        if (it->eventId == INetStatsCallback::NET_STATS_IFACE_CHANGED) {
            NetIfaceStatsUpdated(*it, ifaceStatsEvent);
        } else {
            NETMGR_LOG_E("listener.eventId = [%{public}d] unknown", eventId);
        }
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
