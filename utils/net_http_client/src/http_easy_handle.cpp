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

#include "http_easy_handle.h"

#include <atomic>
#include <cstring>
#include <mutex>

#include "netmanager_base_log.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {
namespace {
/* Guard the collected response body, requests in this library never need more. */
constexpr size_t MAX_RESPONSE_BODY_SIZE = 10 * 1024 * 1024;
/* CURL_MAX_HTTP_HEADER (100K) guards the collected response headers. */

const char *MethodToString(HttpMethod method)
{
    switch (method) {
        case HttpMethod::POST:
            return "POST";
        case HttpMethod::PUT:
            return "PUT";
        case HttpMethod::DELETE:
            return "DELETE";
        case HttpMethod::HEAD:
            return "HEAD";
        case HttpMethod::GET:
        default:
            return "GET";
    }
}

std::string Trim(const std::string &str)
{
    size_t begin = 0;
    while (begin < str.size() && (str[begin] == ' ' || str[begin] == '\t' || str[begin] == '\r')) {
        begin++;
    }
    size_t end = str.size();
    while (end > begin && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r')) {
        end--;
    }
    return str.substr(begin, end - begin);
}

std::string ToLower(const std::string &str)
{
    std::string result = str;
    for (auto &c : result) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return result;
}

/*
 * curl_global_init is not thread safe and must be called before any other
 * curl api. The library keeps it alive for the whole process lifetime on
 * purpose: releasing it while the engine thread still owns handles would be
 * unsafe, and all consumers here are long running system services.
 */
bool EnsureCurlGlobalInit()
{
    static std::once_flag onceFlag;
    static bool inited = false;
    std::call_once(onceFlag, []() {
        inited = (curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK);
        if (!inited) {
            NETMGR_LOG_E("curl_global_init failed");
        }
    });
    return inited;
}

template<typename T>
bool SetCurlEasyOption(CURL *handle, CURLoption option, T data)
{
    CURLcode setResult = curl_easy_setopt(handle, option, data);
    if (setResult != CURLE_OK) {
        NETMGR_LOG_E("set curl option failed, option:%{public}d err:%{public}s",
                     static_cast<int>(option), curl_easy_strerror(setResult));
        return false;
    }
    return true;
}
} // namespace

HttpEasyHandle::~HttpEasyHandle()
{
    if (curl_ != nullptr) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
    if (headerList_ != nullptr) {
        curl_slist_free_all(headerList_);
        headerList_ = nullptr;
    }
    if (resolveList_ != nullptr) {
        curl_slist_free_all(resolveList_);
        resolveList_ = nullptr;
    }
}

bool HttpEasyHandle::Init(const HttpRequest &request)
{
    if (request.url.empty()) {
        NETMGR_LOG_E("request url is empty");
        return false;
    }
    if (!EnsureCurlGlobalInit()) {
        return false;
    }
    curl_ = curl_easy_init();
    if (curl_ == nullptr) {
        NETMGR_LOG_E("curl_easy_init failed");
        return false;
    }
    startTime_ = std::chrono::steady_clock::now();
    return SetOptions(request);
}

bool HttpEasyHandle::SetOptions(const HttpRequest &request)
{
    if (!SetMethodOptions(request) || !SetHeaderOptions(request) || !SetTimeoutOptions(request) ||
        !SetNetworkOptions(request) || !SetTlsOptions(request.tls)) {
        return false;
    }
    if (request.proxy.has_value() && !SetProxyOptions(request.proxy.value())) {
        return false;
    }
    if (!SetResolveOptions(request.resolves)) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_NOSIGNAL, 1L)) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_ERRORBUFFER, errBuffer_)) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_WRITEFUNCTION, HttpEasyHandle::WriteCallback)) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_WRITEDATA, this)) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_HEADERFUNCTION, HttpEasyHandle::HeaderCallback)) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_HEADERDATA, this)) {
        return false;
    }
    return true;
}

bool HttpEasyHandle::SetMethodOptions(const HttpRequest &request)
{
    if (!SetCurlEasyOption(curl_, CURLOPT_URL, request.url.c_str())) {
        return false;
    }
    switch (request.method) {
        case HttpMethod::GET:
            break;
        case HttpMethod::POST:
            if (!SetPostBody(request)) {
                return false;
            }
            break;
        case HttpMethod::PUT:
        case HttpMethod::DELETE:
            if (!request.body.empty() && !SetPostBody(request)) {
                return false;
            }
            if (!SetCurlEasyOption(curl_, CURLOPT_CUSTOMREQUEST, MethodToString(request.method))) {
                return false;
            }
            break;
        case HttpMethod::HEAD:
            if (!SetCurlEasyOption(curl_, CURLOPT_NOBODY, 1L)) {
                return false;
            }
            break;
        default:
            break;
    }
    return true;
}

bool HttpEasyHandle::SetPostBody(const HttpRequest &request)
{
    reqBody_ = request.body;
    if (!SetCurlEasyOption(curl_, CURLOPT_POSTFIELDS, reqBody_.c_str())) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_POSTFIELDSIZE, static_cast<long>(reqBody_.size()))) {
        return false;
    }
    return true;
}

bool HttpEasyHandle::SetHeaderOptions(const HttpRequest &request)
{
    for (const auto &header : request.headers) {
        std::string key = Trim(header.first);
        if (key.empty()) {
            continue;
        }
        std::string entry = key + ": " + Trim(header.second);
        struct curl_slist *newList = curl_slist_append(headerList_, entry.c_str());
        if (newList == nullptr) {
            NETMGR_LOG_E("curl_slist_append failed, header: %{public}s", entry.c_str());
            return false;
        }
        headerList_ = newList;
    }
    if (headerList_ != nullptr) {
        if (!SetCurlEasyOption(curl_, CURLOPT_HTTPHEADER, headerList_)) {
            return false;
        }
    }
    return true;
}

bool HttpEasyHandle::SetTimeoutOptions(const HttpRequest &request)
{
    if (request.timeoutMs > 0) {
        if (!SetCurlEasyOption(curl_, CURLOPT_TIMEOUT_MS, static_cast<long>(request.timeoutMs))) {
            return false;
        }
    }
    if (request.connectTimeoutMs > 0) {
        if (!SetCurlEasyOption(curl_, CURLOPT_CONNECTTIMEOUT_MS,
            static_cast<long>(request.connectTimeoutMs))) {
            return false;
        }
    }
    return true;
}

bool HttpEasyHandle::SetNetworkOptions(const HttpRequest &request)
{
    if (request.port != 0) {
        if (!SetCurlEasyOption(curl_, CURLOPT_PORT, static_cast<long>(request.port))) {
            return false;
        }
    }
    switch (request.ipVersion) {
        case IpVersion::IPV4_ONLY:
            if (!SetCurlEasyOption(curl_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4)) {
                return false;
            }
            break;
        case IpVersion::IPV6_ONLY:
            if (!SetCurlEasyOption(curl_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6)) {
                return false;
            }
            break;
        case IpVersion::AUTO:
        default:
            break;
    }
    if (!request.bindInterface.empty()) {
        if (!SetCurlEasyOption(curl_, CURLOPT_INTERFACE, request.bindInterface.c_str())) {
            return false;
        }
    }
    if (request.forbidReuse) {
        if (!SetCurlEasyOption(curl_, CURLOPT_FORBID_REUSE, 1L)) {
            return false;
        }
    }
    if (request.followRedirects) {
        if (!SetCurlEasyOption(curl_, CURLOPT_FOLLOWLOCATION, 1L)) {
            return false;
        }
    }
    if (request.httpVersion == HttpVersion::HTTP2_TLS) {
        if (!SetCurlEasyOption(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS)) {
            return false;
        }
    }
    return true;
}

bool HttpEasyHandle::SetTlsVerifyOptions(bool verifyPeer)
{
    /* VERIFYHOST 2L also checks the host name inside the certificate;
     * when verification is off both checks are disabled. */
    const long verifyPeerValue = verifyPeer ? 1L : 0L;
    const long verifyHostValue = verifyPeer ? 2L : 0L;
    return SetCurlEasyOption(curl_, CURLOPT_SSL_VERIFYPEER, verifyPeerValue) &&
        SetCurlEasyOption(curl_, CURLOPT_SSL_VERIFYHOST, verifyHostValue);
}

bool HttpEasyHandle::SetTlsMinVersionOption(TlsVersion minVersion)
{
    long version = CURL_SSLVERSION_DEFAULT;
    switch (minVersion) {
        case TlsVersion::TLS_V1_2:
            version = CURL_SSLVERSION_TLSv1_2;
            break;
        case TlsVersion::TLS_V1_3:
            version = CURL_SSLVERSION_TLSv1_3;
            break;
        case TlsVersion::DEFAULT:
        default:
            /* CURL_SSLVERSION_DEFAULT is curl's own default, so setting it
             * explicitly is equivalent to not setting the option at all. */
            break;
    }
    return SetCurlEasyOption(curl_, CURLOPT_SSLVERSION, version);
}

bool HttpEasyHandle::SetOptionalStringOption(CURLoption option, const std::string &value)
{
    if (value.empty()) {
        return true;
    }
    return SetCurlEasyOption(curl_, option, value.c_str());
}

bool HttpEasyHandle::SetTlsOptions(const TlsOptions &tls)
{
    return SetTlsVerifyOptions(tls.verifyPeer) &&
        SetTlsMinVersionOption(tls.minVersion) &&
        SetOptionalStringOption(CURLOPT_CAINFO, tls.caInfoPath) &&
        SetOptionalStringOption(CURLOPT_CAPATH, tls.caDirPath) &&
        SetOptionalStringOption(CURLOPT_SSL_CIPHER_LIST, tls.cipherList) &&
        SetOptionalStringOption(CURLOPT_TLS13_CIPHERS, tls.tls13CipherList);
}

bool HttpEasyHandle::SetProxyOptions(const HttpProxyConfig &proxy)
{
    if (proxy.host.empty()) {
        return true;
    }
    curl_proxytype proxyType =
        (proxy.host.find("https://") != std::string::npos) ? CURLPROXY_HTTPS : CURLPROXY_HTTP;
    if (!SetCurlEasyOption(curl_, CURLOPT_PROXY, proxy.host.c_str())) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_PROXYPORT, static_cast<long>(proxy.port))) {
        return false;
    }
    if (!SetCurlEasyOption(curl_, CURLOPT_PROXYTYPE, proxyType)) {
        return false;
    }
    if (proxy.useTunnel) {
        if (!SetCurlEasyOption(curl_, CURLOPT_HTTPPROXYTUNNEL, 1L)) {
            return false;
        }
    }
    if (!proxy.username.empty()) {
        if (!SetCurlEasyOption(curl_, CURLOPT_PROXYUSERNAME, proxy.username.c_str())) {
            return false;
        }
        if (!proxy.password.empty()) {
            if (!SetCurlEasyOption(curl_, CURLOPT_PROXYPASSWORD, proxy.password.c_str())) {
                return false;
            }
        }
        /* No CURLOPT_HTTPAUTH: proxy credentials above only build the
         * Proxy-Authorization header and no origin credentials are configured. */
        if (!SetCurlEasyOption(curl_, CURLOPT_PROXYAUTH, CURLAUTH_ANY)) {
            return false;
        }
    }
    return true;
}

bool HttpEasyHandle::SetResolveOptions(const std::vector<std::string> &resolves)
{
    for (const auto &resolve : resolves) {
        if (resolve.empty()) {
            continue;
        }
        struct curl_slist *newList = curl_slist_append(resolveList_, resolve.c_str());
        if (newList == nullptr) {
            NETMGR_LOG_E("curl_slist_append resolve failed: %{public}s", resolve.c_str());
            return false;
        }
        resolveList_ = newList;
    }
    if (resolveList_ != nullptr) {
        if (!SetCurlEasyOption(curl_, CURLOPT_RESOLVE, resolveList_)) {
            return false;
        }
    }
    return true;
}

size_t HttpEasyHandle::WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    if (ptr == nullptr || userdata == nullptr) {
        return 0;
    }
    size_t totalSize = size * nmemb;
    auto *handle = static_cast<HttpEasyHandle *>(userdata);
    if (handle->respBody_.size() + totalSize > MAX_RESPONSE_BODY_SIZE) {
        NETMGR_LOG_E("response body exceeds limit, abort transfer");
        return 0;
    }
    handle->respBody_.append(ptr, totalSize);
    return totalSize;
}

size_t HttpEasyHandle::HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    if (buffer == nullptr || userdata == nullptr) {
        return 0;
    }
    size_t totalSize = size * nitems;
    if (totalSize > CURL_MAX_HTTP_HEADER) {
        NETMGR_LOG_E("single response header exceeds 100K, abort transfer");
        return 0;
    }
    auto *handle = static_cast<HttpEasyHandle *>(userdata);
    handle->headerBuf_.append(buffer, totalSize);
    if (handle->headerBuf_.size() > CURL_MAX_HTTP_HEADER) {
        NETMGR_LOG_E("response headers exceed 100K, abort transfer");
        return 0;
    }
    /* Header lines may be split across callbacks, parse only complete lines. */
    size_t lineStart = 0;
    size_t lineEnd = handle->headerBuf_.find('\n');
    while (lineEnd != std::string::npos) {
        handle->ParseHeaderLine(handle->headerBuf_.substr(lineStart, lineEnd - lineStart));
        lineStart = lineEnd + 1;
        lineEnd = handle->headerBuf_.find('\n', lineStart);
    }
    handle->headerBuf_.erase(0, lineStart);
    return totalSize;
}

void HttpEasyHandle::ParseHeaderLine(const std::string &line)
{
    std::string trimmed = Trim(line);
    if (trimmed.empty()) {
        return;
    }
    /* Skip the status line and other non "key: value" lines. */
    if (trimmed.compare(0, 5, "HTTP/") == 0) {
        return;
    }
    size_t pos = trimmed.find(':');
    if (pos == std::string::npos) {
        return;
    }
    std::string key = ToLower(Trim(trimmed.substr(0, pos)));
    std::string value = Trim(trimmed.substr(pos + 1));
    if (key.empty()) {
        return;
    }
    auto iter = respHeaders_.find(key);
    if (iter != respHeaders_.end()) {
        if (!iter->second.empty() && !value.empty()) {
            iter->second += ", ";
        }
        iter->second += value;
        return;
    }
    respHeaders_[key] = value;
}

HttpResponse HttpEasyHandle::BuildResponse(CURLcode curlCode)
{
    HttpResponse response;
    response.rawCurlCode = static_cast<int32_t>(curlCode);
    if (curl_ != nullptr) {
        long responseCode = 0;
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &responseCode);
        response.responseCode = static_cast<int32_t>(responseCode);
        char *url = nullptr;
        curl_easy_getinfo(curl_, CURLINFO_REDIRECT_URL, &url);
        if (url != nullptr) {
            response.redirectUrl = url;
        }
        curl_easy_getinfo(curl_, CURLINFO_EFFECTIVE_URL, &url);
        if (url != nullptr) {
            response.effectiveUrl = url;
        }
    }
    auto costMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startTime_);
    response.totalCostMs = costMs.count();
    response.body = std::move(respBody_);
    response.headers = std::move(respHeaders_);
    response.errMessage = errBuffer_;
    if (curlCode == CURLE_OK) {
        /* Transport succeeded. 4xx/5xx are still valid responses, business
         * judgement (probe success / portal) is up to the caller. */
        response.errCode = NetHttpErrorCode::SUCCESS;
    } else {
        response.responseCode = -1;
        response.errCode = MapCurlCode(curlCode);
    }
    return response;
}

NetHttpErrorCode HttpEasyHandle::MapCurlCode(CURLcode code) const
{
    switch (code) {
        case CURLE_OK:
            return NetHttpErrorCode::SUCCESS;
        case CURLE_UNSUPPORTED_PROTOCOL:
        case CURLE_URL_MALFORMAT:
            return NetHttpErrorCode::INVALID_PARAM;
        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
            return NetHttpErrorCode::RESOLVE_FAILED;
        case CURLE_COULDNT_CONNECT:
        case CURLE_INTERFACE_FAILED:
            return NetHttpErrorCode::CONNECT_FAILED;
        case CURLE_OPERATION_TIMEDOUT:
            return NetHttpErrorCode::OPERATION_TIMEDOUT;
        case CURLE_SSL_CONNECT_ERROR:
        case CURLE_PEER_FAILED_VERIFICATION:
        case CURLE_SSL_CACERT_BADFILE:
        case CURLE_SSL_SHUTDOWN_FAILED:
        case CURLE_SSL_CRL_BADFILE:
        case CURLE_SSL_ISSUER_ERROR:
            return NetHttpErrorCode::SSL_ERROR;
        case CURLE_ABORTED_BY_CALLBACK:
            return NetHttpErrorCode::CANCELLED;
        case CURLE_WRITE_ERROR:
            /* WriteCallback aborts (returns 0) when the response body exceeds
             * MAX_RESPONSE_BODY_SIZE, so a write error means "too large". */
            return NetHttpErrorCode::RESPONSE_TOO_LARGE;
        case CURLE_FAILED_INIT:
            return NetHttpErrorCode::INIT_FAILED;
        default:
            return NetHttpErrorCode::UNKNOWN_ERROR;
    }
}
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS
