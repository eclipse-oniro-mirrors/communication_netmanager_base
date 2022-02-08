/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "net_policy_callback.h"
#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
void NetPolicyCallback::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return;
    }

    uint32_t callBackNum = netPolicyCallback_.size();
    NETMGR_LOG_D("netPolicyCallback_ callback num [%{public}d]", callBackNum);
    if (callBackNum >= LIMIT_CALLBACK_NUM) {
        NETMGR_LOG_E("netPolicyCallback_ callback num cannot more than [%{public}d]", LIMIT_CALLBACK_NUM);
        return;
    }

    for (uint32_t i = 0; i < callBackNum; i++) {
        if (callback->AsObject().GetRefPtr() == netPolicyCallback_[i]->AsObject().GetRefPtr()) {
            NETMGR_LOG_I("netPolicyCallback_ had this callback");
            return;
        }
    }

    netPolicyCallback_.emplace_back(callback);
}

void NetPolicyCallback::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return;
    }

    for (auto iter = netPolicyCallback_.begin(); iter != netPolicyCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            netPolicyCallback_.erase(iter);
            return;
        }
    }
}

int32_t NetPolicyCallback::NotifyNetUidPolicyChanged(uint32_t uid, NetUidPolicy policy)
{
    NETMGR_LOG_I("NotifyNetUidPolicyChanged  uid[%{public}d] policy[%{public}d] netPolicyCallback_[%{public}zd]",
        uid, static_cast<uint32_t>(policy), netPolicyCallback_.size());

    for (const auto &callback : netPolicyCallback_) {
        if (callback != nullptr) {
            callback->NetUidPolicyChanged(uid, policy);
        }
    }

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

int32_t NetPolicyCallback::NotifyNetBackgroundPolicyChanged(bool isBackgroundPolicyAllow)
{
    NETMGR_LOG_I("NotifyNetBackgroundPolicyChanged  backgroundPolicy[%{public}d] netPolicyCallback_[%{public}d]",
        isBackgroundPolicyAllow, static_cast<uint32_t>(netPolicyCallback_.size()));

    for (const auto &callback : netPolicyCallback_) {
        if (callback != nullptr) {
            callback->NetBackgroundPolicyChanged(isBackgroundPolicyAllow);
        }
    }

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

int32_t NetPolicyCallback::NotifyNetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    if (cellularPolicies.empty()) {
        NETMGR_LOG_E("NotifyNetCellularPolicyChanged cellularPolicies empty");
        return static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR);
    }
    NETMGR_LOG_I("NotifyNetCellularPolicyChanged cellularPolicies.size[%{public}zd]", cellularPolicies.size());

    for (const auto &callback : netPolicyCallback_) {
        if (callback != nullptr) {
            callback->NetCellularPolicyChanged(cellularPolicies);
        }
    }

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

int32_t NetPolicyCallback::NotifyNetStrategySwitch(const std::string &simId, bool enable)
{
    NETMGR_LOG_I("NotifyNetStrategySwitch simId[%{public}s] enable[%{public}d]",
        simId.c_str(), static_cast<int32_t>(enable));

    for (const auto &callback : netPolicyCallback_) {
        if (callback != nullptr) {
            callback->NetStrategySwitch(simId, enable);
        }
    }

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}
} // namespace NetManagerStandard
} // namespace OHOS
