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

#![feature(offset_of)]
#![allow(
    unused,
    missing_docs,
    non_snake_case,
    non_camel_case_types,
    non_upper_case_globals
)]

use std::os::raw::c_uint;
use std::os::raw::c_void;

pub const ANI_VERSION_1: u32 = 1;
pub const ANI_FALSE: u32 = 0;
pub const ANI_TRUE: u32 = 1;
pub const ANI_LOGLEVEL_FATAL: u32 = 0;
pub const ANI_LOGLEVEL_ERROR: u32 = 1;
pub const ANI_LOGLEVEL_WARNING: u32 = 2;
pub const ANI_LOGLEVEL_INFO: u32 = 3;
pub const ANI_LOGLEVEL_DEBUG: u32 = 4;

pub type va_list = __builtin_va_list;

pub type ani_size = usize;
pub type ani_boolean = u8;
pub type ani_char = u16;
pub type ani_byte = i8;
pub type ani_short = i16;
pub type ani_int = i32;
pub type ani_long = i64;
pub type ani_float = f32;
pub type ani_double = f64;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_ref {
    _unused: [u8; 0],
}

pub type ani_ref = *mut __ani_ref;
pub type ani_module = ani_ref;
pub type ani_namespace = ani_ref;
pub type ani_object = ani_ref;
pub type ani_fn_object = ani_object;
pub type ani_enum_item = ani_object;
pub type ani_error = ani_object;
pub type ani_tuple_value = ani_object;
pub type ani_type = ani_object;
pub type ani_arraybuffer = ani_object;
pub type ani_string = ani_object;
pub type ani_class = ani_type;
pub type ani_enum = ani_type;
pub type ani_union = ani_type;
pub type ani_array = ani_object;
pub type ani_array_boolean = ani_array;
pub type ani_array_char = ani_array;
pub type ani_array_byte = ani_array;
pub type ani_array_short = ani_array;
pub type ani_array_int = ani_array;
pub type ani_array_long = ani_array;
pub type ani_array_float = ani_array;
pub type ani_array_double = ani_array;
pub type ani_array_ref = ani_array;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_wref {
    _unused: [u8; 0],
}
pub type ani_wref = *mut __ani_wref;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_variable {
    _unused: [u8; 0],
}
pub type ani_variable = *mut __ani_variable;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_function {
    _unused: [u8; 0],
}
pub type ani_function = *mut __ani_function;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_field {
    _unused: [u8; 0],
}
pub type ani_field = *mut __ani_field;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_static_field {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_satic_field {
    _unused: [u8; 0],
}
pub type ani_static_field = *mut __ani_satic_field;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_method {
    _unused: [u8; 0],
}
pub type ani_method = *mut __ani_method;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_static_method {
    _unused: [u8; 0],
}
pub type ani_static_method = *mut __ani_static_method;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_resolver {
    _unused: [u8; 0],
}
pub type ani_resolver = *mut __ani_resolver;
pub type ani_finalizer = ::std::option::Option<
    unsafe extern "C" fn(data: *mut ::std::os::raw::c_void, hint: *mut ::std::os::raw::c_void),
>;
#[repr(C)]
#[derive(Copy, Clone)]
pub union ani_value {
    pub z: ani_boolean,
    pub c: ani_char,
    pub b: ani_byte,
    pub s: ani_short,
    pub i: ani_int,
    pub l: ani_long,
    pub f: ani_float,
    pub d: ani_double,
    pub r: ani_ref,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct ani_native_function {
    pub name: *const ::std::os::raw::c_char,
    pub signature: *const ::std::os::raw::c_char,
    pub pointer: *const ::std::os::raw::c_void,
}

pub type ani_vm = *const __ani_vm_api;
pub type ani_env = *const __ani_interaction_api;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct ani_option {
    pub option: *const ::std::os::raw::c_char,
    pub extra: *mut ::std::os::raw::c_void,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct ani_options {
    pub nr_options: usize,
    pub options: *const ani_option,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_vm_api {
    pub reserved0: *mut ::std::os::raw::c_void,
    pub reserved1: *mut ::std::os::raw::c_void,
    pub reserved2: *mut ::std::os::raw::c_void,
    pub reserved3: *mut ::std::os::raw::c_void,
    pub DestroyVM: ::std::option::Option<unsafe extern "C" fn(vm: *mut ani_vm) -> c_uint>,
    pub GetEnv: ::std::option::Option<
        unsafe extern "C" fn(vm: *mut ani_vm, version: u32, result: *mut *mut ani_env) -> c_uint,
    >,
    pub AttachCurrentThread: ::std::option::Option<
        unsafe extern "C" fn(
            vm: *mut ani_vm,
            options: *const ani_options,
            version: u32,
            result: *mut *mut ani_env,
        ) -> c_uint,
    >,
    pub DetachCurrentThread: ::std::option::Option<unsafe extern "C" fn(vm: *mut ani_vm) -> c_uint>,
}

extern "C" {
    pub fn ANI_CreateVM(
        options: *const ani_options,
        version: u32,
        result: *mut *mut ani_vm,
    ) -> c_uint;
}
extern "C" {
    pub fn ANI_GetCreatedVMs(
        vms_buffer: *mut *mut ani_vm,
        vms_buffer_length: ani_size,
        result: *mut ani_size,
    ) -> c_uint;
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_interaction_api {
    pub reserved0: *mut ::std::os::raw::c_void,
    pub reserved1: *mut ::std::os::raw::c_void,
    pub reserved2: *mut ::std::os::raw::c_void,
    pub reserved3: *mut ::std::os::raw::c_void,

    pub GetVersion:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, result: *mut u32) -> c_uint>,

    pub GetVM: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, result: *mut *mut ani_vm) -> c_uint,
    >,

    pub Object_New: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_method,
            result: *mut ani_object,
            ...
        ) -> c_uint,
    >,

    pub Object_New_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_method,
            result: *mut ani_object,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_New_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_method,
            result: *mut ani_object,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_GetType: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            result: *mut ani_type,
        ) -> c_uint,
    >,

    pub Object_InstanceOf: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            type_: ani_type,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Type_GetSuperClass: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, type_: ani_type, result: *mut ani_class) -> c_uint,
    >,

    pub Type_IsAssignableFrom: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            from_type: ani_type,
            to_type: ani_type,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub FindModule: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            module_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_module,
        ) -> c_uint,
    >,

    pub FindNamespace: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            namespace_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_namespace,
        ) -> c_uint,
    >,

    pub FindClass: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            class_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_class,
        ) -> c_uint,
    >,

    pub FindEnum: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enum_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_enum,
        ) -> c_uint,
    >,

    pub Module_FindNamespace: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            module: ani_module,
            namespace_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_namespace,
        ) -> c_uint,
    >,

    pub Module_FindClass: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            module: ani_module,
            class_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_class,
        ) -> c_uint,
    >,

    pub Module_FindEnum: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            module: ani_module,
            enum_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_enum,
        ) -> c_uint,
    >,

    pub Module_FindFunction: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            module: ani_module,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_function,
        ) -> c_uint,
    >,

    pub Module_FindVariable: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            module: ani_module,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_variable,
        ) -> c_uint,
    >,

    pub Namespace_FindNamespace: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ns: ani_namespace,
            namespace_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_namespace,
        ) -> c_uint,
    >,

    pub Namespace_FindClass: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ns: ani_namespace,
            class_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_class,
        ) -> c_uint,
    >,

    pub Namespace_FindEnum: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ns: ani_namespace,
            enum_descriptor: *const ::std::os::raw::c_char,
            result: *mut ani_enum,
        ) -> c_uint,
    >,

    pub Namespace_FindFunction: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ns: ani_namespace,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_function,
        ) -> c_uint,
    >,

    pub Namespace_FindVariable: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ns: ani_namespace,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_variable,
        ) -> c_uint,
    >,

    pub Module_BindNativeFunctions: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            module: ani_module,
            functions: *const ani_native_function,
            nr_functions: ani_size,
        ) -> c_uint,
    >,

    pub Namespace_BindNativeFunctions: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ns: ani_namespace,
            functions: *const ani_native_function,
            nr_functions: ani_size,
        ) -> c_uint,
    >,
    pub Class_BindNativeMethods: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            methods: *const ani_native_function,
            nr_methods: ani_size,
        ) -> c_uint,
    >,

    pub Reference_Delete:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, ref_: ani_ref) -> c_uint>,

    pub EnsureEnoughReferences:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, nr_refs: ani_size) -> c_uint>,

    pub CreateLocalScope:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, nr_refs: ani_size) -> c_uint>,

    pub DestroyLocalScope: ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env) -> c_uint>,

    pub CreateEscapeLocalScope:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, nr_refs: ani_size) -> c_uint>,

    pub DestroyEscapeLocalScope: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, ref_: ani_ref, result: *mut ani_ref) -> c_uint,
    >,

    pub ThrowError:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, err: ani_error) -> c_uint>,

    pub ExistUnhandledError: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, result: *mut ani_boolean) -> c_uint,
    >,

    pub GetUnhandledError: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, result: *mut ani_error) -> c_uint,
    >,

    pub ResetError: ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env) -> c_uint>,

    pub DescribeError: ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env) -> c_uint>,

    pub Abort: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, message: *const ::std::os::raw::c_char) -> c_uint,
    >,

    pub GetNull: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, result: *mut ani_ref) -> c_uint,
    >,

    pub GetUndefined: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, result: *mut ani_ref) -> c_uint,
    >,

    pub Reference_IsNull: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, ref_: ani_ref, result: *mut ani_boolean) -> c_uint,
    >,

    pub Reference_IsUndefined: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, ref_: ani_ref, result: *mut ani_boolean) -> c_uint,
    >,

    pub Reference_IsNullishValue: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, ref_: ani_ref, result: *mut ani_boolean) -> c_uint,
    >,

    pub Reference_Equals: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ref0: ani_ref,
            ref1: ani_ref,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Reference_StrictEquals: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            ref0: ani_ref,
            ref1: ani_ref,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub String_NewUTF16: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            utf16_string: *const u16,
            utf16_size: ani_size,
            result: *mut ani_string,
        ) -> c_uint,
    >,

    pub String_GetUTF16Size: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            string: ani_string,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub String_GetUTF16: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            string: ani_string,
            utf16_buffer: *mut u16,
            utf16_buffer_size: ani_size,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub String_GetUTF16SubString: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            string: ani_string,
            substr_offset: ani_size,
            substr_size: ani_size,
            utf16_buffer: *mut u16,
            utf16_buffer_size: ani_size,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub String_NewUTF8: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            utf8_string: *const ::std::os::raw::c_char,
            utf8_size: ani_size,
            result: *mut ani_string,
        ) -> c_uint,
    >,

    pub String_GetUTF8Size: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            string: ani_string,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub String_GetUTF8: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            string: ani_string,
            utf8_buffer: *mut ::std::os::raw::c_char,
            utf8_buffer_size: ani_size,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub String_GetUTF8SubString: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            string: ani_string,
            substr_offset: ani_size,
            substr_size: ani_size,
            utf8_buffer: *mut ::std::os::raw::c_char,
            utf8_buffer_size: ani_size,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub Array_GetLength: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, array: ani_array, result: *mut ani_size) -> c_uint,
    >,

    pub Array_New_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_boolean,
        ) -> c_uint,
    >,

    pub Array_New_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_char,
        ) -> c_uint,
    >,

    pub Array_New_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_byte,
        ) -> c_uint,
    >,

    pub Array_New_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_short,
        ) -> c_uint,
    >,

    pub Array_New_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_int,
        ) -> c_uint,
    >,

    pub Array_New_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_long,
        ) -> c_uint,
    >,

    pub Array_New_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_float,
        ) -> c_uint,
    >,

    pub Array_New_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: ani_size,
            result: *mut ani_array_double,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_boolean,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_char,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_char,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_byte,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_byte,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_short,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_short,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_int,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_int,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_long,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_long,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_float,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_float,
        ) -> c_uint,
    >,

    pub Array_GetRegion_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_double,
            offset: ani_size,
            length: ani_size,
            native_buffer: *mut ani_double,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_boolean,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_boolean,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_char,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_char,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_byte,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_byte,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_short,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_short,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_int,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_int,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_long,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_long,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_float,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_float,
        ) -> c_uint,
    >,

    pub Array_SetRegion_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_double,
            offset: ani_size,
            length: ani_size,
            native_buffer: *const ani_double,
        ) -> c_uint,
    >,

    pub Array_New_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            type_: ani_type,
            length: ani_size,
            initial_element: ani_ref,
            result: *mut ani_array_ref,
        ) -> c_uint,
    >,

    pub Array_Set_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_ref,
            index: ani_size,
            ref_: ani_ref,
        ) -> c_uint,
    >,

    pub Array_Get_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            array: ani_array_ref,
            index: ani_size,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Enum_GetEnumItemByName: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enm: ani_enum,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_enum_item,
        ) -> c_uint,
    >,

    pub Enum_GetEnumItemByIndex: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enm: ani_enum,
            index: ani_size,
            result: *mut ani_enum_item,
        ) -> c_uint,
    >,

    pub EnumItem_GetEnum: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enum_item: ani_enum_item,
            result: *mut ani_enum,
        ) -> c_uint,
    >,

    pub EnumItem_GetValue_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enum_item: ani_enum_item,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub EnumItem_GetValue_String: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enum_item: ani_enum_item,
            result: *mut ani_string,
        ) -> c_uint,
    >,

    pub EnumItem_GetName: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enum_item: ani_enum_item,
            result: *mut ani_string,
        ) -> c_uint,
    >,

    pub EnumItem_GetIndex: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            enum_item: ani_enum_item,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub FunctionalObject_Call: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_fn_object,
            argc: ani_size,
            argv: *mut ani_ref,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Variable_SetValue_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            value: ani_boolean,
        ) -> c_uint,
    >,

    pub Variable_SetValue_Char: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, variable: ani_variable, value: ani_char) -> c_uint,
    >,

    pub Variable_SetValue_Byte: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, variable: ani_variable, value: ani_byte) -> c_uint,
    >,

    pub Variable_SetValue_Short: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, variable: ani_variable, value: ani_short) -> c_uint,
    >,

    pub Variable_SetValue_Int: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, variable: ani_variable, value: ani_int) -> c_uint,
    >,

    pub Variable_SetValue_Long: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, variable: ani_variable, value: ani_long) -> c_uint,
    >,

    pub Variable_SetValue_Float: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, variable: ani_variable, value: ani_float) -> c_uint,
    >,

    pub Variable_SetValue_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            value: ani_double,
        ) -> c_uint,
    >,

    pub Variable_SetValue_Ref: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, variable: ani_variable, value: ani_ref) -> c_uint,
    >,

    pub Variable_GetValue_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_char,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_byte,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_short,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_long,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_float,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_double,
        ) -> c_uint,
    >,

    pub Variable_GetValue_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            variable: ani_variable,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Function_Call_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_boolean,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Boolean_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_boolean,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Boolean_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_boolean,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_char,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Char_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_char,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Char_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_char,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_byte,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Byte_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_byte,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Byte_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_byte,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_short,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Short_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_short,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Short_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_short,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_int,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Int_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_int,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Int_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_int,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_long,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Long_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_long,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Long_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_long,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_float,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Float_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_float,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Float_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_float,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_double,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Double_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_double,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Double_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_double,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_ref,
            ...
        ) -> c_uint,
    >,

    pub Function_Call_Ref_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_ref,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Ref_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            result: *mut ani_ref,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Function_Call_Void: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, fn_: ani_function, ...) -> c_uint,
    >,

    pub Function_Call_Void_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Function_Call_Void_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            fn_: ani_function,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_FindField: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_field,
        ) -> c_uint,
    >,

    pub Class_FindStaticField: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_static_field,
        ) -> c_uint,
    >,

    pub Class_FindMethod: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_method,
        ) -> c_uint,
    >,

    pub Class_FindStaticMethod: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_static_method,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_char,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_byte,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_short,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_long,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_float,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_double,
        ) -> c_uint,
    >,

    pub Class_GetStaticField_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_boolean,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_char,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_byte,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_short,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_int,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_long,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_float,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_double,
        ) -> c_uint,
    >,

    pub Class_SetStaticField_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            field: ani_static_field,
            value: ani_ref,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_char,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_short,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_long,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_float,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_double,
        ) -> c_uint,
    >,

    pub Class_GetStaticFieldByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_boolean,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_char,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_byte,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_short,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_int,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_long,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_float,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_double,
        ) -> c_uint,
    >,

    pub Class_SetStaticFieldByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            value: ani_ref,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_boolean,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Boolean_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_boolean,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Boolean_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_boolean,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_char,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Char_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_char,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Char_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_char,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_byte,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Byte_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_byte,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Byte_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_byte,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_short,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Short_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_short,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Short_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_short,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_int,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Int_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_int,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Int_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_int,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_long,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Long_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_long,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Long_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_long,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_float,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Float_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_float,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Float_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_float,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_double,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Double_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_double,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Double_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_double,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_ref,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Ref_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_ref,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Ref_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            result: *mut ani_ref,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Void: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Void_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethod_Void_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            method: ani_static_method,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Boolean_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Boolean_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_char,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Char_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_char,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Char_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_char,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Byte_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Byte_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_short,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Short_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_short,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Short_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_short,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_int,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Int_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_int,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Int_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_int,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_long,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Long_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_long,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Long_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_long,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_float,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Float_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_float,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Float_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_float,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_double,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Double_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_double,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Double_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_double,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Ref_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Ref_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Void: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            ...
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Void_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Class_CallStaticMethodByName_Void_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            cls: ani_class,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_GetField_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Object_GetField_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_char,
        ) -> c_uint,
    >,

    pub Object_GetField_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_byte,
        ) -> c_uint,
    >,

    pub Object_GetField_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_short,
        ) -> c_uint,
    >,

    pub Object_GetField_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub Object_GetField_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_long,
        ) -> c_uint,
    >,

    pub Object_GetField_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_float,
        ) -> c_uint,
    >,

    pub Object_GetField_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_double,
        ) -> c_uint,
    >,

    pub Object_GetField_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Object_SetField_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_boolean,
        ) -> c_uint,
    >,

    pub Object_SetField_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_char,
        ) -> c_uint,
    >,

    pub Object_SetField_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_byte,
        ) -> c_uint,
    >,

    pub Object_SetField_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_short,
        ) -> c_uint,
    >,

    pub Object_SetField_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_int,
        ) -> c_uint,
    >,

    pub Object_SetField_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_long,
        ) -> c_uint,
    >,

    pub Object_SetField_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_float,
        ) -> c_uint,
    >,

    pub Object_SetField_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_double,
        ) -> c_uint,
    >,

    pub Object_SetField_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            field: ani_field,
            value: ani_ref,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_char,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_short,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_long,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_float,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_double,
        ) -> c_uint,
    >,

    pub Object_GetFieldByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_boolean,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_char,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_byte,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_short,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_int,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_long,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_float,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_double,
        ) -> c_uint,
    >,

    pub Object_SetFieldByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_ref,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_char,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_short,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_long,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_float,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_double,
        ) -> c_uint,
    >,

    pub Object_GetPropertyByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_boolean,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_char,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_byte,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_short,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_int,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_long,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_float,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_double,
        ) -> c_uint,
    >,

    pub Object_SetPropertyByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            value: ani_ref,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_boolean,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Boolean_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_boolean,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Boolean_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_boolean,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_char,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Char_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_char,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Char_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_char,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_byte,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Byte_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_byte,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Byte_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_byte,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_short,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Short_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_short,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Short_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_short,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_int,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Int_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_int,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Int_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_int,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_long,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Long_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_long,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Long_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_long,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_float,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Float_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_float,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Float_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_float,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_double,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Double_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_double,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Double_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_double,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_ref,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Ref_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_ref,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Ref_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            result: *mut ani_ref,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Void: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethod_Void_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethod_Void_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            method: ani_method,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Boolean_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Boolean_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_boolean,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_char,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Char_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_char,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Char_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_char,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Byte_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Byte_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_byte,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_short,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Short_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_short,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Short_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_short,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_int,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Int_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_int,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Int_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_int,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_long,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Long_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_long,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Long_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_long,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_float,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Float_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_float,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Float_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_float,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_double,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Double_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_double,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Double_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_double,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Ref_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Ref_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            result: *mut ani_ref,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Void: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            ...
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Void_A: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            args: *const ani_value,
        ) -> c_uint,
    >,

    pub Object_CallMethodByName_Void_V: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            object: ani_object,
            name: *const ::std::os::raw::c_char,
            signature: *const ::std::os::raw::c_char,
            args: *mut __va_list_tag,
        ) -> c_uint,
    >,

    pub TupleValue_GetNumberOfItems: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            result: *mut ani_size,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_boolean,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_char,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_byte,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_short,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_int,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_long,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_float,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_double,
        ) -> c_uint,
    >,

    pub TupleValue_GetItem_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Boolean: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_boolean,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Char: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_char,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Byte: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_byte,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Short: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_short,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Int: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_int,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Long: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_long,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Float: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_float,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Double: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_double,
        ) -> c_uint,
    >,

    pub TupleValue_SetItem_Ref: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            tuple_value: ani_tuple_value,
            index: ani_size,
            value: ani_ref,
        ) -> c_uint,
    >,

    pub GlobalReference_Create: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, ref_: ani_ref, result: *mut ani_ref) -> c_uint,
    >,

    pub GlobalReference_Delete:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, gref: ani_ref) -> c_uint>,

    pub WeakReference_Create: ::std::option::Option<
        unsafe extern "C" fn(env: *mut ani_env, ref_: ani_ref, result: *mut ani_wref) -> c_uint,
    >,

    pub WeakReference_Delete:
        ::std::option::Option<unsafe extern "C" fn(env: *mut ani_env, wref: ani_wref) -> c_uint>,

    pub WeakReference_GetReference: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            wref: ani_wref,
            was_released_result: *mut ani_boolean,
            ref_result: *mut ani_ref,
        ) -> c_uint,
    >,

    pub CreateArrayBuffer: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            length: usize,
            data_result: *mut *mut ::std::os::raw::c_void,
            arraybuffer_result: *mut ani_arraybuffer,
        ) -> c_uint,
    >,

    pub CreateArrayBufferExternal: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            external_data: *mut ::std::os::raw::c_void,
            length: usize,
            finalizer: ani_finalizer,
            hint: *mut ::std::os::raw::c_void,
            result: *mut ani_arraybuffer,
        ) -> c_uint,
    >,

    pub ArrayBuffer_GetInfo: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            arraybuffer: ani_arraybuffer,
            data_result: *mut *mut ::std::os::raw::c_void,
            length_result: *mut usize,
        ) -> c_uint,
    >,

    pub Promise_New: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            result_resolver: *mut ani_resolver,
            result_promise: *mut ani_object,
        ) -> c_uint,
    >,

    pub PromiseResolver_Resolve: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            resolver: ani_resolver,
            resolution: ani_ref,
        ) -> c_uint,
    >,

    pub PromiseResolver_Reject: ::std::option::Option<
        unsafe extern "C" fn(
            env: *mut ani_env,
            resolver: ani_resolver,
            rejection: ani_error,
        ) -> c_uint,
    >,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_vm {
    pub c_api: *const __ani_vm_api,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __ani_env {
    pub c_api: *const __ani_interaction_api,
}

pub type __builtin_va_list = [__va_list_tag; 1usize];

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __va_list_tag {
    pub gp_offset: ::std::os::raw::c_uint,
    pub fp_offset: ::std::os::raw::c_uint,
    pub overflow_arg_area: *mut ::std::os::raw::c_void,
    pub reg_save_area: *mut ::std::os::raw::c_void,
}
