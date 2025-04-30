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

use crate::cstr;

pub const ARRAY_BOOL: &CStr = cstr(b"ArrayBool\0");
pub const ARRAY_I8: &CStr = cstr(b"ArrayByte\0");
pub const ARRAY_I16: &CStr = cstr(b"ArrayShort\0");
pub const ARRAY_I32: &CStr = cstr(b"ArrayInt\0");
pub const ARRAY_I64: &CStr = cstr(b"ArrayLong\0");
pub const ARRAY_F32: &CStr = cstr(b"ArrayFloat\0");
pub const ARRAY_F64: &CStr = cstr(b"ArrayDouble\0");

pub fn array_bool<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Vec<bool> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn array_byte<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Vec<i8> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn array_i16<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Vec<i16> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn array_i32<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Vec<i32> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn array_i64<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Vec<i64> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn array_f32<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Vec<f32> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn array_f64<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Vec<f64> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}
