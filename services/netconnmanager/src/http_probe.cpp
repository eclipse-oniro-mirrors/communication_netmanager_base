/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "http_probe.h"
#include "net_mgr_log_wrapper.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <cstdint>
#include <functional>
#include <vector>
#include <map>
#include <cstring>

namespace OHOS {
namespace NetManagerStandard {
static constexpr int64_t CONNECTION_TIMEOUT = 5000;
static constexpr int64_t TRANSFOR_TIMEOUT = 5000;

struct CurlGlobalInitializer {
    CurlGlobalInitializer() noexcept
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~CurlGlobalInitializer()
    {
        curl_global_cleanup();
    }
} g_curlGlobalInitializer;

struct CurlOptions {
    bool verbose{false};
    CURLcode errCode{CURLE_OK};
    char errorBuf[CURL_ERROR_SIZE];
    int64_t connTimeout{CONNECTION_TIMEOUT};
    int64_t transTimeout{TRANSFOR_TIMEOUT};
    std::string url;
    bool useHttps{false};
    int32_t sockFd{0};
    int32_t resCode{0};
    std::string resMsg;
    std::map<std::string, std::string> fields;
};

static size_t CurlWriteFunction(void *data, size_t size, size_t nmemb, void *userp)
{
    CurlOptions *opts = static_cast<CurlOptions *>(userp);
    if (!opts) {
        return 0;
    }

    size_t realSize = size * nmemb;
    if (realSize <= 0) {
        return 0;
    }

    std::string line(static_cast<const char *>(data), realSize);
    if (line.rfind("HTTP/1.", 0) == 0) {
        int codePos = line.rfind(' ', ::strlen("HTTP/1.x"));
        if (codePos > 0) {
            size_t phrasePos = line.rfind(' ', codePos + 1);
            if (phrasePos > 0 && phrasePos < line.length()) {
                opts->resMsg = line.substr(phrasePos + 1);
            }
            if (phrasePos < 0) {
                phrasePos = line.length();
            }
            // cannot use 'try' with exceptions disabled even -fexceptions specified.
            opts->resCode = std::stoi(line.substr(codePos + 1, phrasePos));
        }
    } else {
        int pos = line.rfind(": ", 0);
        if (pos > 0) {
            std::string fieldName = line.substr(0, pos - 1);
            std::string fieldData = line.substr(pos + 1);
            opts->fields[fieldName] = fieldData;
        }
    }
    return realSize;
}

static curl_socket_t CurlOpenSocketFunction(void *userp, curlsocktype purpose, struct curl_sockaddr *address)
{
    CurlOptions *opts = static_cast<CurlOptions *>(userp);
    if (opts) {
        return opts->sockFd;
    } else {
        return -1;
    }
}

static void CurlSetOptions(CURL *curl, CurlOptions *opts)
{
    /* Print request connection process and return http data on the screen */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, opts->verbose ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, opts->errorBuf);
    /* not include the headers in the write callback */
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    /* Specify url content */
    curl_easy_setopt(curl, CURLOPT_URL, opts->url.c_str());
    /* https support */
    if (opts->useHttps) {
        /* the connection succeeds regardless of the peer certificate validation */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        /* the connection succeeds regardless of the names in the certificate. */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    /* Allow redirect */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    /* Set the maximum number of subsequent redirects */
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1L);
    /* connection timeout time */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, opts->connTimeout);
    /* transfer operation timeout time */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, opts->transTimeout);
    /* cache response data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, opts);
    /* let curl use external socket fd */
    if (opts->sockFd > 0) {
        curl_easy_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, CurlOpenSocketFunction);
        curl_easy_setopt(curl, CURLOPT_OPENSOCKETDATA, opts);
    }
}

static void CurlPerform(CURL *curl, CurlOptions *opts)
{
    CURLcode errCode = curl_easy_perform(curl);
    if (opts) {
        opts->errCode = errCode;
    }
}

HttpProbe::HttpProbe(ProbeType probeType, const std::string &url, int32_t sockFd)
    : curlOpts_(std::make_unique<CurlOptions>())
{
    std::unique_ptr<CURL, std::function<void(CURL *)>> curl(curl_easy_init(), [](CURL *curl) {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    });

    if (curl) {
        curlOpts_->url = url;
        curlOpts_->sockFd = sockFd;
        if (probeType == PROBE_HTTPS) {
            curlOpts_->useHttps = true;
        }
        CurlSetOptions(curl.get(), curlOpts_.get());
        CurlPerform(curl.get(), curlOpts_.get());
    } else {
        curlOpts_->errCode = CURLE_FAILED_INIT;
    }
}

HttpProbe::~HttpProbe() {}

HttpProbeResult HttpProbe::GetResult() const
{
    return HttpProbeResult{curlOpts_->resCode, GetHeaderField("Location")};
}

bool HttpProbe::HasError() const
{
    return curlOpts_->errCode != CURLE_OK;
}

std::string HttpProbe::ErrorString() const
{
    return std::string(curlOpts_->errorBuf);
}

std::string HttpProbe::GetHeaderField(const std::string &name) const
{
    auto iter = curlOpts_->fields.find(name);
    if (iter != curlOpts_->fields.end()) {
        return iter->second;
    }
    return "";
}
} // namespace NetManagerStandard
} // namespace OHOS
