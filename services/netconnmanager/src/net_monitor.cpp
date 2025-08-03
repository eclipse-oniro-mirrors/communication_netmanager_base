/*
 * Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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
#include <string>
#include <chrono>

#include "net_monitor.h"
#include "dns_config_client.h"
#include "event_report.h"
#include "fwmark_client.h"
#include "netmanager_base_common_utils.h"
#include "netsys_controller.h"
#include "net_http_proxy_tracker.h"
#include "net_mgr_log_wrapper.h"
#include "net_manager_constants.h"
#include "tiny_count_down_latch.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t INIT_DETECTION_DELAY_MS = 1 * 1000;
constexpr int32_t MAX_FAILED_DETECTION_DELAY_MS = 10 * 60 * 1000;
constexpr int32_t PRIMARY_DETECTION_RESULT_WAIT_MS = 3 * 1000;
constexpr int32_t ALL_DETECTION_RESULT_WAIT_MS = 10 * 1000;
constexpr int32_t CAPTIVE_PORTAL_DETECTION_DELAY_MS = 15 * 1000;
constexpr int32_t SCREENOFF_PORTAL_DETECTION_DELAY_MS = 5 * 60 * 1000;
constexpr int32_t SCREENOFF_DETECTION_INTERVEL_MS = 2 * 60 * 1000;
constexpr int32_t DOUBLE = 2;
constexpr int32_t SIM_PORTAL_CODE = 302;
constexpr int32_t ONE_URL_DETECT_NUM = 4;
constexpr int32_t ALL_DETECT_THREAD_NUM = 8;
constexpr const char NEW_LINE_STR = '\n';
constexpr const char* URL_CFG_FILE = "/system/etc/netdetectionurl.conf";
constexpr const char* DETECT_CFG_FILE = "/system/etc/detectionconfig.conf";
constexpr const char *SETTINGS_DATASHARE_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
const std::string HTTP_URL_HEADER = "HttpProbeUrl:";
const std::string HTTPS_URL_HEADER = "HttpsProbeUrl:";
const std::string FALLBACK_HTTP_URL_HEADER = "FallbackHttpProbeUrl:";
const std::string FALLBACK_HTTPS_URL_HEADER = "FallbackHttpsProbeUrl:";
const std::string ADD_RANDOM_CFG_PREFIX = "AddSuffix:";
const std::string ADD_RANDOM_CFG_VALUE = "true";
} // namespace
static void NetDetectThread(const std::shared_ptr<NetMonitor> &netMonitor)
{
    if (netMonitor == nullptr) {
        NETMGR_LOG_E("netMonitor is nullptr");
        return;
    }
    netMonitor->DetectionDelayWhenScreenOff();
    while (netMonitor->IsDetecting()) {
        netMonitor->Detection();
    }
}

NetMonitor::NetMonitor(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo,
    const std::weak_ptr<INetMonitorCallback> &callback, NetMonitorInfo &netMonitorInfo)
    : netId_(netId), netLinkInfo_(netLinkInfo), netMonitorCallback_(callback), isScreenOn_(netMonitorInfo.isScreenOn)
{
    netBearType_ = bearType;
    lastDetectTimestamp_ = netMonitorInfo.lastDetectTime;
    LoadGlobalHttpProxy();
    GetDetectUrlConfig();
    GetHttpProbeUrlFromConfig();
}

void NetMonitor::Start()
{
    NETMGR_LOG_D("Start net[%{public}d] monitor in", netId_);
    if (isDetecting_) {
        if (isScreenOn_) {
            NETMGR_LOG_W("Net[%{public}d] monitor is detecting, notify", netId_);
            detectionDelay_ = 0;
            detectionCond_.notify_all();
        }
        return;
    }
    isDetecting_ = true;
    std::thread t([sp = shared_from_this()]() { NetDetectThread(sp); });
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

void NetMonitor::ProcessDetection(NetHttpProbeResult& probeResult, NetDetectionStatus& result)
{
    if (probeResult.IsSuccessful()) {
        NETMGR_LOG_I("Net[%{public}d] probe success", netId_);
        isDetecting_ = false;
        needDetectionWithoutProxy_ = true;
        result = VERIFICATION_STATE;
        detectionDelay_ = 0;
    } else if (probeResult.GetCode() == SIM_PORTAL_CODE && netBearType_ == BEARER_CELLULAR) {
        NETMGR_LOG_E("Net[%{public}d] probe failed with 302 response on Cell", netId_);
        detectionDelay_ = MAX_FAILED_DETECTION_DELAY_MS;
        result = CAPTIVE_PORTAL_STATE;
    } else if (probeResult.IsNeedPortal()) {
        NETMGR_LOG_W("Net[%{public}d] need portal", netId_);
        if (!isScreenOn_ && netBearType_ == BEARER_WIFI) {
            detectionDelay_ = SCREENOFF_PORTAL_DETECTION_DELAY_MS;
        } else {
            detectionDelay_ = CAPTIVE_PORTAL_DETECTION_DELAY_MS;
        }
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
        result = INVALID_DETECTION_STATE;
    }
    auto monitorCallback = netMonitorCallback_.lock();
    if (monitorCallback == nullptr) {
        Stop();
        return;
    }
    monitorCallback->OnHandleNetMonitorResult(result, probeResult.GetRedirectUrl());

    struct EventInfo eventInfo = {.monitorStatus = static_cast<int32_t>(result)};
    EventReport::SendMonitorBehaviorEvent(eventInfo);
    if (isDetecting_) {
        std::unique_lock<std::mutex> locker(detectionMtx_);
        detectionCond_.wait_for(locker, std::chrono::milliseconds(detectionDelay_));
    }
}

void NetMonitor::Detection()
{
    lastDetectTimestamp_ = CommonUtils::GetCurrentMilliSeconds();
    NetHttpProbeResult probeResult = SendProbe();
    bool isTmpDetecting = IsDetecting();
    NETMGR_LOG_I("Detection isTmpDetecting[%{public}d]", isTmpDetecting);
    if (IsDetecting()) {
        NetDetectionStatus result = UNKNOWN_STATE;
        ProcessDetection(probeResult, result);
    }
}

NetHttpProbeResult NetMonitor::SendProbe()
{
    NETMGR_LOG_D("start net detection");
    std::lock_guard<std::mutex> monitorLocker(probeMtx_);
    std::shared_ptr<TinyCountDownLatch> latch = std::make_shared<TinyCountDownLatch>(ONE_URL_DETECT_NUM);
    std::shared_ptr<TinyCountDownLatch> latchAll = std::make_shared<TinyCountDownLatch>(ALL_DETECT_THREAD_NUM);
    std::shared_ptr<ProbeThread> httpProxyThread = nullptr;
    std::shared_ptr<ProbeThread> httpsProxyThread = nullptr;
    std::shared_ptr<ProbeThread> backHttpProxyThread = nullptr;
    std::shared_ptr<ProbeThread> backHttpsProxyThread = nullptr;
    CreateProbeThread(httpProxyThread, httpsProxyThread, latch, latchAll, true);
    CreateProbeThread(backHttpProxyThread, backHttpsProxyThread, latch, latchAll, false);
    StartProbe(httpProxyThread, httpsProxyThread, backHttpProxyThread, backHttpsProxyThread, true);
    latch->Await(std::chrono::milliseconds(PRIMARY_DETECTION_RESULT_WAIT_MS));
    NetHttpProbeResult proxyResult = ProcessThreadDetectResult(httpProxyThread, httpsProxyThread, backHttpProxyThread,
        backHttpsProxyThread);
    if (proxyResult.IsNeedPortal() || proxyResult.IsSuccessful()) {
        return proxyResult;
    }
    NETMGR_LOG_I("backup url detection");
    std::shared_ptr<ProbeThread> httpNoProxyThread = nullptr;
    std::shared_ptr<ProbeThread> httpsNoProxyThread = nullptr;
    std::shared_ptr<ProbeThread> backHttpNoProxyThread = nullptr;
    std::shared_ptr<ProbeThread> backHttpsNoProxyThread = nullptr;
    CreateProbeThread(httpNoProxyThread, httpsNoProxyThread, latch, latchAll, true);
    CreateProbeThread(backHttpNoProxyThread, backHttpsNoProxyThread, latch, latchAll, false);
    StartProbe(httpNoProxyThread, httpsNoProxyThread, backHttpNoProxyThread, backHttpsNoProxyThread, false);
    latchAll->Await(std::chrono::milliseconds(ALL_DETECTION_RESULT_WAIT_MS));
    proxyResult = ProcessThreadDetectResult(httpProxyThread, httpsProxyThread, backHttpProxyThread,
        backHttpsProxyThread);
    NetHttpProbeResult noProxyResult = ProcessThreadDetectResult(httpNoProxyThread, httpsNoProxyThread,
        backHttpNoProxyThread, backHttpsNoProxyThread);
    if (proxyResult.IsNeedPortal()) {
        return proxyResult;
    } else if (noProxyResult.IsNeedPortal()) {
        return noProxyResult;
    } else if (proxyResult.IsSuccessful()) {
        return proxyResult;
    } else if (noProxyResult.IsSuccessful()) {
        return noProxyResult;
    } else {
        return proxyResult;
    }
}

void NetMonitor::CreateProbeThread(std::shared_ptr<ProbeThread>& httpThread, std::shared_ptr<ProbeThread>& httpsThread,
    std::shared_ptr<TinyCountDownLatch>& latch, std::shared_ptr<TinyCountDownLatch>& latchAll, bool isPrimProbe)
{
    if (isPrimProbe) {
        if (netBearType_ == BEARER_CELLULAR) {
            NETMGR_LOG_I("create primary probeThread for cellular");
            httpsThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTPS, httpUrl_, httpsUrl_);
            httpThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTP, httpUrl_, httpsUrl_);
        } else {
            NETMGR_LOG_D("create primary probeThread for others");
            httpThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTP, httpUrl_, httpsUrl_);
            httpsThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTPS, httpUrl_, httpsUrl_);
        }
    } else {
        if (netBearType_ == BEARER_CELLULAR) {
            NETMGR_LOG_I("create fallback probeThread for cellular");
            httpsThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTPS_FALLBACK, fallbackHttpUrl_,
                fallbackHttpsUrl_);
            httpThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTP_FALLBACK, fallbackHttpUrl_,
                fallbackHttpsUrl_);
        } else {
            NETMGR_LOG_D("create fallback probeThread for others");
            httpThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTP_FALLBACK, fallbackHttpUrl_,
                fallbackHttpsUrl_);
            httpsThread = std::make_shared<ProbeThread>(
                netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTPS_FALLBACK, fallbackHttpUrl_,
                fallbackHttpsUrl_);
        }
    }
}

void NetMonitor::StartProbe(std::shared_ptr<ProbeThread>& httpProbeThread,
    std::shared_ptr<ProbeThread>& httpsProbeThread, std::shared_ptr<ProbeThread>& backHttpThread,
    std::shared_ptr<ProbeThread>& backHttpsThread, bool needProxy)
{
    if (needProxy) {
        std::lock_guard<std::mutex> proxyLocker(proxyMtx_);
        httpProbeThread->UpdateGlobalHttpProxy(globalHttpProxy_);
        httpsProbeThread->UpdateGlobalHttpProxy(globalHttpProxy_);
        backHttpThread->UpdateGlobalHttpProxy(globalHttpProxy_);
        backHttpsThread->UpdateGlobalHttpProxy(globalHttpProxy_);
    } else {
        httpProbeThread->ProbeWithoutGlobalHttpProxy();
        httpsProbeThread->ProbeWithoutGlobalHttpProxy();
        backHttpThread->ProbeWithoutGlobalHttpProxy();
        backHttpsThread->ProbeWithoutGlobalHttpProxy();
    }
    if (netBearType_ == BEARER_CELLULAR) {
        httpsProbeThread->Start();
        httpProbeThread->Start();
        backHttpsThread->Start();
        backHttpThread->Start();
    } else {
        httpProbeThread->Start();
        httpsProbeThread->Start();
        backHttpThread->Start();
        backHttpsThread->Start();
    }
}

NetHttpProbeResult NetMonitor::GetThreadDetectResult(std::shared_ptr<ProbeThread>& probeThread, ProbeType probeType)
{
    NetHttpProbeResult result;
    if (!probeThread->IsDetecting()) {
        if (probeType == ProbeType::PROBE_HTTP || probeType == ProbeType::PROBE_HTTP_FALLBACK) {
            return probeThread->GetHttpProbeResult();
        } else {
            return probeThread->GetHttpsProbeResult();
        }
    }
    return result;
}

NetHttpProbeResult NetMonitor::ProcessThreadDetectResult(std::shared_ptr<ProbeThread>& httpProbeThread,
    std::shared_ptr<ProbeThread>& httpsProbeThread, std::shared_ptr<ProbeThread>& backHttpThread,
    std::shared_ptr<ProbeThread>& backHttpsThread)
{
    NetHttpProbeResult httpResult = GetThreadDetectResult(httpProbeThread, ProbeType::PROBE_HTTP);
    NetHttpProbeResult httpsResult = GetThreadDetectResult(httpsProbeThread, ProbeType::PROBE_HTTPS);
    NetHttpProbeResult backHttpResult = GetThreadDetectResult(backHttpThread, ProbeType::PROBE_HTTP_FALLBACK);
    NetHttpProbeResult backHttpsResult = GetThreadDetectResult(backHttpsThread, ProbeType::PROBE_HTTPS_FALLBACK);
    if (httpResult.IsNeedPortal()) {
        NETMGR_LOG_I("primary http detect result: portal");
        return httpResult;
    }
    if (backHttpResult.IsNeedPortal()) {
        NETMGR_LOG_I("fallback http detect result: portal");
        return backHttpResult;
    }
    if (httpsResult.IsSuccessful()) {
        NETMGR_LOG_D("primary https detect result: success");
        return httpsResult;
    }
    if (backHttpsResult.IsSuccessful()) {
        NETMGR_LOG_I("fallback https detect result: success");
        return backHttpsResult;
    }
    if (httpResult.IsSuccessful() && backHttpResult.IsSuccessful()) {
        NETMGR_LOG_I("both primary http and fallback http detect result success");
        return httpResult;
    }
    return httpsResult;
}

void NetMonitor::LoadGlobalHttpProxy()
{
    if (!CheckIfSettingsDataReady()) {
        NETMGR_LOG_E("data_share is not ready");
        return;
    }
    NetHttpProxyTracker httpProxyTracker;
    httpProxyTracker.ReadFromSettingsData(globalHttpProxy_);
}

void NetMonitor::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    std::unique_lock<std::mutex> proxyLocker(proxyMtx_);
    globalHttpProxy_ = httpProxy;
}

void NetMonitor::GetHttpProbeUrlFromConfig()
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
        pos += HTTP_URL_HEADER.length();
        httpUrl_ = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
        if (isNeedSuffix_) {
            uint64_t ranNum = CommonUtils::GenRandomNumber();
            httpUrl_ = httpUrl_ + std::string("_") + std::to_string(ranNum);
        }
    }

    pos = content.find(HTTPS_URL_HEADER);
    if (pos != std::string::npos) {
        pos += HTTPS_URL_HEADER.length();
        httpsUrl_ = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
    }

    pos = content.find(FALLBACK_HTTP_URL_HEADER);
    if (pos != std::string::npos) {
        pos += FALLBACK_HTTP_URL_HEADER.length();
        fallbackHttpUrl_ = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
    }

    pos = content.find(FALLBACK_HTTPS_URL_HEADER);
    if (pos != std::string::npos) {
        pos += FALLBACK_HTTPS_URL_HEADER.length();
        fallbackHttpsUrl_ = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
    }
    NETMGR_LOG_D("Get net detection http url:[%{public}s], https url:[%{public}s], fallback http url:[%{public}s],"
        " fallback https url:[%{public}s]", httpUrl_.c_str(), httpsUrl_.c_str(), fallbackHttpUrl_.c_str(),
        fallbackHttpsUrl_.c_str());
}

void NetMonitor::GetDetectUrlConfig()
{
    if (!std::filesystem::exists(DETECT_CFG_FILE)) {
        NETMGR_LOG_E("File not exist (%{public}s)", DETECT_CFG_FILE);
        return;
    }

    std::ifstream file(DETECT_CFG_FILE);
    if (!file.is_open()) {
        NETMGR_LOG_E("Open file failed (%{public}s)", strerror(errno));
        return;
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();
    auto pos = content.find(ADD_RANDOM_CFG_PREFIX);
    if (pos != std::string::npos) {
        pos += ADD_RANDOM_CFG_PREFIX.length();
        std::string value = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
        value = CommonUtils::Trim(value);
        isNeedSuffix_ = value.compare(ADD_RANDOM_CFG_VALUE) == 0;
    }
    NETMGR_LOG_I("is need add suffix (%{public}d)", isNeedSuffix_);
}

bool NetMonitor::CheckIfSettingsDataReady()
{
    if (isDataShareReady_) {
        return true;
    }
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        NETMGR_LOG_E("GetSystemAbilityManager failed.");
        return false;
    }
    sptr<IRemoteObject> dataShareSa = saManager->GetSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    if (dataShareSa == nullptr) {
        NETMGR_LOG_E("Get dataShare SA Failed.");
        return false;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        NETMGR_LOG_E("NetDataShareHelperUtils GetSystemAbility Service Failed.");
        return false;
    }
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> ret =
            DataShare::DataShareHelper::Create(remoteObj, SETTINGS_DATASHARE_URI, SETTINGS_DATA_EXT_URI);
    NETMGR_LOG_I("create data_share helper, ret=%{public}d", ret.first);
    if (ret.first == DataShare::E_OK) {
        NETMGR_LOG_I("create data_share helper success");
        auto helper = ret.second;
        if (helper != nullptr) {
            bool releaseRet = helper->Release();
            NETMGR_LOG_I("release data_share helper, releaseRet=%{public}d", releaseRet);
        }
        isDataShareReady_ = true;
        return true;
    } else if (ret.first == DataShare::E_DATA_SHARE_NOT_READY) {
        NETMGR_LOG_E("create data_share helper failed");
        isDataShareReady_ = false;
        return false;
    }
    NETMGR_LOG_E("data_share unknown.");
    return true;
}

void NetMonitor::SetScreenState(bool isScreenOn)
{
    isScreenOn_ = isScreenOn;
}

void NetMonitor::DetectionDelayWhenScreenOff()
{
    uint64_t nowTime = CommonUtils::GetCurrentMilliSeconds();
    if (!isScreenOn_ && (nowTime - lastDetectTimestamp_) < SCREENOFF_DETECTION_INTERVEL_MS) {
        uint64_t delayTime = SCREENOFF_DETECTION_INTERVEL_MS - (nowTime - lastDetectTimestamp_);
        std::unique_lock<std::mutex> locker(detectionMtx_);
        detectionCond_.wait_for(locker, std::chrono::milliseconds(delayTime));
        locker.unlock();
    }
}

uint64_t NetMonitor::GetLastDetectTime()
{
    return lastDetectTimestamp_;
}
} // namespace NetManagerStandard
} // namespace OHOS
