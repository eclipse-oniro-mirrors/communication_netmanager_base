/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "http_request.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <cinttypes>

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
HttpRequest::HttpRequest()
{
    CURLcode errCode = curl_global_init(CURL_GLOBAL_ALL);
    if (errCode != CURLE_OK) {
        NETMGR_LOG_E("curl_global_init failed, errCode:[%{public}x]!", errCode);
    }
}

HttpRequest::~HttpRequest()
{
    curl_global_cleanup();
}

void HttpRequest::SetIfaceName(const std::string &ifaceName)
{
    const std::string ifPrefix("if!");
    const auto preLength = ifPrefix.length();
    if ((ifaceName.length() > preLength && ifaceName.substr(0, preLength) == ifPrefix) || ifaceName.empty()) {
        ifaceName_ = ifaceName;
    } else {
        ifaceName_ = ifPrefix + ifaceName;
    }
}

int32_t HttpRequest::HttpGetHeader(const std::string &strUrl, std::string &strHeader)
{
    NETMGR_LOG_I("Enter HttpGetHeader");
    return HttpRequestHeaderExec(strUrl, strHeader);
}

int32_t HttpRequest::HttpGet(const std::string &strUrl, std::string &strResponse)
{
    NETMGR_LOG_I("Enter HttpGet");
    return HttpRequestExec(HttpReqType::HTTP_REQUEST_TYPE_GET, strUrl, "", strResponse);
}

int32_t HttpRequest::HttpPost(const std::string &strUrl, const std::string &strData, std::string &strResponse)
{
    NETMGR_LOG_I("Enter HttpPost");
    return HttpRequestExec(HttpReqType::HTTP_REQUEST_TYPE_POST, strUrl, strData, strResponse);
}

void HttpRequest::SetTransportTimeout(int64_t timeout)
{
    NETMGR_LOG_I("Enter SetTransportTimeout");
    if (timeout > 0) {
        const double ratio = 0.75;
        connectionTimeout_ = static_cast<int64_t>(timeout * ratio);
        transOpTimeout_ = timeout;
        NETMGR_LOG_D("TimeoutMs,connection:[%{public}" PRId64 "],transport:[%{public}" PRId64 "]!", connectionTimeout_,
            transOpTimeout_);
    }
}

int64_t HttpRequest::GetLastTotalTime() const
{
    NETMGR_LOG_I("Enter GetLastTotalTime");
    return lastTransTime_;
}

void HttpRequest::CURLClean::operator()(CURL *p) const
{
    if (p) {
        curl_easy_cleanup(p);
    }
}

int32_t HttpRequest::HttpRequestExec(
    HttpReqType type, const std::string &strUrl, const std::string &strData, std::string &strResponse)
{
    /* Check whether the URL is valid. */
    if (strUrl.empty() || strUrl.length() > URL_SIZE) {
        NETMGR_LOG_E("URL error!");
        return -1;
    }

    std::unique_ptr<CURL, CURLClean> curl(curl_easy_init(), CURLClean());
    if (curl.get() == nullptr) {
        return -1;
    }

    int32_t rlt = SetCurlOpt(curl, type, strUrl, strData, strResponse);
    if (rlt == 0) {
        CURLcode errCode = CURLE_OK;
        errCode = curl_easy_perform(curl.get());
        rlt = static_cast<int32_t>(errCode);
    }

    return ParseExecResult(curl, rlt);
}

int32_t HttpRequest::HttpRequestHeaderExec(const std::string &strUrl, std::string &strHeader)
{
    NETMGR_LOG_D("Enter HttpRequestHeaderExec, ifaceName:[%{public}s]", ifaceName_.c_str());
    /* Check whether the URL is valid. */
    if (strUrl.empty() || strUrl.length() > URL_SIZE) {
        NETMGR_LOG_E("URL error!");
        return -1;
    }

    std::unique_ptr<CURL, CURLClean> curl(curl_easy_init(), CURLClean());
    if (curl.get() == nullptr) {
        return -1;
    }

    int32_t rlt = SetCurlOptHeader(curl, strUrl, strHeader);
    if (rlt == 0) {
        CURLcode errCode = curl_easy_perform(curl.get());
        errCode = (errCode == CURLE_OK || errCode == CURLE_WRITE_ERROR) ? CURLE_OK : errCode;
        rlt = static_cast<int32_t>(errCode);
    }

    return ParseExecResult(curl, rlt);
}

int32_t HttpRequest::ParseExecResult(const std::unique_ptr<CURL, CURLClean> &curl, int32_t rlt)
{
    CURLcode errCode = static_cast<CURLcode>(rlt);
    if (errCode != CURLE_OK) {
        NETMGR_LOG_E("HTTP request failed, errStr:[%{public}s], errorBuffer_:[%{public}s]!",
            curl_easy_strerror(errCode), errorBuffer_);
        return rlt;
    }

    lastTransTime_ = 0;
    curl_off_t totalTimeUs = 0L;
    errCode = curl_easy_getinfo(curl.get(), CURLINFO_TOTAL_TIME_T, &totalTimeUs);
    if (errCode == CURLE_OK) {
        lastTransTime_ = static_cast<int64_t>(totalTimeUs / 1000L);
        NETMGR_LOG_D("HTTP request OK,total time in ms:[%{public}" PRId64 "]", lastTransTime_);
    }
    return static_cast<int32_t>(errCode);
}

int32_t HttpRequest::SetCurlOpt(const std::unique_ptr<CURL, CURLClean> &curl, HttpReqType type,
    const std::string &strUrl, const std::string &strData, std::string &strResponse)
{
    int32_t rlt = SetCurlOptCommon(curl, strUrl);
    if (rlt != 0) {
        return rlt;
    }

    /* receive the http header */
    curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, nullptr);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, nullptr);

    /* receive the whole http response */
    strResponse.clear();
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, DataCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &strResponse);

    if (type == HttpReqType::HTTP_REQUEST_TYPE_POST) {
        /* Specify post content */
        curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, strData.length());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, strData.c_str());
    }

    return 0;
}

int32_t HttpRequest::SetCurlOptHeader(
    const std::unique_ptr<CURL, CURLClean> &curl, const std::string &strUrl, std::string &strHeader)
{
    int32_t rlt = SetCurlOptCommon(curl, strUrl);
    if (rlt != 0) {
        return rlt;
    }

    /* receive the http header */
    strHeader.clear();
    curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, DataCallback);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &strHeader);

    /* not receive the whole http response */
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, DataCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, nullptr);
    return 0;
}

int32_t HttpRequest::SetCurlOptCommon(const std::unique_ptr<CURL, CURLClean> &curl, const std::string &strUrl)
{
    /* Print request connection process and return http data on the screen */
    curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 0L);

    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errorBuffer_);
    /* not include the headers in the write callback */
    curl_easy_setopt(curl.get(), CURLOPT_HEADER, 0L);
    /* Specify url content */
    curl_easy_setopt(curl.get(), CURLOPT_URL, strUrl.c_str());

    /* https support */
    /* the connection succeeds regardless of the peer certificate validation */
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    /* the connection succeeds regardless of the names in the certificate. */
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);

    /* Allow redirect */
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    /* Set the maximum number of subsequent redirects */
    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 1L);

    /* connection timeout time */
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, connectionTimeout_);
    /* transfer operation timeout time */
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, transOpTimeout_);

    CURLcode errCode = CURLE_OK;
    if (!ifaceName_.empty()) {
        NETMGR_LOG_D("CURLOPT_INTERFACE:[%{public}s]!", ifaceName_.c_str());
        errCode = curl_easy_setopt(curl.get(), CURLOPT_INTERFACE, ifaceName_.c_str());
        if (errCode != CURLE_OK) {
            NETMGR_LOG_E("CURLOPT_INTERFACE failed:%d!", errCode);
        }
    }
    return static_cast<int32_t>(errCode);
}

int32_t HttpRequest::DataCallback(char *data, size_t size, size_t nmemb, std::string *strBuffer)
{
    if (strBuffer == nullptr || data == nullptr || size == 0) {
        return 0;
    }

    int32_t writtenLen = size * nmemb;
    strBuffer->append(data, writtenLen);
    return writtenLen;
}
}  // namespace NetManagerStandard
}  // namespace OHOS