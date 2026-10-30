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

#ifndef HTTP_PROXY_CALLBACK_STUB_H
#define HTTP_PROXY_CALLBACK_STUB_H

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include "i_http_proxy_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class HttpProxyCallbackStub : public IRemoteStub<IHttpProxyCallback> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;
    int32_t OnHttpProxyChange() override;
    void SetCallback(std::function<void()> callback);
    void ClearCallbackAndWait();

private:
    class ActiveCallbackGuard final {
    public:
        explicit ActiveCallbackGuard(HttpProxyCallbackStub &owner) : owner_(owner) {}
        ~ActiveCallbackGuard();
        ActiveCallbackGuard(const ActiveCallbackGuard &) = delete;
        ActiveCallbackGuard &operator=(const ActiveCallbackGuard &) = delete;

    private:
        HttpProxyCallbackStub &owner_;
    };
    void FinishActiveCallback();
    std::mutex mutex_;
    std::condition_variable callbackCv_;
    uint32_t activeCallbacks_ = 0;
    std::function<void()> callback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // HTTP_PROXY_CALLBACK_STUB_H
