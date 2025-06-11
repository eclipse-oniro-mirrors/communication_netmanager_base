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

use ani_sys::{ani_fn_object, ani_object};
use serde::Serialize;
use std::{
    ops::Deref,
    sync::{Arc, Once},
};

use crate::{
    error::AniError,
    global::GlobalRef,
    objects::{AniObject, AniRef},
    AniEnv, AniVm,
};

#[repr(transparent)]
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct AniFnObject<'local>(AniObject<'local>);

impl<'local> AsRef<AniFnObject<'local>> for AniFnObject<'local> {
    fn as_ref(&self) -> &AniFnObject<'local> {
        &self
    }
}

impl<'local> AsMut<AniFnObject<'local>> for AniFnObject<'local> {
    fn as_mut(&mut self) -> &mut AniFnObject<'local> {
        self
    }
}

impl<'local> Deref for AniFnObject<'local> {
    type Target = AniObject<'local>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl<'local> From<AniFnObject<'local>> for AniObject<'local> {
    fn from(value: AniFnObject<'local>) -> Self {
        value.0
    }
}

impl<'local> From<AniFnObject<'local>> for AniRef<'local> {
    fn from(value: AniFnObject<'local>) -> Self {
        value.0.into()
    }
}

impl<'local> From<AniRef<'local>> for AniFnObject<'local> {
    fn from(value: AniRef<'local>) -> Self {
        Self::from_raw(value.as_raw() as ani_fn_object)
    }
}

impl<'local> From<AniObject<'local>> for AniFnObject<'local> {
    fn from(value: AniObject<'local>) -> Self {
        Self::from_raw(value.into_raw())
    }
}

impl<'local> AniFnObject<'local> {
    pub fn from_raw(ptr: ani_fn_object) -> Self {
        Self(AniObject::from_raw(ptr as ani_object))
    }

    pub fn as_raw(&self) -> ani_fn_object {
        self.0.as_raw() as _
    }

    pub fn into_raw(self) -> ani_fn_object {
        self.0.into_raw() as _
    }

    pub fn into_global(self, env: &AniEnv) -> Result<GlobalRef<AniFnObject<'static>>, AniError> {
        let global = env.create_global_ref(self.into())?;
        let fn_object = AniFnObject::from_raw(global.as_raw() as ani_fn_object);
        Ok(GlobalRef(fn_object))
    }
}

impl<'local> AniFnObject<'local> {
    pub fn execute_local<const N: usize, T>(
        &self,
        env: &AniEnv<'local>,
        input: T,
    ) -> Result<AniRef, AniError>
    where
        T: Input<N>,
    {
        let input = input.input(&env);
        env.function_object_call(&self, &input)
    }

    pub fn execute_current<const N: usize, T>(&self, input: T) -> Result<AniRef, AniError>
    where
        T: Input<N>,
    {
        if let Ok(env) = AniVm::get_instance().get_env() {
            return self.execute_local(&env, input);
        }
        let env = AniVm::get_instance().attach_current_thread()?;
        let input = input.input(&env);
        let res = env.function_object_call(&self, &input);
        res
    }

    pub fn into_global_callback<T: InputVec + Send + 'static>(
        self,
        env: &AniEnv<'local>,
    ) -> Result<GlobalRefCallback<T>, AniError> {
        let global_ref = self.into_global(env)?;
        Ok(GlobalRefCallback {
            inner: Arc::new(global_ref),
            phantom: std::marker::PhantomData::<T>,
        })
    }
}

impl GlobalRef<AniFnObject<'static>> {
    pub fn execute_global<T>(self: &Arc<Self>, input: T)
    where
        Self: 'static,
        T: InputVec + Send + 'static,
    {
        thread_local! {
            pub static ONCE:Once = Once::new();
        }
        let me = self.clone();
        ylong_runtime::spawn_blocking(move || {
            ONCE.with(|a| {
                a.call_once(|| {
                    AniVm::get_instance().attach_current_thread().unwrap();
                });
            });
            if let Ok(env) = AniVm::get_instance().get_env() {
                let input = input.input(&env);
                let _ = env.function_object_call(&me.0, &input);
            } else {
                if let Ok(env) = AniVm::get_instance().attach_current_thread() {
                    let input = input.input(&env);
                    let _ = env.function_object_call(&me.0, &input);
                }
            }
        });
    }
}

#[derive(Clone)]
pub struct GlobalRefCallback<T: InputVec + Send + 'static> {
    inner: Arc<GlobalRef<AniFnObject<'static>>>,
    phantom: std::marker::PhantomData<T>,
}

impl<T: InputVec + Send + 'static> PartialEq for GlobalRefCallback<T> {
    fn eq(&self, other: &Self) -> bool {
        self.inner == other.inner
    }
}

impl<T: InputVec + Send + 'static> Eq for GlobalRefCallback<T> {}

impl<T: InputVec + Send + 'static> GlobalRefCallback<T> {
    pub fn execute(&self, input: T) {
        self.inner.execute_global(input);
    }
}

pub trait Input<const N: usize> {
    fn input<'local>(&self, env: &AniEnv<'local>) -> [AniRef<'local>; N];
}

pub trait InputVec {
    fn input<'local>(&self, env: &AniEnv<'local>) -> Vec<AniRef<'local>>;
}
impl Input<0> for () {
    fn input<'local>(&self, _env: &AniEnv<'local>) -> [AniRef<'local>; 0] {
        []
    }
}

impl InputVec for () {
    fn input<'local>(&self, _env: &AniEnv<'local>) -> Vec<AniRef<'local>> {
        vec![]
    }
}

macro_rules! single_tuple_impl {
    ( $flen:tt $(($field:tt $ftype:ident)),*) => {
        impl<$($ftype),*> Input<$flen> for ($($ftype,)*)
        where $($ftype: Serialize), *
        {
            fn input<'local>(&self, env: &AniEnv<'local>) -> [AniRef<'local>; $flen] {
                [
                    $(env.serialize(&self.$field).unwrap(),)*
                ]
            }
        }

        impl<$($ftype),*> InputVec for ($($ftype,)*)
        where $($ftype: Serialize), *
        {
            fn input<'local>(&self, env: &AniEnv<'local>) -> Vec<AniRef<'local>> {
                vec![
                    $(env.serialize(&self.$field).unwrap(),)*
                ]
            }
        }

    };
}

single_tuple_impl!(1 (0 A));
single_tuple_impl!(2 (0 A), (1 B));
single_tuple_impl!(3 (0 A), (1 B), (2 C));
single_tuple_impl!(4 (0 A), (1 B), (2 C), (3 D));
single_tuple_impl!(5 (0 A), (1 B), (2 C), (3 D), (4 E));
single_tuple_impl!(6 (0 A), (1 B), (2 C), (3 D), (4 E), (5 F));
