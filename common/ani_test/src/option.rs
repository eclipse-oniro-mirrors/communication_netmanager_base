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

use std::ffi::CStr;

use ani_rs::{
    objects::{AniObject, AniRef},
    AniEnv,
};

use crate::cstr;

pub const OPTION_BOOL: &CStr = cstr(b"OptionBool\0");
pub const OPTION_I8: &CStr = cstr(b"OptionByte\0");
pub const OPTION_I16: &CStr = cstr(b"OptionShort\0");
pub const OPTION_I32: &CStr = cstr(b"OptionInt\0");
pub const OPTION_I64: &CStr = cstr(b"OptionLong\0");
pub const OPTION_F64: &CStr = cstr(b"OptionDouble\0");

pub fn option_bool<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> AniRef<'local> {
    let input: Option<bool> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn option_byte<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> AniRef<'local> {
    let input: Option<i8> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn option_i16<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> AniRef<'local> {
    let input: Option<i16> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn option_i32<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> AniRef<'local> {
    let input: Option<i32> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn option_i64<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> AniRef<'local> {
    let input: Option<i64> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn option_f64<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> AniRef<'local> {
    let input: Option<f64> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}