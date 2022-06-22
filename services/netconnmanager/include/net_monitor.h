/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_NET_MONITOR_H
#define NET_CONN_NET_MONITOR_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include "url.h"
#include "http_probe.h"
#include "socket_factory.h"
#include "net_conn_async.h"

namespace OHOS {
namespace NetManagerStandard {
class NetMonitor : public virtual RefBase {
public:
    NetMonitor(uint32_t netId, SocketFactory& sockFactory, NetConnAsync& async);

    virtual ~NetMonitor();
    
    void Start();

    void Stop();

    void Restart();

    bool IsEvaluating() const;

    bool IsValidated() const;

    HttpProbeResult GetEvaluationResult() const;

private:
    void OnEvaluating();

    void OnProbeResultChanged();

private:
    HttpProbeResult SendParallelHttpProbes(const Url& httpUrl, const Url& httpsUrl);

    HttpProbeResult SendDnsAndHttpProbes(const Url& url, HttpProbe::ProbeType probeType);

    HttpProbeResult SendHttpProbe(const std::string& url, HttpProbe::ProbeType probeType);
    
    void SendDnsProbe(const std::string& host);

private:
    uint32_t netId_;
    SocketFactory& sockFactory_;
    std::thread evaluationThread_;
    bool evaluating_ {false};
    std::mutex evaluationTimerMtx_;
    std::condition_variable evaluationTimerCond_;
    HttpProbeResult result_;
    NetConnAsync& async_;
    uint32_t reevaluateDelay_ {0};
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif // NET_CONN_MANAGER_NET_MONITOR_H
