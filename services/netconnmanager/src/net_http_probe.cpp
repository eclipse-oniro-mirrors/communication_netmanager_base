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

#include <cerrno>
#include <memory>
#include <numeric>
#include <unistd.h>

#include "fwmark_client.h"
#include "netsys_controller.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

#define NETPROBE_CURL_EASY_SET_OPTION(handle, opt, data)                                                     \
    do {                                                                                                     \
        CURLcode result = curl_easy_setopt(handle, opt, data);                                               \
        if (result != CURLE_OK) {                                                                            \
            const char *err = curl_easy_strerror(result);                                                    \
            NETMGR_LOG_E("Failed to set curl option: %{public}s, %{public}s %{public}d", #opt, err, result); \
            return false;                                                                                    \
        }                                                                                                    \
    } while (0)

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int PERFORM_POLL_INTERVAL_MS = 50;
constexpr int CURL_CONNECT_TIME_OUT_MS = 5000;
constexpr int CURL_OPERATE_TIME_OUT_MS = 5000;
constexpr int32_t DOMAIN_IP_ADDR_LEN_MAX = 128;
constexpr int32_t DEFAULT_HTTP_PORT = 80;
constexpr int32_t DEFAULT_HTTPS_PORT = 443;
constexpr const char *ADDR_SEPARATOR = ",";
constexpr const char *SYMBOL_COLON = ":";
} // namespace

std::mutex NetHttpProbe::initCurlMutex_;
int32_t NetHttpProbe::useCurlCount_ = 0;
bool NetHttpProbe::CurlGlobalInit()
{
    NETMGR_LOG_D("curl_global_init() in");
    std::lock_guard<std::mutex> lock(initCurlMutex_);
    if (useCurlCount_ == 0) {
        NETMGR_LOG_D("Call curl_global_init()");
        if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
            NETMGR_LOG_E("curl_global_init() failed");
            return false;
        }
    }
    useCurlCount_++;
    NETMGR_LOG_D("curl_global_init() count:[%{public}d]", useCurlCount_);
    return true;
}

void NetHttpProbe::CurlGlobalCleanup()
{
    NETMGR_LOG_D("CurlGlobalCleanup() in");
    std::lock_guard<std::mutex> lock(initCurlMutex_);
    useCurlCount_ = useCurlCount_ > 0 ? (useCurlCount_ - 1) : 0;
    NETMGR_LOG_D("Curl global used count remain:[%{public}d]", useCurlCount_);
    if (useCurlCount_ == 0) {
        NETMGR_LOG_D("Call curl_global_cleanup()");
        curl_global_cleanup();
    }
}

NetHttpProbe::NetHttpProbe(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo)
    : netId_(netId), netBearType_(bearType), netLinkInfo_(netLinkInfo)
{
    isCurlInit_ = NetHttpProbe::CurlGlobalInit();
}

NetHttpProbe::~NetHttpProbe()
{
    NetHttpProbe::CurlGlobalCleanup();
    isCurlInit_ = false;
}

int32_t NetHttpProbe::SendProbe(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl)
{
    NETMGR_LOG_I("Send net:[%{public}d] %{public}s probe in", netId_,
                 ((probeType == ProbeType::PROBE_HTTP_HTTPS)
                      ? "HTTP&HTTPS"
                      : ((probeType == ProbeType::PROBE_HTTPS) ? "https" : "http")));
    ClearProbeResult();
    if (!CheckCurlGlobalInitState()) {
        return NETMANAGER_ERR_INTERNAL;
    }

    if (!InitHttpCurl(probeType)) {
        CleanHttpCurl();
        return NETMANAGER_ERR_INTERNAL;
    }

    if (!SetCurlOptions(probeType, httpUrl, httpsUrl)) {
        NETMGR_LOG_E("Set http/https probe options failed");
        CleanHttpCurl();
        return NETMANAGER_ERR_INTERNAL;
    }

    SendHttpProbeRequest();
    RecvHttpProbeResponse();
    CleanHttpCurl();
    return NETMANAGER_SUCCESS;
}

NetHttpProbeResult NetHttpProbe::GetHttpProbeResult() const
{
    return httpProbeResult_;
}

NetHttpProbeResult NetHttpProbe::GetHttpsProbeResult() const
{
    return httpsProbeResult_;
}

void NetHttpProbe::UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo)
{
    netLinkInfo_ = netLinkInfo;
}

void NetHttpProbe::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    std::lock_guard<std::mutex> locker(proxyMtx_);
    globalHttpProxy_ = httpProxy;
}

bool NetHttpProbe::CheckCurlGlobalInitState()
{
    if (!isCurlInit_) {
        NETMGR_LOG_E("Curl global does not initialized, attempting to reinitialize.");
        isCurlInit_ = NetHttpProbe::CurlGlobalInit();
    }
    return isCurlInit_;
}

void NetHttpProbe::CleanHttpCurl()
{
    if (httpCurl_) {
        if (curlMulti_) {
            curl_multi_remove_handle(curlMulti_, httpCurl_);
        }
        curl_easy_cleanup(httpCurl_);
        httpCurl_ = nullptr;
    }

    if (httpsCurl_) {
        if (curlMulti_) {
            curl_multi_remove_handle(curlMulti_, httpsCurl_);
        }
        curl_easy_cleanup(httpsCurl_);
        httpsCurl_ = nullptr;
    }

    if (httpResolveList_) {
        curl_slist_free_all(httpResolveList_);
        httpResolveList_ = nullptr;
    }

    if (httpsResolveList_) {
        curl_slist_free_all(httpsResolveList_);
        httpsResolveList_ = nullptr;
    }

    if (curlMulti_) {
        curl_multi_cleanup(curlMulti_);
        curlMulti_ = nullptr;
    }
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

    std::vector<AddrInfo> result;
    AddrInfo hints = {};
    std::string serverName;
    if (NetsysController::GetInstance().GetAddrInfo(domain, serverName, hints, netId_, result) < 0) {
        NETMGR_LOG_E("Get net[%{public}d] address info failed,errno[%{public}d]:%{public}s", netId_, errno,
                     strerror(errno));
        return std::string();
    }
    if (result.empty()) {
        NETMGR_LOG_E("Get net[%{public}d] address info return nullptr result", netId_);
        return std::string();
    }

    std::string ipAddress;
    char ip[DOMAIN_IP_ADDR_LEN_MAX] = {0};
    for (auto &node : result) {
        errno_t err = memset_s(&ip, sizeof(ip), 0, sizeof(ip));
        if (err != EOK) {
            NETMGR_LOG_E("memset_s failed,err:%{public}d", err);
            return std::string();
        }

        if (node.aiFamily == AF_INET) {
            if (!inet_ntop(AF_INET, &node.aiAddr.sin.sin_addr, ip, sizeof(ip))) {
                continue;
            }
        } else if (node.aiFamily == AF_INET6) {
            if (!inet_ntop(AF_INET6, &node.aiAddr.sin6.sin6_addr, ip, sizeof(ip))) {
                continue;
            }
        }
        if (ipAddress.find(ip) != std::string::npos) {
            continue;
        }
        ipAddress = ipAddress.empty() ? (ipAddress + ip) : (ipAddress + ADDR_SEPARATOR + ip);
    }
    return ipAddress;
}

bool NetHttpProbe::HasProbeType(ProbeType inputProbeType, ProbeType hasProbeType)
{
    return (inputProbeType & hasProbeType) != 0;
}

bool NetHttpProbe::InitHttpCurl(ProbeType probeType)
{
    curlMulti_ = curl_multi_init();
    if (curlMulti_ == nullptr) {
        NETMGR_LOG_E("curl_multi_init() failed.");
        return false;
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTP)) {
        httpCurl_ = curl_easy_init();
        if (!httpCurl_) {
            NETMGR_LOG_E("httpCurl_ init failed");
            return false;
        }
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTPS)) {
        httpsCurl_ = curl_easy_init();
        if (!httpsCurl_) {
            NETMGR_LOG_E("httpsCurl_ init failed");
            return false;
        }
    }
    return true;
}

bool NetHttpProbe::SetCurlOptions(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl)
{
    bool useProxy = false;
    if (!SetProxyOption(probeType, useProxy)) {
        NETMGR_LOG_E("Set curl proxy option failed.");
        return false;
    }
    if (!SendDnsProbe(probeType, httpUrl, httpsUrl, useProxy)) {
        NETMGR_LOG_E("Set resolve option failed.");
        return false;
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTP)) {
        if (!SetHttpOptions(ProbeType::PROBE_HTTP, httpCurl_, httpUrl)) {
            return false;
        }
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTPS)) {
        if (!SetHttpOptions(ProbeType::PROBE_HTTPS, httpsCurl_, httpsUrl)) {
            return false;
        }
    }

    return true;
}

bool NetHttpProbe::SetHttpOptions(ProbeType probeType, CURL *curl, const std::string &url)
{
    if (!curl) {
        NETMGR_LOG_E("curl is nullptr");
        return false;
    }
    if (url.empty()) {
        NETMGR_LOG_E("Probe url is empty");
        return false;
    }

    NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_VERBOSE, 0L);
    NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_HEADER, 0L);
    NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_URL, url.c_str());
    if (probeType == ProbeType::PROBE_HTTPS) {
        /* the connection succeeds regardless of the peer certificate validation */
        NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        /* the connection succeeds regardless of the names in the certificate. */
        NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_NOSIGNAL, 1L);
    /* connection timeout time */
    NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_CONNECTTIMEOUT_MS, CURL_CONNECT_TIME_OUT_MS);
    /* transfer operation timeout time */
    NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_TIMEOUT_MS, CURL_OPERATE_TIME_OUT_MS);
    NETPROBE_CURL_EASY_SET_OPTION(curl, CURLOPT_INTERFACE, netLinkInfo_.ifaceName_.c_str());

    CURLMcode code = curl_multi_add_handle(curlMulti_, curl);
    if (code != CURLM_OK) {
        NETMGR_LOG_E("curl multi add handle failed, code:[%{public}d]", code);
        return false;
    }
    return true;
}

bool NetHttpProbe::SetProxyOption(ProbeType probeType, bool &useHttpProxy)
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

    NETMGR_LOG_I("Using proxy for http probe on netId:[%{public}d]", netId_);
    bool ret = false;
    if (HasProbeType(probeType, ProbeType::PROBE_HTTP)) {
        if (httpCurl_ == nullptr) {
            NETMGR_LOG_E("httpCurl_ is nullptr");
            return false;
        }
        NETPROBE_CURL_EASY_SET_OPTION(httpCurl_, CURLOPT_PROXY, proxyHost.c_str());
        NETPROBE_CURL_EASY_SET_OPTION(httpCurl_, CURLOPT_PROXYPORT, proxyPort);
        NETPROBE_CURL_EASY_SET_OPTION(httpCurl_, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        NETPROBE_CURL_EASY_SET_OPTION(httpCurl_, CURLOPT_HTTPPROXYTUNNEL, 1L);
        ret = SetResolveOption(ProbeType::PROBE_HTTP, proxyDomain, proxyIpAddress, proxyPort);
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTPS)) {
        if (httpsCurl_ == nullptr) {
            NETMGR_LOG_E("httpsCurl_ is nullptr");
            return false;
        }
        NETPROBE_CURL_EASY_SET_OPTION(httpsCurl_, CURLOPT_PROXY, proxyHost.c_str());
        NETPROBE_CURL_EASY_SET_OPTION(httpsCurl_, CURLOPT_PROXYPORT, proxyPort);
        NETPROBE_CURL_EASY_SET_OPTION(httpsCurl_, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        NETPROBE_CURL_EASY_SET_OPTION(httpsCurl_, CURLOPT_HTTPPROXYTUNNEL, 1L);
        ret &= SetResolveOption(ProbeType::PROBE_HTTPS, proxyDomain, proxyIpAddress, proxyPort);
    }
    useHttpProxy = true;
    return ret;
}

bool NetHttpProbe::SetResolveOption(ProbeType probeType, const std::string &domain, const std::string &ipAddress,
                                    int32_t port)
{
    if (domain.empty()) {
        NETMGR_LOG_E("domain is empty");
        return false;
    }

    if (ipAddress.empty()) {
        NETMGR_LOG_E("ipAddress is empty");
        return false;
    }

    std::string resolve = domain + SYMBOL_COLON + std::to_string(port) + SYMBOL_COLON + ipAddress;
    if (probeType == ProbeType::PROBE_HTTP) {
        httpResolveList_ = curl_slist_append(httpResolveList_, resolve.c_str());
        NETPROBE_CURL_EASY_SET_OPTION(httpCurl_, CURLOPT_RESOLVE, httpResolveList_);
    }

    if (probeType == ProbeType::PROBE_HTTPS) {
        httpsResolveList_ = curl_slist_append(httpsResolveList_, resolve.c_str());
        NETPROBE_CURL_EASY_SET_OPTION(httpsCurl_, CURLOPT_RESOLVE, httpsResolveList_);
    }
    return true;
}

bool NetHttpProbe::SendDnsProbe(ProbeType probeType, const std::string &httpUrl, const std::string &httpsUrl,
                                const bool useProxy)
{
    if (useProxy) {
        NETMGR_LOG_W("Net[%{public}d] probe use http proxy,no DNS detection required.", netId_);
        return true;
    }

    std::string httpDomain;
    std::string httpsDomain;
    if (HasProbeType(probeType, ProbeType::PROBE_HTTP)) {
        httpDomain = ExtractDomainFormUrl(httpUrl);
        if (httpDomain.empty()) {
            NETMGR_LOG_E("The http domain extracted from [%{public}s] is empty", httpUrl.c_str());
            return false;
        }
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTPS)) {
        httpsDomain = ExtractDomainFormUrl(httpsUrl);
        if (httpsDomain.empty()) {
            NETMGR_LOG_E("The https domain extracted from [%{public}s] is empty", httpsUrl.c_str());
            return false;
        }
    }

    std::string ipAddress;
    if (httpDomain == httpsDomain) {
        NETMGR_LOG_I("Get net[%{public}d] ip addr for HTTP&HTTPS probe url ", netId_);
        ipAddress = GetAddrInfo(httpDomain);
        return SetResolveOption(ProbeType::PROBE_HTTP, httpDomain, ipAddress, DEFAULT_HTTP_PORT) &&
               SetResolveOption(ProbeType::PROBE_HTTPS, httpsDomain, ipAddress, DEFAULT_HTTPS_PORT);
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTP)) {
        NETMGR_LOG_I("Get net[%{public}d] ip addr for HTTP probe url ", netId_);
        ipAddress = GetAddrInfo(httpDomain);
        return SetResolveOption(ProbeType::PROBE_HTTP, httpDomain, ipAddress, DEFAULT_HTTP_PORT);
    }

    if (HasProbeType(probeType, ProbeType::PROBE_HTTPS)) {
        NETMGR_LOG_I("Get net[%{public}d] ip addr for HTTPS probe url ", netId_);
        ipAddress = GetAddrInfo(httpsDomain);
        return SetResolveOption(ProbeType::PROBE_HTTPS, httpsDomain, ipAddress, DEFAULT_HTTPS_PORT);
    }
    return false;
}

void NetHttpProbe::SendHttpProbeRequest()
{
    if (!curlMulti_) {
        NETMGR_LOG_E("curlMulti_ is nullptr");
        return;
    }

    int running = 0;
    do {
        CURLMcode result = curl_multi_perform(curlMulti_, &running);
        if ((result == CURLM_OK) && running) {
            result = curl_multi_poll(curlMulti_, nullptr, 0, PERFORM_POLL_INTERVAL_MS, nullptr);
        }
        if (result != CURLM_OK) {
            NETMGR_LOG_E("curl_multi_perform() error, error code:[%{public}d]", result);
            break;
        }
    } while (running);
}

void NetHttpProbe::RecvHttpProbeResponse()
{
    if (!curlMulti_) {
        NETMGR_LOG_E("curlMulti_ is nullptr");
        return;
    }
    CURLMsg *curlMsg = nullptr;
    int32_t msgQueue = 0;
    while ((curlMsg = curl_multi_info_read(curlMulti_, &msgQueue)) != nullptr) {
        if (curlMsg->msg != CURLMSG_DONE) {
            NETMGR_LOG_W("curl multi read not done, msg:[%{public}d]", curlMsg->msg);
            continue;
        }

        if (!curlMsg->easy_handle) {
            NETMGR_LOG_E("Read nullptr curl easy handle");
            continue;
        }

        int64_t responseCode = 0;
        curl_easy_getinfo(curlMsg->easy_handle, CURLINFO_RESPONSE_CODE, &responseCode);

        std::string redirectUrl;
        char* url = nullptr;
        curl_easy_getinfo(curlMsg->easy_handle, CURLINFO_REDIRECT_URL, &url);
        if (url != nullptr) {
            redirectUrl = url;
        } else {
            curl_easy_getinfo(curlMsg->easy_handle, CURLINFO_EFFECTIVE_URL, &url);
            redirectUrl = url;
        }

        if (curlMsg->easy_handle == httpCurl_) {
            httpProbeResult_ = {responseCode, redirectUrl};
            NETMGR_LOG_I("Recv net[%{public}d] http probe response, code:[%{public}d], redirectUrl:[%{public}s]",
                         netId_, httpProbeResult_.GetCode(), httpProbeResult_.GetRedirectUrl().c_str());
        } else if (curlMsg->easy_handle == httpsCurl_) {
            httpsProbeResult_ = {responseCode, redirectUrl};
            NETMGR_LOG_I("Recv net[%{public}d] https probe response, code:[%{public}d], redirectUrl:[%{public}s]",
                         netId_, httpsProbeResult_.GetCode(), httpsProbeResult_.GetRedirectUrl().c_str());
        } else {
            NETMGR_LOG_E("Unknown curl handle.");
        }
    }
}
int32_t NetHttpProbe::LoadProxy(std::string &proxyHost, int32_t &proxyPort)
{
    std::lock_guard<std::mutex> locker(proxyMtx_);
    if (!globalHttpProxy_.GetHost().empty()) {
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
} // namespace NetManagerStandard
} // namespace OHOS
