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

use crate::bridge;
use crate::register::*;
use std::collections::HashMap;

#[repr(transparent)]
pub struct AniEnv {
    pub inner: ani_rs::AniEnv<'static>,
}

#[repr(transparent)]
pub struct AniObject {
    pub inner: ani_rs::objects::AniObject<'static>,
}

pub struct NetPolicyClient;

impl NetPolicyClient {
    pub fn set_background_allowed(allowed: bool) -> Result<(), i32> {
        let res = ffi::SetBackgroundAllowed(allowed);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn get_background_policy() -> Result<bool, i32> {
        let mut out: bool = false;
        let res = ffi::GetBackgroundPolicy(&mut out);
        if res != 0 { return Err(res); }
        Ok(out)
    }

    pub fn get_background_policy_by_uid(uid: i32) -> Result<bridge::NetBackgroundPolicy, i32> {
        let mut out: u32 = 0;
        let res = ffi::GetBackgroundPolicyByUid(uid, &mut out);
        if res != 0 { return Err(res); }
        Ok(bridge::NetBackgroundPolicy::from(out as i32))
    }

    pub fn set_policy_by_uid(uid: i32, policy: bridge::NetUidPolicy) -> Result<(), i32> {
        let res = ffi::SetPolicyByUid(uid, policy as u32);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn get_policy_by_uid(uid: i32) -> Result<bridge::NetUidPolicy, i32> {
        let mut out: u32 = 0;
        let res = ffi::GetPolicyByUid(uid, &mut out);
        if res != 0 { return Err(res); }
        Ok(bridge::NetUidPolicy::from(out as i32))
    }

    pub fn get_uids_by_policy(policy: bridge::NetUidPolicy) -> Result<Vec<i32>, i32> {
        let mut out = Vec::new();
        let res = ffi::GetUidsByPolicy(policy as u32, &mut out);
        if res != 0 { return Err(res); }
        Ok(out.into_iter().map(|v| v as i32).collect())
    }

    pub fn is_uid_net_allowed(uid: i32, metered: bool) -> Result<bool, i32> {
        let mut out: bool = false;
        let res = ffi::IsUidNetAllowed(uid, metered, &mut out);
        if res != 0 { return Err(res); }
        Ok(out)
    }

    pub fn is_uid_net_allowed_by_iface(uid: i32, iface: String) -> Result<bool, i32> {
        let mut out: bool = false;
        let res = ffi::IsUidNetAllowedByIface(uid, iface, &mut out);
        if res != 0 { return Err(res); }
        Ok(out)
    }

    pub fn set_net_quota_policies(policies: Vec<bridge::NetQuotaPolicy>) -> Result<(), i32> {
        let ani_policies: Vec<ffi::NetQuotaPolicyAni> =
            policies.into_iter().map(|p| p.into()).collect();
        let res = ffi::SetNetQuotaPolicies(ani_policies);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn restore_all_policies(iccid: String) -> Result<(), i32> {
        let res = ffi::RestoreAllPolicies(iccid);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn get_net_quota_policies() -> Result<Vec<bridge::NetQuotaPolicy>, i32> {
        let mut policies = Vec::new();
        let res = ffi::GetNetQuotaPolicies(&mut policies);
        if res != 0 { return Err(res); }
        Ok(policies.into_iter().map(|p| p.into()).collect())
    }

    pub fn update_remind_policy(
        net_type: bridge::NetBearType,
        iccid: String,
        remind_type: bridge::RemindType,
    ) -> Result<(), i32> {
        let res = ffi::UpdateRemindPolicy(net_type as i32, iccid, remind_type as u32);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn set_device_idle_trustlist(uids: Vec<i32>, is_allowed: bool) -> Result<(), i32> {
        let res = ffi::SetDeviceIdleTrustlist(uids, is_allowed);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn get_device_idle_trustlist() -> Result<Vec<i32>, i32> {
        let mut result = Vec::new();
        let res = ffi::GetDeviceIdleTrustlist(&mut result);
        if res != 0 { return Err(res); }
        Ok(result.into_iter().map(|v| v as i32).collect())
    }

    pub fn set_power_save_trustlist(uids: Vec<i32>, is_allowed: bool) -> Result<(), i32> {
        let res = ffi::SetPowerTrustlist(uids, is_allowed);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn get_power_save_trustlist() -> Result<Vec<i32>, i32> {
        let mut result = Vec::new();
        let res = ffi::GetPowerSaveTrustlist(&mut result);
        if res != 0 { return Err(res); }
        Ok(result.into_iter().map(|v| v as i32).collect())
    }

    pub fn reset_policies(iccid: String) -> Result<(), i32> {
        let res = ffi::RestoreAllPolicies(iccid.to_owned());
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn set_network_access_policy(uid: i32,
        policy: bridge::NetworkAccessPolicy, reconfirm_flag: bool) -> Result<(), i32> {
        let ani_policy: ffi::NetworkAccessPolicyAni = policy.into();
        let res = ffi::SetNetworkAccessPolicyTyped(uid, ani_policy, reconfirm_flag);
        if res != 0 { return Err(res); }
        Ok(())
    }

    pub fn get_network_access_policy(uid: i32) -> Result<bridge::NetworkAccessPolicy, i32> {
        let mut policy = ffi::NetworkAccessPolicyAni {
            allow_wi_fi: false,
            allow_cellular: false,
            always_allow_wi_fi: false,
            always_allow_cellular: false,
        };
        let res = ffi::GetNetworkAccessPolicyByUid(uid, &mut policy);
        if res != 0 { return Err(res); }
        Ok(policy.into())
    }

    pub fn get_all_network_access_policies() -> Result<bridge::UidNetworkAccessPolicy, i32> {
        let mut items = Vec::new();
        let res = ffi::GetAllNetworkAccessPolicies(&mut items);
        if res != 0 { return Err(res); }
        let mut result = HashMap::new();
        for item in items {
            result.insert(item.uid.to_string(), item.policy.into());
        }
        Ok(bridge::UidNetworkAccessPolicy::MapNetworkAccessPolicy(result))
    }

    pub fn get_self_network_access_policy() -> Result<bridge::NetAccessPolicy, i32> {
        let mut policy = ffi::NetAccessPolicyAni {
            allow_wi_fi: false,
            allow_cellular: false,
        };
        let res = ffi::GetSelfNetworkAccessPolicy(&mut policy);
        if res != 0 { return Err(res); }
        Ok(policy.into())
    }

    pub fn show_app_net_policy_settings<'local>(
        env: &ani_rs::AniEnv<'local>,
        context_object: &ani_rs::objects::AniObject<'local>,
    ) -> Result<(), i32> {
        let env = env as *const ani_rs::AniEnv as *mut ani_rs::AniEnv as *mut *mut AniEnv;
        let context_object = context_object as *const ani_rs::objects::AniObject as *mut ani_rs::objects::AniObject
            as *mut AniObject;
        let ret = unsafe { ffi::ShowAppNetPolicySettings(env, context_object) };
        if ret != 0 {
            crate::policy_error!("wrapper::show_app_net_policy_settings failed, ret={}", ret);
            return Err(ret);
        }
        crate::policy_info!("wrapper::show_app_net_policy_settings success");
        Ok(())
    }

    pub fn register_policy_observer() -> Result<(), i32> {
        let res = ffi::RegisterNetPolicyChangeCallback();
        if res != 0 {
            return Err(res);
        }
        Ok(())
    }

    pub fn unregister_policy_observer() -> Result<(), i32> {
        let res = ffi::UnregisterNetPolicyChangeCallback();
        if res != 0 {
            return Err(res);
        }
        Ok(())
    }

    pub fn check_policy_permission() -> Result<(), i32> {
        let res = ffi::CheckPolicyPermission();
        if res != 0 {
            return Err(res);
        }
        Ok(())
    }
}

impl From<bridge::NetQuotaPolicy> for ffi::NetQuotaPolicyAni {
    fn from(policy: bridge::NetQuotaPolicy) -> Self {
        ffi::NetQuotaPolicyAni {
            network_match_rule: ffi::NetworkMatchRuleAni {
                net_type: policy.network_match_rule.net_type as i32,
                identity: policy.network_match_rule.identity,
                sim_id: policy.network_match_rule.sim_id,
            },
            quota_policy: ffi::QuotaPolicyAni {
                period_duration: policy.quota_policy.period_duration,
                warning_bytes: policy.quota_policy.warning_bytes,
                limit_bytes: policy.quota_policy.limit_bytes,
                metered: policy.quota_policy.metered,
                limit_action: policy.quota_policy.limit_action as i32,
                last_warning_remind: policy.quota_policy.last_warning_remind.unwrap_or(0),
                last_limit_remind: policy.quota_policy.last_limit_remind.unwrap_or(0),
            },
        }
    }
}

impl From<ffi::NetQuotaPolicyAni> for bridge::NetQuotaPolicy {
    fn from(policy: ffi::NetQuotaPolicyAni) -> Self {
        bridge::NetQuotaPolicy {
            network_match_rule: bridge::NetworkMatchRule {
                net_type: bridge::NetBearType::from(policy.network_match_rule.net_type),
                identity: policy.network_match_rule.identity,
                sim_id: policy.network_match_rule.sim_id,
            },
            quota_policy: bridge::QuotaPolicy {
                period_duration: policy.quota_policy.period_duration,
                warning_bytes: policy.quota_policy.warning_bytes,
                limit_bytes: policy.quota_policy.limit_bytes,
                metered: policy.quota_policy.metered,
                limit_action: bridge::LimitAction::from(policy.quota_policy.limit_action),
                last_warning_remind: Some(policy.quota_policy.last_warning_remind),
                last_limit_remind: Some(policy.quota_policy.last_limit_remind),
            },
        }
    }
}

impl From<bridge::NetworkAccessPolicy> for ffi::NetworkAccessPolicyAni {
    fn from(policy: bridge::NetworkAccessPolicy) -> Self {
        ffi::NetworkAccessPolicyAni {
            allow_wi_fi: policy.allow_wi_fi.unwrap_or(false),
            allow_cellular: policy.allow_cellular.unwrap_or(false),
            always_allow_wi_fi: policy.always_allow_wi_fi.unwrap_or(false),
            always_allow_cellular: policy.always_allow_cellular.unwrap_or(false),
        }
    }
}

impl From<ffi::NetworkAccessPolicyAni> for bridge::NetworkAccessPolicy {
    fn from(policy: ffi::NetworkAccessPolicyAni) -> Self {
        bridge::NetworkAccessPolicy {
            allow_wi_fi: Some(policy.allow_wi_fi),
            allow_cellular: Some(policy.allow_cellular),
            always_allow_wi_fi: if policy.always_allow_wi_fi { Some(true) } else { None },
            always_allow_cellular: if policy.always_allow_cellular { Some(true) } else { None },
        }
    }
}

impl From<ffi::NetAccessPolicyAni> for bridge::NetAccessPolicy {
    fn from(policy: ffi::NetAccessPolicyAni) -> Self {
        bridge::NetAccessPolicy {
            allow_wi_fi: policy.allow_wi_fi,
            allow_cellular: policy.allow_cellular,
        }
    }
}

impl From<ffi::NetUidRuleInfoAni> for bridge::NetUidRuleInfo {
    fn from(info: ffi::NetUidRuleInfoAni) -> Self {
        bridge::NetUidRuleInfo {
            uid: info.uid,
            rule: bridge::NetUidRule::from(info.rule),
        }
    }
}

impl From<ffi::NetUidPolicyInfoAni> for bridge::NetUidPolicyInfo {
    fn from(info: ffi::NetUidPolicyInfoAni) -> Self {
        bridge::NetUidPolicyInfo {
            uid: info.uid,
            policy: bridge::NetUidPolicy::from(info.policy),
        }
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
    pub mod ffi {
        pub struct NetworkMatchRuleAni {
        pub net_type: i32,
        pub identity: String,
        pub sim_id: String,
    }

    pub struct QuotaPolicyAni {
        pub period_duration: String,
        pub warning_bytes: i64,
        pub limit_bytes: i64,
        pub metered: bool,
        pub limit_action: i32,
        pub last_warning_remind: i64,
        pub last_limit_remind: i64,
    }

    pub struct NetQuotaPolicyAni {
        pub network_match_rule: NetworkMatchRuleAni,
        pub quota_policy: QuotaPolicyAni,
    }

    pub struct NetAccessPolicyAni {
        pub allow_wi_fi: bool,
        pub allow_cellular: bool,
    }

    pub struct NetUidRuleInfoAni {
        pub uid: i32,
        pub rule: i32,
    }
    pub struct NetUidPolicyInfoAni {
        pub uid: i32,
        pub policy: i32,
    }
    pub struct MeteredIfacesAni {
        pub ifaces: Vec<String>,
    }

    pub struct NetworkAccessPolicyAni {
        pub allow_wi_fi: bool,
        pub allow_cellular: bool,
        pub always_allow_wi_fi: bool,
        pub always_allow_cellular: bool,
    }

    pub struct UidNetworkPolicyAni {
        pub uid: i32,
        pub policy: NetworkAccessPolicyAni,
    }

    extern "Rust" {
        type AniEnv;
        type AniObject;

        pub fn execute_net_quota_policy_changed(policies: Vec<NetQuotaPolicyAni>);
        pub fn execute_net_background_policy_changed(allow: bool);
        pub fn execute_net_metered_ifaces_changed(info: MeteredIfacesAni);
        pub fn execute_net_uid_rule_changed(info: NetUidRuleInfoAni);
        pub fn execute_net_uid_policy_changed(info: NetUidPolicyInfoAni);
    }

    unsafe extern "C++" {
        include!("policy_ani.h");

        fn SetBackgroundAllowed(allowed: bool) -> i32;
        fn GetBackgroundPolicy(backgroundPolicy: &mut bool) -> i32;
        fn GetBackgroundPolicyByUid(uid: i32, backgroundPolicyOfUid: &mut u32) -> i32;
        fn SetPolicyByUid(uid: i32, policy: u32) -> i32;
        fn GetPolicyByUid(uid: i32, policy: &mut u32) -> i32;
        fn GetUidsByPolicy(policy: u32, result: &mut Vec<u32>) -> i32;
        fn IsUidNetAllowed(uid: i32, metered: bool, isAllowed: &mut bool) -> i32;
        fn IsUidNetAllowedByIface(uid: i32, iface: String, isAllowed: &mut bool) -> i32;
        fn SetNetQuotaPolicies(policies: Vec<NetQuotaPolicyAni>) -> i32;
        fn RestoreAllPolicies(iccid: String) -> i32;
        fn GetNetQuotaPolicies(result: &mut Vec<NetQuotaPolicyAni>) -> i32;
        fn UpdateRemindPolicy(netType: i32, iccid: String, remindType: u32) -> i32;
        fn SetDeviceIdleTrustlist(uids: Vec<i32>, isAllowed: bool) -> i32;
        fn GetDeviceIdleTrustlist(result: &mut Vec<u32>) -> i32;
        fn SetPowerTrustlist(uids: Vec<i32>, isAllowed: bool) -> i32;
        fn GetPowerSaveTrustlist(result: &mut Vec<u32>) -> i32;
        fn SetNetworkAccessPolicyTyped(uid: i32, policy: NetworkAccessPolicyAni,
            reconfirmFlag: bool) -> i32;
        fn GetNetworkAccessPolicyByUid(uid: i32, result: &mut NetworkAccessPolicyAni) -> i32;
        fn GetAllNetworkAccessPolicies(result: &mut Vec<UidNetworkPolicyAni>) -> i32;
        fn GetSelfNetworkAccessPolicy(result: &mut NetAccessPolicyAni) -> i32;
        unsafe fn ShowAppNetPolicySettings(env: *mut *mut AniEnv, context: *mut AniObject) -> i32;

        fn RegisterNetPolicyChangeCallback() -> i32;
        fn UnregisterNetPolicyChangeCallback() -> i32;
        fn CheckPolicyPermission() -> i32;

        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}