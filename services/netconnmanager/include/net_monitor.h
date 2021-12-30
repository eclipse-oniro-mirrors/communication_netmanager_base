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

#ifndef NET_MONITOR_H
#define NET_MONITOR_H

#include <mutex>
#include <thread>

#include "http_request.h"
#include "net_conn_types.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string DEFAULT_PORTAL_HTTPS_URL = "http://connectivitycheck.platform.hicloud.com/generate_204";
constexpr int32_t HTTP_DETECTION_WAIT_TIME_MS = 10000;
const std::string PORTAL_URL_REDIRECT_FIRST_CASE = "Location: ";
const std::string PORTAL_URL_REDIRECT_SECOND_CASE = "http";
const std::string CONTENT_STR = "Content-Length:";
const std::string PORTAL_END_STR = "?";
constexpr int32_t PORTAL_CONTENT_LENGTH_MIN = 4;
constexpr int32_t NET_CONTENT_LENGTH = 6;

class NetMonitor {
public:
    NetMonitor(NetDetectionStateHandler handle);
    virtual ~NetMonitor();
    /**
     * @brief : Start NetMonitor thread
     * @Return success : NET_OPT_SUCCESS  failed : NET_OPT_FAILED
     */
    ResultCode InitNetMonitorThread();

    /**
     * @brief : wake up the DHCP processing thread.
     * @param ifaceName
     */
    void SignalNetMonitorThread(const std::string &ifaceName);

    /**
     * @brief : stop the NetMonitor processing thread.
     *
     */
    void StopNetMonitorThread();

    /**
     * @brief Get the Status Code From Response object
     *
     * @param strResponse
     * @return int32_t
     */
    int32_t GetStatusCodeFromResponse(const std::string &strResponse);

    /**
     * @brief Get the Url Redirect From Response object
     *
     * @param strResponse  Response data obtained from the server
     * @param urlRedirect    The redirected url obtained from the response data
     * @return int32_t   if urlRedirect > -1 Get url
     */
    int32_t GetUrlRedirectFromResponse(const std::string &strResponse, std::string &urlRedirect);

private:
    /**
     * @brief : Detect Internet ability
     * @return true http detection success, false http detection failed
     */
    bool HttpDetection();

    /**
     * @brief : NetMonitor thread function
     */
    void RunNetMonitorThreadFunc();

    /**
     * @brief : Exit the NetMonitor thread.
     *
     */
    void ExitNetMonitorThread();

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::condition_variable conditionTimeout_;
    bool isStopNetMonitor_;
    bool isExitNetMonitorThread_;
    std::thread *netMonitorThread_;
    NetDetectionStateHandler netDetectionStatus_;
    NetDetectionStatus lastDetectionState_;
    std::string ifaceName_;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif // NET_MONITOR_H