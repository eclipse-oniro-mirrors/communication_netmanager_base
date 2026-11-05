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

#include "http_engine.h"

#include "netmanager_base_log.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {
namespace {
constexpr int ENGINE_POLL_INTERVAL_MS = 20;
} // namespace

HttpEngine &HttpEngine::GetInstance()
{
    static HttpEngine instance;
    return instance;
}

std::shared_ptr<NetHttpTask> HttpEngine::Submit(const HttpRequest &request, NetHttpResponseCallback callback)
{
    if (request.url.empty()) {
        NETMGR_LOG_E("submit failed, request url is empty");
        return nullptr;
    }
    auto handle = std::make_shared<HttpEasyHandle>();
    if (!handle->Init(request)) {
        NETMGR_LOG_E("submit failed, easy handle init failed, url: %{private}s", request.url.c_str());
        return nullptr;
    }

    auto task = std::make_shared<PendingTask>();
    auto state = std::make_shared<NetHttpTask::TaskState>();
    {
        std::lock_guard<std::mutex> lock(mtx_);
        task->taskId = nextTaskId_++;
        task->handle = handle;
        task->callback = std::move(callback);
        task->state = state;
        pending_.push_back(task);
        if (!running_) {
            running_ = true;
            /* Detached on purpose: the engine thread manages its own lifetime
             * and exits when idle, nobody joins it. */
            std::thread(&HttpEngine::Run, this).detach();
        }
    }
    NETMGR_LOG_I("submit task %{public}llu, url: %{private}s", static_cast<unsigned long long>(task->taskId),
                      request.url.c_str());
    return std::shared_ptr<NetHttpTask>(new NetHttpTask(task->taskId, state));
}

void HttpEngine::Run()
{
    CURLM *multi = curl_multi_init();
    if (multi == nullptr) {
        NETMGR_LOG_E("curl_multi_init failed");
        std::lock_guard<std::mutex> lock(mtx_);
        running_ = false;
        return;
    }
    /* Owned by this engine thread only. */
    std::map<CURL *, std::shared_ptr<PendingTask>> inflight;
    std::vector<std::shared_ptr<PendingTask>> toAdd;
    std::vector<std::pair<std::shared_ptr<PendingTask>, HttpResponse>> toComplete;

    while (true) {
        toAdd.clear();
        toComplete.clear();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (pending_.empty()) {
                /* Nothing to do at all: announce the exit under the lock so a
                 * concurrent Submit either sees running_ == false (and starts
                 * a new engine) or pushes into pending_ which we re-check below. */
                if (inflight.empty()) {
                    running_ = false;
                    break;
                }
            } else {
                toAdd = std::move(pending_);
                pending_.clear();
            }
        }

        for (auto &task : toAdd) {
            if (task->state->cancelled) {
                toComplete.emplace_back(task, MakeCancelledResponse());
                continue;
            }
            CURL *curl = task->handle->GetCurl();
            CURLMcode code = curl_multi_add_handle(multi, curl);
            if (code != CURLM_OK || curl == nullptr) {
                NETMGR_LOG_E("curl_multi_add_handle failed, task %{public}llu, code: %{public}d",
                                   static_cast<unsigned long long>(task->taskId), code);
                toComplete.emplace_back(
                    task, HttpResponse::MakeError(NetHttpErrorCode::INIT_FAILED, "curl_multi_add_handle failed"));
                continue;
            }
            inflight[curl] = task;
        }

        int runningHandles = 0;
        CURLMcode performCode = curl_multi_perform(multi, &runningHandles);
        if (performCode != CURLM_OK) {
            NETMGR_LOG_E("curl_multi_perform failed, code: %{public}d", performCode);
        }

        int msgsLeft = 0;
        CURLMsg *msg = nullptr;
        while ((msg = curl_multi_info_read(multi, &msgsLeft)) != nullptr) {
            if (msg->msg != CURLMSG_DONE || msg->easy_handle == nullptr) {
                continue;
            }
            auto iter = inflight.find(msg->easy_handle);
            if (iter == inflight.end()) {
                continue;
            }
            auto task = iter->second;
            inflight.erase(iter);
            curl_multi_remove_handle(multi, msg->easy_handle);
            toComplete.emplace_back(task, task->handle->BuildResponse(msg->data.result));
        }

        /* Handle cancellations of in-flight tasks. */
        for (auto iter = inflight.begin(); iter != inflight.end();) {
            if (iter->second->state->cancelled) {
                auto task = iter->second;
                iter = inflight.erase(iter);
                curl_multi_remove_handle(multi, task->handle->GetCurl());
                toComplete.emplace_back(task, MakeCancelledResponse());
                continue;
            }
            iter++;
        }

        /* Invoke callbacks without holding mtx_: a callback may call
         * Submit/Cancel/Execute again. */
        for (auto &item : toComplete) {
            CompleteTask(item.first, std::move(item.second));
        }

        if (!inflight.empty()) {
            curl_multi_poll(multi, nullptr, 0, ENGINE_POLL_INTERVAL_MS, nullptr);
        }
    }

    /* All handles were removed above, safe to release the multi handle. */
    curl_multi_cleanup(multi);
    NETMGR_LOG_I("http engine thread exited");
}

void HttpEngine::CompleteTask(const std::shared_ptr<PendingTask> &task, HttpResponse response)
{
    if (task == nullptr) {
        return;
    }
    task->state->done = true;
    if (task->callback) {
        task->callback(response);
    }
}

HttpResponse HttpEngine::MakeCancelledResponse()
{
    HttpResponse response;
    response.errCode = NetHttpErrorCode::CANCELLED;
    response.rawCurlCode = CURLE_ABORTED_BY_CALLBACK;
    response.errMessage = "task cancelled";
    return response;
}
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS
