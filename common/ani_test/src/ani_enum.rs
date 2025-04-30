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

#[ani_rs::ani(path = "Lani/rs/test/EnumNumber;")]
enum EnumNumber {
    One = 1,
    Two = 2,
    Three = 3,
}

#[ani_rs::ani(path = "Lani/rs/test/EnumString;")]
enum EnumString {
    One = 1,
    Two = 2,
    Three = 3,
}

pub const ENUM_TEST_NUMBER: &CStr = cstr(b"EnumTestNumber\0");
pub const ENUM_TEST_STRING: &CStr = cstr(b"EnumTestString\0");

pub fn enum_test_number<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: EnumNumber = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}

pub fn enum_test_string<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    ani_obj: ani_rs::objects::AniObject<'local>,
) -> ani_rs::objects::AniRef<'local> {
    let input: EnumString = env.deserialize(ani_obj).unwrap();
    env.serialize(&input).unwrap()
}
