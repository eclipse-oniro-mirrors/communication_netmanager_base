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

use ani_rs::{callback::Callback, context::get_context};

use crate::cstr;

pub const CALLBACK_TEST: &CStr = cstr(b"CallbackTest\0");

pub fn callback_test<'local>(
    env: ani_rs::AniEnv<'local>,
    _ani_this: ani_rs::objects::AniRef<'local>,
    callback: ani_rs::objects::AniObject<'local>,
) {
    get_context(&env, callback);
}
