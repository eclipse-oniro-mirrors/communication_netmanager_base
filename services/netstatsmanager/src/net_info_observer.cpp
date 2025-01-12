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

namespace OHOS {
namespace NetManagerStandard {

int32_t NetInfoObserver::NetCapabilitiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
    const sptr<NetManagerStandard::NetAllCapabilities> &netAllCap)
{
    if (netAllCap == nullptr) {
        return 0;
    }

    NETMGR_LOG_E("NetInfoObserver NetCapabilitiesChange");

    if (netAllCap->netCaps_.count(NetManagerStandard::NET_CAPABILITY_INTERNET) <= 0 ||
        netAllCap->netCaps_.count(NetManagerStandard::NET_CAPABILITY_VALIDATED) <= 0) {
        NETMGR_LOG_E("NetCapabilitiesChange not NetAvailable");
        return 0;
    }
    return 0;
}

int32_t NetInfoObserver::NetAvailable(sptr<NetManagerStandard::NetHandle> &netHandle)
{
    NETMGR_LOG_E("NetInfoObserver NetAvailable");
    return 0;
}

int32_t NetInfoObserver::NetLost(sptr<NetManagerStandard::NetHandle> &netHandle)
{
    NETMGR_LOG_E("NetInfoObserver NetLost");
    if (netHandle == nullptr) {
        NETMGR_LOG_E("netHandle is nullptr");
        return -1;
    }

    return 0;
}

int32_t NetInfoObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info)
{
    NETMGR_LOG_E("NetInfoObserver NetConnectionPropertiesChange");
    if (info == nullptr) {
        return -1;
    }
    NETMGR_LOG_E("NetInfoObserver ifName: %{public}s, idnet: %{public}s",
        info->ifaceName_.c_str(), info->ident_.c_str());
    if (info->ident_ == "") {  // wifi场景
        uint64_t ifindex = if_nametoindex(info->ifaceName_.c_str());
        DelayedSingleton<NetStatsService>::GetInstance()->ProcessNetConnectionPropertiesChange(INT32_MAX, ifindex_);
        return 0;
    }
    if (stoul(info->ident_) == ident_) {  // 默认网络对应的sim卡没变
        return 0;
    }

    ident_ = stoul(info->ident_);
    ifaceName_ = info->ifaceName_;
    ifindex_ = if_nametoindex(ifaceName_.c_str());
    DelayedSingleton<NetStatsService>::GetInstance()->ProcessNetConnectionPropertiesChange(ident_, ifindex_);
    return 0;
}

} // namespace NetManagerStandard
} // namespace OHOS