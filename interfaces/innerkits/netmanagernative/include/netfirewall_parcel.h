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

#ifndef NET_FIREWALL_PARCEL_H
#define NET_FIREWALL_PARCEL_H

#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
// Intercept only one record per minute, with a buffer time of 60 seconds
constexpr const int32_t INTERCEPT_BUFF_INTERVAL_SEC = 60;
// Maximum number of rules per user
constexpr int32_t FIREWALL_RULE_SIZE_MAX = 1000;
// Maximum number of domain for all users
constexpr int32_t FIREWALL_DOMAIN_RULE_SIZE_MAX = 20000;
constexpr int32_t FIREWALL_IPC_IP_RULE_PAGE_SIZE = 150;
constexpr int32_t FIREWALL_IPC_DOMAIN_RULE_PAGE_SIZE = 6000;

constexpr const char *COMMA = ",";
constexpr const char *NET_FIREWALL_IS_OPEN = "isOpen";
constexpr const char *NET_FIREWALL_IN_ACTION = "inAction";
constexpr const char *NET_FIREWALL_OUT_ACTION = "outAction";

namespace {
const std::string NET_FIREWALL_RULE_ID = "id";
const std::string NET_FIREWALL_RULE_NAME = "name";
const std::string NET_FIREWALL_RULE_DESC = "description";
const std::string NET_FIREWALL_RULE_DIR = "direction";
const std::string NET_FIREWALL_RULE_ACTION = "action";
const std::string NET_FIREWALL_RULE_TYPE = "type";
const std::string NET_FIREWALL_IS_ENABLED = "isEnabled";
const std::string NET_FIREWALL_APP_ID = "appUid";
const std::string NET_FIREWALL_LOCAL_IP = "localIps";
const std::string NET_FIREWALL_REMOTE_IP = "remoteIps";
const std::string NET_FIREWALL_PROTOCOL = "protocol";
const std::string NET_FIREWALL_LOCAL_PORT = "localPorts";
const std::string NET_FIREWALL_REMOTE_PORT = "remotePorts";
const std::string NET_FIREWALL_RULE_DOMAIN = "domains";
const std::string NET_FIREWALL_DNS = "dns";
const std::string NET_FIREWALL_USER_ID = "userId";
const std::string NET_FIREWALL_IP_FAMILY = "family";
const std::string NET_FIREWALL_IP_TYPE = "type";
const std::string NET_FIREWALL_IP_ADDRESS = "address";
const std::string NET_FIREWALL_IP_MASK = "mask";
const std::string NET_FIREWALL_IP_START = "startIp";
const std::string NET_FIREWALL_IP_END = "endIp";
const std::string NET_FIREWALL_PORT_START = "startPort";
const std::string NET_FIREWALL_PORT_END = "endPort";
const std::string NET_FIREWALL_DOMAIN_IS_WILDCARD = "isWildcard";
const std::string NET_FIREWALL_DOMAIN = "domain";
const std::string NET_FIREWALL_DNS_PRIMARY = "primaryDns";
const std::string NET_FIREWALL_DNS_STANDY = "standbyDns";
const std::string NET_FIREWALL_RECORD_TIME = "time";
const std::string NET_FIREWALL_RECORD_LOCAL_IP = "localIp";
const std::string NET_FIREWALL_RECORD_REMOTE_IP = "remoteIp";
const std::string NET_FIREWALL_RECORD_LOCAL_PORT = "localPort";
const std::string NET_FIREWALL_RECORD_REMOTE_PORT = "remotePort";
const std::string NET_FIREWALL_RECORD_PROTOCOL = "protocol";
const std::string NET_FIREWALL_RECORD_UID = "appUid";

const std::string EQUAL = "=";
} // namespace

// Firewall rule direction enumeration
enum class NetFirewallRuleDirection {
    RULE_IN = 1, // Inbound
    RULE_OUT     // Outbound
};

// Firewall rule behavior enumeration
enum class FirewallRuleAction {
    RULE_INVALID = -1,
    RULE_ALLOW = 0, // allow
    RULE_DENY       // deny
};

// Firewall Rule Type
enum class NetFirewallRuleType {
    RULE_INVALID = -1, // TYPE INVALID
    RULE_IP = 1,       // TYPE IP
    RULE_DOMAIN,       // TYPE Domain
    RULE_DNS,          // TYPE DNS
    RULE_ALL           // TYPE ALL
};

// Network protocol, currently only supports the following enumeration. Please refer to the enumeration data for
// details: https://learn.microsoft.com/en-us/graph/api/resources/securitynetworkprotocol?view=graph-rest-1.0
enum class NetworkProtocol {
    ICMP = 1,       // Internet Control Message Protocol.
    TCP = 6,        // Transmission Control Protocol.
    UDP = 17,       // User Datagram Protocol.
    ICMPV6 = 58,    // Internet Control Message Protocol for ipv6.
    GRE = 47,       // General Routing Encapsulation
    IPSEC_ESP = 50, // Encap Security Payload [RFC2406]
    IPSEC_AH = 51,  // Authentication Header [RFC2402]
    L2TP = 115,     // Layer Two Tunneling Protocol [RFC2661]
};

// Firewall IP parameters
struct NetFirewallIpParam : public Parcelable {
    int32_t family;      // IPv4=1, IPv6=2, default IPv4, not currently supported for others, optional
    int32_t type;        // 1：IP address or subnet, when using a single IP, the mask is 32,2: IP segment. Optional
    std::string address; // IP address, optional
    int32_t mask;        // IPv4: subnet mask, IPv6: prefix. Optional
    std::string startIp; // Starting IP, optional
    std::string endIp;   // End IP, optional

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetFirewallIpParam> Unmarshalling(Parcel &parcel);
    std::string ToString() const;
};

// Firewall port parameters
struct NetFirewallPortParam : public Parcelable {
    int32_t startPort; // When there is only one port, the starting port is the same as the ending port. Optional
    int32_t endPort;   // When there is only one end port, the start port is the same as the end port. Optional

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetFirewallPortParam> Unmarshalling(Parcel &parcel);
    std::string ToString() const;
};

// Firewall domain name parameters
struct NetFirewallDomainParam : public Parcelable {
    bool isWildcard;    // Is there a universal configuration rule? It is mandatory
    std::string domain; // Domain, mandatory

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetFirewallDomainParam> Unmarshalling(Parcel &parcel);
    std::string ToString() const;
};

// Firewall DNS parameters
struct NetFirewallDnsParam : public Parcelable {
    std::string primaryDns; // Primary DNS, mandatory
    std::string standbyDns; // Backup DNS, optional

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetFirewallDnsParam> Unmarshalling(Parcel &parcel);
};

struct NetFirewallDomainRule : public Parcelable {
    int32_t userId;
    int32_t ruleId;
    int32_t appUid;
    FirewallRuleAction ruleAction;
    std::string domain;
    bool isWildcard;

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetFirewallDomainRule> Unmarshalling(Parcel &parcel);
    std::string ToString() const;
};

struct NetFirewallIpRule : public Parcelable {
    int32_t userId;
    int32_t ruleId;
    int32_t appUid;
    NetFirewallRuleDirection ruleDirection;
    FirewallRuleAction ruleAction;
    NetworkProtocol protocol;
    std::vector<NetFirewallIpParam> localIps;
    std::vector<NetFirewallIpParam> remoteIps;
    std::vector<NetFirewallPortParam> localPorts;
    std::vector<NetFirewallPortParam> remotePorts;

    static sptr<NetFirewallIpRule> Unmarshalling(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    std::string ToString() const;
};

struct NetFirewallDnsRule : public Parcelable {
    int32_t userId;
    int32_t appUid;
    std::string primaryDns;
    std::string standbyDns;

    static sptr<NetFirewallDnsRule> Unmarshalling(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
};

// Firewall rules, external interfaces
struct NetFirewallRule : public Parcelable {
    int32_t ruleId;                                // Rule ID, optional
    std::string ruleName;                          // Rule name, mandatory
    std::string ruleDescription;                   // Rule description, optional
    NetFirewallRuleDirection ruleDirection;        // Rule direction, inbound or outbound, mandatory
    FirewallRuleAction ruleAction;                 // Behavior rules, mandatory
    NetFirewallRuleType ruleType;                  // Rule type, mandatory
    bool isEnabled;                                // Enable or not, required
    int32_t appUid;                                // Application or service ID, optional
    std::vector<NetFirewallIpParam> localIps;      // Local IP address, optional
    std::vector<NetFirewallIpParam> remoteIps;     // Remote IP address, optional
    NetworkProtocol protocol;                      // Protocol, TCP: 6, UDP: 17. Optional
    std::vector<NetFirewallPortParam> localPorts;  // Local port, optional
    std::vector<NetFirewallPortParam> remotePorts; // Remote port, optional
    std::vector<NetFirewallDomainParam> domains;   // Domain name list, optional
    NetFirewallDnsParam dns;                       // DNS, optional
    int32_t userId;                                // User ID, mandatory

    static sptr<NetFirewallRule> Unmarshalling(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    std::string ToString() const;
};

// Interception Record
struct InterceptRecord : public Parcelable {
    int32_t time;         // time stamp
    std::string localIp;  // Local IP
    std::string remoteIp; // Remote IP
    int32_t localPort;    // Local Port
    int32_t remotePort;   // Destination Port
    int32_t protocol;     // Transport Layer Protocol
    int32_t appUid;       // Application or Service ID
    std::string domain;   // domain name

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<InterceptRecord> Unmarshalling(Parcel &parcel);
    std::string ToString() const;
};

class NetFirewallUtils {
public:
    NetFirewallUtils() = default;
    ~NetFirewallUtils() = default;
    NetFirewallUtils(const NetFirewallUtils &) = delete;
    NetFirewallUtils &operator = (const NetFirewallUtils &) = delete;
    // String segmentation
    static std::vector<std::string> split(const std::string &text, char delim = ',');
    // Delete substring to obtain the remaining strings after deletion
    static std::string erase(const std::string &src, const std::string &sub);

    // Serialization&Deserialization List
    template <typename T> static bool MarshallingList(const std::vector<T> &list, Parcel &parcel);
    template <typename T> static bool UnmarshallingList(Parcel &parcel, std::vector<T> &list);
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NET_FIREWALL_PARCEL_H