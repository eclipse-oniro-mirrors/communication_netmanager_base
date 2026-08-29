/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <arpa/inet.h>
#include <sstream>

#include "netfirewall_parcel.h"
#include "net_mgr_log_wrapper.h"
#include "refbase.h"


namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t FIREWALL_MAX_LIST_SIZE = 100;
}
// Firewall IP parameters
bool NetFirewallIpParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint8(family)) {
        return false;
    }
    if (!parcel.WriteUint8(type)) {
        return false;
    }
    if (!parcel.WriteUint8(mask)) {
        return false;
    }
    if (family == FAMILY_IPV4) {
        if (!parcel.WriteUint32(ipv4.startIp.s_addr)) {
            return false;
        }
        if (type == MULTIPLE_IP) {
            if (!parcel.WriteUint32(ipv4.endIp.s_addr)) {
                return false;
            }
        }
        return true;
    }
    for (int32_t index = 0; index < IPV6_ARRAY_SIZE; index++) {
        if (!parcel.WriteUint8(ipv6.startIp.s6_addr[index])) {
            return false;
        }
        if (type == MULTIPLE_IP) {
            if (!parcel.WriteUint8(ipv6.endIp.s6_addr[index])) {
                return false;
            }
        }
    }
    return true;
}

sptr<NetFirewallIpParam> NetFirewallIpParam::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallIpParam> ptr = new (std::nothrow) NetFirewallIpParam();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallIpParam ptr is null");
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->family)) {
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->type)) {
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->mask)) {
        return nullptr;
    }

    if (ptr->family == FAMILY_IPV4) {
        if (!parcel.ReadUint32(ptr->ipv4.startIp.s_addr)) {
            return nullptr;
        }
        if (ptr->type == MULTIPLE_IP) {
            if (!parcel.ReadUint32(ptr->ipv4.endIp.s_addr)) {
                return nullptr;
            }
        }
        return ptr;
    }
    for (int32_t index = 0; index < IPV6_ARRAY_SIZE; index++) {
        if (!parcel.ReadUint8(ptr->ipv6.startIp.s6_addr[index])) {
            return nullptr;
        }
        if (ptr->type == MULTIPLE_IP) {
            if (!parcel.ReadUint8(ptr->ipv6.endIp.s6_addr[index])) {
                return nullptr;
            }
        }
    }
    return ptr;
}

std::vector<std::string> NetFirewallUtils::split(const std::string &text, char delim)
{
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            tokens.emplace_back(item);
        }
    }
    return tokens;
}

std::string NetFirewallUtils::erase(const std::string &src, const std::string &sub)
{
    size_t index = src.find(sub);
    if (index == std::string::npos) {
        return "";
    }
    return src.substr(index + sub.length(), src.length() - sub.length());
}

std::string NetFirewallIpParam::GetStartIp() const
{
    char ip[INET6_ADDRSTRLEN] = {};
    if (this->family == FAMILY_IPV4) {
        inet_ntop(AF_INET, &(this->ipv4.startIp), ip, INET_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET6, &(this->ipv6.startIp), ip, INET6_ADDRSTRLEN);
    }
    return ip;
}

std::string NetFirewallIpParam::GetEndIp() const
{
    if (this->type == SINGLE_IP) {
        return "";
    }
    char ip[INET6_ADDRSTRLEN] = {};
    if (this->family == FAMILY_IPV4) {
        inet_ntop(AF_INET, &(this->ipv4.endIp), ip, INET_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET6, &(this->ipv6.endIp), ip, INET6_ADDRSTRLEN);
    }
    return ip;
}

// Firewall port parameters
bool NetFirewallPortParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(startPort)) {
        return false;
    }
    if (!parcel.WriteUint16(endPort)) {
        return false;
    }
    return true;
}

sptr<NetFirewallPortParam> NetFirewallPortParam::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallPortParam> ptr = new (std::nothrow) NetFirewallPortParam();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallPortParam ptr is null");
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->startPort)) {
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->endPort)) {
        return nullptr;
    }
    return ptr;
}

// Firewall domain name parameters
bool NetFirewallDomainParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isWildcard)) {
        return false;
    }
    if (!parcel.WriteString(domain)) {
        return false;
    }
    return true;
}

sptr<NetFirewallDomainParam> NetFirewallDomainParam::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallDomainParam> ptr = new (std::nothrow) NetFirewallDomainParam();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallDomainParam ptr is null");
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->isWildcard)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->domain)) {
        return nullptr;
    }
    return ptr;
}

// Firewall DNS parameters
bool NetFirewallDnsParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(primaryDns)) {
        return false;
    }
    if (!parcel.WriteString(standbyDns)) {
        return false;
    }
    return true;
}

sptr<NetFirewallDnsParam> NetFirewallDnsParam::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallDnsParam> ptr = new (std::nothrow) NetFirewallDnsParam();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallDnsParam ptr is null");
        return nullptr;
    }
    if (!parcel.ReadString(ptr->primaryDns)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->standbyDns)) {
        return nullptr;
    }
    return ptr;
}

template <typename T> bool NetFirewallUtils::MarshallingList(const std::vector<T> &list, Parcel &parcel)
{
    uint32_t size = static_cast<uint32_t>(list.size());
    size = std::min(size, FIREWALL_MAX_LIST_SIZE);
    if (!parcel.WriteUint32(size)) {
        NETMGR_LOG_E("write netAddrList size to parcel failed");
        return false;
    }

    for (uint32_t index = 0; index < size; ++index) {
        auto value = list[index];
        if (!value.Marshalling(parcel)) {
            NETMGR_LOG_E("write MarshallingList to parcel failed");
            return false;
        }
    }
    return true;
}

template <typename T> bool NetFirewallUtils::UnmarshallingList(Parcel &parcel, std::vector<T> &list)
{
    std::vector<T>().swap(list);

    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        NETMGR_LOG_E("Read UnmarshallingList list size failed");
        return false;
    }
    size = std::min(size, FIREWALL_MAX_LIST_SIZE);
    for (uint32_t i = 0; i < size; i++) {
        auto value = T::Unmarshalling(parcel);
        if (value == nullptr) {
            return false;
        }
        list.emplace_back(*value);
    }
    return true;
}

// Firewall rules, external interfaces
bool NetFirewallRule::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(ruleId)) {
        return false;
    }
    if (!parcel.WriteString(ruleName)) {
        return false;
    }
    if (!parcel.WriteString(ruleDescription)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleDirection))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleAction))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleType))) {
        return false;
    }
    if (!parcel.WriteBool(isEnabled)) {
        return false;
    }
    if (!parcel.WriteInt32(appUid)) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(localIps, parcel)) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(remoteIps, parcel)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(protocol))) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(localPorts, parcel)) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(remotePorts, parcel)) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(domains, parcel)) {
        return false;
    }
    if (!dns.Marshalling(parcel)) {
        return false;
    }
    if (!parcel.WriteInt32(userId)) {
        return false;
    }
    if (!parcel.WriteString(interface)) {
        return false;
    }
    return true;
}

sptr<NetFirewallRule> NetFirewallRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallRule> ptr = sptr<NetFirewallRule>::MakeSptr();
    if (!parcel.ReadInt32(ptr->ruleId)) {
        return nullptr;
    }

    if (!parcel.ReadString(ptr->ruleName)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->ruleDescription)) {
        return nullptr;
    }
    int32_t ruleDirection = 0;
    if (!parcel.ReadInt32(ruleDirection)) {
        return nullptr;
    }
    if (ruleDirection < static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN) ||
        ruleDirection > static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT)) {
        NETMGR_LOG_E("Invalid ruleDirection: %{public}d", ruleDirection);
        return nullptr;
    }
    ptr->ruleDirection = static_cast<NetFirewallRuleDirection>(ruleDirection);
    int32_t ruleAction = 0;
    if (!parcel.ReadInt32(ruleAction)) {
        return nullptr;
    }
    if (ruleAction <= static_cast<int32_t>(FirewallRuleAction::RULE_INVALID) ||
        ruleAction > static_cast<int32_t>(FirewallRuleAction::RULE_DENY)) {
        NETMGR_LOG_E("Invalid ruleAction: %{public}d", ruleAction);
        return nullptr;
    }
    ptr->ruleAction = static_cast<FirewallRuleAction>(ruleAction);
    int32_t ruleType = 0;
    if (!parcel.ReadInt32(ruleType)) {
        return nullptr;
    }
    if (ruleType <= static_cast<int32_t>(NetFirewallRuleType::RULE_INVALID) ||
        ruleType > static_cast<int32_t>(NetFirewallRuleType::RULE_ALL) || ruleType == 0) {
        NETMGR_LOG_E("Invalid ruleType: %{public}d", ruleType);
        return nullptr;
    }
    ptr->ruleType = static_cast<NetFirewallRuleType>(ruleType);
    if (!parcel.ReadBool(ptr->isEnabled)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->appUid)) {
        return nullptr;
    }
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->localIps)) {
        return nullptr;
    }
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->remoteIps)) {
        return nullptr;
    }
    int32_t protocol = 0;
    if (!parcel.ReadInt32(protocol)) {
        return nullptr;
    }
    ptr->protocol = static_cast<NetworkProtocol>(protocol);
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->localPorts)) {
        return nullptr;
    }
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->remotePorts)) {
        return nullptr;
    }
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->domains)) {
        return nullptr;
    }
    sptr<NetFirewallDnsParam> dns = NetFirewallDnsParam::Unmarshalling(parcel);
    if (dns == nullptr) {
        return nullptr;
    }
    ptr->dns = *dns;
    if (!parcel.ReadInt32(ptr->userId)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->interface)) {
        return nullptr;
    }
    return ptr;
}

bool NetFirewallBaseRule::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(userId)) {
        return false;
    }
    if (!parcel.WriteInt32(appUid)) {
        return false;
    }
    return true;
}

sptr<NetFirewallBaseRule> NetFirewallBaseRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallBaseRule> ptr = new (std::nothrow) NetFirewallBaseRule();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallBaseRule ptr is null");
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->userId)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->appUid)) {
        return nullptr;
    }
    return ptr;
}

bool NetFirewallBaseRule::UnmarshallingBase(Parcel &parcel, sptr<NetFirewallBaseRule> ptr)
{
    if (ptr == nullptr) {
        NETMGR_LOG_E("UnmarshallingBase ptr is null");
        return false;
    }
    if (!parcel.ReadInt32(ptr->userId)) {
        return false;
    }
    if (!parcel.ReadInt32(ptr->appUid)) {
        return false;
    }
    return true;
}

// IP rule data
bool NetFirewallIpRule::Marshalling(Parcel &parcel) const
{
    if (!NetFirewallBaseRule::Marshalling(parcel)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleDirection))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleAction))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(protocol))) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(localIps, parcel)) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(remoteIps, parcel)) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(localPorts, parcel)) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(remotePorts, parcel)) {
        return false;
    }
    if (!parcel.WriteString(interface)) {
        return false;
    }
    return true;
}

sptr<NetFirewallIpRule> NetFirewallIpRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallIpRule> ptr = new (std::nothrow) NetFirewallIpRule();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallIpRule ptr is null");
        return nullptr;
    }
    if (!NetFirewallBaseRule::UnmarshallingBase(parcel, ptr)) {
        return nullptr;
    }
    int32_t ruleDirection = 0;
    if (!parcel.ReadInt32(ruleDirection)) {
        return nullptr;
    }
    if (ruleDirection < static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN) ||
        ruleDirection > static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT)) {
        NETMGR_LOG_E("Invalid ruleDirection: %{public}d", ruleDirection);
        return nullptr;
    }
    ptr->ruleDirection = static_cast<NetFirewallRuleDirection>(ruleDirection);
    int32_t ruleAction = 0;
    if (!parcel.ReadInt32(ruleAction)) {
        return nullptr;
    }
    if (ruleAction <= static_cast<int32_t>(FirewallRuleAction::RULE_INVALID) ||
        ruleAction > static_cast<int32_t>(FirewallRuleAction::RULE_DENY)) {
        NETMGR_LOG_E("Invalid ruleAction: %{public}d", ruleAction);
        return nullptr;
    }
    ptr->ruleAction = static_cast<FirewallRuleAction>(ruleAction);
    int32_t protocol = 0;
    if (!parcel.ReadInt32(protocol)) {
        return nullptr;
    }
    ptr->protocol = static_cast<NetworkProtocol>(protocol);
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->localIps)) {
        return nullptr;
    }
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->remoteIps)) {
        return nullptr;
    }
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->localPorts)) {
        return nullptr;
    }
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->remotePorts)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->interface)) {
        return nullptr;
    }
    return ptr;
}

// domain rule data
bool NetFirewallDomainRule::Marshalling(Parcel &parcel) const
{
    if (!NetFirewallBaseRule::Marshalling(parcel)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleAction))) {
        return false;
    }
    if (!NetFirewallUtils::MarshallingList(domains, parcel)) {
        return false;
    }
    return true;
}

sptr<NetFirewallDomainRule> NetFirewallDomainRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallDomainRule> ptr = new (std::nothrow) NetFirewallDomainRule();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallDomainRule ptr is null");
        return nullptr;
    }
    if (!NetFirewallBaseRule::UnmarshallingBase(parcel, ptr)) {
        return nullptr;
    }
    int32_t ruleAction = 0;
    if (!parcel.ReadInt32(ruleAction)) {
        return nullptr;
    }
    if (ruleAction <= static_cast<int32_t>(FirewallRuleAction::RULE_INVALID) ||
        ruleAction > static_cast<int32_t>(FirewallRuleAction::RULE_DENY)) {
        NETMGR_LOG_E("Invalid ruleAction: %{public}d", ruleAction);
        return nullptr;
    }
    ptr->ruleAction = static_cast<FirewallRuleAction>(ruleAction);
    if (!NetFirewallUtils::UnmarshallingList(parcel, ptr->domains)) {
        return nullptr;
    }
    return ptr;
}

// DNS rule data
bool NetFirewallDnsRule::Marshalling(Parcel &parcel) const
{
    if (!NetFirewallBaseRule::Marshalling(parcel)) {
        return false;
    }
    if (!parcel.WriteString(primaryDns)) {
        return false;
    }
    if (!parcel.WriteString(standbyDns)) {
        return false;
    }
    return true;
}

sptr<NetFirewallDnsRule> NetFirewallDnsRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallDnsRule> ptr = new (std::nothrow) NetFirewallDnsRule();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallDnsRule ptr is null");
        return nullptr;
    }
    if (!NetFirewallBaseRule::UnmarshallingBase(parcel, ptr)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->primaryDns)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->standbyDns)) {
        return nullptr;
    }
    return ptr;
}

// Interception Record
bool InterceptRecord::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(localPort)) {
        return false;
    }
    if (!parcel.WriteUint16(remotePort)) {
        return false;
    }
    if (!parcel.WriteUint16(protocol)) {
        return false;
    }
    if (!parcel.WriteUint64(time)) {
        return false;
    }
    if (!parcel.WriteString(localIp)) {
        return false;
    }
    if (!parcel.WriteString(remoteIp)) {
        return false;
    }
    if (!parcel.WriteInt32(appUid)) {
        return false;
    }
    if (!parcel.WriteString(domain)) {
        return false;
    }
    return true;
}

sptr<InterceptRecord> InterceptRecord::Unmarshalling(Parcel &parcel)
{
    sptr<InterceptRecord> ptr = new (std::nothrow) InterceptRecord();
    if (ptr == nullptr) {
        NETMGR_LOG_E("InterceptRecord ptr is null");
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->localPort)) {
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->remotePort)) {
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->protocol)) {
        return nullptr;
    }
    if (!parcel.ReadUint64(ptr->time)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->localIp)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->remoteIp)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->appUid)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->domain)) {
        return nullptr;
    }
    return ptr;
}

bool NfqCtx::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(seq)) {
        return false;
    }
    uint32_t count = 0;
    for (uint32_t i = 0; i < NFQ_MAX_QUEUES; i++) {
        if (queues[i] != nullptr) {
            count++;
        }
    }
    if (!parcel.WriteUint32(count)) {
        return false;
    }
    for (uint32_t i = 0; i < NFQ_MAX_QUEUES; i++) {
        if (queues[i] != nullptr) {
            if (!parcel.WriteUint32(i)) {
                return false;
            }
            if (!queues[i]->Marshalling(parcel)) {
                return false;
            }
        }
    }
    if (!parcel.WriteUint32(ctxId)) {
        return false;
    }
    return true;
}

sptr<NfqCtx> NfqCtx::Unmarshalling(Parcel &parcel)
{
    sptr<NfqCtx> ctx = new (std::nothrow) NfqCtx();
    if (ctx == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadUint32(ctx->seq)) {
        return nullptr;
    }
    uint32_t count = 0;
    if (!parcel.ReadUint32(count)) {
        return nullptr;
    }
    for (uint32_t i = 0; i < count; i++) {
        uint32_t index = 0;
        if (!parcel.ReadUint32(index)) {
            return nullptr;
        }
        if (index >= NFQ_MAX_QUEUES) {
            return nullptr;
        }
        ctx->queues[index] = NfqQueue::Unmarshalling(parcel);
        if (ctx->queues[index] == nullptr) {
            return nullptr;
        }
    }
    if (!parcel.ReadUint32(ctx->ctxId)) {
        return nullptr;
    }
    return ctx;
}

bool NfqQueue::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(queueNum)) {
        return false;
    }
    return true;
}

sptr<NfqQueue> NfqQueue::Unmarshalling(Parcel &parcel)
{
    sptr<NfqQueue> q = new (std::nothrow) NfqQueue();
    if (q == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadUint16(q->queueNum)) {
        return nullptr;
    }
    return q;
}
} // namespace NetManagerStandard
} // namespace OHOS