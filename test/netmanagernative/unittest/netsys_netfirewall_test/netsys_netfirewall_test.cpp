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

#include "bpf_loader.h"
#include "i_netfirewall_callback.h"
#include "netfirewall_callback_stub.h"
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define private public
#define protected public

#include "bitmap_manager.h"
#include "bpf_netfirewall.h"
#include "firewall_test_env.h"
#include "netsys/dns_param_cache.h"

#define FILE_NAME (strrchr(__FILE__, '/') + 1)
#define MANUAL_TEST 0

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::NetManagerStandard;

static sptr<NetFirewallIpRule> GeIpFirewallRule(NetFirewallRuleDirection dir, string addr, int32_t start = 80,
                                                int32_t end = 80)
{
    sptr<NetFirewallIpRule> rule = (std::make_unique<NetFirewallIpRule>()).release();
    if (!rule) {
        return rule;
    }
    rule->ruleAction = FirewallRuleAction::RULE_DENY;
    rule->appUid = 0;

    std::vector<NetFirewallIpParam> remoteIp;
    NetFirewallIpParam remoteIpParam;
    remoteIpParam.family = 1;
    remoteIpParam.type = 1;
    inet_pton(AF_INET, addr.c_str(), &remoteIpParam.ipv4.startIp);
    remoteIpParam.mask = IPV4_MAX_PREFIXLEN;
    remoteIp.push_back(remoteIpParam);
    rule->remoteIps = remoteIp;

    rule->protocol = NetworkProtocol::TCP;

    std::vector<NetFirewallPortParam> ports;
    NetFirewallPortParam remotePort;
    remotePort.startPort = start;
    remotePort.endPort = end;
    ports.emplace_back(remotePort);
    rule->localPorts = ports;
    rule->remotePorts = ports;

    return rule;
}

class NetsysNetFirewallTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetsysNetFirewallTest::SetUpTestCase()
{
    /* Suite-level device integration on the production singleton: load the
     * elf once (pins every map the write-family asserts against) and start
     * the ring-buffer listener thread. Test bodies never use this
     * singleton — they get isolated instances via NewFirewallInstance(). */
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NetsysBpfNetFirewall::GetInstance();
    if (!bpfNetFirewall->IsBpfLoaded()) {
        auto ret = OHOS::NetManagerStandard::LoadElf(FIREWALL_BPF_PATH);
        printf("LoadElf is %d\n", ret);

        if (ret == ElfLoadError::ELF_LOAD_ERR_NONE) {
            bpfNetFirewall->SetBpfLoaded(true);
        }
        bpfNetFirewall->StartListener();
    }
}

void NetsysNetFirewallTest::TearDownTestCase()
{
    /* Zero-residue exit: a fresh instance (empty snapshot cache -> full
     * reconcile) sweeps every dimension map back to empty, so later suites
     * on the same device start from a clean slate. The write and clear
     * entry points are not flag-gated, so no load pin is needed here. */
    auto sweeper = NewFirewallInstance();
    sweeper->ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_IN);
    sweeper->ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_OUT);
    sweeper->ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
}

void NetsysNetFirewallTest::SetUp()
{
    /* Canonical per-case start state: the load flag is CLASS-STATIC and
     * shared by every instance — no case may inherit another case's
     * toggle. (Constructing an instance resets it as well; doing it here
     * makes the contract explicit instead of ctor-side-effect dependent.) */
    NetsysBpfNetFirewall::GetInstance()->SetBpfLoaded(false);
}

void NetsysNetFirewallTest::TearDown() {}

class TestNetFirewallCallbackStub : public OHOS::NetsysNative::NetFirewallCallbackStub {
public:
    int32_t OnIntercept(OHOS::sptr<InterceptRecord> &info)
    {
        if (!info) {
            return -1;
        }

        std::thread t = std::thread([&]() {
            printf("\ttransProtocol=%u\n", info->protocol);
            printf("\tsourcePort=%u\n", info->localPort);
            printf("\tdestPort=%u\n", info->remotePort);
            printf("\tsourceIp=%s\n", (info->localIp).c_str());
            printf("\tdestIp=%s\n", (info->remoteIp).c_str());
            printf("\tappUid=%d\n", info->appUid);
        });
        t.join();

        return 0;
    }
};

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest001, TestSize.Level0)
{
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();

    OHOS::sptr<OHOS::NetsysNative::INetFirewallCallback> callback = new (nothrow) TestNetFirewallCallbackStub;
    int32_t ret = bpfNetFirewall->RegisterCallback(callback);
    EXPECT_EQ(ret, 0);

    ret = bpfNetFirewall->UnregisterCallback(callback);
    EXPECT_EQ(ret, 0);

    Ip4Key srcIp = 0;
    string src = "192.168.8.116";
    inet_pton(AF_INET, src.c_str(), &srcIp);

    Ip4Key dstIp = 0;
    string dst = "192.168.1.5";
    inet_pton(AF_INET, dst.c_str(), &dstIp);
    InterceptEvent interceptEv = {
        .dir = INGRESS,
        .family = AF_INET,
        .protocol = IPPROTO_TCP,
        .sport = 80,
        .dport = 5684,
        .appuid = 0,
    };
    interceptEv.ipv4.saddr = srcIp;
    interceptEv.ipv4.daddr = dstIp;

    bpfNetFirewall->NotifyInterceptEvent(&interceptEv);

    Event ev = {
        .intercept = interceptEv,
        .type = EVENT_INTERCEPT,
        .len = sizeof(InterceptEvent),
    };
    ret = NetsysBpfNetFirewall::HandleEvent(NULL, &ev, sizeof(ev));
    EXPECT_EQ(ret, 0);

    DebugEvent debugEv = {
        .type = DBG_GENERIC,
        .dir = INGRESS,
        .arg1 = 0,
    };
    NetsysBpfNetFirewall::HandleDebugEvent(&debugEv);
}

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest002, TestSize.Level0)
{
    Bitmap a(1);
    Bitmap b(a);

    b.Clear();
    b.Set(3);
    EXPECT_TRUE(a.SpecialHash() != 0);
    a.Or(b);
    a.And(b);

    EXPECT_TRUE(a == b);
}

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest003, TestSize.Level0)
{
    BpfUnorderedMap<int> map;
    int key = 1;
    Bitmap val(10);
    Bitmap other(20);
    map.OrInsert(key, val);
    map.OrInsert(key + 1, val);
    map.OrForEach(other);

    map.Delete(key);
    EXPECT_FALSE(map.Empty());
    map.Clear();
    EXPECT_TRUE(map.Empty());

    EXPECT_TRUE(map.Get().empty());
}

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest004, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    ruleList.push_back(rule);
    sptr<NetFirewallIpRule> rule2 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102");
    ruleList.push_back(rule2);
    sptr<NetFirewallIpRule> rule3 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102", 80, 443);
    ruleList.push_back(rule3);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    /* fix: unconstrained dimensions have no sentinel entries any more; the
     * fallback bitmaps live in the wildcard store keyed by dimension role. */
    EXPECT_FALSE(manager.GetSrcIp4Map().empty());
    EXPECT_TRUE(manager.GetSrcIp6Map().empty());
    EXPECT_TRUE(manager.GetDstIp4Map().empty());
    EXPECT_TRUE(manager.GetDstIp6Map().empty());
    EXPECT_FALSE(manager.GetSrcPortMap().Get().empty());
    EXPECT_FALSE(manager.GetDstPortMap().Get().empty());
    EXPECT_FALSE(manager.GetProtoMap().Empty());
    EXPECT_FALSE(manager.GetActionMap().Empty());
    EXPECT_TRUE(manager.GetAppIdMap().Empty());

    Bitmap *wildcardStore = manager.GetWildcardStore();
    ASSERT_NE(wildcardStore, nullptr);
    EXPECT_EQ(wildcardStore[WILDCARD_SRC_ADDR].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_DST_ADDR].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_SPORT].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_DPORT].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_PROTO].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_APPUID].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_UID].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_IFACE].SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest005, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    ruleList.push_back(rule);
    sptr<NetFirewallIpRule> rule2 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102");
    ruleList.push_back(rule2);

    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    int32_t ret = bpfNetFirewall->SetFirewallIpRules(ruleList);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest006, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102", 1, 65535);
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    /* fix: unconstrained dimensions have no sentinel entries any more. */
    EXPECT_FALSE(manager.GetSrcIp4Map().empty());
    EXPECT_TRUE(manager.GetSrcIp6Map().empty());
    EXPECT_TRUE(manager.GetDstIp4Map().empty());
    EXPECT_TRUE(manager.GetDstIp6Map().empty());
    EXPECT_FALSE(manager.GetSrcPortMap().Get().empty());
    EXPECT_FALSE(manager.GetDstPortMap().Get().empty());
    EXPECT_FALSE(manager.GetProtoMap().Empty());
    EXPECT_FALSE(manager.GetActionMap().Empty());
    EXPECT_TRUE(manager.GetAppIdMap().Empty());

    Bitmap *wildcardStore = manager.GetWildcardStore();
    ASSERT_NE(wildcardStore, nullptr);
    EXPECT_EQ(wildcardStore[WILDCARD_SRC_ADDR].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_DST_ADDR].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_SPORT].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_DPORT].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_APPUID].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_UID].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_IFACE].SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, IpParamParserTest001, TestSize.Level0)
{
    std::vector<Ip4Data> list;
    in_addr startAddr;
    startAddr.s_addr = 2887802737;
    in_addr endAddr;
    endAddr.s_addr = 2887802770;
    EXPECT_NE(IpParamParser::GetIp4AndMask(endAddr, startAddr, list), 0);
    EXPECT_EQ(IpParamParser::GetIp4AndMask(startAddr, endAddr, list), 0);
    EXPECT_EQ(IpParamParser::GetSuffixZeroLength(0), 32);
    EXPECT_GE(IpParamParser::GetSuffixZeroLength(1000), 0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap001, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    segBitMap.AddMap(1, 65535, bitmap0);
    ASSERT_EQ(segBitMap.segments_.size(), 1);
    EXPECT_EQ(segBitMap.segments_[0].start, 1);
    EXPECT_EQ(segBitMap.segments_[0].end, 65535);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap002, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 1000, bitmap0);
    segBitMap.AddMap(65535, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    EXPECT_EQ(segBitMap.segments_[0].start, 500);
    EXPECT_EQ(segBitMap.segments_[0].end, 1000);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
    EXPECT_EQ(segBitMap.segments_[1].start, 65535);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, bitmap1);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap003, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(65535, 65535, bitmap0);
    segBitMap.AddMap(500, 1000, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    EXPECT_EQ(segBitMap.segments_[0].start, 500);
    EXPECT_EQ(segBitMap.segments_[0].end, 1000);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 65535);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, bitmap0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap004, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 65535, bitmap0);
    segBitMap.AddMap(65535, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    EXPECT_EQ(segBitMap.segments_[0].start, 500);
    EXPECT_EQ(segBitMap.segments_[0].end, 65534);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 65535);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap005, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(65535, 65535, bitmap0);
    segBitMap.AddMap(500, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    EXPECT_EQ(segBitMap.segments_[0].start, 500);
    EXPECT_EQ(segBitMap.segments_[0].end, 65534);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap1);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 65535);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap006, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 65535, bitmap0);
    segBitMap.AddMap(500, 500, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[0].start, 500);
    EXPECT_EQ(segBitMap.segments_[0].end, 500);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[1].start, 501);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, bitmap0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap007, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 500, bitmap0);
    segBitMap.AddMap(500, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[0].start, 500);
    EXPECT_EQ(segBitMap.segments_[0].end, 500);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[1].start, 501);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, bitmap1);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap008, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 65535, bitmap0);
    segBitMap.AddMap(400, 400, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    EXPECT_EQ(segBitMap.segments_[0].start, 400);
    EXPECT_EQ(segBitMap.segments_[0].end, 400);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, bitmap0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap009, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(1, 65535, bitmap0);
    segBitMap.AddMap(1, 65535, bitmap1);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 1);
    EXPECT_EQ(segBitMap.segments_[0].start, 1);
    EXPECT_EQ(segBitMap.segments_[0].end, 65535);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, result);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap010, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(100, 65535, bitmap0);
    segBitMap.AddMap(500, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap011, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 65535, bitmap0);
    segBitMap.AddMap(100, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap1);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap012, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(100, 65535, bitmap0);
    segBitMap.AddMap(100, 1000, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 1000);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[1].start, 1001);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, bitmap0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap013, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(100, 1000, bitmap0);
    segBitMap.AddMap(100, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 2);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 1000);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[1].start, 1001);
    EXPECT_EQ(segBitMap.segments_[1].end, 65535);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, bitmap1);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap014, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(100, 65535, bitmap0);
    segBitMap.AddMap(500, 1000, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 3);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 1000);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[2].start, 1001);
    EXPECT_EQ(segBitMap.segments_[2].end, 65535);
    EXPECT_EQ(segBitMap.segments_[2].bitmap, bitmap0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap015, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 65535, bitmap0);
    segBitMap.AddMap(100, 1000, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 3);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap1);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 1000);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[2].start, 1001);
    EXPECT_EQ(segBitMap.segments_[2].end, 65535);
    EXPECT_EQ(segBitMap.segments_[2].bitmap, bitmap0);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap016, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(100, 1000, bitmap0);
    segBitMap.AddMap(500, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 3);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 1000);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[2].start, 1001);
    EXPECT_EQ(segBitMap.segments_[2].end, 65535);
    EXPECT_EQ(segBitMap.segments_[2].bitmap, bitmap1);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap017, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    segBitMap.AddMap(500, 1000, bitmap0);
    segBitMap.AddMap(100, 65535, bitmap1);
    ASSERT_EQ(segBitMap.segments_.size(), 3);
    EXPECT_EQ(segBitMap.segments_[0].start, 100);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap1);
    Bitmap result;
    result.Or(bitmap0);
    result.Or(bitmap1);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 1000);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result);
    EXPECT_EQ(segBitMap.segments_[2].start, 1001);
    EXPECT_EQ(segBitMap.segments_[2].end, 65535);
    EXPECT_EQ(segBitMap.segments_[2].bitmap, bitmap1);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap018, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    Bitmap bitmap2(2);
    segBitMap.AddMap(40, 800, bitmap0);
    segBitMap.AddMap(1000, 65535, bitmap1);
    segBitMap.AddMap(500, 5000, bitmap2);
    ASSERT_EQ(segBitMap.segments_.size(), 5);
    EXPECT_EQ(segBitMap.segments_[0].start, 40);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
    Bitmap result02;
    result02.Or(bitmap0);
    result02.Or(bitmap2);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 800);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result02);
    EXPECT_EQ(segBitMap.segments_[2].start, 801);
    EXPECT_EQ(segBitMap.segments_[2].end, 999);
    EXPECT_EQ(segBitMap.segments_[2].bitmap, bitmap2);
    Bitmap result12;
    result12.Or(bitmap1);
    result12.Or(bitmap2);
    EXPECT_EQ(segBitMap.segments_[3].start, 1000);
    EXPECT_EQ(segBitMap.segments_[3].end, 5000);
    EXPECT_EQ(segBitMap.segments_[3].bitmap, result12);
    EXPECT_EQ(segBitMap.segments_[4].start, 5001);
    EXPECT_EQ(segBitMap.segments_[4].end, 65535);
    EXPECT_EQ(segBitMap.segments_[4].bitmap, bitmap1);
}

HWTEST_F(NetsysNetFirewallTest, SegmentBitmapMap019, TestSize.Level0)
{
    SegmentBitmapMap segBitMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    Bitmap bitmap2(2);
    segBitMap.AddMap(40, 1000, bitmap0);
    segBitMap.AddMap(800, 65535, bitmap1);
    segBitMap.AddMap(500, 5000, bitmap2);
    ASSERT_EQ(segBitMap.segments_.size(), 5);
    EXPECT_EQ(segBitMap.segments_[0].start, 40);
    EXPECT_EQ(segBitMap.segments_[0].end, 499);
    EXPECT_EQ(segBitMap.segments_[0].bitmap, bitmap0);
    Bitmap result02;
    result02.Or(bitmap0);
    result02.Or(bitmap2);
    EXPECT_EQ(segBitMap.segments_[1].start, 500);
    EXPECT_EQ(segBitMap.segments_[1].end, 799);
    EXPECT_EQ(segBitMap.segments_[1].bitmap, result02);
    Bitmap result012;
    result012.Or(bitmap0);
    result012.Or(bitmap1);
    result012.Or(bitmap2);
    EXPECT_EQ(segBitMap.segments_[2].start, 800);
    EXPECT_EQ(segBitMap.segments_[2].end, 1000);
    EXPECT_EQ(segBitMap.segments_[2].bitmap, result012);
    Bitmap result12;
    result12.Or(bitmap1);
    result12.Or(bitmap2);
    EXPECT_EQ(segBitMap.segments_[3].start, 1001);
    EXPECT_EQ(segBitMap.segments_[3].end, 5000);
    EXPECT_EQ(segBitMap.segments_[3].bitmap, result12);
    EXPECT_EQ(segBitMap.segments_[4].start, 5001);
    EXPECT_EQ(segBitMap.segments_[4].end, 65535);
    EXPECT_EQ(segBitMap.segments_[4].bitmap, bitmap1);
}

HWTEST_F(NetsysNetFirewallTest, InterfaceBitmapBuild001, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    rule->interface = "lo";
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();
    EXPECT_FALSE(interfaceMap.Empty());
}

HWTEST_F(NetsysNetFirewallTest, InterfaceBitmapBuild002, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    /* A rule with an EMPTY interface constraint contributes no interface
     * entry: the former empty-name anchor was removed with the wildcard
     * store redesign (bitmap_manager.cpp Insert(), "The former interface
     * empty-name anchor is removed") — unconstrained dimensions are
     * collected in wildcardStore_ instead. */
    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();
    EXPECT_TRUE(interfaceMap.Empty());
}

HWTEST_F(NetsysNetFirewallTest, InterfaceBitmapBuild003, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule1 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    rule1->interface = "lo";
    ruleList.push_back(rule1);
    sptr<NetFirewallIpRule> rule2 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102");
    rule2->interface = "lo";
    ruleList.push_back(rule2);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();
    EXPECT_FALSE(interfaceMap.Empty());
    auto &map = interfaceMap.Get();
    /* Two rules both constrained to "lo" collapse to ONE interface entry
     * (keyed by interface name); the former auto-added anchor entry no
     * longer exists — see bitmap_manager.cpp Insert(). */
    EXPECT_EQ(map.size(), 1u);
    auto it = map.find("lo");
    ASSERT_NE(it, map.end());
    EXPECT_NE(it->second.SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, InterfaceBitmapBuild004, TestSize.Level0)
{
    BpfInterfaceMap interfaceMap;
    Bitmap bitmap0(0);
    Bitmap bitmap1(1);
    interfaceMap.OrInsert("wlan0", bitmap0);
    interfaceMap.OrInsert("eth0", bitmap1);

    EXPECT_FALSE(interfaceMap.Empty());
    auto &map = interfaceMap.Get();
    EXPECT_EQ(map.size(), 2u);

    interfaceMap.Clear();
    EXPECT_TRUE(interfaceMap.Empty());
}

HWTEST_F(NetsysNetFirewallTest, InterfaceBitmapBuild005, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule1 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    rule1->interface = "wlan0";
    ruleList.push_back(rule1);
    sptr<NetFirewallIpRule> rule2 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102");
    rule2->interface = "eth0";
    ruleList.push_back(rule2);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();
    EXPECT_FALSE(interfaceMap.Empty());
    auto &map = interfaceMap.Get();
    /* Exactly the two explicit interface names — no environment-enumerated
     * or auto-added anchor entry (see bitmap_manager.cpp Insert()). */
    EXPECT_EQ(map.size(), 2u);
    EXPECT_NE(map.find("wlan0"), map.end());
    EXPECT_NE(map.find("eth0"), map.end());
}

HWTEST_F(NetsysNetFirewallTest, InterfaceBitmapBuild006, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule1 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_OUT, "153.3.238.110");
    rule1->interface = "wlan0";
    ruleList.push_back(rule1);
    sptr<NetFirewallIpRule> rule2 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_OUT, "153.3.238.102");
    ruleList.push_back(rule2);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();
    EXPECT_FALSE(interfaceMap.Empty());
    auto &map = interfaceMap.Get();
    /* rule2 has an EMPTY interface constraint: it contributes no entry of
     * its own (the former ""-named anchor is gone, see bitmap_manager.cpp
     * Insert()); only rule1's explicit "wlan0" lands in the map. */
    EXPECT_EQ(map.size(), 1u);
    EXPECT_NE(map.find("wlan0"), map.end());
    EXPECT_EQ(map.find(std::string("")), map.end());
}

HWTEST_F(NetsysNetFirewallTest, InterfaceBitmapBuild007, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    rule->interface = "lo";
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();
    EXPECT_FALSE(interfaceMap.Empty());

    manager.Clear();
    EXPECT_TRUE(interfaceMap.Empty());
}

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest007, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    rule->interface = "lo";
    ruleList.push_back(rule);

    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    int32_t ret = bpfNetFirewall->SetFirewallIpRules(ruleList);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNetFirewallTest, WriteInterfaceBpfMapEmptyTest, TestSize.Level0)
{
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    BitmapManager manager;
    manager.Clear();
    /* fix: an empty desired set is no longer rejected by a guard; it reconciles the
     * bpf map to empty. The fixture guarantees the maps are pinned, so both
     * directions must succeed — a broken environment fails loudly here
     * instead of passing vacuously on a conditional. */
    int32_t ret = bpfNetFirewall->WriteInterfaceBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(ret, 0);

    ret = bpfNetFirewall->WriteInterfaceBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNetFirewallTest, WriteInterfaceBpfMapLongNameTest, TestSize.Level0)
{
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    rule->interface = "abcdefghijklmnopq";
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    int32_t writeRet = bpfNetFirewall->WriteInterfaceBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(writeRet, 0);
}

HWTEST_F(NetsysNetFirewallTest, WriteInterfaceBpfMapEgressTest, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_OUT, "153.3.238.110");
    rule->interface = "eth0";
    ruleList.push_back(rule);

    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    int32_t ret = bpfNetFirewall->SetFirewallIpRules(ruleList);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNetFirewallTest, InterfaceRebuildTest, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    ruleList.push_back(rule);

    BitmapManager manager;
    manager.BuildBitmapMap(ruleList);
    manager.BuildBitmapMap(ruleList);

    /* Rebuild is a full Clear()+rebuild: still no interface entry for a
     * rule with an empty interface constraint (former empty-name anchor
     * removed — see bitmap_manager.cpp Insert()). */
    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();
    EXPECT_TRUE(interfaceMap.Empty());
}

HWTEST_F(NetsysNetFirewallTest, WriteInterfaceBpfMapSuccessTest, TestSize.Level0)
{
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    rule->interface = "lo";
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    int32_t writeRet = bpfNetFirewall->WriteInterfaceBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(writeRet, 0);
}

HWTEST_F(NetsysNetFirewallTest, ClearBpfFirewallRulesInterfaceTest, TestSize.Level0)
{
    /* void-returning sweep: the contract is "completes without crashing on
     * the fixture-pinned maps". The old nullptr-check-only assertion could
     * never fail and was removed. */
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    bpfNetFirewall->ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_IN);
    bpfNetFirewall->ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_OUT);
}

/*
 * fix regression tests: extreme rule values must stay normal single-value rules
 * (of FIREWALL_P2_SENTINEL_FIX_DESIGN.md §8) and rule sets unconstrained on
 * a dimension must land in the wildcard store instead of dimension-map sentinels.
 */

HWTEST_F(NetsysNetFirewallTest, WildcardStoreExtremePortTest, TestSize.Level0)
{
    /* proxy: a rule constrained to remote port 65535 (the old sentinel key value)
     * must not leak its bit into the sport wildcard store; other ports fall back to
     * the wildcard store and never see this rule. */
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102", 65535, 65535);
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    /* proxy: the port rule must land in the sport/dport dimension maps as an
     * exact entry (single rule, single port param => exactly one entry per map,
     * carrying port 65535), not as a sentinel-shaped wildcard entry. */
    EXPECT_EQ(manager.GetSrcPortMap().Get().size(), 1u);
    EXPECT_EQ(manager.GetSrcPortMap().Get().front().data, 65535);
    EXPECT_EQ(manager.GetDstPortMap().Get().size(), 1u);
    EXPECT_EQ(manager.GetDstPortMap().Get().front().data, 65535);
    Bitmap *wildcardStore = manager.GetWildcardStore();
    ASSERT_NE(wildcardStore, nullptr);
    EXPECT_EQ(wildcardStore[WILDCARD_SPORT].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_DPORT].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_SRC_ADDR].SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, WildcardStoreExtremeIpTest, TestSize.Level0)
{
    /* proxy: deny 255.255.255.0/24 only produces its own subnet entry; there is no
     * /32 sentinel node any more for covering prefixes to merge into. */
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "255.255.255.0");
    rule->remoteIps[0].mask = 24; // 255.255.255.0/24 contains 255.255.255.255
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    auto &srcIp4Map = manager.GetSrcIp4Map();
    ASSERT_EQ(srcIp4Map.size(), 1u);
    EXPECT_EQ(srcIp4Map[0].mask, 24u);
    Bitmap *wildcardStore = manager.GetWildcardStore();
    ASSERT_NE(wildcardStore, nullptr);
    EXPECT_EQ(wildcardStore[WILDCARD_SRC_ADDR].SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, WildcardStoreExtremeUidTest, TestSize.Level0)
{
    /* proxy: userId 0xFFFFFFFF arrives as int32_t -1. The marker guard (>0) keeps
     * it out of the uid map as a rule key and the no-marker branch (<1) collects it
     * into the uid wildcard store, so it can never collide with a wildcard entry. */
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102");
    rule->userId = -1; // 0xFFFFFFFF as int32_t
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    EXPECT_TRUE(manager.GetUidMap().Get().empty());
    Bitmap *wildcardStore = manager.GetWildcardStore();
    ASSERT_NE(wildcardStore, nullptr);
    EXPECT_NE(wildcardStore[WILDCARD_UID].SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, WildcardStorePurePortRuleTest, TestSize.Level0)
{
    /* A pure port rule set (no IP constraint) leaves the IP dimension maps empty
     * (reconciled to cleared bpf maps) and puts the rule bit into the address wildcard
     * store, so any address still matches the rule. */
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "");
    rule->remoteIps.clear();
    rule->localIps.clear();
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    EXPECT_TRUE(manager.GetSrcIp4Map().empty());
    EXPECT_TRUE(manager.GetSrcIp6Map().empty());
    EXPECT_TRUE(manager.GetDstIp4Map().empty());
    EXPECT_TRUE(manager.GetDstIp6Map().empty());
    EXPECT_FALSE(manager.GetSrcPortMap().Get().empty());
    Bitmap *wildcardStore = manager.GetWildcardStore();
    ASSERT_NE(wildcardStore, nullptr);
    EXPECT_NE(wildcardStore[WILDCARD_SRC_ADDR].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_DST_ADDR].SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, WildcardStorePureUidRuleTest, TestSize.Level0)
{
    /* A pure uid rule set (no IP/port constraint) leaves IP and port maps empty
     * and puts the rule bit into the corresponding wildcard stores. */
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "");
    rule->remoteIps.clear();
    rule->localIps.clear();
    rule->remotePorts.clear();
    rule->localPorts.clear();
    rule->userId = 100;
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    EXPECT_TRUE(manager.GetSrcIp4Map().empty());
    EXPECT_TRUE(manager.GetDstIp4Map().empty());
    EXPECT_TRUE(manager.GetSrcPortMap().Get().empty());
    EXPECT_TRUE(manager.GetDstPortMap().Get().empty());
    EXPECT_EQ(manager.GetUidMap().Get().size(), 1u);
    Bitmap *wildcardStore = manager.GetWildcardStore();
    ASSERT_NE(wildcardStore, nullptr);
    EXPECT_NE(wildcardStore[WILDCARD_SRC_ADDR].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_SPORT].SpecialHash(), 0u);
    EXPECT_NE(wildcardStore[WILDCARD_DPORT].SpecialHash(), 0u);
    EXPECT_EQ(wildcardStore[WILDCARD_UID].SpecialHash(), 0u);
}

HWTEST_F(NetsysNetFirewallTest, SetBpfFirewallRulesEmptyIpMapsTest, TestSize.Level0)
{
    /* A pure port rule set must be accepted end to end: empty IP maps are
     * reconciled to cleared bpf maps instead of failing the whole rule push. */
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "");
    rule->remoteIps.clear();
    rule->localIps.clear();
    ruleList.push_back(rule);

    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    int32_t ret = bpfNetFirewall->SetFirewallIpRules(ruleList);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysNetFirewallTest, WriteWildcardBpfMapTest, TestSize.Level0)
{
    /* The wildcard store is written as 8 role entries per direction; on a device with
     * bpf maps loaded this must succeed and leave a snapshot for later reconciliation. */
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102");
    ruleList.push_back(rule);

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NewFirewallInstance();
    /* Fixture-pinned maps: both direction writes must succeed — a broken
     * environment fails loudly instead of passing vacuously on a
     * conditional. */
    int32_t writeRet = bpfNetFirewall->WriteWildcardBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(writeRet, 0);
    writeRet = bpfNetFirewall->WriteWildcardBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(writeRet, 0);
}

/* Lives in this TU (not bpf_netfirewall_test.cpp): DnsParamCache pulls
 * bpf_def.h, whose bpf_map_def collides with the BPF-parser stubs defined
 * there. This TU has no stubs. */
HWTEST_F(NetsysNetFirewallTest, GetBaseDnsServersOrder001, TestSize.Level1)
{
    /* 检视 闭环: the default network's servers must LEAD the union in
     * insertion order — with a 10-server union the cap-8 truncation in
     * BuildDnsServerKeys must keep the default network's resolvers and drop
     * secondary-network ones, never the other way around (a lexicographic
     * std::set ordering would drop "8.8.x.x" in favor of "1.1.1.1").
     * Isolated netIds, default network restored and test nets destroyed. */
    auto &cache = OHOS::nmd::DnsParamCache::GetInstance();
    const uint16_t defNet = 901;
    const uint16_t secNet = 902;
    int32_t oldDefault = cache.GetDefaultNetwork();
    /* Non-fatal assertions from here on — an ASSERT mid-test
     * would skip the cleanup below and leak netId 901/902 caches plus the
     * default-network change into the process-wide singleton. ASSERT_GE
     * remains only as an index guard before direct element access. */
    EXPECT_EQ(cache.CreateCacheForNet(defNet), 0);
    EXPECT_EQ(cache.CreateCacheForNet(secNet), 0);
    std::vector<std::string> defServers = {"8.8.8.8", "8.8.4.4"};
    std::vector<std::string> secServers = {"1.1.1.1",        "1.0.0.1",        "9.9.9.9",   "149.112.112.112",
                                           "208.67.222.222", "208.67.220.220", "64.6.64.6", "64.6.65.6"};
    std::vector<std::string> noDomains;
    EXPECT_EQ(cache.SetResolverConfig(defNet, 0, 0, defServers, noDomains), 0);
    EXPECT_EQ(cache.SetResolverConfig(secNet, 0, 0, secServers, noDomains), 0);
    cache.SetDefaultNetwork(defNet);
    std::vector<std::string> servers;
    EXPECT_EQ(cache.GetBaseDnsServers(servers), 0);
    ASSERT_GE(servers.size(), 2U); /* index guard only */
    EXPECT_EQ(servers.size(), 10U);
    /* insertion order: the default network's pair leads the union */
    EXPECT_EQ(servers[0], "8.8.8.8");
    EXPECT_EQ(servers[1], "8.8.4.4");
    /* the cap-8 truncation keeps exactly the default network's servers first */
    std::vector<DnsServerV4Key> v4Keys;
    std::vector<DnsServerV6Key> v6Keys;
    EXPECT_EQ(NetsysBpfNetFirewall::BuildDnsServerKeys(servers, 990004, v4Keys, v6Keys), NETFIREWALL_SUCCESS);
    ASSERT_GE(v4Keys.size(), 2U); /* index guard only */
    EXPECT_EQ(v4Keys.size() + v6Keys.size(), 8U);
    EXPECT_EQ(v4Keys[0].uid, 990004U);
    EXPECT_EQ(v4Keys[1].uid, 990004U);
    struct in_addr expect = {};
    EXPECT_EQ(inet_pton(AF_INET, "8.8.8.8", &expect), 1);
    EXPECT_EQ(v4Keys[0].ip, expect.s_addr);
    EXPECT_EQ(inet_pton(AF_INET, "8.8.4.4", &expect), 1);
    EXPECT_EQ(v4Keys[1].ip, expect.s_addr);
    /* cleanup: restore the previous default and drop the isolated nets */
    cache.SetDefaultNetwork(oldDefault);
    cache.DestroyNetworkCache(defNet);
    cache.DestroyNetworkCache(secNet);
}
