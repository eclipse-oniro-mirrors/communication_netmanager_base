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
    monthDay_ = {MONTH_THIRTY_ONE, MONTH_TWENTY_EIGHT, MONTH_THIRTY_ONE, MONTH_THIRTY,
        MONTH_THIRTY_ONE, MONTH_THIRTY, MONTH_THIRTY_ONE, MONTH_THIRTY_ONE, MONTH_THIRTY,
        MONTH_THIRTY_ONE, MONTH_THIRTY, MONTH_THIRTY_ONE};
}

bool NetPolicyTraffic::IsPolicyValid(NetUidPolicy policy)
{
    switch (policy) {
        case NetUidPolicy::NET_POLICY_NONE:
        case NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND:
        case NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED:
        case NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND:
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

bool NetPolicyTraffic::IsNetPolicyTypeValid(NetQuotaPolicyType netType)
{
    switch (netType) {
        case NetQuotaPolicyType::NET_POLICY_MOBILE:
        case NetQuotaPolicyType::NET_POLICY_ETHERNET:
        case NetQuotaPolicyType::NET_POLICY_WIFI:
        case NetQuotaPolicyType::NET_POLICY_BLUETOOTH:
        case NetQuotaPolicyType::NET_POLICY_PROXY: {
            return true;
        }
        default: {
            NETMGR_LOG_E("Invalid netType [%{public}d]", static_cast<uint32_t>(netType));
            return false;
        }
    }
}

NetPolicyResultCode NetPolicyTraffic::AddUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("AddUidPolicy netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_ADD, uid, policy)) {
        NETMGR_LOG_E("AddUidPolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("SetUidPolicy netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE, uid, policy)) {
        NETMGR_LOG_E("SetUidPolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::DeleteUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("DeleteUidPolicy netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE, uid, policy)) {
        NETMGR_LOG_E("DeleteUidPolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

std::int64_t NetPolicyTraffic::GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec;
}

int32_t NetPolicyTraffic::GetPeriodEndTime()
{
    struct tm *timeNow;
    time_t second = time(nullptr);
    if (!second) {
        NETMGR_LOG_E("time second error");
        return -1;
    }
    timeNow = localtime(&second);
    if (timeNow == nullptr) {
        NETMGR_LOG_E("timeNow is nullptr");
        return -1;
    }

    if (timeNow->tm_mon == NET_POLICY_FEBRUARY
        && ((timeNow->tm_year % NET_POLICY_LEAP_YEAR_FOUR == 0
        && timeNow->tm_year % NET_POLICY_LEAP_YEAR_ONEHUNDRED != 0)
        || timeNow->tm_year % NET_POLICY_LEAP_YEAR_FOURHUNDRED == 0)) {
        return (monthDay_[timeNow->tm_mon] + NET_POLICY_LEAP_YEAR_ONE) * NET_POLICY_ONEDAYTIME;
    } else {
        return monthDay_[timeNow->tm_mon] * NET_POLICY_ONEDAYTIME;
    }
}

void NetPolicyTraffic::CheckNetStatsOverLimit(const std::vector<NetPolicyCellularPolicy> &cellularPolicys,
    const sptr<NetPolicyCallback> netPolicyCallback)
{
    if (cellularPolicys.empty()) {
        NETMGR_LOG_W("cellularPolicys size is 0.");
        return;
    }

    int32_t periodEndTime = GetPeriodEndTime();
    if (periodEndTime <= 0) {
        NETMGR_LOG_E("periodEndTime error.");
        return;
    }

    std::string ifaceName;
    for (uint32_t i = 0; i < cellularPolicys.size(); ++i) {
        /* -1 : unlimited */
        if (cellularPolicys[i].limitBytes_ == -1) {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(cellularPolicys[i].subscriberId_, true);
            }
            continue;
        }
        int32_t ret = NetManagerCenter::GetInstance().GetIfaceNameByType(NET_TYPE_CELLULAR,
            cellularPolicys[i].subscriberId_, ifaceName);
        if (ret != 0 || ifaceName.empty()) {
            NETMGR_LOG_E("GetIfaceNameByType ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NetStatsInfo netStatsInfo;
        ret = NetManagerCenter::GetInstance().GetIfaceStatsDetail(ifaceName, cellularPolicys[i].periodStartTime_,
            cellularPolicys[i].periodStartTime_ + periodEndTime, netStatsInfo);
        if (ret != 0) {
            NETMGR_LOG_E("GetIfaceStatsDetail ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NETMGR_LOG_I("GetIfaceStatsDetail txBytes_[%{public}" PRId64 "] rxBytes_[%{public}" PRId64 "]",
            netStatsInfo.txBytes_, netStatsInfo.rxBytes_);
        /*  The traffic exceeds the limit. You need to notify telephony to shut down the network. */
        if (netStatsInfo.txBytes_ + netStatsInfo.rxBytes_ < cellularPolicys[i].limitBytes_) {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(cellularPolicys[i].subscriberId_, true);
            }
        } else {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(cellularPolicys[i].subscriberId_, false);
            }
        }
    }
}

void NetPolicyTraffic::CheckNetStatsOverLimit(const std::vector<NetPolicyQuotaPolicy> &quotaPolicys,
    const sptr<NetPolicyCallback> netPolicyCallback)
{
    if (quotaPolicys.empty()) {
        NETMGR_LOG_W("quotaPolicys size is 0.");
        return;
    }

    int32_t periodEndTime = GetPeriodEndTime();
    if (periodEndTime <= 0) {
        NETMGR_LOG_E("periodEndTime error.");
        return;
    }

    std::string ifaceName;
    for (uint32_t i = 0; i < quotaPolicys.size(); ++i) {
        /* only control cellular traffic */
        if (static_cast<NetQuotaPolicyType>(quotaPolicys[i].netType_) != NetQuotaPolicyType::NET_POLICY_MOBILE) {
            NETMGR_LOG_I("need not notify telephony netType_[%{public}d]", quotaPolicys[i].netType_);
            continue;
        }
        int32_t ret = NetManagerCenter::GetInstance().GetIfaceNameByType(quotaPolicys[i].netType_,
            quotaPolicys[i].subscriberId_, ifaceName);
        if (ret != 0 || ifaceName.empty()) {
            NETMGR_LOG_E("GetIfaceNameByType ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NetStatsInfo netStatsInfo;
        ret = NetManagerCenter::GetInstance().GetIfaceStatsDetail(ifaceName, quotaPolicys[i].periodStartTime_,
            quotaPolicys[i].periodStartTime_ + periodEndTime, netStatsInfo);
        if (ret != 0) {
            NETMGR_LOG_E("GetIfaceStatsDetail ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NETMGR_LOG_I("GetIfaceStatsDetail txBytes_[%{public}" PRId64 "] rxBytes_[%{public}" PRId64 "]",
            netStatsInfo.txBytes_, netStatsInfo.rxBytes_);

        /* Sleep time is not up Or nerverSnooze : lastLimitSnooze_=1 */
        if ((quotaPolicys[i].lastLimitSnooze_ >= quotaPolicys[i].periodStartTime_ ||
            quotaPolicys[i].lastLimitSnooze_ == -1)
            && (netStatsInfo.txBytes_ + netStatsInfo.rxBytes_ < quotaPolicys[i].limitBytes_)) {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(quotaPolicys[i].subscriberId_, true);
            }
        } else {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(quotaPolicys[i].subscriberId_, false);
            }
        }
    }
}

NetPolicyResultCode NetPolicyTraffic::SetNetPolicys(const std::vector<NetPolicyQuotaPolicy> &quotaPolicys,
    const sptr<NetPolicyCallback> &netPolicyCallback)
{
    if (quotaPolicys.empty()) {
        NETMGR_LOG_E("quotaPolicys size is 0");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    int32_t netPolicyType = 0;
    for (uint32_t i = 0; i < quotaPolicys.size(); ++i) {
        netPolicyType = static_cast<int32_t>(quotaPolicys[i].netType_);
        if (!IsNetPolicyTypeValid(static_cast<NetQuotaPolicyType>(netPolicyType))) {
            NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", netPolicyType);
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }

        if (!IsNetPolicyPeriodDurationValid(quotaPolicys[i].periodDuration_)) {
            NETMGR_LOG_E("periodDuration [%{public}s] must Mx", quotaPolicys[i].periodDuration_.c_str());
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }
    }

    /* Judge whether the flow exceeds the limit */
    CheckNetStatsOverLimit(quotaPolicys, netPolicyCallback);

    if (!netPolicyFile_->WriteFile(quotaPolicys)) {
        NETMGR_LOG_E("SetNetPolicys WriteFile failed");
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

NetPolicyResultCode NetPolicyTraffic::SetCellularPolicys(const std::vector<NetPolicyCellularPolicy> &cellularPolicys,
    const sptr<NetPolicyCallback> &netPolicyCallback)
{
    if (cellularPolicys.empty()) {
        NETMGR_LOG_E("cellularPolicys size is 0");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    for (uint32_t i = 0; i < cellularPolicys.size(); ++i) {
        if (!IsNetPolicyPeriodDurationValid(cellularPolicys[i].periodDuration_)) {
            NETMGR_LOG_E("periodDuration [%{public}s] must Mx", cellularPolicys[i].periodDuration_.c_str());
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }

        if (cellularPolicys[i].limitAction_ < LIMIT_ACTION_ONE
            || cellularPolicys[i].limitAction_ > LIMIT_ACTION_THREE) {
            NETMGR_LOG_E("limitAction [%{public}d] must 1-3 ", cellularPolicys[i].limitAction_);
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }
    }

    /* Judge whether the flow exceeds the limit */
    CheckNetStatsOverLimit(cellularPolicys, netPolicyCallback);

    if (!netPolicyFile_->WriteFile(cellularPolicys)) {
        NETMGR_LOG_E("SetCellularPolicys WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    return NetPolicyResultCode::ERR_NONE;
}

bool NetPolicyTraffic::IsQuotaPolicyExist(const NetPolicyQuotaPolicy &quotaPolicy)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicys;
    if (netPolicyFile_->GetNetPolicys(quotaPolicys) != NetPolicyResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetNetPolicys failed");
        return false;
    }

    if (quotaPolicys.empty()) {
        NETMGR_LOG_E("quotaPolicys is empty");
        return false;
    }

    for (uint32_t i = 0; i < quotaPolicys.size(); i++) {
        if (quotaPolicy.netType_ == quotaPolicys[i].netType_
            && quotaPolicy.subscriberId_ == quotaPolicys[i].subscriberId_) {
            NETMGR_LOG_D("netQuotaPolicy exist");
            return true;
        }
    }

    return false;
}

NetPolicyResultCode NetPolicyTraffic::SnoozePolicy(const NetPolicyQuotaPolicy &quotaPolicy,
    const sptr<NetPolicyCallback> &netPolicyCallback)
{
    if (!IsNetPolicyTypeValid(static_cast<NetQuotaPolicyType>(quotaPolicy.netType_))) {
        NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", static_cast<int32_t>(quotaPolicy.netType_));
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    if (!IsQuotaPolicyExist(quotaPolicy)) {
        NETMGR_LOG_E("quotaPolicy is not exist");
        return NetPolicyResultCode::ERR_QUOTA_POLICY_NOT_EXIST;
    }

    std::vector<NetPolicyQuotaPolicy> quotaPolicys = {quotaPolicy};
    /* Set the sleep time to the current time. */
    quotaPolicys[0].lastLimitSnooze_ = GetCurrentTime();
    /* Judge whether the flow exceeds the limit */
    CheckNetStatsOverLimit(quotaPolicys, netPolicyCallback);
    if (!netPolicyFile_->WriteFile(quotaPolicys)) {
        NETMGR_LOG_E("SnoozePolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetIdleWhitelist(uint32_t uid, bool isWhiteList)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("SetIdleWhitelist netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    /* If it exists, update it directly */
    for (auto iter = idleWhiteList_.begin(); iter != idleWhiteList_.end(); ++iter) {
        if (uid == *iter) {
            if (!isWhiteList) {
                idleWhiteList_.erase(iter);
                return NetPolicyResultCode::ERR_NONE;
            } else {
                return NetPolicyResultCode::ERR_NONE;
            }
        }
    }
    /* Does not exist, add it */
    if (isWhiteList) {
        idleWhiteList_.emplace_back(uid);
    }
    /* Determine whether the app is idle ? than update netd's interface. */
    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::GetIdleWhitelist(std::vector<uint32_t> &uids)
{
    if (idleWhiteList_.empty()) {
        NETMGR_LOG_I("idleWhiteList_ is empty.");
    } else {
        uids = idleWhiteList_;
    }

    return NetPolicyResultCode::ERR_NONE;
}

void NetPolicyTraffic::ClearIdleWhiteList()
{
    idleWhiteList_.clear();
}
} // namespace NetManagerStandard
} // namespace OHOS
