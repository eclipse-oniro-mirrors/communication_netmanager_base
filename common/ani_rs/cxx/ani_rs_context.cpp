/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ani_rs_context.h"
#include "ani_base_context.h"
#include <ani.h>

namespace OHOS {

namespace AniRs {

std::shared_ptr<AbilityRuntime::Context> GetStageModeContext(size_t envPtr, size_t objectPtr)
{
    ani_env *env = reinterpret_cast<ani_env *>(envPtr);
    ani_object object = reinterpret_cast<ani_object>(objectPtr);
    return AbilityRuntime::GetStageModeContext(env, object);
}

rust::string GetBundleName(AbilityRuntime::Context const &context)
{
    return context.GetBundleName();
}

} // namespace AniRs
} // namespace OHOS