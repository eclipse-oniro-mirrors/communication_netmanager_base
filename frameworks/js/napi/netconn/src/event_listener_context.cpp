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

#include "event_listener_context.h"

#include <cinttypes>

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "net_conn_client.h"
#include "napi_net_conn_observer.h"
#include "napi_net_conn.h"

namespace OHOS {
namespace NetManagerStandard {
std::map<intptr_t, std::map<int32_t, EventListener>> EventListenerContext::listenses;
std::map<intptr_t, sptr<INetConnCallback>> EventListenerContext::callbacks;
EventListenerContext &EventListenerContext::GetInstance()
{
    NETMGR_LOG_D("EventListenerContext::GetInstance");
    static EventListenerContext instance;
    return instance;
}

int32_t EventListenerContext::AddListense(NapiNetConnection *conn, EventListener &listen)
{
    NETMGR_LOG_D("EventListenerContext::AddListense");
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_D("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    listenses[index].insert(std::pair<int32_t, EventListener>(listen.eventId, listen));
    Display();
    return 0;
}

int32_t EventListenerContext::RemoveListense(NapiNetConnection *conn, EventListener &listen)
{
    NETMGR_LOG_D("EventListenerContext::RemoveListense");
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_D("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    listenses[index].erase(listen.eventId);
    Display();
    return 0;
}

int32_t EventListenerContext::Register(NapiNetConnection *conn)
{
    NETMGR_LOG_D("EventListenerContext::Register");
    int32_t ret = 0;
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_D("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    sptr<INetConnCallback> callback = (std::make_unique<NapiNetConnObserver>()).release();
    if ((conn->hasSpecifier) && (conn->hasTimeout)) {
        sptr<NetSpecifier> specifier = (std::make_unique<NetSpecifier>(conn->netSpecifier_)).release();
        ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(
            specifier, callback, conn->timeout_);
    } else {
        ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    }
    if (ret == 0) {
        callbacks[index] = callback;
    }
    NETMGR_LOG_D("Register ret = [%{public}d]", ret);
    return ret;
}

int32_t EventListenerContext::Unregister(NapiNetConnection *conn)
{
    NETMGR_LOG_D("EventListenerContext::Unregister");
    int32_t ret = 0;
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_D("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    sptr<INetConnCallback> callback = callbacks[index];
    if (callback != nullptr) {
        NETMGR_LOG_D("Hava callback");
        ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
        NETMGR_LOG_D("Unregister ret = [%{public}d]", ret);
        if (ret == 0) {
            listenses.erase(index);
        }
    } else {
        NETMGR_LOG_D("Hava not callback");
    }
    return ret;
}

int32_t EventListenerContext::Display()
{
    for (std::map<intptr_t, std::map<int32_t, EventListener>>::iterator it = listenses.begin();
         it != listenses.end(); ++it) {
        for (std::map<int32_t, EventListener>::iterator itt = it->second.begin(); itt != it->second.end(); ++itt) {
            NETMGR_LOG_D("listenses[%{public}" PRIdPTR "][%{public}d] = [%{public}d]", it->first, itt->first,
                itt->second.eventId);
        }
    }
    return 0;
}

int32_t EventListenerContext::FindListener(NapiNetConnObserver *observer, EventListener &listen)
{
    int32_t eventId = EVENT_NET_UNKNOW_CHANGE;
    int32_t index = 0;
    for (std::map<intptr_t, sptr<INetConnCallback>>::iterator it = callbacks.begin(); it!=callbacks.end(); ++it) {
        if (observer == it->second) {
            index = it->first;
            eventId = listen.eventId;
            listen = listenses[index][eventId];
        }
    }
    return eventId;
}
} // namespace NetManagerStandard
} // namespace OHOS
