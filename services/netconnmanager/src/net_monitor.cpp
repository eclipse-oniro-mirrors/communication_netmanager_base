/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "net_monitor.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetMonitor::NetMonitor(NetDetectionStateHandler handle)
{
    netMonitorThread_ = nullptr;
    isExitNetMonitorThread_ = false;
    isStopNetMonitor_ = true;
    isExitNetMonitorThread_ = false;
    netDetectionStatus_ = handle;
    lastDetectionState_ = INVALID_DETECTION_STATE;
}

NetMonitor::~NetMonitor()
{
    ExitNetMonitorThread();
}

bool NetMonitor::HttpDetection()
{
    HttpRequest httpRequest;
    httpRequest.SetIfaceName(ifaceName_);

    std::string httpMsg(DEFAULT_PORTAL_HTTPS_URL);
    std::string httpHeader;
    int32_t ret = httpRequest.HttpGetHeader(httpMsg, httpHeader);
    std::string urlRedirect;
    if (ret != 0 || httpHeader.empty()) {
        NETMGR_LOG_E("The network is abnormal or Response code returned by http [httpHeader] is empty!");
        netDetectionStatus_(NetDetectionStatus::INVALID_DETECTION_STATE, urlRedirect);
        lastDetectionState_ = INVALID_DETECTION_STATE;
        return true;
    }

    int32_t retCode = GetUrlRedirectFromResponse(httpHeader, urlRedirect);
    int32_t statusCode = GetStatusCodeFromResponse(httpHeader);
    bool isNotPortal = true;
    if ((statusCode == OK || (statusCode >= BAD_REQUEST && statusCode <= CLIENT_ERROR_MAX)) &&
        retCode > PORTAL_CONTENT_LENGTH_MIN) {
        if (retCode > -1) {
            netDetectionStatus_(NetDetectionStatus::CAPTIVE_PORTAL_STATE, urlRedirect);
        }
        lastDetectionState_ = CAPTIVE_PORTAL_STATE;
        NETMGR_LOG_D("This network is portal AP, need certification!");
        isNotPortal = false;
    } else if (statusCode == NO_CONTENT) {
        NETMGR_LOG_D("This network is normal!");
        netDetectionStatus_(NetDetectionStatus::VERIFICATION_STATE, urlRedirect);
        lastDetectionState_ = VERIFICATION_STATE;
        isNotPortal = true;
    } else if (statusCode != NO_CONTENT && statusCode >= CREATED && statusCode <= URL_REDIRECT_MAX) {
        if (retCode > -1) {
            netDetectionStatus_(NetDetectionStatus::CAPTIVE_PORTAL_STATE, urlRedirect);
        }
        lastDetectionState_ = CAPTIVE_PORTAL_STATE;
        NETMGR_LOG_D("This network is portal AP, need certification!");
        isNotPortal = false;
    } else {
        NETMGR_LOG_D("This network can't online!");
        netDetectionStatus_(NetDetectionStatus::INVALID_DETECTION_STATE, urlRedirect);
        lastDetectionState_ = INVALID_DETECTION_STATE;
        isNotPortal = true;
    }

    NETMGR_LOG_D("statusCode[%{public}d], urlRedirect[%{public}s]", statusCode, urlRedirect.c_str());
    return isNotPortal;
}

void NetMonitor::RunNetMonitorThreadFunc()
{
    int32_t timeoutMs = HTTP_DETECTION_WAIT_TIME_MS;
    for (;;) {
        while (isStopNetMonitor_ && !isExitNetMonitorThread_) {
            NETMGR_LOG_D("waiting for signal");
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock);
        }

        if (isExitNetMonitorThread_) {
            NETMGR_LOG_D("break the loop");
            break;
        }
        HttpDetection();
        if (!isExitNetMonitorThread_) {
            NETMGR_LOG_D("conditionTimeout_ wait_for.");
            std::unique_lock<std::mutex> lock(mutex_);
            if (conditionTimeout_.wait_for(lock, std::chrono::milliseconds(timeoutMs)) == std::cv_status::timeout) {
                NETMGR_LOG_D("conditionTimeout_ timeout.");
            } else {
                NETMGR_LOG_D("for SignalNetMonitor, wakeup!");
            }
        }
    }
}

ResultCode NetMonitor::InitNetMonitorThread()
{
    netMonitorThread_ = new (std::nothrow) std::thread(&NetMonitor::RunNetMonitorThreadFunc, this);
    if (netMonitorThread_ == nullptr) {
        NETMGR_LOG_E("Start NetMonitor thread failed!");
        return  ResultCode::ERR_NET_MONITOR_OPT_FAILED;
    }
    return  ResultCode::ERR_NET_MONITOR_OPT_SUCCESS;
}

void NetMonitor::StopNetMonitorThread()
{
    NETMGR_LOG_D("Enter StopNetMonitorThread");
    std::unique_lock<std::mutex> lock(mutex_);
    isStopNetMonitor_ = true;
}

void NetMonitor::SignalNetMonitorThread(const std::string &ifaceName)
{
    NETMGR_LOG_D("Enter SignalNetMonitorThread");
    std::unique_lock<std::mutex> lock(mutex_);
    ifaceName_ = ifaceName;
    lastDetectionState_ = INVALID_DETECTION_STATE;
    isStopNetMonitor_ = false;
    condition_.notify_one();
    conditionTimeout_.notify_one();
    NETMGR_LOG_D("leave SignalNetMonitorThread!");
}

void NetMonitor::ExitNetMonitorThread()
{
    NETMGR_LOG_D("Enter NetMonitor::ExitNetMonitorThread");
    {
        std::unique_lock<std::mutex> lock(mutex_);
        isStopNetMonitor_ = false;
        isExitNetMonitorThread_ = true;
        condition_.notify_one();
        conditionTimeout_.notify_one();
    }

    if (netMonitorThread_ != nullptr) {
        netMonitorThread_->join();
        delete netMonitorThread_;
        netMonitorThread_ = nullptr;
    }
}

int32_t NetMonitor::GetStatusCodeFromResponse(const std::string &strResponse)
{
    if (strResponse.empty()) {
        NETMGR_LOG_E("strResponse is empty");
        return -1;
    }

    std::string::size_type newLinePos = strResponse.find("\r\n");
    if (newLinePos == std::string::npos) {
        NETMGR_LOG_E("StrResponse did not find the response line!");
        return -1;
    }
    std::string statusLine = strResponse.substr(0, newLinePos);
    std::string::size_type spacePos = statusLine.find(" ");
    if (spacePos == std::string::npos) {
        NETMGR_LOG_E("No spaces found in the response line!");
        return -1;
    }
    std::string strStatusCode = statusLine.substr(spacePos + 1, statusLine.length() - 1);
    std::string::size_type pos = strStatusCode.find(" ");
    if (pos == std::string::npos) {
        NETMGR_LOG_E("No other space was found in the response line!");
        return -1;
    }
    strStatusCode = strStatusCode.substr(0, pos);
    if (strStatusCode.empty()) {
        NETMGR_LOG_E("String status code is empty!");
        return -1;
    }

    int32_t statusCode = std::stoi(strStatusCode);
    return statusCode;
}

int32_t NetMonitor::GetUrlRedirectFromResponse(const std::string &strResponse, std::string &urlRedirect)
{
    if (strResponse.empty()) {
        NETMGR_LOG_E("strResponse is empty");
        return -1;
    }

    std::string::size_type startPos = strResponse.find(PORTAL_URL_REDIRECT_FIRST_CASE);
    std::string::size_type endPos = 0;
    if (startPos != std::string::npos) {
        startPos += PORTAL_URL_REDIRECT_FIRST_CASE.length();
        endPos = strResponse.find(PORTAL_END_STR);
        if (endPos != std::string::npos) {
            urlRedirect = strResponse.substr(startPos, endPos - startPos);
        }
        return 0;
    }

    startPos = strResponse.find(PORTAL_URL_REDIRECT_SECOND_CASE);
    if (startPos != std::string::npos) {
        startPos += PORTAL_URL_REDIRECT_SECOND_CASE.length();
        endPos = strResponse.find(PORTAL_END_STR);
        if (endPos != std::string::npos) {
            urlRedirect = strResponse.substr(startPos, endPos - startPos);
        }
        startPos = strResponse.find(CONTENT_STR);
        return std::atoi(strResponse.substr(startPos + CONTENT_STR.length(), NET_CONTENT_LENGTH).c_str());
    }

    return -1;
}
}  // namespace NetManagerStandard
}  // namespace OHOS