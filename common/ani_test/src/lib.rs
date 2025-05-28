// Copyright (C) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "license");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "aS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

mod ani_enum;
mod ani_union;
mod array;
mod array_buffer;
mod option;
mod primitive;
mod record;

ani_rs::ani_constructor!(
    namespace "Lanirs/test/ani_test"
    [
        "primitiveTest" : primitive::primitive_test,
        "optionBool" : option::option_bool,
        "optionByte" : option::option_byte,
        "optionShort" : option::option_i16,
        "optionInt" : option::option_i32,
        "optionLong" : option::option_i64,
        "optionDouble" : option::option_f64,
        "arrayBool":  array::array_bool,
        "arrayByte" : array::array_byte,
        "arrayShort" :  array::array_i16,
        "arrayInt":  array::array_i32,
        "arrayLong":  array::array_i64,
        "arrayFloat":  array::array_f32,
        "arrayDouble" :  array::array_f64,
        "enumTestNumber": ani_enum::enum_test_number,
        "enumTestString": ani_enum::enum_test_string,
        "recordString" : record::record_string,
        "recordLong" : record::record_long,
        "unionTest" : ani_union::union_test,
        "arrayBufferTest": array_buffer::array_buffer_test,
        "uint8ArrayTest": array_buffer::uint8_array_test,
    ]
);
