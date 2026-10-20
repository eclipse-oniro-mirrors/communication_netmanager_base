/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "policy_ani.h"
#include "ability_context.h"
#include "ani_base_context.h"
#include "errorcode_convertor.h"
#include "ipc_skeleton.h"
#include "modal_ui_extension_config.h"
#include "net_access_policy.h"
#include "net_manager_constants.h"
#include "net_policy_callback_stub.h"
#include "net_policy_client.h"
#include "net_quota_policy.h"
#include "ui_content.h"
#include "ui_extension_context.h"
#include "want.h"

namespace OHOS {
namespace NetManagerAni {

static constexpr size_t MAX_ARRAY_LEN = 64;
static constexpr const char* SETTINGS_PACKAGE_NAME = "com.huawei.hmos.communicationsetting";
static constexpr const char* SETTINGS_ABILITY_NAME = "NetAccessPolicySettingUIExtension";
static constexpr const char* UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
static constexpr const char* UIEXTENSION_TYPE_VALUE = "sys/commonUI";
static constexpr const char* CONTEXT_TYPE_KEY = "storeKit.ability.contextType";
static constexpr const char* UI_ABILITY_CONTEXT_VALUE = "uiAbility";
static constexpr const char* UI_EXTENSION_CONTEXT_VALUE = "uiExtension";
static constexpr const char* APP_UID = "appUid";

struct AppBaseContext {
    std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext = nullptr;
    std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> uiExtensionContext = nullptr;
};

static OHOS::Ace::UIContent* GetUIContent(std::shared_ptr<AppBaseContext> &asyncContext)
{
    if (!asyncContext) {
        return nullptr;
    }
    if (asyncContext->abilityContext != nullptr) {
        auto* uic = asyncContext->abilityContext->GetUIContent();
        return uic;
    }
    if (asyncContext->uiExtensionContext != nullptr) {
        auto* uic = asyncContext->uiExtensionContext->GetUIContent();
        return uic;
    }
    return nullptr;
}

class ModalUICallback {
public:
    explicit ModalUICallback(std::shared_ptr<AppBaseContext> baseContext) : baseContext_(baseContext) {}
    void OnRelease(int32_t releaseCode) { CloseModalUI(); }
    void SetSessionId(int32_t sessionId) { sessionId_ = sessionId; }
private:
    int32_t sessionId_ = 0;
    std::shared_ptr<AppBaseContext> baseContext_ = nullptr;
    void CloseModalUI()
    {
        auto uiContent = GetUIContent(baseContext_);
        if (uiContent != nullptr) {
            uiContent->CloseModalUIExtension(sessionId_);
        }
    }
};

static bool StartUiExtensionAbility(OHOS::AAFwk::Want &request, std::shared_ptr<AppBaseContext> &asyncContext)
{
    auto uiContent = GetUIContent(asyncContext);
    if (uiContent == nullptr) {
        return false;
    }
    std::string info = uiContent->GetContentInfo();
    auto callback = std::make_shared<ModalUICallback>(asyncContext);
    OHOS::Ace::ModalUIExtensionCallbacks extensionCallbacks = {
        std::bind(&ModalUICallback::OnRelease, callback, std::placeholders::_1)
    };
    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = false;
    int32_t sessionId = uiContent->CreateModalUIExtension(request, extensionCallbacks, config);
    if (sessionId == 0) {
        return false;
    }
    callback->SetSessionId(sessionId);
    return true;
}

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::string(convertor.ConvertErrorCode(errorCode));
}

int32_t CheckPolicyPermission()
{
    return NetManagerStandard::NetPolicyClient::GetInstance().CheckPermission();
}

class PolicyEventCallbackObserverAni : public NetManagerStandard::NetPolicyCallbackStub {
public:
    PolicyEventCallbackObserverAni() {}
    virtual ~PolicyEventCallbackObserverAni() {}

    int32_t NetUidPolicyChange(uint32_t uid, uint32_t policy) override
    {
        NetUidPolicyInfoAni info{.uid = static_cast<int32_t>(uid), .policy = static_cast<int32_t>(policy)};
        execute_net_uid_policy_changed(info);
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    int32_t NetUidRuleChange(uint32_t uid, uint32_t rule) override
    {
        NetUidRuleInfoAni info{.uid = static_cast<int32_t>(uid), .rule = static_cast<int32_t>(rule)};
        execute_net_uid_rule_changed(info);
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    int32_t NetQuotaPolicyChange(const std::vector<NetManagerStandard::NetQuotaPolicy> &quotaPolicies) override
    {
        rust::Vec<NetQuotaPolicyAni> vec;
        for (const auto &p : quotaPolicies) {
            NetQuotaPolicyAni ani;
            ani.network_match_rule.net_type = p.networkmatchrule.netType;
            ani.network_match_rule.identity = rust::string(p.networkmatchrule.ident);
            ani.network_match_rule.sim_id = rust::string(p.networkmatchrule.simId);
            ani.quota_policy.period_duration = rust::string(p.quotapolicy.periodDuration);
            ani.quota_policy.warning_bytes = p.quotapolicy.warningBytes;
            ani.quota_policy.limit_bytes = p.quotapolicy.limitBytes;
            ani.quota_policy.last_warning_remind = p.quotapolicy.lastWarningRemind;
            ani.quota_policy.last_limit_remind = p.quotapolicy.lastLimitRemind;
            ani.quota_policy.metered = p.quotapolicy.metered;
            ani.quota_policy.limit_action = p.quotapolicy.limitAction;
            vec.push_back(std::move(ani));
        }
        execute_net_quota_policy_changed(vec);
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    int32_t NetBackgroundPolicyChange(bool isBackgroundPolicyAllow) override
    {
        execute_net_background_policy_changed(isBackgroundPolicyAllow);
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    int32_t NetMeteredIfacesChange(std::vector<std::string> &ifaces) override
    {
        MeteredIfacesAni info;
        for (auto &iface : ifaces) {
            info.ifaces.push_back(rust::string(iface));
        }
        execute_net_metered_ifaces_changed(info);
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }
};

sptr<PolicyEventCallbackObserverAni> g_policyEventCallbackObserverAni =
    sptr<PolicyEventCallbackObserverAni>(new (std::nothrow) PolicyEventCallbackObserverAni());

std::atomic_bool g_isPolicyObserverRegistered = false;

int32_t RegisterNetPolicyChangeCallback()
{
    bool expected = false;
    if (!g_isPolicyObserverRegistered.compare_exchange_strong(expected, true)) {
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    if (g_policyEventCallbackObserverAni == nullptr) {
        g_isPolicyObserverRegistered = false;
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }

    int32_t ret = NetManagerStandard::NetPolicyClient::GetInstance()
        .RegisterNetPolicyCallback(g_policyEventCallbackObserverAni);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        g_isPolicyObserverRegistered = false;
    }
    return ret;
}

int32_t UnregisterNetPolicyChangeCallback()
{
    bool expected = true;
    if (!g_isPolicyObserverRegistered.compare_exchange_strong(expected, false)) {
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    if (g_policyEventCallbackObserverAni == nullptr) {
        g_isPolicyObserverRegistered = true;
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }

    auto ret = NetManagerStandard::NetPolicyClient::GetInstance()
        .UnregisterNetPolicyCallback(g_policyEventCallbackObserverAni);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        g_isPolicyObserverRegistered = true;
    }
    return ret;
}

int32_t SetBackgroundAllowed(bool allowed)
{
    return NetManagerStandard::NetPolicyClient::GetInstance().SetBackgroundPolicy(allowed);
}

int32_t GetBackgroundPolicy(bool &backgroundPolicy)
{
    return NetManagerStandard::NetPolicyClient::GetInstance().GetBackgroundPolicy(backgroundPolicy);
}

int32_t GetBackgroundPolicyByUid(int32_t uid, uint32_t &backgroundPolicyOfUid)
{
    if (uid < 0) {
        return NetManagerStandard::POLICY_ERR_INVALID_UID;
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().GetBackgroundPolicyByUid(
        static_cast<uint32_t>(uid), backgroundPolicyOfUid);
}

int32_t SetPolicyByUid(int32_t uid, uint32_t policy)
{
    if (uid < 0) {
        return NetManagerStandard::POLICY_ERR_INVALID_UID;
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().SetPolicyByUid(static_cast<uint32_t>(uid), policy);
}

int32_t GetPolicyByUid(int32_t uid, uint32_t &policy)
{
    if (uid < 0) {
        return NetManagerStandard::POLICY_ERR_INVALID_UID;
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().GetPolicyByUid(static_cast<uint32_t>(uid), policy);
}

int32_t GetUidsByPolicy(uint32_t policy, rust::Vec<uint32_t> &result)
{
    std::vector<uint32_t> uids;
    int32_t ret = NetManagerStandard::NetPolicyClient::GetInstance().GetUidsByPolicy(policy, uids);
    if (ret == NetManagerStandard::NETMANAGER_SUCCESS) {
        for (auto uid : uids) {
            result.push_back(uid);
        }
    }
    return ret;
}

int32_t IsUidNetAllowed(int32_t uid, bool metered, bool &isAllowed)
{
    if (uid < 0) {
        return NetManagerStandard::POLICY_ERR_INVALID_UID;
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().IsUidNetAllowed(static_cast<uint32_t>(uid),
        metered, isAllowed);
}

int32_t IsUidNetAllowedByIface(int32_t uid, rust::String iface, bool &isAllowed)
{
    if (uid < 0) {
        return NetManagerStandard::POLICY_ERR_INVALID_UID;
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().IsUidNetAllowed(static_cast<uint32_t>(uid),
        std::string(iface), isAllowed);
}

int32_t SetNetQuotaPolicies(rust::Vec<NetQuotaPolicyAni> policies)
{
    size_t policyCount = policies.size() > MAX_ARRAY_LEN ? MAX_ARRAY_LEN : policies.size();
    std::vector<NetManagerStandard::NetQuotaPolicy> vec;
    for (size_t i = 0; i < policyCount; i++) {
        const auto &ani = policies[i];
        NetManagerStandard::NetQuotaPolicy p;
        p.networkmatchrule.netType = ani.network_match_rule.net_type;
        p.networkmatchrule.ident = static_cast<std::string>(ani.network_match_rule.identity);
        p.networkmatchrule.simId = static_cast<std::string>(ani.network_match_rule.sim_id);
        p.quotapolicy.periodDuration = static_cast<std::string>(ani.quota_policy.period_duration);
        p.quotapolicy.warningBytes = ani.quota_policy.warning_bytes;
        p.quotapolicy.limitBytes = ani.quota_policy.limit_bytes;
        p.quotapolicy.lastWarningRemind = ani.quota_policy.last_warning_remind;
        p.quotapolicy.lastLimitRemind = ani.quota_policy.last_limit_remind;
        p.quotapolicy.metered = ani.quota_policy.metered;
        p.quotapolicy.limitAction = ani.quota_policy.limit_action;
        vec.push_back(std::move(p));
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().SetNetQuotaPolicies(vec);
}

int32_t RestoreAllPolicies(rust::String iccid)
{
    return NetManagerStandard::NetPolicyClient::GetInstance().ResetPolicies(std::string(iccid));
}

int32_t GetNetQuotaPolicies(rust::Vec<NetQuotaPolicyAni> &result)
{
    std::vector<NetManagerStandard::NetQuotaPolicy> policies;
    int32_t ret = NetManagerStandard::NetPolicyClient::GetInstance().GetNetQuotaPolicies(policies);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ret;
    }
    for (const auto &p : policies) {
        NetQuotaPolicyAni ani;
        ani.network_match_rule.net_type = p.networkmatchrule.netType;
        ani.network_match_rule.identity = rust::string(p.networkmatchrule.ident);
        ani.network_match_rule.sim_id = rust::string(p.networkmatchrule.simId);
        ani.quota_policy.period_duration = rust::string(p.quotapolicy.periodDuration);
        ani.quota_policy.warning_bytes = p.quotapolicy.warningBytes;
        ani.quota_policy.limit_bytes = p.quotapolicy.limitBytes;
        ani.quota_policy.last_warning_remind = p.quotapolicy.lastWarningRemind;
        ani.quota_policy.last_limit_remind = p.quotapolicy.lastLimitRemind;
        ani.quota_policy.metered = p.quotapolicy.metered;
        ani.quota_policy.limit_action = p.quotapolicy.limitAction;
        result.push_back(std::move(ani));
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t UpdateRemindPolicy(int32_t netType, rust::String iccid, uint32_t remindType)
{
    return NetManagerStandard::NetPolicyClient::GetInstance().UpdateRemindPolicy(
        netType, std::string(iccid), remindType);
}

int32_t SetDeviceIdleTrustlist(rust::Vec<int32_t> uids, bool isAllowed)
{
    size_t uidCount = uids.size() > MAX_ARRAY_LEN ? MAX_ARRAY_LEN : uids.size();
    std::vector<uint32_t> vec;
    for (size_t i = 0; i < uidCount; i++) {
        if (uids[i] > 0) {
            vec.push_back(static_cast<uint32_t>(uids[i]));
        }
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().SetDeviceIdleTrustlist(vec, isAllowed);
}

int32_t GetDeviceIdleTrustlist(rust::Vec<uint32_t> &result)
{
    std::vector<uint32_t> uids;
    int32_t ret = NetManagerStandard::NetPolicyClient::GetInstance().GetDeviceIdleTrustlist(uids);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ret;
    }
    for (auto uid : uids) {
        result.push_back(uid);
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t SetPowerTrustlist(rust::Vec<int32_t> ids, bool allowed)
{
    size_t uidCount = ids.size() > MAX_ARRAY_LEN ? MAX_ARRAY_LEN : ids.size();
    std::vector<uint32_t> vec;
    for (size_t i = 0; i < uidCount; i++) {
        if (ids[i] > 0) {
            vec.push_back(static_cast<uint32_t>(ids[i]));
        }
    }
    return NetManagerStandard::NetPolicyClient::GetInstance().SetPowerSaveTrustlist(vec, allowed);
}

int32_t GetPowerSaveTrustlist(rust::Vec<uint32_t> &result)
{
    std::vector<uint32_t> uids;
    int32_t ret = NetManagerStandard::NetPolicyClient::GetInstance().GetPowerSaveTrustlist(uids);
    if (ret == NetManagerStandard::NETMANAGER_SUCCESS) {
        for (auto uid : uids) {
            result.push_back(uid);
        }
    }
    return ret;
}

int32_t SetNetworkAccessPolicyTyped(int32_t uid, NetworkAccessPolicyAni policy, bool reconfirmFlag)
{
    NetManagerStandard::NetworkAccessPolicy nativePolicy;
    nativePolicy.wifiAllow = policy.allow_wi_fi;
    nativePolicy.cellularAllow = policy.allow_cellular;
    nativePolicy.wifiSwitchDisable = policy.always_allow_wi_fi;
    nativePolicy.cellularSwitchDisable = policy.always_allow_cellular;
    return NetManagerStandard::NetPolicyClient::GetInstance()
        .SetNetworkAccessPolicy(uid < 0 ? 0 : static_cast<uint32_t>(uid), nativePolicy, reconfirmFlag);
}

int32_t GetNetworkAccessPolicyByUid(int32_t uid, NetworkAccessPolicyAni &result)
{
    NetManagerStandard::AccessPolicyParameter parameter;
    parameter.flag = true;
    parameter.uid = uid < 0 ? 0 : static_cast<uint32_t>(uid);
    NetManagerStandard::AccessPolicySave save;
    int32_t ret = NetManagerStandard::NetPolicyClient::GetInstance().GetNetworkAccessPolicy(parameter, save);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ret;
    }
    result.allow_wi_fi = save.policy.wifiAllow;
    result.allow_cellular = save.policy.cellularAllow;
    result.always_allow_wi_fi = save.policy.wifiSwitchDisable;
    result.always_allow_cellular = save.policy.cellularSwitchDisable;
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t GetAllNetworkAccessPolicies(rust::Vec<UidNetworkPolicyAni> &result)
{
    NetManagerStandard::AccessPolicyParameter parameter;
    parameter.flag = false;
    parameter.uid = 0;
    NetManagerStandard::AccessPolicySave save;
    int32_t ret = NetManagerStandard::NetPolicyClient::GetInstance().GetNetworkAccessPolicy(parameter, save);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ret;
    }
    for (const auto &kv : save.uid_policies) {
        UidNetworkPolicyAni item;
        item.uid = static_cast<int32_t>(kv.first);
        item.policy.allow_wi_fi = kv.second.wifiAllow;
        item.policy.allow_cellular = kv.second.cellularAllow;
        item.policy.always_allow_wi_fi = kv.second.wifiSwitchDisable;
        item.policy.always_allow_cellular = kv.second.cellularSwitchDisable;
        result.push_back(std::move(item));
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t GetSelfNetworkAccessPolicy(NetAccessPolicyAni &result)
{
    NetManagerStandard::NetAccessPolicy netPolicy;
    auto ret = NetManagerStandard::NetPolicyClient::GetInstance().GetSelfNetworkAccessPolicy(netPolicy);
    if (ret == NetManagerStandard::NETMANAGER_SUCCESS) {
        result.allow_wi_fi = netPolicy.allowWiFi;
        result.allow_cellular = netPolicy.allowCellular;
    }
    return ret;
}

int32_t ShowAppNetPolicySettings(AniEnv **env, AniObject *context)
{
    if (env == nullptr || *env == nullptr || context == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }
    auto aniEnv = reinterpret_cast<ani_env *>(*env);
    auto obj = *reinterpret_cast<ani_object *>(context);
    ani_boolean isStage = false;
    auto status = AbilityRuntime::IsStageContext(aniEnv, obj, isStage);
    if (status != ANI_OK || !isStage) {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }
    auto ctx = AbilityRuntime::GetStageModeContext(aniEnv, obj);
    if (ctx == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }

    auto loadProductContext = std::make_shared<AppBaseContext>();
    auto abilityCtx = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(ctx);
    if (abilityCtx != nullptr) {
        loadProductContext->abilityContext = abilityCtx;
    } else {
        auto uiExtCtx = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::UIExtensionContext>(ctx);
        if (uiExtCtx != nullptr) {
            loadProductContext->uiExtensionContext = uiExtCtx;
        } else {
            return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        }
    }

    uint32_t currentUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    OHOS::AAFwk::Want want;
    want.SetElementName(std::string(SETTINGS_PACKAGE_NAME), std::string(SETTINGS_ABILITY_NAME));
    want.SetParam(std::string(UIEXTENSION_TYPE_KEY), std::string(UIEXTENSION_TYPE_VALUE));
    want.SetParam(std::string(CONTEXT_TYPE_KEY),
        loadProductContext->uiExtensionContext != nullptr ?
        std::string(UI_EXTENSION_CONTEXT_VALUE) : std::string(UI_ABILITY_CONTEXT_VALUE));
    want.SetParam(std::string(APP_UID), static_cast<int64_t>(currentUid));
    if (!StartUiExtensionAbility(want, loadProductContext)) {
        return NetManagerStandard::NETMANAGER_ERR_INTERNAL;
    }

    return NetManagerStandard::NETMANAGER_SUCCESS;
}

} // namespace NetManagerAni
} // namespace OHOS
