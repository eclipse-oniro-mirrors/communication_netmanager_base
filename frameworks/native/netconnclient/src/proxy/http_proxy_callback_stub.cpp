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

#include "http_proxy_callback_stub.h"
#include "conn_ipc_interface_code.h"
#include "net_conn_constants.h"
#include <utility>

namespace OHOS {
namespace NetManagerStandard {
HttpProxyCallbackStub::ActiveCallbackGuard::~ActiveCallbackGuard()
{
    owner_.FinishActiveCallback();
}

int32_t HttpProxyCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
    }
    if (code == static_cast<uint32_t>(HttpProxyCallbackInterfaceCode::ON_HTTP_PROXY_CHANGE)) {
        return OnHttpProxyChange();
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t HttpProxyCallbackStub::OnHttpProxyChange()
{
    std::function<void()> callback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callback = callback_;
        if (callback) {
            ++activeCallbacks_;
        }
    }
    if (callback) {
        ActiveCallbackGuard guard(*this);
        callback();
    }
    return NETMANAGER_SUCCESS;
}

void HttpProxyCallbackStub::FinishActiveCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    --activeCallbacks_;
    if (activeCallbacks_ == 0) {
        callbackCv_.notify_all();
    }
}

void HttpProxyCallbackStub::SetCallback(std::function<void()> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

void HttpProxyCallbackStub::ClearCallbackAndWait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    callback_ = nullptr;
    callbackCv_.wait(lock, [this]() { return activeCallbacks_ == 0; });
}
} // namespace NetManagerStandard
} // namespace OHOS
