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

#ifndef NETMANAGER_BASE_NET_HTTP_TASK_H
#define NETMANAGER_BASE_NET_HTTP_TASK_H

#include <atomic>
#include <cstdint>
#include <memory>

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {

class HttpEngine;

/* Handle of an asynchronous request submitted via NetHttpClient::Submit. */
class NetHttpTask {
public:
    struct TaskState {
        std::atomic<bool> cancelled{false};
        std::atomic<bool> done{false};
    };

    uint64_t TaskId() const;
    /* Whether the transfer has finished (normally, failed or cancelled). */
    bool IsDone() const;
    /*
     * Request cancellation. Takes effect asynchronously on the engine thread,
     * the callback will then be invoked with CANCELLED. No-op if already done.
     */
    void Cancel();

private:
    friend class HttpEngine;
    NetHttpTask(uint64_t taskId, std::shared_ptr<TaskState> state);

    uint64_t taskId_ = 0;
    std::shared_ptr<TaskState> state_;
};

} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_NET_HTTP_TASK_H
