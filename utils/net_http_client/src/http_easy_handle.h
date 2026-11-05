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

#ifndef NETMANAGER_BASE_HTTP_EASY_HANDLE_H
#define NETMANAGER_BASE_HTTP_EASY_HANDLE_H

#include <chrono>
#include <map>
#include <string>

#include "curl/curl.h"
#include "curl/easy.h"

#include "net_http_request.h"
#include "net_http_response.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {

/*
 * Internal: one curl easy handle bound to one HttpRequest. Owns the slists,
 * the error buffer and the collected response data. Usable both for a plain
 * curl_easy_perform (sync path) and inside a curl_multi loop (engine path).
 */
class HttpEasyHandle {
public:
    HttpEasyHandle() = default;
    ~HttpEasyHandle();

    HttpEasyHandle(const HttpEasyHandle &) = delete;
    HttpEasyHandle &operator=(const HttpEasyHandle &) = delete;

    /* Create the easy handle and apply all request options. */
    bool Init(const HttpRequest &request);

    CURL *GetCurl() const
    {
        return curl_;
    }

    /* Collect response info. Call once the transfer finished with curlCode. */
    HttpResponse BuildResponse(CURLcode curlCode);

    /* Map a CURLcode to the public NetHttpErrorCode. Pure mapping, no I/O:
     * public so unit tests can verify error translation without network. */
    NetHttpErrorCode MapCurlCode(CURLcode code) const;

private:
    bool SetOptions(const HttpRequest &request);
    bool SetMethodOptions(const HttpRequest &request);
    /* Copy the request body into reqBody_ and point curl at it (POST body, or
     * body of a custom-request method such as PUT/DELETE). */
    bool SetPostBody(const HttpRequest &request);
    bool SetHeaderOptions(const HttpRequest &request);
    bool SetTimeoutOptions(const HttpRequest &request);
    bool SetNetworkOptions(const HttpRequest &request);
    bool SetTlsOptions(const TlsOptions &tls);
    /* TLS subgroup helpers, kept small and independently testable. */
    bool SetTlsVerifyOptions(bool verifyPeer);
    bool SetTlsMinVersionOption(TlsVersion minVersion);
    /* Set a curl string option only when the value is non-empty. */
    bool SetOptionalStringOption(CURLoption option, const std::string &value);
    bool SetProxyOptions(const HttpProxyConfig &proxy);
    bool SetResolveOptions(const std::vector<std::string> &resolves);

    static size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata);
    void ParseHeaderLine(const std::string &line);

    CURL *curl_ = nullptr;
    struct curl_slist *headerList_ = nullptr;
    struct curl_slist *resolveList_ = nullptr;
    /* Body copy owned by the handle: curl reads it during the transfer. */
    std::string reqBody_;
    std::string headerBuf_;
    std::map<std::string, std::string> respHeaders_;
    std::string respBody_;
    char errBuffer_[CURL_ERROR_SIZE] = {0};
    std::chrono::steady_clock::time_point startTime_;
};
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_HTTP_EASY_HANDLE_H
