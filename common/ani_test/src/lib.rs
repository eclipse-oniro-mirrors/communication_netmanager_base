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

use ani_rs::AniVm;
use primitive::PRIMITIVE_TEST;
use std::ffi::{c_uint, CStr};

mod ani_callback;
mod ani_enum;
mod ani_union;
mod array;
mod array_buffer;
mod option;
mod primitive;
mod record;
const ANI_TEST_NAMESPACE: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"Lani/rs/test/ani_test;\0") };

pub(crate) const fn cstr(input: &[u8]) -> &CStr {
    unsafe { CStr::from_bytes_with_nul_unchecked(input) }
}

#[no_mangle]
pub extern "C" fn ANI_Constructor(vm: AniVm, result: *mut u32) -> c_uint {
    unsafe {
        let env = vm.get_env().unwrap();
        AniVm::init(vm);

        let namespace = env.find_namespace(ANI_TEST_NAMESPACE).unwrap();

        let methods = [
            (PRIMITIVE_TEST, primitive::primitive_test as _),
            (option::OPTION_BOOL, option::option_bool as _),
            (option::OPTION_I8, option::option_byte as _),
            (option::OPTION_I16, option::option_i16 as _),
            (option::OPTION_I32, option::option_i32 as _),
            (option::OPTION_I64, option::option_i64 as _),
            (option::OPTION_F64, option::option_f64 as _),
            (array::ARRAY_BOOL, array::array_bool as _),
            (array::ARRAY_I8, array::array_byte as _),
            (array::ARRAY_I16, array::array_i16 as _),
            (array::ARRAY_I32, array::array_i32 as _),
            (array::ARRAY_I64, array::array_i64 as _),
            (array::ARRAY_F32, array::array_f32 as _),
            (array::ARRAY_F64, array::array_f64 as _),
            (ani_enum::ENUM_TEST_NUMBER, ani_enum::enum_test_number as _),
            (ani_enum::ENUM_TEST_STRING, ani_enum::enum_test_string as _),
            (record::RECORD_STRING, record::record_string as _),
            (record::RECORD_LONG, record::record_long as _),
            (ani_union::UNION_TEST, ani_union::union_test as _),
            (
                array_buffer::ARRAY_BUFFER_TEST,
                array_buffer::array_buffer_test as _,
            ),
            (
                array_buffer::UINT8_ARRAY_TEST,
                array_buffer::uint8_array_test as _,
            ),
            (
                ani_callback::CALLBACK_TEST,
                ani_callback::callback_test as _,
            ),
        ];

        env.bind_namespace_functions(namespace, &methods).unwrap();
        *result = 1;
    };
    0
}
