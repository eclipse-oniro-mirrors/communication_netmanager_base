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

use ani_rs::business_error::BusinessError;

pub const fn convert_to_business_error(code: i32) -> BusinessError {
    match code {
        201 => BusinessError::PERMISSION,
        401 => BusinessError::PARAMETER,
        2100001 => BusinessError::new_static(2100001, "Invalid parameter value."),
        2100002 => BusinessError::new_static(2100002, "Failed to connect to the service."),
        2100003 => BusinessError::new_static(2100003, "System internal error."),
        _ => BusinessError::new_static(code, "Unknown error"),
    }
}
