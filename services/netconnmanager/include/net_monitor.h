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

#include "system_ability.h"
#include "system_ability_definition.h"
#include "i_net_monitor_callback.h"
#include "net_conn_types.h"
#include "net_datashare_utils.h"
#include "net_http_probe.h"
#include "net_link_info.h"
#include "probe_thread.h"

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
        const std::weak_ptr<INetMonitorCallback> &callback, bool isScreenOn);

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

    /**
     * Set screen state
     *
     */
    void SetScreenState(bool isScreenOn);

private:
    void LoadGlobalHttpProxy();
    void ProcessDetection(NetHttpProbeResult& probeResult, NetDetectionStatus& result);
    NetHttpProbeResult SendProbe();
    NetHttpProbeResult ProcessThreadDetectResult(std::shared_ptr<ProbeThread>& httpProbeThread,
        std::shared_ptr<ProbeThread>& httpsProbeThread, std::shared_ptr<ProbeThread>& backHttpThread,
        std::shared_ptr<ProbeThread>& backHttpsThread);
    void StartProbe(std::shared_ptr<ProbeThread>& httpProbeThread, std::shared_ptr<ProbeThread>& httpsProbeThread,
        std::shared_ptr<ProbeThread>& backHttpThread, std::shared_ptr<ProbeThread>& backHttpsThread, bool needProxy);
    NetHttpProbeResult GetThreadDetectResult(std::shared_ptr<ProbeThread>& probeThread, ProbeType probeType);
    void GetHttpProbeUrlFromConfig();
    void GetDetectUrlConfig();
    bool CheckIfSettingsDataReady();

private:
    uint32_t netId_ = 0;
    NetBearType netBearType_;
    NetLinkInfo netLinkInfo_;
    std::atomic<bool> isDetecting_ = false;
    std::mutex detectionMtx_;
    std::mutex probeMtx_;
    std::condition_variable detectionCond_;
    std::mutex primaryDetectMutex_;
    std::condition_variable primaryDetectionCond_;
    uint32_t detectionDelay_ = 0;
    std::weak_ptr<INetMonitorCallback> netMonitorCallback_;
    bool needDetectionWithoutProxy_ = true;
    HttpProxy globalHttpProxy_;
    std::string httpUrl_;
    std::string httpsUrl_;
    std::string fallbackHttpUrl_;
    std::string fallbackHttpsUrl_;
    std::mutex proxyMtx_;
    bool isNeedSuffix_ = false;
    bool isDataShareReady_ = false;
    bool isScreenOn_ = true;
    std::string prevRedirectUrl_;
    NetDetectionStatus prevProbeStatus_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_MONITOR_H
