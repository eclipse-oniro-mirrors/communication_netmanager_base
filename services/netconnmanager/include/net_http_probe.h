/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NET_HTTP_PROBE_H
#define NET_HTTP_PROBE_H

#include <curl/curl.h>
#include <curl/easy.h>
#include <map>
#include <mutex>
#include <string>

#include "net_http_probe_result.h"
#include "net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
enum ProbeType : uint32_t { PROBE_HTTP = 1, PROBE_HTTPS = 2, PROBE_HTTP_HTTPS = 3 };

class NetHttpProbe {
public:
    NetHttpProbe(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo);
    ~NetHttpProbe();

    int32_t SendProbe(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl);
    NetHttpProbeResult GetHttpProbeResult() const;
    NetHttpProbeResult GetHttpsProbeResult() const;
    void UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo);
    void UpdateGlobalHttpProxy(const HttpProxy &httpProxy);
    bool HasProbeType(ProbeType inputProbeType, ProbeType hasProbeType);

private:
    static bool CurlGlobalInit();
    static void CurlGlobalCleanup();

    bool CheckCurlGlobalInitState();
    void CleanHttpCurl();
    void ClearProbeResult();
    std::string ExtractDomainFormUrl(const std::string &url);
    std::string GetAddrInfo(const std::string &domain);
    bool InitHttpCurl(ProbeType probeType);
    bool SetCurlOptions(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl);
    bool SetHttpOptions(ProbeType probeType, CURL *curl, const std::string &url);
    bool SetProxyOption(ProbeType probeType, bool &useProxy);
    bool SetResolveOption(ProbeType probeType, const std::string &domain, const std::string &ipAddress, int32_t port);
    bool SendDnsProbe(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl,
                      const bool useProxy);
    void SendHttpProbeRequest();
    void RecvHttpProbeResponse();
    int32_t LoadProxy(std::string &proxyHost, int32_t &proxyPort);

private:
    static std::mutex initCurlMutex_;
    static int32_t useCurlCount_;

    std::mutex proxyMtx_;
    bool isCurlInit_ = false;
    uint32_t netId_ = 0;
    NetBearType netBearType_ = BEARER_DEFAULT;
    NetLinkInfo netLinkInfo_;
    HttpProxy globalHttpProxy_;
    CURLM *curlMulti_ = nullptr;
    CURL *httpCurl_ = nullptr;
    CURL *httpsCurl_ = nullptr;
    curl_slist *httpResolveList_ = nullptr;
    curl_slist *httpsResolveList_ = nullptr;
    NetHttpProbeResult httpProbeResult_;
    NetHttpProbeResult httpsProbeResult_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_HTTP_PROBE_H