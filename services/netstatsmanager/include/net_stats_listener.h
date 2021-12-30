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

#ifndef NET_STATS_LISTENER_H
#define NET_STATS_LISTENER_H

#include "common_event_manager.h"
#include "common_event_support.h"

#include "net_stats_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetStatsListener  : public EventFwk::CommonEventSubscriber {
public:
    explicit NetStatsListener(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp) {};
    NetStatsListener() = default;
    void SetStatsCallback(const sptr<NetStatsCallback> &callback);

public:
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);

private:
    sptr<NetStatsCallback> netStatsCallback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_LISTENER_H