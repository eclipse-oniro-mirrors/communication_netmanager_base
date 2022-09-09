/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "net_policy_rule.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyRule::NetPolicyRule() = default;

void NetPolicyRule::Init()
{
    // Init uid、policy and background allow status from file,and save uid、policy into uidPolicyRules_.
    NETMGR_LOG_I("Start init uid and policy.");
    GetFileInst()->InitPolicy();
    const auto &uidsPolicies = GetFileInst()->GetNetPolicies();
    backgroundAllow_ = GetFileInst()->GetBackgroundPolicy();

    for (const auto &i : uidsPolicies) {
        auto uid = CommonUtils::StrToUint(i.uid.c_str());
        auto policy = CommonUtils::StrToUint(i.policy.c_str());
        uidPolicyRules_[uid] = {.policy_ = policy};
        TransPolicyToRule(uid, policy);
    }
}

void NetPolicyRule::TransPolicyToRule()
{
    // When system status is changed,traverse uidPolicyRules_ to calculate the rule and netsys.
    for (const auto &[uid, policy] : uidPolicyRules_) {
        NETMGR_LOG_I("TransPolicyToRule without input value:uid[%{public}u] policy[%{public}u]", uid, policy.policy_);
        TransPolicyToRule(uid, policy.policy_);
    }
}

void NetPolicyRule::TransPolicyToRule(uint32_t uid)
{
    uint32_t policy;
    const auto &itr = uidPolicyRules_.find(uid);
    if (itr == uidPolicyRules_.end()) {
        policy = NET_POLICY_NONE;
    } else {
        policy = itr->second.policy_;
    }
    NETMGR_LOG_I("TransPolicyToRule only with uid value: uid[%{public}u] policy[%{public}u]", uid, policy);
    TransPolicyToRule(uid, policy);
    return;
}

uint32_t NetPolicyRule::TransPolicyToRule(uint32_t uid, uint32_t policy)
{
    NetmanagerHiTrace::NetmanagerStartSyncTrace("TransPolicyToRule start");
    auto policyRule = uidPolicyRules_.find(uid);
    if (policyRule == uidPolicyRules_.end()) {
        NETMGR_LOG_I("Don't find this uid, need to add uid:[%{public}u] policy[%{public}u].", uid, policy);
        uidPolicyRules_[uid] = {.policy_ = policy};
        GetCbInst()->NotifyNetUidPolicyChange(uid, policy);
    } else {
        if (policyRule->second.policy_ != policy) {
            NETMGR_LOG_I("Update policy's value.uid:[%{public}u] policy[%{public}u]", uid, policy);
            policyRule->second.policy_ = policy;
            GetCbInst()->NotifyNetUidPolicyChange(uid, policy);
        }
    }

    auto policyCondition = BuildTransCondition(uid, policy);
    TransConditionToRuleAndNetsys(policyCondition, uid, policy);
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("TransPolicyToRule end");
    return ERR_NONE;
}

uint32_t NetPolicyRule::BuildTransCondition(uint32_t uid, uint32_t policy)
{
    // Integrate status values, the result of policyCondition will be use in TransConditionToRuleAndNetsys.
    uint32_t policyCondition = ChangePolicyToPolicytranscondition(policy);

    if (IsIdleMode()) {
        policyCondition |= POLICY_TRANS_CONDITION_IDLE_MODE;
    }
    if (InIdleAllowedList(uid)) {
        policyCondition |= POLICY_TRANS_CONDITION_IDLE_ALLOWEDLIST;
    }
    if (IsLimitByAdmin()) {
        policyCondition |= POLICY_TRANS_CONDITION_ADMIN_RESTRICT;
    }
    if (IsPowerSave()) {
        policyCondition |= POLICY_TRANS_CONDITION_POWERSAVE_MODE;
    }
    if (InPowerSaveAllowedList(uid)) {
        policyCondition |= POLICY_TRANS_CONDITION_POWERSAVE_ALLOWEDLIST;
    }
    if (!GetBackgroundPolicy()) {
        policyCondition |= POLICY_TRANS_CONDITION_BACKGROUND_RESTRICT;
    }
    if (IsForeground(uid)) {
        policyCondition |= POLICY_TRANS_CONDITION_FOREGROUND;
    }
    NETMGR_LOG_I("BuildTransCondition uid[%{public}u] policy[%{public}u]", uid, policy);
    return policyCondition;
}

void NetPolicyRule::TransConditionToRuleAndNetsys(uint32_t policyCondition, uint32_t uid, uint32_t policy)
{
    uint32_t conditionValue = GetMatchTransCondition(policyCondition);

    auto rule = MoveToRuleBit(conditionValue & POLICY_TRANS_RULE_MASK);
    NETMGR_LOG_I("NetPolicyRule->uid:[%{public}u] policy:[%{public}u] rule:[%{public}u] policyCondition[%{public}u]",
                 uid, policy, rule, policyCondition);
    auto policyRuleNetsys = uidPolicyRules_.find(uid)->second;
    auto netsys = conditionValue & POLICY_TRANS_NET_CTRL_MASK;

    if (policyRuleNetsys.netsys_ != netsys) {
        NetsysCtrl(uid, netsys);
        policyRuleNetsys.netsys_ = netsys;
    } else {
        NETMGR_LOG_I("Same netsys and uid ,don't need to do others.now netsys is: [%{public}u]", netsys);
    }

    GetFileInst()->WriteFile(uid, policy);

    if (policyRuleNetsys.rule_ == rule) {
        NETMGR_LOG_I("Same rule and uid ,don't need to do others.uid is:[%{public}u] rule is:[%{public}u]", uid, rule);
        return;
    }

    policyRuleNetsys.rule_ = rule;
    GetCbInst()->NotifyNetUidRuleChange(uid, rule);
}

uint32_t NetPolicyRule::GetMatchTransCondition(uint32_t policyCondition)
{
    for (const auto &i : POLICY_TRANS_MAP) {
        auto condition = MoveToConditionBit(i & POLICY_TRANS_CONDITION_MASK);
        if ((policyCondition & condition) == condition) {
            NETMGR_LOG_D("GetMatchTransCondition condition: %{public}d.", i);
            return i;
        }
    }
    return POLICY_TRANS_MAP.back();
}

void NetPolicyRule::NetsysCtrl(uint32_t uid, uint32_t netsysCtrl)
{
    if (netsysCtrl == POLICY_TRANS_CTRL_NONE) {
        NETMGR_LOG_I("Don't need to do anything,keep now status.uid[%{public}u],netsysCtrl: [%{public}u]", uid,
                     netsysCtrl);
        return;
    }

    if (netsysCtrl == POLICY_TRANS_CTRL_REMOVE_ALL) {
        GetNetsysInst()->BandwidthRemoveAllowedList(uid);
        GetNetsysInst()->BandwidthRemoveDeniedList(uid);
        NETMGR_LOG_I("Remove uid:[%{public}u] from black list and white list.netsysCtrl: [%{public}u]", uid,
                     netsysCtrl);
        return;
    }

    if (netsysCtrl == POLICY_TRANS_CTRL_ADD_DENIEDLIST) {
        GetNetsysInst()->BandwidthAddDeniedList(uid);
        GetNetsysInst()->BandwidthRemoveAllowedList(uid);
        NETMGR_LOG_I("Add uid:[%{public}u] into reject list and remove from white list.netsysCtrl: [%{public}u]", uid,
                     netsysCtrl);
        return;
    }

    if (netsysCtrl == POLICY_TRANS_CTRL_ADD_ALLOWEDLIST) {
        GetNetsysInst()->BandwidthRemoveDeniedList(uid);
        GetNetsysInst()->BandwidthAddAllowedList(uid);
        NETMGR_LOG_I("Add uid:[%{public}u] into white list and remove from reject list.netsysCtrl: [%{public}u]", uid,
                     netsysCtrl);
        return;
    }
    NETMGR_LOG_E("Error netsysCtrl value, need to check.uid:[%{public}u],netsysCtrl: [%{public}u]", uid, netsysCtrl);
}

uint32_t NetPolicyRule::MoveToConditionBit(uint32_t value)
{
    return value >> CONDITION_START_BIT;
}

uint32_t NetPolicyRule::MoveToRuleBit(uint32_t value)
{
    return value >> RULE_START_BIT;
}

uint32_t NetPolicyRule::ChangePolicyToPolicytranscondition(uint32_t policy)
{
    if (policy == NET_POLICY_NONE) {
        return POLICY_TRANS_CONDITION_UID_POLICY_NONE;
    }
    return policy << 1;
}

uint32_t NetPolicyRule::GetPolicyByUid(uint32_t uid)
{
    auto policyRule = uidPolicyRules_.find(uid);
    if (policyRule == uidPolicyRules_.end()) {
        NETMGR_LOG_I("Can't find uid:[%{public}u] and its policy, return default value.", uid);
        return NET_POLICY_NONE;
    }
    return policyRule->second.policy_;
}

std::vector<uint32_t> NetPolicyRule::GetUidsByPolicy(uint32_t policy)
{
    NETMGR_LOG_I("GetUidsByPolicy:policy:[%{public}u]", policy);
    std::vector<uint32_t> uids;
    for (auto &iter : uidPolicyRules_) {
        if (iter.second.policy_ == policy) {
            uids.push_back(iter.first);
        }
    }
    return uids;
}

bool NetPolicyRule::IsUidNetAllowed(uint32_t uid, bool metered)
{
    NETMGR_LOG_I("IsUidNetAllowed:uid[%{public}u] metered:[%{public}d]", uid, metered);
    uint32_t rule = NetUidRule::NET_RULE_NONE;
    auto iter = uidPolicyRules_.find(uid);
    if (iter != uidPolicyRules_.end()) {
        rule = iter->second.rule_;
    }

    if (rule == NetUidRule::NET_RULE_REJECT_ALL) {
        return false;
    }

    if (!metered) {
        return true;
    }

    if (rule == NetUidRule::NET_RULE_REJECT_METERED) {
        return false;
    }

    if (rule == NetUidRule::NET_RULE_ALLOW_METERED) {
        return true;
    }

    if (rule == NetUidRule::NET_RULE_ALLOW_METERED_FOREGROUND) {
        return true;
    }

    if (!backgroundAllow_) {
        return false;
    }

    return true;
}

uint32_t NetPolicyRule::SetBackgroundPolicy(bool allow)
{
    if (backgroundAllow_ != allow) {
        GetCbInst()->NotifyNetBackgroundPolicyChange(allow);
        backgroundAllow_ = allow;
        TransPolicyToRule();
        GetFileInst()->SetBackgroundPolicy(allow);
        NetmanagerHiTrace::NetmanagerStartSyncTrace("SetBackgroundPolicy policy start");
        GetNetsysInst()->BandwidthEnableDataSaver(!allow);
        NetmanagerHiTrace::NetmanagerFinishSyncTrace("SetBackgroundPolicy policy end");
    } else {
        NETMGR_LOG_I("Same background policy,don't need to repeat set. now background policy is:[%{public}d]", allow);
    }
    return ERR_NONE;
}

uint32_t NetPolicyRule::GetBackgroundPolicyByUid(uint32_t uid)
{
    uint32_t policy = GetPolicyByUid(uid);
    NETMGR_LOG_I("GetBackgroundPolicyByUid GetPolicyByUid uid: %{public}u policy: %{public}u.", uid, policy);
    if ((policy & NET_POLICY_REJECT_METERED_BACKGROUND) != 0) {
        return NET_BACKGROUND_POLICY_DISABLE;
    }

    if (backgroundAllow_) {
        return NET_BACKGROUND_POLICY_ENABLE;
    }

    if ((policy & NET_POLICY_ALLOW_METERED_BACKGROUND) != 0) {
        return NET_BACKGROUND_POLICY_ALLOWEDLIST;
    }
    return NET_BACKGROUND_POLICY_DISABLE;
}

uint32_t NetPolicyRule::ResetPolicies()
{
    NETMGR_LOG_I("Reset uids-policies and backgroundpolicy");
    for (auto iter : uidPolicyRules_) {
        TransPolicyToRule(iter.first, NetUidPolicy::NET_POLICY_NONE);
    }
    SetBackgroundPolicy(true);
    return ERR_NONE;
}

bool NetPolicyRule::GetBackgroundPolicy()
{
    NETMGR_LOG_I("GetBackgroundPolicy:backgroundAllow_[%{public}d", backgroundAllow_);
    return backgroundAllow_;
}

bool NetPolicyRule::IsIdleMode()
{
    NETMGR_LOG_I("IsIdleMode:deviceIdleMode_[%{public}d", deviceIdleMode_);
    return deviceIdleMode_;
}

bool NetPolicyRule::InIdleAllowedList(uint32_t uid)
{
    if (std::find(deviceIdleAllowedList_.begin(), deviceIdleAllowedList_.end(), uid) != deviceIdleAllowedList_.end()) {
        return true;
    }
    return false;
}

bool NetPolicyRule::IsLimitByAdmin()
{
    // to judge if limit by admin.
    return false;
}

bool NetPolicyRule::IsForeground(uint32_t uid)
{
    // to judge if this uid is foreground.
    return false;
}

bool NetPolicyRule::IsPowerSave()
{
    // to judge if in power save mode.
    return false;
}

bool NetPolicyRule::InPowerSaveAllowedList(uint32_t uid)
{
    // to judge this uid if in power save allow list.
    return false;
}

void NetPolicyRule::DeleteUid(uint32_t uid)
{
    NETMGR_LOG_I("DeleteUid:uid[%{public}u]", uid);
    for (auto iter : uidPolicyRules_) {
        if (iter.first == uid) {
            uidPolicyRules_.erase(iter.first);
            break;
        }
    }
    GetNetsysInst()->BandwidthRemoveDeniedList(uid);
    GetNetsysInst()->BandwidthRemoveAllowedList(uid);
}

void NetPolicyRule::HandleEvent(int32_t eventId, const std::shared_ptr<PolicyEvent> &policyEvent)
{
    switch (eventId) {
        case NetPolicyEventHandler::MSG_DEVICE_IDLE_LIST_UPDATED:
            deviceIdleAllowedList_ = policyEvent->deviceIdleList;
            break;
        case NetPolicyEventHandler::MSG_DEVICE_IDLE_MODE_CHANGED:
            deviceIdleMode_ = policyEvent->deviceIdleMode;
            TransPolicyToRule();
            break;
        case NetPolicyEventHandler::MSG_UID_REMOVED:
            DeleteUid(policyEvent->deletedUid);
            break;
        default:
            break;
    }
}

void NetPolicyRule::GetDumpMessage(std::string &message)
{
    static const std::string TAB = "    ";
    message.append(TAB + "UidPolicy:\n");
    std::for_each(uidPolicyRules_.begin(), uidPolicyRules_.end(), [&message](const auto &pair) {
        message.append(TAB + TAB + "Uid: " + std::to_string(pair.first) + TAB + "Rule: " +
                       std::to_string(pair.second.rule_) + TAB + "Policy:" + std::to_string(pair.second.policy_) + TAB +
                       "NetSys: " + std::to_string(pair.second.netsys_) + "\n");
    });
    message.append(TAB + "DeviceIdleAllowList: {");
    std::for_each(deviceIdleAllowedList_.begin(), deviceIdleAllowedList_.end(),
                  [&message](const auto &item) { message.append(std::to_string(item) + ", "); });
    message.append("}\n");
    message.append(TAB + "DeviceIdleMode: " + std::to_string(deviceIdleMode_) + "\n");
    message.append(TAB + "BackgroundPolicy: " + std::to_string(backgroundAllow_) + "\n");
}
} // namespace NetManagerStandard
} // namespace OHOS
