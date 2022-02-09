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

#include "net_stats_event_listener_manager.h"
#include "iservice_registry.h"
#include "net_stats_callback.h"
#include "net_stats_client.h"

namespace OHOS {
namespace NetManagerStandard {
static sptr<INetStatsCallback> callback = (std::make_unique<NetStatsCallback>()).release();

NetStatsEventListenerManager &NetStatsEventListenerManager::GetInstance()
{
    static NetStatsEventListenerManager instance;
    return instance;
}

NetStatsEventListenerManager::NetStatsEventListenerManager() {}

int32_t NetStatsEventListenerManager::AddEventListener(EventListener &eventListener)
{
    listenerList.push_back(eventListener);
    return DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(callback);
}

int32_t NetStatsEventListenerManager::RemoveEventListener(EventListener &eventListener)
{
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        if (it->env != nullptr && it->callbackRef != nullptr) {
            napi_delete_reference(it->env, it->callbackRef);
        }
    }
    listenerList.erase(listenerList.begin(), listenerList.end());

    return DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(callback);
}

int32_t NetStatsEventListenerManager::FindListener(EventListener &listen)
{
    int32_t eventId = EVENT_NET_UNKNOW_CHANGE;
    for (auto it = listenerList.begin(); it != listenerList.end(); ++it) {
        NETMGR_LOG_I("NetStatsCallback it->eventId = %{public}d, callbackRef = %{public}d", it->eventId,
            it->callbackRef != nullptr);
        if (it->eventId == listen.eventId) {
            eventId = listen.eventId;
            listen.eventId = it->eventId;
            listen.env = it->env;
            listen.callbackRef = it->callbackRef;
        }
    }
    return eventId;
}
} // namespace NetManagerStandard
} // namespace OHOS
