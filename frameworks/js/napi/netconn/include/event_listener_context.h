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

#ifndef EVENT_LISTENER_CONTEXT_H
#define EVENT_LISTENER_CONTEXT_H

#include <memory>
#include <mutex>
#include <map>
#include "net_mgr_log_wrapper.h"
#include "event_context.h"
#include "napi_net_conn_observer.h"
#include "napi_net_connection.h"

namespace OHOS {
namespace NetManagerStandard {
class EventListenerContext {
public:
    static EventListenerContext &GetInstance();
    static int32_t AddListense(NapiNetConnection *conn, EventListener &listen);
    static int32_t RemoveListense(NapiNetConnection *conn, EventListener &listen);
    static int32_t Register(NapiNetConnection *conn);
    static int32_t Unregister(NapiNetConnection *conn);
    static int32_t Display();
    static int32_t FindListener(NapiNetConnObserver *observer, EventListener &listen);
private:
    static std::map<int32_t, std::map<int32_t, EventListener>> listenses; // Netconnection obj to a group refences
    static std::map<int32_t, sptr<INetConnCallback>> callbacks; // Netconnection obj to callback obj
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // EVENT_LISTENER_CONTEXT_H
