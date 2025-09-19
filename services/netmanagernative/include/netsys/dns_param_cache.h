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

#ifndef NETSYS_DNS_PARAM_CACHE_H
#define NETSYS_DNS_PARAM_CACHE_H

#include <iostream>
#include <map>

#include "ffrt.h"
#include "dns_resolv_config.h"
#include "netnative_log_wrapper.h"
#include "uid_range.h"
#ifdef FEATURE_NET_FIREWALL_ENABLE
#include "netfirewall_parcel.h"
#include "i_netfirewall_callback.h"
#include "suffix_match_trie.h"
#include <unordered_map>
#endif

namespace OHOS::nmd {
#ifdef FEATURE_NET_FIREWALL_ENABLE
using namespace OHOS::NetManagerStandard;
#endif
class DnsParamCache {
public:
    ~DnsParamCache() = default;

    static DnsParamCache &GetInstance();

    // for net_conn_service
    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                              const std::vector<std::string> &servers, const std::vector<std::string> &domains);

    int32_t CreateCacheForNet(uint16_t netId, bool isVpnNet = false);

    void SetDefaultNetwork(uint16_t netId);

    // for client
    void SetDnsCache(uint16_t netId, const std::string &hostName, const AddrInfo &addrInfo);

    void SetCacheDelayed(uint16_t netId, const std::string &hostName);

    std::vector<AddrInfo> GetDnsCache(uint16_t netId, const std::string &hostName);

    int32_t GetResolverConfig(uint16_t netId, std::vector<std::string> &servers, std::vector<std::string> &domains,
                              uint16_t &baseTimeoutMsec, uint8_t &retryCount);

    int32_t GetResolverConfig(uint16_t netId, uint32_t uid, std::vector<std::string> &servers,
                              std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount);

    int32_t GetDefaultNetwork() const;

    void GetDumpInfo(std::string &info);

    int32_t DestroyNetworkCache(uint16_t netId, bool isVpnNet = false);

    bool IsIpv6Enable(uint16_t netId);

    void EnableIpv6(uint16_t netId);

    int32_t AddUidRange(uint32_t netId, const std::vector<NetManagerStandard::UidRange> &uidRanges);

    int32_t DelUidRange(uint32_t netId, const std::vector<NetManagerStandard::UidRange> &uidRanges);

    bool IsVpnOpen() const;

#ifdef FEATURE_NET_FIREWALL_ENABLE
    int32_t SetFirewallDefaultAction(FirewallRuleAction inDefault, FirewallRuleAction outDefault);

    int32_t SetFirewallCurrentUserId(int32_t userId);

    void ClearAllDnsCache();

    int32_t SetFirewallRules(NetFirewallRuleType type, const std::vector<sptr<NetFirewallBaseRule>> &ruleList,
                             bool isFinish);

    int32_t ClearFirewallRules(NetFirewallRuleType type);

    void SetCallingUid(uint32_t callingUid)
    {
        callingUid_ = callingUid;
    }

    uint32_t GetCallingUid()
    {
        return callingUid_;
    }

    int32_t RegisterNetFirewallCallback(const sptr<NetsysNative::INetFirewallCallback> &callback);

    int32_t UnRegisterNetFirewallCallback(const sptr<NetsysNative::INetFirewallCallback> &callback);
#endif
    int32_t SetUserDefinedServerFlag(uint16_t netId, bool flag);

    int32_t GetUserDefinedServerFlag(uint16_t netId, bool &flag);

    int32_t GetUserDefinedServerFlag(uint16_t netId, bool &flag, uint32_t uid);

    bool IsUseVpnDns(uint32_t uid);

    int32_t FlushDnsCache(uint16_t netId);

private:
    DnsParamCache();

    std::vector<NetManagerStandard::UidRange> vpnUidRanges_;

    int32_t vpnNetId_;

    ffrt::mutex cacheMutex_;

    ffrt::mutex uidRangeMutex_;

    std::atomic_uint defaultNetId_;

    std::map<uint16_t, DnsResolvConfig> serverConfigMap_;

    static std::vector<std::string> SelectNameservers(const std::vector<std::string> &servers);

    std::vector<std::string> RemoveDuplicateNameservers(const std::vector<std::string> &servers);

#ifdef FEATURE_NET_FIREWALL_ENABLE
    int32_t GetUserId(int32_t appUid);

    bool GetDnsServersByAppUid(int32_t appUid, std::vector<std::string> &servers);

    void BuildFirewallDomainLsmTrie(const sptr<NetFirewallDomainRule> &rule, const std::string &domain);

    void BuildFirewallDomainMap(const sptr<NetFirewallDomainRule> &rule, const std::string &domain);

    int32_t SetFirewallDnsRules(const std::vector<sptr<NetFirewallDnsRule>> &ruleList);

    int32_t SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList);

    FirewallRuleAction GetFirewallRuleAction(int32_t appUid, const std::vector<sptr<NetFirewallDomainRule>> &rules);

    bool checkEmpty4InterceptDomain(const std::string &hostName);

    bool IsInterceptDomain(int32_t appUid, const std::string &host, bool &isMatchAllow);

    void NotifyDomianIntercept(int32_t appUid, const std::string &host);

    std::vector<sptr<NetFirewallDomainRule>> firewallDomainRules_;

    std::vector<sptr<NetFirewallDnsRule>> firewallDnsRules_;

    sptr<NetManagerStandard::InterceptRecord> oldRecord_ = nullptr;

    std::unordered_map<int32_t, std::vector<sptr<NetFirewallDnsRule>>> netFirewallDnsRuleMap_;

    std::unordered_map<std::string, std::vector<sptr<NetFirewallDomainRule>>> netFirewallDomainRulesAllowMap_;

    std::unordered_map<std::string, std::vector<sptr<NetFirewallDomainRule>>> netFirewallDomainRulesDenyMap_;

    std::shared_ptr<NetManagerStandard::SuffixMatchTrie<std::vector<sptr<NetFirewallDomainRule>>>> domainAllowLsmTrie_ =
        nullptr;

    std::shared_ptr<NetManagerStandard::SuffixMatchTrie<std::vector<sptr<NetFirewallDomainRule>>>> domainDenyLsmTrie_ =
        nullptr;

    uint32_t callingUid_;

    int32_t currentUserId_ = 0;

    std::vector<sptr<NetsysNative::INetFirewallCallback>> callbacks_;

    FirewallRuleAction firewallDefaultAction_ = FirewallRuleAction::RULE_INVALID;
#endif
};
} // namespace OHOS::nmd
#endif // NETSYS_DNS_PARAM_CACHE_H
