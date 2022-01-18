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
#include "net_policy_service.h"

#include <sys/time.h>
#include <unistd.h>
#include <cinttypes>

#include "system_ability_definition.h"

#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_policy_file.h"
#include "net_policy_quota_policy.h"
#include "net_policy_traffic.h"

#include "net_mgr_log_wrapper.h"
#include "net_settings.h"
#include "net_manager_center.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetPolicyService>::GetInstance().get());

NetPolicyService::NetPolicyService()
    : SystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    netPolicyFile_ = (std::make_unique<NetPolicyFile>()).release();
    netPolicyTraffic_ = (std::make_unique<NetPolicyTraffic>(netPolicyFile_)).release();
    netPolicyFirewall_ = (std::make_unique<NetPolicyFirewall>(netPolicyFile_)).release();
    netPolicyCallback_ = (std::make_unique<NetPolicyCallback>()).release();
    monthDay_ = {MONTH_THIRTY_ONE, MONTH_TWENTY_EIGHT, MONTH_THIRTY_ONE, MONTH_THIRTY,
        MONTH_THIRTY_ONE, MONTH_THIRTY, MONTH_THIRTY_ONE, MONTH_THIRTY_ONE, MONTH_THIRTY,
        MONTH_THIRTY_ONE, MONTH_THIRTY, MONTH_THIRTY_ONE};
}

NetPolicyService::~NetPolicyService() {}

void NetPolicyService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    NETMGR_LOG_D("NetPolicyService::OnStart begin timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
    if (state_ == STATE_RUNNING) {
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("init failed");
        return;
    }
    state_ = STATE_RUNNING;
    gettimeofday(&tv, NULL);
    NETMGR_LOG_D("NetPolicyService::OnStart end timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
}

void NetPolicyService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool NetPolicyService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOG_E("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetPolicyService>::GetInstance().get())) {
            NETMGR_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }

    bool error = netPolicyFile_->InitPolicy();
    if (!error) {
        NETMGR_LOG_E("InitPolicyTraffic failed");
        return false;
    }
    serviceComm_ = (std::make_unique<NetPolicyServiceCommon>()).release();
    NetManagerCenter::GetInstance().RegisterPolicyService(serviceComm_);
    return true;
}

NetPolicyResultCode NetPolicyService::SetPolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NetPolicyResultCode ret = NetPolicyResultCode::ERR_INTERNAL_ERROR;
    NETMGR_LOG_I("SetPolicyByUid info: uid[%{public}d] policy[%{public}d]", uid, static_cast<uint32_t>(policy));
    /* delete uid policy */
    if (policy == NetUidPolicy::NET_POLICY_NONE) {
        ret = netPolicyTraffic_->DeletePolicyByUid(uid, policy);
        lock.unlock();
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
        return ret;
    }

    /* update policy */
    if (!netPolicyFile_->IsUidPolicyExist(uid)) {
        ret = netPolicyTraffic_->AddPolicyByUid(uid, policy);
        lock.unlock();
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
    } else {
        ret = netPolicyTraffic_->SetPolicyByUid(uid, policy);
        lock.unlock();
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
    }

    return ret;
}

NetUidPolicy NetPolicyService::GetPolicyByUid(uint32_t uid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetPolicyByUid info: uid[%{public}d]", uid);
    return netPolicyFile_->GetPolicyByUid(uid);
}

std::vector<uint32_t> NetPolicyService::GetUidsByPolicy(NetUidPolicy policy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetUidsByPolicy info: policy[%{public}d]", static_cast<uint32_t>(policy));
    std::vector<uint32_t> uids;
    if (!netPolicyFile_->GetUidsByPolicy(policy, uids)) {
        NETMGR_LOG_E("GetUidsByPolicy  failed");
    };

    return uids;
}

bool NetPolicyService::IsUidNetAccess(uint32_t uid, bool metered)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("IsUidNetAccess info: uid[%{public}d]", uid);

    if (NetSettings::GetInstance().IsSystem(uid)) {
        return true;
    }

    NetUidPolicy uidPolicy = netPolicyFile_->GetPolicyByUid(uid);
    if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_ALL)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_ALL)) {
        return false;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL)) {
        return true;
    }

    if (!metered) {
        return true;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED)) {
        return false;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED) ||
        (static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED)) {
        return true;
    }

    if (NetSettings::GetInstance().IsUidForeground(uid)) {
        return true;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND)) {
        return false;
    } else if (netPolicyFile_->GetBackgroundPolicy()) {
        return true;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND)) {
        return true;
    }

    return false;
}

bool NetPolicyService::IsUidNetAccess(uint32_t uid, const std::string &ifaceName)
{
    bool metered = netPolicyFile_->IsInterfaceMetered(ifaceName);

    return IsUidNetAccess(uid, metered);
}

int32_t NetPolicyService::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback == nullptr) {
        NETMGR_LOG_E("RegisterNetPolicyCallback parameter callback is null");
        return static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR);
    }

    netPolicyCallback_->RegisterNetPolicyCallback(callback);

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

int32_t NetPolicyService::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback == nullptr) {
        NETMGR_LOG_E("UnregisterNetPolicyCallback parameter callback is null");
        return static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR);
    }

    netPolicyCallback_->UnregisterNetPolicyCallback(callback);

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

std::int64_t NetPolicyService::GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec;
}

int32_t NetPolicyService::GetPeriodEndTime()
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

void NetPolicyService::CheckNetStatsOverLimit(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    if (cellularPolicies.empty()) {
        NETMGR_LOG_W("cellularPolicies size is 0.");
        return;
    }

    int32_t periodEndTime = GetPeriodEndTime();
    if (periodEndTime <= 0) {
        NETMGR_LOG_E("periodEndTime error.");
        return;
    }

    std::string ifaceName;
    for (uint32_t i = 0; i < cellularPolicies.size(); ++i) {
        /* -1 : unlimited */
        if (cellularPolicies[i].limitBytes_ == -1) {
            if (netPolicyCallback_ != nullptr) {
                netPolicyCallback_->NotifyNetStrategySwitch(cellularPolicies[i].slotId_, true);
            }
            continue;
        }
        std::string slotId = IDENT_PREFIX + std::to_string(cellularPolicies[i].slotId_);
        int32_t ret = NetManagerCenter::GetInstance().GetIfaceNameByType(BEARER_CELLULAR,
            slotId, ifaceName);
        if (ret != 0 || ifaceName.empty()) {
            NETMGR_LOG_E("GetIfaceNameByType ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NetStatsInfo netStatsInfo;
        ret = NetManagerCenter::GetInstance().GetIfaceStatsDetail(ifaceName, cellularPolicies[i].periodStartTime_,
            cellularPolicies[i].periodStartTime_ + periodEndTime, netStatsInfo);
        if (ret != 0) {
            NETMGR_LOG_E("GetIfaceStatsDetail ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NETMGR_LOG_I("GetIfaceStatsDetail txBytes_[%{public}" PRId64 "] rxBytes_[%{public}" PRId64 "]",
            netStatsInfo.txBytes_, netStatsInfo.rxBytes_);
        /*  The traffic exceeds the limit. You need to notify telephony to shut down the network. */
        if (netStatsInfo.txBytes_ + netStatsInfo.rxBytes_ < cellularPolicies[i].limitBytes_) {
            if (netPolicyCallback_ != nullptr) {
                netPolicyCallback_->NotifyNetStrategySwitch(cellularPolicies[i].slotId_, true);
            }
        } else {
            if (netPolicyCallback_ != nullptr) {
                netPolicyCallback_->NotifyNetStrategySwitch(cellularPolicies[i].slotId_, false);
            }
        }
    }
}

void NetPolicyService::CheckNetStatsOverLimit(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    if (quotaPolicies.empty()) {
        NETMGR_LOG_W("quotaPolicies size is 0.");
        return;
    }

    int32_t periodEndTime = GetPeriodEndTime();
    if (periodEndTime <= 0) {
        NETMGR_LOG_E("periodEndTime error.");
        return;
    }

    std::string ifaceName;
    for (uint32_t i = 0; i < quotaPolicies.size(); ++i) {
        /* only control cellular traffic */
        if (static_cast<NetBearType>(quotaPolicies[i].netType_) != BEARER_CELLULAR) {
            NETMGR_LOG_I("need not notify telephony netType_[%{public}d]", quotaPolicies[i].netType_);
            continue;
        }
        NetBearType bearerType = static_cast<NetBearType>(quotaPolicies[i].netType_);
        std::string slotId = IDENT_PREFIX + std::to_string(quotaPolicies[i].slotId_);
        int32_t ret = NetManagerCenter::GetInstance().GetIfaceNameByType(
            bearerType, slotId, ifaceName);
        if (ret != 0 || ifaceName.empty()) {
            NETMGR_LOG_E("GetIfaceNameByType ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NetStatsInfo netStatsInfo;
        ret = NetManagerCenter::GetInstance().GetIfaceStatsDetail(ifaceName, quotaPolicies[i].periodStartTime_,
            quotaPolicies[i].periodStartTime_ + periodEndTime, netStatsInfo);
        if (ret != 0) {
            NETMGR_LOG_E("GetIfaceStatsDetail ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NETMGR_LOG_I("GetIfaceStatsDetail txBytes_[%{public}" PRId64 "] rxBytes_[%{public}" PRId64 "]",
            netStatsInfo.txBytes_, netStatsInfo.rxBytes_);

        /* Sleep time is not up Or nerverSnooze : lastLimitSnooze_=1 */
        if ((quotaPolicies[i].lastLimitSnooze_ >= quotaPolicies[i].periodStartTime_ ||
            quotaPolicies[i].lastLimitSnooze_ == -1)
            && (netStatsInfo.txBytes_ + netStatsInfo.rxBytes_ < quotaPolicies[i].limitBytes_)) {
            if (netPolicyCallback_ != nullptr) {
                netPolicyCallback_->NotifyNetStrategySwitch(quotaPolicies[i].slotId_, true);
            }
        } else {
            if (netPolicyCallback_ != nullptr) {
                netPolicyCallback_->NotifyNetStrategySwitch(quotaPolicies[i].slotId_, false);
            }
        }
    }
}

NetPolicyResultCode NetPolicyService::SetNetQuotaPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    NETMGR_LOG_I("SetNetQuotaPolicies quotaPolicySize[%{public}zd]", quotaPolicies.size());
    if (quotaPolicies.empty()) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    NetPolicyResultCode ret = netPolicyTraffic_->SetNetQuotaPolicies(quotaPolicies);
    lock.unlock();
    if (ret == NetPolicyResultCode::ERR_NONE) {
        /* Judge whether the flow exceeds the limit */
        CheckNetStatsOverLimit(quotaPolicies);
    }

    return ret;
}

NetPolicyResultCode NetPolicyService::GetNetQuotaPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetNetQuotaPolicies begin");
    return netPolicyFile_->GetNetQuotaPolicies(quotaPolicies);
}

NetPolicyResultCode NetPolicyService::SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    NETMGR_LOG_I("SetCellularPolicies cellularPolicies[%{public}zd]", cellularPolicies.size());
    if (cellularPolicies.empty()) {
        NETMGR_LOG_E("cellularPolicies size 0");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    NetPolicyResultCode ret = netPolicyTraffic_->SetCellularPolicies(cellularPolicies);
    lock.unlock();
    if (ret == NetPolicyResultCode::ERR_NONE) {
        /* Judge whether the flow exceeds the limit */
        CheckNetStatsOverLimit(cellularPolicies);
        netPolicyCallback_->NotifyNetCellularPolicyChanged(cellularPolicies);
    }

    return ret;
}

NetPolicyResultCode NetPolicyService::GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetCellularPolicies begin");
    return netPolicyFile_->GetCellularPolicies(cellularPolicies);
}

NetPolicyResultCode NetPolicyService::SetFactoryPolicy(const std::string &slotId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetFactoryPolicy begin");
    netPolicyTraffic_->ClearIdleTrustList();
    return netPolicyFile_->SetFactoryPolicy(slotId);
}

NetPolicyResultCode NetPolicyService::SetBackgroundPolicy(bool backgroundPolicy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetBackgroundPolicy begin");

    bool oldBackgroundPolicy = netPolicyFile_->GetBackgroundPolicy();
    NetPolicyResultCode ret = netPolicyFile_->SetBackgroundPolicy(backgroundPolicy);
    if (ret != NetPolicyResultCode::ERR_NONE) {
        return ret;
    }

    bool newBackgroundPolicy = netPolicyFile_->GetBackgroundPolicy();
    lock.unlock();
    if (newBackgroundPolicy != oldBackgroundPolicy) {
        netPolicyCallback_->NotifyNetBackgroundPolicyChanged(newBackgroundPolicy);
        NetManagerCenter::GetInstance().RestrictBackgroundChanged(newBackgroundPolicy);
    }

    return ret;
}

bool NetPolicyService::GetBackgroundPolicy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetBackgroundPolicy begin");
    return netPolicyFile_->GetBackgroundPolicy();
}

bool NetPolicyService::GetBackgroundPolicyByUid(uint32_t uid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetBackgroundPolicyByUid begin");
    return netPolicyFirewall_->GetBackgroundPolicyByUid(uid);
}

NetBackgroundPolicy NetPolicyService::GetCurrentBackgroundPolicy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetCurrentBackgroundPolicy begin");
    return netPolicyFirewall_->GetCurrentBackgroundPolicy();
}

NetPolicyResultCode NetPolicyService::SetSnoozePolicy(int8_t netType, int32_t slotId)
{
    NETMGR_LOG_I("SetSnoozePolicy begin");

    NetPolicyQuotaPolicy quotaPolicy;
    std::unique_lock<std::mutex> lock(mutex_);
    NetPolicyResultCode ret = netPolicyFile_->GetNetQuotaPolicy(netType, slotId, quotaPolicy);
    if (NetPolicyResultCode::ERR_NONE != ret) {
        NETMGR_LOG_E("SetSnoozePolicy GetQuotaPolicy failed");
        return ret;
    }
    /* Set the sleep time to the current time. */
    quotaPolicy.lastLimitSnooze_ = GetCurrentTime();
    std::vector<NetPolicyQuotaPolicy> quotaPolicies = {quotaPolicy};
    ret = netPolicyTraffic_->SetSnoozePolicy(netType, slotId, quotaPolicies);
    lock.unlock();
    if (ret == NetPolicyResultCode::ERR_NONE) {
        /* Judge whether the flow exceeds the limit */
        CheckNetStatsOverLimit(quotaPolicies);
    }

    return ret;
}

NetPolicyResultCode NetPolicyService::SetIdleTrustlist(uint32_t uid, bool isTrustlist)
{
    NETMGR_LOG_I("SetIdleTrustlist info: uid[%{public}d] isTrustlist[%{public}d]", uid,
        static_cast<uint32_t>(isTrustlist));

    std::unique_lock<std::mutex> lock(mutex_);
    return netPolicyTraffic_->SetIdleTrustlist(uid, isTrustlist);
}

NetPolicyResultCode NetPolicyService::GetIdleTrustlist(std::vector<uint32_t> &uids)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetIdleTrustlist begin");
    return netPolicyTraffic_->GetIdleTrustlist(uids);
}
} // namespace NetManagerStandard
} // namespace OHOS
