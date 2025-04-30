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

pub const PRIMITIVE_TEST: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"PrimitiveTest\0") };

#[ani_rs::ani(path = "Lani/rs/test/PrimitiveTest;")]
struct PrimitiveTest {
    primitive_bool: bool,
    primitive_i8: i8,
    primitive_i16: i16,
    primitive_i32: i32,
    primitive_i64: i64,
    primitive_f32: f32,
    primitive_f64: f64,
}

pub fn primitive_test<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> AniRef<'local> {
    let input: PrimitiveTest = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}
