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

#ifndef NETMANAGER_BASE_NET_HTTP_RESPONSE_H
#define NETMANAGER_BASE_NET_HTTP_RESPONSE_H

#include <cstdint>
#include <map>
#include <string>

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {

/*
 * Transport level error codes of the http client. Self-contained on purpose:
 * no dependency on NetManagerConstants or any other errorcode module.
 * Business judgement of the response (e.g. 204 means probe success, 3xx means
 * portal) is left to the caller.
 */
enum class NetHttpErrorCode : int32_t {
    SUCCESS = 0,
    INVALID_PARAM = 1,
    INIT_FAILED = 2,
    CONNECT_FAILED = 3,
    RESOLVE_FAILED = 4,
    OPERATION_TIMEDOUT = 5,
    SSL_ERROR = 6,
    CANCELLED = 7,
    UNKNOWN_ERROR = 8,
    /* Response body exceeded the size limit, the transfer was aborted on purpose. */
    RESPONSE_TOO_LARGE = 9,
};

struct HttpResponse {
    /* HTTP status code. -1 when the transfer failed before a response was received. */
    int32_t responseCode = -1;
    /* Response body. Empty when nothing was received. */
    std::string body;
    /* Response headers, keys are lower-cased, duplicate keys joined with ", ". */
    std::map<std::string, std::string> headers;
    /* Value of CURLINFO_REDIRECT_URL, empty when there is no redirect. */
    std::string redirectUrl;
    /* Value of CURLINFO_EFFECTIVE_URL, the url actually used. */
    std::string effectiveUrl;
    /* Content of the curl error buffer, for diagnostics and portal judgement. */
    std::string errMessage;
    /* Whole transfer cost in ms, -1 when unknown. */
    int64_t totalCostMs = -1;
    NetHttpErrorCode errCode = NetHttpErrorCode::SUCCESS;
    /* Raw libcurl code (CURLE_*), 0 on success, for diagnostics. */
    int32_t rawCurlCode = 0;

    /* Transport succeeded (a response was received). Not a business success flag. */
    bool IsTransportSuccess() const
    {
        return errCode == NetHttpErrorCode::SUCCESS;
    }

    /* Uniform factory for error responses used by both the sync and async paths. */
    static HttpResponse MakeError(NetHttpErrorCode errCode, const std::string &msg)
    {
        HttpResponse response;
        response.errCode = errCode;
        response.errMessage = msg;
        return response;
    }
};

} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_NET_HTTP_RESPONSE_H
