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

#ifndef NETMANAGER_BASE_HTTP_ENGINE_H
#define NETMANAGER_BASE_HTTP_ENGINE_H

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "curl/curl.h"
#include "curl/easy.h"
#include "curl/multi.h"

#include "http_easy_handle.h"
#include "net_http_client.h"
#include "net_http_task.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {

/*
 * Internal: async engine driving curl_multi on a lazily started thread.
 *
 * The engine thread is NOT resident: it starts on the first Submit, and exits
 * (and releases the CURLM handle) as soon as no task is pending or in-flight.
 * The exit decision and the running_ flag are always taken under mtx_, and
 * Submit pushes new tasks under the same mtx_, so a Submit racing with the
 * thread exit either wakes the old thread or starts a new one; a task is
 * never lost and two engines never share a CURLM.
 *
 * The CURLM handle and the inflight_ map are owned by the engine thread only
 * and need no locking. Callbacks are invoked on the engine thread without
 * holding mtx_, so callbacks may freely call Cancel/Execute/Submit.
 */
class HttpEngine {
public:
    static HttpEngine &GetInstance();

    std::shared_ptr<NetHttpTask> Submit(const HttpRequest &request, NetHttpResponseCallback callback);

private:
    struct PendingTask {
        uint64_t taskId = 0;
        std::shared_ptr<HttpEasyHandle> handle;
        NetHttpResponseCallback callback;
        std::shared_ptr<NetHttpTask::TaskState> state;
    };

    HttpEngine() = default;
    ~HttpEngine() = default;

    void Run();
    void CompleteTask(const std::shared_ptr<PendingTask> &task, HttpResponse response);
    static HttpResponse MakeCancelledResponse();

    std::mutex mtx_;
    /* Guarded by mtx_. */
    bool running_ = false;
    uint64_t nextTaskId_ = 1;
    std::vector<std::shared_ptr<PendingTask>> pending_;
};
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_HTTP_ENGINE_H
