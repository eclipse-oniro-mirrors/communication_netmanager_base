/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <future>
#include <list>
#include <memory>
#include <netdb.h>
#include <regex>
#include <securec.h>
#include <sys/socket.h>
#include <thread>
#include <pthread.h>
#include <unistd.h>

#include "net_monitor.h"
#include "dns_config_client.h"
#include "event_report.h"
#include "fwmark_client.h"
#include "netmanager_base_common_utils.h"
#include "netsys_controller.h"
#include "net_http_proxy_tracker.h"
#include "net_mgr_log_wrapper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t INIT_DETECTION_DELAY_MS = 1 * 1000;
constexpr int32_t MAX_FAILED_DETECTION_DELAY_MS = 10 * 60 * 1000;
constexpr int32_t CAPTIVE_PORTAL_DETECTION_DELAY_MS = 30 * 1000;
constexpr int32_t DOUBLE = 2;
constexpr const char NEW_LINE_STR = '\n';
constexpr const char *URL_CFG_FILE = "/system/etc/netdetectionurl.conf";
constexpr const char *HTTP_URL_HEADER = "HttpProbeUrl:";
constexpr const char *HTTPS_URL_HEADER = "HttpsProbeUrl:";
constexpr const char *NET_HTTP_PROBE_URL = "http://connectivitycheck.platform.hicloud.com/generate_204";
constexpr const char *NET_HTTPS_PROBE_URL = "https://connectivitycheck.platform.hicloud.com/generate_204";
} // namespace
static void NetDetectThread(const std::shared_ptr<NetMonitor> &netMonitor)
{
    if (netMonitor == nullptr) {
        NETMGR_LOG_E("netMonitor is nullptr");
        return;
    }
    while (netMonitor->IsDetecting()) {
        netMonitor->Detection();
    }
}

NetMonitor::NetMonitor(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo,
                       const std::weak_ptr<INetMonitorCallback> &callback)
    : netId_(netId), netMonitorCallback_(callback)
{
    httpProbe_ = std::make_unique<NetHttpProbe>(netId, bearType, netLinkInfo);
    LoadGlobalHttpProxy();
}

void NetMonitor::Start()
{
    NETMGR_LOG_D("Start net[%{public}d] monitor in", netId_);
    if (isDetecting_) {
        NETMGR_LOG_W("Net[%{public}d] monitor is detecting, no need to start", netId_);
        return;
    }
    isDetecting_ = true;
    std::shared_ptr<NetMonitor> netMonitor = shared_from_this();
    std::thread t([netMonitor] { return NetDetectThread(netMonitor); });
    std::string threadName = "netDetect";
    pthread_setname_np(t.native_handle(), threadName.c_str());
    t.detach();
}

void NetMonitor::Stop()
{
    NETMGR_LOG_I("Stop net[%{public}d] monitor in", netId_);
    isDetecting_ = false;
    detectionCond_.notify_all();
    NETMGR_LOG_D("Stop net[%{public}d] monitor out", netId_);
}

bool NetMonitor::IsDetecting()
{
    return isDetecting_.load();
}

void NetMonitor::Detection()
{
    NetHttpProbeResult probeResult = SendHttpProbe(PROBE_HTTP_HTTPS);
    if (isDetecting_) {
        NetDetectionStatus result = UNKNOWN_STATE;
        if (probeResult.IsSuccessful()) {
            NETMGR_LOG_I("Net[%{public}d] probe success", netId_);
            isDetecting_ = false;
            detectionSteps_ = 0;
            result = VERIFICATION_STATE;
        } else if (probeResult.IsNeedPortal()) {
            NETMGR_LOG_W("Net[%{public}d] need portal", netId_);
            detectionDelay_ = CAPTIVE_PORTAL_DETECTION_DELAY_MS;
            result = CAPTIVE_PORTAL_STATE;
        } else {
            NETMGR_LOG_E("Net[%{public}d] probe failed", netId_);
            detectionDelay_ *= DOUBLE;
            if (detectionDelay_ == 0) {
                detectionDelay_ = INIT_DETECTION_DELAY_MS;
            } else if (detectionDelay_ >= MAX_FAILED_DETECTION_DELAY_MS) {
                detectionDelay_ = MAX_FAILED_DETECTION_DELAY_MS;
            }
            NETMGR_LOG_I("Net probe failed detectionDelay time [%{public}d]", detectionDelay_);
            detectionSteps_++;
            result = INVALID_DETECTION_STATE;
        }
        auto monitorCallback = netMonitorCallback_.lock();
        if (monitorCallback) {
            monitorCallback->OnHandleNetMonitorResult(result, probeResult.GetRedirectUrl());
        }
        struct EventInfo eventInfo = {.monitorStatus = static_cast<int32_t>(result)};
        EventReport::SendMonitorBehaviorEvent(eventInfo);
        if (isDetecting_) {
            std::unique_lock<std::mutex> locker(detectionMtx_);
            detectionCond_.wait_for(locker, std::chrono::milliseconds(detectionDelay_));
        }
    }
}

NetHttpProbeResult NetMonitor::SendHttpProbe(ProbeType probeType)
{
    std::lock_guard<std::mutex> locker(probeMtx_);
    std::string httpProbeUrl;
    std::string httpsProbeUrl;
    GetHttpProbeUrlFromConfig(httpProbeUrl, httpsProbeUrl);

    if (httpProbe_ == nullptr) {
        NETMGR_LOG_E("Net:[%{public}d] httpProbe_ is nullptr", netId_);
        return NetHttpProbeResult();
    }

    if (httpProbe_->SendProbe(probeType, httpProbeUrl, httpsProbeUrl) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Net:[%{public}d] send probe failed.", netId_);
        return NetHttpProbeResult();
    }

    if (httpProbe_->GetHttpProbeResult().IsNeedPortal()) {
        return httpProbe_->GetHttpProbeResult();
    }

#ifdef NEED_REPORT_PARTIAL_CONNECTION
    if (httpProbe_->HasProbeType(probeType, ProbeType::PROBE_HTTP) &&
        httpProbe_->HasProbeType(probeType, ProbeType::PROBE_HTTPS)) {
        if (httpProbe_->GetHttpProbeResult().IsSuccessful() && httpProbe_->GetHttpsProbeResult().IsFailed) {
            // return probe result: PARTIAL;
        }
    }
#endif

    if (httpProbe_->HasProbeType(probeType, ProbeType::PROBE_HTTPS)) {
        return httpProbe_->GetHttpsProbeResult();
    }

    return httpProbe_->GetHttpProbeResult();
}

void NetMonitor::GetHttpProbeUrlFromConfig(std::string &httpUrl, std::string &httpsUrl)
{
    if (!std::filesystem::exists(URL_CFG_FILE)) {
        NETMGR_LOG_E("File not exist (%{public}s)", URL_CFG_FILE);
        return;
    }

    std::ifstream file(URL_CFG_FILE);
    if (!file.is_open()) {
        NETMGR_LOG_E("Open file failed (%{public}s)", strerror(errno));
        return;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();
    auto pos = content.find(HTTP_URL_HEADER);
    if (pos != std::string::npos) {
        pos += strlen(HTTP_URL_HEADER);
        httpUrl = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
    }
    httpUrl = httpUrl.empty() ? NET_HTTP_PROBE_URL : httpUrl;

    pos = content.find(HTTPS_URL_HEADER);
    if (pos != std::string::npos) {
        pos += strlen(HTTPS_URL_HEADER);
        httpsUrl = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
    }
    httpsUrl = httpsUrl.empty() ? NET_HTTPS_PROBE_URL : httpsUrl;
    NETMGR_LOG_D("Get net detection http url:[%{public}s], https url:[%{public}s]", httpUrl.c_str(), httpsUrl.c_str());
}

void NetMonitor::LoadGlobalHttpProxy()
{
    HttpProxy globalHttpProxy;
    NetHttpProxyTracker httpProxyTracker;
    httpProxyTracker.ReadFromSettingsData(globalHttpProxy);
    UpdateGlobalHttpProxy(globalHttpProxy);
}

void NetMonitor::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    if (httpProbe_) {
        httpProbe_->UpdateGlobalHttpProxy(httpProxy);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
