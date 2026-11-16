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

#include "net_http_probe.h"

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <netdb.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "fwmark_client.h"
#include "net_manager_constants.h"
#include "net_http_client.h"
#include "net_mgr_log_wrapper.h"
#include "net_proxy_userinfo.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t HTTP_OK_CODE = 200;
constexpr int32_t HTTP_SUCCESS_CODE = 204;
constexpr int32_t DEFAULT_CONTENT_LENGTH_VALUE = -1;
constexpr int32_t MIN_VALID_CONTENT_LENGTH_VALUE = 5;
constexpr int32_t FAIL_CODE = 599;
constexpr int32_t PORTAL_CODE = 302;
constexpr int32_t HTTP_RES_CODE_BAD_REQUEST = 400;
constexpr int32_t HTTP_RES_CODE_CLIENT_ERRORS_MAX = 499;
constexpr int CURL_CONNECT_TIME_OUT_MS = 10000;
constexpr int CURL_OPERATE_TIME_OUT_MS = 10000;
constexpr int CURL_PORTAL_DETECTION_TIME_OUT_S = 1;
constexpr int32_t DOMAIN_IP_ADDR_LEN_MAX = 128;
constexpr int32_t DEFAULT_HTTP_PORT = 80;
constexpr int32_t DEFAULT_HTTPS_PORT = 443;
constexpr const char *ADDR_SEPARATOR = ",";
constexpr const char *SYMBOL_COLON = ":";
const std::string DEFAULT_USER_AGENT = std::string("Mozilla/5.0 (X11; Linux x86_64) ") +
    std::string("AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.32 Safari/537.36");
const std::string CONNECTION_CLOSE_VALUE = "close";
const std::string CONTENT_LENGTH_KEY = "content-length";
const std::string CONNECTION_KEY = "connection";
const std::string KEY_WORDS_REDIRECTION = "location.replace";
const std::string HTML_TITLE_HTTP_EN = "http://";
const std::string HTML_TITLE_HTTPS_EN = "https://";
const std::string PORTAL_IFACE_FALLBACK = "wlan0";
constexpr const char *TLS12_SECURITY_CIPHER_SUITE =
        "DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-DSS-AES128-GCM-SHA256:DHE-DSS-AES256-GCM-SHA384:"
        "PSK-AES256-GCM-SHA384:DHE-PSK-AES128-GCM-SHA256:DHE-PSK-AES256-GCM-SHA384:DHE-PSK-CHACHA20-POLY1305:"
        "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:"
        "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-PSK-CHACHA20-POLY1305:DHE-RSA-AES128-CCM:"
        "DHE-RSA-AES256-CCM:DHE-RSA-CHACHA20-POLY1305:PSK-AES256-CCM:DHE-PSK-AES128-CCM:DHE-PSK-AES256-CCM:"
        "ECDHE-ECDSA-AES128-CCM:ECDHE-ECDSA-AES256-CCM:ECDHE-ECDSA-CHACHA20-POLY1305";
constexpr const char *TLS13_SECURITY_CIPHER_SUITE = "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:"
        "TLS_AES_128_GCM_SHA256:TLS_AES_128_CCM_SHA256";
} // namespace

NetHttpProbe::NetHttpProbe(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo,
    ProbeType probeType, std::string ipAddrList)
    : netId_(netId), netBearType_(bearType), netLinkInfo_(netLinkInfo),
    probeType_(probeType), ipAddrList_(ipAddrList)
{
}

NetHttpProbe::NetHttpProbe()
    : netId_(0), netBearType_(BEARER_WIFI), probeType_(ProbeType::PROBE_HTTP)
{}

NetHttpProbe::~NetHttpProbe() {}

void NetHttpProbe::SetXReqId(const std::string& xReqId, int8_t xReqIdLen)
{
    xReqId_ = xReqId;
    xReqIdLen_ = xReqIdLen;
}

int32_t NetHttpProbe::SendProbe(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl)
{
    NETMGR_LOG_I("Send net:[%{public}d] %{public}s probe in", netId_,
        ((IsHttpsDetect(probeType)) ? "https" : "http"));
    ClearProbeResult();
    /* Historical behavior: a combined/unknown probe type never reaches a transfer and fails fast. */
    if (!IsHttpDetect(probeType) && !IsHttpsDetect(probeType)) {
        NETMGR_LOG_E("Invalid probe type:%{public}u", static_cast<uint32_t>(probeType));
        return NETMANAGER_ERR_INTERNAL;
    }

    if (IsHttpDetect(probeType) && !ExecuteProbe(ProbeType::PROBE_HTTP, httpUrl, httpProbeResult_)) {
        NETMGR_LOG_E("Set http probe options failed");
        return NETMANAGER_ERR_INTERNAL;
    }

    if (IsHttpsDetect(probeType) && !ExecuteProbe(ProbeType::PROBE_HTTPS, httpsUrl, httpsProbeResult_)) {
        NETMGR_LOG_E("Set https probe options failed");
        return NETMANAGER_ERR_INTERNAL;
    }

    if (!defaultUseGlobalHttpProxy_) {
        defaultUseGlobalHttpProxy_ = true;
    }
    return NETMANAGER_SUCCESS;
}

bool NetHttpProbe::ExecuteProbe(ProbeType type, const std::string &url, NetHttpProbeResult &result)
{
    NetHttp::HttpRequest request;
    if (!BuildHttpRequest(type, url, request)) {
        return false;
    }
    NetHttp::HttpResponse response = NetHttp::NetHttpClient::GetInstance().Execute(request);
    int32_t checkedCode = CheckRespCode(response);
    std::string redirectUrl = response.redirectUrl.empty() ? response.effectiveUrl : response.redirectUrl;
    result = NetHttpProbeResult(checkedCode, redirectUrl);
    NETMGR_LOG_I("Recv net[%{public}d] probe response, code:[%{public}d], raw code:[%{public}d], cost:[%{public}lld]",
                 netId_, result.GetCode(),
                 response.IsTransportSuccess() ? response.responseCode : 0,
                 static_cast<long long>(response.totalCostMs));
    return true;
}

NetHttpProbeResult NetHttpProbe::GetHttpProbeResult() const
{
    return httpProbeResult_;
}

NetHttpProbeResult NetHttpProbe::GetHttpsProbeResult() const
{
    return httpsProbeResult_;
}

void NetHttpProbe::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    std::lock_guard<std::mutex> locker(proxyMtx_);
    globalHttpProxy_ = httpProxy;
}

void NetHttpProbe::ClearProbeResult()
{
    httpProbeResult_ = {};
    httpsProbeResult_ = {};
}

std::string NetHttpProbe::ExtractDomainFormUrl(const std::string &url)
{
    if (url.empty()) {
        return std::string();
    }

    size_t doubleSlashPos = url.find("//");
    if (doubleSlashPos == std::string::npos) {
        return url;
    }

    std::string domain;
    size_t domainStartPos = doubleSlashPos + 2;
    size_t domainEndPos = url.find('/', domainStartPos);
    if (domainEndPos != std::string::npos) {
        domain = url.substr(domainStartPos, domainEndPos - domainStartPos);
    } else {
        domain = url.substr(domainStartPos);
    }
    return domain;
}

std::string NetHttpProbe::GetAddrInfo(const std::string &domain)
{
    if (domain.empty()) {
        NETMGR_LOG_E("domain is empty");
        return std::string();
    }

    struct addrinfo *result = nullptr;
    struct queryparam qparam = {};
    qparam.qp_netid = static_cast<int>(netId_);
    qparam.qp_type = QEURY_TYPE_NETSYS;

    int32_t ret = getaddrinfo_ext(domain.c_str(), nullptr, nullptr, &result, &qparam);
    if (ret < 0) {
        NETMGR_LOG_E("Get net[%{public}d] address info failed,errno[%{public}d]:%{public}s", netId_, errno,
                     strerror(errno));
        return std::string();
    }

    std::string ipAddress;
    char ip[DOMAIN_IP_ADDR_LEN_MAX] = {0};
    for (addrinfo *tmp = result; tmp != nullptr; tmp = tmp->ai_next) {
        errno_t err = memset_s(&ip, sizeof(ip), 0, sizeof(ip));
        if (err != EOK) {
            NETMGR_LOG_E("memset_s failed,err:%{public}d", err);
            freeaddrinfo(result);
            return std::string();
        }
        if (tmp->ai_family == AF_INET) {
            auto addr = reinterpret_cast<sockaddr_in *>(tmp->ai_addr);
            if (!inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip))) {
                continue;
            }
        } else if (tmp->ai_family == AF_INET6) {
            auto addr = reinterpret_cast<sockaddr_in6 *>(tmp->ai_addr);
            if (!inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip))) {
                continue;
            }
        }
        if (ipAddress.find(ip) != std::string::npos) {
            continue;
        }
        ipAddress = ipAddress.empty() ? (ipAddress + ip) : (ipAddress + ADDR_SEPARATOR + ip);
    }

    freeaddrinfo(result);

    if (ipAddress.empty()) {
        NETMGR_LOG_E("Get net[%{public}d] address info return nullptr result",  netId_);
        return std::string();
    }
    return ipAddress;
}

bool NetHttpProbe::BuildHttpRequest(ProbeType type, const std::string &url, NetHttp::HttpRequest &request)
{
    if (url.empty()) {
        NETMGR_LOG_E("Probe url is empty");
        return false;
    }
    request.url = url;
    request.method = NetHttp::HttpMethod::GET;
    request.headers["Accept"] = "";
    request.headers["Connection"] = "close";
    request.headers["User-Agent"] = DEFAULT_USER_AGENT;
    request.headers["Accept-Encoding"] = "gzip";
    /* one-shot probe request, never reuse the connection */
    request.forbidReuse = true;
    /* connection timeout time */
    request.connectTimeoutMs = CURL_CONNECT_TIME_OUT_MS;
    /* transfer operation timeout time */
    request.timeoutMs = CURL_OPERATE_TIME_OUT_MS;
    /* bind the probe to the interface of this network */
    request.bindInterface = netLinkInfo_.ifaceName_;

    if (IsHttpsDetect(type)) {
        /* the connection succeeds regardless of the peer certificate validation */
        request.tls.verifyPeer = false;
        request.tls.minVersion = NetHttp::TlsVersion::TLS_V1_2;
        request.tls.cipherList = TLS12_SECURITY_CIPHER_SUITE;
        request.tls.tls13CipherList = TLS13_SECURITY_CIPHER_SUITE;
    }

    bool useProxy = false;
    if (!SetProxyOption(type, request, useProxy)) {
        NETMGR_LOG_E("Set http probe proxy option failed.");
        return false;
    }
    if (!SendDnsProbe(type, url, request, useProxy)) {
        NETMGR_LOG_E("Set resolve option failed.");
        return false;
    }
    return true;
}

bool NetHttpProbe::SetProxyOption(ProbeType probeType, NetHttp::HttpRequest &request, bool &useHttpProxy)
{
    useHttpProxy = false;
    /* WIFI or Ethernet require the use of proxy for network detection */
    if (netBearType_ != BEARER_WIFI && netBearType_ != BEARER_ETHERNET) {
        NETMGR_LOG_W("Net:[%{public}d] bear type:[%{public}d], no proxy probe required.", netId_, probeType);
        return true;
    }

    std::string proxyHost;
    int32_t proxyPort = 0;
    /* Prioritize the use of global HTTP proxy, if there is no global proxy, use network http proxy */
    if (!LoadProxy(proxyHost, proxyPort)) {
        NETMGR_LOG_D("global http proxy or network proxy is empty.");
        return true;
    }

    std::string proxyDomain = ExtractDomainFormUrl(proxyHost);
    if (proxyDomain.empty()) {
        NETMGR_LOG_E("Extract proxy domain from host return empty.");
        return true;
    }

    std::string proxyIpAddress = GetAddrInfo(proxyDomain);
    if (proxyIpAddress.empty()) {
        NETMGR_LOG_E("Resolve proxy domain [%{public}s] failed.", proxyDomain.c_str());
        return false;
    }

    NETMGR_LOG_I("Using proxy for http probe on netId:[%{public}d]", netId_);
    NetHttp::HttpProxyConfig proxyConfig;
    proxyConfig.host = proxyHost;
    proxyConfig.port = static_cast<uint16_t>(proxyPort);
    proxyConfig.useTunnel = true;
    FillProxyUserInfo(proxyConfig);
    request.proxy = proxyConfig;
    request.resolves.push_back(proxyDomain + SYMBOL_COLON + std::to_string(proxyPort) + SYMBOL_COLON +
                               proxyIpAddress);
    useHttpProxy = true;
    return true;
}

void NetHttpProbe::FillProxyUserInfo(NetHttp::HttpProxyConfig &proxyConfig)
{
    HttpProxy tempProxy;
    auto userInfoHelp = NetProxyUserinfo::GetInstance();
    userInfoHelp.GetHttpProxyHostPass(tempProxy);
    proxyConfig.username = tempProxy.GetUsername();
    proxyConfig.password = tempProxy.GetPassword();
    if (proxyConfig.username.empty()) {
        NETMGR_LOG_I("username is empty.");
    }
    if (proxyConfig.password.empty()) {
        NETMGR_LOG_I("passwd is empty.");
    }
}

bool NetHttpProbe::SendDnsProbe(ProbeType probeType, const std::string &url, NetHttp::HttpRequest &request,
                                const bool useProxy)
{
    if (useProxy) {
        NETMGR_LOG_W("Net[%{public}d] probe use http proxy,no DNS detection required.", netId_);
        return true;
    }
    int32_t port = IsHttpDetect(probeType) ? DEFAULT_HTTP_PORT : DEFAULT_HTTPS_PORT;
    if (!ipAddrList_.empty()) {
        return AppendResolve(request, ExtractDomainFormUrl(url), ipAddrList_, port);
    }

    std::string domain = ExtractDomainFormUrl(url);
    if (domain.empty()) {
        NETMGR_LOG_E("The domain extracted from [%{public}s] is empty", url.c_str());
        return false;
    }
    if (netBearType_ == BEARER_CELLULAR && IsHttpDetect(probeType)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    NETMGR_LOG_D("Get net[%{public}d] ip addr for probe url ", netId_);
    std::string ipAddress = GetAddrInfo(domain);
    return AppendResolve(request, domain, ipAddress, port);
}

bool NetHttpProbe::AppendResolve(NetHttp::HttpRequest &request, const std::string &domain,
                                 const std::string &ipAddress, int32_t port)
{
    if (domain.empty()) {
        NETMGR_LOG_E("domain is empty");
        return false;
    }

    if (ipAddress.empty()) {
        NETMGR_LOG_E("ipAddress is empty");
        return false;
    }

    request.resolves.push_back(domain + SYMBOL_COLON + std::to_string(port) + SYMBOL_COLON + ipAddress);
    return true;
}

int32_t NetHttpProbe::CheckRespCode(const NetHttp::HttpResponse &response)
{
    /* A failed transfer historically produced code 0 (no response read), keep that. */
    int32_t respCode = response.IsTransportSuccess() ? response.responseCode : 0;
    NETMGR_LOG_D("net[%{public}d], response code before check:%{public}d", netId_, respCode);
    if (respCode == HTTP_OK_CODE) {
        int32_t lengthValue = DEFAULT_CONTENT_LENGTH_VALUE;
        auto iter = response.headers.find(CONTENT_LENGTH_KEY);
        if (iter != response.headers.end()) {
            lengthValue = CommonUtils::StrToInt(iter->second, DEFAULT_CONTENT_LENGTH_VALUE);
        }
        if (lengthValue == DEFAULT_CONTENT_LENGTH_VALUE) {
            if (response.headers.empty()) {
                NETMGR_LOG_I("net[%{public}d], response code 200 with content length -1, consider as fail", netId_);
                return FAIL_CODE;
            }
        } else if (lengthValue < MIN_VALID_CONTENT_LENGTH_VALUE) {
            NETMGR_LOG_I("net[%{public}d], response code 200, content length less 5, consider as fail", netId_);
            return FAIL_CODE;
        }

        auto connIter = response.headers.find(CONNECTION_KEY);
        if (connIter != response.headers.end() &&
            CONNECTION_CLOSE_VALUE.compare(CommonUtils::ToLower(connIter->second)) == 0 &&
            probeType_ == ProbeType::PROBE_HTTP) {
            NETMGR_LOG_I("net[%{public}d] http detection, response code 200 with connection close, consider as fail",
                netId_);
            return FAIL_CODE;
        }
    }
    int32_t result = respCode;
    if (IsHttpDetect(probeType_)) {
        result = CheckSuccessRespCode(response, result);
        result = CheckClientErrorRespCode(response, result);
    }
    return result;
}

int32_t NetHttpProbe::CheckSuccessRespCode(const NetHttp::HttpResponse &response, int32_t respCode)
{
    int32_t result = respCode;
    if (result != HTTP_SUCCESS_CODE) {
        return result;
    }
    // LCOV_EXCL_START
    if (xReqIdLen_ < 0) {
        NETMGR_LOG_I("xReqIdLen: %{public}d", xReqIdLen_);
        return result;
    }
    std::string requestId;
    auto iter = response.headers.find(CommonUtils::ToLower(xReqId_));
    if (iter != response.headers.end()) {
        requestId = iter->second;
    }
    if (requestId.empty() || xReqIdLen_ < 0 ||
        static_cast<size_t>(xReqIdLen_) != requestId.length()) {
        NETMGR_LOG_I("http return 204, but request id error and unreachable! xReqIdLen:%{public}d, rlen: %{public}lu",
            xReqIdLen_, requestId.length());
        result = FAIL_CODE;
    }
    // LCOV_EXCL_STOP
    return result;
}

int32_t NetHttpProbe::CheckClientErrorRespCode(const NetHttp::HttpResponse &response, int32_t respCode)
{
    int32_t result = respCode;
    if (respCode >= HTTP_RES_CODE_BAD_REQUEST && respCode <= HTTP_RES_CODE_CLIENT_ERRORS_MAX) {
        const std::string &errMsg = response.errMessage;
        if ((errMsg.find(HTML_TITLE_HTTP_EN) != std::string::npos ||
            errMsg.find(HTML_TITLE_HTTPS_EN) != std::string::npos) &&
            errMsg.find(KEY_WORDS_REDIRECTION) != std::string::npos) {
            NETMGR_LOG_I("net[%{public}d] reset url in content, consider as portal when http return %{public}d",
                netId_, respCode);
            result = PORTAL_CODE;
        }
    }
    return result;
}

bool NetHttpProbe::NetDetection(const std::string& portalUrl, PortalResponse& resp)
{
    if (portalUrl.size() > MAX_URL_LEN || portalUrl.empty()) {
        NETMGR_LOG_E("rawUrl too long or empty");
        return false;
    }
    /* Keep the historical behavior: portal detection is bound to wlan0 unless the
     * probe was constructed with a concrete network link info. */
    NetHttp::HttpRequest request;
    request.url = portalUrl;
    request.timeoutMs = CURL_PORTAL_DETECTION_TIME_OUT_S * 1000;
    request.bindInterface = netLinkInfo_.ifaceName_.empty() ? PORTAL_IFACE_FALLBACK : netLinkInfo_.ifaceName_;
    NetHttp::HttpResponse response = NetHttp::NetHttpClient::GetInstance().Execute(request);
    // LCOV_EXCL_START
    if (!response.IsTransportSuccess()) {
        NETMGR_LOG_E("net detection request failed, err: %{public}d %{public}s", response.rawCurlCode,
                     response.errMessage.c_str());
        return false;
    }
    resp.respCode = response.responseCode;
    std::string respUrl = CommonUtils::ExtractMetaRefreshUrl(response.body);
    if (strcpy_s(resp.url, MAX_URL_LEN - 1, respUrl.c_str()) != 0) {
        NETMGR_LOG_E("url copy failed");
        return false;
    }
    return true;
    // LCOV_EXCL_STOP
}

int32_t NetHttpProbe::LoadProxy(std::string &proxyHost, int32_t &proxyPort)
{
    std::lock_guard<std::mutex> locker(proxyMtx_);
    if (!globalHttpProxy_.GetHost().empty() && defaultUseGlobalHttpProxy_) {
        proxyHost = globalHttpProxy_.GetHost();
        proxyPort = static_cast<int32_t>(globalHttpProxy_.GetPort());
    } else if (!netLinkInfo_.httpProxy_.GetHost().empty()) {
        proxyHost = netLinkInfo_.httpProxy_.GetHost();
        proxyPort = static_cast<int32_t>(netLinkInfo_.httpProxy_.GetPort());
    } else {
        return false;
    }
    return true;
}

void NetHttpProbe::ProbeWithoutGlobalHttpProxy()
{
    defaultUseGlobalHttpProxy_ = false;
}

bool NetHttpProbe::IsHttpDetect(ProbeType probeType)
{
    return probeType == ProbeType::PROBE_HTTP || probeType == ProbeType::PROBE_HTTP_FALLBACK;
}

bool NetHttpProbe::IsHttpsDetect(ProbeType probeType)
{
    return probeType == ProbeType::PROBE_HTTPS || probeType == ProbeType::PROBE_HTTPS_FALLBACK;
}

} // namespace NetManagerStandard
} // namespace OHOS
