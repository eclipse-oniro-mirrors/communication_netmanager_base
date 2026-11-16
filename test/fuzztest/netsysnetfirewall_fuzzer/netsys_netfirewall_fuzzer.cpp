/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <securec.h>
#include <thread>

#include "netsys_netfirewall_fuzzer.h"
#include "notify_callback_stub.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "netsys_native_service.h"

/* Fuzz the real tri-state parser against fuzz-built packets. Only
 * netfirewall_l4_parse.h is included (not netfirewall_utils.h): the parser
 * header is byte-load-only and free of the kernel UAPI structs that would
 * clash with the musl netinet structs (netinet/ip.h, tcp.h, ip_icmp.h)
 * already in this TU's chain.
 * struct __sk_buff's user-space mirror stores only a 32-bit data offset (real
 * BPF contexts rewrite it to the actual packet pointer), so the fuzz buffer is
 * published through a side global consumed by the stub below.
 * bpf_skb_load_bytes is not declared by the parser header (the BPF build gets
 * it from bpf_helpers.h), so the mock is declared first and defined at global
 * scope with kernel-identical bounds semantics. */
static const uint8_t *g_parseL4FuzzData = nullptr;
static uint32_t g_parseL4FuzzLen = 0;
long bpf_skb_load_bytes(const struct __sk_buff *skb, __u32 offset, void *to, __u32 len);
#include "netfirewall/netfirewall_l4_parse.h"

long bpf_skb_load_bytes(const struct __sk_buff *skb, __u32 offset, void *to, __u32 len)
{
    if (skb == NULL || to == NULL || g_parseL4FuzzData == nullptr) {
        return -1;
    }
    if (static_cast<uint64_t>(offset) + len > g_parseL4FuzzLen) {
        return -1;
    }
    if (memcpy(to, g_parseL4FuzzData + offset, len) == NULL) {
        return -1;
    }
    return 0;
}

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
constexpr uint32_t MAX_RULES = 15;
constexpr int32_t NUMBER_TWO = 2;
constexpr int32_t NUMBER_ONE = 1;
bool g_isWaitAsync = false;
} // namespace

template <class T> T NetFireWallGetData()
{
    T object{};
    size_t netFireWallSize = sizeof(object);
    if (g_baseFuzzData == nullptr || netFireWallSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, netFireWallSize, g_baseFuzzData + g_baseFuzzPos, netFireWallSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += netFireWallSize;
    return object;
}

std::string NetFireWallGetString(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = NetFireWallGetData<char>();
    }
    std::string str(cstr);
    return str;
}

static bool g_isInited = false;
__attribute__((no_sanitize("cfi"))) void Init()
{
    nmd::IptablesWrapper::GetInstance();
    g_isInited = DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->Init();
}

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;

    return DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->OnRemoteRequest(code, data, reply,
                                                                                               option);
}

bool WriteInterfaceToken(MessageParcel &data)
{
    return data.WriteInterfaceToken(NetsysNative::NetsysNativeServiceStub::GetDescriptor());
}

bool WriteInterfaceTokenCallback(MessageParcel &data)
{
    return data.WriteInterfaceToken(NetsysNative::NotifyCallbackStub::GetDescriptor());
}

bool IsDataAndSizeValid(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    return WriteInterfaceToken(dataParcel);
}

#ifdef FEATURE_NET_FIREWALL_ENABLE
void InterfaceBitmapBuild001FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("lo");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void InterfaceBitmapBuild002FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void InterfaceBitmapBuild003FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 2;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    for (int i = 0; i < numRules; i++) {
        dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
        dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
        dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
        dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
        dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
        dataParcel.WriteUint32(0);
        dataParcel.WriteUint32(0);
        dataParcel.WriteUint32(0);
        dataParcel.WriteUint32(0);
        dataParcel.WriteString("lo");
    }

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void InterfaceBitmapBuild004FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 2;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("wlan0");

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("eth0");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void InterfaceBitmapBuild005FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 2;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("wlan0");

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("eth0");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void InterfaceBitmapBuild006FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 2;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("wlan0");

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void InterfaceBitmapBuild007FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("lo");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);

    MessageParcel dataParcel2;
    if (!WriteInterfaceToken(dataParcel2)) {
        return;
    }
    int32_t clearType = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    dataParcel2.WriteInt32(clearType);
    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_CLEAR_RULES),
        dataParcel2);
}

/* Wildcard-sentinel entry: unlike InterfaceBitmapBuild001-007, this entry fills
 * non-empty localIps/remoteIps/localPorts/remotePorts and a fuzz-driven protocol,
 * so corpus seeds carrying extreme values (port 65535, 255.255.255.255/32,
 * 255.255.255.0/24, protocol 255, uid 0xFFFFFFFF) actually reach the bitmap
 * build path instead of dying on empty lists. The parcel is produced by the real
 * NetFirewallIpRule::Marshalling to keep the layout exact. */
void InterfaceBitmapExtremeParamsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    NetFirewallIpRule rule;
    rule.userId = NetFireWallGetData<int32_t>();
    rule.appUid = NetFireWallGetData<int32_t>();
    rule.ruleDirection = static_cast<NetFirewallRuleDirection>(NetFireWallGetData<int32_t>());
    rule.ruleAction = static_cast<FirewallRuleAction>(NetFireWallGetData<int32_t>());
    rule.protocol = static_cast<NetworkProtocol>(NetFireWallGetData<int32_t>());

    NetFirewallIpParam localIp;
    localIp.family = FAMILY_IPV4;
    localIp.type = NetFireWallGetData<uint8_t>();
    localIp.mask = NetFireWallGetData<uint8_t>();
    localIp.ipv4.startIp.s_addr = NetFireWallGetData<uint32_t>();
    if (localIp.type == MULTIPLE_IP) {
        localIp.ipv4.endIp.s_addr = NetFireWallGetData<uint32_t>();
    }
    rule.localIps.push_back(localIp);

    NetFirewallIpParam remoteIp;
    remoteIp.family = FAMILY_IPV4;
    remoteIp.type = NetFireWallGetData<uint8_t>();
    remoteIp.mask = NetFireWallGetData<uint8_t>();
    remoteIp.ipv4.startIp.s_addr = NetFireWallGetData<uint32_t>();
    if (remoteIp.type == MULTIPLE_IP) {
        remoteIp.ipv4.endIp.s_addr = NetFireWallGetData<uint32_t>();
    }
    rule.remoteIps.push_back(remoteIp);

    NetFirewallPortParam localPort;
    localPort.startPort = NetFireWallGetData<uint16_t>();
    localPort.endPort = NetFireWallGetData<uint16_t>();
    rule.localPorts.push_back(localPort);

    NetFirewallPortParam remotePort;
    remotePort.startPort = NetFireWallGetData<uint16_t>();
    remotePort.endPort = NetFireWallGetData<uint16_t>();
    rule.remotePorts.push_back(remotePort);

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    if (!rule.Marshalling(dataParcel)) {
        return;
    }

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void NetsysNetFirewallTest007FuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("lo");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void WriteInterfaceBpfMapEmptyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 0;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void WriteInterfaceBpfMapLongNameFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("abcdefghijklmnopq");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void WriteInterfaceBpfMapEgressFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("eth0");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void InterfaceRebuildFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = false;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);

    MessageParcel dataParcel2;
    if (!WriteInterfaceToken(dataParcel2)) {
        return;
    }
    type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    numRules = 1;
    isFinish = true;

    dataParcel2.WriteInt32(type);
    dataParcel2.WriteUint32(numRules);
    dataParcel2.WriteBool(isFinish);
    dataParcel2.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel2.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel2.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel2.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel2.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel2.WriteUint32(0);
    dataParcel2.WriteUint32(0);
    dataParcel2.WriteUint32(0);
    dataParcel2.WriteUint32(0);
    dataParcel2.WriteString("lo");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel2);
}

void WriteInterfaceBpfMapIngressFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 2;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("lo");

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("eth0");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void BuildMarkBitmapAllowWithInterfaceFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_ALLOW));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("wlan0");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void BuildNoMarkBitmapMixedInterfaceFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 3;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("eth0");

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("");

    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_OUT));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("wlan0");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

void HandleDebugEventFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    DebugEvent debugEv = {};
    debugEv.type = (enum debug_type)NetFireWallGetData<uint32_t>();
    debugEv.dir = (enum stream_dir)NetFireWallGetData<uint32_t>();
    debugEv.arg1 = NetFireWallGetData<uint32_t>();
    debugEv.arg2 = NetFireWallGetData<uint32_t>();
    NetsysBpfNetFirewall::HandleDebugEvent(&debugEv);
}

/* Mode switch entry. The fuzz input is constrained to the invalid domain
 * (bit 1 forced on, so the value is always >= 2): valid modes 0/1 would really
 * switch a live firewall-enabled device to the OBSERVE escape hatch. */
void SetEvalFallbackModeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t mode = NetFireWallGetData<uint32_t>() | 2u;
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    (void)bpfNet->SetEvalFallbackMode(mode);
}

void ClearBpfFirewallRulesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    dataParcel.WriteInt32(type);

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_CLEAR_RULES),
        dataParcel);
}

/* Rule-clear paths including the new conntrack-invalidation
 * variants (RULE_DOMAIN/RULE_DNS/RULE_ALL) must run without crash. */
void ClearBpfFirewallRulesTypedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFireWallGetData<uint32_t>());
    dataParcel.WriteInt32(type);

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_CLEAR_RULES),
        dataParcel);
}

/* Default-action entries including the reserved
 * userId keys (1 = system, -1 = unattributed) must be written without crash. */
void SetFirewallDefaultActionFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t userId = static_cast<int32_t>(NetFireWallGetData<uint32_t>());
    int32_t inDefault = static_cast<int32_t>(NetFireWallGetData<uint32_t>());
    int32_t outDefault = static_cast<int32_t>(NetFireWallGetData<uint32_t>());
    dataParcel.WriteInt32(userId);
    dataParcel.WriteInt32(inDefault);
    dataParcel.WriteInt32(outDefault);

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_DEFAULT_ACTION),
        dataParcel);
}

/* Fuzz the unified tri-state L4 parser with fuzz-built packets. The
 * contract under fuzz is: never crash, never read out of bounds (the stub
 * enforces skb->len like the kernel helper), and the verdict is always one of
 * the three states. byte 0 selects the family (0 = AF_UNSPEC corner, odd =
 * AF_INET6, even = AF_INET); the rest is the raw packet starting at L3. */
void ParseL4FuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < 2) {
        return;
    }
    struct __sk_buff skb = {};
    uint8_t selector = data[0];
    skb.family = (selector == 0) ? 0 : ((selector & 1) ? AF_INET6 : AF_INET);
    g_parseL4FuzzData = data + 1;
    g_parseL4FuzzLen = static_cast<uint32_t>(size - 1);
    skb.len = static_cast<uint32_t>(size - 1);

    struct l4_info l4 = {};
    (void)parse_l4(skb.family, &skb, &l4);
    if (l4.state != L4_PARSE_OK && l4.state != L4_PARSE_NO_L4 && l4.state != L4_PARSE_UNPARSEABLE) {
        return; /* unreachable: keeps static analyzers calm */
    }
}

void WriteInterfaceBpfMapSuccessFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t type = static_cast<int32_t>(NetFirewallRuleType::RULE_IP);
    uint32_t numRules = 1;
    bool isFinish = true;

    dataParcel.WriteInt32(type);
    dataParcel.WriteUint32(numRules);
    dataParcel.WriteBool(isFinish);
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(NetFireWallGetData<int32_t>());
    dataParcel.WriteInt32(static_cast<int32_t>(NetFirewallRuleDirection::RULE_IN));
    dataParcel.WriteInt32(static_cast<int32_t>(FirewallRuleAction::RULE_DENY));
    dataParcel.WriteInt32(static_cast<int32_t>(NetworkProtocol::TCP));
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteUint32(0);
    dataParcel.WriteString("lo");

    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NET_FIREWALL_SET_RULES),
        dataParcel);
}

/* DnsServerMapFuzzTest（进程内 DNS server map 控制面烟雾入口）已删除
 * （2026-11-14 用户裁决）：NetsysBpfNetFirewall 为进程内类、无新增 IPC 命令，
 * 控制面输入无外部直达路径，不构成攻击面；畸形 server 字符串处理已由
 * BuildDnsServerKeys 单测覆盖，ffrt 并发面由 DnsPrewarmTriggerCoalescing 等
 * 确定性单测覆盖。网络侧报文字节攻击面保留在 netsys_dns_fuzz.cpp
 * （DnsSniffFuzzTest，伪造应答/猜 txid/TCP 偏移类种子）。 */
#endif // FEATURE_NET_FIREWALL_ENABLE
} // namespace NetManagerStandard
} // namespace OHOS

/* Real BPF DNS sniffing entry lives in netsys_dns_fuzz.cpp (separate TU —
 * that file's comment explains the include-chain separation). */
namespace OHOS {
namespace NetManagerStandard {
void DnsSniffFuzzTest(const uint8_t *data, size_t size);
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
#ifdef FEATURE_NET_FIREWALL_ENABLE
    /* Real BPF DNS sniffing chain (separate TU — see
     * netsys_dns_fuzz.cpp for why it cannot live in this one). */
    OHOS::NetManagerStandard::DnsSniffFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapBuild001FuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapBuild002FuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapBuild003FuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapBuild004FuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapBuild005FuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapBuild006FuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapBuild007FuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceBitmapExtremeParamsFuzzTest(data, size);
    OHOS::NetManagerStandard::NetsysNetFirewallTest007FuzzTest(data, size);
    OHOS::NetManagerStandard::WriteInterfaceBpfMapEmptyFuzzTest(data, size);
    OHOS::NetManagerStandard::WriteInterfaceBpfMapLongNameFuzzTest(data, size);
    OHOS::NetManagerStandard::WriteInterfaceBpfMapEgressFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceRebuildFuzzTest(data, size);
    OHOS::NetManagerStandard::WriteInterfaceBpfMapIngressFuzzTest(data, size);
    OHOS::NetManagerStandard::BuildMarkBitmapAllowWithInterfaceFuzzTest(data, size);
    OHOS::NetManagerStandard::BuildNoMarkBitmapMixedInterfaceFuzzTest(data, size);
    OHOS::NetManagerStandard::HandleDebugEventFuzzTest(data, size);
    OHOS::NetManagerStandard::SetEvalFallbackModeFuzzTest(data, size);
    OHOS::NetManagerStandard::ClearBpfFirewallRulesFuzzTest(data, size);
    OHOS::NetManagerStandard::ClearBpfFirewallRulesTypedFuzzTest(data, size);
    OHOS::NetManagerStandard::SetFirewallDefaultActionFuzzTest(data, size);
    OHOS::NetManagerStandard::ParseL4FuzzTest(data, size);
    OHOS::NetManagerStandard::WriteInterfaceBpfMapSuccessFuzzTest(data, size);
#endif
    return 0;
}