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

#include <thread>

#include "dns_resolv_listen.h"
#include "netnative_log_wrapper.h"
#include "singleton.h"

#include "dns_manager.h"

namespace OHOS {
namespace nmd {
void StartListen()
{
    NETNATIVE_LOGE("Enter threadStart");
    DnsResolvListen().StartListen();
}

DnsManager::DnsManager()
{
    std::thread(StartListen).detach();
}

int32_t DnsManager::SetResolverConfig(uint16_t netId,
                                      uint16_t baseTimeoutMillis,
                                      uint8_t retryCount,
                                      const std::vector<std::string> &servers,
                                      const std::vector<std::string> &domains)
{
    NETNATIVE_LOGI("manager_SetResolverConfig netId[%{public}d]", netId);
    return DelayedSingleton<DnsParamCache>::GetInstance()->SetResolverConfig(netId, baseTimeoutMillis, retryCount,
                                                                             servers, domains);
}

int32_t DnsManager::GetResolverConfig(uint16_t netId,
                                      std::vector<std::string> &servers,
                                      std::vector<std::string> &domains,
                                      uint16_t &baseTimeoutMillis,
                                      uint8_t &retryCount)
{
    NETNATIVE_LOGI("manager_GetResolverConfig netId[%{public}d]", netId);
    return DelayedSingleton<DnsParamCache>::GetInstance()->GetResolverConfig(netId, servers, domains, baseTimeoutMillis,
                                                                             retryCount);
}

int32_t DnsManager::CreateNetworkCache(uint16_t netId)
{
    NETNATIVE_LOGI("manager_CreateNetworkCache netId[%{public}d]", netId);
    return DelayedSingleton<DnsParamCache>::GetInstance()->CreateCacheForNet(netId);
}

void DnsManager::SetDefaultNetwork(int netId)
{
    NETNATIVE_LOGI("SetDefaultNetwork netId[%{public}d]", netId);
    DelayedSingleton<DnsParamCache>::GetInstance()->SetDefaultNetwork(netId);
}

DnsManager::~DnsManager() {}
} // namespace nmd
} // namespace OHOS
