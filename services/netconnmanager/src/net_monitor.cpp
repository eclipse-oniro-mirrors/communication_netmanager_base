/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <netdb.h>
#include <netinet/tcp.h>
#include <regex>
#include <resolv.h>
#include <securec.h>
#include <sys/socket.h>
#include <unistd.h>
#include "event_report.h"
#include "net_mgr_log_wrapper.h"
#include "fwmark_client.h"
#include "netsys_controller.h"
#include "net_monitor.h"
namespace OHOS {
namespace NetManagerStandard {
static constexpr int32_t INIT_DETECTION_DELAY_MS = 8 * 1000;
static constexpr int32_t MAX_FAILED_DETECTION_DELAY_MS = 5 * 60 * 1000;
static constexpr int32_t SUCCESSED_DETECTION_DELAY_MS = 30 * 1000;
static constexpr int32_t CAPTIVE_PORTAL_DETECTION_DELAY_MS = 60 * 1000;
static constexpr int32_t DOUBLE = 2;
static constexpr int32_t DOMAINIPADDR = 128;
static constexpr int32_t PORTAL_CONTENT_LENGTH_MIN = 4;
static constexpr int32_t NET_CONTENT_LENGTH = 6;
static constexpr uint16_t DEFAULT_PORT = 80;
static constexpr int32_t MAX_RECIVE_SIZE = 2048;
static constexpr int32_t DOMAIN_POSITION = 3;
static constexpr int32_t URLPATH_POSITION = 4;
static constexpr int32_t SOCKET_TIMEOUT = 3;
constexpr const char* PORTAL_URL_REDIRECT_FIRST_CASE = "Location: ";
constexpr const char* PORTAL_URL_REDIRECT_SECOND_CASE = "http";
constexpr const char* CONTENT_STR = "Content-Length:";
constexpr const char* PORTAL_END_STR = ".com";
static std::string MakeDefaultNetDetectionUrl()
{
    std::string url = "http";
    url += "://";
    url += "connectivitycheck";
    url += ".platform";
    url += ".hicloud";
    url += ".com/";
    url += "generate_204";
    return url;
}

NetMonitor::NetMonitor(uint32_t netId, NetDetectionStateHandler handle)
    :netId_(netId), netDetectionStatus_(handle)
{
}

NetMonitor::~NetMonitor()
{
    Stop();
}

void NetMonitor::Start(bool needReport)
{
    needReport_ = needReport;
    if (IsDetecting()) {
        return;
    }
    detecting_ = true;
    detectionThread_ = std::thread([this]() {
        while (detecting_) {
            Detection();
        }
    });
}

void NetMonitor::Stop()
{
    auto now = std::chrono::system_clock::now();
    {
        std::unique_lock<std::mutex> locker(detectionMtx_);
        detecting_ = false;
        detectionSteps_ = 0;
        detectionCond_.notify_all();
    }
    if (detectionThread_.joinable()) {
        detectionThread_.join();
    }
    NETMGR_LOG_I("NetMonitor[%{public}d] Stop cost %{public}lld ms", netId_,
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count());
}

bool NetMonitor::IsDetecting() const
{
    return detecting_;
}

NetDetectionStatus NetMonitor::GetDetectionResult() const
{
    return result_;
}

void NetMonitor::Detection()
{
    NetDetectionStatus result  = SendParallelHttpProbes();
    struct EventInfo eventInfo = {
        .monitorStatus = static_cast<int32_t>(result)
    };
    EventReport::SendMonitorBehaviorEvent(eventInfo);
    std::unique_lock<std::mutex> locker(detectionMtx_);
    if (detecting_) {
        if (result == CAPTIVE_PORTAL_STATE) {
            NETMGR_LOG_I("currentNetMonitor[%{public}d] need portal", netId_);
            detectionDelay_ = CAPTIVE_PORTAL_DETECTION_DELAY_MS;
        } else if (result == VERIFICATION_STATE) {
            NETMGR_LOG_I("currentNetMonitor[%{public}d] evaluation success", netId_);
            detectionDelay_ = SUCCESSED_DETECTION_DELAY_MS;
            detectionSteps_ = 0;
        } else {
            NETMGR_LOG_I("currentNetMonitor[%{public}d] evaluation failed", netId_);
            detectionDelay_ = INIT_DETECTION_DELAY_MS * DOUBLE * detectionSteps_;
            if (detectionDelay_ == 0) {
                detectionDelay_ = INIT_DETECTION_DELAY_MS;
            } else if (detectionDelay_ >= MAX_FAILED_DETECTION_DELAY_MS) {
                detectionDelay_ = MAX_FAILED_DETECTION_DELAY_MS;
            }
            detectionSteps_++;
        }
        if (result != result_ || needReport_) {
            NETMGR_LOG_I("netDetectionStatus_ is need report, needReport_ = %{public}d", needReport_);
            needReport_ = false;
            result_ = result;
            netDetectionStatus_(result_, portalUrlRedirect_);
        }
        detectionCond_.wait_for(locker, std::chrono::milliseconds(detectionDelay_));
    }
}

NetDetectionStatus NetMonitor::SendParallelHttpProbes()
{
    std::string url = MakeDefaultNetDetectionUrl();
    std::string domain;
    std::string urlPath;
    if (ParseUrl(url, domain, urlPath)) {
        NETMGR_LOG_E("ParseUrl error");
        return INVALID_DETECTION_STATE;
    }
    return SendHttpProbe(domain, urlPath, DEFAULT_PORT);
}

NetDetectionStatus NetMonitor::SendHttpProbe(const std::string &defaultDomain, const std::string &defaultUrl,
    const uint16_t defaultPort)
{
    struct hostent ipHost = {};
    char domainIpAddr[DOMAINIPADDR];
    if (GetIpAddr(defaultDomain.c_str(), domainIpAddr, ipHost)) {
        NETMGR_LOG_E("Error at GetIpAddr");
        return INVALID_DETECTION_STATE;
    }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = ipHost.h_addrtype;
    serverAddr.sin_port = htons(defaultPort);
    serverAddr.sin_addr.s_addr = inet_addr(domainIpAddr);
    int32_t sockFd_ = socket(ipHost.h_addrtype, SOCK_STREAM, IPPROTO_TCP);
    if (sockFd_  == -1) {
        NETMGR_LOG_E("Error at socket(), errno is %{public}d:%{public}s", errno, strerror(errno));
        return INVALID_DETECTION_STATE;
    }
    if (SetSocketParameter(sockFd_)) {
        close(sockFd_);
        NETMGR_LOG_E("SetSocketParameter error");
        return INVALID_DETECTION_STATE;
    }
    int32_t connectResult = connect(sockFd_, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (connectResult) {
        close(sockFd_);
        NETMGR_LOG_E("Error at connect socket(), errno is %{public}d:%{public}s", errno, strerror(errno));
        return INVALID_DETECTION_STATE;
    }
    std::string head = "GET /" + defaultUrl + " HTTP/1.1\r\n";
    head.append("Host: " + defaultDomain + "\r\n\r\n");
    int32_t sendResult = send(sockFd_, head.c_str(), static_cast<int32_t>(head.size()), 0);
    if (sendResult <= 0) {
        close(sockFd_);
        NETMGR_LOG_E("Error at send socket(), errno is %{public}d:%{public}s", errno, strerror(errno));
        return INVALID_DETECTION_STATE;
    }
    char buff[MAX_RECIVE_SIZE];
    int32_t recvBytes = recv(sockFd_, buff, MAX_RECIVE_SIZE, 0);
    if (recvBytes <= 0) {
        NETMGR_LOG_E("Error at recv socket(), errno is %{public}d:%{public}s", errno, strerror(errno));
        close(sockFd_);
        return INVALID_DETECTION_STATE;
    }
    return dealRecvResult(std::string(buff, recvBytes), sockFd_);
}

int32_t NetMonitor::SetSocketParameter(int32_t sockFd)
{
    std::unique_ptr<nmd::FwmarkClient> fwmarkClient = std::make_unique<nmd::FwmarkClient>();
    if (fwmarkClient->BindSocket(sockFd, netId_) < 0) {
        NETMGR_LOG_E("Error at BindSocket");
        struct EventInfo eventInfo = {
            .socketFd = sockFd,
            .errorType = static_cast<int32_t>(FAULT_BIND_SOCKET_FAILED),
            .errorMsg = std::string("Bind socket:").append(std::to_string(sockFd)).append(" failed")
        };
        EventReport::SendMonitorFaultEvent(eventInfo);
        return -1;
    }
    int32_t syncnt = SOCKET_TIMEOUT;
    struct timeval timeout;
    timeout.tv_sec = SOCKET_TIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt(sockFd, SOL_SOCKET, TCP_SYNCNT, &syncnt, sizeof(syncnt)) ||
        setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) ||
        setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
        NETMGR_LOG_E("Error at set timeout, errno is %{public}d:%{public}s", errno, strerror(errno));
        return -1;
    }
    return 0;
}

NetDetectionStatus NetMonitor::dealRecvResult(const std::string &strResponse, int32_t sockFd)
{
    std::string urlRedirect = "";
    int32_t retCode = GetUrlRedirectFromResponse(strResponse, urlRedirect);
    int32_t statusCode = GetStatusCodeFromResponse(strResponse);
    portalUrlRedirect_ = urlRedirect;
    NETMGR_LOG_D("statusCode[%{public}d], retCode[%{public}d]", statusCode, retCode);
    if ((statusCode == OK || (statusCode >= BAD_REQUEST && statusCode <= CLIENT_ERROR_MAX)) &&
        retCode > PORTAL_CONTENT_LENGTH_MIN) {
        if (retCode > -1) {
            close(sockFd);
            return CAPTIVE_PORTAL_STATE;
        } else {
            close(sockFd);
            return INVALID_DETECTION_STATE;
        }
    } else if (statusCode == NO_CONTENT) {
        close(sockFd);
        return VERIFICATION_STATE;
    } else if (statusCode != NO_CONTENT && statusCode >= CREATED && statusCode <= URL_REDIRECT_MAX) {
        if (retCode > -1) {
            close(sockFd);
            return CAPTIVE_PORTAL_STATE;
        } else {
            close(sockFd);
            return INVALID_DETECTION_STATE;
        }
    } else {
        close(sockFd);
        return INVALID_DETECTION_STATE;
    }
}

int32_t NetMonitor::GetStatusCodeFromResponse(const std::string &strResponse)
{
    if (strResponse.empty()) {
        NETMGR_LOG_E("strResponse is empty");
        return -1;
    }

    std::string::size_type newLinePos = strResponse.find("\r\n");
    if (newLinePos == std::string::npos) {
        NETMGR_LOG_E("StrResponse did not find the response line!");
        return -1;
    }
    std::string statusLine = strResponse.substr(0, newLinePos);
    std::string::size_type spacePos = statusLine.find(" ");
    if (spacePos == std::string::npos) {
        NETMGR_LOG_E("No spaces found in the response line!");
        return -1;
    }
    std::string strStatusCode = statusLine.substr(spacePos + 1, statusLine.length() - 1);
    std::string::size_type pos = strStatusCode.find(" ");
    if (pos == std::string::npos) {
        NETMGR_LOG_E("No other space was found in the response line!");
        return -1;
    }
    strStatusCode = strStatusCode.substr(0, pos);
    if (strStatusCode.empty()) {
        NETMGR_LOG_E("String status code is empty!");
        return -1;
    }

    int32_t statusCode = std::stoi(strStatusCode);
    return statusCode;
}

int32_t NetMonitor::GetUrlRedirectFromResponse(const std::string &strResponse, std::string &urlRedirect)
{
    if (strResponse.empty()) {
        NETMGR_LOG_E("strResponse is empty");
        return -1;
    }

    std::string::size_type startPos = strResponse.find(PORTAL_URL_REDIRECT_FIRST_CASE);
    if (startPos != std::string::npos) {
        startPos += strlen(PORTAL_URL_REDIRECT_FIRST_CASE);
        std::string::size_type endPos = strResponse.find(PORTAL_END_STR, startPos);
        if (endPos != std::string::npos) {
            urlRedirect = strResponse.substr(startPos, endPos - startPos + strlen(PORTAL_END_STR));
        }
        return 0;
    }

    startPos = strResponse.find(PORTAL_URL_REDIRECT_SECOND_CASE);
    if (startPos != std::string::npos) {
        startPos += strlen(PORTAL_URL_REDIRECT_SECOND_CASE);
        std::string::size_type endPos = strResponse.find(PORTAL_END_STR, startPos);
        if (endPos != std::string::npos) {
            urlRedirect = strResponse.substr(startPos, endPos - startPos + strlen(PORTAL_END_STR));
        }
        startPos = strResponse.find(CONTENT_STR);
        return std::atoi(strResponse.substr(startPos + strlen(CONTENT_STR), NET_CONTENT_LENGTH).c_str());
    }
    return -1;
}

int32_t NetMonitor::GetIpAddr(const char *domain, char *ip_addr, struct hostent &ipHost)
{
    struct hostent *host = gethostbyname(domain);
    if (!host) {
        ip_addr = nullptr;
        NETMGR_LOG_I("gethostbyname failed first time");
        if (res_init() < 0) {
            NETMGR_LOG_E("res_init failed");
            return -1;
        }
        host = gethostbyname(domain);
        if (!host) {
            NETMGR_LOG_E("gethostbyname failed second time");
            return -1;
        }
    }
    ipHost = *host;
    for (int32_t i = 0; host->h_addr_list[i]; i++) {
        if (strcpy_s(ip_addr, DOMAINIPADDR, inet_ntoa(*(struct in_addr*) host->h_addr_list[i])) < 0) {
            NETMGR_LOG_E("strcpy_s ip_addr failed");
            return -1;
        }
        break;
    }
    return 0;
}

int32_t NetMonitor::ParseUrl(const std::string &url, std::string &domain, std::string &urlPath)
{
    std::regex reg_domain_port("/");
    std::cregex_token_iterator itrBegin(url.c_str(), url.c_str() + url.size(), reg_domain_port, -1);
    std::cregex_token_iterator itrEnd;
    int32_t i = 0;
    for (std::cregex_token_iterator itr = itrBegin; itr != itrEnd; ++itr) {
        i++;
        if (i == DOMAIN_POSITION) {
            domain = *itr;
        } else if (i == URLPATH_POSITION) {
            urlPath = *itr;
        }
    }
    return (domain.empty() || urlPath.empty())?-1:0;
}
} // namespace NetManagerStandard
} // namespace OHOS
