/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <net/if.h>
#include "net_info_observer.h"
#include <unistd.h>
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_service.h"
#include "net_stats_utils.h"

namespace OHOS {
namespace NetManagerStandard {

int32_t NetInfoObserver::NetCapabilitiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
    const sptr<NetManagerStandard::NetAllCapabilities> &netAllCap)
{
    NETMGR_LOG_D("NetInfoObserver NetCapabilitiesChange");
    return 0;
}

int32_t NetInfoObserver::NetAvailable(sptr<NetManagerStandard::NetHandle> &netHandle)
{
    NETMGR_LOG_D("NetInfoObserver NetAvailable");
    return 0;
}

int32_t NetInfoObserver::NetLost(sptr<NetManagerStandard::NetHandle> &netHandle)
{
    isNeedUpdate_ = true;
    NETMGR_LOG_I("NetInfoObserver NetLost");
    return 0;
}

int32_t NetInfoObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info)
{
    if (info == nullptr) {
        isNeedUpdate_ = true;
        return -1;
    }
    NETMGR_LOG_I("NetInfoObserver NetConnectionPropertiesChange ifName: %{public}s, idnet: %{public}s",
        info->ifaceName_.c_str(), info->ident_.c_str());
    if (info->ident_.empty()) {  // wifi场景
        uint64_t ifindex = if_nametoindex(info->ifaceName_.c_str());
        DelayedSingleton<NetStatsService>::GetInstance()->ProcessNetConnectionPropertiesChange(INT32_MAX, ifindex_);
        isNeedUpdate_ = true;
        return 0;
    }

    int32_t ident = 0;
    NetStatsUtils::ConvertToInt32(info->ident_, ident);
    if (!isNeedUpdate_ && ident == ident_ && info->ifaceName_ == ifaceName_) {
        return 0;
    }
    isNeedUpdate_ = false;
    ident_ = ident;
    ifaceName_ = info->ifaceName_;
    ifindex_ = if_nametoindex(ifaceName_.c_str());
    DelayedSingleton<NetStatsService>::GetInstance()->ProcessNetConnectionPropertiesChange(ident_, ifindex_);
    return 0;
}

} // namespace NetManagerStandard
} // namespace OHOS
