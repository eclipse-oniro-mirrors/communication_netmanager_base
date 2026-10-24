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

#ifndef NET_PAC_PROXY_CONTROLLER_H
#define NET_PAC_PROXY_CONTROLLER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "ffrt.h"
#include "http_proxy.h"
#include "net_pac_local_proxy_server.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnService;

class NetPacProxyController {
public:
    explicit NetPacProxyController(NetConnService &service);

    void Refresh();
    void FillLocalProxy(int32_t netId, HttpProxy &httpProxy);
    bool HasWifiPac();
    void SendEffectiveProxyBroadcast();

    class WifiPacPause {
    public:
        explicit WifiPacPause(NetPacProxyController &controller);
        ~WifiPacPause();
        WifiPacPause(const WifiPacPause &) = delete;
        WifiPacPause &operator=(const WifiPacPause &) = delete;

    private:
        NetPacProxyController &controller_;
        std::unique_lock<ffrt::mutex> lock_;
    };
    WifiPacPause PauseWifiPac();

private:
    enum class PacSource { NONE, GLOBAL, WIFI };
    struct PacDecision {
        PacSource source = PacSource::NONE;
        int32_t netId = 0;
        std::string url;
        bool operator==(const PacDecision &other) const;
    };
    struct Runtime {
        PacDecision decision;
        int32_t port = 0;
    };

    PacDecision Decide();
    bool IsGlobalPacRunning();
    void ApplyWifiPac();
    bool StartServer(const PacDecision &decision);
    bool StopServer();

    NetConnService &service_;
    std::once_flag queueOnce_;
    std::shared_ptr<ffrt::queue> queue_;
    ffrt::task_handle taskHandle_ = nullptr;
    ffrt::mutex operationMutex_;
    std::mutex stateMutex_;
    Runtime runtime_;
    std::shared_ptr<ProxyServer> server_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_PAC_PROXY_CONTROLLER_H
