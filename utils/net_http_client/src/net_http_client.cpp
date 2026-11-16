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

#include "net_http_client.h"

#include "http_easy_handle.h"
#include "http_engine.h"
#include "netmanager_base_log.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {

NetHttpTask::NetHttpTask(uint64_t taskId, std::shared_ptr<TaskState> state)
    : taskId_(taskId), state_(std::move(state))
{
}

uint64_t NetHttpTask::TaskId() const
{
    return taskId_;
}

bool NetHttpTask::IsDone() const
{
    return state_ != nullptr && state_->done.load();
}

void NetHttpTask::Cancel()
{
    if (state_ != nullptr && !state_->done.load()) {
        state_->cancelled.store(true);
    }
}

NetHttpClient &NetHttpClient::GetInstance()
{
    static NetHttpClient instance;
    return instance;
}

HttpResponse NetHttpClient::Execute(const HttpRequest &request)
{
    if (request.url.empty()) {
        return HttpResponse::MakeError(NetHttpErrorCode::INVALID_PARAM, "request url is empty");
    }
    HttpEasyHandle handle;
    if (!handle.Init(request)) {
        return HttpResponse::MakeError(NetHttpErrorCode::INIT_FAILED, "easy handle init failed");
    }
    CURLcode code = curl_easy_perform(handle.GetCurl());
    if (code != CURLE_OK) {
        /* The url may carry business identifiers (e.g. QoD instance ids), never log it in clear. */
        NETMGR_LOG_W("execute failed, url: %{private}s, curl: %{public}d %{public}s", request.url.c_str(),
                          static_cast<int32_t>(code), curl_easy_strerror(code));
    }
    return handle.BuildResponse(code);
}

std::shared_ptr<NetHttpTask> NetHttpClient::Submit(const HttpRequest &request, NetHttpResponseCallback callback)
{
    return HttpEngine::GetInstance().Submit(request, std::move(callback));
}
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS
