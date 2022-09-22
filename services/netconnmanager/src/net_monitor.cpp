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
static constexpr int32_t SOCKET_TIMEOUT = 2;
static constexpr int32_t MAX_SEND_RETRY = 4;
static constexpr uint8_t SOCKET_NOT_READY = 0;
static constexpr uint8_t SOCKET_READY = 1;
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
    NETMGR_LOG_I("Start net[%{public}d] monitor", netId_);
    needReport_ = needReport;
    if (IsDetecting()) {
        NETMGR_LOG_E("Net[%{public}d] monitor is detecting", netId_);
        return;
    }
    detecting_ = true;
    detectionThread_ = std::thread([this]() {
        result_ = UNKNOWN_STATE;
        while (detecting_) {
            Detection();
        }
    });
}

void NetMonitor::Stop()
{
    NETMGR_LOG_I("Stop net[%{public}d] monitor in", netId_);
    {
        std::unique_lock<std::mutex> locker(detectionMtx_);
        detecting_ = false;
        detectionSteps_ = 0;
        detectionCond_.notify_all();
    }
    if (detectionThread_.joinable()) {
        detectionThread_.join();
    }
    NETMGR_LOG_I("Stop net[%{public}d] monitor out", netId_);
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
            NETMGR_LOG_I("Net[%{public}d] monitor need report,result:%{public}d", netId_, result);
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
        NETMGR_LOG_E("Parse net[%{public}d] url:%{public}s error", netId_, url.c_str());
        return INVALID_DETECTION_STATE;
    }
    return SendHttpProbe(domain, urlPath, DEFAULT_PORT);
}

NetDetectionStatus NetMonitor::SendHttpProbe(const std::string &defaultDomain, const std::string &defaultUrl,
    const uint16_t defaultPort)
{
    int socketType = AF_INET;
    std::string ipAddr;
    if (GetIpAddr(defaultDomain.c_str(), ipAddr, socketType)) {
        NETMGR_LOG_E("NetMonitor::Error at GetIpAddr");
        return INVALID_DETECTION_STATE;
    }
    int32_t sockFd = socket(socketType, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (sockFd  < 0) {
        NETMGR_LOG_E("Create net[%{public}d] socket failed,errno[%{public}d]:%{public}s",
            netId_, errno, strerror(errno));
        return INVALID_DETECTION_STATE;
    }
    if (SetSocketParameter(sockFd)) {
        close(sockFd);
        NETMGR_LOG_E("Set net[%{public}d] socket parameter error", netId_);
        return INVALID_DETECTION_STATE;
    }
    if (Connect(sockFd, socketType, defaultPort, ipAddr) < 0) {
        NETMGR_LOG_E("Connect net[%{public}d] nonblock socket:%{public}d failed", netId_, sockFd);
        close(sockFd);
        return INVALID_DETECTION_STATE;
    }
    if (Send(sockFd, defaultDomain, defaultUrl) < 0) {
        NETMGR_LOG_E("Send http probe data to net[%{public}d] socket:%{public}d failed",
            netId_, sockFd);
        close(sockFd);
        return INVALID_DETECTION_STATE;
    }
    std::string probeResult;
    if (Receive(sockFd, probeResult) < 0) {
        NETMGR_LOG_E("Recevie probe result from net[%{public}d] socket:%{public}d failed", netId_, sockFd);
        close(sockFd);
        return INVALID_DETECTION_STATE;
    }
    close(sockFd);
    return dealRecvResult(probeResult);
}

int32_t NetMonitor::Connect(int32_t sockFd, int socketType, const uint16_t port, const std::string &ipAddr)
{
    int flags = fcntl(sockFd, F_GETFL, 0);
    if (flags == -1 && errno == EINTR) {
        flags = fcntl(sockFd, F_GETFL, 0);
        if (flags == -1) {
            NETMGR_LOG_E("Make net[%{public}d] socket:%{public}d non block failed,error[%{public}d]: %s",
                netId_, sockFd, errno, strerror(errno));
            return -1;
        }
    }
    uint32_t tempFlags = (uint32_t)flags | O_NONBLOCK;
    int32_t ret = fcntl(sockFd, F_SETFL, tempFlags);
    if (ret == -1 && errno == EINTR) {
        ret = fcntl(sockFd, F_SETFL, tempFlags);
        if (ret == -1) {
            NETMGR_LOG_E("Make net[%{public}d] socket:%{public}d non block failed,error[%{public}d]: %s",
                netId_, sockFd, errno, strerror(errno));
            return -1;
        }
    }
    if (socketType == AF_INET6) {
        return ConnectIpv6(sockFd, port, ipAddr);
    } else {
        return ConnectIpv4(sockFd, port, ipAddr);
    }
}

int32_t NetMonitor::ConnectIpv4(int32_t sockFd, const uint16_t port, const std::string &ipAddr)
{
    NETMGR_LOG_D("Net[%{public}d] connect ipv4 ip:[%{public}s] socket:%{public}d in",
        netId_, ipAddr.c_str(), sockFd);
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    int32_t ret = connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        if (errno != EINPROGRESS) {
            NETMGR_LOG_E("Connect net[%{public}d] ipv4 ip:%{public}s failed,error[%{public}d]:%{public}s",
                netId_, ipAddr.c_str(), errno, strerror(errno));
            return -1;
        }
    }
    return 0;
}

int32_t NetMonitor::ConnectIpv6(int32_t sockFd, const uint16_t port, const std::string &ipAddr)
{
    NETMGR_LOG_D("Net[%{public}d] connect ipv6 ip:[%{public}s] socket:%{public}d in",
        netId_, ipAddr.c_str(), sockFd);
    struct sockaddr_in6 serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(port);
    inet_pton(AF_INET6, ipAddr.c_str(), &serverAddr.sin6_addr);
    int32_t ret = connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        if (errno != EINPROGRESS) {
            NETMGR_LOG_E("Connect net[%{public}d] ipv6 ip:%{public}s failed,error[%{public}d]:%{public}s",
                netId_, ipAddr.c_str(), errno, strerror(errno));
            return -1;
        }
    }
    return 0;
}

int32_t NetMonitor::Wait(int32_t sockFd, uint8_t *canRead, uint8_t *canWrite)
{
    fd_set writeFds, readFds;
    FD_ZERO(&readFds);
    FD_ZERO(&writeFds);
    if (canRead) {
        FD_SET(sockFd, &readFds);
    }
    if (canWrite) {
        FD_SET(sockFd, &writeFds);
    }
    struct timeval tval = {
        tval.tv_sec = 8,
        tval.tv_usec = 0
    };
    int ret = select((sockFd + 1), &readFds, &writeFds, NULL, &tval);
    if (ret <= 0) {
        NETMGR_LOG_E("Select net[%{public}d] failed,errno[%{public}d]:%{public}s", netId_, errno, strerror(errno));
        return -1;
    }
    if (FD_ISSET(sockFd, &readFds) && canRead) {
        *canRead = SOCKET_READY;
    }
    if (FD_ISSET(sockFd, &writeFds) && canWrite) {
        *canWrite = SOCKET_READY;
    }
    return 0;
}

int32_t NetMonitor::Send(int32_t sockFd, const std::string &domain, const std::string &url)
{
    uint8_t canWrite = SOCKET_NOT_READY;
    int32_t ret = Wait(sockFd, nullptr, &canWrite);
    if (ret < 0 || canWrite != SOCKET_READY) {
        NETMGR_LOG_E("Net[%{public}d] socket not ready to send data,ret:%{public}d, canWrite:%{public}d",
            netId_, ret, canWrite);
        return -1;
    }
    std::string sendData = "GET /" + url + " HTTP/1.1\r\n";
    sendData.append("Host: " + domain + "\r\n\r\n");
    const char *writeBuf = sendData.c_str();
    int32_t bufSize = static_cast<int32_t>(sendData.size());
    int32_t written = 0;
    int32_t retry = 0;
    while (bufSize > 0) {
        written = send(sockFd, writeBuf, bufSize, 0);
        if (written < 0) {
            if (errno == EAGAIN && retry < MAX_SEND_RETRY) {
                ++retry;
                NETMGR_LOG_W("Net[%{public}d] try to resend the data to sokcet:%{public}d for the :%{public}d times",
                    netId_, sockFd, retry);
                continue;
            }
            NETMGR_LOG_E("Net[%{public}d] Send data to socket:%{public}d failed, errno[%{public}d]:%{public}s",
                netId_, sockFd, errno, strerror(errno));
            return -1;
        }
        if (written == 0) {
            break;
        }
        writeBuf += written;
        bufSize -= written;
    }
    return 0;
}

int32_t NetMonitor::Receive(int32_t sockFd, std::string &probResult)
{
    uint8_t canRead = SOCKET_NOT_READY;
    int32_t ret = Wait(sockFd, &canRead, nullptr);
    if (ret < 0 || canRead != SOCKET_READY) {
        NETMGR_LOG_E("Net[%{public}d] socket not ready to read data, ret:%{public}d, canRead:%{public}d",
            netId_, ret, canRead);
        return -1;
    }
    char buff[MAX_RECIVE_SIZE] = {0};
    int32_t recvBytes = recv(sockFd, buff, MAX_RECIVE_SIZE, 0);
    if (recvBytes <= 0) {
        NETMGR_LOG_E("Receive net[%{public}d] data from socket failed,errno[%{public}d]:%{public}s",
            netId_, errno, strerror(errno));
        return -1;
    }
    probResult = std::string(buff, recvBytes);
    return 0;
}

int32_t NetMonitor::SetSocketParameter(int32_t sockFd)
{
    std::unique_ptr<nmd::FwmarkClient> fwmarkClient = std::make_unique<nmd::FwmarkClient>();
    if (fwmarkClient->BindSocket(sockFd, netId_) < 0) {
        NETMGR_LOG_E("Error at bind net[%{public}d] socket", netId_);
        struct EventInfo eventInfo = {
            .socketFd = sockFd,
            .errorType = static_cast<int32_t>(FAULT_BIND_SOCKET_FAILED),
            .errorMsg = std::string("Bind socket:").append(std::to_string(sockFd)).append(" failed")
        };
        EventReport::SendMonitorFaultEvent(eventInfo);
        return -1;
    }
    struct timeval timeout;
    timeout.tv_sec = SOCKET_TIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) ||
        setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
        NETMGR_LOG_E("Error at set net[%{public}d] socket timeout,error[%{public}d]:%{public}s",
            netId_, errno, strerror(errno));
        return -1;
    }
    return 0;
}

NetDetectionStatus NetMonitor::dealRecvResult(const std::string &strResponse)
{
    std::string urlRedirect = "";
    int32_t retCode = GetUrlRedirectFromResponse(strResponse, urlRedirect);
    int32_t statusCode = GetStatusCodeFromResponse(strResponse);
    portalUrlRedirect_ = urlRedirect;
    NETMGR_LOG_D("statusCode[%{public}d], retCode[%{public}d]", statusCode, retCode);
    if ((statusCode == OK || (statusCode >= BAD_REQUEST && statusCode <= CLIENT_ERROR_MAX)) &&
        retCode > PORTAL_CONTENT_LENGTH_MIN) {
        if (retCode > -1) {
            return CAPTIVE_PORTAL_STATE;
        } else {
            return INVALID_DETECTION_STATE;
        }
    } else if (statusCode == NO_CONTENT) {
        return VERIFICATION_STATE;
    } else if (statusCode != NO_CONTENT && statusCode >= CREATED && statusCode <= URL_REDIRECT_MAX) {
        if (retCode > -1) {
            return CAPTIVE_PORTAL_STATE;
        } else {
            return INVALID_DETECTION_STATE;
        }
    } else {
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

int32_t NetMonitor::GetIpAddr(const char *domain, std::string &ip_addr, int &socketType)
{
    struct addrinfo *result = nullptr;
    int ret = getaddrinfo(domain, nullptr, nullptr, &result);
    if (ret < 0) {
        NETMGR_LOG_E("Get net[%{public}d] address info failed,errno[%{public}d]:%{public}s",
            netId_, errno, strerror(errno));
        return -1;
    }
    if (result == nullptr) {
        NETMGR_LOG_E("Get net[%{public}d] address info return nullptr result", netId_);
        return -1;
    }
    char ip[DOMAINIPADDR] = {0};
    if (result->ai_family == AF_INET) {
        auto addr = reinterpret_cast<sockaddr_in *>(result->ai_addr);
        inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
        ip_addr = ip;
    } else if (result->ai_family == AF_INET6) {
        auto addr = reinterpret_cast<sockaddr_in6 *>(result->ai_addr);
        inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
        ip_addr = ip;
    }
    socketType = result->ai_family;
    NETMGR_LOG_D("Get net[%{public}d] monitor ip:%{public}s", netId_, ip_addr.c_str());
    freeaddrinfo(result);
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
