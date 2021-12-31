/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef EVENT_CONTEXT_H
#define EVENT_CONTEXT_H

#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace NetManagerStandard {
// event listener
struct EventListener {
    std::string event; // JS register event
    napi_env env;
    napi_ref callbackRef = nullptr;
    std::string identifier;
    int32_t netType = 0;
    int32_t netCapabilities = 0;
};

// net conn event
struct NetConnEvent {
    int32_t netState = 0;
    int32_t netType = 0;
};

// event context
struct EventContext {
    EventListener listen;
    NetConnEvent ev;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // EVENT_CONTEXT_H