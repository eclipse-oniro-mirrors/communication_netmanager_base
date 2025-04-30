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

use std::{ffi::CStr, net};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniObject, AniRef},
    AniDe, AniEnv, AniSer,
};
use serde::{Deserialize, Serialize};

use crate::{
    bridge::{ConnectionProperties, HttpProxy, NetAddress, NetCapabilities, NetHandle},
    wrapper::NetConnClient,
};

#[ani_rs::native]
pub(crate) fn get_default_net() -> Result<NetHandle, BusinessError> {
    NetConnClient::get_default_net_handle()
        .map_err(|e| BusinessError::new(e, format!("Failed to get default net handle")))
}

#[ani_rs::native]
pub(crate) fn get_all_nets() -> Result<Vec<NetHandle>, BusinessError> {
    NetConnClient::get_all_nets()
        .map_err(|e| BusinessError::new(e, format!("Failed to get all nets")))
}

#[ani_rs::native]
pub(crate) fn has_default_net() -> Result<bool, BusinessError> {
    NetConnClient::has_default_net()
        .map_err(|e| BusinessError::new(e, format!("Failed to check default net")))
}

#[ani_rs::native]
pub(crate) fn get_net_capabilities(
    net_handle: NetHandle,
) -> Result<NetCapabilities, BusinessError> {
    NetConnClient::get_net_capabilities(net_handle)
        .map_err(|e| BusinessError::new(e, format!("Failed to get net capabilities")))
}

#[ani_rs::native]
pub(crate) fn get_default_http_proxy() -> Result<HttpProxy, BusinessError> {
    NetConnClient::get_default_http_proxy()
        .map_err(|e| BusinessError::new(e, format!("Failed to get default http proxy")))
}

#[ani_rs::native]
pub(crate) fn get_global_http_proxy() -> Result<HttpProxy, BusinessError> {
    NetConnClient::get_global_http_proxy()
        .map_err(|e| BusinessError::new(e, format!("Failed to get global http proxy")))
}

#[ani_rs::native_v]
pub(crate) fn enable_airplane_mode() -> Result<(), BusinessError> {
    NetConnClient::set_airplane_mode(true)
        .map_err(|e| BusinessError::new(e, format!("Failed to enable airplane mode")))
}

#[ani_rs::native_v]
pub(crate) fn disable_airplane_mode() -> Result<(), BusinessError> {
    NetConnClient::set_airplane_mode(false)
        .map_err(|e| BusinessError::new(e, format!("Failed to disable airplane mode")))
}

#[ani_rs::native]
pub(crate) fn get_app_net() -> Result<NetHandle, BusinessError> {
    NetConnClient::get_app_net()
        .map(|net_id| NetHandle { net_id })
        .map_err(|e| BusinessError::new(e, format!("Failed to get app net")))
}

#[ani_rs::native_v]
pub(crate) fn set_app_net(net_handle: NetHandle) -> Result<(), BusinessError> {
    NetConnClient::set_app_net(net_handle.net_id)
        .map_err(|e| BusinessError::new(e, format!("Failed to set app net")))
}

#[ani_rs::native]
pub(crate) fn get_pac_url() -> Result<String, BusinessError> {
    NetConnClient::get_pac_url()
        .map_err(|e| BusinessError::new(e, format!("Failed to get PAC URL")))
}

#[ani_rs::native_v]
pub(crate) fn set_pac_url(pac_url: String) -> Result<(), BusinessError> {
    NetConnClient::set_pac_url(&pac_url)
        .map_err(|e| BusinessError::new(e, format!("Failed to set PAC URL")))
}

#[ani_rs::native_v]
pub(crate) fn factory_reset_network() -> Result<(), BusinessError> {
    NetConnClient::factory_reset_network()
        .map_err(|e| BusinessError::new(e, format!("Failed to factory reset network")))
}

#[ani_rs::native]
pub(crate) fn is_default_net_metered() -> Result<bool, BusinessError> {
    NetConnClient::is_default_net_metered()
        .map_err(|e| BusinessError::new(e, format!("Failed to check if default net is metered")))
}

#[ani_rs::native]
pub(crate) fn get_connection_properties(
    net_handle: NetHandle,
) -> Result<ConnectionProperties, BusinessError> {
    NetConnClient::get_connection_properties(net_handle.net_id)
        .map_err(|e| BusinessError::new(e, format!("Failed to get connection properties")))
}

#[ani_rs::native]
pub(crate) fn get_addresses_by_name(host: String) -> Result<Vec<NetAddress>, BusinessError> {
    let net_handle = NetConnClient::get_default_net_handle()
        .map_err(|e| BusinessError::new(e, format!("Failed to get default net handle")))?;
    NetConnClient::get_addresses_by_name(&host, net_handle.net_id)
        .map_err(|e| BusinessError::new(e, format!("Failed to get addresses by name")))
}

#[ani_rs::native_v]
pub(crate) fn set_global_http_proxy(proxy: HttpProxy) -> Result<(), BusinessError> {
    NetConnClient::set_global_http_proxy(proxy)
        .map_err(|e| BusinessError::new(e, format!("Failed to set HTTP proxy")))
}
