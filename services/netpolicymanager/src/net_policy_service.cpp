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

    return true;
}

NetPolicyResultCode NetPolicyService::SetUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NetPolicyResultCode ret = NetPolicyResultCode::ERR_INTERNAL_ERROR;
    NETMGR_LOG_I("SetUidPolicy info: uid[%{public}d] policy[%{public}d]", uid, static_cast<uint32_t>(policy));
    /* delete uid policy */
    if (policy == NetUidPolicy::NET_POLICY_NONE) {
        ret = netPolicyTraffic_->DeleteUidPolicy(uid, policy);
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
        return ret;
    }

    /* update policy */
    if (!netPolicyFile_->IsUidPolicyExist(uid)) {
        ret = netPolicyTraffic_->AddUidPolicy(uid, policy);
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
    } else {
        ret = netPolicyTraffic_->SetUidPolicy(uid, policy);
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
    }

    return ret;
}

NetUidPolicy NetPolicyService::GetUidPolicy(uint32_t uid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetUidPolicy info: uid[%{public}d]", uid);
    return netPolicyFile_->GetUidPolicy(uid);
}

std::vector<uint32_t> NetPolicyService::GetUids(NetUidPolicy policy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetUids info: policy[%{public}d]", static_cast<uint32_t>(policy));
    std::vector<uint32_t> uids;
    if (!netPolicyFile_->GetUids(policy, uids)) {
        NETMGR_LOG_E("GetUids  failed");
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

    NetUidPolicy uidPolicy = netPolicyFile_->GetUidPolicy(uid);
    if (static_cast<uint32_t>(uidPolicy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_ALL)) {
        return false;
    } else if (static_cast<uint32_t>(uidPolicy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL)) {
        return true;
    }

    if (!metered) {
        return true;
    } else if (static_cast<uint32_t>(uidPolicy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED)) {
        return false;
    } else if ((static_cast<uint32_t>(uidPolicy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED)) ||
        (static_cast<uint32_t>(uidPolicy) & static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED))) {
        return true;
    }

    if (netPolicyFile_->GetBackgroundPolicy() || NetSettings::GetInstance().IsUidForeground(uid)) {
        return true;
    } else if (static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND)) {
        return false;
    } else if (static_cast<uint32_t>(uidPolicy) &
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

NetPolicyResultCode NetPolicyService::SetNetPolicys(const std::vector<NetPolicyQuotaPolicy> &quotaPolicys)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetNetPolicys quotaPolicySize[%{public}d]", static_cast<uint32_t>(quotaPolicys.size()));
    if (quotaPolicys.empty()) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return netPolicyTraffic_->SetNetPolicys(quotaPolicys, netPolicyCallback_);
}

NetPolicyResultCode NetPolicyService::GetNetPolicys(std::vector<NetPolicyQuotaPolicy> &quotaPolicys)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetNetPolicys begin");
    return netPolicyFile_->GetNetPolicys(quotaPolicys);
}

NetPolicyResultCode NetPolicyService::SetCellularPolicys(const std::vector<NetPolicyCellularPolicy> &cellularPolicys)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetCellularPolicys cellularPolicys[%{public}d]", static_cast<uint32_t>(cellularPolicys.size()));
    if (cellularPolicys.empty()) {
        NETMGR_LOG_E("cellularPolicys size 0");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    NetPolicyResultCode ret = netPolicyTraffic_->SetCellularPolicys(cellularPolicys, netPolicyCallback_);
    if (ret == NetPolicyResultCode::ERR_NONE) {
        netPolicyCallback_->NotifyNetCellularPolicyChanged(cellularPolicys);
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyService::GetCellularPolicys(std::vector<NetPolicyCellularPolicy> &cellularPolicys)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetCellularPolicys begin");
    return netPolicyFile_->GetCellularPolicys(cellularPolicys);
}

NetPolicyResultCode NetPolicyService::ResetFactory(const std::string &subscriberId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("ResetFactory begin");
    netPolicyTraffic_->ClearIdleWhiteList();
    return netPolicyFile_->ResetFactory(subscriberId);
}

NetPolicyResultCode NetPolicyService::SetBackgroundPolicy(bool backgroundPolicy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetBackgroundPolicy begin");
    return netPolicyFile_->SetBackgroundPolicy(backgroundPolicy);
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

bool NetPolicyService::GetCurrentBackgroundPolicy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetCurrentBackgroundPolicy begin");
    return netPolicyFirewall_->GetCurrentBackgroundPolicy();
}

NetPolicyResultCode NetPolicyService::SnoozePolicy(const NetPolicyQuotaPolicy &quotaPolicy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SnoozePolicy begin");

    return netPolicyTraffic_->SnoozePolicy(quotaPolicy, netPolicyCallback_);
}

NetPolicyResultCode NetPolicyService::SetIdleWhitelist(uint32_t uid, bool isWhiteList)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetIdleWhitelist info: uid[%{public}d] isWhiteList[%{public}d]", uid,
        static_cast<uint32_t>(isWhiteList));

    return netPolicyTraffic_->SetIdleWhitelist(uid, isWhiteList);
}

NetPolicyResultCode NetPolicyService::GetIdleWhitelist(std::vector<uint32_t> &uids)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetIdleWhitelist begin");
    return netPolicyTraffic_->GetIdleWhitelist(uids);
}
} // namespace NetManagerStandard
} // namespace OHOS
