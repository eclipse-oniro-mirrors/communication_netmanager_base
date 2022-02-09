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

#ifndef NETMANAGER_BASE_NET_STATS_EVENT_LISTENER_MANAGER_H
#define NETMANAGER_BASE_NET_STATS_EVENT_LISTENER_MANAGER_H
#include <memory>
#include <list>
#include "net_mgr_log_wrapper.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
enum EventHandleRun { STATE_NOT_START, STATE_RUNNING };

class NetStatsEventListenerManager {
public:
    static NetStatsEventListenerManager &GetInstance();
    int32_t AddEventListener(EventListener &eventListener);
    int32_t RemoveEventListener(EventListener &eventListener);
    int32_t FindListener(EventListener &listen);

private:
    NetStatsEventListenerManager();
    std::list<EventListener> listenerList;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_NET_STATS_EVENT_LISTENER_MANAGER_H
