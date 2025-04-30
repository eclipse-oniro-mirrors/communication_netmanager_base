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

#![feature(lazy_cell)]

pub use ani_rs_macros::ani;
pub use ani_rs_macros::native;
pub use ani_rs_macros::native_v;

mod env;
mod vm;

pub use env::AniEnv;
pub use vm::AniVm;

pub mod error;
pub mod objects;

mod ani;
pub use ani::AniDe;
pub use ani::AniSer;
mod iterator;
pub mod signature;

pub mod callback;
pub mod context;
mod global;
mod primitive;
pub mod typed_array;
mod wrapper;

pub mod business_error;
mod macros;
