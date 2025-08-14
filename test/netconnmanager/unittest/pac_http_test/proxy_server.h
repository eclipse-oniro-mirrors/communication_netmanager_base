/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include "map"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace NetManagerStandard {

#define LOCAL_PROXY_9000 "local_proxy_9000"
#define LOCAL_PROXY_9001 "local_proxy_9001"
#define ALL_DIRECT "local_proxy_all"

struct Stats {
    std::atomic<uint64_t> totalConnections;
    std::atomic<uint64_t> activeConnections;
    std::atomic<uint64_t> httpRequests;
    std::atomic<uint64_t> httpsRequests;
    std::atomic<uint64_t> bytesReceived;
    std::atomic<uint64_t> bytesSent;
    std::chrono::time_point<std::chrono::steady_clock> startTime;

    Stats()
        : totalConnections(0),
          activeConnections(0),
          httpRequests(0), httpsRequests(0), bytesReceived(0), bytesSent(0)
    {
    }
};
class ProxyServer {
public:
    explicit ProxyServer(int port = 8080, int numThreads = 0);

    ~ProxyServer();

    void SetFindPacProxyFunction(std::function<std::string(std::string, std::string)> pac);

    bool Start();

    void Stop();

    bool IsRunning() const;

    std::shared_ptr<Stats> GetStats();

    void ResetStats();

    double GetThroughput() const;

    int FindAvailablePort(int startPort = 1024, int endPort = 65535);

    static std::string proxServerTargetUrl;
    static int proxServerPort;
    static std::map<std::string, std::string> pacScripts;
private:
    bool IsPortAvailable(int port);
    void ForwardResponseToClient(int clientSocket, int serverSocket);
    void SendErrorResponse(int clientSocket, const char *response);
    bool SetSocketTimeout(int socket);
    bool ReadRequestHeader(int clientSocket, std::string &requestHeader);
    void CleanupConnection(int clientSocket);
    void HandleConnectRequest(int clientSocket, const std::string &requestHeader, const std::string &url);
    void HandleHttpRequest(int clientSocket, std::string &requestHeader, const std::string &url);
    bool ParsePacResult(const std::string &pacResult, std::string &proxyType, std::string &proxyHost, int &proxyPort);
    int EstablishServerConnection(const std::string &url, const std::string &host, int port,
                                  const std::string &requestType, std::string &requestHeader);
    bool ParseProxyInfo(std::string url, std::string host, std::string &proxyType, std::string &proxyHost,
                        int &proxyPort);

    struct ClientTask {
        int clientSocket;
        struct sockaddr_in clientAddr;

        ClientTask(int socket, const struct sockaddr_in &addr) : clientSocket(socket), clientAddr(addr) {}
    };

    void HandleClient(int clientSocket);

    std::string GetRequestUrl(const std::string &header);

    std::string GetRequestMethod(const std::string &header);

    bool ParseConnectRequest(const std::string &header, std::string &host, int &port);

    bool ParseHttpRequest(const std::string &header, std::string &host, int &port);

    void TunnelData(int client, int server);

    int ConnectToServer(const std::string &host, int port);

    int ConnectViaUpstreamProxy(const std::string &targetHost, int targetPort, const std::string &originalRequest,
                                std::string proxyHost, int proxyPort);

    int ConnectViaUpstreamProxyHttps(const std::string &targetHost, int targetPort, std::string proxyHost,
                                     int proxyPort);

    void AcceptLoop();

    void WorkerThread();

    void AddTask(const ClientTask &task);

    std::string ReceiveResponseHeader(int socket);

    int port_;
    int serverSocket_;
    int numThreads_;
    std::atomic<bool> running_;
    std::thread acceptThread_;
    std::vector<std::thread> workers_;

    std::function<std::string(std::string, std::string)> pacFunction_;
    std::queue<ClientTask> taskQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;

    mutable std::mutex statsMutex_;
    std::shared_ptr<Stats> stats_;

    static const int bufferSize = 8192;
    static const int backlog = 128;
    static const int maxHeaderSize = 8192;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // PROXY_SERVER_H
