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
    /**
     * Construct a new NetMonitor to detection a network
     *
     * @param netId Detection network's id
     * @param sockFactory Use to create detection socket
     * @param async Async callback
     */
    NetMonitor(uint32_t netId, SocketFactory &sockFactory, NetConnAsync &async);

    /**
     * Destroy the NetMonitor
     *
     */
    virtual ~NetMonitor();

    /**
     * Start evaluation
     *
     */
    void Start();

    /**
     * Stop evaluation
     *
     */
    void Stop();

    /**
     * Determine NetMonitor is evaluating or not
     *
     * @return bool NetMonitor is evaluating or not
     */
    bool IsEvaluating() const;

    /**
     * Determine NetMonitor's current evaluation result is validated or not
     *
     * @return bool NetMonitor's current evaluation result is validated or not
     */
    bool IsValidated() const;

    /**
     * Get current evaluation result
     *
     * @return Current evaluation result
     */
    HttpProbeResult GetEvaluationResult() const;

private:
    void Reevaluate();

    void OnProbeResultChanged();

private:
    HttpProbeResult SendParallelHttpProbes(const Url &httpUrl, const Url &httpsUrl);

    HttpProbeResult SendDnsAndHttpProbes(const Url &url, HttpProbe::ProbeType probeType);

    HttpProbeResult SendHttpProbe(const std::string &url, HttpProbe::ProbeType probeType);

    void SendDnsProbe(const std::string &host);

private:
    uint32_t netId_;
    SocketFactory &sockFactory_;
    bool evaluating_{false};
    std::mutex mtx_;
    HttpProbeResult result_;
    NetConnAsync &async_;
    uint32_t reevaluateDelay_ {0};
    uint32_t reevaluateSteps_ {0};
    std::shared_ptr<Scheduler::Task> reevaluateTask_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_MANAGER_NET_MONITOR_H
