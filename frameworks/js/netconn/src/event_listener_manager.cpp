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

#include "event_listener_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "i_net_conn_service.h"
#include "i_net_conn_callback.h"
#include "net_activate.h"
#include "net_conn_client.h"
#include "net_specifier.h"
#include "napi_net_conn_observer.h"

namespace OHOS {
namespace NetManagerStandard {
static sptr<INetConnCallback> callback = std::make_unique<NapiNetConnObserver>().release();

EventListenerManager& EventListenerManager::GetInstance()
{
    NETMGR_LOG_D("EventListenerManager::GetInstance");
    static EventListenerManager instance;
    return instance;
}

EventListenerManager::EventListenerManager()
{
    NETMGR_LOG_D("EventListenerManager");
    if (eventStateRun_ == STATE_RUNNING) {
        NETMGR_LOG_D("eventListenerHandler is running");
        return;
    }
    eventLoop = AppExecFwk::EventRunner::Create("EventListenerHandler");
    if (eventLoop.get() == nullptr) {
        NETMGR_LOG_E("failed to create EventRunner");
        return;
    }
    eventListenerHandler = std::make_shared<EventListenerHandler>(eventLoop);
    if (eventListenerHandler == nullptr) {
        NETMGR_LOG_E("failed to create new eventListenerHandler");
        return;
    }
    eventLoop->Run();
    NETMGR_LOG_D("eventLoop  is running");
    eventStateRun_ = STATE_RUNNING;
}

int32_t EventListenerManager::AddEventListener(EventListener &eventListener)
{
    int32_t result = 0;
    NETMGR_LOG_D("eventListener.identifier = [%{public}s]", eventListener.identifier.c_str());
    NETMGR_LOG_D("eventListener.netType = [%{public}d]", eventListener.netType);
    NETMGR_LOG_D("eventListener.netCapabilities = [%{public}d]", eventListener.netCapabilities);
    eventListenerHandler->AddEventListener(eventListener);
    if (!eventListener.identifier.empty()) {
        sptr<NetSpecifier> netSpecifier = (std::make_unique<NetSpecifier>()).release();
        netSpecifier->ident_ = eventListener.identifier;
        netSpecifier->netType_ = eventListener.netType;
        netSpecifier->netCapabilities_ = eventListener.netCapabilities;
        result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback);
    } else {
        result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    }
    return result;
}

int32_t EventListenerManager::RemoveEventListener(EventListener &eventListener)
{
    NETMGR_LOG_D("RemoveEventListener");
    int32_t result = 0;
    eventListenerHandler->RemoveEventListener();
    if (!eventListener.identifier.empty()) {
        sptr<NetSpecifier> netSpecifier = (std::make_unique<NetSpecifier>()).release();
        netSpecifier->ident_ = eventListener.identifier;
        netSpecifier->netType_ = eventListener.netType;
        netSpecifier->netCapabilities_ = eventListener.netCapabilities;
        result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(netSpecifier, callback);
    } else {
        result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
    }
    return result;
}
} // namespace NetManagerStandard
} // namespace OHOS
