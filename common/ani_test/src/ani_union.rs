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

use ani_rs::objects::{AniObject, AniRef};
use serde::{Deserialize, Serialize};

use crate::cstr;

#[derive(Serialize, Deserialize)]
enum Data<'local> {
    Boolean(bool),
    I8(i8),
    I16(i16),
    I32(i32),
    I64(i64),
    F64(f64),
    S(String),
    ArrayBuffer(&'local [u8]),
    Null(()),
}

pub const UNION_TEST: &CStr = cstr(b"UnionTest\0");

pub fn union_test<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: AniRef<'local>,
    ani_obj: AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: Data<'local> = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}
