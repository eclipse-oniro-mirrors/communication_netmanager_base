/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef BPF_MANAGER_H
#define BPF_MANAGER_H

#include "singleton.h"

namespace OHOS {
namespace nmd {
class BpfManager {
public:
    BpfManager() = default;

    /**
     * Initialize Bpf Manager.
     *
     * @return Returns true on success, false on failure.
     */
    bool Init() const;

private:
    /**
     * Modify the permission of maps.
     *
     * @return Returns true on success, false on failure.
     */
    bool ModifyMapPermission() const;
};
} // namespace nmd
} // namespace OHOS
#endif // BPF_MANAGER_H
