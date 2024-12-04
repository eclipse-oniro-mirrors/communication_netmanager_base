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
#ifdef NETMANAGER_BASE_POWER_MANAGER_ENABLE
#include "power_mgr_client.h"
#endif

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t INIT_DETECTION_DELAY_MS = 1 * 1000;
constexpr int32_t MAX_FAILED_DETECTION_DELAY_MS = 10 * 60 * 1000;
constexpr int32_t PRIMARY_DETECTION_RESULT_WAIT_MS = 3 * 1000;
constexpr int32_t ALL_DETECTION_RESULT_WAIT_MS = 10 * 1000;
constexpr int32_t CAPTIVE_PORTAL_DETECTION_DELAY_MS = 15 * 1000;
#ifdef NETMANAGER_BASE_POWER_MANAGER_ENABLE
constexpr int32_t SCREENOFF_PORTAL_DETECTION_DELAY_MS = 5 * 60 * 1000;
#endif
constexpr int32_t DOUBLE = 2;
constexpr int32_t SIM_PORTAL_CODE = 302;
constexpr int32_t ONE_URL_DETECT_NUM = 2;
constexpr int32_t ALL_DETECT_THREAD_NUM = 4;
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
    while (netMonitor->IsDetecting()) {
        netMonitor->Detection();
    }
}

NetMonitor::NetMonitor(uint32_t netId, NetBearType bearType, const NetLinkInfo &netLinkInfo,
                       const std::weak_ptr<INetMonitorCallback> &callback)
    : netId_(netId), netLinkInfo_(netLinkInfo), netMonitorCallback_(callback)
{
    netBearType_ = bearType;
#ifdef NETMANAGER_BASE_POWER_MANAGER_ENABLE
    isScreenOn_ = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
#endif
    LoadGlobalHttpProxy();
    GetDetectUrlConfig();
    GetHttpProbeUrlFromConfig();
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

void NetMonitor::ProcessDetection(NetHttpProbeResult& probeResult, NetDetectionStatus& result)
{
    if (probeResult.IsSuccessful()) {
        NETMGR_LOG_I("Net[%{public}d] probe success", netId_);
        isDetecting_ = false;
        needDetectionWithoutProxy_ = true;
        result = VERIFICATION_STATE;
    } else if (probeResult.GetCode() == SIM_PORTAL_CODE && netBearType_ == BEARER_CELLULAR) {
        NETMGR_LOG_E("Net[%{public}d] probe failed with 302 response on Cell", netId_);
        detectionDelay_ = MAX_FAILED_DETECTION_DELAY_MS;
        result = CAPTIVE_PORTAL_STATE;
    } else if (probeResult.IsNeedPortal()) {
        NETMGR_LOG_W("Net[%{public}d] need portal", netId_);
#ifdef NETMANAGER_BASE_POWER_MANAGER_ENABLE
        if (!isScreenOn_ && netBearType_ == BEARER_WIFI) {
            detectionDelay_ = SCREENOFF_PORTAL_DETECTION_DELAY_MS;
        } else {
            detectionDelay_ = CAPTIVE_PORTAL_DETECTION_DELAY_MS;
        }
#else
        detectionDelay_ = CAPTIVE_PORTAL_DETECTION_DELAY_MS;
#endif
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

void NetMonitor::Detection()
{
    NetHttpProbeResult probeResult = SendProbe();
    if (isDetecting_) {
        NetDetectionStatus result = UNKNOWN_STATE;
        ProcessDetection(probeResult, result);
    }
}

NetHttpProbeResult NetMonitor::SendProbe()
{
    NETMGR_LOG_I("start net detection");
    std::lock_guard<std::mutex> monitorLocker(probeMtx_);
    std::shared_ptr<TinyCountDownLatch> latch = std::make_shared<TinyCountDownLatch>(ONE_URL_DETECT_NUM);
    std::shared_ptr<TinyCountDownLatch> latchAll = std::make_shared<TinyCountDownLatch>(ALL_DETECT_THREAD_NUM);
    std::shared_ptr<ProbeThread> primaryHttpThread = std::make_shared<ProbeThread>(
        netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTP, httpUrl_, httpsUrl_);
    std::shared_ptr<ProbeThread> primaryHttpsThread = std::make_shared<ProbeThread>(
        netId_, netBearType_, netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTPS, httpUrl_, httpsUrl_);
    {
        std::lock_guard<std::mutex> proxyLocker(proxyMtx_);
        primaryHttpThread->UpdateGlobalHttpProxy(globalHttpProxy_);
        primaryHttpsThread->UpdateGlobalHttpProxy(globalHttpProxy_);
    }
    primaryHttpThread->Start();
    primaryHttpsThread->Start();

    latch->Await(std::chrono::milliseconds(PRIMARY_DETECTION_RESULT_WAIT_MS));
    NetHttpProbeResult httpProbeResult = GetThreadDetectResult(primaryHttpThread, ProbeType::PROBE_HTTP);
    NetHttpProbeResult httpsProbeResult = GetThreadDetectResult(primaryHttpsThread, ProbeType::PROBE_HTTPS);
    if (httpProbeResult.IsNeedPortal()) {
        NETMGR_LOG_I("http detect result: portal");
        return httpProbeResult;
    }
    if (httpsProbeResult.IsSuccessful()) {
        NETMGR_LOG_I("https detect result: success");
        return httpsProbeResult;
    }
    NETMGR_LOG_I("backup url detection start");
    std::shared_ptr<ProbeThread> fallbackHttpThread = std::make_shared<ProbeThread>(netId_, netBearType_,
        netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTP_FALLBACK, fallbackHttpUrl_, fallbackHttpsUrl_);
    std::shared_ptr<ProbeThread> fallbackHttpsThread = std::make_shared<ProbeThread>(netId_, netBearType_,
        netLinkInfo_, latch, latchAll, ProbeType::PROBE_HTTPS_FALLBACK, fallbackHttpUrl_, fallbackHttpsUrl_);
    fallbackHttpThread->ProbeWithoutGlobalHttpProxy();
    fallbackHttpsThread->ProbeWithoutGlobalHttpProxy();
    fallbackHttpThread->Start();
    fallbackHttpsThread->Start();
    latchAll->Await(std::chrono::milliseconds(ALL_DETECTION_RESULT_WAIT_MS));
    httpProbeResult = GetThreadDetectResult(primaryHttpThread, ProbeType::PROBE_HTTP);
    httpsProbeResult = GetThreadDetectResult(primaryHttpsThread, ProbeType::PROBE_HTTPS);
    NetHttpProbeResult fallbackHttpProbeResult =
        GetThreadDetectResult(fallbackHttpThread, ProbeType::PROBE_HTTP_FALLBACK);
    NetHttpProbeResult fallbackHttpsProbeResult =
        GetThreadDetectResult(fallbackHttpsThread, ProbeType::PROBE_HTTPS_FALLBACK);
    return ProcessThreadDetectResult(httpProbeResult, httpsProbeResult, fallbackHttpProbeResult,
        fallbackHttpsProbeResult);
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

NetHttpProbeResult NetMonitor::ProcessThreadDetectResult(NetHttpProbeResult& httpProbeResult,
    NetHttpProbeResult& httpsProbeResult, NetHttpProbeResult& fallbackHttpProbeResult,
    NetHttpProbeResult& fallbackHttpsProbeResult)
{
    if (httpProbeResult.IsNeedPortal()) {
        NETMGR_LOG_I("primary http detect result: portal");
        return httpProbeResult;
    }
    if (fallbackHttpProbeResult.IsNeedPortal()) {
        NETMGR_LOG_I("fallback http detect result: portal");
        return fallbackHttpProbeResult;
    }
    if (httpsProbeResult.IsSuccessful()) {
        NETMGR_LOG_I("primary https detect result: success");
        return httpsProbeResult;
    }
    if (fallbackHttpsProbeResult.IsSuccessful()) {
        NETMGR_LOG_I("fallback https detect result: success");
        return fallbackHttpsProbeResult;
    }
    if (httpProbeResult.IsSuccessful() && fallbackHttpProbeResult.IsSuccessful()) {
        NETMGR_LOG_I("both primary http and fallback http detect result success");
        return httpProbeResult;
    }
    return httpsProbeResult;
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

#ifdef NETMANAGER_BASE_POWER_MANAGER_ENABLE
void NetMonitor::SetScreenState(bool isScreenOn)
{
    isScreenOn_ = isScreenOn;
}
#endif
} // namespace NetManagerStandard
} // namespace OHOS
