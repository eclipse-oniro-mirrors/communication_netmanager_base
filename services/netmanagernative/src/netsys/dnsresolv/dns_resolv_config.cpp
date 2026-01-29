/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dns_resolv_config.h"

namespace OHOS::nmd {
DnsResolvConfig::DelayedTaskWrapper::DelayedTaskWrapper(std::string hostName,
                                                        NetManagerStandard::LRUCache<AddrInfoWithTtl> &cache)
    : hostName_(std::move(hostName)), cache_(cache)
{
}

void DnsResolvConfig::DelayedTaskWrapper::Execute() const
{
    std::vector<AddrInfoWithTtl> infos = cache_.Get(hostName_);
    cache_.Delete(hostName_);
    if (infos.size() == 0) {
        return;
    }

    for (auto info : infos) {
        if (info.ttl > remainTime_) {
            info.ttl -= remainTime_;
            cache_.Put(hostName_, info);
        }
    }
}

uint32_t DnsResolvConfig::DelayedTaskWrapper::GetUpdateTime()
{
    std::vector<AddrInfoWithTtl> infos = cache_.Get(hostName_);
    if (infos.size() == 0) {
        return 0;
    }
    remainTime_ = infos[0].ttl;
    for (auto info : infos) {
        remainTime_ = remainTime_ < info.ttl ? remainTime_ : info.ttl;
    }
    return remainTime_;
}

bool DnsResolvConfig::DelayedTaskWrapper::operator<(const DelayedTaskWrapper &other) const
{
    return hostName_ < other.hostName_;
}

DnsResolvConfig::DnsResolvConfig()
    : netId_(0), netIdIsSet_(false), revisionId_(0), timeoutMsec_(0), retryCount_(0), isIpv6Enable_(false),
    isIpv4Enable_(false), isUserDefinedDnsServer_(false), isClatIpv4Enable_(false)
{
}

void DnsResolvConfig::EnableIpv6()
{
    isIpv6Enable_ = true;
}

bool DnsResolvConfig::IsIpv6Enable()
{
    return isIpv6Enable_;
}

void DnsResolvConfig::EnableIpv4()
{
    isIpv4Enable_ = true;
}

void DnsResolvConfig::SetClatDnsEnableIpv4(bool enable)
{
    isClatIpv4Enable_ = enable;
}

bool DnsResolvConfig::IsClatIpv4Enable()
{
    return isClatIpv4Enable_;
}

bool DnsResolvConfig::IsIpv4Enable()
{
    return isIpv4Enable_;
}

void DnsResolvConfig::SetNetId(uint16_t netId)
{
    if (netIdIsSet_) {
        return;
    }
    netIdIsSet_ = true;
    netId_ = netId;
}

uint16_t DnsResolvConfig::GetNetId() const
{
    return netId_;
}

void DnsResolvConfig::SetTimeoutMsec(int32_t baseTimeoutMsec)
{
    timeoutMsec_ = baseTimeoutMsec;
}

uint16_t DnsResolvConfig::GetTimeoutMsec() const
{
    return timeoutMsec_;
}

void DnsResolvConfig::SetRetryCount(uint8_t retryCount)
{
    retryCount_ = retryCount;
}

uint8_t DnsResolvConfig::GetRetryCount() const
{
    return retryCount_;
}

void DnsResolvConfig::SetServers(const std::vector<std::string> &servers)
{
    nameServers_ = servers;
    revisionId_++;
}

std::vector<std::string> DnsResolvConfig::GetServers() const
{
    return nameServers_;
}

void DnsResolvConfig::SetDomains(const std::vector<std::string> &domains)
{
    searchDomains_ = domains;
}

std::vector<std::string> DnsResolvConfig::GetDomains() const
{
    return searchDomains_;
}

NetManagerStandard::LRUCache<AddrInfoWithTtl> &DnsResolvConfig::GetCache()
{
    return cache_;
}

void DnsResolvConfig::SetCacheDelayed(const std::string &hostName)
{
    auto wrapper = std::make_shared<DelayedTaskWrapper>(hostName, cache_);
    uint32_t time = wrapper->GetUpdateTime();
    if (time == 0) {
        return;
    }
    delayedQueue_.Put(wrapper, time);
}

void DnsResolvConfig::SetUserDefinedServerFlag(bool flag)
{
    isUserDefinedDnsServer_ = flag;
}

bool DnsResolvConfig::IsUserDefinedServer()
{
    return isUserDefinedDnsServer_;
}
} // namespace OHOS::nmd
