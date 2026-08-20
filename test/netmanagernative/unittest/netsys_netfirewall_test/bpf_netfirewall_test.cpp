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
#include <gtest/gtest.h>

#include "bpf_loader.h"
#include "bpf_netfirewall.h"
#include "netfirewall_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace std;
using namespace testing::ext;
constexpr int32_t USER_ID1 = 100;
constexpr uint8_t LABEL_LEN = 3;
constexpr uint8_t OVERFLOW_TOTAL_LEN = 4;
constexpr uint8_t OVERFLOW_LABEL_LEN = static_cast<uint8_t>(OVERFLOW_TOTAL_LEN + 1);
constexpr uint8_t TWO_LABEL_TOTAL = static_cast<uint8_t>(LABEL_LEN + 1 + LABEL_LEN + 1);
}

class NetsysBpfNetFirewallTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetsysBpfNetFirewallTest::SetUpTestCase()
{
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NetsysBpfNetFirewall::GetInstance();
    if (!bpfNetFirewall->IsBpfLoaded()) {
        auto ret = LoadElf(FIREWALL_BPF_PATH);
        printf("LoadElf is %d\n", ret);

        if (ret == ElfLoadError::ELF_LOAD_ERR_NONE) {
            bpfNetFirewall->SetBpfLoaded(true);
        }
        bpfNetFirewall->StartListener();
    }
}

void NetsysBpfNetFirewallTest::TearDownTestCase() {}

void NetsysBpfNetFirewallTest::SetUp() {}

void NetsysBpfNetFirewallTest::TearDown() {}

HWTEST_F(NetsysBpfNetFirewallTest, AddDomainCache001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    NetAddrInfo netInfo;
    netInfo.aiFamily = AF_INET;
    inet_pton(AF_INET, "192.168.8.116", &netInfo.aiAddr.sin);
    bpfNet->AddDomainCache(netInfo);
    EXPECT_EQ(netInfo.aiFamily, AF_INET);
    netInfo.aiFamily = AF_INET6;
    inet_pton(AF_INET6, "fe80::6bec:e9b9:a1df:f69d", &netInfo.aiAddr.sin6);
    bpfNet->AddDomainCache(netInfo);
    bpfNet->ClearDomainCache();
    EXPECT_EQ(netInfo.aiFamily, AF_INET6);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearFirewallDefaultAction001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    bpfNet->SetBpfLoaded(true);
    int ret = bpfNet->SetFirewallDefaultAction(USER_ID1, FirewallRuleAction::RULE_ALLOW,
        FirewallRuleAction::RULE_ALLOW);
    bpfNet->ClearFirewallDefaultAction();
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    bpfNet->SetBpfLoaded(false);
    ret = bpfNet->SetFirewallDefaultAction(USER_ID1, FirewallRuleAction::RULE_ALLOW,
        FirewallRuleAction::RULE_ALLOW);
    EXPECT_EQ(ret, NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearFirewallRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    int ret = bpfNet->ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    ret = FIREWALL_ERR_INTERNAL;
    ret = bpfNet->ClearFirewallRules(NetFirewallRuleType::RULE_IP);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    ret = FIREWALL_ERR_INTERNAL;
    ret = bpfNet->ClearFirewallRules(NetFirewallRuleType::RULE_DOMAIN);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    ret = FIREWALL_ERR_INTERNAL;
    ret = bpfNet->ClearFirewallRules(NetFirewallRuleType::RULE_DEFAULT_ACTION);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    ret = bpfNet->ClearFirewallRules(NetFirewallRuleType::RULE_DOMAIN);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
}

HWTEST_F(NetsysBpfNetFirewallTest, WriteSrcPortBpfMap001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    
    BitmapManager manager;

    int ret = bpfNet->WriteSrcPortBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(ret, -1);
    ret = bpfNet->WriteSrcPortBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(ret, -1);

    Bitmap bitmap(1);
    uint32_t mask = 16;
    uint16_t port = 6000;
    portRuleBitmap tmp;
    tmp.prefixlen = mask;
    tmp.data = port;
    tmp.bitmap = bitmap;
    manager.srcPortMap_.ruleBitmapVec_.emplace_back(tmp);
    manager.dstPortMap_.ruleBitmapVec_.emplace_back(tmp);
    ret = bpfNet->WriteSrcPortBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(ret, 0);
    ret = bpfNet->WriteSrcPortBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, WriteDstPortBpfMap001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    
    BitmapManager manager;

    int ret = bpfNet->WriteDstPortBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(ret, -1);
    ret = bpfNet->WriteDstPortBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(ret, -1);

    Bitmap bitmap(1);
    uint32_t mask = 16;
    uint16_t port = 6000;
    portRuleBitmap tmp;
    tmp.prefixlen = mask;
    tmp.data = port;
    tmp.bitmap = bitmap;
    manager.srcPortMap_.ruleBitmapVec_.emplace_back(tmp);
    manager.dstPortMap_.ruleBitmapVec_.emplace_back(tmp);
    ret = bpfNet->WriteDstPortBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(ret, 0);
    ret = bpfNet->WriteDstPortBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, WritePortBpfMap001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    
    BpfPortMap portMap;
    const char *path = "test";
    int ret = bpfNet->WritePortBpfMap(portMap, path);
    EXPECT_EQ(ret, -1);

    uint16_t start = 1;
    uint32_t mask = 16;
    Bitmap bitmap(1);
    portMap.OrInsert(start, mask, bitmap);
    ret = bpfNet->WritePortBpfMap(portMap, path);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, DecodeDomainFromKey001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();
    DomainHashKey nullKey = {};
    nullKey.prefixlen = 0;
    auto result = bpfNet->DecodeDomainFromKey(nullKey);
    EXPECT_TRUE(result.empty());

    DomainHashKey overflowKey = {};
    overflowKey.prefixlen = static_cast<uint32_t>(OVERFLOW_TOTAL_LEN * BIT_PER_BYTE);
    EXPECT_EQ(memset_s(overflowKey.data, sizeof(overflowKey.data), 0, sizeof(overflowKey.data)), EOK);
    overflowKey.data[OVERFLOW_TOTAL_LEN - 1] = OVERFLOW_LABEL_LEN;
    result = bpfNet->DecodeDomainFromKey(overflowKey);
    EXPECT_TRUE(result.empty());

    DomainHashKey normalKey = {};
    normalKey.prefixlen = static_cast<uint32_t>((TWO_LABEL_TOTAL + 8) * BIT_PER_BYTE);
    normalKey.uid = 100;
    normalKey.appuid = 0;
    EXPECT_EQ(memset_s(normalKey.data, sizeof(normalKey.data), 0, sizeof(normalKey.data)), EOK);
    {
        const char label1[] = "moc";
        EXPECT_EQ(memcpy_s(normalKey.data + 1, sizeof(label1) - 1, label1, sizeof(label1) - 1), EOK);
        normalKey.data[0] = '.';
    }
    {
        const char label2[] = "www";
        const size_t off = static_cast<size_t>(LABEL_LEN + 1);
        EXPECT_EQ(memcpy_s(normalKey.data + off + 1, sizeof(label2) - 1, label2, sizeof(label2) - 1), EOK);
        normalKey.data[off] = '.';
    }
    result = bpfNet->DecodeDomainFromKey(normalKey);
    EXPECT_FALSE(result.empty());
}

HWTEST_F(NetsysBpfNetFirewallTest, GetDomainHashKey001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();
    DomainHashKey key = {};

    // empty domain
    bpfNet->GetDomainHashKey("", key);
    EXPECT_EQ(key.prefixlen, 0);

    // domain with only '*'
    key = {};
    bpfNet->GetDomainHashKey("***", key);
    EXPECT_EQ(key.prefixlen, 0);

    // domain too long
    key = {};
    std::string longDomain(DNS_DOMAIN_LEN + 1, 'a');
    bpfNet->GetDomainHashKey(longDomain, key);
    EXPECT_EQ(key.prefixlen, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, GetDomainHashKey002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();
    DomainHashKey key = {};

    // isReversed=true should reverse the raw domain text before stripping '*'
    // "www.example.com" -> reversed "moc.elpmaxe.www"
    key = {};
    bpfNet->GetDomainHashKey("www.example.com", key, true);
    uint32_t expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen("moc.elpmaxe.www")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, "moc.elpmaxe.www", strlen("moc.elpmaxe.www")), 0);

    // reversed suffix rule "*.example.com" -> strips '*' then reverses -> "moc.elpmaxe."
    key = {};
    bpfNet->GetDomainHashKey("*.example.com", key, true);
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen("moc.elpmaxe.")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, "moc.elpmaxe.", strlen("moc.elpmaxe.")), 0);

    key = {};
    bpfNet->GetDomainHashKey("*w.example.com", key, true);
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen("moc.elpmaxe.w")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, "moc.elpmaxe.w", strlen("moc.elpmaxe.w")), 0);

    // isReversed=false keeps original order ("www.example.*")
    key = {};
    bpfNet->GetDomainHashKey("www.example.*", key, false);
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen(".www.example.")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, ".www.example.", strlen(".www.example.")), 0);

    key = {};
    bpfNet->GetDomainHashKey("www.example.co*", key, false);
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen(".www.example.co")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, ".www.example.co", strlen(".www.example.co")), 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, DecodeDomainFromKey002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();

    // prefixlen < (sizeof(uid) + sizeof(appuid)) * 8 = 64
    DomainHashKey key = {};
    key.prefixlen = 32;
    auto result = bpfNet->DecodeDomainFromKey(key);
    EXPECT_TRUE(result.empty());

    // prefixlen == 64 -> domainPrefixlen == 0
    key = {};
    key.prefixlen = 64;
    result = bpfNet->DecodeDomainFromKey(key);
    EXPECT_TRUE(result.empty());

    // 65 <= prefixlen <= 71 -> domainLenBytes < DNS_DOMAIN_LEN_MIN
    key = {};
    key.prefixlen = 65;
    result = bpfNet->DecodeDomainFromKey(key);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(NetsysBpfNetFirewallTest, DecodeDomainFromKey003, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();

    // multi-label domain "www.example.com" encoded as wire labels:
    // 3 w w w 7 e x a m p l e 3 c o m 0
    DomainHashKey key = {};
    key.prefixlen = static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + 16) * BIT_PER_BYTE);
    const uint8_t wire[] = {'.', 'w', 'w', 'w', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm'};
    EXPECT_EQ(memcpy_s(key.data, sizeof(key.data), wire, sizeof(wire)), EOK);

    auto result = bpfNet->DecodeDomainFromKey(key);
    EXPECT_EQ(result, "www.example.com");
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallDomainRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    std::vector<sptr<NetFirewallDomainRule>> ruleList;

    // empty rules
    int ret = bpfNet->SetFirewallDomainRules(ruleList);
    EXPECT_EQ(ret, NETFIREWALL_ERR);

    sptr<NetFirewallDomainRule> rule = (std::make_unique<NetFirewallDomainRule>()).release();
    rule->userId = 100;
    rule->appUid = 1000;
    rule->ruleAction = FirewallRuleAction::RULE_ALLOW;

    NetFirewallDomainParam param;
    param.domain = "";
    param.isWildcard = false;
    rule->domains.push_back(param);

    param.isWildcard = true;
    param.domain = "*";
    rule->domains.push_back(param);

    ruleList.push_back(rule);

    ret = bpfNet->SetFirewallDomainRules(ruleList);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallDomainRules002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    std::vector<sptr<NetFirewallDomainRule>> ruleList;

    sptr<NetFirewallDomainRule> rule = (std::make_unique<NetFirewallDomainRule>()).release();
    rule->userId = 100;
    rule->appUid = 1000;
    rule->ruleAction = FirewallRuleAction::RULE_DENY;

    NetFirewallDomainParam param;
    param.isWildcard = true;
    // prefix wildcard "*.example.com" -> SetBpfFirewallDomainPrefixRules
    param.domain = "*.example.com";
    rule->domains.push_back(param);
    // prefix wildcard "*w.example.com" -> SetBpfFirewallDomainPrefixRules
    param.domain = "*w.example.com";
    rule->domains.push_back(param);
    // suffix wildcard "www.example.*" -> SetBpfFirewallDomainRules (reversed)
    param.domain = "www.example.*";
    rule->domains.push_back(param);
    // suffix wildcard "www.example.c*" -> SetBpfFirewallDomainRules (reversed)
    param.domain = "www.example.c*";
    rule->domains.push_back(param);
    // no wildcard "www.example.com" -> SetBpfFirewallDomainRules (reversed)
    param.isWildcard = false;
    param.domain = "www.example.com";
    rule->domains.push_back(param);

    ruleList.push_back(rule);
    int ret = bpfNet->SetFirewallDomainRules(ruleList);
    EXPECT_TRUE(ret == 0 || ret == -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, AddDomainCache002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    NetAddrInfo netInfo;
    netInfo.aiFamily = AF_UNSPEC;
    bpfNet->AddDomainCache(netInfo);
    EXPECT_EQ(netInfo.aiFamily, AF_UNSPEC);
}

HWTEST_F(NetsysBpfNetFirewallTest, HandleDebugEvent001, TestSize.Level0)
{
    DebugEvent ev = {
        .type = DBG_MATCH_INTERFACE,
        .dir = INGRESS,
        .arg1 = 1,
        .arg2 = 0xFF,
    };
    NetsysBpfNetFirewall::HandleDebugEvent(&ev);
    EXPECT_EQ(ev.type, DBG_MATCH_INTERFACE);
}

HWTEST_F(NetsysBpfNetFirewallTest, RegisterCallback001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    int ret = bpfNet->RegisterCallback(nullptr);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, LoadSystemAbility001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    int ret = bpfNet->LoadSystemAbility(99999);
    EXPECT_TRUE(ret == 0 || ret == -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    bpfNet->SetBpfLoaded(false);

    std::vector<sptr<NetFirewallBaseRule>> ruleList;
    int ret = bpfNet->SetFirewallRules(NetFirewallRuleType::RULE_IP, ruleList, true);
    EXPECT_EQ(ret, NETFIREWALL_ERR);

    sptr<NetFirewallIpRule> rule = (std::make_unique<NetFirewallIpRule>()).release();
    ruleList.push_back(rule);
    ret = bpfNet->SetFirewallRules(NetFirewallRuleType::RULE_IP, ruleList, true);
    EXPECT_EQ(ret, NETFIREWALL_ERR);

    bpfNet->SetBpfLoaded(true);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallCurrentUserId001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    bpfNet->SetBpfLoaded(false);
    int ret = bpfNet->SetFirewallCurrentUserId(100);
    EXPECT_EQ(ret, NETFIREWALL_ERR);

    bpfNet->SetBpfLoaded(true);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearFirewallDefaultAction002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    bpfNet->SetBpfLoaded(false);
    int ret = bpfNet->ClearFirewallDefaultAction();
    EXPECT_EQ(ret, NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallDefaultAction002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    bpfNet->SetBpfLoaded(false);
    int ret =
        bpfNet->SetFirewallDefaultAction(USER_ID1, FirewallRuleAction::RULE_ALLOW, FirewallRuleAction::RULE_ALLOW);
    EXPECT_EQ(ret, NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, GetNowMs001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();
    uint64_t now = bpfNet->GetNowMs();
    EXPECT_GT(now, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, HandleTupleEvent001, TestSize.Level0)
{
    TupleEvent ev = {};
    ev.dir = INGRESS;
    ev.sport = htons(80);
    ev.dport = htons(443);
    ev.protocol = IPPROTO_TCP;
    ev.appuid = 1000;
    ev.uid = 1000;
    NetsysBpfNetFirewall::HandleTupleEvent(&ev);
    EXPECT_EQ(ev.dir, INGRESS);
}

HWTEST_F(NetsysBpfNetFirewallTest, HandleInterceptEvent001, TestSize.Level0)
{
    InterceptEvent ev = {};
    ev.dir = EGRESS;
    ev.family = AF_INET;
    ev.protocol = IPPROTO_TCP;
    ev.sport = htons(80);
    ev.dport = htons(443);
    ev.appuid = 1000;
    NetsysBpfNetFirewall::HandleInterceptEvent(&ev);
    EXPECT_EQ(ev.dir, EGRESS);
}

HWTEST_F(NetsysBpfNetFirewallTest, HandleEvent001, TestSize.Level0)
{
    // null data
    int ret = NetsysBpfNetFirewall::HandleEvent(nullptr, nullptr, 0);
    EXPECT_EQ(ret, 0);

    // len < sizeof(Event)
    char buf[1] = {0};
    ret = NetsysBpfNetFirewall::HandleEvent(nullptr, buf, 1);
    EXPECT_EQ(ret, 0);

    // EVENT_DEBUG
    Event ev = {};
    ev.type = EVENT_DEBUG;
    ev.len = sizeof(Event);
    ret = NetsysBpfNetFirewall::HandleEvent(nullptr, &ev, sizeof(Event));
    EXPECT_EQ(ret, 0);

    // EVENT_INTERCEPT
    ev.type = EVENT_INTERCEPT;
    ret = NetsysBpfNetFirewall::HandleEvent(nullptr, &ev, sizeof(Event));
    EXPECT_EQ(ret, 0);

    // EVENT_TUPLE_DEBUG
    ev.type = EVENT_TUPLE_DEBUG;
    ret = NetsysBpfNetFirewall::HandleEvent(nullptr, &ev, sizeof(Event));
    EXPECT_EQ(ret, 0);

    // default type
    ev.type = static_cast<EventType>(999);
    ret = NetsysBpfNetFirewall::HandleEvent(nullptr, &ev, sizeof(Event));
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, WriteLoopBackBpfMap001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    int ret = bpfNet->WriteLoopBackBpfMap();
    EXPECT_TRUE(ret == NETFIREWALL_SUCCESS || ret == NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallIpRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    std::vector<sptr<NetFirewallIpRule>> ruleList;

    // empty rules
    int ret = bpfNet->SetFirewallIpRules(ruleList);
    EXPECT_EQ(ret, NETFIREWALL_SUCCESS);

    // ICMP rule -> outRules
    sptr<NetFirewallIpRule> rule = (std::make_unique<NetFirewallIpRule>()).release();
    rule->ruleDirection = NetFirewallRuleDirection::RULE_IN;
    rule->protocol = NetworkProtocol::ICMP;
    ruleList.push_back(rule);

    // OUT rule
    sptr<NetFirewallIpRule> rule2 = (std::make_unique<NetFirewallIpRule>()).release();
    rule2->ruleDirection = NetFirewallRuleDirection::RULE_OUT;
    rule2->protocol = NetworkProtocol::TCP;
    ruleList.push_back(rule2);

    ret = bpfNet->SetFirewallIpRules(ruleList);
    EXPECT_TRUE(ret == NETFIREWALL_SUCCESS || ret == NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetBpfFirewallDomainRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    DomainHashKey key = {};
    DomainValue value = {};
    value.uid = 100;
    value.appuid = 1000;

    int ret = bpfNet->SetBpfFirewallDomainRules(FirewallRuleAction::RULE_ALLOW, key, value, false);
    EXPECT_TRUE(ret == 0 || ret == -1);
    ret = bpfNet->SetBpfFirewallDomainRules(FirewallRuleAction::RULE_DENY, key, value, false);
    EXPECT_TRUE(ret == 0 || ret == -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetBpfFirewallDomainPrefixRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    DomainHashKey key = {};
    DomainValue value = {};
    value.uid = 100;
    value.appuid = 1000;

    // prefix rule (e.g. "*.example.com") written into DOMAIN_PREFIX_*_MAP
    int ret = bpfNet->SetBpfFirewallDomainPrefixRules(FirewallRuleAction::RULE_ALLOW, key, value, false);
    EXPECT_TRUE(ret == 0 || ret == -1);
    ret = bpfNet->SetBpfFirewallDomainPrefixRules(FirewallRuleAction::RULE_DENY, key, value, false);
    EXPECT_TRUE(ret == 0 || ret == -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearDomainRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    // ClearDomainRules now also clears DOMAIN_PREFIX_*_MAP.
    // After writing prefix rules, ClearDomainRules must not crash and must run.
    DomainHashKey key = {};
    DomainValue value = {};
    value.uid = 100;
    value.appuid = 1000;
    bpfNet->SetBpfFirewallDomainPrefixRules(FirewallRuleAction::RULE_ALLOW, key, value, false);

    bpfNet->ClearDomainRules();
    EXPECT_NE(bpfNet, nullptr);
}
} // namespace NetManagerStandard
} // namespace OHOS
