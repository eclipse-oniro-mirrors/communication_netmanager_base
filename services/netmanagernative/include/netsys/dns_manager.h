/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef __INCLUDE_MANAGER_DNS_MANAGER_H__
#define __INCLUDE_MANAGER_DNS_MANAGER_H__

#include <vector>

#include "dns_param_cache.h"

namespace OHOS {
namespace nmd {
class DnsManager {
public:
    DnsManager();
    ~DnsManager();

    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMillis, uint8_t retryCount,
                              const std::vector<std::string> &servers, const std::vector<std::string> &domains);
    int32_t GetResolverConfig(uint16_t netId, std::vector<std::string> &servers, std::vector<std::string> &domains,
                              uint16_t &baseTimeoutMillis, uint8_t &retryCount);

    int32_t CreateNetworkCache(uint16_t netId);

    void SetDefaultNetwork(int netId);
};
} // namespace nmd
} // namespace OHOS
#endif // !__INCLUDE_MANAGER_DNS_MANAGER_H__
