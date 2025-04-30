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

#![feature(fn_ptr_trait)]
#![allow(unused, missing_docs, clippy::not_unsafe_ptr_arg_deref)]

mod bridge;
mod connection;
pub mod wrapper;

use ani_rs::business_error::BusinessError;
use ani_rs::objects::{AniObject, AniRef};
use ani_rs::{AniDe, AniSer};
use hilog_rust::{info, HiLogLabel, LogLevel, LogType};
use serde::{Deserialize, Serialize};
use std::ffi::{CStr, CString};
use std::fmt::format;
use std::marker::FnPtr;
use std::os::raw::{c_char, c_void};
use std::{ffi::c_uint, ptr::null_mut};
use std::{prelude::*, vec};

use hilog_rust::hilog;

use ani_rs::ani_constructor;
use ani_rs::{objects::AniNativeFunction, AniEnv, AniVm};
use ani_sys::{ani_native_function, ani_ref};

ani_constructor!(
    namespace "L@ohos/net/connection/connection"
    [
        "getDefaultNetSync" : connection::get_default_net,
        "getAllNetsSync" : connection::get_all_nets,
        "hasDefaultNetSync" : connection::has_default_net,
        "getNetCapabilitiesSync": connection::get_net_capabilities,
        "getDefaultHttpProxySync": connection::get_default_http_proxy,
        "getGlobalHttpProxySync": connection::get_global_http_proxy,
        "setGlobalHttpProxySync": connection::set_global_http_proxy,
        "enableAirplaneModeSync": connection::enable_airplane_mode,
        "disableAirplaneModeSync": connection::disable_airplane_mode,
        "getAppNetSync": connection::get_app_net,
        "setAppNetSync": connection::set_app_net,
        "getPacUrl" : connection::get_pac_url,
        "setPacUrl" : connection::set_pac_url,
        "factoryResetSync" : connection::factory_reset_network,
        "isDefaultNetMeteredSync" : connection::is_default_net_metered,
        "getConnectionPropertiesSync" : connection::get_connection_properties,
        "getAddressesByNameSync" : connection::get_addresses_by_name,
    ]
);
