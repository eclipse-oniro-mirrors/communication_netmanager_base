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

#include "net_policy_event_listener_context.h"
#include "net_policy_client.h"
#include "napi_net_policy_observer.h"
#include "napi_net_policy.h"

namespace OHOS {
namespace NetManagerStandard {
std::map<int32_t, EventListener> NetPolicyEventListenerContext::listenses;
static sptr<INetPolicyCallback> callback = (std::make_unique<NapiNetPolicyObserver>()).release();

NetPolicyEventListenerContext& NetPolicyEventListenerContext::GetInstance()
{
    NETMGR_LOG_D("NetPolicyEventListenerContext::GetInstance");
    static NetPolicyEventListenerContext instance;
    return instance;
}

int32_t NetPolicyEventListenerContext::AddEventListener(EventListener &eventListener)
{
    int32_t ret = 0;
    NETMGR_LOG_D("eventListener.eventId = [%{public}d]", eventListener.eventId);
    ret = DelayedSingleton<NetPolicyClient>::GetInstance()->RegisterNetPolicyCallback(callback);
    if (ret == 0) {
        listenses.insert(std::pair<int32_t, EventListener>(eventListener.eventId, eventListener));
    }
    return ret;
}

int32_t NetPolicyEventListenerContext::RemoveEventListener(EventListener &eventListener)
{
    int32_t ret = 0;
    NETMGR_LOG_D("RemoveEventListener");
    ret = DelayedSingleton<NetPolicyClient>::GetInstance()->UnregisterNetPolicyCallback(callback);
    if (ret == 0) {
        std::map<int32_t, EventListener>::iterator it = listenses.find(eventListener.eventId);
        if (it != listenses.end()) {
            napi_delete_reference(it->second.env, it->second.callbackRef);
            listenses.erase(eventListener.eventId);
        }
    }
    return ret;
}

int32_t NetPolicyEventListenerContext::FindEventListense(EventListener &eventListener)
{
    int32_t eventId = EVENT_NET_UNKNOW_CHANGE;
    std::map<int32_t, EventListener>::iterator it = listenses.find(eventListener.eventId);
    if (it != listenses.end()) {
        eventListener = it->second;
        eventId = it->first;
    }
    return eventId;
}
} // namespace NetManagerStandard
} // namespace OHOS
