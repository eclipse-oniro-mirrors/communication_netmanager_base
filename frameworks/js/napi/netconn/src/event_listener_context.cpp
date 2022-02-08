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
std::mutex EventListenerContext::mtx;

EventListenerContext &EventListenerContext::GetInstance()
{
    NETMGR_LOG_I("EventListenerContext::GetInstance");
    static EventListenerContext instance;
    return instance;
}

int32_t EventListenerContext::AddListense(NapiNetConnection *conn, EventListener &listen)
{
    NETMGR_LOG_I("EventListenerContext::AddListense");
    std::lock_guard<std::mutex> lck(mtx);
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_I("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    listenses[index].insert(std::pair<int32_t, EventListener>(listen.eventId, listen));
    Display();
    return 0;
}

int32_t EventListenerContext::RemoveListense(NapiNetConnection *conn, EventListener &listen)
{
    NETMGR_LOG_I("EventListenerContext::RemoveListense");
    std::lock_guard<std::mutex> lck(mtx);
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_I("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    listenses[index].erase(listen.eventId);
    Display();
    return 0;
}

int32_t EventListenerContext::Register(NapiNetConnection *conn)
{
    NETMGR_LOG_I("EventListenerContext::Register");
    int32_t ret = 0;
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_I("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    sptr<INetConnCallback> callback = (std::make_unique<NapiNetConnObserver>()).release();
    {
        std::lock_guard<std::mutex> lck(mtx);
        callbacks[index] = callback;
    }
    if ((conn->hasSpecifier) && (conn->hasTimeout)) {
        sptr<NetSpecifier> specifier = (std::make_unique<NetSpecifier>(conn->netSpecifier_)).release();
        ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(
            specifier, callback, conn->timeout_);
    } else {
        ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    }
    if (ret == 0) {
        NETMGR_LOG_I("Register successful");
    } else {
        NETMGR_LOG_I("Register fail");
        std::lock_guard<std::mutex> lck(mtx);
        RemoveIndexListense(index);
        callbacks.erase(index);
    }
    NETMGR_LOG_I("Register ret = [%{public}d]", ret);
    return ret;
}

int32_t EventListenerContext::Unregister(NapiNetConnection *conn)
{
    NETMGR_LOG_I("EventListenerContext::Unregister");
    int32_t ret = 0;
    intptr_t index = intptr_t(static_cast<void *>(conn));
    NETMGR_LOG_I("NetConnection *conn = [%{public}" PRIdPTR "]", index);
    sptr<INetConnCallback> callback;
    {
        std::lock_guard<std::mutex> lck(mtx);
        std::map<intptr_t, sptr<INetConnCallback>>::iterator iter = callbacks.find(index);
        if (iter != callbacks.end()) {
            callback = iter->second;
        }
    }
    if (callback != nullptr) {
        NETMGR_LOG_I("Hava callback");
        ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
        if (ret != 0) {
            NETMGR_LOG_I("Unregister fail = [%{public}d]", ret);
            return ret;
        } else {
            NETMGR_LOG_I("Unregister successful = [%{public}d]", ret);
            std::lock_guard<std::mutex> lck(mtx);
            RemoveIndexListense(index);
            callbacks.erase(index);
        }
    } else {
        NETMGR_LOG_I("Hava not callback");
    }
    return ret;
}

int32_t EventListenerContext::RemoveCallback(NapiNetConnObserver *observer)
{
    std::lock_guard<std::mutex> lck(mtx);
    intptr_t index = 0;
    for (std::map<intptr_t, sptr<INetConnCallback>>::iterator it = callbacks.begin(); it != callbacks.end(); ++it) {
        if (observer == it->second) {
            NETMGR_LOG_I("RemoveCallback, [%{public}p] == [%{public}p]", observer, it->second->AsObject().GetRefPtr());
            index = it->first;
            RemoveIndexListense(index);
            callbacks.erase(index);
            break;
        }
    }
    NETMGR_LOG_I("RemoveCallback index = [%{public}" PRIdPTR "]", index);
    return index;
}

int32_t EventListenerContext::RemoveIndexListense(intptr_t index)
{
    std::map<intptr_t, std::map<int32_t, EventListener>>::iterator it = listenses.find(index);
    if (it == listenses.end()) {
        return 0;
    }
    for (std::map<int32_t, EventListener>::iterator itt = it->second.begin(); itt != it->second.end(); ++itt) {
        NETMGR_LOG_I("Remove EventListener[%{public}" PRIdPTR "][%{public}d] = [%{public}d]", it->first,
            itt->first, itt->second.eventId);
        napi_delete_reference(itt->second.env, itt->second.callbackRef);
    }
    listenses.erase(index);
    return 0;
}

int32_t EventListenerContext::Display()
{
    for (std::map<intptr_t, std::map<int32_t, EventListener>>::iterator it = listenses.begin();
         it != listenses.end(); ++it) {
        for (std::map<int32_t, EventListener>::iterator itt = it->second.begin(); itt != it->second.end(); ++itt) {
            NETMGR_LOG_I("listenses[%{public}" PRIdPTR "][%{public}d] = [%{public}d]", it->first, itt->first,
                itt->second.eventId);
        }
    }
    return 0;
}

int32_t EventListenerContext::FindListener(NapiNetConnObserver *observer, EventListener &listen)
{
    std::lock_guard<std::mutex> lck(mtx);
    int32_t eventId = EVENT_NET_UNKNOW_CHANGE;
    intptr_t index = 0;
    NETMGR_LOG_I("FindListener, callbacks.size = [%{public}zd]", callbacks.size());
    for (std::map<intptr_t, sptr<INetConnCallback>>::iterator it = callbacks.begin(); it != callbacks.end(); ++it) {
        if (observer == it->second) {
            NETMGR_LOG_I("FindListener, [%{public}p] == [%{public}p]", observer, it->second->AsObject().GetRefPtr());
            index = it->first;
            eventId = listen.eventId;
            listen = listenses[index][eventId];
            break;
        }
    }
    return eventId;
}
} // namespace NetManagerStandard
} // namespace OHOS
