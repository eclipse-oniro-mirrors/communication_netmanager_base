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
    typed_array::Uint8Array,
    AniEnv,
};

use crate::cstr;

pub const ARRAY_BUFFER_TEST: &CStr = cstr(b"ArrayBufferTest\0");
pub const UINT8_ARRAY_TEST: &CStr = cstr(b"Uint8ArrayTest\0");

pub fn array_buffer_test<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    input: AniObject<'local>,
) -> AniRef<'local> {
    let input: Uint8Array = env.deserialize(input).unwrap();

    env.serialize(&input).unwrap()
}

pub fn uint8_array_test<'local>(
    env: AniEnv<'local>,
    _ani_this: AniRef<'local>,
    input: AniObject<'local>,
) -> AniRef<'local> {
    let input: Uint8Array<'local> = env.deserialize(input).unwrap();
    println!("uint8_array_test: {:?}", input.as_slice());
    env.serialize(&input).unwrap()
}
