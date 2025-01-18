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

#include "dns_param_cache.h"

#include <algorithm>

#include "netmanager_base_common_utils.h"
#ifdef FEATURE_NET_FIREWALL_ENABLE
#include "bpf_netfirewall.h"
#include "netfirewall_parcel.h"
#include <ctime>
#endif

namespace OHOS::nmd {
using namespace OHOS::NetManagerStandard::CommonUtils;
namespace {
void GetVectorData(const std::vector<std::string> &data, std::string &result)
{
    result.append("{ ");
    std::for_each(data.begin(), data.end(), [&result](const auto &str) { result.append(ToAnonymousIp(str) + ", "); });
    result.append("}\n");
}
constexpr int RES_TIMEOUT = 4000;    // min. milliseconds between retries
constexpr int RES_DEFAULT_RETRY = 2; // Default
} // namespace

DnsParamCache::DnsParamCache() : defaultNetId_(0) {}

DnsParamCache &DnsParamCache::GetInstance()
{
    static DnsParamCache instance;
    return instance;
}

std::vector<std::string> DnsParamCache::SelectNameservers(const std::vector<std::string> &servers)
{
    std::vector<std::string> res = servers;
    if (res.size() > MAX_SERVER_NUM - 1) {
        res.resize(MAX_SERVER_NUM - 1);
    }
    return res;
}

int32_t DnsParamCache::CreateCacheForNet(uint16_t netId)
{
    NETNATIVE_LOG_D("DnsParamCache::CreateCacheForNet, netid:%{public}d,", netId);
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    auto it = serverConfigMap_.find(netId);
    if (it != serverConfigMap_.end()) {
        NETNATIVE_LOGE("DnsParamCache::CreateCacheForNet, netid already exist, no need to create");
        return -EEXIST;
    }
    serverConfigMap_[netId].SetNetId(netId);
    return 0;
}

int32_t DnsParamCache::DestroyNetworkCache(uint16_t netId)
{
    NETNATIVE_LOG_D("DnsParamCache::CreateCacheForNet, netid:%{public}d,", netId);
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        return -ENOENT;
    }
    serverConfigMap_.erase(it);
    if (defaultNetId_ == netId) {
        defaultNetId_ = 0;
    }
    for (auto it = serverConfigMap_.begin(); it != serverConfigMap_.end(); it++) {
        it->second.GetCache().Clear();
    }
    return 0;
}

int32_t DnsParamCache::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                         const std::vector<std::string> &servers,
                                         const std::vector<std::string> &domains)
{
    std::vector<std::string> nameservers = SelectNameservers(servers);
    NETNATIVE_LOG_D("DnsParamCache::SetResolverConfig, netid:%{public}d, numServers:%{public}d,", netId,
                    static_cast<int>(nameservers.size()));

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);

    // select_domains
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        NETNATIVE_LOGE("DnsParamCache::SetResolverConfig failed, netid is non-existent");
        return -ENOENT;
    }

    auto oldDnsServers = it->second.GetServers();
    std::sort(oldDnsServers.begin(), oldDnsServers.end());

    auto newDnsServers = servers;
    std::sort(newDnsServers.begin(), newDnsServers.end());

    for (auto iterator = serverConfigMap_.begin(); iterator != serverConfigMap_.end(); iterator++) {
        iterator->second.GetCache().Clear();
    }

    if (oldDnsServers != newDnsServers) {
        it->second.GetCache().Clear();
    }

    it->second.SetNetId(netId);
    it->second.SetServers(servers);
    it->second.SetDomains(domains);
    if (retryCount == 0) {
        it->second.SetRetryCount(RES_DEFAULT_RETRY);
    } else {
        it->second.SetRetryCount(retryCount);
    }
    if (baseTimeoutMsec == 0) {
        it->second.SetTimeoutMsec(RES_TIMEOUT);
    } else {
        it->second.SetTimeoutMsec(baseTimeoutMsec);
    }
    return 0;
}

void DnsParamCache::SetDefaultNetwork(uint16_t netId)
{
    defaultNetId_ = netId;
}

void DnsParamCache::EnableIpv6(uint16_t netId)
{
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        DNS_CONFIG_PRINT("get Config failed: netid is not have netid:%{public}d,", netId);
        return;
    }

    it->second.EnableIpv6();
}

bool DnsParamCache::IsIpv6Enable(uint16_t netId)
{
    if (netId == 0) {
        netId = defaultNetId_;
    }

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        DNS_CONFIG_PRINT("get Config failed: netid is not have netid:%{public}d,", netId);
        return false;
    }

    return it->second.IsIpv6Enable();
}

int32_t DnsParamCache::GetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
                                         std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                         uint8_t &retryCount)
{
    NETNATIVE_LOG_D("DnsParamCache::GetResolverConfig no uid");
    if (netId == 0) {
        netId = defaultNetId_;
        NETNATIVE_LOG_D("defaultNetId_ = [%{public}u]", netId);
    }

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        DNS_CONFIG_PRINT("get Config failed: netid is not have netid:%{public}d,", netId);
        return -ENOENT;
    }

    servers = it->second.GetServers();
#ifdef FEATURE_NET_FIREWALL_ENABLE
    std::vector<std::string> dns;
    if (GetDnsServersByAppUid(GetCallingUid(), dns)) {
        DNS_CONFIG_PRINT("GetResolverConfig hit netfirewall");
        servers.assign(dns.begin(), dns.end());
    }
#endif
    domains = it->second.GetDomains();
    baseTimeoutMsec = it->second.GetTimeoutMsec();
    retryCount = it->second.GetRetryCount();

    return 0;
}

int32_t DnsParamCache::GetResolverConfig(uint16_t netId, uint32_t uid, std::vector<std::string> &servers,
                                         std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                         uint8_t &retryCount)
{
    NETNATIVE_LOG_D("DnsParamCache::GetResolverConfig has uid");
    if (netId == 0) {
        netId = defaultNetId_;
        NETNATIVE_LOG_D("defaultNetId_ = [%{public}u]", netId);
    }
    
    {
        std::lock_guard<ffrt::mutex> guard(cacheMutex_);
        for (auto mem : vpnUidRanges_) {
            if (static_cast<int32_t>(uid) >= mem.begin_ && static_cast<int32_t>(uid) <= mem.end_) {
                NETNATIVE_LOG_D("is vpn hap");
                auto it = serverConfigMap_.find(vpnNetId_);
                if (it == serverConfigMap_.end()) {
                    NETNATIVE_LOG_D("vpn get Config failed: not have vpnnetid:%{public}d,", vpnNetId_);
                    break;
                }
                servers = it->second.GetServers();
#ifdef FEATURE_NET_FIREWALL_ENABLE
                std::vector<std::string> dns;
                if (GetDnsServersByAppUid(GetCallingUid(), dns)) {
                    DNS_CONFIG_PRINT("GetResolverConfig hit netfirewall");
                    servers.assign(dns.begin(), dns.end());
                }
#endif
                domains = it->second.GetDomains();
                baseTimeoutMsec = it->second.GetTimeoutMsec();
                retryCount = it->second.GetRetryCount();
                return 0;
            }
        }
    }
    return GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
}

int32_t DnsParamCache::GetDefaultNetwork() const
{
    return defaultNetId_;
}

void DnsParamCache::SetDnsCache(uint16_t netId, const std::string &hostName, const AddrInfo &addrInfo)
{
    if (netId == 0) {
        netId = defaultNetId_;
    }
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
#ifdef FEATURE_NET_FIREWALL_ENABLE
    int32_t appUid = static_cast<int32_t>(GetCallingUid());
    bool isMatchAllow = false;
    if (IsInterceptDomain(appUid, hostName, isMatchAllow)) {
        DNS_CONFIG_PRINT("SetDnsCache failed: domain was Intercepted: %{public}s,", hostName.c_str());
        return;
    }
    if (isMatchAllow && (addrInfo.aiFamily == AF_INET || addrInfo.aiFamily == AF_INET6)) {
        NetAddrInfo netInfo;
        netInfo.aiFamily = addrInfo.aiFamily;
        if (addrInfo.aiFamily == AF_INET) {
            netInfo.aiAddr.sin = addrInfo.aiAddr.sin.sin_addr;
        } else {
            memcpy_s(&netInfo.aiAddr.sin6, sizeof(addrInfo.aiAddr.sin6.sin6_addr), &addrInfo.aiAddr.sin6.sin6_addr,
                     sizeof(addrInfo.aiAddr.sin6.sin6_addr));
        }
        OHOS::NetManagerStandard::NetsysBpfNetFirewall::GetInstance()->AddDomainCache(netInfo);
    }
#endif
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        DNS_CONFIG_PRINT("SetDnsCache failed: netid is not have netid:%{public}d,", netId);
        return;
    }

    it->second.GetCache().Put(hostName, addrInfo);
}

std::vector<AddrInfo> DnsParamCache::GetDnsCache(uint16_t netId, const std::string &hostName)
{
    if (netId == 0) {
        netId = defaultNetId_;
    }

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
#ifdef FEATURE_NET_FIREWALL_ENABLE
    int32_t appUid = static_cast<int32_t>(GetCallingUid());
    bool isMatchAllow = false;
    if (IsInterceptDomain(appUid, hostName, isMatchAllow)) {
        NotifyDomianIntercept(appUid, hostName);
        AddrInfo fakeAddr = { 0 };
        fakeAddr.aiFamily = AF_UNSPEC;
        fakeAddr.aiAddr.sin.sin_family = AF_UNSPEC;
        fakeAddr.aiAddr.sin.sin_addr.s_addr = INADDR_NONE;
        fakeAddr.aiAddrLen = sizeof(struct sockaddr_in);
        return { fakeAddr };
    }
#endif

    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        DNS_CONFIG_PRINT("GetDnsCache failed: netid is not have netid:%{public}d,", netId);
        return {};
    }

    return it->second.GetCache().Get(hostName);
}

void DnsParamCache::SetCacheDelayed(uint16_t netId, const std::string &hostName)
{
    if (netId == 0) {
        netId = defaultNetId_;
    }

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        DNS_CONFIG_PRINT("SetCacheDelayed failed: netid is not have netid:%{public}d,", netId);
        return;
    }

    it->second.SetCacheDelayed(hostName);
}

int32_t DnsParamCache::AddUidRange(uint32_t netId, const std::vector<NetManagerStandard::UidRange> &uidRanges)
{
    std::lock_guard<ffrt::mutex> guard(uidRangeMutex_);
    NETNATIVE_LOG_D("DnsParamCache::AddUidRange size = [%{public}zu]", uidRanges.size());
    vpnNetId_ = netId;
    auto middle = vpnUidRanges_.insert(vpnUidRanges_.end(), uidRanges.begin(), uidRanges.end());
    std::inplace_merge(vpnUidRanges_.begin(), middle, vpnUidRanges_.end());
    return 0;
}

int32_t DnsParamCache::DelUidRange(uint32_t netId, const std::vector<NetManagerStandard::UidRange> &uidRanges)
{
    std::lock_guard<ffrt::mutex> guard(uidRangeMutex_);
    NETNATIVE_LOG_D("DnsParamCache::DelUidRange size = [%{public}zu]", uidRanges.size());
    vpnNetId_ = 0;
    auto end = std::set_difference(vpnUidRanges_.begin(), vpnUidRanges_.end(), uidRanges.begin(),
                                   uidRanges.end(), vpnUidRanges_.begin());
    vpnUidRanges_.erase(end, vpnUidRanges_.end());
    return 0;
}

bool DnsParamCache::IsVpnOpen() const
{
    return vpnUidRanges_.size();
}

#ifdef FEATURE_NET_FIREWALL_ENABLE
int32_t DnsParamCache::GetUserId(int32_t appUid)
{
    int32_t userId = appUid / USER_ID_DIVIDOR;
    return userId > 0 ? userId : currentUserId_;
}

bool DnsParamCache::GetDnsServersByAppUid(int32_t appUid, std::vector<std::string> &servers)
{
    if (netFirewallDnsRuleMap_.empty()) {
        return false;
    }
    DNS_CONFIG_PRINT("GetDnsServersByAppUid: appUid=%{public}d", appUid);
    auto it = netFirewallDnsRuleMap_.find(appUid);
    if (it == netFirewallDnsRuleMap_.end()) {
        // if appUid not found, try to find invalid appUid=0;
        it = netFirewallDnsRuleMap_.find(0);
    }
    if (it != netFirewallDnsRuleMap_.end()) {
        int32_t userId = GetUserId(appUid);
        std::vector<sptr<NetFirewallDnsRule>> rules = it->second;
        for (const auto &rule : rules) {
            if (rule->userId != userId) {
                continue;
            }
            servers.emplace_back(rule->primaryDns);
            servers.emplace_back(rule->standbyDns);
        }
        return true;
    }
    return false;
}

int32_t DnsParamCache::SetFirewallRules(NetFirewallRuleType type,
                                        const std::vector<sptr<NetFirewallBaseRule>> &ruleList, bool isFinish)
{
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    NETNATIVE_LOGI("SetFirewallRules: size=%{public}zu isFinish=%{public}" PRId32, ruleList.size(), isFinish);
    if (ruleList.empty()) {
        NETNATIVE_LOGE("SetFirewallRules: rules is empty");
        return -1;
    }
    int32_t ret = 0;
    switch (type) {
        case NetFirewallRuleType::RULE_DNS: {
            for (const auto &rule : ruleList) {
                firewallDnsRules_.emplace_back(firewall_rule_cast<NetFirewallDnsRule>(rule));
            }
            if (isFinish) {
                ret = SetFirewallDnsRules(firewallDnsRules_);
                firewallDnsRules_.clear();
            }
            break;
        }
        case NetFirewallRuleType::RULE_DOMAIN: {
            for (const auto &rule : ruleList) {
                firewallDomainRules_.emplace_back(firewall_rule_cast<NetFirewallDomainRule>(rule));
            }
            if (isFinish) {
                ret = SetFirewallDomainRules(firewallDomainRules_);
                firewallDomainRules_.clear();
                OHOS::NetManagerStandard::NetsysBpfNetFirewall::GetInstance()->ClearDomainCache();
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

int32_t DnsParamCache::SetFirewallDnsRules(const std::vector<sptr<NetFirewallDnsRule>> &ruleList)
{
    for (const auto &rule : ruleList) {
        std::vector<sptr<NetFirewallDnsRule>> rules;
        auto it = netFirewallDnsRuleMap_.find(rule->appUid);
        if (it != netFirewallDnsRuleMap_.end()) {
            rules = it->second;
        }
        rules.emplace_back(std::move(rule));
        netFirewallDnsRuleMap_.emplace(rule->appUid, std::move(rules));
    }
    return 0;
}

FirewallRuleAction DnsParamCache::GetFirewallRuleAction(int32_t appUid,
                                                        const std::vector<sptr<NetFirewallDomainRule>> &rules)
{
    int32_t userId = GetUserId(appUid);
    for (const auto &rule : rules) {
        if (rule->userId != userId) {
            continue;
        }
        if ((rule->appUid && appUid == rule->appUid) || !rule->appUid) {
            return rule->ruleAction;
        }
    }

    return FirewallRuleAction::RULE_INVALID;
}

bool DnsParamCache::checkEmpty4InterceptDomain(const std::string &hostName)
{
    if (hostName.empty()) {
        return true;
    }
    if (!netFirewallDomainRulesAllowMap_.empty() || !netFirewallDomainRulesDenyMap_.empty()) {
        return false;
    }
    if (domainAllowLsmTrie_ && !domainAllowLsmTrie_->Empty()) {
        return false;
    }
    return !domainDenyLsmTrie_ || domainDenyLsmTrie_->Empty();
}

bool DnsParamCache::IsInterceptDomain(int32_t appUid, const std::string &hostName, bool &isMatchAllow)
{
    if (checkEmpty4InterceptDomain(hostName)) {
        return false;
    }
    std::string host = hostName.substr(0, hostName.find(' '));
    DNS_CONFIG_PRINT("IsInterceptDomain: appUid: %{public}d, hostName: %{private}s", appUid, host.c_str());
    std::transform(host.begin(), host.end(), host.begin(), ::tolower);
    std::vector<sptr<NetFirewallDomainRule>> rules;
    FirewallRuleAction exactAllowAction = FirewallRuleAction::RULE_INVALID;
    auto it = netFirewallDomainRulesAllowMap_.find(host);
    if (it != netFirewallDomainRulesAllowMap_.end()) {
        rules = it->second;
        exactAllowAction = GetFirewallRuleAction(appUid, rules);
    }
    FirewallRuleAction exactDenyAction = FirewallRuleAction::RULE_INVALID;
    auto iter = netFirewallDomainRulesDenyMap_.find(host);
    if (iter != netFirewallDomainRulesDenyMap_.end()) {
        rules = iter->second;
        exactDenyAction = GetFirewallRuleAction(appUid, rules);
    }
    FirewallRuleAction wildcardAllowAction = FirewallRuleAction::RULE_INVALID;
    if (domainAllowLsmTrie_->LongestSuffixMatch(host, rules)) {
        wildcardAllowAction = GetFirewallRuleAction(appUid, rules);
    }
    FirewallRuleAction wildcardDenyAction = FirewallRuleAction::RULE_INVALID;
    if (domainDenyLsmTrie_->LongestSuffixMatch(host, rules)) {
        wildcardDenyAction = GetFirewallRuleAction(appUid, rules);
    }
    isMatchAllow = (exactAllowAction != FirewallRuleAction::RULE_INVALID) ||
                   (wildcardAllowAction != FirewallRuleAction::RULE_INVALID);
    bool isDeny = (exactDenyAction != FirewallRuleAction::RULE_INVALID) ||
                  (wildcardDenyAction != FirewallRuleAction::RULE_INVALID);
    if (isMatchAllow) {
        // Apply default rules in case of conflict
        return isDeny && (firewallDefaultAction_ == FirewallRuleAction::RULE_DENY);
    }
    return isDeny;
}

int32_t DnsParamCache::SetFirewallDefaultAction(FirewallRuleAction inDefault, FirewallRuleAction outDefault)
{
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    DNS_CONFIG_PRINT("SetFirewallDefaultAction: firewallDefaultAction_: %{public}d", (int)outDefault);
    firewallDefaultAction_ = outDefault;
    return 0;
}

void DnsParamCache::BuildFirewallDomainLsmTrie(const sptr<NetFirewallDomainRule> &rule, const std::string &domain)
{
    std::vector<sptr<NetFirewallDomainRule>> rules;
    std::string suffix(domain);
    auto wildcardCharIndex = suffix.find('*');
    if (wildcardCharIndex != std::string::npos) {
        suffix = suffix.substr(wildcardCharIndex + 1);
    }
    DNS_CONFIG_PRINT("BuildFirewallDomainLsmTrie: suffix: %{public}s", suffix.c_str());
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
    if (rule->ruleAction == FirewallRuleAction::RULE_DENY) {
        if (domainDenyLsmTrie_->LongestSuffixMatch(suffix, rules)) {
            rules.emplace_back(std::move(rule));
            domainDenyLsmTrie_->Update(suffix, rules);
            return;
        }
        rules.emplace_back(std::move(rule));
        domainDenyLsmTrie_->Insert(suffix, rules);
    } else {
        if (domainAllowLsmTrie_->LongestSuffixMatch(suffix, rules)) {
            rules.emplace_back(std::move(rule));
            domainAllowLsmTrie_->Update(suffix, rules);
            return;
        }
        rules.emplace_back(std::move(rule));
        domainAllowLsmTrie_->Insert(suffix, rules);
    }
}

void DnsParamCache::BuildFirewallDomainMap(const sptr<NetFirewallDomainRule> &rule, const std::string &raw)
{
    DNS_CONFIG_PRINT("BuildFirewallDomainMap: domain: %{public}s", raw.c_str());
    std::string domain(raw);
    std::vector<sptr<NetFirewallDomainRule>> rules;
    std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);
    if (rule->ruleAction == FirewallRuleAction::RULE_DENY) {
        auto it = netFirewallDomainRulesDenyMap_.find(domain);
        if (it != netFirewallDomainRulesDenyMap_.end()) {
            rules = it->second;
        }

        rules.emplace_back(std::move(rule));
        netFirewallDomainRulesDenyMap_.emplace(domain, std::move(rules));
    } else {
        auto it = netFirewallDomainRulesAllowMap_.find(domain);
        if (it != netFirewallDomainRulesAllowMap_.end()) {
            rules = it->second;
        }

        rules.emplace_back(rule);
        netFirewallDomainRulesAllowMap_.emplace(domain, std::move(rules));
    }
}

int32_t DnsParamCache::SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList)
{
    if (!domainAllowLsmTrie_) {
        domainAllowLsmTrie_ =
            std::make_shared<NetManagerStandard::SuffixMatchTrie<std::vector<sptr<NetFirewallDomainRule>>>>();
    }
    if (!domainDenyLsmTrie_) {
        domainDenyLsmTrie_ =
            std::make_shared<NetManagerStandard::SuffixMatchTrie<std::vector<sptr<NetFirewallDomainRule>>>>();
    }
    for (const auto &rule : ruleList) {
        for (const auto &param : rule->domains) {
            if (param.isWildcard) {
                BuildFirewallDomainLsmTrie(rule, param.domain);
            } else {
                BuildFirewallDomainMap(rule, param.domain);
            }
        }
    }
    return 0;
}

int32_t DnsParamCache::ClearFirewallRules(NetFirewallRuleType type)
{
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    switch (type) {
        case NetFirewallRuleType::RULE_DNS:
            firewallDnsRules_.clear();
            netFirewallDnsRuleMap_.clear();
            break;
        case NetFirewallRuleType::RULE_DOMAIN: {
            firewallDomainRules_.clear();
            netFirewallDomainRulesAllowMap_.clear();
            netFirewallDomainRulesDenyMap_.clear();
            if (domainAllowLsmTrie_) {
                domainAllowLsmTrie_ = nullptr;
            }
            if (domainDenyLsmTrie_) {
                domainDenyLsmTrie_ = nullptr;
            }
            OHOS::NetManagerStandard::NetsysBpfNetFirewall::GetInstance()->ClearDomainCache();
            break;
        }
        case NetFirewallRuleType::RULE_ALL: {
            firewallDnsRules_.clear();
            netFirewallDnsRuleMap_.clear();
            firewallDomainRules_.clear();
            netFirewallDomainRulesAllowMap_.clear();
            netFirewallDomainRulesDenyMap_.clear();
            if (domainAllowLsmTrie_) {
                domainAllowLsmTrie_ = nullptr;
            }
            if (domainDenyLsmTrie_) {
                domainDenyLsmTrie_ = nullptr;
            }
            OHOS::NetManagerStandard::NetsysBpfNetFirewall::GetInstance()->ClearDomainCache();
            break;
        }
        default:
            break;
    }
    return 0;
}

void DnsParamCache::NotifyDomianIntercept(int32_t appUid, const std::string &hostName)
{
    if (hostName.empty()) {
        return;
    }
    std::string host = hostName.substr(0, hostName.find(' '));
    NETNATIVE_LOGI("NotifyDomianIntercept: appUid: %{public}d, hostName: %{private}s", appUid, host.c_str());
    sptr<NetManagerStandard::InterceptRecord> record = new (std::nothrow) NetManagerStandard::InterceptRecord();
    record->time = (int32_t)time(NULL);
    record->appUid = appUid;
    record->domain = host;

    if (oldRecord_ != nullptr && (record->time - oldRecord_->time) < INTERCEPT_BUFF_INTERVAL_SEC) {
        if (record->appUid == oldRecord_->appUid && record->domain == oldRecord_->domain) {
            return;
        }
    }
    oldRecord_ = record;
    for (const auto &callback : callbacks_) {
        callback->OnIntercept(record);
    }
}

int32_t DnsParamCache::RegisterNetFirewallCallback(const sptr<NetsysNative::INetFirewallCallback> &callback)
{
    if (!callback) {
        return -1;
    }

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    callbacks_.emplace_back(callback);

    return 0;
}

int32_t DnsParamCache::UnRegisterNetFirewallCallback(const sptr<NetsysNative::INetFirewallCallback> &callback)
{
    if (!callback) {
        return -1;
    }

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it) {
        if (*it == callback) {
            callbacks_.erase(it);
            return 0;
        }
    }
    return -1;
}
#endif

void DnsParamCache::GetDumpInfo(std::string &info)
{
    std::string dnsData;
    static const std::string TAB = "  ";
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    std::for_each(serverConfigMap_.begin(), serverConfigMap_.end(), [&dnsData](const auto &serverConfig) {
        dnsData.append(TAB + "NetId: " + std::to_string(serverConfig.second.GetNetId()) + "\n");
        dnsData.append(TAB + "TimeoutMsec: " + std::to_string(serverConfig.second.GetTimeoutMsec()) + "\n");
        dnsData.append(TAB + "RetryCount: " + std::to_string(serverConfig.second.GetRetryCount()) + "\n");
        dnsData.append(TAB + "Servers:");
        GetVectorData(serverConfig.second.GetServers(), dnsData);
        dnsData.append(TAB + "Domains:");
        GetVectorData(serverConfig.second.GetDomains(), dnsData);
    });
    info.append(dnsData);
}

int32_t DnsParamCache::SetUserDefinedServerFlag(uint16_t netId, bool flag)
{
    NETNATIVE_LOGI("DnsParamCache::SetUserDefinedServerFlag, netid:%{public}d, flag:%{public}d,", netId, flag);

    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    // select_domains
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        NETNATIVE_LOGE("DnsParamCache::SetUserDefinedServerFlag failed, netid is non-existent");
        return -ENOENT;
    }
    it->second.SetUserDefinedServerFlag(flag);
    return 0;
}

int32_t DnsParamCache::GetUserDefinedServerFlag(uint16_t netId, bool &flag)
{
    if (netId == 0) {
        netId = defaultNetId_;
        NETNATIVE_LOG_D("defaultNetId_ = [%{public}u]", netId);
    }
    std::lock_guard<ffrt::mutex> guard(cacheMutex_);
    auto it = serverConfigMap_.find(netId);
    if (it == serverConfigMap_.end()) {
        DNS_CONFIG_PRINT("GetUserDefinedServerFlag failed: netid is not have netid:%{public}d,", netId);
        return -ENOENT;
    }
    flag = it->second.IsUserDefinedServer();
    return 0;
}

int32_t DnsParamCache::GetUserDefinedServerFlag(uint16_t netId, bool &flag, uint32_t uid)
{
    if (netId == 0) {
        netId = defaultNetId_;
        NETNATIVE_LOG_D("defaultNetId_ = [%{public}u]", netId);
    }
    {
        std::lock_guard<ffrt::mutex> guard(cacheMutex_);
        for (auto mem : vpnUidRanges_) {
            if (static_cast<int32_t>(uid) >= mem.begin_ && static_cast<int32_t>(uid) <= mem.end_) {
                NETNATIVE_LOG_D("is vpn hap");
                auto it = serverConfigMap_.find(vpnNetId_);
                if (it == serverConfigMap_.end()) {
                    NETNATIVE_LOG_D("vpn get Config failed: not have vpnnetid:%{public}d,", vpnNetId_);
                    break;
                }
                flag = it->second.IsUserDefinedServer();
                return 0;
            }
        }
        auto it = serverConfigMap_.find(netId);
        if (it == serverConfigMap_.end()) {
            DNS_CONFIG_PRINT("GetUserDefinedServerFlag failed: netid is not have netid:%{public}d,", netId);
            return -ENOENT;
        }
    }
    return GetUserDefinedServerFlag(netId, flag);
}
} // namespace OHOS::nmd
