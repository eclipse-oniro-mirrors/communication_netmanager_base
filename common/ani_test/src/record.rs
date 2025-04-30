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

use std::{collections::HashMap, ffi::CStr};

use ani_rs::objects::{AniObject, AniRef};

use crate::cstr;

pub const RECORD_STRING: &CStr = cstr(b"RecordString\0");
pub const RECORD_LONG: &CStr = cstr(b"RecordLong\0");

pub fn record_string<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: AniRef<'local>,
    record: AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: HashMap<String, String> = env.deserialize(record).unwrap();
    env.serialize(&input).unwrap()
}

pub fn record_long<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: AniRef<'local>,
    record: AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: HashMap<i64, i64> = env.deserialize(record).unwrap();
    env.serialize(&input).unwrap()
}


