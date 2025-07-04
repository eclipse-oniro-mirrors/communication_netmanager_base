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

use std::thread;

use ani_rs::{business_error::BusinessError, objects::{AniAsyncCallback, AniFnObject}, AniEnv};

#[ani_rs::native]
pub fn execute_callback1(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    callback.execute_local(env, (1,)).unwrap();
    Ok(())
}

#[ani_rs::native]
pub fn execute_callback2(callback: AniFnObject) -> Result<(), BusinessError> {
    callback.execute_current((2,)).unwrap();
    Ok(())
}

#[ani_rs::native]
pub fn execute_callback3(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let global_callback = callback.into_global_callback::<(i32,)>(env).unwrap();
    global_callback.execute_spawn_thread((3,));
    Ok(())
}

#[ani_rs::native]
pub fn execute_callback4(env: &AniEnv, callback: AniFnObject) -> Result<(), BusinessError> {
    let global_callback = callback.into_global_callback::<(i32,)>(env).unwrap();

    thread::spawn(move || {
        global_callback.execute((4,));
    });
    
    Ok(())
}


#[ani_rs::native]
pub fn execute_async_callback1(env: &AniEnv, async_callback: AniAsyncCallback) -> Result<(), BusinessError> {
    let err = BusinessError::new(401, "failed1".to_string());
    async_callback.execute_local(env, Some(err), (1,)).unwrap();
    Ok(())
}

#[ani_rs::native]
pub fn execute_async_callback2(async_callback: AniAsyncCallback) -> Result<(), BusinessError> {
    let err = BusinessError::new(402, "failed2".to_string());
    async_callback.execute_current(Some(err), (2,)).unwrap();
    Ok(())
}

#[ani_rs::native]
pub fn execute_async_callback3(env: &AniEnv, async_callback: AniAsyncCallback) -> Result<(), BusinessError> {
    let err = BusinessError::new(403, "failed3".to_string());
    let global_callback = async_callback.into_global_callback::<(i32,)>(env).unwrap();
    global_callback.execute_spawn_thread(Some(err), (3,));
    Ok(())
}

#[ani_rs::native]
pub fn execute_async_callback4(env: &AniEnv, async_callback: AniAsyncCallback) -> Result<(), BusinessError> {
    let err = BusinessError::new(404, "failed4".to_string());
    let global_callback = async_callback.into_global_callback::<(i32,)>(env).unwrap();
    thread::spawn(move || {
        global_callback.execute(Some(err), (4,));
    });
    
    Ok(())
}