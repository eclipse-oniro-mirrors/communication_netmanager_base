/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_BASE_MOCK_HTTP_SERVER_H
#define NETMANAGER_BASE_MOCK_HTTP_SERVER_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {
/*
 * Minimal threaded TCP http server for unit tests. Supports three behaviors:
 *  - RESPONSE: read the request, reply with the canned raw response.
 *  - HANG: accept the connection and never reply (for timeout / cancel tests).
 *  - CLOSE: accept the connection and close it immediately.
 */
class MockHttpServer {
public:
    enum class Behavior { RESPONSE, HANG, CLOSE };

    ~MockHttpServer()
    {
        Stop();
    }

    bool Start(Behavior behavior, const std::string &rawResponse = "")
    {
        behavior_ = behavior;
        rawResponse_ = rawResponse;
        listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listenFd_ < 0) {
            return false;
        }
        int reuse = 1;
        setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0; /* Pick a free port. */
        if (bind(listenFd_, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0) {
            close(listenFd_);
            listenFd_ = -1;
            return false;
        }
        if (listen(listenFd_, 8) != 0) {
            close(listenFd_);
            listenFd_ = -1;
            return false;
        }
        struct sockaddr_in bound = {};
        socklen_t len = sizeof(bound);
        if (getsockname(listenFd_, reinterpret_cast<struct sockaddr *>(&bound), &len) != 0) {
            close(listenFd_);
            listenFd_ = -1;
            return false;
        }
        port_ = ntohs(bound.sin_port);
        running_ = true;
        thread_ = std::thread(&MockHttpServer::Run, this);
        return true;
    }

    void Stop()
    {
        running_ = false;
        if (listenFd_ >= 0) {
            shutdown(listenFd_, SHUT_RDWR);
            close(listenFd_);
            listenFd_ = -1;
        }
        if (thread_.joinable()) {
            thread_.join();
        }
        /* Close fds kept open by the HANG behavior (the run thread has joined
         * and will not touch them anymore). */
        for (int fd : openFds_) {
            if (fd >= 0) {
                shutdown(fd, SHUT_RDWR);
                close(fd);
            }
        }
        openFds_.clear();
    }

    uint16_t Port() const
    {
        return port_;
    }

    std::string BaseUrl() const
    {
        return "http://127.0.0.1:" + std::to_string(port_);
    }

    std::string LastRequest()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return lastRequest_;
    }

    int ConnectionCount() const
    {
        return connectionCount_.load();
    }

private:
    void Run()
    {
        while (running_) {
            struct sockaddr_in peer = {};
            socklen_t len = sizeof(peer);
            int fd = accept(listenFd_, reinterpret_cast<struct sockaddr *>(&peer), &len);
            if (fd < 0) {
                break;
            }
            connectionCount_++;
            HandleConnection(fd);
        }
    }

    void HandleConnection(int fd)
    {
        if (behavior_ == Behavior::CLOSE) {
            close(fd);
            return;
        }
        if (behavior_ == Behavior::HANG) {
            /* Keep the connection open without replying. Closed in Stop(). */
            openFds_.push_back(fd);
            return;
        }
        std::string request;
        char buf[4096] = {0};
        while (true) {
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            if (n <= 0) {
                break;
            }
            request.append(buf, static_cast<size_t>(n));
            if (request.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }
        {
            std::lock_guard<std::mutex> lock(mtx_);
            lastRequest_ = request;
        }
        if (!rawResponse_.empty()) {
            size_t sent = 0;
            while (sent < rawResponse_.size()) {
                ssize_t n = send(fd, rawResponse_.c_str() + sent, rawResponse_.size() - sent, 0);
                if (n <= 0) {
                    break;
                }
                sent += static_cast<size_t>(n);
            }
        }
        close(fd);
    }

    Behavior behavior_ = Behavior::RESPONSE;
    std::string rawResponse_;
    int listenFd_ = -1;
    uint16_t port_ = 0;
    std::thread thread_;
    std::atomic<bool> running_{false};
    std::atomic<int> connectionCount_{0};
    std::mutex mtx_;
    std::string lastRequest_;
    std::vector<int> openFds_;
};
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_MOCK_HTTP_SERVER_H
