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

#include "napi_net_policy_observer.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_event_listener_manager.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NapiNetPolicyObserver::NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy)
{
    NETMGR_LOG_I("NapiNetPolicyObserver NetUidPolicyChanged(), uid = [%{public}d], policy = [%{public}u]", uid, policy);
    std::unique_ptr<UidPolicyEvent> event = std::make_unique<UidPolicyEvent>(uid, policy);
    NetPolicyEventListenerManager::GetInstance().SendEvent(EVENT_NET_UID_POLICY_CHANGE, event);
    return 0;
}

int32_t NapiNetPolicyObserver::NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow)
{
    return 0;
}

int32_t NapiNetPolicyObserver::NetCellularPolicyChanged(
    const std::vector<NetPolicyCellularPolicy> &cellularPolicys)
{
    NETMGR_LOG_I("NapiNetPolicyObserver NetCellularPolicyChanged(), cellularPolicys.size = [%{public}zd]",
        cellularPolicys.size());
    std::unique_ptr<CellularPolicyEvent> event = std::make_unique<CellularPolicyEvent>(cellularPolicys);
    NetPolicyEventListenerManager::GetInstance().SendEvent(EVENT_NET_CELLULAR_POLICY_CHANGE, event);
    return 0;
}

int32_t NapiNetPolicyObserver::NetStrategySwitch(int32_t slotId, bool enable)
{
    NETMGR_LOG_I("NapiNetPolicyObserver NetStrategySwitch(), subscriberId = [%{public}d], enable = [%{public}d]",
        slotId, enable);
    std::unique_ptr<StrategySwitchEvent> event = std::make_unique<StrategySwitchEvent>(slotId, enable);
    NetPolicyEventListenerManager::GetInstance().SendEvent(EVENT_NET_STRATEGY_SWITCH_CHANGE, event);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
