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

#ifndef NET_POLICY_EVENT_LISTENER_CONTEXT_H
#define NET_POLICY_EVENT_LISTENER_CONTEXT_H

#include <memory>
#include <map>
#include "net_mgr_log_wrapper.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyEventListenerContext {
public:
    static NetPolicyEventListenerContext& GetInstance();
    static int32_t AddEventListener(EventListener &eventListener);
    static int32_t RemoveEventListener(EventListener &eventListener);
    static int32_t FindEventListense(EventListener &eventListener);
private:
    static std::map<int32_t, EventListener> listenses;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_EVENT_LISTENER_CONTEXT_H
