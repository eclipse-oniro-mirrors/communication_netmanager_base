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

use ani_rs::{business_error::BusinessError, typed_array::Uint8Array};

#[ani_rs::native]
pub fn array_buffer_test<'local>(
    input: Uint8Array<'local>,
) -> Result<Uint8Array<'local>, BusinessError> {
    Ok(input)
}

#[ani_rs::native]
pub fn uint8_array_test<'local>(
    input: Uint8Array<'local>,
) -> Result<Uint8Array<'local>, BusinessError> {
    Ok(input)
}
