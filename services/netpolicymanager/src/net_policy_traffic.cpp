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
#include "net_policy_traffic.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>
#include <cinttypes>

#include "system_ability_definition.h"

#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_policy_file.h"
#include "net_policy_quota_policy.h"
#include "net_specifier.h"
#include "net_stats_info.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyTraffic::NetPolicyTraffic(sptr<NetPolicyFile> netPolicyFile) : netPolicyFile_(netPolicyFile)
{
}

bool NetPolicyTraffic::IsPolicyValid(NetUidPolicy policy)
{
    switch (policy) {
        case NetUidPolicy::NET_POLICY_NONE:
        case NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND:
        case NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED:
        case NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND:
        case NetUidPolicy::NET_POLICY_ALLOW_METERED:
        case NetUidPolicy::NET_POLICY_REJECT_METERED:
        case NetUidPolicy::NET_POLICY_ALLOW_ALL:
        case NetUidPolicy::NET_POLICY_REJECT_ALL: {
            return true;
        }
        default: {
            NETMGR_LOG_E("Invalid policy [%{public}d]", static_cast<uint32_t>(policy));
            return false;
        }
    }
}

bool NetPolicyTraffic::IsNetPolicyTypeValid(NetBearType netType)
{
    switch (netType) {
        case NetBearType::BEARER_CELLULAR:
        case NetBearType::BEARER_WIFI:
        case NetBearType::BEARER_BLUETOOTH:
        case NetBearType::BEARER_ETHERNET:
        case NetBearType::BEARER_VPN:
        case NetBearType::BEARER_WIFI_AWARE: {
            return true;
        }
        default: {
            NETMGR_LOG_E("Invalid netType [%{public}d]", static_cast<uint32_t>(netType));
            return false;
        }
    }
}

NetPolicyResultCode NetPolicyTraffic::AddPolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("AddPolicyByUid netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_ADD, uid, policy)) {
        NETMGR_LOG_E("AddPolicyByUid WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetPolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("SetPolicyByUid netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE, uid, policy)) {
        NETMGR_LOG_E("SetPolicyByUid WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::DeletePolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("DeletePolicyByUid netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE, uid, policy)) {
        NETMGR_LOG_E("DeletePolicyByUid WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetNetQuotaPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("quotaPolicies size is 0");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    for (uint32_t i = 0; i < quotaPolicies.size(); ++i) {
        int32_t netPolicyType = static_cast<int32_t>(quotaPolicies[i].netType_);
        if (!IsNetPolicyTypeValid(static_cast<NetBearType>(netPolicyType))) {
            NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", netPolicyType);
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }

        if (!IsNetPolicyPeriodDurationValid(quotaPolicies[i].periodDuration_)) {
            NETMGR_LOG_E("periodDuration [%{public}s] must Mx", quotaPolicies[i].periodDuration_.c_str());
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }
    }

    if (!netPolicyFile_->WriteFile(quotaPolicies)) {
        NETMGR_LOG_E("SetNetQuotaPolicies WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

bool NetPolicyTraffic::IsNetPolicyPeriodDurationValid(const std::string &periodDuration)
{
    if (periodDuration.empty()) {
        NETMGR_LOG_E("periodDuration is empty");
        return false;
    }

    std::string subString = periodDuration.substr(0, PERIODDURATION_POS_NUM_ONE);
    if (subString != POLICY_QUOTA_MONTH_U && subString != POLICY_QUOTA_MONTH_L) {
        NETMGR_LOG_E("periodDuration must Mx");
        return false;
    }

    subString = periodDuration.substr(PERIODDURATION_POS_NUM_ONE, periodDuration.length() - PERIODDURATION_POS_NUM_ONE);
    int32_t day = static_cast<int32_t>(std::stol(subString));
    if (day > DAY_THIRTY_ONE || day < DAY_ONE) {
        NETMGR_LOG_E("periodDuration must Mx, x is [%{public}d] - [%{public}d]", DAY_ONE, DAY_THIRTY_ONE);
        return false;
    }

    return true;
}

NetPolicyResultCode NetPolicyTraffic::SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    if (cellularPolicies.empty()) {
        NETMGR_LOG_E("cellularPolicies size is 0");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    for (uint32_t i = 0; i < cellularPolicies.size(); ++i) {
        if (!IsNetPolicyPeriodDurationValid(cellularPolicies[i].periodDuration_)) {
            NETMGR_LOG_E("periodDuration [%{public}s] must Mx", cellularPolicies[i].periodDuration_.c_str());
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }

        if (cellularPolicies[i].limitAction_ < LIMIT_ACTION_ONE
            || cellularPolicies[i].limitAction_ > LIMIT_ACTION_THREE) {
            NETMGR_LOG_E("limitAction [%{public}d] must 1-3 ", cellularPolicies[i].limitAction_);
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }
    }

    if (!netPolicyFile_->WriteFile(cellularPolicies)) {
        NETMGR_LOG_E("SetCellularPolicies WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

bool NetPolicyTraffic::IsQuotaPolicyExist(int8_t netType, const std::string &simId)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;
    if (netPolicyFile_->GetNetQuotaPolicies(quotaPolicies) != NetPolicyResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetNetQuotaPolicies failed");
        return false;
    }

    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("quotaPolicies is empty");
        return false;
    }

    for (uint32_t i = 0; i < quotaPolicies.size(); i++) {
        if (netType == quotaPolicies[i].netType_ && simId == quotaPolicies[i].simId_) {
            NETMGR_LOG_D("netQuotaPolicy exist");
            return true;
        }
    }

    return false;
}

NetPolicyResultCode NetPolicyTraffic::SetSnoozePolicy(int8_t netType, const std::string &simId,
    std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    if (!IsNetPolicyTypeValid(static_cast<NetBearType>(netType))) {
        NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", netType);
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    if (!IsQuotaPolicyExist(netType, simId)) {
        NETMGR_LOG_E("quotaPolicy is not exist");
        return NetPolicyResultCode::ERR_QUOTA_POLICY_NOT_EXIST;
    }

    if (!netPolicyFile_->WriteFile(quotaPolicies)) {
        NETMGR_LOG_E("SetSnoozePolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetIdleTrustlist(uint32_t uid, bool isTrustlist)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("SetIdleTrustlist netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    /* If it exists, update it directly */
    for (auto iter = idleTrustList_.begin(); iter != idleTrustList_.end(); ++iter) {
        if (uid == *iter) {
            if (!isTrustlist) {
                idleTrustList_.erase(iter);
                return NetPolicyResultCode::ERR_NONE;
            } else {
                return NetPolicyResultCode::ERR_NONE;
            }
        }
    }
    /* Does not exist, add it */
    if (isTrustlist) {
        idleTrustList_.emplace_back(uid);
    }
    /* Determine whether the app is idle ? than update netd's interface. */
    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::GetIdleTrustlist(std::vector<uint32_t> &uids)
{
    if (idleTrustList_.empty()) {
        NETMGR_LOG_I("idleTrustList_ is empty.");
    } else {
        uids = idleTrustList_;
    }

    return NetPolicyResultCode::ERR_NONE;
}

void NetPolicyTraffic::ClearIdleTrustList()
{
    idleTrustList_.clear();
}
} // namespace NetManagerStandard
} // namespace OHOS
