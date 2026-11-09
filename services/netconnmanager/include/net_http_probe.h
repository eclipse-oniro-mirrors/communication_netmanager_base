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

#include <atomic>
#include <map>
#include <mutex>
#include <string>

#include "net_http_probe_result.h"
#include "net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {
struct HttpRequest;
struct HttpResponse;
struct HttpProxyConfig;
} // namespace NetHttp
enum ProbeType : uint32_t {
    PROBE_HTTP = 1,
    PROBE_HTTPS = 2,
    PROBE_HTTP_HTTPS = 3,
    PROBE_HTTP_FALLBACK = 4,
    PROBE_HTTPS_FALLBACK = 5
};

/*
 * Http probe business layer: builds probe requests (url / proxy / dns
 * pre-resolution / iface binding), drives them through the shared
 * NetHttp::NetHttpClient and judges the raw response (204 + XReqId success,
 * redirect portal, content-length sanity). All curl handling lives in
 * libnet_http_client, this class holds no curl state.
 */
class NetHttpProbe {
public:
    NetHttpProbe(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo,
        ProbeType probeType, std::string ipAddrList = "");
    NetHttpProbe();
    ~NetHttpProbe();

    int32_t SendProbe(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl);
    NetHttpProbeResult GetHttpProbeResult() const;
    NetHttpProbeResult GetHttpsProbeResult() const;
    void UpdateGlobalHttpProxy(const HttpProxy &httpProxy);
    bool IsHttpDetect(ProbeType probeType);
    bool IsHttpsDetect(ProbeType probeType);
    void ProbeWithoutGlobalHttpProxy();
    bool NetDetection(const std::string& portalUrl, PortalResponse& resp);
    void SetXReqId(const std::string& xReqId, int8_t xReqIdLen);

private:
    bool ExecuteProbe(ProbeType type, const std::string &url, NetHttpProbeResult &result);
    bool BuildHttpRequest(ProbeType type, const std::string &url, NetHttp::HttpRequest &request);
    bool SetProxyOption(ProbeType probeType, NetHttp::HttpRequest &request, bool &useProxy);
    void FillProxyUserInfo(NetHttp::HttpProxyConfig &proxyConfig);
    bool SendDnsProbe(ProbeType probeType, const std::string &url, NetHttp::HttpRequest &request,
                      const bool useProxy);
    bool AppendResolve(NetHttp::HttpRequest &request, const std::string &domain, const std::string &ipAddress,
                       int32_t port);
    void ClearProbeResult();
    std::string ExtractDomainFormUrl(const std::string &url);
    std::string GetAddrInfo(const std::string &domain);
    int32_t LoadProxy(std::string &proxyHost, int32_t &proxyPort);
    int32_t CheckRespCode(const NetHttp::HttpResponse &response);
    int32_t CheckSuccessRespCode(const NetHttp::HttpResponse &response, int32_t respCode);
    int32_t CheckClientErrorRespCode(const NetHttp::HttpResponse &response, int32_t respCode);

private:
    std::mutex proxyMtx_;
    std::atomic<bool> defaultUseGlobalHttpProxy_ = true;
    uint32_t netId_ = 0;
    NetBearType netBearType_ = BEARER_DEFAULT;
    NetLinkInfo netLinkInfo_;
    HttpProxy globalHttpProxy_;
    NetHttpProbeResult httpProbeResult_;
    NetHttpProbeResult httpsProbeResult_;
    ProbeType probeType_;
    std::string ipAddrList_ = "";
    std::string xReqId_;
    int8_t xReqIdLen_ = -1;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_HTTP_PROBE_H
