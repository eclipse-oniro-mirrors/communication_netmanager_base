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

use crate::bridge::*;
use crate::error_code::convert_to_business_error;
use crate::wrapper::NetPolicyClient;
use ani_rs::business_error::BusinessError;
use ani_rs::objects::{AniObject, AniRef};
use ani_rs::AniEnv;

#[ani_rs::native]
pub fn set_background_allowed(is_allowed: bool) -> Result<(), BusinessError> {
    NetPolicyClient::set_background_allowed(is_allowed).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn is_background_allowed() -> Result<bool, BusinessError> {
    NetPolicyClient::get_background_policy().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_background_policy_by_uid(uid: i32) -> Result<NetBackgroundPolicy, BusinessError> {
    NetPolicyClient::get_background_policy_by_uid(uid).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_policy_by_uid(uid: i32, policy: NetUidPolicy) -> Result<(), BusinessError> {
    NetPolicyClient::set_policy_by_uid(uid, policy).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_policy_by_uid(uid: i32) -> Result<NetUidPolicy, BusinessError> {
    NetPolicyClient::get_policy_by_uid(uid).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_uids_by_policy(policy: NetUidPolicy) -> Result<Vec<i32>, BusinessError> {
    NetPolicyClient::get_uids_by_policy(policy).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn is_uid_net_allowed(uid: i32, is_metered: bool) -> Result<bool, BusinessError> {
    NetPolicyClient::is_uid_net_allowed(uid, is_metered).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn is_uid_net_allowed_by_iface(uid: i32, iface: String) -> Result<bool, BusinessError> {
    NetPolicyClient::is_uid_net_allowed_by_iface(uid, iface).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_net_quota_policies(policies: Vec<NetQuotaPolicy>) -> Result<(), BusinessError> {
    NetPolicyClient::set_net_quota_policies(policies).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn restore_all_policies(iccid: String) -> Result<(), BusinessError> {
    NetPolicyClient::restore_all_policies(iccid).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_net_quota_policies() -> Result<Vec<NetQuotaPolicy>, BusinessError> {
    NetPolicyClient::get_net_quota_policies().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn update_remind_policy(
    net_type: NetBearType,
    iccid: String,
    remind_type: RemindType,
) -> Result<(), BusinessError> {
    NetPolicyClient::update_remind_policy(net_type, iccid, remind_type)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_device_idle_trustlist(uids: Vec<i32>, is_allowed: bool) -> Result<(), BusinessError> {
    NetPolicyClient::set_device_idle_trustlist(uids, is_allowed).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_device_idle_trustlist() -> Result<Vec<i32>, BusinessError> {
    NetPolicyClient::get_device_idle_trustlist().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_power_save_trustlist(uids: Vec<i32>, is_allowed: bool) -> Result<(), BusinessError> {
    NetPolicyClient::set_power_save_trustlist(uids, is_allowed).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_power_save_trustlist() -> Result<Vec<i32>, BusinessError> {
    NetPolicyClient::get_power_save_trustlist().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn reset_policies(iccid: String) -> Result<(), BusinessError> {
    NetPolicyClient::reset_policies(iccid).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn set_network_access_policy(
    uid: i32,
    policy: NetworkAccessPolicy,
    reconfirm_flag: Option<bool>,
) -> Result<(), BusinessError> {
    let reconfirm_flag = reconfirm_flag.unwrap_or(false);
    NetPolicyClient::set_network_access_policy(uid, policy, reconfirm_flag).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_network_access_policy_by_uid(uid: i32) -> Result<NetworkAccessPolicy, BusinessError> {
    NetPolicyClient::get_network_access_policy(uid).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_all_network_access_policies() -> Result<UidNetworkAccessPolicy, BusinessError> {
    NetPolicyClient::get_all_network_access_policies().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_net_access_policy() -> Result<NetAccessPolicy, BusinessError> {
    NetPolicyClient::get_self_network_access_policy().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn show_app_net_policy_settings<'local>(
    env: &AniEnv<'local>,
    context: AniRef<'local>,
) -> Result<(), BusinessError> {
    let context_obj = AniObject::from(context);
    NetPolicyClient::show_app_net_policy_settings(env, &context_obj).map_err(convert_to_business_error)
}