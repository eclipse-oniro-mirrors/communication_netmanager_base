/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <sys/stat.h>
#include <thread>

#include "dns_config_client.h"
#include "dns_param_cache.h"
#include "netnative_log_wrapper.h"
#include "netsys_client.h"
#include "securec.h"
#include "singleton.h"

#include "dns_resolv_listen.h"

#if DNS_CONFIG_DEBUG
#ifdef DNS_CONFIG_PRINT
#undef DNS_CONFIG_PRINT
#endif
#define DNS_CONFIG_PRINT(fmt, ...) NETNATIVE_LOGI("DNS" fmt, ##__VA_ARGS__)
#else
#define DNS_CONFIG_PRINT(fmt, ...)
#endif

namespace OHOS::nmd {
static constexpr const uint32_t MAX_LISTEN_NUM = 1024;

DnsResolvListen::DnsResolvListen() : serverSockFd_(-1)
{
    NETNATIVE_LOGE("DnsResolvListen start");
}

DnsResolvListen::~DnsResolvListen()
{
    NETNATIVE_LOGE("DnsResolvListen end");
    if (serverSockFd_ > 0) {
        close(serverSockFd_);
    }
}

void DnsResolvListen::ProcGetConfigCommand(int clientSockFd, uint32_t netId)
{
    DNS_CONFIG_PRINT("ProcGetConfigCommand");
    ResolvConfig sendData = {0};
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    uint16_t baseTimeoutMsec;
    uint8_t retryCount;

    auto status = DelayedSingleton<DnsParamCache>::GetInstance()->GetResolverConfig(
        static_cast<uint16_t>(netId), servers, domains, baseTimeoutMsec, retryCount);
    DNS_CONFIG_PRINT("GetResolverConfig status: %{public}d", status);
    if (status < 0) {
        sendData.retryCount = retryCount;
        sendData.timeoutMs = baseTimeoutMsec;
        if (strcpy_s(sendData.nameservers[0], sizeof(sendData.nameservers[0]), DEFAULT_SERVER) <= 0) {
            DNS_CONFIG_PRINT("ProcGetConfigCommand strcpy_s failed");
        }
    } else {
        sendData.retryCount = retryCount;
        sendData.timeoutMs = baseTimeoutMsec;
        for (int i = 0; i < std::min<size_t>(MAX_SERVER_NUM, servers.size()); i++) {
            if (memcpy_s(sendData.nameservers[i], sizeof(sendData.nameservers[i]), servers[i].c_str(),
                         servers[i].length()) < 0) {
                DNS_CONFIG_PRINT("mem copy failed");
                continue;
            }
            DNS_CONFIG_PRINT("i = %{public}d sendData.nameservers: %{public}s", i, sendData.nameservers[i]);
        }
    }
    if (!PollSendData(clientSockFd, reinterpret_cast<char *>(&sendData), sizeof(ResolvConfig))) {
        DNS_CONFIG_PRINT("send failed");
    }
    DNS_CONFIG_PRINT("ProcGetConfigCommand end");
}

void DnsResolvListen::ProcGetCacheCommand(int clientSockFd, uint32_t netId)
{
    DNS_CONFIG_PRINT("ProcGetCacheCommand");
    uint32_t nameLen = 0;
    if (!PollRecvData(clientSockFd, reinterpret_cast<char *>(&nameLen), sizeof(nameLen))) {
        DNS_CONFIG_PRINT("read errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    if (nameLen > MAX_HOST_NAME_LEN) {
        DNS_CONFIG_PRINT("MAX_HOST_NAME_LEN is %{public}u, but get %{public}u", MAX_HOST_NAME_LEN, nameLen);
        close(clientSockFd);
        return;
    }

    char name[MAX_HOST_NAME_LEN] = {0};
    if (!PollRecvData(clientSockFd, name, nameLen)) {
        DNS_CONFIG_PRINT("read errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    auto cacheRes = DelayedSingleton<DnsParamCache>::GetInstance()->GetDnsCache(netId, name);

    uint32_t resNum = std::min<uint32_t>(MAX_RESULTS, static_cast<uint32_t>(cacheRes.size()));
    if (!PollSendData(clientSockFd, reinterpret_cast<char *>(&resNum), sizeof(resNum))) {
        DNS_CONFIG_PRINT("send errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    if (resNum == 0) {
        return;
    }

    AddrInfo addrInfo[MAX_RESULTS] = {};
    std::copy(cacheRes.begin(), cacheRes.end(), addrInfo);

    if (!PollSendData(clientSockFd, reinterpret_cast<char *>(addrInfo), sizeof(AddrInfo) * resNum)) {
        DNS_CONFIG_PRINT("send errno %{public}d", errno);
        close(clientSockFd);
        return;
    }
    DNS_CONFIG_PRINT("ProcGetCacheCommand end");
}

void DnsResolvListen::ProcSetCacheCommand(int clientSockFd, uint32_t netId)
{
    DNS_CONFIG_PRINT("ProcSetCacheCommand");
    uint32_t nameLen = 0;
    if (!PollRecvData(clientSockFd, reinterpret_cast<char *>(&nameLen), sizeof(nameLen))) {
        DNS_CONFIG_PRINT("read errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    if (nameLen > MAX_HOST_NAME_LEN) {
        DNS_CONFIG_PRINT("MAX_HOST_NAME_LEN is %{public}u, but get %{public}u", MAX_HOST_NAME_LEN, nameLen);
        close(clientSockFd);
        return;
    }

    char name[MAX_HOST_NAME_LEN] = {0};
    if (!PollRecvData(clientSockFd, name, nameLen)) {
        DNS_CONFIG_PRINT("read errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    uint32_t resNum = 0;
    if (!PollRecvData(clientSockFd, reinterpret_cast<char *>(&resNum), sizeof(resNum))) {
        DNS_CONFIG_PRINT("read errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    resNum = std::min<uint32_t>(MAX_RESULTS, resNum);
    if (resNum == 0) {
        return;
    }

    AddrInfo addrInfo[MAX_RESULTS] = {};
    if (!PollRecvData(clientSockFd, reinterpret_cast<char *>(addrInfo), sizeof(AddrInfo) * resNum)) {
        DNS_CONFIG_PRINT("read errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    for (int i = 0; i < resNum; ++i) {
        DelayedSingleton<DnsParamCache>::GetInstance()->SetDnsCache(netId, name, addrInfo[i]);
    }

    DelayedSingleton<DnsParamCache>::GetInstance()->SetCacheDelayed(netId, name);
    DNS_CONFIG_PRINT("ProcSetCacheCommand end");
}

void DnsResolvListen::ProcCommand(int clientSockFd)
{
    char buff[sizeof(RequestInfo)] = {0};
    if (!PollRecvData(clientSockFd, buff, sizeof(buff))) {
        DNS_CONFIG_PRINT("read errno %{public}d", errno);
        close(clientSockFd);
        return;
    }

    auto info = reinterpret_cast<RequestInfo *>(buff);
    auto netId = info->netId;

    switch (info->command) {
        case GET_CONFIG:
            ProcGetConfigCommand(clientSockFd, netId);
            break;
        case GET_CACHE:
            ProcGetCacheCommand(clientSockFd, netId);
            break;
        case SET_CACHE:
            ProcSetCacheCommand(clientSockFd, netId);
            break;
        default:
            DNS_CONFIG_PRINT("invalid command %{public}u", info->command);
            break;
    }

    close(clientSockFd);
}

void DnsResolvListen::StartListen()
{
    NETNATIVE_LOGE("Enter StartListen");

    unlink(DNS_SOCKET_PATH);

    serverSockFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSockFd_ < 0) {
        NETNATIVE_LOGE("create socket failed %{public}d", errno);
        return;
    }

    sockaddr_un server_addr = {0};
    server_addr.sun_family = AF_UNIX;

    if (strcpy_s(server_addr.sun_path, sizeof(server_addr.sun_path), DNS_SOCKET_PATH) < 0) {
        NETNATIVE_LOGE("str copy failed ");
        close(serverSockFd_);
        return;
    }

    int addrLen = offsetof(sockaddr_un, sun_path) + strlen(server_addr.sun_path) + 1;
    if (bind(serverSockFd_, (sockaddr *)&server_addr, addrLen) < 0) {
        NETNATIVE_LOGE("bind errno %{public}d", errno);
        close(serverSockFd_);
        return;
    }

    if (chmod(DNS_SOCKET_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0) {
        NETNATIVE_LOGE("chmod errno %{public}d", errno);
        close(serverSockFd_);
        return;
    }

    // listen
    if (listen(serverSockFd_, MAX_LISTEN_NUM) < 0) {
        NETNATIVE_LOGE("listen errno %{public}d", errno);
        close(serverSockFd_);
        return;
    }

    NETNATIVE_LOGE("begin listen");

    while (true) {
        sockaddr_un clientAddr = {0};
        socklen_t len = sizeof(clientAddr);

        int clientSockFd = accept(serverSockFd_, (sockaddr *)&clientAddr, &len);
        if (clientSockFd < 0) {
            DNS_CONFIG_PRINT("accept errno %{public}d", errno);
            continue;
        }
        if (!MakeNonBlock(clientSockFd)) {
            DNS_CONFIG_PRINT("MakeNonBlock errno %{public}d", errno);
            close(clientSockFd);
            continue;
        }

        std::thread(DnsResolvListen::ProcCommand, clientSockFd).detach();
    }
}
} // namespace OHOS::nmd
