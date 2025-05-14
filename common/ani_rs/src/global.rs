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

use crate::{objects::AniRef, AniVm};

impl<T: Into<AniRef<'static>> + Clone> Drop for GlobalDrop<T> {
    fn drop(&mut self) {
        let env = AniVm::get_instance().get_env().unwrap();
        env.delete_global_ref(self.0.clone().into()).unwrap();
    }
}

pub(crate) struct GlobalDrop<T: Into<AniRef<'static>> + Clone>(pub T);

impl<T: PartialEq + Into<AniRef<'static>> + Clone> PartialEq for GlobalDrop<T> {
    fn eq(&self, other: &Self) -> bool {
        self.0 == other.0
    }
}

impl<T: Eq + Into<AniRef<'static>> + Clone> Eq for GlobalDrop<T> {}
