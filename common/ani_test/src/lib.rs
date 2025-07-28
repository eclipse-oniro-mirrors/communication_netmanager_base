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
mod ani_struct;
mod ani_callback;
mod business_error;

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
        "unionTest2" : ani_union::union_test2,
        "arrayBufferTest": array_buffer::array_buffer_test,
        "changeArrayBuffer": array_buffer::change_array_buffer,
        "createArrayBuffer": array_buffer::create_array_buffer,
        "uint8ArrayTest": array_buffer::uint8_array_test,
        "int8ArrayTest": array_buffer::int8_array_test,
        "uint16ArrayTest": array_buffer::uint16_array_test,
        "int16ArrayTest": array_buffer::int16_array_test,
        "uint32ArrayTest": array_buffer::uint32_array_test,
        "int32ArrayTest": array_buffer::int32_array_test,
        "changeUint8Array": array_buffer::change_uint8_array,
        "changeInt8Array": array_buffer::change_int8_array,
        "changeUint16Array": array_buffer::change_uint16_array,
        "changeInt16Array": array_buffer::change_int16_array,
        "changeUint32Array": array_buffer::change_uint32_array,
        "changeInt32Array": array_buffer::change_int32_array,
        "createInt8Array": array_buffer::create_int8_array,
        "createUint8Array": array_buffer::create_uint8_array,
        "createInt16Array": array_buffer::create_int16_array,
        "createUint16Array": array_buffer::create_uint16_array,
        "createInt32Array": array_buffer::create_int32_array,
        "createUint32Array": array_buffer::create_uint32_array,
        "structEnum": ani_struct::struct_enum,
        "enumTestStruct": ani_enum::enum_test_struct,
        "executeCallback1": ani_callback::execute_callback1,
        "executeCallback2": ani_callback::execute_callback2,
        "executeCallback3": ani_callback::execute_callback3,
        "executeCallback4": ani_callback::execute_callback4,
        "executeAsyncCallback1": ani_callback::execute_async_callback1,
        "executeAsyncCallback2": ani_callback::execute_async_callback2,
        "executeAsyncCallback3": ani_callback::execute_async_callback3,
        "executeAsyncCallback4": ani_callback::execute_async_callback4,
        "businessErrorTest": business_error::business_error_test,
    ]
);
