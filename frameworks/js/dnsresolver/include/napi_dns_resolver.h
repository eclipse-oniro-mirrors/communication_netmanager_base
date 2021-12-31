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

#ifndef NAPI_DNS_RESOLVER_H
#define NAPI_DNS_RESOLVER_H

#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t HOST_MAX_BYTES = 256;

// dns resolver async context
struct DnsResolverAsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    // Data context
    char host[HOST_MAX_BYTES] = {0};
    size_t hostRealBytes = 0;
    int32_t result = 0;
    std::vector<std::string> hostAddress;
};

class NapiDnsResolver {
public:
    NapiDnsResolver();
    ~NapiDnsResolver() = default;
    static napi_value DeclareNapiDnsResolverInterface(napi_env env, napi_value exports);
    static napi_value RegisterDnsResolverInterface(napi_env env, napi_value exports);
    static void ExecDnsResolverCallback(napi_env env, void *data);
    static void CompleteDnsResolverCallback(napi_env env, napi_status status, void *data);
    // declare napi interface for JS
    static napi_value GetAddressesByName(napi_env env, napi_callback_info info);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_DNS_RESOLVER_H
