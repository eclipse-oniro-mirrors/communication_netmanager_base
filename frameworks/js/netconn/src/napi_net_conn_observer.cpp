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

#include "napi_net_conn_observer.h"
#include "net_mgr_log_wrapper.h"
#include "event_listener_manager.h"
#include "napi_net_conn.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NapiNetConnObserver::NetConnStateChanged(const sptr<NetConnCallbackInfo> &info)
{
    std::unique_ptr<NetConnCallbackInfo> netConnInfo = std::make_unique<NetConnCallbackInfo>();
    netConnInfo->netState_ = info->netState_;
    netConnInfo->netType_ = info->netType_;
    NETMGR_LOG_D("NetConnStateChanged, netConnInfo->netState_ = [%{public}d], netConnInfo->netType_ = [%{public}d]",
        netConnInfo->netState_, netConnInfo->netType_);
    bool result = EventListenerManager::GetInstance().SendEvent(EVENT_NET_CONN_CHANGE, netConnInfo);
    NETMGR_LOG_D("NetConnStateChanged result = [%{public}d]", result);
    return 0;
}

int32_t NapiNetConnObserver::NetAvailable(int32_t netId)
{
    NETMGR_LOG_D("NapiNetConnObserver NetAvailable netId [%{public}d]", netId);
    return 0;
}

int32_t NapiNetConnObserver::NetCapabilitiesChange(int32_t netId, const uint64_t &netCap)
{
    NETMGR_LOG_D("NapiNetConnObserver NetCapabilitiesChange netId [%{public}d], netcap [%{public}" PRIu64 "]", netId,
        netCap);
    return 0;
}

int32_t NapiNetConnObserver::NetConnectionPropertiesChange(int32_t netId, const sptr<NetLinkInfo> &info)
{
    NETMGR_LOG_D("NapiNetConnObserver NetConnectionPropertiesChange netId [%{public}d], info is [%{public}s]",
        netId, info == nullptr ? "nullptr" : "not nullptr");
    return 0;
}

int32_t NapiNetConnObserver::NetLost(int32_t netId)
{
    NETMGR_LOG_D("NapiNetConnObserver NetLost netId [%{public}d]", netId);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
