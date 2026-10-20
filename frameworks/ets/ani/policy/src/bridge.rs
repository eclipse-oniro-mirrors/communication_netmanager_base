// Copyright (C) 2026 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use std::collections::HashMap;

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetBearType")]
#[repr(i32)]
#[derive(Clone, Debug)]
pub enum NetBearType {
    BearerCellular = 0,
    BearerWifi = 1,
    BearerBluetooth = 2,
    BearerEthernet = 3,
    BearerVpn = 4,
}

impl From<i32> for NetBearType {
    fn from(value: i32) -> Self {
        match value {
            0 => NetBearType::BearerCellular,
            1 => NetBearType::BearerWifi,
            2 => NetBearType::BearerBluetooth,
            3 => NetBearType::BearerEthernet,
            4 => NetBearType::BearerVpn,
            _ => NetBearType::BearerCellular,
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetBackgroundPolicy")]
#[repr(i32)]
#[derive(Clone, Debug)]
pub enum NetBackgroundPolicy {
    NetBackgroundPolicyNone = 0,
    NetBackgroundPolicyEnable = 1,
    NetBackgroundPolicyDisable = 2,
    NetBackgroundPolicyTrustlist = 3,
}

impl From<i32> for NetBackgroundPolicy {
    fn from(value: i32) -> Self {
        match value {
            0 => NetBackgroundPolicy::NetBackgroundPolicyNone,
            1 => NetBackgroundPolicy::NetBackgroundPolicyEnable,
            2 => NetBackgroundPolicy::NetBackgroundPolicyDisable,
            3 => NetBackgroundPolicy::NetBackgroundPolicyTrustlist,
            _ => NetBackgroundPolicy::NetBackgroundPolicyNone,
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetQuotaPolicyInner")]
#[derive(Clone, Debug)]
pub struct NetQuotaPolicy {
    pub network_match_rule: NetworkMatchRule,
    pub quota_policy: QuotaPolicy,
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.QuotaPolicyInner")]
#[derive(Clone, Debug)]
pub struct QuotaPolicy {
    pub period_duration: String,
    pub warning_bytes: i64,
    pub limit_bytes: i64,
    pub metered: bool,
    pub limit_action: LimitAction,
    pub last_warning_remind: Option<i64>,
    pub last_limit_remind: Option<i64>,
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetworkMatchRuleInner")]
#[derive(Clone, Debug)]
pub struct NetworkMatchRule {
    pub net_type: NetBearType,
    pub identity: String,
    pub sim_id: String,
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetUidRuleInfoInner")]
#[derive(Clone, Debug)]
pub struct NetUidRuleInfo {
    pub uid: i32,
    pub rule: NetUidRule,
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetUidPolicyInfoInner")]
#[derive(Clone, Debug)]
pub struct NetUidPolicyInfo {
    pub uid: i32,
    pub policy: NetUidPolicy,
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.LimitAction")]
#[repr(i32)]
#[derive(Clone, Debug)]
pub enum LimitAction {
    LimitActionNone = -1,
    LimitActionAccessDisabled = 0,
    LimitActionAlertOnly = 1
}

impl From<i32> for LimitAction {
    fn from(value: i32) -> Self {
        match value {
            -1 => LimitAction::LimitActionNone,
            0 => LimitAction::LimitActionAccessDisabled,
            _ => LimitAction::LimitActionAlertOnly,
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetUidRule")]
#[repr(i32)]
#[derive(Clone, Debug)]
pub enum NetUidRule {
    NetRuleNone = 0,
    NetRuleAllowMeteredForeground = 1 << 0,
    NetRuleAllowMetered = 1 << 1,
    NetRuleRejectMetered = 1 << 2,
    NetRuleAllowAll = 1 << 5,
    NetRuleRejectAll = 1 << 6
}

impl From<i32> for NetUidRule {
    fn from(value: i32) -> Self {
        match value {
            1 => NetUidRule::NetRuleAllowMeteredForeground,
            2 => NetUidRule::NetRuleAllowMetered,
            4 => NetUidRule::NetRuleRejectMetered,
            32 => NetUidRule::NetRuleAllowAll,
            64 => NetUidRule::NetRuleRejectAll,
            _ => NetUidRule::NetRuleNone,
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.RemindType")]
#[repr(u32)]
#[derive(Clone, Debug)]
pub enum RemindType {
    RemindTypeWarning = 1,
    RemindTypeLimit = 2
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetUidPolicy")]
#[repr(u32)]
#[derive(Clone, Debug)]
pub enum NetUidPolicy {
    NetPolicyNone = 0,
    NetPolicyAllowMeteredBackground = 1 << 0,
    NetPolicyRejectMeteredBackground = 1 << 1
}

impl From<i32> for NetUidPolicy {
    fn from(value: i32) -> Self {
        match value {
            1 => NetUidPolicy::NetPolicyAllowMeteredBackground,
            2 => NetUidPolicy::NetPolicyRejectMeteredBackground,
            _ => NetUidPolicy::NetPolicyNone,
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetworkAccessPolicyInner")]
#[derive(Clone, Debug)]
pub struct NetworkAccessPolicy {
    pub allow_wi_fi: Option<bool>,
    pub allow_cellular: Option<bool>,
    pub always_allow_wi_fi: Option<bool>,
    pub always_allow_cellular: Option<bool>,
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.NetAccessPolicyInner")]
#[derive(Clone, Debug)]
pub struct NetAccessPolicy {
    pub allow_wi_fi: bool,
    pub allow_cellular: bool,
}

#[ani_rs::ani(path = "@ohos.net.policy.policy.UidNetworkAccessPolicy")]
#[derive(Clone, Debug)]
pub enum UidNetworkAccessPolicy {
    MapNetworkAccessPolicy(HashMap<String, NetworkAccessPolicy>),
}
