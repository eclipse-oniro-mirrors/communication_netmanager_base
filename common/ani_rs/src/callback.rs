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

use std::sync::{mpsc::Sender, Arc, OnceLock};

use serde::Serialize;

thread_local! {}

use crate::{
    error::AniError,
    global::GlobalDrop,
    objects::{AniFnObject, AniRef},
    AniEnv, AniVm,
};
static SENDER: OnceLock<
    Sender<(
        Arc<GlobalDrop<AniFnObject<'static>>>,
        Box<dyn InputVec + Send>,
    )>,
> = OnceLock::new();

pub struct Callback<'local, T> {
    inner: AniFnObject<'local>,
    phantom: std::marker::PhantomData<T>,
}

pub struct GlobalCallback<T> {
    inner: Arc<GlobalDrop<AniFnObject<'static>>>,
    phantom: std::marker::PhantomData<T>,
}

impl<'e, 'local, T> Callback<'local, T> {
    pub fn new(inner: AniFnObject<'local>) -> Self {
        Self {
            inner,
            phantom: std::marker::PhantomData,
        }
    }

    pub fn into_global(self, env: AniEnv) -> Result<GlobalCallback<T>, AniError> {
        let global = env.create_global_ref(self.inner.into())?;
        Ok(GlobalCallback {
            inner: Arc::new(GlobalDrop(global.into())),
            phantom: std::marker::PhantomData,
        })
    }

    pub fn execute_local<const N: usize>(
        &self,
        env: AniEnv<'local>,
        input: T,
    ) -> Result<AniRef, AniError>
    where
        T: Input<N>,
    {
        let input = input.input(&env);
        env.function_object_call(&self.inner, &input)
    }

    pub fn execute_current_thread<const N: usize>(
        &self,
        vm: &AniVm,
        input: T,
    ) -> Result<AniRef, AniError>
    where
        T: Input<N>,
    {
        if let Ok(env) = vm.get_env() {
            return self.execute_local(env, input);
        }
        let env = vm.attach_current_thread()?;
        let input = input.input(&env);
        let res = env.function_object_call(&self.inner, &input);
        vm.detach_current_thread()?;
        res
    }
}

impl<'local, T> GlobalCallback<T> {
    pub fn execute_collective(&self, input: T)
    where
        Self: 'static,
        T: InputVec + Send,
    {
        let tx = SENDER.get_or_init(|| {
            let (tx, rx) = std::sync::mpsc::channel::<(
                Arc<GlobalDrop<AniFnObject<'static>>>,
                Box<dyn InputVec + Send>,
            )>();
            std::thread::spawn(move || {
                let env = AniVm::get_instance().attach_current_thread().unwrap();
                while let Ok(fn_obj) = rx.recv() {
                    let input = fn_obj.1.input(&env);
                    let res = env.function_object_call(&fn_obj.0 .0, &input);
                    if let Err(err) = res {
                        eprintln!("Error executing callback: {:?}", err);
                    }
                }
            });
            tx
        });
        tx.send((self.inner.clone(), Box::new(input))).unwrap();
    }
}

pub trait Input<const N: usize> {
    fn input<'local>(&self, env: &AniEnv<'local>) -> [AniRef<'local>; N];
}

pub trait InputVec {
    fn input<'local>(&self, env: &AniEnv<'local>) -> Vec<AniRef<'local>>;
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

single_tuple_impl!(1 (0 A));
single_tuple_impl!(2 (0 A), (1 B));
single_tuple_impl!(3 (0 A), (1 B), (2 C));
single_tuple_impl!(4 (0 A), (1 B), (2 C), (3 D));
single_tuple_impl!(5 (0 A), (1 B), (2 C), (3 D), (4 E));
single_tuple_impl!(6 (0 A), (1 B), (2 C), (3 D), (4 E), (5 F));
