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

use std::sync::{Arc, Mutex, OnceLock};

use ani_rs::{
    business_error::BusinessError,
    objects::{AniFnObject, GlobalRefCallback},
    AniEnv,
};

use crate::wrapper::{ffi, NetStatsClient, StatisCallbackUnregister};

struct Registar {
    inner: Mutex<Vec<(Arc<CallbackFlavor>, StatisCallbackUnregister)>>,
}

use crate::bridge;

impl Registar {
    fn new() -> Self {
        Self {
            inner: Mutex::new(Vec::new()),
        }
    }

    pub fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<Registar> = OnceLock::new();
        INSTANCE.get_or_init(Registar::new)
    }

    pub fn register(&self, callback: CallbackFlavor) -> Result<(), i32> {
        let mut inner = self.inner.lock().unwrap();
        for w in inner.iter() {
            if *w.0 == callback {
                return Err(-1);
            }
        }
        let callback = Arc::new(callback);
        let unregister =
            NetStatsClient::register_statis_callback(StatisticsCallback::new(callback.clone()))?;
        inner.push((callback, unregister));
        Ok(())
    }

    pub fn unregister_callback(&self, callback: &CallbackFlavor) -> Result<(), i32> {
        let mut ret = 0;
        self.inner.lock().unwrap().retain_mut(|cb| {
            if *cb.0 == *callback {
                if let Err(e) = cb.1.unregister() {
                    ret = e;
                };
                return false;
            }
            true
        });
        Ok(())
    }

    pub fn unregister_net_states_change(
        &self,
        callback: Option<&CallbackFlavor>,
    ) -> Result<(), i32> {
        if let Some(callback) = callback {
            self.unregister_callback(callback)
        } else {
            let mut ret = 0;
            self.inner.lock().unwrap().retain_mut(|cb| {
                if let CallbackFlavor::NetStatesChange(_) = &*cb.0 {
                    if let Err(e) = cb.1.unregister() {
                        ret = e;
                        true
                    } else {
                        false
                    }
                } else {
                    true
                }
            });
            if ret != 0 {
                return Err(ret);
            }
            Ok(())
        }
    }
}

#[ani_rs::native]
pub fn on_net_states_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::NetStatesChange(callback);
    Registar::get_instance().register(flavor).map_err(|err| {
        BusinessError::new(
            err,
            "Failed to register net state change callback".to_string(),
        )
    })
}

#[ani_rs::native]
pub fn off_net_states_change(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    if env.is_undefined(&callback.clone().into()).unwrap() {
        return Registar::get_instance()
            .unregister_net_states_change(None)
            .map_err(|err| {
                BusinessError::new(
                    err,
                    "Failed to unregister net state change callback".to_string(),
                )
            });
    }
    let callback = callback.into_global_callback(env).unwrap();
    let flavor = CallbackFlavor::NetStatesChange(callback);
    Registar::get_instance()
        .unregister_net_states_change(Some(&flavor))
        .map_err(|err| {
            BusinessError::new(
                err,
                "Failed to unregister net state change callback".to_string(),
            )
        })
}

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    NetStatesChange(GlobalRefCallback<(bridge::NetStatsChangeInfo,)>),
}

#[derive(PartialEq, Eq)]
pub struct StatisticsCallback {
    inner: Arc<CallbackFlavor>,
}

impl StatisticsCallback {
    fn new(flavor: Arc<CallbackFlavor>) -> Self {
        Self { inner: flavor }
    }

    pub fn net_iface_stats_changed(&self, info: ffi::NetStatsChangeInfo) -> i32 {
        if let CallbackFlavor::NetStatesChange(callback) = &*self.inner {
            callback.execute((info.into(),));
        }
        0
    }

    pub fn net_uid_stats_changed(&self, info: ffi::NetStatsChangeInfo) -> i32 {
        if let CallbackFlavor::NetStatesChange(callback) = &*self.inner {
            callback.execute((info.into(),));
        }
        0
    }
}
