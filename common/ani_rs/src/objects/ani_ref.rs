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

use std::{marker::PhantomData, ops::Deref, ptr::null_mut};

use ani_sys::ani_ref;

#[repr(transparent)]
#[derive(Debug, Clone)]
pub struct AniRef<'local> {
    pub inner: ani_ref,
    lifetime: PhantomData<&'local ()>,
}

unsafe impl Send for AniRef<'static> {}
unsafe impl Sync for AniRef<'static> {}

impl<'local> AsRef<AniRef<'local>> for AniRef<'local> {
    fn as_ref(&self) -> &AniRef<'local> {
        self
    }
}

impl<'local> AsMut<AniRef<'local>> for AniRef<'local> {
    fn as_mut(&mut self) -> &mut AniRef<'local> {
        self
    }
}

impl Deref for AniRef<'_> {
    type Target = ani_ref;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl AniRef<'_> {
    pub fn from_raw(ptr: ani_ref) -> Self {
        Self {
            inner: ptr,
            lifetime: PhantomData,
        }
    }

    pub fn as_raw(&self) -> ani_ref {
        self.inner
    }

    pub fn into_raw(self) -> ani_ref {
        self.inner
    }

    pub fn null() -> Self {
        Self::from_raw(null_mut() as _)
    }
}
