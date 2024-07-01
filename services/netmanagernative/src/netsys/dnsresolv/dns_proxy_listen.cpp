/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "dns_config_client.h"
#include "dns_param_cache.h"
#include "netnative_log_wrapper.h"
#include "netsys_udp_transfer.h"
#include "singleton.h"
#include "ffrt.h"

#include "dns_proxy_listen.h"

namespace OHOS {
namespace nmd {
uint16_t DnsProxyListen::netId_ = 0;
std::atomic_bool DnsProxyListen::proxyListenSwitch_ = false;
std::mutex DnsProxyListen::listenerMutex_;
constexpr uint16_t DNS_PROXY_PORT = 53;
constexpr uint8_t RESPONSE_FLAG = 0x80;
constexpr uint8_t RESPONSE_FLAG_USED = 80;
constexpr size_t FLAG_BUFF_LEN = 1;
constexpr size_t FLAG_BUFF_OFFSET = 2;
constexpr size_t DNS_HEAD_LENGTH = 12;
constexpr int32_t EPOLL_TASK_NUMBER = 10;
DnsProxyListen::DnsProxyListen() : proxySockFd_(-1), proxySockFd6_(-1) {}
DnsProxyListen::~DnsProxyListen()
{
    if (proxySockFd_ > 0) {
        close(proxySockFd_);
        proxySockFd_ = -1;
    }
    if (proxySockFd6_ > 0) {
        close(proxySockFd6_);
        proxySockFd6_ = -1;
    }
    if (epollFd_ > 0) {
        close(epollFd_);
        epollFd_ = -1;
    }
    serverIdxOfSocket.clear();
}

void DnsProxyListen::DnsParseBySocket(std::unique_ptr<RecvBuff> &recvBuff, std::unique_ptr<AlignedSockAddr> &clientSock)
{
    int32_t socketFd = -1;
    if (clientSock->sa.sa_family == AF_INET) {
        socketFd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_UDP);
    } else if (clientSock->sa.sa_family == AF_INET6) {
        socketFd = socket(AF_INET6, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_UDP);
    }
    if (socketFd < 0) {
        NETNATIVE_LOGE("socketFd create socket failed %{public}d", errno);
        return;
    }
    if (!PollUdpDataTransfer::MakeUdpNonBlock(socketFd)) {
        NETNATIVE_LOGE("MakeNonBlock error %{public}d: %{public}s", errno, strerror(errno));
        close(socketFd);
        return;
    }
    serverIdxOfSocket.emplace(std::piecewise_construct, std::forward_as_tuple(socketFd),
                              std::forward_as_tuple(socketFd, std::move(clientSock), std::move(recvBuff)));
    SendRequest2Server(socketFd);
}

bool DnsProxyListen::GetDnsProxyServers(std::vector<std::string> &servers, size_t serverIdx)
{
    std::vector<std::string> domains;
    uint16_t baseTimeoutMsec;
    uint8_t retryCount;
    auto status = DnsParamCache::GetInstance().GetResolverConfig(DnsProxyListen::netId_, servers, domains,
                                                                 baseTimeoutMsec, retryCount);
    if (serverIdx >= servers.size()) {
        NETNATIVE_LOGE("no server useful");
        return false;
    }
    return true;
}

void DnsProxyListen::SendRequest2Server(int32_t socketFd)
{
    auto iter = serverIdxOfSocket.find(socketFd);
    if (iter == serverIdxOfSocket.end()) {
        NETNATIVE_LOGE("no idx found");
        close(socketFd);
        return;
    }
    auto serverIdx = iter->second.GetIdx();
    std::vector<std::string> servers;
    if (!GetDnsProxyServers(servers, serverIdx)) {
        serverIdxOfSocket.erase(iter);
        return;
    }
    iter->second.IncreaseIdx();
    epoll_ctl(epollFd_, EPOLL_CTL_DEL, socketFd, nullptr);
    socklen_t addrLen;
    AlignedSockAddr &addrParse = iter->second.GetAddr();
    AlignedSockAddr &clientSock = iter->second.GetClientSock();
    if (clientSock.sa.sa_family == AF_INET) {
        addrParse.sin.sin_family = AF_INET;
        addrParse.sin.sin_port = htons(DNS_PROXY_PORT);
        addrParse.sin.sin_addr.s_addr = inet_addr(servers[serverIdx].c_str());
        if (addrParse.sin.sin_addr.s_addr == INADDR_NONE) {
            NETNATIVE_LOGE("Input dns server [%{public}s] is not correct!", servers[serverIdx].c_str());
            return SendRequest2Server(socketFd);
        }
    } else if (clientSock.sa.sa_family == AF_INET6) {
        if (servers[serverIdx].find(":") == std::string::npos) {
            return SendRequest2Server(socketFd);
        }
        addrParse.sin6.sin6_family = AF_INET6;
        addrParse.sin6.sin6_port = htons(DNS_PROXY_PORT);
        inet_pton(AF_INET6, servers[serverIdx].c_str(), &(addrParse.sin6.sin6_addr));
        if (IN6_IS_ADDR_UNSPECIFIED(&addrParse.sin6.sin6_addr)) {
            NETNATIVE_LOGE("Input dns server [%{public}s] is not correct!", servers[serverIdx].c_str());
            return SendRequest2Server(socketFd);
        }
    }
    if (PollUdpDataTransfer::PollUdpSendData(socketFd, iter->second.GetRecvBuff().questionsBuff,
                                             iter->second.GetRecvBuff().questionLen, addrParse, addrLen) < 0) {
        NETNATIVE_LOGE("send failed %{public}d: %{public}s", errno, strerror(errno));
        return SendRequest2Server(socketFd);
    }
    iter->second.endTime = std::chrono::system_clock::now() + std::chrono::milliseconds(EPOLL_TIMEOUT);
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, socketFd, iter->second.GetEventPtr()) < 0) {
        NETNATIVE_LOGE("epoll add sock %{public}d failed, errno: %{public}d", socketFd, errno);
        serverIdxOfSocket.erase(iter);
    }
}
void DnsProxyListen::SendDnsBack2Client(int32_t socketFd)
{
    NETNATIVE_LOG_D("epoll send back to client.");
    auto iter = serverIdxOfSocket.find(socketFd);
    if (iter == serverIdxOfSocket.end()) {
        NETNATIVE_LOGE("no idx found");
        close(socketFd);
        return;
    }
    AlignedSockAddr &addrParse = iter->second.GetAddr();
    AlignedSockAddr &clientSock = iter->second.GetClientSock();
    NETNATIVE_LOG_D("=====sa_family:[%{public}d]", clientSock.sa.sa_family);
    int32_t proxySocket = proxySockFd_;
    socklen_t addrLen = 0;
    if (clientSock.sa.sa_family == AF_INET) {
        proxySocket = proxySockFd_;
        addrLen = sizeof(sockaddr_in);
    } else {
        proxySocket = proxySockFd6_;
        addrLen = sizeof(sockaddr_in6);
    }
    char requesData[MAX_REQUESTDATA_LEN] = {0};
    int32_t resLen =
        PollUdpDataTransfer::PollUdpRecvData(socketFd, requesData, MAX_REQUESTDATA_LEN, addrParse, addrLen);
    if (resLen > 0 && CheckDnsResponse(requesData, MAX_REQUESTDATA_LEN)) {
        NETNATIVE_LOG_D("send %{public}d back to client.", socketFd);
        DnsSendRecvParseData(proxySocket, requesData, resLen, iter->second.GetClientSock());
        serverIdxOfSocket.erase(iter);
        return;
    }
    NETNATIVE_LOGE("response not correct, retry for next server.");
    SendRequest2Server(socketFd);
}

void DnsProxyListen::DnsSendRecvParseData(int32_t clientSocket, char *requesData, int32_t resLen,
                                          AlignedSockAddr &proxyAddr)
{
    socklen_t addrLen = 0;
    if (proxyAddr.sa.sa_family == AF_INET) {
        addrLen = sizeof(sockaddr_in);
    } else {
        addrLen = sizeof(sockaddr_in6);
    }
    if (PollUdpDataTransfer::PollUdpSendData(clientSocket, requesData, resLen, proxyAddr, addrLen) < 0) {
        NETNATIVE_LOGE("send failed %{public}d: %{public}s", errno, strerror(errno));
    }
}

void DnsProxyListen::StartListen()
{
    NETNATIVE_LOGI("StartListen proxySockFd_ : %{public}d, proxySockFd6_ : %{public}d", proxySockFd_, proxySockFd6_);
    epoll_event proxyEvent;
    epoll_event proxy6Event;
    if (!InitForListening(proxyEvent, proxy6Event)) {
        return;
    }
    epoll_event eventsReceived[EPOLL_TASK_NUMBER];
    while (true) {
        int32_t nfds =
            epoll_wait(epollFd_, eventsReceived, EPOLL_TASK_NUMBER, serverIdxOfSocket.size() == 0 ? -1 : EPOLL_TIMEOUT);
        NETNATIVE_LOG_D("now socket num: %{public}zu", serverIdxOfSocket.size());
        if (nfds < 0) {
            NETNATIVE_LOGE("epoll errno: %{public}d", errno);
            continue; // now ignore all errno.
        }
        if (nfds == 0) {
            // dns timeout
            EpollTimeout();
            continue;
        }
        for (int i = 0; i < nfds; ++i) {
            if (eventsReceived[i].data.fd == proxySockFd_ || eventsReceived[i].data.fd == proxySockFd6_) {
                int32_t family = (eventsReceived[i].data.fd == proxySockFd_) ? AF_INET : AF_INET6;
                GetRequestAndTransmit(family);
            } else {
                SendDnsBack2Client(eventsReceived[i].data.fd);
            }
        }
        CollectSocks();
    }
}
void DnsProxyListen::GetRequestAndTransmit(int32_t family)
{
    NETNATIVE_LOG_D("epoll got request from client.");
    auto recvBuff = std::make_unique<RecvBuff>();
    (void)memset_s(recvBuff->questionsBuff, MAX_REQUESTDATA_LEN, 0, MAX_REQUESTDATA_LEN);

    auto clientAddr = std::make_unique<AlignedSockAddr>();
    int32_t proxySocket = proxySockFd_;
    if (family == AF_INET) {
        socklen_t len = sizeof(sockaddr_in);
        recvBuff->questionLen = recvfrom(proxySockFd_, recvBuff->questionsBuff, MAX_REQUESTDATA_LEN, 0,
                                         reinterpret_cast<sockaddr *>(&(clientAddr->sin)), &len);
    } else {
        socklen_t len = sizeof(sockaddr_in6);
        recvBuff->questionLen = recvfrom(proxySockFd6_, recvBuff->questionsBuff, MAX_REQUESTDATA_LEN, 0,
                                         reinterpret_cast<sockaddr *>(&(clientAddr->sin6)), &len);
    }
    if (recvBuff->questionLen <= 0) {
        NETNATIVE_LOGE("read errno %{public}d", errno);
        return;
    }
    if (!CheckDnsQuestion(recvBuff->questionsBuff, MAX_REQUESTDATA_LEN)) {
        NETNATIVE_LOGE("read buff is not dns question");
        return;
    }
    DnsParseBySocket(recvBuff, clientAddr);
}

bool DnsProxyListen::InitListenForIpv4(sockaddr_in &proxyAddr)
{
    if (proxySockFd_ < 0) {
        proxySockFd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (proxySockFd_ < 0) {
            NETNATIVE_LOGE("proxySockFd_ create socket failed %{public}d", errno);
            return false;
        }
    }
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    proxyAddr.sin_port = htons(DNS_PROXY_PORT);
    return true;
}

bool DnsProxyListen::InitListenForIpv6(sockaddr_in6 &proxyAddr6)
{
    if (proxySockFd6_ < 0) {
        proxySockFd6_ = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (proxySockFd6_ < 0) {
            NETNATIVE_LOGE("proxySockFd_ create socket failed %{public}d", errno);
            return false;
        }
    }
    proxyAddr6.sin6_family = AF_INET6;
    proxyAddr6.sin6_addr = in6addr_any;
    proxyAddr6.sin6_port = htons(DNS_PROXY_PORT);

    int on = 1;
    if (setsockopt(proxySockFd6_, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0) {
        NETNATIVE_LOGE("setsockopt failed");
        return false;
    }
    return true;
}

bool DnsProxyListen::InitForListening(epoll_event &proxyEvent, epoll_event &proxy6Event)
{
    sockaddr_in proxyAddr{};
    if (!InitListenForIpv4(proxyAddr)) {
        NETNATIVE_LOGE("InitListenForIpv4 failed");
        return false;
    }
    sockaddr_in6 proxyAddr6{};
    if (!InitListenForIpv6(proxyAddr6)) {
        NETNATIVE_LOGE("InitListenForIpv6 failed");
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        if (bind(proxySockFd_, (sockaddr *)&proxyAddr, sizeof(proxyAddr)) == -1) {
            NETNATIVE_LOGE("bind errno %{public}d: %{public}s", errno, strerror(errno));
            clearResource();
            return false;
        }
        if (bind(proxySockFd6_, (sockaddr *)&proxyAddr6, sizeof(proxyAddr6)) == -1) {
            NETNATIVE_LOGE("bind6 errno %{public}d: %{public}s", errno, strerror(errno));
            clearResource();
            return false;
        }
        epollFd_ = epoll_create1(0);
        if (epollFd_ < 0) {
            NETNATIVE_LOGE("epoll_create1 errno %{public}d: %{public}s", errno, strerror(errno));
            clearResource();
            return false;
        }
        proxyEvent.data.fd = proxySockFd_;
        proxyEvent.events = EPOLLIN;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, proxySockFd_, &proxyEvent) < 0) {
            NETNATIVE_LOGE("EPOLL_CTL_ADD proxy errno %{public}d: %{public}s", errno, strerror(errno));
            clearResource();
            return false;
        }
        proxy6Event.data.fd = proxySockFd6_;
        proxy6Event.events = EPOLLIN;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, proxySockFd6_, &proxy6Event) < 0) {
            NETNATIVE_LOGE("EPOLL_CTL_ADD proxy6 errno %{public}d: %{public}s", errno, strerror(errno));
            clearResource();
            return false;
        }
    }
    collectTime = std::chrono::system_clock::now() + std::chrono::milliseconds(EPOLL_TIMEOUT);
    return true;
}

void DnsProxyListen::CollectSocks()
{
    if (std::chrono::system_clock::now() >= collectTime) {
        NETNATIVE_LOG_D("collect socks");
        std::list<int32_t> sockTemp;
        for (const auto &[sock, request] : serverIdxOfSocket) {
            if (std::chrono::system_clock::now() >= request.endTime) {
                sockTemp.push_back(sock);
            }
        }
        for (const auto sock : sockTemp) {
            SendRequest2Server(sock);
        }
        collectTime = std::chrono::system_clock::now() + std::chrono::milliseconds(EPOLL_TIMEOUT);
    }
}

void DnsProxyListen::EpollTimeout()
{
    NETNATIVE_LOGE("epoll timeout, try next server.");
    std::list<int32_t> sockTemp;
    if (serverIdxOfSocket.size() > 0) {
        std::transform(serverIdxOfSocket.cbegin(), serverIdxOfSocket.cend(), std::back_inserter(sockTemp),
                       [](auto &iter) { return iter.first; });
        for (const auto sock : sockTemp) {
            SendRequest2Server(sock);
        }
    }
    collectTime = std::chrono::system_clock::now() + std::chrono::milliseconds(EPOLL_TIMEOUT);
}

bool DnsProxyListen::CheckDnsQuestion(char *recBuff, size_t recLen)
{
    if (recLen < DNS_HEAD_LENGTH) {
        return false;
    }
    uint8_t flagBuff;
    char *recFlagBuff = recBuff + FLAG_BUFF_OFFSET;
    if (memcpy_s(reinterpret_cast<char *>(&flagBuff), FLAG_BUFF_LEN, recFlagBuff, FLAG_BUFF_LEN) != 0) {
        return false;
    }
    int reqFlag = (flagBuff & RESPONSE_FLAG) / RESPONSE_FLAG_USED;
    if (reqFlag) {
        return false; // answer
    } else {
        return true; // question
    }
}

bool DnsProxyListen::CheckDnsResponse(char *recBuff, size_t recLen)
{
    if (recLen < FLAG_BUFF_LEN + FLAG_BUFF_OFFSET) {
        return false;
    }
    uint8_t flagBuff;
    char *recFlagBuff = recBuff + FLAG_BUFF_OFFSET;
    if (memcpy_s(reinterpret_cast<char *>(&flagBuff), FLAG_BUFF_LEN, recFlagBuff, FLAG_BUFF_LEN) != 0) {
        return false;
    }
    int reqFlag = (flagBuff & RESPONSE_FLAG) / RESPONSE_FLAG_USED;
    if (reqFlag) {
        return true; // answer
    } else {
        return false; // question
    }
}

void DnsProxyListen::OnListen()
{
    DnsProxyListen::proxyListenSwitch_ = true;
    NETNATIVE_LOGI("DnsProxy OnListen");
}

void DnsProxyListen::OffListen()
{
    if (proxySockFd_ > 0) {
        close(proxySockFd_);
        proxySockFd_ = -1;
    }
    if (proxySockFd6_ > 0) {
        close(proxySockFd6_);
        proxySockFd6_ = -1;
    }
    NETNATIVE_LOGI("DnsProxy OffListen");
}

void DnsProxyListen::SetParseNetId(uint16_t netId)
{
    DnsProxyListen::netId_ = netId;
    NETNATIVE_LOGI("SetParseNetId");
}

void DnsProxyListen::clearResource()
{
    if (proxySockFd_ > 0) {
        close(proxySockFd_);
        proxySockFd_ = -1;
    }
    if (proxySockFd6_ > 0) {
        close(proxySockFd6_);
        proxySockFd6_ = -1;
    }
    if (epollFd_ > 0) {
        close(epollFd_);
        epollFd_ = -1;
    }
    serverIdxOfSocket.clear();
}
} // namespace nmd
} // namespace OHOS
