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

use ani_rs::business_error::BusinessError;
use serde::{Deserialize, Serialize};

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

#[ani_rs::native]
pub fn union_test<'local>(input: Data<'local>) -> Result<Data<'local>, BusinessError> {
    Ok(input)
}
