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

#include <pthread.h>
#include <thread>

#include "dns_resolv_listen.h"
#include "netnative_log_wrapper.h"
#include "singleton.h"

#include "dns_manager.h"

namespace OHOS {
namespace nmd {
void StartListen()
{
    NETNATIVE_LOG_D("Enter threadStart");
    DnsResolvListen().StartListen();
}

DnsManager::DnsManager() : dnsProxyListen_(std::make_shared<DnsProxyListen>())
{
    std::thread t(StartListen);
    std::string threadName = "DnsMgerListen";
    pthread_setname_np(t.native_handle(), threadName.c_str());
    t.detach();
}

void DnsManager::EnableIpv6(uint16_t netId)
{
    DnsParamCache::GetInstance().EnableIpv6(netId);
}

int32_t DnsManager::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMillis, uint8_t retryCount,
                                      const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    NETNATIVE_LOG_D("manager_SetResolverConfig netId[%{public}d]", netId);
    return DnsParamCache::GetInstance().SetResolverConfig(netId, baseTimeoutMillis, retryCount, servers, domains);
}

int32_t DnsManager::GetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
                                      std::vector<std::string> &domains, uint16_t &baseTimeoutMillis,
                                      uint8_t &retryCount)
{
    NETNATIVE_LOG_D("manager_GetResolverConfig netId[%{public}d]", netId);
    return DnsParamCache::GetInstance().GetResolverConfig(netId, servers, domains, baseTimeoutMillis, retryCount);
}

int32_t DnsManager::CreateNetworkCache(uint16_t netId)
{
    NETNATIVE_LOG_D("manager_CreateNetworkCache netId[%{public}d]", netId);
    return DnsParamCache::GetInstance().CreateCacheForNet(netId);
}

int32_t DnsManager::DestroyNetworkCache(uint16_t netId)
{
    return DnsParamCache::GetInstance().DestroyNetworkCache(netId);
}

void DnsManager::SetDefaultNetwork(uint16_t netId)
{
    DnsParamCache::GetInstance().SetDefaultNetwork(netId);
}

void StartProxyListen()
{
    NETNATIVE_LOG_D("begin StartProxyListen");
    DnsProxyListen().StartListen();
}

void DnsManager::ShareDnsSet(uint16_t netId)
{
    dnsProxyListen_->SetParseNetId(netId);
}

void DnsManager::StartDnsProxyListen()
{
    dnsProxyListen_->OnListen();
    std::thread t(StartProxyListen);
    std::string threadName = "DnsPxyListen";
    pthread_setname_np(t.native_handle(), threadName.c_str());
    t.detach();
}

void DnsManager::StopDnsProxyListen()
{
    dnsProxyListen_->OffListen();
}

void DnsManager::GetDumpInfo(std::string &info)
{
    NETNATIVE_LOG_D("Get dump info");
    DnsParamCache::GetInstance().GetDumpInfo(info);
}

int32_t DnsManager::GetAddrInfo(const std::string &hostName, const std::string &serverName, const AddrInfo &hints,
                                uint16_t netId, std::vector<AddrInfo> &res)
{
    if (netId == 0) {
        netId = DnsParamCache::GetInstance().GetDefaultNetwork();
        NETNATIVE_LOG_D("DnsManager DnsGetaddrinfo netId == 0 defaultNetId_ : %{public}d", netId);
    }
    return dnsGetAddrInfo_->GetAddrInfo(hostName, serverName, hints, netId, res);
}
} // namespace nmd
} // namespace OHOS
