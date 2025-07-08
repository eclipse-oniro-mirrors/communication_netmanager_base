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

use serde::{Deserialize, Serialize};

use crate::{error::AniError, objects::AniClass, signature, AniEnv};

#[derive(Debug)]
pub enum TypedArray {
    Int8,
    Int16,
    Int32,
    Uint8,
    Uint16,
    Uint32,
}

impl TypedArray {
    pub fn ani_class<'local>(&self, env: &AniEnv<'local>) -> Result<AniClass<'local>, AniError> {
        let class_name: &'static CStr = match self {
            TypedArray::Int8 => signature::INT8_ARRAY,
            TypedArray::Int16 => signature::INT16_ARRAY,
            TypedArray::Int32 => signature::INT32_ARRAY,
            TypedArray::Uint8 => signature::UINT8_ARRAY,
            TypedArray::Uint16 => signature::UINT16_ARRAY,
            TypedArray::Uint32 => signature::UINT32_ARRAY,
        };
        env.find_class(class_name)
    }
}

macro_rules! typed_array {
    ($ftype:ident) => {
        #[derive(Serialize, Deserialize)]
        pub struct $ftype<'local>(&'local [u8]);

        impl<'local> $ftype<'local> {
            pub fn new(input: &'local [u8]) -> Self {
                $ftype(input)
            }

            pub fn as_slice(&self) -> &'local [u8] {
                self.0
            }

            pub fn len(&self) -> usize {
                self.0.len()
            }
        }
    };
}

typed_array!(Int8Array);
typed_array!(Int16Array);
typed_array!(Int32Array);

typed_array!(Uint8Array);
typed_array!(Uint16Array);
typed_array!(Uint32Array);
