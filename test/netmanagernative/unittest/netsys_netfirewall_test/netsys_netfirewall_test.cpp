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
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define private public
#define protected public

#include "bitmap_manager.h"
#include "bpf_netfirewall.h"

#define FILE_NAME (strrchr(__FILE__, '/') + 1)
#define MANUAL_TEST 0

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::NetManagerStandard;

static sptr<NetFirewallIpRule> GeIpFirewallRule(NetFirewallRuleDirection dir, string addr)
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
    const int32_t defaultPort = 80;
    remotePort.startPort = defaultPort;
    remotePort.endPort = defaultPort;
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

void NetsysNetFirewallTest::TearDownTestCase() {}

void NetsysNetFirewallTest::SetUp() {}

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
    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NetsysBpfNetFirewall::GetInstance();

    OHOS::sptr<OHOS::NetsysNative::INetFirewallCallback> callback = new (nothrow)TestNetFirewallCallbackStub;
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

    BitmapManager manager;
    int ret = manager.BuildBitmapMap(ruleList);
    EXPECT_EQ(ret, 0);

    EXPECT_FALSE(manager.GetSrcIp4Map().empty());
    EXPECT_FALSE(manager.GetSrcIp6Map().empty());
    EXPECT_FALSE(manager.GetDstIp4Map().empty());
    EXPECT_FALSE(manager.GetDstIp6Map().empty());
    EXPECT_FALSE(manager.GetSrcPortMap().Empty());
    EXPECT_FALSE(manager.GetDstPortMap().Empty());
    EXPECT_FALSE(manager.GetProtoMap().Empty());
    EXPECT_FALSE(manager.GetActionMap().Empty());
    EXPECT_FALSE(manager.GetAppIdMap().Empty());
}

HWTEST_F(NetsysNetFirewallTest, NetsysNetFirewallTest005, TestSize.Level0)
{
    std::vector<sptr<NetFirewallIpRule>> ruleList;
    sptr<NetFirewallIpRule> rule = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.110");
    ruleList.push_back(rule);
    sptr<NetFirewallIpRule> rule2 = GeIpFirewallRule(NetFirewallRuleDirection::RULE_IN, "153.3.238.102");
    ruleList.push_back(rule2);

    shared_ptr<NetsysBpfNetFirewall> bpfNetFirewall = NetsysBpfNetFirewall::GetInstance();
    int32_t ret = bpfNetFirewall->SetFirewallIpRules(ruleList);
    EXPECT_EQ(ret, 0);
}