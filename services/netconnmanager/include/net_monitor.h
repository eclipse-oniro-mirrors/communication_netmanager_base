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

#ifndef NET_MONITOR_H
#define NET_MONITOR_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "refbase.h"

#include "i_net_monitor_callback.h"
#include "net_conn_types.h"
#include "net_http_probe.h"
#include "net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
class NetMonitor : public virtual RefBase, public std::enable_shared_from_this<NetMonitor> {
public:
    /**
     * Construct a new NetMonitor to detection a network
     *
     * @param netId Detection network's id
     * @param bearType bearType network type
     * @param netLinkInfo Network link information
     * @param callback Network monitor callback weak reference
     */
    NetMonitor(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo,
               const std::weak_ptr<INetMonitorCallback> &callback);

    /**
     * Destroy the NetMonitor
     *
     */
    virtual ~NetMonitor() = default;

    /**
     * Start detection
     *
     */
    void Start();

    /**
     * Stop detecting
     *
     */
    void Stop();

    /**
     * Set network socket parameter
     *
     * @return Socket parameter setting result
     */
    int32_t SetSocketParameter(int32_t sockFd);

    /**
     * Is network monitor detecting
     *
     * @return Status value of whether the network is detecting
     */
    bool IsDetecting();

    /**
     * Network monitor detection
     *
     */
    void Detection();

    /**
     * Update global http proxy
     *
     */
    void UpdateGlobalHttpProxy(const HttpProxy &httpProxy);

private:
    NetHttpProbeResult SendHttpProbe(ProbeType probeType);
    void GetHttpProbeUrlFromConfig(std::string &httpUrl, std::string &httpsUrl);
    void LoadGlobalHttpProxy();

private:
    uint32_t netId_ = 0;
    std::atomic<bool> isDetecting_ = false;
    int32_t detectionSteps_ = 0;
    std::mutex detectionMtx_;
    std::mutex probeMtx_;
    std::condition_variable detectionCond_;
    uint32_t detectionDelay_ = 0;
    std::weak_ptr<INetMonitorCallback> netMonitorCallback_;
    std::unique_ptr<NetHttpProbe> httpProbe_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_MONITOR_H
