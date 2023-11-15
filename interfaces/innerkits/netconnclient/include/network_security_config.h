/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_BASE_NET_SECURITY_CONFIG_H
#define NETMANAGER_BASE_NET_SECURITY_CONFIG_H

#include <string>
#include <set>
#include <json/json.h>

namespace OHOS {
namespace NetManagerStandard {

struct Domain {
    std::string domainName_;
    bool includeSubDomains_;
};

struct TrustAnchors {
    std::vector<std::string> certs_;
};

struct Pin {
    std::string digestAlgorithm_;
    std::string digest_;
};

struct PinSet {
    std::vector<Pin> pins_;
    std::string expiration_;
};

struct BaseConfig {
    TrustAnchors trustAnchors_;
};

struct DomainConfig {
    std::vector<Domain> domains_;
    TrustAnchors trustAnchors_;
    PinSet pinSet_;
};

class NetworkSecurityConfig final {
public:
    int32_t GetPinSetForHostName(const std::string &hostname, std::string &pins);

private:
    int32_t GetConfig();
    int32_t GetJsonFromBundle(std::string &jsonProfile);
    int32_t ParseJsonConfig(const std::string &content);
    void ParseJsonBaseConfig(const Json::Value &root, BaseConfig &baseConfig);
    void ParseJsonDomainConfigs(const Json::Value &root, std::vector<DomainConfig> &domainConfigs);
    void ParseJsonTrustAnchors(const Json::Value &root, TrustAnchors &trustAnchors);
    void ParseJsonDomains(const Json::Value &root, std::vector<Domain> &domains);
    void ParseJsonPinSet(const Json::Value &root, PinSet &pinSet);
    bool ValidateDate(const std::string &dateStr);
    void DumpConfigs();

private:
    BaseConfig baseConfig_;
    std::vector<DomainConfig> domainConfigs_;
};

}
}
#endif /* NETMANAGER_BASE_NET_SECURITY_CONFIG_H */
