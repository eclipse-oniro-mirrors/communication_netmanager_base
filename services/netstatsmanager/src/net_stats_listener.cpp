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
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace OHOS::EventFwk;

void NetStatsListener::SetStatsCallback(const sptr<NetStatsCallback> &callback)
{
    netStatsCallback_ = callback;
}

void NetStatsListener::OnReceiveEvent(const CommonEventData &data)
{
    NETMGR_LOG_I("NetStatsListener::OnReceiveEvent(), event:%s, data:%s, code:%d",
        data.GetWant().GetAction().c_str(), data.GetData().c_str(), data.GetCode());

    auto eventName = data.GetWant().GetAction();
    if (eventName.compare("usual.event.netmanager.NETMANAGER_NET_STATE_LIMITED") == 0) {
        int32_t code = data.GetCode();
        std::string iface = data.GetWant().GetStringParam("NetStatsIface");
        uint32_t uid = std::stoi(data.GetWant().GetStringParam("NetStatsUid"));
        netStatsCallback_->NotifyNetIfaceStatsChanged(iface);
        netStatsCallback_->NotifyNetUidStatsChanged(iface, uid);
        NETMGR_LOG_I("NetStatsListener::OnReceiveEvent, is NETMANAGER_NET_STATE_LIMITED, code:%d", code);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
