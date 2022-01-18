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

#include "net_policy_event_listener_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "net_policy_client.h"
#include "napi_net_policy_observer.h"
#include "napi_net_policy.h"

namespace OHOS {
namespace NetManagerStandard {
static sptr<INetPolicyCallback> callback = (std::make_unique<NapiNetPolicyObserver>()).release();

NetPolicyEventListenerManager& NetPolicyEventListenerManager::GetInstance()
{
    NETMGR_LOG_D("NetPolicyEventListenerManager::GetInstance");
    static NetPolicyEventListenerManager instance;
    return instance;
}

NetPolicyEventListenerManager::NetPolicyEventListenerManager()
{
    NETMGR_LOG_D("NetPolicyEventListenerManager");
    if (eventStateRun_ == STATE_RUNNING) {
        NETMGR_LOG_D("eventListenerHandler is running");
        return;
    }
    eventLoop = AppExecFwk::EventRunner::Create("EventListenerHandler");
    if (eventLoop.get() == nullptr) {
        NETMGR_LOG_E("failed to create EventRunner");
        return;
    }
    eventListenerHandler = std::make_shared<NetPolicyEventListenerHandler>(eventLoop);
    if (eventListenerHandler == nullptr) {
        NETMGR_LOG_E("failed to create new eventListenerHandler");
        return;
    }
    eventLoop->Run();
    NETMGR_LOG_D("eventLoop  is running");
    eventStateRun_ = STATE_RUNNING;
}

int32_t NetPolicyEventListenerManager::AddEventListener(EventListener &eventListener)
{
    NETMGR_LOG_D("eventListener.eventId = [%{public}d]", eventListener.eventId);
    eventListenerHandler->AddEventListener(eventListener);
    return DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
}

int32_t NetPolicyEventListenerManager::RemoveEventListener(EventListener &eventListener)
{
    NETMGR_LOG_D("RemoveEventListener");
    eventListenerHandler->RemoveEventListener(eventListener);
    return DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
}
} // namespace NetManagerStandard
} // namespace OHOS
