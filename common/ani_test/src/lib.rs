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

mod ani_enum;
mod ani_union;
mod array;
mod array_buffer;
mod option;
mod primitive;
mod record;

ani_rs::ani_constructor!(
    namespace "Lani/rs/test/ani_test"
    [
        "PrimitiveTest" : primitive::primitive_test,
        "OptionBool" : option::option_bool,
        "OptionByte" : option::option_byte,
        "OptionShort" : option::option_i16,
        "OptionInt" : option::option_i32,
        "OptionLong" : option::option_i64,
        "OptionDouble" : option::option_f64,
        "ArrayBool":  array::array_bool,
        "ArrayByte" : array::array_byte,
        "ArrayShort" :  array::array_i16,
        "ArrayInt":  array::array_i32,
        "ArrayLong":  array::array_i64,
        "ArrayFloat":  array::array_f32,
        "ArrayDouble" :  array::array_f64,
        "EnumTestNumber ": ani_enum::enum_test_number,
        "EnumTestString ": ani_enum::enum_test_string,
        "RecordString" : record::record_string,
        "RecordLong" : record::record_long,
        "UnionTest" : ani_union::union_test,
        "ArrayBufferTest": array_buffer::array_buffer_test,
        "Uint8ArrayTest": array_buffer::uint8_array_test,
    ]
);
