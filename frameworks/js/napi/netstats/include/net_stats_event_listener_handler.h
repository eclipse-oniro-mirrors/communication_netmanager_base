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

#ifndef NET_STATS_EVENT_LISTENER_HANDLER_H
#define NET_STATS_EVENT_LISTENER_HANDLER_H

#include <list>
#include <uv.h> // support libuv
#include <memory>

#include "event_handler.h"
#include "event_runner.h"

#include "napi_common.h"
#include "napi_net_stats.h"
#include "net_stats_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetStatsEventListenerHandler : public AppExecFwk::EventHandler {
public:
    NetStatsEventListenerHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    ~NetStatsEventListenerHandler() = default;
    int32_t AddEventListener(EventListener &eventListener);
    int32_t RemoveEventListener(EventListener &eventListener);

    void NetIfaceStatsUpdated(EventListener &eventListener, std::unique_ptr<IfaceStatsEvent> &event) ;
    void NetUidStatsUpdated(EventListener &eventListener, std::unique_ptr<UidStatsEvent> &event) ;
private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
private:
    std::list<EventListener> listenerList;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_EVENT_LISTENER_HANDLER_H