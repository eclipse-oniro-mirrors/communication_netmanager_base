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

use std::{ffi::CStr, ptr::null_mut};

use ani_sys::{ani_array, ani_boolean, ani_char, ani_float, ani_ref};

use crate::{
    error::AniError,
    objects::{AniObject, AniRef, AniString},
    signature, AniEnv,
};

use super::env::AniExt;

impl AniExt for bool {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value: ani_boolean = 0;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Boolean.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!(
                "Get field failed to retrieve bool value from field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value == 1)
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value: ani_boolean = 0;
        println!("name: {:?}", name);
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Boolean.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };

        if res != 0 {
            let msg = format!(
                "Get Property failed to retrieve bool value from field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value == 1)
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let value = value as ani_boolean;
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Boolean.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!(
                "Set Field failed to set bool value of field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let value = value as ani_boolean;
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Boolean.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!(
                "Set Property failed to set bool value of field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res =
            unsafe { (**env.inner).Array_New_Boolean.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new bool array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let value = value as ani_boolean;
        let res = unsafe {
            (**env.inner).Array_SetRegion_Boolean.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &value as *const ani_boolean,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set bool value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value: ani_boolean = 0;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Boolean.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut ani_boolean,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get bool value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value == 1)
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value: ani_boolean = 0;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Boolean.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut ani_boolean,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox boolean");
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value == 1)
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        let value = value as ani_boolean;
        unsafe {
            let class = env.find_class(signature::BOOLEAN)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"Z:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for i8 {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i8;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Byte.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i8;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Byte.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Byte.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Byte.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res =
            unsafe { (**env.inner).Array_New_Byte.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new byte array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Array_SetRegion_Byte.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &value as *const i8 as _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set byte value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = 0i8;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Byte.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value = 0i8;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Byte.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox byte");
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        unsafe {
            let class = env.find_class(signature::BYTE)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"B:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for i16 {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i16;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Short.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i16;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Short.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Short.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Short.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res =
            unsafe { (**env.inner).Array_New_Short.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new short array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Array_SetRegion_Short.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &value as *const i16 as _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set short value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = 0i16;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Short.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value = 0i16;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Short.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox short");
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        unsafe {
            let class = env.find_class(signature::SHORT)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"S:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for i32 {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i32;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Int.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i32;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Int.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Int.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Int.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res = unsafe { (**env.inner).Array_New_Int.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new int array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Array_SetRegion_Int.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &value as *const i32 as _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set int value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = 0i32;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Int.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value = 0i32;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Int.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox int");
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        unsafe {
            let class = env.find_class(signature::INT)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"I:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for i64 {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i64;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Long.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0i64;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Long.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Long.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Long.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res =
            unsafe { (**env.inner).Array_New_Long.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new long array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Array_SetRegion_Long.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &value as *const i64 as _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set long value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }
    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = 0i64;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Long.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value = 0i64;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Long.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox long");
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        unsafe {
            let class = env.find_class(signature::LONG)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"J:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for f32 {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value: ani_float = 0.0;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Float.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0f32;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Float.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Float.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Float.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res =
            unsafe { (**env.inner).Array_New_Float.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new float array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Array_SetRegion_Float.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &value as *const f32 as _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set float value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = 0f32;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Float.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value = 0f32;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Float.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox float");
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        unsafe {
            let class = env.find_class(signature::FLOAT)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"F:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for f64 {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0f64;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Double.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = 0f64;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Double.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }
    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Double.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Double.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res =
            unsafe { (**env.inner).Array_New_Double.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new double array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Array_SetRegion_Double.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &value as *const f64 as _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to set double value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = 0f64;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Double.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value = 0f64;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Double.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox double");
            Err(AniError::from_code(msg, res))
        } else {
            Ok(value)
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        unsafe {
            let class = env.find_class(signature::DOUBLE)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"D:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for char {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value: ani_char = 0;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Char.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut ani_char,
            )
        };
        if res != 0 {
            let msg = format!(
                "Get field failed to retrieve char value from field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            match char::decode_utf16(vec![value]).next() {
                Some(Ok(c)) => Ok(c),
                _ => {
                    let msg = format!("Failed to decode char from value {}", value);
                    Err(AniError::from_code(msg, res))
                }
            }
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value: ani_char = 0;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Char.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!(
                "Get Property failed to retrieve char value from field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            match char::decode_utf16(vec![value]).next() {
                Some(Ok(c)) => Ok(c),
                _ => {
                    let msg = format!("Failed to decode char from value {}", value);
                    Err(AniError::from_code(msg, res))
                }
            }
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let mut v: [ani_char; 2] = [0; 2];
        let c = value.encode_utf16(&mut v);
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Char.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                c[0],
            )
        };
        if res != 0 {
            let msg = format!(
                "Set Field failed to set char value of field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let mut v: [ani_char; 2] = [0; 2];
        let c = value.encode_utf16(&mut v);
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Char.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                c[0],
            )
        };
        if res != 0 {
            let msg = format!(
                "Set Property failed to set char value of field {}",
                name.to_string_lossy()
            );
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let mut array = null_mut() as ani_array;
        let res =
            unsafe { (**env.inner).Array_New_Char.unwrap()(env.inner, size, &mut array as _) };

        if res != 0 {
            let msg = format!("Failed to create a new char array of size {}", size);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(array))
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value: ani_char = 0;
        let res = unsafe {
            (**env.inner).Array_GetRegion_Char.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get char value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            match char::decode_utf16(vec![value]).next() {
                Some(Ok(c)) => Ok(c),
                _ => {
                    let msg = format!("Failed to decode char from value {}", value);
                    Err(AniError::from_code(msg, res))
                }
            }
        }
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let mut v: [ani_char; 2] = [0; 2];
        let c = value.encode_utf16(&mut v);
        let res = unsafe {
            (**env.inner).Array_SetRegion_Char.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                1,
                c.as_ptr(),
            )
        };
        if res != 0 {
            let msg = format!("Failed to set char value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let mut value: ani_char = 0;
        let res = unsafe {
            (**env.inner).Object_CallMethodByName_Char.unwrap()(
                env.inner,
                ani_ref.as_raw(),
                signature::UNBOX.as_ptr(),
                null_mut(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = String::from("Failed to unbox char");
            Err(AniError::from_code(msg, res))
        } else {
            match char::decode_utf16(vec![value]).next() {
                Some(Ok(c)) => Ok(c),
                _ => {
                    let msg = format!("Failed to decode char from value {}", value);
                    Err(AniError::from_code(msg, res))
                }
            }
        }
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        unsafe {
            let class = env.find_class(signature::CHAR)?;
            env.new_object_with_signature(
                &class,
                CStr::from_bytes_with_nul_unchecked(b"C:V\0"),
                (value,),
            )
            .map(|obj| obj.into())
        }
    }
}

impl AniExt for AniRef<'_> {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = null_mut() as ani_ref;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(value))
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = null_mut() as ani_ref;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(value))
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value.as_raw(),
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                value.as_raw(),
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(_env: &AniEnv<'local>, _size: usize) -> Result<AniRef<'local>, AniError> {
        unimplemented!("use new_array_ref instead");
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let res = unsafe {
            (**env.inner).Array_Set_Ref.unwrap()(env.inner, array.as_raw(), index, value.as_raw())
        };
        if res != 0 {
            let msg = format!("Failed to set ani ref value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = null_mut() as ani_ref;
        let res = unsafe {
            (**env.inner).Array_Get_Ref.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(AniRef::from_raw(value))
        }
    }

    fn unbox(_env: &AniEnv, ani_ref: &AniRef) -> Result<Self, AniError> {
        Ok(AniRef::from_raw(ani_ref.as_raw()))
    }

    fn inbox<'local>(_env: &AniEnv<'local>, ani_ref: Self) -> Result<AniRef<'local>, AniError> {
        Ok(AniRef::from_raw(ani_ref.into_raw()))
    }
}

impl AniExt for String {
    fn get_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = null_mut() as ani_ref;
        let res = unsafe {
            (**env.inner).Object_GetFieldByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            env.convert_ani_string(&AniString::from_raw(value))
        }
    }

    fn get_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
    ) -> Result<Self, AniError> {
        let mut value = null_mut() as ani_ref;
        let res = unsafe {
            (**env.inner).Object_GetPropertyByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            env.convert_ani_string(&AniString::from_raw(value))
        }
    }

    fn set_field<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let ani_s = env.convert_std_string(&value)?;
        let res = unsafe {
            (**env.inner).Object_SetFieldByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                ani_s.as_raw(),
            )
        };
        if res != 0 {
            let msg = format!("Failed to set field {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn set_property<'local>(
        env: &AniEnv<'local>,
        obj: &AniObject<'local>,
        name: &CStr,
        value: Self,
    ) -> Result<(), AniError> {
        let ani_s = env.convert_std_string(&value)?;

        let res = unsafe {
            (**env.inner).Object_SetPropertyByName_Ref.unwrap()(
                env.inner,
                obj.as_raw(),
                name.as_ptr(),
                ani_s.as_raw(),
            )
        };
        if res != 0 {
            let msg = format!("Failed to set property {}", name.to_string_lossy());
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn new_array<'local>(env: &AniEnv<'local>, size: usize) -> Result<AniRef<'local>, AniError> {
        let class = env.find_class(signature::STRING)?;
        env.new_array_ref(&class, size)
    }

    fn array_set<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
        value: Self,
    ) -> Result<(), AniError> {
        let ani_s = env.convert_std_string(&value)?;

        let res = unsafe {
            (**env.inner).Array_Set_Ref.unwrap()(env.inner, array.as_raw(), index, ani_s.as_raw())
        };
        if res != 0 {
            let msg = format!("Failed to set ani string value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            Ok(())
        }
    }

    fn array_get<'local>(
        env: &AniEnv<'local>,
        array: &AniRef<'local>,
        index: usize,
    ) -> Result<Self, AniError> {
        let mut value = null_mut() as ani_ref;
        let res = unsafe {
            (**env.inner).Array_Get_Ref.unwrap()(
                env.inner,
                array.as_raw(),
                index,
                &mut value as *mut _,
            )
        };
        if res != 0 {
            let msg = format!("Failed to get value at index {}", index);
            Err(AniError::from_code(msg, res))
        } else {
            env.convert_ani_string(&AniString::from_raw(value))
        }
    }

    fn unbox<'local>(env: &AniEnv<'local>, ani_ref: &AniRef<'local>) -> Result<Self, AniError> {
        let ani_string = AniString::from_raw(ani_ref.as_raw());
        env.convert_ani_string(&ani_string)
    }

    fn inbox<'local>(env: &AniEnv<'local>, value: Self) -> Result<AniRef<'local>, AniError> {
        env.convert_std_string(&value).map(|ani_s| ani_s.into())
    }
}
