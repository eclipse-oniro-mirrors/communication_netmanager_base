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
#include <sstream>

#include "netfirewall_parcel.h"
#include "net_mgr_log_wrapper.h"
#include "refbase.h"


namespace OHOS {
namespace NetManagerStandard {
// Firewall IP parameters
bool NetFirewallIpParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteUint8(family);
    if (!parcel.WriteUint8(type)) {
        return false;
    }
    parcel.WriteUint8(mask);
    parcel.WriteString(address);
    parcel.WriteString(startIp);
    parcel.WriteString(endIp);
    return true;
}

sptr<NetFirewallIpParam> NetFirewallIpParam::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallIpParam> ptr = new (std::nothrow) NetFirewallIpParam();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallIpParam ptr is null");
        return nullptr;
    }
    parcel.ReadUint8(ptr->family);
    if (!parcel.ReadUint8(ptr->type)) {
        return nullptr;
    }
    parcel.ReadUint8(ptr->mask);
    parcel.ReadString(ptr->address);
    parcel.ReadString(ptr->startIp);
    parcel.ReadString(ptr->endIp);
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

bool NetFirewallDomainRule::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(userId);
    parcel.WriteInt32(ruleId);
    parcel.WriteInt32(appUid);
    if (!parcel.WriteBool(isWildcard)) {
        return false;
    }
    if (!parcel.WriteString(domain)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleAction))) {
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
    parcel.ReadInt32(ptr->userId);
    parcel.ReadInt32(ptr->ruleId);
    parcel.ReadInt32(ptr->appUid);
    if (!parcel.ReadBool(ptr->isWildcard)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->domain)) {
        return nullptr;
    }
    int32_t ruleAction = 0;
    if (!parcel.ReadInt32(ruleAction)) {
        return nullptr;
    }
    ptr->ruleAction = static_cast<FirewallRuleAction>(ruleAction);
    return ptr;
}

// Firewall DNS parameters
bool NetFirewallDnsParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(primaryDns)) {
        return false;
    }
    parcel.WriteString(standbyDns);
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
    parcel.ReadString(ptr->standbyDns);
    return ptr;
}

template <typename T> bool NetFirewallUtils::MarshallingList(const std::vector<T> &list, Parcel &parcel)
{
    uint32_t size = static_cast<uint32_t>(list.size());
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
    parcel.WriteInt32(ruleId);
    if (!parcel.WriteString(ruleName)) {
        return false;
    }
    parcel.WriteString(ruleDescription);
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleDirection))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleAction))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleType))) {
        return false;
    }
    parcel.WriteBool(isEnabled);
    parcel.WriteInt32(appUid);
    NetFirewallUtils::MarshallingList(localIps, parcel);
    NetFirewallUtils::MarshallingList(remoteIps, parcel);
    parcel.WriteInt32(static_cast<int32_t>(protocol));
    NetFirewallUtils::MarshallingList(localPorts, parcel);
    NetFirewallUtils::MarshallingList(remotePorts, parcel);
    NetFirewallUtils::MarshallingList(domains, parcel);
    dns.Marshalling(parcel);
    if (!parcel.WriteInt32(userId)) {
        return false;
    }
    return true;
}

sptr<NetFirewallRule> NetFirewallRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallRule> ptr = new (std::nothrow) NetFirewallRule();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallRule ptr is null");
        return nullptr;
    }
    parcel.ReadInt32(ptr->ruleId);

    if (!parcel.ReadString(ptr->ruleName)) {
        return nullptr;
    }
    parcel.ReadString(ptr->ruleDescription);
    int32_t ruleDirection = 0;
    if (!parcel.ReadInt32(ruleDirection)) {
        return nullptr;
    }
    ptr->ruleDirection = static_cast<NetFirewallRuleDirection>(ruleDirection);
    int32_t ruleAction = 0;
    if (!parcel.ReadInt32(ruleAction)) {
        return nullptr;
    }
    ptr->ruleAction = static_cast<FirewallRuleAction>(ruleAction);
    int32_t ruleType = 0;
    if (!parcel.ReadInt32(ruleType)) {
        return nullptr;
    }
    ptr->ruleType = static_cast<NetFirewallRuleType>(ruleType);
    parcel.ReadBool(ptr->isEnabled);
    parcel.ReadInt32(ptr->appUid);
    NetFirewallUtils::UnmarshallingList(parcel, ptr->localIps);
    NetFirewallUtils::UnmarshallingList(parcel, ptr->remoteIps);
    int32_t protocol = 0;
    if (parcel.ReadInt32(protocol)) {
        ptr->protocol = static_cast<NetworkProtocol>(protocol);
    }
    NetFirewallUtils::UnmarshallingList(parcel, ptr->localPorts);
    NetFirewallUtils::UnmarshallingList(parcel, ptr->remotePorts);
    NetFirewallUtils::UnmarshallingList(parcel, ptr->domains);
    sptr<NetFirewallDnsParam> dns = NetFirewallDnsParam::Unmarshalling(parcel);
    if (dns != nullptr) {
        ptr->dns = *dns;
    }
    if (!parcel.ReadInt32(ptr->userId)) {
        return nullptr;
    }
    return ptr;
}

// IP rule data
bool NetFirewallIpRule::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(userId);
    parcel.WriteInt32(ruleId);
    parcel.WriteInt32(appUid);
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleDirection))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(ruleAction))) {
        return false;
    }
    parcel.WriteInt32(static_cast<int32_t>(protocol));
    NetFirewallUtils::MarshallingList(localIps, parcel);
    NetFirewallUtils::MarshallingList(remoteIps, parcel);
    NetFirewallUtils::MarshallingList(localPorts, parcel);
    NetFirewallUtils::MarshallingList(remotePorts, parcel);
    return true;
}

sptr<NetFirewallIpRule> NetFirewallIpRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallIpRule> ptr = new (std::nothrow) NetFirewallIpRule();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallIpRule ptr is null");
        return nullptr;
    }
    parcel.ReadInt32(ptr->userId);
    parcel.ReadInt32(ptr->ruleId);
    parcel.ReadInt32(ptr->appUid);
    int32_t ruleDirection = 0;
    if (!parcel.ReadInt32(ruleDirection)) {
        return nullptr;
    }
    ptr->ruleDirection = static_cast<NetFirewallRuleDirection>(ruleDirection);
    int32_t ruleAction = 0;
    if (!parcel.ReadInt32(ruleAction)) {
        return nullptr;
    }
    ptr->ruleAction = static_cast<FirewallRuleAction>(ruleAction);
    int32_t protocol = 0;
    if (parcel.ReadInt32(protocol)) {
        ptr->protocol = static_cast<NetworkProtocol>(protocol);
    }
    NetFirewallUtils::UnmarshallingList(parcel, ptr->localIps);
    NetFirewallUtils::UnmarshallingList(parcel, ptr->remoteIps);
    NetFirewallUtils::UnmarshallingList(parcel, ptr->localPorts);
    NetFirewallUtils::UnmarshallingList(parcel, ptr->remotePorts);
    return ptr;
}

// DNS rule data
bool NetFirewallDnsRule::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(userId);
    parcel.WriteInt32(appUid);
    if (!parcel.WriteString(primaryDns)) {
        return false;
    }
    parcel.WriteString(standbyDns);
    return true;
}

sptr<NetFirewallDnsRule> NetFirewallDnsRule::Unmarshalling(Parcel &parcel)
{
    sptr<NetFirewallDnsRule> ptr = new (std::nothrow) NetFirewallDnsRule();
    if (ptr == nullptr) {
        NETMGR_LOG_E("NetFirewallDnsRule ptr is null");
        return nullptr;
    }
    parcel.ReadInt32(ptr->userId);
    parcel.ReadInt32(ptr->appUid);
    if (!parcel.ReadString(ptr->primaryDns)) {
        return nullptr;
    }
    parcel.ReadString(ptr->standbyDns);
    return ptr;
}

// Interception Record
bool InterceptRecord::Marshalling(Parcel &parcel) const
{
    parcel.WriteUint16(localPort);
    parcel.WriteUint16(remotePort);
    parcel.WriteUint16(protocol);
    if (!parcel.WriteInt32(time)) {
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
    parcel.ReadUint16(ptr->localPort);
    parcel.ReadUint16(ptr->remotePort);
    parcel.ReadUint16(ptr->protocol);
    if (!parcel.ReadInt32(ptr->time)) {
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
} // namespace NetManagerStandard
} // namespace OHOS