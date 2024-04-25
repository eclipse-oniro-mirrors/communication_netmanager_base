/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <algorithm>

#include "system_ability_definition.h"

#include "bundle_constants.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_base.h"
#include "net_policy_constants.h"
#include "net_policy_core.h"
#include "net_policy_file.h"
#include "net_policy_inner_define.h"
#include "net_quota_policy.h"
#include "net_settings.h"
#include "netmanager_base_permission.h"
#include "system_ability_definition.h"
#include "net_policy_listener.h"
#include "net_access_policy_dialog.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<bool> g_RegisterToService(
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetPolicyService>::GetInstance().get()));

NetPolicyService::NetPolicyService()
    : SystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, true), state_(STATE_STOPPED)
{
}

NetPolicyService::~NetPolicyService() = default;

void NetPolicyService::OnStart()
{
    NETMGR_LOG_I("OnStart");
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_W("NetPolicyService already start.");
        return;
    }

    if (!g_RegisterToService) {
        g_RegisterToService =
            SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetPolicyService>::GetInstance().get());
        if (!g_RegisterToService) {
            NETMGR_LOG_E("Register to local sa manager failed again, give up.");
            return;
        }
    }
    if (!Publish(DelayedSingleton<NetPolicyService>::GetInstance().get())) {
        NETMGR_LOG_E("Register to sa manager failed");
        return;
    }

    state_ = STATE_RUNNING;
    Init();
}

void NetPolicyService::OnStop()
{
    runner_->Stop();
    handler_.reset();
    runner_.reset();
    netPolicyCore_.reset();
    netPolicyCallback_.reset();
    netPolicyTraffic_.reset();
    netPolicyFirewall_.reset();
    netPolicyRule_.reset();
    state_ = STATE_STOPPED;
    g_RegisterToService = false;
}

int32_t NetPolicyService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_LOG_D("Start policy Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? NETMANAGER_ERR_PARAMETER_ERROR : NETMANAGER_SUCCESS;
}

void NetPolicyService::Init()
{
    NETMGR_LOG_D("Init");
    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    handler_->PostTask(
        [this]() {
            serviceComm_ = (std::make_unique<NetPolicyServiceCommon>()).release();
            NetManagerCenter::GetInstance().RegisterPolicyService(serviceComm_);
            netPolicyCore_ = DelayedSingleton<NetPolicyCore>::GetInstance();
            netPolicyCallback_ = DelayedSingleton<NetPolicyCallback>::GetInstance();
            netPolicyTraffic_ = netPolicyCore_->CreateCore<NetPolicyTraffic>();
            netPolicyFirewall_ = netPolicyCore_->CreateCore<NetPolicyFirewall>();
            netPolicyRule_ = netPolicyCore_->CreateCore<NetPolicyRule>();
            RegisterFactoryResetCallback();
            netAccessPolicy_.InitRdbStore();
            UpdateNetAccessPolicyToMapFromDB();
        },
        AppExecFwk::EventQueue::Priority::HIGH);
}

int32_t NetPolicyService::SetPolicyByUid(uint32_t uid, uint32_t policy)
{
    NETMGR_LOG_I("SetPolicyByUid uid[%{public}d] policy[%{public}d]", uid, policy);
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyRule_->TransPolicyToRule(uid, policy);
}

int32_t NetPolicyService::GetPolicyByUid(uint32_t uid, uint32_t &policy)
{
    NETMGR_LOG_D("GetPolicyByUid uid[%{public}d]", uid);
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyRule_->GetPolicyByUid(uid, policy);
}

int32_t NetPolicyService::GetUidsByPolicy(uint32_t policy, std::vector<uint32_t> &uids)
{
    NETMGR_LOG_D("GetUidsByPolicy policy[%{public}d]", policy);
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyRule_->GetUidsByPolicy(policy, uids);
}

int32_t NetPolicyService::IsUidNetAllowed(uint32_t uid, bool metered, bool &isAllowed)
{
    NETMGR_LOG_I("IsUidNetAllowed uid[%{public}d metered[%{public}d]", uid, metered);
    if (NetSettings::GetInstance().IsSystem(uid)) {
        isAllowed = true;
        return NETMANAGER_SUCCESS;
    }
    if (netPolicyRule_ != nullptr) {
        return netPolicyRule_->IsUidNetAllowed(uid, metered, isAllowed);
    }
    return NETMANAGER_ERR_LOCAL_PTR_NULL;
}

int32_t NetPolicyService::IsUidNetAllowed(uint32_t uid, const std::string &ifaceName, bool &isAllowed)
{
    NETMGR_LOG_D("IsUidNetAllowed uid[%{public}d ifaceName[%{public}s]", uid, ifaceName.c_str());
    const auto &vec = netPolicyTraffic_->GetMeteredIfaces();
    if (std::find(vec.begin(), vec.end(), ifaceName) != vec.end()) {
        return IsUidNetAllowed(uid, true, isAllowed);
    }
    return IsUidNetAllowed(uid, false, isAllowed);
}

int32_t NetPolicyService::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    NETMGR_LOG_I("RegisterNetPolicyCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("RegisterNetPolicyCallback parameter callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    if (netPolicyCallback_ == nullptr) {
        NETMGR_LOG_E("netPolicyCallback_ is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    return netPolicyCallback_->RegisterNetPolicyCallbackAsync(callback);
}

int32_t NetPolicyService::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    NETMGR_LOG_I("UnregisterNetPolicyCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("UnregisterNetPolicyCallback parameter callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    if (netPolicyCallback_ == nullptr) {
        NETMGR_LOG_E("netPolicyCallback_ is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyCallback_->UnregisterNetPolicyCallbackAsync(callback);
}

int32_t NetPolicyService::SetNetQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    NETMGR_LOG_I("SetNetQuotaPolicies quotaPolicySize[%{public}zd]", quotaPolicies.size());
    if (netPolicyTraffic_ == nullptr) {
        NETMGR_LOG_E("netPolicyTraffic_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyTraffic_->UpdateQuotaPolicies(quotaPolicies);
}

int32_t NetPolicyService::GetNetQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    NETMGR_LOG_D("GetNetQuotaPolicies begin");
    if (netPolicyTraffic_ == nullptr) {
        NETMGR_LOG_E("netPolicyTraffic_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyTraffic_->GetNetQuotaPolicies(quotaPolicies);
}

int32_t NetPolicyService::ResetPolicies(const std::string &simId)
{
    NETMGR_LOG_I("ResetPolicies begin");
    if (netPolicyRule_ != nullptr && netPolicyFirewall_ != nullptr && netPolicyTraffic_ != nullptr) {
        netPolicyRule_->ResetPolicies();
        netPolicyFirewall_->ResetPolicies();
        netPolicyTraffic_->ResetPolicies(simId);
        NETMGR_LOG_I("ResetPolicies end.");
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_ERR_LOCAL_PTR_NULL;
}

int32_t NetPolicyService::SetBackgroundPolicy(bool allow)
{
    NETMGR_LOG_I("SetBackgroundPolicy allow[%{public}d]", allow);
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyRule_->SetBackgroundPolicy(allow);
}

int32_t NetPolicyService::GetBackgroundPolicy(bool &backgroundPolicy)
{
    NETMGR_LOG_D("GetBackgroundPolicy begin");
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyRule_->GetBackgroundPolicy(backgroundPolicy);
}

int32_t NetPolicyService::GetBackgroundPolicyByUid(uint32_t uid, uint32_t &backgroundPolicyOfUid)
{
    NETMGR_LOG_D("GetBackgroundPolicyByUid uid[%{public}d]", uid);
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyRule_->GetBackgroundPolicyByUid(uid, backgroundPolicyOfUid);
}

int32_t NetPolicyService::UpdateRemindPolicy(int32_t netType, const std::string &simId, uint32_t remindType)
{
    NETMGR_LOG_I("UpdateRemindPolicy start");
    if (netPolicyTraffic_ == nullptr) {
        NETMGR_LOG_E("netPolicyTraffic_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyTraffic_->UpdateRemindPolicy(netType, simId, remindType);
}

int32_t NetPolicyService::SetDeviceIdleTrustlist(const std::vector<uint32_t> &uids, bool isAllowed)
{
    NETMGR_LOG_I("SetDeviceIdleTrustlist start");
    if (netPolicyFirewall_ == nullptr) {
        NETMGR_LOG_E("netPolicyFirewall_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyFirewall_->SetDeviceIdleTrustlist(uids, isAllowed);
}

int32_t NetPolicyService::GetDeviceIdleTrustlist(std::vector<uint32_t> &uids)
{
    NETMGR_LOG_D("GetDeviceIdleTrustlist start");
    if (netPolicyFirewall_ == nullptr) {
        NETMGR_LOG_E("netPolicyFirewall_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyFirewall_->GetDeviceIdleTrustlist(uids);
}

int32_t NetPolicyService::SetDeviceIdlePolicy(bool enable)
{
    NETMGR_LOG_I("SetDeviceIdlePolicy enable[%{public}d]", enable);
    if (netPolicyFirewall_ == nullptr) {
        NETMGR_LOG_E("netPolicyFirewall_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyFirewall_->UpdateDeviceIdlePolicy(enable);
}

int32_t NetPolicyService::GetPowerSaveTrustlist(std::vector<uint32_t> &uids)
{
    NETMGR_LOG_D("GetPowerSaveTrustlist start");
    if (netPolicyFirewall_ == nullptr) {
        NETMGR_LOG_E("netPolicyFirewall_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyFirewall_->GetPowerSaveTrustlist(uids);
}

int32_t NetPolicyService::SetPowerSaveTrustlist(const std::vector<uint32_t> &uids, bool isAllowed)
{
    NETMGR_LOG_I("SetPowerSaveTrustlist start");
    if (netPolicyFirewall_ == nullptr) {
        NETMGR_LOG_E("netPolicyFirewall_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyFirewall_->SetPowerSaveTrustlist(uids, isAllowed);
}

int32_t NetPolicyService::SetPowerSavePolicy(bool enable)
{
    NETMGR_LOG_I("SetPowerSavePolicy enable[%{public}d]", enable);
    if (netPolicyFirewall_ == nullptr) {
        NETMGR_LOG_E("netPolicyFirewall_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netPolicyFirewall_->UpdatePowerSavePolicy(enable);
}

int32_t NetPolicyService::GetDumpMessage(std::string &message)
{
    if (netPolicyRule_ == nullptr || netPolicyTraffic_ == nullptr) {
        NETMGR_LOG_E("netPolicyFirewall_ or netPolicyTraffic_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netPolicyRule_->GetDumpMessage(message);
    netPolicyTraffic_->GetDumpMessage(message);
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyService::CheckPermission()
{
    return NETMANAGER_SUCCESS;
}

void NetPolicyService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_LOG_I("OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSARemoved_) {
            OnNetSysRestart();
            hasSARemoved_ = false;
        }

        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(1);
        std::shared_ptr<NetPolicyListener> subscriber = std::make_shared<NetPolicyListener>(
            subscribeInfo, std::static_pointer_cast<NetPolicyService>(shared_from_this()));
        EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    }
}

void NetPolicyService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_LOG_I("OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSARemoved_ = true;
    }
}

void NetPolicyService::OnNetSysRestart()
{
    NETMGR_LOG_I("OnNetSysRestart");
    
    if (netPolicyRule_ != nullptr) {
        netPolicyRule_->TransPolicyToRule();
    }
}

int32_t NetPolicyService::FactoryResetPolicies()
{
    NETMGR_LOG_I("FactoryResetPolicies begin");
    if (netPolicyRule_ != nullptr && netPolicyFirewall_ != nullptr && netPolicyTraffic_ != nullptr) {
        netPolicyRule_->ResetPolicies();
        netPolicyFirewall_->ResetPolicies();
        netPolicyTraffic_->ResetPolicies();
        NETMGR_LOG_I("FactoryResetPolicies end.");
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_ERR_LOCAL_PTR_NULL;
}

void NetPolicyService::RegisterFactoryResetCallback()
{
    NETMGR_LOG_I("RegisterFactetCallback enter.");

    if (netFactoryResetCallback_ == nullptr) {
        netFactoryResetCallback_ =
            (std::make_unique<FactoryResetCallBack>(std::static_pointer_cast<NetPolicyService>(shared_from_this())))
                .release();
    }

    if (netFactoryResetCallback_ != nullptr) {
        int32_t ret = NetManagerCenter::GetInstance().RegisterNetFactoryResetCallback(netFactoryResetCallback_);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("RegisterFactoryResetCallback ret[%{public}d]", ret);
        }
    } else {
        NETMGR_LOG_E("netFactoryResetCallback_ is null");
    }
}

void NetPolicyService::UpdateNetAccessPolicyToMapFromDB()
{
    NETMGR_LOG_I("UpdateNetAccessPolicyToMapFromDB enter.");
    std::vector<NetAccessPolicyData> result = netAccessPolicy_.QueryAll();
    for (size_t i = 0; i < result.size(); i++) {
        NetworkAccessPolicy policy;
        policy.wifiAllow = result[i].wifiPolicy;
        policy.cellularAllow = result[i].cellularPolicy;
        (void)netPolicyRule_->SetNetworkAccessPolicy(result[i].uid, policy, result[i].setFromConfigFlag);
    }
}

// Do not post into event handler, because this interface should have good performance
int32_t NetPolicyService::SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag)
{
    NETMGR_LOG_I("SetNetworkAccessPolicy enter.");
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    NetAccessPolicyData data;
    data.uid = uid;
    data.wifiPolicy = policy.wifiAllow;
    data.cellularPolicy = policy.cellularAllow;
    data.setFromConfigFlag = !reconfirmFlag;
    netAccessPolicy_.InsertData(data);
    return netPolicyRule_->SetNetworkAccessPolicy(uid, policy, !reconfirmFlag);
}

int32_t NetPolicyService::GetNetworkAccessPolicy(AccessPolicyParameter parameter, AccessPolicySave &policy)
{
    NETMGR_LOG_I("GetNetworkAccessPolicy enter.");
    if (parameter.flag) {
        NetAccessPolicyData policyData;
        if (netAccessPolicy_.QueryByUid(parameter.uid, policyData) != NETMANAGER_SUCCESS) {
            policy.policy.wifiAllow = true;
            policy.policy.cellularAllow = true;
            return NETMANAGER_SUCCESS;
        }
        policy.policy.wifiAllow = policyData.wifiPolicy;
        policy.policy.cellularAllow = policyData.cellularPolicy;
        return NETMANAGER_SUCCESS;
    }

    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        NETMGR_LOG_E("fail to get system ability mgr.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        NETMGR_LOG_E("fail to get bundle manager proxy.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    sptr<AppExecFwk::BundleMgrProxy> bundleMgrProxy = iface_cast<AppExecFwk::BundleMgrProxy>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        NETMGR_LOG_E("Failed to get bundle manager proxy.");
        return NETMANAGER_ERR_INTERNAL;
    }

    std::vector<AppExecFwk::ApplicationInfo> appInfos;
    bool retC = bundleMgrProxy->GetApplicationInfos(AppExecFwk::ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMISSION,
                                                    static_cast<uint32_t>(parameter.userId), appInfos);
    if (!retC) {
        NETMGR_LOG_E("GetApplicationInfos Error");
        return NETMANAGER_ERR_INTERNAL;
    }
    for (const auto &appInfo : appInfos) {
        NetworkAccessPolicy policyTmp;
        NetAccessPolicyData policyData;
        if (netAccessPolicy_.QueryByUid(appInfo.uid, policyData) == NETMANAGER_SUCCESS) {
            policyTmp.wifiAllow = policyData.wifiPolicy;
            policyTmp.cellularAllow = policyData.cellularPolicy;
        } else {
            policyTmp.wifiAllow = true;
            policyTmp.cellularAllow = true;
        }
        policy.uid_policies.insert(std::pair<uint32_t, NetworkAccessPolicy>(appInfo.uid, policyTmp));
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyService::DeleteNetworkAccessPolicy(uint32_t uid)
{
    if (netPolicyRule_ == nullptr) {
        NETMGR_LOG_E("netPolicyRule_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    return netPolicyRule_->DeleteNetworkAccessPolicy(uid);
}

int32_t NetPolicyService::NotifyNetAccessPolicyDiag(uint32_t uid)
{
    NETMGR_LOG_I("NotifyNetAccessPolicyDiag");

    NetAccessPolicyDialog policyDialog;
    if (!policyDialog.ConnectSystemUi(uid)) {
        NETMGR_LOG_E("connect systemUi failed");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }

    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
