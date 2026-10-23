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

#ifndef HTTP_PROXY_CALLBACK_PROXY_H
#define HTTP_PROXY_CALLBACK_PROXY_H

#include "i_http_proxy_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
class HttpProxyCallbackProxy : public IRemoteProxy<IHttpProxyCallback> {
public:
    explicit HttpProxyCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IHttpProxyCallback>(impl) {}
    int32_t OnHttpProxyChange() override;

private:
    static inline BrokerDelegator<HttpProxyCallbackProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // HTTP_PROXY_CALLBACK_PROXY_H
