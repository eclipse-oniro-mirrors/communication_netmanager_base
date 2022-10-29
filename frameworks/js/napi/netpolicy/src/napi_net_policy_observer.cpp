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

#include "napi_common.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_event_listener_context.h"

namespace OHOS {
namespace NetManagerStandard {
static void OnNetUidPolicyEvent(EventListener &eventListener, uint32_t uid, NetUidPolicy policy)
{
    napi_value info = nullptr;
    napi_create_object(eventListener.env, &info);
    NapiCommon::SetPropertyUint32(eventListener.env, info, "uid", uid);
    NapiCommon::SetPropertyInt32(eventListener.env, info, "policy", static_cast<int32_t>(policy));
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    napi_value recv = nullptr;
    napi_value result = nullptr;
    napi_value callbackFunc = nullptr;
    if ((eventListener.env == nullptr) || (eventListener.callbackRef == nullptr)) {
        NETMGR_LOG_E("eventListener.env = nullptr || eventListener.callbackRef =nullptr");
        return;
    }
    napi_get_undefined(eventListener.env, &recv);
    napi_get_reference_value(eventListener.env, eventListener.callbackRef, &callbackFunc);
    callbackValues[ARGV_INDEX_1] = info;
    napi_call_function(eventListener.env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
}

int32_t NapiNetPolicyObserver::NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy)
{
    NETMGR_LOG_D("NapiNetPolicyObserver NetUidPolicyChanged(), uid = [%{public}d], policy = [%{public}u]", uid, policy);
    EventListener eventListener;
    eventListener.eventId = EVENT_NET_UID_POLICY_CHANGE;
    if (NetPolicyEventListenerContext::FindEventListense(eventListener) != EVENT_NET_UNKNOW_CHANGE) {
        OnNetUidPolicyEvent(eventListener, uid, policy);
    }
    return 0;
}

int32_t NapiNetPolicyObserver::NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow)
{
    return 0;
}

int32_t NapiNetPolicyObserver::NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicys)
{
    NETMGR_LOG_I("NapiNetPolicyObserver NetCellularPolicyChanged(), cellularPolicys.size = [%{public}zd]",
                 cellularPolicys.size());
    return 0;
}

int32_t NapiNetPolicyObserver::NetStrategySwitch(const std::string &simId, bool enable)
{
    NETMGR_LOG_D("NapiNetPolicyObserver NetStrategySwitch(), simId = [%{public}s], enable = [%{public}d]",
                 simId.c_str(), enable);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
