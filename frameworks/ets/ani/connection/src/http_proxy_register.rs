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

use std::sync::{Arc, Mutex, OnceLock};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniFnObject, GlobalRefCallback},
    AniEnv,
};

use crate::{
    bridge::HttpProxy,
    error_code::convert_to_business_error,
    wrapper::{self, NetConnClient},
};

type ProxyCallback = GlobalRefCallback<(HttpProxy,)>;

#[derive(Clone, Copy, PartialEq, Eq)]
enum ProxyType {
    App,
    Default,
}

struct CallbackEntry {
    callback: Arc<ProxyCallback>,
    callback_id: Option<u32>,
    proxy_type: ProxyType,
    token: u64,
}

struct RegistryState {
    entries: Vec<CallbackEntry>,
    next_token: u64,
}

struct ProxyCallbackRegistry {
    state: Mutex<RegistryState>,
}

impl ProxyCallbackRegistry {
    fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<ProxyCallbackRegistry> = OnceLock::new();
        INSTANCE.get_or_init(|| Self {
            state: Mutex::new(RegistryState {
                entries: Vec::new(),
                next_token: 0,
            }),
        })
    }

    fn allocate_token(state: &mut RegistryState) -> u64 {
        loop {
            let token = state.next_token;
            state.next_token = state.next_token.wrapping_add(1);
            if !state.entries.iter().any(|entry| entry.token == token) {
                return token;
            }
        }
    }

    fn register(&self, proxy_type: ProxyType, callback: ProxyCallback) -> Result<(), i32> {
        // Native registration may replay the current proxy synchronously.
        // Publish the callback first so the replay can find it.
        let token = {
            let mut state = self.state.lock().unwrap_or_else(|error| error.into_inner());
            if state
                .entries
                .iter()
                .any(|entry| entry.proxy_type == proxy_type && entry.callback.as_ref() == &callback)
            {
                return Ok(());
            }
            let token = Self::allocate_token(&mut state);
            state.entries.push(CallbackEntry {
                callback: Arc::new(callback),
                callback_id: None,
                proxy_type,
                token,
            });
            token
        };

        let register_result = match proxy_type {
            ProxyType::App => Ok(NetConnClient::register_app_http_proxy_callback(token)),
            ProxyType::Default => NetConnClient::register_default_http_proxy_callback(token),
        };
        let callback_id = match register_result {
            Ok(callback_id) => callback_id,
            Err(error) => {
                let mut state = self
                    .state
                    .lock()
                    .unwrap_or_else(|poison| poison.into_inner());
                state.entries.retain(|entry| entry.token != token);
                return Err(error);
            }
        };

        let registered = {
            let mut state = self.state.lock().unwrap_or_else(|error| error.into_inner());
            if let Some(entry) = state
                .entries
                .iter_mut()
                .find(|entry| entry.token == token)
            {
                entry.callback_id = Some(callback_id);
                true
            } else {
                false
            }
        };
        if !registered {
            // A concurrent off() removed the callback while native registration was in progress.
            Self::unregister_native(proxy_type, callback_id);
        }
        Ok(())
    }

    fn unregister(&self, proxy_type: ProxyType, callback: Option<ProxyCallback>) {
        let callback_ids = {
            let mut state = self.state.lock().unwrap_or_else(|error| error.into_inner());
            let mut callback_ids = Vec::new();
            state.entries.retain(|entry| {
                let should_remove = entry.proxy_type == proxy_type
                    && callback
                        .as_ref()
                        .map_or(true, |target| entry.callback.as_ref() == target);
                if should_remove {
                    if let Some(callback_id) = entry.callback_id {
                        callback_ids.push(callback_id);
                    }
                    false
                } else {
                    true
                }
            });
            callback_ids
        };
        for callback_id in callback_ids {
            Self::unregister_native(proxy_type, callback_id);
        }
    }

    fn unregister_native(proxy_type: ProxyType, callback_id: u32) {
        match proxy_type {
            ProxyType::App => NetConnClient::unregister_app_http_proxy_callback(callback_id),
            ProxyType::Default => NetConnClient::unregister_default_http_proxy_callback(callback_id),
        }
    }

    fn dispatch(&self, proxy_type: ProxyType, token: u64, proxy: wrapper::ffi::HttpProxy) {
        // Keep the callback alive, then release the registry lock before entering the ANI runtime.
        let callback = {
            let state = self.state.lock().unwrap_or_else(|error| error.into_inner());
            state
                .entries
                .iter()
                .find(|entry| entry.proxy_type == proxy_type && entry.token == token)
                .map(|entry| entry.callback.clone())
        };
        if let Some(callback) = callback {
            let mut public_proxy: HttpProxy = proxy.into();
            public_proxy.username = None;
            public_proxy.password = None;
            callback.execute((public_proxy,));
        }
    }
}

fn callback_error(error: impl std::fmt::Display) -> BusinessError {
    BusinessError::new(401, format!("Parameter error: {}", error))
}

fn to_optional_callback(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<Option<ProxyCallback>, BusinessError> {
    if env.is_undefined(&callback).map_err(callback_error)? {
        Ok(None)
    } else {
        callback
            .into_global_callback(env)
            .map(Some)
            .map_err(callback_error)
    }
}

#[ani_rs::native]
pub(crate) fn on_app_http_proxy(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback
        .into_global_callback(env)
        .map_err(callback_error)?;
    ProxyCallbackRegistry::get_instance()
        .register(ProxyType::App, callback)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn off_app_http_proxy(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = to_optional_callback(env, callback)?;
    ProxyCallbackRegistry::get_instance().unregister(ProxyType::App, callback);
    Ok(())
}

#[ani_rs::native]
pub(crate) fn on_default_http_proxy(
    env: &AniEnv,
    callback: AniFnObject,
) -> Result<(), BusinessError> {
    let callback = callback
        .into_global_callback(env)
        .map_err(callback_error)?;
    ProxyCallbackRegistry::get_instance()
        .register(ProxyType::Default, callback)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub(crate) fn off_default_http_proxy(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = to_optional_callback(env, callback)?;
    ProxyCallbackRegistry::get_instance().unregister(ProxyType::Default, callback);
    Ok(())
}

pub fn execute_app_http_proxy_changed(token: u64, proxy: wrapper::ffi::HttpProxy) {
    ProxyCallbackRegistry::get_instance().dispatch(ProxyType::App, token, proxy);
}

pub fn execute_default_http_proxy_changed(token: u64, proxy: wrapper::ffi::HttpProxy) {
    ProxyCallbackRegistry::get_instance().dispatch(ProxyType::Default, token, proxy);
}
