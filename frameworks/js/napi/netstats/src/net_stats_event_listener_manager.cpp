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
#include "net_stats_event_listener_handler.h"
#include "net_stats_client.h"

namespace OHOS {
namespace NetManagerStandard {
static sptr<INetStatsCallback> callback = (std::make_unique<NetStatsCallback>()).release();

NetStatsEventListenerManager& NetStatsEventListenerManager::GetInstance()
{
    static NetStatsEventListenerManager instance;
    return instance;
}

NetStatsEventListenerManager::NetStatsEventListenerManager()
{
    if (eventStateRun_ == STATE_RUNNING) {
        return;
    }
    eventLoop = AppExecFwk::EventRunner::Create("EventListenerHandler");
    if (eventLoop.get() == nullptr) {
        return;
    }
    eventListenerHandler = std::make_shared<NetStatsEventListenerHandler>(eventLoop);
    if (eventListenerHandler == nullptr) {
        return;
    }
    eventLoop->Run();
    eventStateRun_ = STATE_RUNNING;
}

int32_t NetStatsEventListenerManager::AddEventListener(EventListener &eventListener)
{
    eventListenerHandler->AddEventListener(eventListener);
    return DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(callback);
}

int32_t NetStatsEventListenerManager::RemoveEventListener(EventListener &eventListener)
{
    eventListenerHandler->RemoveEventListener(eventListener);
    return DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(callback);
}
} // namespace NetManagerStandard
} // namespace OHOS
