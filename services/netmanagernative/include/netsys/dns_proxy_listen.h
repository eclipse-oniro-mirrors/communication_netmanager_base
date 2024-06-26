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

#ifndef INCLUDE_DNS_PROXY_LISTEN_H
#define INCLUDE_DNS_PROXY_LISTEN_H

#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <vector>
#include <map>
#include <sys/eventfd.h>

#include "dns_proxy_request_socket.h"

namespace OHOS {
namespace nmd {
class DnsProxyListen {
public:
    DnsProxyListen();
    ~DnsProxyListen();

    /**
     * Begin dns proxy listen
     *
     */
    void OnListen();

    /**
     * Close dns proxy listen
     */
    void OffListen();

    /**
     * Dns proxy listen obj
     *
     */
    void StartListen();

    /**
     * Set the Parse Net Id objectse
     *
     * @param netId network ID
     */
    void SetParseNetId(uint16_t netId);

private:
    void DnsParseBySocket(std::unique_ptr<RecvBuff> &recvBuff, std::unique_ptr<AlignedSockAddr> &clientSock);
    static void DnsSendRecvParseData(int32_t clientSocket, char *requestData, int32_t resLen,
                                     AlignedSockAddr &proxyAddr);
    static bool CheckDnsResponse(char *recBuff, size_t recLen);
    static bool CheckDnsQuestion(char *recBuff, size_t recLen);
    void SendDnsBack2Client(int32_t socketFd);
    void clearResource();
    void SendRequest2Server(int32_t socketFd);
    bool GetDnsProxyServers(std::vector<std::string> &servers, size_t serverIdx);
    int32_t proxySockFd_;
    int32_t proxySockFd6_;
    int32_t epollFd_ = -1;
    static uint16_t netId_;
    static std::atomic_bool proxyListenSwitch_;
    static std::mutex listenerMutex_;
    std::map<int32_t, DnsProxyRequestSocket> serverIdxOfSocket;
    std::chrono::system_clock::time_point collectTime;
    void EpollTimeout();
    void CollectSocks();
    bool InitListenForIpv4(sockaddr_in &proxyAddr);
    bool InitListenForIpv6(sockaddr_in6 &proxyAddr6);
    bool InitForListening(epoll_event &proxyEvent, epoll_event &proxy6Event);
    void GetRequestAndTransmit(int32_t family);
};
} // namespace nmd
} // namespace OHOS
#endif // INCLUDE_DNS_PROXY_LISTEN_H
