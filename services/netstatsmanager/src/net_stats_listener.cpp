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

#include "net_stats_listener.h"

#include "common_event_support.h"

#include "net_stats_csv.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string EVENT_DATA_IFACE_LIMITED = "Net Manager Iface States Limited";
const std::string EVENT_DATA_UID_LIMITED = "Net Manager Uid States Limited";
const std::string EVENT_DATA_IFACE_PARAM = "NetStatsIface";
const std::string EVENT_DATA_UID_PARAM = "NetStatsUid";
const std::string EVENT_DATA_DELETED_UID_PARAM = "DeletedUid";

using namespace OHOS::EventFwk;

void NetStatsListener::SetStatsCallback(const sptr<NetStatsCallback> &callback)
{
    netStatsCallback_ = callback;
}

void NetStatsListener::OnReceiveEvent(const CommonEventData &data)
{
    NETMGR_LOG_I("NetStatsListener::OnReceiveEvent(), event:[%{public}s], data:[%{public}s], code:[%{public}d]",
        data.GetWant().GetAction().c_str(), data.GetData().c_str(), data.GetCode());

    auto eventName = data.GetWant().GetAction();
    if (eventName.compare((EventFwk::CommonEventSupport::COMMON_EVENT_NETMANAGER_NETSTATES_LIMITED).c_str()) == 0) {
        NETMGR_LOG_I("usual.event.netmanager.base.STATES_LIMITED");
        auto eventData = data.GetData();
        if (eventData.compare(EVENT_DATA_IFACE_LIMITED.c_str()) == 0) {
            std::string iface = data.GetWant().GetStringParam(EVENT_DATA_IFACE_PARAM.c_str());
            netStatsCallback_->NotifyNetIfaceStatsChanged(iface);
            NETMGR_LOG_I("Net Manager Iface States Limited, iface:[%{public}s]", iface.c_str());
        } else if (eventData.compare(EVENT_DATA_UID_LIMITED.c_str()) == 0) {
            std::string iface = data.GetWant().GetStringParam(EVENT_DATA_IFACE_PARAM.c_str());
            uint32_t uid = std::stoi(data.GetWant().GetStringParam(EVENT_DATA_UID_PARAM.c_str()));
            netStatsCallback_->NotifyNetUidStatsChanged(iface, uid);
            NETMGR_LOG_I("Net Manager Uid States Limited, iface:[%{public}s], uid:[%{public}d]", iface.c_str(), uid);
        }
    } else if (eventName.compare((EventFwk::CommonEventSupport::COMMON_EVENT_UID_REMOVED).c_str()) == 0) {
        NETMGR_LOG_I("usual.event.UID_REMOVED");
        uint32_t uid = std::stoi(data.GetWant().GetStringParam(EVENT_DATA_DELETED_UID_PARAM.c_str()));
        sptr<NetStatsCsv> statsCsv = (std::make_unique<NetStatsCsv>()).release();
        statsCsv->DeleteUidStatsCsv(uid);
        NETMGR_LOG_I("Net Manager delete uid, uid:[%{public}d]", uid);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
