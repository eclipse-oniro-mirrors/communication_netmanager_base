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

#include "url.h"
#include <curl/curl.h>
#include <curl/easy.h>

namespace OHOS {
namespace NetManagerStandard {
static void CurlParserUrl(const std::string &url, std::string &host, std::string &path, int32_t &port)
{
    CURLU *h;
    CURLUcode uc;

    h = curl_url(); /* get a handle to work with */
    if (!h) {
        return;
    }

    /* parse a full URL */
    uc = curl_url_set(h, CURLUPART_URL, url.c_str(), 0);
    if (!uc) {
        char *result;

        uc = curl_url_get(h, CURLUPART_HOST, &result, 0);
        if (!uc) {
            host = result;
            curl_free(result);
        }

        uc = curl_url_get(h, CURLUPART_PATH, &result, 0);
        if (!uc) {
            path = result;
            curl_free(result);
        }

        uc = curl_url_get(h, CURLUPART_PORT, &result, 0);
        if (!uc) {
            port = atoi(result);
            curl_free(result);
        }
    }

    curl_url_cleanup(h); /* free url handle */
}

Url::Url(const std::string &url) : url_(url)
{
    CurlParserUrl(url_, host_, path_, port_);
}

Url::Url(const Url &other) : url_(other.url_)
{
    CurlParserUrl(url_, host_, path_, port_);
}

Url::~Url() {}

std::string Url::ToString() const
{
    return url_;
}

std::string Url::GetHost() const
{
    return host_;
}

std::string Url::GetPath() const
{
    return path_;
}

int32_t Url::GetPort()
{
    return port_;
}

Url &Url::operator=(const std::string &url)
{
    url_ = url;
    CurlParserUrl(url_, host_, path_, port_);
    return *this;
}

bool Url::operator==(const Url &other) const
{
    return url_ == other.url_;
}
} // namespace NetManagerStandard
} // namespace OHOS