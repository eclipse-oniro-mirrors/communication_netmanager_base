// Copyright (C) 2025 Huawei Device Co., Ltd.
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

use std::{ffi::CStr, mem};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniFnObject, AniRef, GlobalRefCallback, AniObject},
    AniEnv, error::AniError,
};

use crate::{
    bridge::{
        Cleaner, ConnectionProperties, HttpProxy, NetAddress, NetBlockStatusInfo, NetCapabilities,
        NetCapabilityInfo, NetConnection, NetConnectionPropertyInfo, NetHandle, NetSpecifier,
    },
    error_code::convert_to_business_error,
    wrapper::{check_permission, ConnUnregisterHandle, NetConnClient}, connection_info,
};

const INTERNET_PERMISSION: &str = "ohos.permission.INTERNET";
const NATIVE_PTR: &CStr = unsafe { CStr::from_bytes_with_nul_unchecked(b"nativePtr\0") };

#[ani_rs::native]
pub(crate) fn get_default_net() -> Result<NetHandle, BusinessError> {
    NetConnClient::get_default_net_handle().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_all_nets() -> Result<Vec<NetHandle>, BusinessError> {
    NetConnClient::get_all_nets().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn has_default_net() -> Result<bool, BusinessError> {
    NetConnClient::has_default_net().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_net_capabilities(
    net_handle: NetHandle,
) -> Result<NetCapabilities, BusinessError> {
    NetConnClient::get_net_capabilities(net_handle).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_default_http_proxy() -> Result<HttpProxy, BusinessError> {
    NetConnClient::get_default_http_proxy().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_global_http_proxy() -> Result<HttpProxy, BusinessError> {
    NetConnClient::get_global_http_proxy().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn enable_airplane_mode() -> Result<(), BusinessError> {
    NetConnClient::set_airplane_mode(true).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn disable_airplane_mode() -> Result<(), BusinessError> {
    NetConnClient::set_airplane_mode(false).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_app_net() -> Result<NetHandle, BusinessError> {
    NetConnClient::get_app_net()
        .map(|net_id| NetHandle { net_id })
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn set_app_net(net_handle: NetHandle) -> Result<(), BusinessError> {
    NetConnClient::set_app_net(net_handle.net_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_pac_url() -> Result<String, BusinessError> {
    NetConnClient::get_pac_url().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn set_pac_url(pac_url: String) -> Result<(), BusinessError> {
    NetConnClient::set_pac_url(&pac_url).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn factory_reset_network() -> Result<(), BusinessError> {
    NetConnClient::factory_reset_network().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn is_default_net_metered() -> Result<bool, BusinessError> {
    NetConnClient::is_default_net_metered().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_connection_properties(
    net_handle: NetHandle,
) -> Result<ConnectionProperties, BusinessError> {
    NetConnClient::get_connection_properties(net_handle.net_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_addresses_by_name(host: String) -> Result<Vec<NetAddress>, BusinessError> {
    NetConnClient::get_addresses_by_name(&host, 0).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_address_by_name_with_handle(
    this: NetHandle,
    host: String,
) -> Result<NetAddress, BusinessError> {
    NetConnClient::get_address_by_name(&host, this.net_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn get_addresses_by_name_with_handle(
    this: NetHandle,
    host: String,
) -> Result<Vec<NetAddress>, BusinessError> {
    NetConnClient::get_addresses_by_name(&host, this.net_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn set_global_http_proxy(proxy: HttpProxy) -> Result<(), BusinessError> {
    NetConnClient::set_global_http_proxy(proxy).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn set_app_http_proxy(proxy: HttpProxy) -> Result<(), BusinessError> {
    NetConnClient::set_app_http_proxy(proxy).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn bind_socket(this: NetHandle, socket: i32) -> Result<(), BusinessError> {
    NetConnClient::bind_socket(socket, this.net_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn net_detection(net_handle: NetHandle) -> Result<(), BusinessError> {
    NetConnClient::net_detection(net_handle.net_id).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn clear_custom_dns_rules() -> Result<(), BusinessError> {
    if !check_permission(INTERNET_PERMISSION) {
        return Err(BusinessError::PERMISSION);
    }
    NetConnClient::clear_custom_dns_rules().map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn set_custom_dns_rule(host: String, ips: Vec<String>) -> Result<(), BusinessError> {
    if !check_permission(INTERNET_PERMISSION) {
        return Err(BusinessError::PERMISSION);
    }
    NetConnClient::set_custom_dns_rules(host, ips).map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn remove_custom_dns_rule(host: String) -> Result<(), BusinessError> {
    if !check_permission(INTERNET_PERMISSION) {
        return Err(BusinessError::PERMISSION);
    }
    NetConnClient::remove_custom_dns_rule(host).map_err(convert_to_business_error)
}

pub struct Connection {
    pub net_specifier: Option<NetSpecifier>,
    pub timeout: Option<i32>,
    pub callback: ConnCallback,
    pub unregister: Option<ConnUnregisterHandle>,
}

pub struct ConnCallback {
    pub on_net_available: Option<GlobalRefCallback<(NetHandle,)>>,
    pub on_net_block_status_change: Option<GlobalRefCallback<(NetBlockStatusInfo,)>>,
    pub on_net_capabilities_change: Option<GlobalRefCallback<(NetCapabilityInfo,)>>,
    pub on_net_connection_properties_change:
        Option<GlobalRefCallback<(NetConnectionPropertyInfo,)>>,
    pub on_net_lost: Option<GlobalRefCallback<(NetHandle,)>>,
    pub on_net_unavailable: Option<GlobalRefCallback<()>>,
}

impl ConnCallback {
    pub fn new() -> Self {
        Self {
            on_net_available: None,
            on_net_block_status_change: None,
            on_net_capabilities_change: None,
            on_net_connection_properties_change: None,
            on_net_lost: None,
            on_net_unavailable: None,
        }
    }
}

impl Connection {
    pub fn new(net_specifier: Option<NetSpecifier>, timeout: Option<i32>) -> Self {
        Self {
            net_specifier,
            timeout,
            callback: ConnCallback::new(),
            unregister: None,
        }
    }
}

#[ani_rs::native]
pub(crate) fn create_net_connection_ptr(
    net_specifier: Option<NetSpecifier>,
    timeout: Option<i32>,
) -> Result<i64, BusinessError> {
    let connection = Box::new(Connection::new(net_specifier, timeout));
    let ptr = Box::into_raw(connection);
    Ok(ptr as i64)
}

#[ani_rs::native]
pub(crate) fn on_net_available(
    env: &AniEnv,
    obj: AniObject,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };
    connection.callback.on_net_available = Some(callback.into_global_callback(env).unwrap());
    Ok(())
}

#[ani_rs::native]
pub(crate) fn on_net_block_status_change(
    env: &AniEnv,
    obj: AniObject,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };
    connection.callback.on_net_block_status_change =
        Some(callback.into_global_callback(env).unwrap());
    Ok(())
}

#[ani_rs::native]
pub(crate) fn on_net_capabilities_change(
    env: &AniEnv,
    obj: AniObject,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };
    connection.callback.on_net_capabilities_change =
        Some(callback.into_global_callback(env).unwrap());
    Ok(())
}

#[ani_rs::native]
pub(crate) fn on_net_connection_properties_change(
    env: &AniEnv,
    obj: AniObject,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };
    connection.callback.on_net_connection_properties_change =
        Some(callback.into_global_callback(env).unwrap());
    Ok(())
}

#[ani_rs::native]
pub(crate) fn on_net_lost(
    env: &AniEnv,
    obj: AniObject,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };
    connection.callback.on_net_lost = Some(callback.into_global_callback(env).unwrap());
    Ok(())
}

#[ani_rs::native]
pub(crate) fn on_net_unavailable(
    env: &AniEnv,
    obj: AniObject,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };
    connection.callback.on_net_unavailable = Some(callback.into_global_callback(env).unwrap());
    Ok(())
}

#[ani_rs::native]
pub(crate) fn register_network_change(env: &AniEnv, obj: AniObject) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };

    let callback_ref = &mut connection.callback;
    let unregister = NetConnClient::register_net_conn_callback(callback_ref).map_err(|e| {
        BusinessError::new(e, format!("Failed to register network change callback"))
    })?;
    connection.unregister = Some(unregister);

    Ok(())
}

#[ani_rs::native]
pub(crate) fn unregister_network_change(env: &AniEnv, obj: AniObject) -> Result<(), BusinessError> {
    let native_ptr = env.get_field::<i64>(&obj, NATIVE_PTR)?;
    let connection = unsafe { &mut *(native_ptr as *mut Connection) };

    if let Some(unregister) = connection.unregister.as_mut() {
        if let Err(e) = unregister.unregister() {
            return Err(BusinessError::new(
                e,
                format!("Failed to unregister network change callback"),
            ));
        }
        Ok(())
    } else {
        Err(BusinessError::new(
            2101007,
            format!("No network change callback to unregister"),
        ))
    }
}

#[ani_rs::native]
pub(crate) fn connection_clean(this: Cleaner) -> Result<(), BusinessError> {
    let _ = unsafe { Box::from_raw(this.ptr as *mut Connection) };
    Ok(())
}
