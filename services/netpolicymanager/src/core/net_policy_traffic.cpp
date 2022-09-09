/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_publish_info.h"
#include "common_event_support.h"
#include "system_ability_definition.h"

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_constants.h"
#include "net_policy_file.h"
#include "net_policy_inner_define.h"
#include "net_quota_policy.h"
#include "net_specifier.h"
#include "net_stats_info.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
void NetPolicyTraffic::Init()
{
    netsysCallback_ = new NetsysControllerCallbackImpl((
        std::static_pointer_cast<NetPolicyTraffic>(shared_from_this())));
    GetNetsysInst()->RegisterNetsysCallback(netsysCallback_);
    ReadQuotaPolicies();
}

bool NetPolicyTraffic::IsValidQuotaPolicy(const NetQuotaPolicy &quotaPolicy)
{
    if (quotaPolicy.netType == -1) {
        return false;
    }
    int32_t netType = quotaPolicy.netType;
    if (!IsValidNetType(netType)) {
        NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", netType);
        return false;
    }

    if (!IsValidPeriodDuration(quotaPolicy.periodDuration)) {
        NETMGR_LOG_E("periodDuration [%{public}s] must Mx", quotaPolicy.periodDuration.c_str());
        return false;
    }
    return true;
}

bool NetPolicyTraffic::IsValidNetType(int32_t netType)
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
            NETMGR_LOG_E("Invalid netType [%{public}d]", netType);
            return false;
        }
    }
}

int32_t NetPolicyTraffic::UpdateQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    // formalize the quota policy
    NetmanagerHiTrace::NetmanagerStartSyncTrace("FormalizeQuotaPolicies quotaPolicies start");
    FormalizeQuotaPolicies(quotaPolicies);
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("FormalizeQuotaPolicies quotaPolicies end");
    return UpdateQuotaPoliciesInner();
}

int32_t NetPolicyTraffic::UpdateQuotaPoliciesInner()
{
    NETMGR_LOG_I("UpdateQuotaPoliciesInner");
    // calculate the quota remain and get the metered ifaces
    NetmanagerHiTrace::NetmanagerStartSyncTrace("UpdateMeteredIfacesQuota start");
    auto meteredIfaces = UpdateMeteredIfacesQuota();
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("UpdateMeteredIfacesQuota end");

    // update the metered ifaces and notify the changes.
    NetmanagerHiTrace::NetmanagerStartSyncTrace("UpdateMeteredIfaces meteredIfaces start");
    UpdateMeteredIfaces(meteredIfaces);
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("UpdateMeteredIfaces meteredIfaces end");

    // notify quota limit or warning.
    NetmanagerHiTrace::NetmanagerStartSyncTrace("UpdateQuotaNotify start");
    UpdateQuotaNotify();
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("UpdateQuotaNotify end");
    // write quota policies to file.
    if (!WriteQuotaPolicies()) {
        NETMGR_LOG_E("UpdateQuotaPolicies WriteFile failed");
        return ERR_INTERNAL_ERROR;
    }
    // notify the the quota policy change.
    GetCbInst()->NotifyNetQuotaPolicyChange(quotaPolicies_);
    return ERR_NONE;
}

void NetPolicyTraffic::FormalizeQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    quotaPolicies_.clear();
    for (auto quotaPolicy : quotaPolicies) {
        if (!IsValidQuotaPolicy(quotaPolicy)) {
            NETMGR_LOG_E("UpdateQuotaPolicies invalid quota policy[%{public}p]", &quotaPolicy);
            continue;
        }
        if (quotaPolicy.limitBytes == DATA_USAGE_UNKNOWN) {
            quotaPolicy.limitAction = LIMIT_ACTION_AUTO_BILL;
        } else if (quotaPolicy.warningBytes == DATA_USAGE_UNKNOWN) {
            quotaPolicy.warningBytes = quotaPolicy.limitBytes * NINETY_PERCENTAGE / HUNDRED_PERCENTAGE;
        }
        if (quotaPolicy.limitAction == LIMIT_ACTION_AUTO_BILL) {
            quotaPolicy.limitBytes = DATA_USAGE_UNLIMITED;
        }
        if (quotaPolicy.warningBytes > quotaPolicy.limitBytes) {
            quotaPolicy.warningBytes = DATA_USAGE_UNLIMITED;
        }
        if (quotaPolicy.limitBytes == DATA_USAGE_UNLIMITED) {
            quotaPolicy.limitAction = LIMIT_ACTION_AUTO_BILL;
        }
        quotaPolicies_.push_back(quotaPolicy);
    }
}

const std::vector<std::string> NetPolicyTraffic::UpdateMeteredIfacesQuota()
{
    std::vector<std::string> newMeteredIfaces;
    for (auto &quotaPolicy : quotaPolicies_) {
        std::string iface = GetMatchIfaces(quotaPolicy);
        // set quota for metered iface.
        NETMGR_LOG_D("GetMatchIfaces is:[%{public}s] metered is:[%{public}d]", iface.c_str(), quotaPolicy.metered);
        if (iface == UNKNOW_IFACE || !quotaPolicy.metered) {
            continue;
        }
        newMeteredIfaces.push_back(iface);
        int64_t quotaRemain = GetQuotaRemain(quotaPolicy);
        if (quotaRemain >= 0) {
            GetNetsysInst()->BandwidthSetIfaceQuota(iface, quotaRemain);
        }
    }
    // remove the iface quota that not metered.
    for (uint32_t i = 0; i < meteredIfaces_.size(); ++i) {
        if (!std::count(newMeteredIfaces.begin(), newMeteredIfaces.end(), meteredIfaces_[i])) {
            GetNetsysInst()->BandwidthRemoveIfaceQuota(meteredIfaces_[i]);
        }
    }
    return newMeteredIfaces;
}

void NetPolicyTraffic::UpdateMeteredIfaces(std::vector<std::string> &newMeteredIfaces)
{
    NETMGR_LOG_I("UpdateMeteredIfaces size[%{public}zu]", newMeteredIfaces.size());
    meteredIfaces_.clear();
    meteredIfaces_.reserve(newMeteredIfaces.size());
    for (auto &iface : newMeteredIfaces) {
        meteredIfaces_.push_back(iface);
    }
    // notify the callback of metered ifaces changed.
    GetCbInst()->NotifyNetMeteredIfacesChange(meteredIfaces_);
}

void NetPolicyTraffic::UpdateQuotaNotify()
{
    NETMGR_LOG_I("UpdateQuotaNotify");
    NetmanagerHiTrace::NetmanagerStartSyncTrace("Traverse cellular network start");
    for (auto &quotaPolicy : quotaPolicies_) {
        NetmanagerHiTrace::NetmanagerStartSyncTrace("Get the start time of the metering cycle start");
        int64_t start = quotaPolicy.GetPeriodStart();
        NetmanagerHiTrace::NetmanagerFinishSyncTrace("Get the start time of the metering cycle end");

        NetmanagerHiTrace::NetmanagerStartSyncTrace("Get the usage of traffic start");
        int64_t totalQuota = GetTotalQuota(quotaPolicy);
        NetmanagerHiTrace::NetmanagerFinishSyncTrace("Get the usage of traffic end");
        // check if the quota is over the limit
        if (quotaPolicy.IsOverLimit(totalQuota)) {
            if (quotaPolicy.lastLimitRemind > start) {
                // notify the quota reach limit and has reminded before.
                NetmanagerHiTrace::NetmanagerStartSyncTrace("Notify quota limit reminded start");
                NotifyQuotaLimitReminded(totalQuota);
                NetmanagerHiTrace::NetmanagerFinishSyncTrace("Notify quota limit reminded end");
                continue;
            }
            NetmanagerHiTrace::NetmanagerStartSyncTrace("Update net enable status start");
            UpdateNetEnableStatus(quotaPolicy);
            NetmanagerHiTrace::NetmanagerFinishSyncTrace("Update net enable status end");
            // notify the quota reach limit
            NotifyQuotaLimit(totalQuota);
            continue;
        }
        // check if the quota is over the warning
        if (quotaPolicy.IsOverWarning(totalQuota) && quotaPolicy.lastWarningRemind < start) {
            NetmanagerHiTrace::NetmanagerStartSyncTrace("Notify quota warning remind start");
            NotifyQuotaWarning(totalQuota);
            NetmanagerHiTrace::NetmanagerFinishSyncTrace("Notify quota warning remind end");
        }
    }
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("Traverse cellular network end");
}

int64_t NetPolicyTraffic::GetQuotaRemain(NetQuotaPolicy &quotaPolicy)
{
    int64_t start = quotaPolicy.GetPeriodStart();
    int64_t totalQuota = GetTotalQuota(quotaPolicy);
    NETMGR_LOG_D("GetQuotaRemain totalQuota[%{public}s] limit[%{public}s] start[%{public}s]", std::to_string(totalQuota).c_str(),
                 std::to_string(quotaPolicy.limitBytes).c_str(), ctime(&start));
    // calculate the quota for each policy.
    bool hasLimit = quotaPolicy.limitBytes != DATA_USAGE_UNKNOWN;
    int64_t quota = LONG_MAX;
    if (hasLimit || quotaPolicy.metered) {
        if (hasLimit && quotaPolicy.periodDuration != QUOTA_POLICY_NO_PERIOD) {
            if (quotaPolicy.lastLimitRemind >= start) {
                return LONG_MAX;
            }
            quota = quotaPolicy.limitBytes - totalQuota;
        }
    }
    return quota < 0 ? 0 : quota;
}

void NetPolicyTraffic::UpdateNetEnableStatus(const NetQuotaPolicy &quotaPolicy)
{
    NETMGR_LOG_I("UpdateNetEnableStatus metered[%{public}d] limitAction[%{public}d]", quotaPolicy.metered,
                 quotaPolicy.limitAction);
    if (quotaPolicy.metered || quotaPolicy.limitAction == LIMIT_ACTION_DISABLE) {
        SetNetworkEnableStatus(quotaPolicy, false);
    }
}

int32_t NetPolicyTraffic::GetNetQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    quotaPolicies.clear();
    quotaPolicies.reserve(quotaPolicies_.size());
    for (uint32_t i = 0; i < quotaPolicies_.size(); ++i) {
        quotaPolicies.push_back(quotaPolicies_[i]);
    }
    NETMGR_LOG_D("GetNetQuotaPolicies quotaPolicies end size[%{public}zu]", quotaPolicies.size());
    return ERR_NONE;
}

int32_t NetPolicyTraffic::UpdateRemindPolicy(int32_t netType, const std::string &iccid, uint32_t remindType)
{
    if (!IsValidNetType(static_cast<NetBearType>(netType))) {
        NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", netType);
        return ERR_INVALID_PARAM;
    }
    for (uint32_t i = 0; i < quotaPolicies_.size(); ++i) {
        NetQuotaPolicy &quotaPolicy = quotaPolicies_[i];
        int32_t netTypeTemp = quotaPolicy.netType;
        std::string iccidTemp = quotaPolicy.iccid;
        if (netTypeTemp == netType && iccidTemp == iccid) {
            switch (remindType) {
                case REMIND_TYPE_WARNING:
                    quotaPolicy.lastWarningRemind = time(nullptr);
                    break;
                case REMIND_TYPE_LIMIT:
                    quotaPolicy.lastLimitRemind = time(nullptr);
                    break;
                default:
                    return ERR_INVALID_PARAM;
            }
        }
    }

    UpdateQuotaPoliciesInner();

    return ERR_NONE;
}

const std::vector<std::string> &NetPolicyTraffic::GetMeteredIfaces()
{
    return meteredIfaces_;
}

void NetPolicyTraffic::ResetPolicies(const std::string &iccid)
{
    for (auto &quotaPolicy : quotaPolicies_) {
        if (quotaPolicy.iccid == iccid) {
            quotaPolicy.Reset();
        }
    }
    UpdateQuotaPoliciesInner();
}

void NetPolicyTraffic::ReachedLimit(const std::string &iface)
{
    NETMGR_LOG_I("ReachedLimit iface:%{public}s.", iface.c_str());
    auto &ifaces = GetMeteredIfaces();
    if (std::find(ifaces.begin(), ifaces.end(), iface) != ifaces.end()) {
        UpdateQuotaPoliciesInner();
    }
}

int64_t NetPolicyTraffic::GetTotalQuota(NetQuotaPolicy &quotaPolicy)
{
    std::string iface = GetMatchIfaces(quotaPolicy);
    NetStatsInfo info;
    int64_t start = quotaPolicy.GetPeriodStart();
    int64_t end = static_cast<int64_t>(time(nullptr));
    int64_t quota = GetNetCenterInst().GetIfaceStatsDetail(iface, start, end, info);

    return quota < 0 ? 0 : quota;
}

int32_t NetPolicyTraffic::ReadQuotaPolicies()
{
    GetFileInst()->ReadQuotaPolicies(quotaPolicies_);
    UpdateQuotaPoliciesInner();
    return 0;
}

bool NetPolicyTraffic::WriteQuotaPolicies()
{
    if (GetFileInst()->WriteFile(quotaPolicies_)) {
        return true;
    }
    return false;
}

const std::string NetPolicyTraffic::GetMatchIfaces(const NetQuotaPolicy &quotaPolicy)
{
    std::string ident = "";
    if (quotaPolicy.netType == BEARER_CELLULAR) {
        ident = IDENT_PREFIX_CELLULAR + quotaPolicy.iccid;
    } else if (quotaPolicy.netType == BEARER_WIFI) {
        ident = quotaPolicy.ident;
    } else if (quotaPolicy.netType == BEARER_ETHERNET) {
        ident = quotaPolicy.ident;
    }
    std::string iface;
    GetNetCenterInst().GetIfaceNameByType(static_cast<NetBearType>(quotaPolicy.netType), ident, iface);
    NETMGR_LOG_D("GetMatchIfaces netType: %{public}d ident: %{public}s iface: %{public}s.", quotaPolicy.netType,
                 ident.c_str(), iface.c_str());
    return iface;
}

void NetPolicyTraffic::SetNetworkEnableStatus(const NetQuotaPolicy &quotaPolicy, bool enable)
{
    NETMGR_LOG_D("SetNetworkEnableStatus enable: %{public}d ", enable);
}

void NetPolicyTraffic::NotifyQuotaWarning(int64_t totalQuota)
{
    PublishQuotaEvent(COMMON_EVENT_NET_QUOTA_WARNING, totalQuota);
}

void NetPolicyTraffic::NotifyQuotaLimitReminded(int64_t totalQuota)
{
    PublishQuotaEvent(COMMON_EVENT_NET_QUOTA_LIMIT_REMINDED, totalQuota);
}

void NetPolicyTraffic::NotifyQuotaLimit(int64_t totalQuota)
{
    PublishQuotaEvent(COMMON_EVENT_NET_QUOTA_LIMIT, totalQuota);
}

void NetPolicyTraffic::PublishQuotaEvent(const std::string &action, int64_t quota)
{
    EventFwk::CommonEventData data;
    AAFwk::Want want;
    want.SetParam("totalQuota", quota);
    want.SetAction(action);
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    if (!EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr)) {
        NETMGR_LOG_E("Publish %{public}s event fail.", action.c_str());
        return;
    }
    NETMGR_LOG_I("Publish %{public}s event.", action.c_str());
}

bool NetPolicyTraffic::IsValidPeriodDuration(const std::string &periodDuration)
{
    if (periodDuration.empty() || periodDuration.size() < 2) {
        NETMGR_LOG_E("periodDuration is empty");
        return false;
    }

    std::string cycle = periodDuration.substr(0, 1);
    NETMGR_LOG_E("Invalid periodDuration [%{public}s].", cycle.c_str());
    int32_t start =
        static_cast<int32_t>(std::strtol(periodDuration.substr(1, periodDuration.size()).c_str(), nullptr, 10));
    if (cycle == PERIOD_DAY) {
        if (start < PERIOD_START || start > DAY_MAX) {
            NETMGR_LOG_E("Invalid periodDuration D[%{public}d]", start);
            return false;
        }
    }

    if (cycle == PERIOD_MONTH) {
        if (start < PERIOD_START || start > MONTH_MAX) {
            NETMGR_LOG_E("Invalid periodDuration M[%{public}d]", start);
            return false;
        }
    }

    if (cycle == PERIOD_YEAR) {
        if (start < PERIOD_START || start > YEAR_MAX) {
            NETMGR_LOG_E("Invalid periodDuration Y[%{public}d]", start);
            return false;
        }
    }

    return true;
}

bool NetPolicyTraffic::IsQuotaPolicyExist(int32_t netType, const std::string &iccid)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    if (GetFileInst()->ReadQuotaPolicies(quotaPolicies) != ERR_NONE) {
        NETMGR_LOG_E("GetNetQuotaPolicies failed");
        return false;
    }

    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("quotaPolicies is empty");
        return false;
    }

    for (uint32_t i = 0; i < quotaPolicies.size(); i++) {
        if (netType == quotaPolicies[i].netType && iccid == quotaPolicies[i].iccid) {
            NETMGR_LOG_D("netQuotaPolicy exist");
            return true;
        }
    }

    return false;
}

void NetPolicyTraffic::HandleEvent(int32_t eventId, const std::shared_ptr<PolicyEvent> &policyEvent)
{
    NETMGR_LOG_D("NetPolicyTraffic HandleEvent");
}

void NetPolicyTraffic::GetDumpMessage(std::string &message)
{
    static const std::string TAB = "    ";
    message.append(TAB + "MeteredIfaces: {");
    std::for_each(meteredIfaces_.begin(), meteredIfaces_.end(),
                  [&message](const std::string &item) { message.append(item + ", "); });
    message.append("}\n");
    message.append(TAB + "QuotaPolicies:\n");
    std::for_each(quotaPolicies_.begin(), quotaPolicies_.end(), [&message](const auto &item) {
        message.append(TAB + TAB + "NetType: " + std::to_string(item.netType) + "\n" + TAB + TAB +
                       "IccId: " + item.iccid + "\n" + TAB + TAB + "Ident: " + item.ident + "\n");
        message.append(TAB + TAB + "PeriodStartTime: " + std::to_string(item.periodStartTime) + "\n");
        message.append(TAB + TAB + "PeriodDuration: " + item.periodDuration + "\n");
        message.append(TAB + TAB + "Title: " + item.title + "\n" + TAB + TAB + "Summary: " + item.summary + "\n");
        message.append(TAB + TAB + "WarningBytes: " + std::to_string(item.warningBytes) + "\n");
        message.append(TAB + TAB + "LimitBytes: " + std::to_string(item.limitBytes) + "\n");
        message.append(TAB + TAB + "LastWarningRemind: " + std::to_string(item.lastWarningRemind) + "\n");
        message.append(TAB + TAB + "LastLimitRemind: " + std::to_string(item.lastLimitRemind) + "\n");
        message.append(TAB + TAB + "Metered: " + std::to_string(item.metered) + "\n" + TAB + TAB +
                       "Source: " + std::to_string(item.source) + "\n");
        message.append(TAB + TAB + "LimitAction: " + std::to_string(item.limitAction) + "\n" + TAB + TAB +
                       "UsedBytes: " + std::to_string(item.usedBytes) + "\n");
        message.append(TAB + TAB + "UsedTimeDuration: " + std::to_string(item.usedTimeDuration) + "\n");
        message.append(TAB + TAB + "Possessor: " + item.possessor + "\n\n");
    });
}
} // namespace NetManagerStandard
} // namespace OHOS