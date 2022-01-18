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

#include "net_stats_callback.h"
#include "i_net_stats_callback.h"
#include "net_stats_event_listener_manager.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NetStatsCallback::NetIfaceStatsChanged(const std::string &iface)
{
    NETMGR_LOG_I("NetStatsCallback NetIfaceStatsChanged iface = %{public}s", iface.c_str());
    std::unique_ptr<IfaceStatsEvent> event = std::make_unique<IfaceStatsEvent>(iface);
    NetStatsEventListenerManager::GetInstance().SendEvent(NET_STATS_IFACE_CHANGED, event);
    return 0;
};

int32_t NetStatsCallback::NetUidStatsChanged(const std::string &iface, uint32_t uid)
{
    NETMGR_LOG_I("NetStatsCallback NetUidStatsChanged iface = %{public}s, uid = %{public}d", iface.c_str(), uid);
    std::unique_ptr<UidStatsEvent> event = std::make_unique<UidStatsEvent>(iface, uid);
    NetStatsEventListenerManager::GetInstance().SendEvent(NET_STATS_UID_CHANGED, event);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS