/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NET_ACCESS_POLICY_CONFIG_H
#define NET_ACCESS_POLICY_CONFIG_H

#include "cJSON.h"
#include "ffrt.h"
#include "string"

namespace OHOS {
namespace NetManagerStandard {
struct NetAccessPolicyConfig {
    std::string bundleName;
    bool disableWlanSwitch = false;
    bool disableCellularSwitch = false;
};
class NetAccessPolicyConfigUtils {
public:
    static NetAccessPolicyConfigUtils &GetInstance();
    std::vector<NetAccessPolicyConfig> GetNetAccessPolicyConfig();

private:
    NetAccessPolicyConfigUtils() = default;
    ~NetAccessPolicyConfigUtils() = default;
    void Init();
    void ParseNetAccessPolicyConfigs();
    bool ReadFile(std::string &content, const std::string &fileName);
    std::string ParseString(cJSON *value);
    bool ParseBoolean(cJSON *value);
    int32_t ParseInt32(cJSON *value);

private:
    static NetAccessPolicyConfigUtils instance_;

    std::vector<NetAccessPolicyConfig> netAccessPolicyConfigs_;
    std::atomic<bool> isInit_ = false;
    ffrt::mutex lock_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif