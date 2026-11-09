/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_BASE_NET_HTTP_CLIENT_H
#define NETMANAGER_BASE_NET_HTTP_CLIENT_H

#include <functional>
#include <memory>
#include <string>

#include "net_http_request.h"
#include "net_http_response.h"
#include "net_http_task.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {

/*
 * Threading contract of the response callback:
 * - Callbacks are invoked on the engine thread, strictly serialized: one at a
 *   time, in completion order. A blocking callback delays EVERY other pending
 *   http callback in the process.
 * - NEVER block inside the callback: no synchronous IPC, no synchronous DB
 *   access, no long computation, no waiting on other threads.
 * - The engine holds no lock while invoking the callback, so calling
 *   Submit/Execute/Cancel again from inside the callback is allowed (chained
 *   requests, retries). Keep such follow-up submissions non-blocking too.
 */
using NetHttpResponseCallback = std::function<void(const HttpResponse &)>;

/*
 * Lightweight curl based http client shared by netmanager subsystems.
 *
 * Execute(): synchronous, runs an independent curl easy handle on the calling
 * thread and blocks until the transfer finishes (or times out).
 *
 * Submit(): asynchronous, the request is driven by a lazily started engine
 * thread (curl_multi). The engine thread is NOT resident: it exits as soon as
 * there is no in-flight task left, and is restarted on the next Submit.
 * The callback is invoked on the engine thread WITHOUT any engine lock held,
 * so it is safe to call Submit/Execute/Cancel again from inside the callback
 * (e.g. chained requests or retries).
 *
 * The client only transports: business judgement of response codes (probe
 * success, portal detection, retry policy) stays with the caller.
 */
class NetHttpClient {
public:
    static NetHttpClient &GetInstance();

    /* Synchronous request. Returns a response even on invalid param or failure. */
    HttpResponse Execute(const HttpRequest &request);

    /*
     * Asynchronous request. Returns nullptr when the request is invalid or the
     * curl handle cannot be created. Otherwise the callback is invoked exactly
     * once, with a response carrying the transport result.
     */
    std::shared_ptr<NetHttpTask> Submit(const HttpRequest &request, NetHttpResponseCallback callback);

private:
    NetHttpClient() = default;
    ~NetHttpClient() = default;
};
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_NET_HTTP_CLIENT_H
