/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "net_http_proxy_tracker.h"

#include "base64_utils.h"
#include "net_datashare_utils.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *EXCLUSIONS_SPLIT_SYMBOL = ",";
constexpr const char *DEFAULT_HTTP_PROXY_HOST = "NONE";
constexpr const char *DEFAULT_HTTP_PROXY_PORT = "0";
constexpr const char *DEFAULT_HTTP_PROXY_EXCLUSION_LIST = "NONE";
} // namespace

void NetHttpProxyTracker::ReadFromSettingsData(HttpProxy &httpProxy)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    std::string proxyHost;
    std::string proxyPort;
    std::string proxyExclusions;
    Uri hostUri(GLOBAL_PROXY_HOST_URI);
    int32_t ret = dataShareHelperUtils->Query(hostUri, KEY_GLOBAL_PROXY_HOST, proxyHost);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_D("Query global proxy host failed.");
    }
    std::string host = Base64::Decode(proxyHost);
    host = (host == DEFAULT_HTTP_PROXY_HOST ? "" : host);

    Uri portUri(GLOBAL_PROXY_PORT_URI);
    ret = dataShareHelperUtils->Query(portUri, KEY_GLOBAL_PROXY_PORT, proxyPort);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_D("Query global proxy port failed.");
    }
    uint16_t port = (proxyPort.empty() || host.empty()) ? 0 : static_cast<uint16_t>(CommonUtils::StrToUint(proxyPort));

    Uri exclusionsUri(GLOBAL_PROXY_EXCLUSIONS_URI);
    ret = dataShareHelperUtils->Query(exclusionsUri, KEY_GLOBAL_PROXY_EXCLUSIONS, proxyExclusions);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_D("Query global proxy exclusions failed.");
    }
    std::list<std::string> exclusionList =
        host.empty() ? std::list<std::string>() : ParseExclusionList(proxyExclusions);
    httpProxy = {host, port, exclusionList};
}

bool NetHttpProxyTracker::WriteToSettingsData(HttpProxy &httpProxy)
{
    std::string host =
        httpProxy.GetHost().empty() ? Base64::Encode(DEFAULT_HTTP_PROXY_HOST) : Base64::Encode(httpProxy.GetHost());
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri hostUri(GLOBAL_PROXY_HOST_URI);
    int32_t ret = dataShareHelperUtils->Update(hostUri, KEY_GLOBAL_PROXY_HOST, host);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Set host:%{public}s to datashare failed", host.c_str());
        return false;
    }

    Uri portUri(GLOBAL_PROXY_PORT_URI);
    std::string port = httpProxy.GetHost().empty() ? DEFAULT_HTTP_PROXY_PORT : std::to_string(httpProxy.GetPort());
    ret = dataShareHelperUtils->Update(portUri, KEY_GLOBAL_PROXY_PORT, port);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Set port:%{public}s to datashare failed", port.c_str());
        return false;
    }

    std::string exclusions = GetExclusionsAsString(httpProxy.GetExclusionList());
    exclusions = (httpProxy.GetHost().empty() || exclusions.empty()) ? DEFAULT_HTTP_PROXY_EXCLUSION_LIST : exclusions;
    Uri exclusionsUri(GLOBAL_PROXY_EXCLUSIONS_URI);
    ret = dataShareHelperUtils->Update(exclusionsUri, KEY_GLOBAL_PROXY_EXCLUSIONS, exclusions);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Set exclusions:%{public}s to datashare", exclusions.c_str());
        return false;
    }
    httpProxy.SetExclusionList(ParseExclusionList(exclusions));
    return true;
}

std::list<std::string> NetHttpProxyTracker::ParseExclusionList(const std::string &exclusions) const
{
    std::list<std::string> exclusionList;
    if (exclusions.empty() || exclusions == DEFAULT_HTTP_PROXY_EXCLUSION_LIST) {
        return exclusionList;
    }
    size_t startPos = 0;
    size_t searchPos = exclusions.find(EXCLUSIONS_SPLIT_SYMBOL);
    std::string exclusion;
    while (searchPos != std::string::npos) {
        exclusion = exclusions.substr(startPos, (searchPos - startPos));
        exclusionList.push_back(exclusion);
        startPos = searchPos + 1;
        searchPos = exclusions.find(EXCLUSIONS_SPLIT_SYMBOL, startPos);
    }
    exclusion = exclusions.substr(startPos, (exclusions.size() - startPos));
    exclusionList.push_back(exclusion);
    return exclusionList;
}

std::string NetHttpProxyTracker::GetExclusionsAsString(const std::list<std::string> &exclusionList) const
{
    std::string exclusions;
    int32_t index = 0;
    for (const auto &exclusion : exclusionList) {
        if (exclusion.empty()) {
            continue;
        }
        if (index > 0) {
            exclusions = exclusions + EXCLUSIONS_SPLIT_SYMBOL;
        }
        exclusions = exclusions + exclusion;
        index++;
    }
    return exclusions;
}
} // namespace NetManagerStandard
} // namespace OHOS