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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "bpf_loader.h"
#include "bpf_netfirewall.h"
#include "firewall_test_env.h"
#include "netfirewall/netfirewall_ct_def.h"
#include "netfirewall_callback_stub.h"

/* Parser unit tests run the REAL BPF parser header against a user-space
 * buffer. struct __sk_buff's user-space mirror stores only a 32-bit data
 * offset (real BPF contexts rewrite it to the actual packet pointer), so the
 * buffer pointer is kept in a side global set by MockSkb::Done(). utils.h
 * does not declare bpf_skb_load_bytes itself (the BPF build gets it from
 * bpf_helpers.h), so the mock stub is declared before the include and defined
 * at global scope (matching the declaration utils.h's inline functions see)
 * with kernel-identical bounds semantics (offset + len must fit the packet). */
const uint8_t *g_mockPacketData = nullptr;
uint32_t g_mockPacketLen = 0;
long bpf_skb_load_bytes(const struct __sk_buff *skb, __u32 offset, void *to, __u32 len);
#include "netfirewall/netfirewall_utils.h"

long bpf_skb_load_bytes(const struct __sk_buff *skb, __u32 offset, void *to, __u32 len)
{
    if (skb == NULL || to == NULL || g_mockPacketData == nullptr) {
        return -1;
    }
    if (static_cast<uint64_t>(offset) + len > g_mockPacketLen) {
        return -1;
    }
    if (memcpy(to, g_mockPacketData + offset, len) == NULL) {
        return -1;
    }
    return 0;
}

/* ===== User-space shim for the BPF netfirewall_domain.h chain.
 * domain.h/match.h include <bpf_helpers.h> only under __BPF__ (the utils.h
 * pattern), so this TU must provide the pieces that header would: a mirror
 * bpf_map_def for the "maps" section globals (SEC comes from netfirewall_map_def.h
 * itself) and stub implementations of the helpers the domain code calls.
 * Declarations precede the include (the inline bodies inside the headers must
 * see them); the definitions follow it because they dispatch on the map
 * globals the include defines. The stubs are NOT static: they match the
 * global-scope bpf_skb_load_bytes stub pattern above and nothing else in the
 * test binary defines these names (libbpf's C-ABI helpers mangle differently).
 * =====
 * udphdr member names: this TU builds with _GNU_SOURCE, under which OHOS musl
 * netinet/udp.h renames the uh_* fields to source/dest at preprocessing time
 * (#define uh_sport source) — exactly what the headers' udph.source accesses
 * expect. No source/dest remap may be defined here: it would ping-pong with
 * musl's own macros and corrupt later struct definitions (linux/tcp.h). The
 * domain.h remap only exists for the BPF build, which has no _GNU_SOURCE. */
struct bpf_map_def {
    __u32 type;
    __u32 key_size;
    __u32 value_size;
    __u32 max_entries;
    __u32 map_flags;
    __u32 inner_map_idx;
    __u32 numa_node;
};
#define bpf_ntohs(x) __builtin_bswap16(x)
#define bpf_htons(x) __builtin_bswap16(x)
#define bpf_ntohl(x) __builtin_bswap32(x)
#define bpf_htonl(x) __builtin_bswap32(x)

/* libbpf's generated declarations give every map helper a typed-pointer
 * return; a user stub can only return one type. This proxy converts
 * implicitly to whatever pointer type the caller assigns to — the C
 * equivalent of the void* conversion the header bodies rely on. Every call
 * site in the chain assigns the result to a typed pointer before use, so no
 * bool or comparison conversion is needed. */
struct StubBpfValue {
    void *ptr;
    explicit StubBpfValue(void *p) : ptr(p) {}
    template <typename T> operator T *() const
    {
        return static_cast<T *>(ptr);
    }
};

StubBpfValue bpf_map_lookup_elem(const void *map, const void *key);
long bpf_map_update_elem(void *map, const void *key, const void *value, __u64 flags);
long bpf_map_delete_elem(void *map, const void *key);
__u32 bpf_get_socket_uid(struct __sk_buff *skb);
__u64 bpf_ktime_get_ns(void);
StubBpfValue bpf_ringbuf_reserve(const void *ringbuf, __u64 size, __u64 flags);
void bpf_ringbuf_submit(void *data, __u64 flags);
long bpf_skb_get_dev_name(struct __sk_buff *skb, char *name, __u32 name_size);
int bpf_sock_tcp_send_reset(struct __sk_buff *skb);
int bpf_skb_sock_destroy(struct __sk_buff *skb);

#include "netfirewall/netfirewall_domain.h"
/* Full policy-chain hooks (fw_policy_after_ingress/egress): pulls in
 * netfirewall_match.h/ct.h/event.h on top of domain.h. The helpers the
 * chain needs beyond the stubs above are bpf_sock_tcp_send_reset and
 * bpf_skb_sock_destroy (declared before this point — the inline hook
 * bodies must see them at definition). */
#include "netfirewall/netfirewall.h"

/* ---- controlled stub state (ResetDnsStubs restores the defaults) ---- */
static struct defalut_action_value g_stubDefaultAction = {SK_PASS, SK_PASS};
static struct domain_value g_stubDomainV4Entry = {};
static struct domain_value *g_stubDomainV4Ptr = nullptr;
static struct domain_value g_stubDomainV6Entry = {};
static struct domain_value *g_stubDomainV6Ptr = nullptr;
static struct domain_hash_key g_stubScratch[2] = {};
static __u32 g_stubSocketUid = 200100;       /* >= USER_ID_DIVIDOR: get_user_id -> 1 */
static uid_key g_stubCurrentUid = 0;         /* CURRENT_UID_MAP value (foreground user) */
static bool g_stubCurrentUidPresent = false; /* CURRENT_UID_MAP hit flag (F6 folding) */
static __u64 g_stubNowNs = 1000ULL * 1000ULL * 1000ULL * 1000ULL;
struct StubDomainUpdate {
    bool isV6 = false;
    struct ipv4_lpm_key k4 = {};
    struct ipv6_lpm_key k6 = {};
    struct domain_value value = {};
};
static std::vector<StubDomainUpdate> g_stubDomainUpdates;
static struct dns_query_pending g_stubPending = {};
static struct dns_flow_info g_stubFlow = {};
static struct dns_query_record_value g_stubRecordValue = {};
static struct dns_query_record_value *g_stubRecordPtr = nullptr;
/* keyed mode: the record is "found" only when the lookup key byte-matches the
 * expected one — lets the end-to-end test prove reversal correctness through
 * the real lookup path instead of a blind pointer return. */
static bool g_stubKeyedRecord = false;
static struct dns_query_record_key g_stubExpectedRecordKey = {};
struct StubRecordUpdate {
    struct dns_query_record_key key = {};
    struct dns_query_record_value value = {};
};
static std::vector<StubRecordUpdate> g_stubRecordUpdates;
/* Keyed DNS server stubs — the "map" contains exactly the
 * configured keys (byte-exact), everything else misses. The value object is
 * irrelevant: MatchDnsServer only tests presence. */
static std::vector<struct dns_server_v4_key> g_stubDnsServerV4Keys;
static std::vector<struct dns_server_v6_key> g_stubDnsServerV6Keys;
static struct dns_server_val g_stubDnsServerVal = {};
/* The action map stub — NULL = no rule map installed (the
 * no-rule branch); non-NULL carries the deny-rule bitmap. */
static struct bitmap g_stubActionBitmap = {};
static struct bitmap *g_stubActionBitmapPtr = nullptr;
/* ===== A' AC trie table stubs =====
 * The maps hold exactly the entries the builder emitted (byte-exact keys,
 * memcmp ordering — a hash key must never carry garbage bytes). AC_GEN_MAP
 * mirrors the single-entry array: g_stubAcGenPresent=false models the
 * absent entry (fresh boot / map-less environment) — the scans fail closed,
 * which is the behavior the pre-A' DNS tests already see, so their
 * expectations are untouched. */
struct AcTransKeyLess {
    bool operator()(const struct ac_trans_key &lhs, const struct ac_trans_key &rhs) const
    {
        return memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
    }
};
static std::map<struct ac_trans_key, struct ac_trans_val, AcTransKeyLess> g_stubAcPrefixMap;
static std::map<struct ac_trans_key, struct ac_trans_val, AcTransKeyLess> g_stubAcSuffixMap;
static __u32 g_stubAcGen = 0;
static bool g_stubAcGenPresent = false;
/* ===== Policy-chain (hook-level) stubs: CT_MAP keyed table + scratch /
 * zero bitmaps + loopback presence + interface-name availability =====
 * The ct map stores byte-exact ct_tuple keys (both sides build them via
 * get_ct_tuple from zero-initialized tuples, so padding is deterministic);
 * the value is returned as a mutable pointer exactly like the LRU_HASH
 * lookup the ct helpers rely on (in-place timeout/flag updates). */
struct CtTupleLess {
    bool operator()(const struct ct_tuple &lhs, const struct ct_tuple &rhs) const
    {
        return memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
    }
};
static std::map<struct ct_tuple, struct ct_entry, CtTupleLess> g_stubCtMap;
static struct bitmap g_stubNfScratch = {};  /* NF_BITMAP_SCRATCH (hook-mutable) */
static struct bitmap g_stubZeroBitmap = {}; /* ZERO_BITMAP_MAP fallback row */
static __u8 g_stubLoopbackPresent = 0;      /* LOOP_BACK_*_MAP hit flag */
static bool g_stubDevNameOk = false;        /* iface name readable (rule path) */
static int g_stubResetSent = 0;             /* bpf_sock_tcp_send_reset calls */
static int g_stubSockDestroyed = 0;         /* bpf_skb_sock_destroy calls (F5) */

StubBpfValue bpf_map_lookup_elem(const void *map, const void *key)
{
    if (map == &DOMAIN_SCRATCH_MAP) {
        __u32 idx = *static_cast<const __u32 *>(key);
        return StubBpfValue((idx == 1) ? &g_stubScratch[1] : &g_stubScratch[0]);
    }
    if (map == &DEFAULT_ACTION_MAP) {
        return StubBpfValue(&g_stubDefaultAction);
    }
    if (map == &DOMAIN_IPV4_MAP) {
        return StubBpfValue(g_stubDomainV4Ptr);
    }
    if (map == &DOMAIN_IPV6_MAP) {
        return StubBpfValue(g_stubDomainV6Ptr);
    }
    if (map == &DNS_QUERY_SCRATCH_MAP) {
        return StubBpfValue(&g_stubPending);
    }
    if (map == &DNS_FLOW_SCRATCH_MAP) {
        return StubBpfValue(&g_stubFlow);
    }
    if (map == &DNS_QUERY_RECORD_MAP) {
        if (g_stubKeyedRecord) {
            const struct dns_query_record_key *k = static_cast<const struct dns_query_record_key *>(key);
            return StubBpfValue((memcmp(k, &g_stubExpectedRecordKey, sizeof(*k)) == 0) ? &g_stubRecordValue : nullptr);
        }
        return StubBpfValue(g_stubRecordPtr);
    }
    if (map == &DNS_SERVER_V4_MAP) {
        const struct dns_server_v4_key *k = static_cast<const struct dns_server_v4_key *>(key);
        for (const auto &e : g_stubDnsServerV4Keys) {
            if (memcmp(&e, k, sizeof(e)) == 0) {
                return StubBpfValue(&g_stubDnsServerVal);
            }
        }
        return StubBpfValue(nullptr);
    }
    if (map == &DNS_SERVER_V6_MAP) {
        const struct dns_server_v6_key *k = static_cast<const struct dns_server_v6_key *>(key);
        for (const auto &e : g_stubDnsServerV6Keys) {
            if (memcmp(&e, k, sizeof(e)) == 0) {
                return StubBpfValue(&g_stubDnsServerVal);
            }
        }
        return StubBpfValue(nullptr);
    }
    if (map == &INGRESS_ACTION_MAP || map == &EGRESS_ACTION_MAP) {
        return StubBpfValue(g_stubActionBitmapPtr);
    }
    if (map == &AC_GEN_MAP) {
        return StubBpfValue(g_stubAcGenPresent ? &g_stubAcGen : nullptr);
    }
    if (map == &AC_PREFIX_TRANS_MAP || map == &AC_SUFFIX_TRANS_MAP) {
        const struct ac_trans_key *k = static_cast<const struct ac_trans_key *>(key);
        const auto &table = (map == &AC_PREFIX_TRANS_MAP) ? g_stubAcPrefixMap : g_stubAcSuffixMap;
        auto it = table.find(*k);
        return StubBpfValue(it == table.end() ? nullptr : const_cast<struct ac_trans_val *>(&it->second));
    }
    if (map == &CT_MAP) {
        const struct ct_tuple *k = static_cast<const struct ct_tuple *>(key);
        auto it = g_stubCtMap.find(*k);
        return StubBpfValue(it == g_stubCtMap.end() ? nullptr : &it->second);
    }
    if (map == &NF_BITMAP_SCRATCH) {
        return StubBpfValue(&g_stubNfScratch);
    }
    if (map == &ZERO_BITMAP_MAP) {
        return StubBpfValue(&g_stubZeroBitmap);
    }
    if (map == &LOOP_BACK_IPV4_MAP || map == &LOOP_BACK_IPV6_MAP) {
        /* match_loopback only tests presence; the value is never read */
        return StubBpfValue(g_stubLoopbackPresent ? &g_stubLoopbackPresent : nullptr);
    }
    if (map == &CURRENT_UID_MAP) {
        /* F6: foreground-user folding source for real uid < USER_ID_DIVIDOR */
        return StubBpfValue(g_stubCurrentUidPresent ? &g_stubCurrentUid : nullptr);
    }
    return StubBpfValue(nullptr); /* DENY maps, ...: lookup miss */
}

long bpf_map_update_elem(void *map, const void *key, const void *value, __u64 flags)
{
    (void)flags;
    if (map == &DOMAIN_IPV4_MAP) {
        StubDomainUpdate u;
        u.k4 = *static_cast<const struct ipv4_lpm_key *>(key);
        u.value = *static_cast<const struct domain_value *>(value);
        g_stubDomainUpdates.push_back(u);
        return 0;
    }
    if (map == &DOMAIN_IPV6_MAP) {
        StubDomainUpdate u;
        u.isV6 = true;
        u.k6 = *static_cast<const struct ipv6_lpm_key *>(key);
        u.value = *static_cast<const struct domain_value *>(value);
        g_stubDomainUpdates.push_back(u);
        return 0;
    }
    if (map == &DNS_QUERY_RECORD_MAP) {
        StubRecordUpdate u;
        u.key = *static_cast<const struct dns_query_record_key *>(key);
        u.value = *static_cast<const struct dns_query_record_value *>(value);
        g_stubRecordUpdates.push_back(u);
        return 0;
    }
    if (map == &CT_MAP) {
        /* insert-or-replace, mirroring the BPF_ANY semantics ct_create /
         * ct update paths use on the LRU_HASH */
        g_stubCtMap[*static_cast<const struct ct_tuple *>(key)] = *static_cast<const struct ct_entry *>(value);
        return 0;
    }
    return 0; /* untracked maps: updates swallowed */
}

__u32 bpf_get_socket_uid(struct __sk_buff *skb)
{
    (void)skb;
    return g_stubSocketUid;
}

__u64 bpf_ktime_get_ns(void)
{
    return g_stubNowNs;
}

StubBpfValue bpf_ringbuf_reserve(const void *ringbuf, __u64 size, __u64 flags)
{
    /* no ring buffer exists in user space: debug output is dropped */
    (void)ringbuf;
    (void)size;
    (void)flags;
    return StubBpfValue(nullptr);
}

void bpf_ringbuf_submit(void *data, __u64 flags)
{
    (void)data;
    (void)flags;
}

static int g_stubDeleteCalls = 0;

long bpf_map_delete_elem(void *map, const void *key)
{
    /* user-space stubs keep no real tables: the delete is a no-op, but
     * counted — read paths must never delete, and MatchDomainTimeGate001
     * asserts the count stays 0. The ct table is the exception (real
     * keyed state): erase so expiry paths are observable. */
    ++g_stubDeleteCalls;
    if (map == &CT_MAP) {
        g_stubCtMap.erase(*static_cast<const struct ct_tuple *>(key));
    }
    (void)map;
    (void)key;
    return 0;
}

long bpf_skb_get_dev_name(struct __sk_buff *skb, char *name, __u32 name_size)
{
    /* Off-device there is no real netdev context. Default keeps the
     * "interface unreadable" semantics (match_interface fails → eval_error
     * path); hook-level tests that exercise the real rule path raise
     * g_stubDevNameOk to hand back a plausible name — the iface dimension
     * lookup misses either way, but the failure is no longer fatal. */
    (void)skb;
    if (g_stubDevNameOk && name != nullptr && name_size >= 4) {
        name[0] = 'w';
        name[1] = 'l';
        name[2] = '0';
        name[3] = '\0';
        return 0;
    }
    return -1;
}

int bpf_sock_tcp_send_reset(struct __sk_buff *skb)
{
    /* no kernel TCP stack off-device: count only, so hook tests can assert
     * "reset sent" vs "reset never sent" on the DROP paths */
    (void)skb;
    ++g_stubResetSent;
    return 0;
}

int bpf_skb_sock_destroy(struct __sk_buff *skb)
{
    /* no kernel sockets off-device: count only, so hook tests can assert
     * the F5 fast-fail destroy fired on the DNS deny paths (and only
     * there — ingress drops and non-query drops must leave it at zero). */
    (void)skb;
    ++g_stubSockDestroyed;
    return 0;
}

static void ResetDnsStubs()
{
    g_stubDefaultAction = {SK_PASS, SK_PASS};
    g_stubDomainV4Entry = {};
    g_stubDomainV4Ptr = nullptr;
    g_stubDomainV6Entry = {};
    g_stubDomainV6Ptr = nullptr;
    g_stubScratch[0] = {};
    g_stubScratch[1] = {};
    g_stubSocketUid = 200100;
    g_stubCurrentUid = 0;
    g_stubCurrentUidPresent = false;
    g_stubNowNs = 1000ULL * 1000ULL * 1000ULL * 1000ULL;
    g_stubDomainUpdates.clear();
    g_stubPending = {};
    /* default: a fresh, matched query record — the validated path's "happy"
     * default (mirrors the fuzz TU's permanently-fresh record). Tests that
     * need a miss set g_stubRecordPtr = nullptr themselves; the end-to-end
     * test overrides this with keyed mode. */
    g_stubRecordValue = {};
    g_stubRecordValue.uid = 1;
    g_stubRecordValue.appuid = 200100;
    g_stubRecordValue.timestamp = g_stubNowNs - 1000U; /* well inside the 60s window */
    g_stubRecordPtr = &g_stubRecordValue;
    g_stubKeyedRecord = false;
    g_stubExpectedRecordKey = {};
    g_stubRecordUpdates.clear();
    g_stubDnsServerV4Keys.clear();
    g_stubDnsServerV6Keys.clear();
    g_stubDnsServerVal = {};
    g_stubActionBitmap = {};
    g_stubActionBitmapPtr = nullptr;
    g_stubAcPrefixMap.clear();
    g_stubAcSuffixMap.clear();
    g_stubAcGen = 0;
    g_stubAcGenPresent = false;
    /* policy-chain (hook-level) state */
    g_stubCtMap.clear();
    g_stubNfScratch = {};
    g_stubZeroBitmap = {};
    g_stubLoopbackPresent = 0;
    g_stubDevNameOk = false;
    g_stubResetSent = 0;
    g_stubSockDestroyed = 0;
}

/* A' AC environment for the DNS sniffing tests: one wild ALLOW rule
 * under uid 1 — get_user_id() folds the stub's 200100 socket uid to 1 —
 * at appuid 0, so both the exact and the wild anchored scans hit. Gen-
 * present with EMPTY tables models "tables initialized, no rules": the
 * scans miss cleanly (no AC_PARSE_FAIL), the gate fills with
 * is_query=1/allowrst=0 and nothing stages or learns. */
static void AcDnsEnv(bool allowQueryDomain, bool denyQueryDomain = false)
{
    g_stubAcPrefixMap.clear();
    g_stubAcSuffixMap.clear();
    OHOS::NetManagerStandard::AcDomainTableBuilder builder;
    if (allowQueryDomain) {
        EXPECT_TRUE(builder.AddRule(1, 0, true, "www.example.com"));
    }
    if (denyQueryDomain) {
        EXPECT_TRUE(builder.AddRule(1, 0, false, "www.example.com"));
    }
    std::vector<OHOS::NetManagerStandard::AcTransEntry> prefix;
    std::vector<OHOS::NetManagerStandard::AcTransEntry> suffix;
    if (builder.Emit(1, prefix, suffix)) {
        for (const auto &e : prefix) {
            g_stubAcPrefixMap[e.key] = e.val;
        }
        for (const auto &e : suffix) {
            g_stubAcSuffixMap[e.key] = e.val;
        }
    }
    g_stubAcGen = 1;
    g_stubAcGenPresent = true;
}

/* The baidu shape of a double hit: a wildcard deny (*.example.com) and an
 * exact allow (www.example.com) are DISTINCT rules that both match the
 * test qname — denyrst=1 and allowrst=1 from different rules (the
 * same-domain both-actions merge variant is covered by the builder merge
 * tests). Used by the double-hit adjudication matrix. */
static void AcDnsEnvConflict()
{
    g_stubAcPrefixMap.clear();
    g_stubAcSuffixMap.clear();
    OHOS::NetManagerStandard::AcDomainTableBuilder builder;
    EXPECT_TRUE(builder.AddRule(1, 0, false, "*.example.com"));  /* wildcard deny */
    EXPECT_TRUE(builder.AddRule(1, 0, true, "www.example.com")); /* exact allow */
    std::vector<OHOS::NetManagerStandard::AcTransEntry> prefix;
    std::vector<OHOS::NetManagerStandard::AcTransEntry> suffix;
    if (builder.Emit(1, prefix, suffix)) {
        for (const auto &e : prefix) {
            g_stubAcPrefixMap[e.key] = e.val;
        }
        for (const auto &e : suffix) {
            g_stubAcSuffixMap[e.key] = e.val;
        }
    }
    g_stubAcGen = 1;
    g_stubAcGenPresent = true;
}

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace std;
using namespace testing::ext;
constexpr int32_t USER_ID1 = 100;
} // namespace

class NetsysBpfNetFirewallTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetsysBpfNetFirewallTest::SetUpTestCase()
{
    /* Suite-level device integration on the production singleton: load the
     * elf once (this pins every map the Write-family asserts against) and
     * start the ring-buffer listener thread. Test bodies NEVER use this
     * singleton — they get isolated instances via NewFirewallInstance(). */
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

void NetsysBpfNetFirewallTest::TearDownTestCase()
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

void NetsysBpfNetFirewallTest::SetUp()
{
    /* Canonical per-case start state: the load flag is CLASS-STATIC and
     * shared by every instance — no case may inherit another case's
     * toggle. (Constructing an instance resets it as well; doing it here
     * makes the contract explicit instead of ctor-side-effect dependent.) */
    NetsysBpfNetFirewall::GetInstance()->SetBpfLoaded(false);
}

void NetsysBpfNetFirewallTest::TearDown() {}

/* AddDomainCache(NetAddrInfo)001/002 were removed together with
 * the API — it was dead control-plane code that wrote all-zero domain_value
 * entries (uid=0/appuid=0) and had no production callers. Learning is written
 * by the BPF chain now and the pre-warm engine gets a proper
 * writer. The clear side stays covered by ClearDomainCache below and the new
 * ClearDnsQueryRecordMapNoCrash001. */

HWTEST_F(NetsysBpfNetFirewallTest, ClearFirewallDefaultAction001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    {
        FirewallLoadScope loaded(bpfNet, true);
        int ret =
            bpfNet->SetFirewallDefaultAction(USER_ID1, FirewallRuleAction::RULE_ALLOW, FirewallRuleAction::RULE_ALLOW);
        bpfNet->ClearFirewallDefaultAction();
        EXPECT_EQ(ret, FIREWALL_SUCCESS);
    }
    /* scope exit restored the not-loaded start state: the gate must refuse */
    int ret =
        bpfNet->SetFirewallDefaultAction(USER_ID1, FirewallRuleAction::RULE_ALLOW, FirewallRuleAction::RULE_ALLOW);
    EXPECT_EQ(ret, NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsCacheFlushProviderOnRuleChangeTest, TestSize.Level0)
{
    /* 规则变更后必须触发 DNS 缓存冲刷 provider(缓存旁路 deny 修复):
     * 注册计数 lambda, 任一 ClearFirewallRules 完成即被调用一次.
     * The provider registry is instance state — the fresh instance here
     * guarantees no residue for later cases, no manual unregister needed. */
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    int called = 0;
    bpfNet->RegisterDnsCacheFlushProvider([&called]() {
        ++called;
        return 2;
    });
    int ret = bpfNet->ClearFirewallRules(NetFirewallRuleType::RULE_ALL);
    EXPECT_EQ(ret, FIREWALL_SUCCESS);
    EXPECT_EQ(called, 1);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearFirewallRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
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
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();

    BitmapManager manager;

    /* An empty desired set is no longer rejected; it reconciles the bpf map
     * to empty. The fixture guarantees the maps are pinned (suite-level
     * LoadElf), so the write must succeed — a broken environment fails
     * loudly here instead of passing vacuously on a conditional. */
    int ret = bpfNet->WriteSrcPortBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(ret, 0);
    ret = bpfNet->WriteSrcPortBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(ret, 0);

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
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();

    BitmapManager manager;

    /* See WriteSrcPortBpfMap001: fixture-pinned maps, unconditional success. */
    int ret = bpfNet->WriteDstPortBpfMap(manager, NetFirewallRuleDirection::RULE_IN);
    EXPECT_EQ(ret, 0);
    ret = bpfNet->WriteDstPortBpfMap(manager, NetFirewallRuleDirection::RULE_OUT);
    EXPECT_EQ(ret, 0);

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
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();

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

HWTEST_F(NetsysBpfNetFirewallTest, GetDomainHashKey001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();
    DomainHashKey key = {};

    // empty domain
    EXPECT_FALSE(bpfNet->GetDomainHashKey("", key));
    EXPECT_EQ(key.prefixlen, 0);

    // domain with only '*'
    key = {};
    EXPECT_FALSE(bpfNet->GetDomainHashKey("***", key));
    EXPECT_EQ(key.prefixlen, 0);

    // domain too long
    key = {};
    std::string longDomain(DNS_DOMAIN_LEN + 1, 'a');
    EXPECT_FALSE(bpfNet->GetDomainHashKey(longDomain, key));
    EXPECT_EQ(key.prefixlen, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, GetDomainHashKey002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = std::make_shared<NetsysBpfNetFirewall>();
    DomainHashKey key = {};

    // isReversed=true should reverse the raw domain text before stripping '*'
    // "www.example.com" -> reversed "moc.elpmaxe.www"
    key = {};
    EXPECT_TRUE(bpfNet->GetDomainHashKey("www.example.com", key, true));
    uint32_t expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen("moc.elpmaxe.www")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, "moc.elpmaxe.www", strlen("moc.elpmaxe.www")), 0);

    // reversed suffix rule "*.example.com" -> strips '*' then reverses -> "moc.elpmaxe."
    key = {};
    EXPECT_TRUE(bpfNet->GetDomainHashKey("*.example.com", key, true));
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen("moc.elpmaxe.")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, "moc.elpmaxe.", strlen("moc.elpmaxe.")), 0);

    key = {};
    EXPECT_TRUE(bpfNet->GetDomainHashKey("*w.example.com", key, true));
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen("moc.elpmaxe.w")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, "moc.elpmaxe.w", strlen("moc.elpmaxe.w")), 0);

    // isReversed=false keeps original order ("www.example.*")
    key = {};
    EXPECT_TRUE(bpfNet->GetDomainHashKey("www.example.*", key, false));
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen(".www.example.")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, ".www.example.", strlen(".www.example.")), 0);

    key = {};
    EXPECT_TRUE(bpfNet->GetDomainHashKey("www.example.co*", key, false));
    expectedPrefixlen =
        static_cast<uint32_t>((sizeof(key.uid) + sizeof(key.appuid) + strlen(".www.example.co")) * BIT_PER_BYTE);
    EXPECT_EQ(key.prefixlen, expectedPrefixlen);
    EXPECT_EQ(memcmp(key.data, ".www.example.co", strlen(".www.example.co")), 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallDomainRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
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
    /* Fixture-pinned maps: the trailing conntrack clear must succeed. The
     * old conditional form (§10 M10) masked the map-less environment
     * instead of guaranteeing the map state — see firewall_test_env.h. */
    EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallDomainRules002, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    std::vector<sptr<NetFirewallDomainRule>> ruleList;

    sptr<NetFirewallDomainRule> rule = (std::make_unique<NetFirewallDomainRule>()).release();
    rule->userId = 100;
    rule->appUid = 1000;
    rule->ruleAction = FirewallRuleAction::RULE_DENY;

    NetFirewallDomainParam param;
    param.isWildcard = true;
    // prefix wildcard "*.example.com" -> prefix trie (name-start anchored)
    param.domain = "*.example.com";
    rule->domains.push_back(param);
    // prefix wildcard "*w.example.com" -> prefix trie
    param.domain = "*w.example.com";
    rule->domains.push_back(param);
    // suffix wildcard "www.example.*" -> suffix trie (terminator anchored)
    param.domain = "www.example.*";
    rule->domains.push_back(param);
    // suffix wildcard "www.example.c*" -> suffix trie
    param.domain = "www.example.c*";
    rule->domains.push_back(param);
    // no wildcard "www.example.com" -> suffix trie (bare domain + subdomains)
    param.isWildcard = false;
    param.domain = "www.example.com";
    rule->domains.push_back(param);

    ruleList.push_back(rule);
    int ret = bpfNet->SetFirewallDomainRules(ruleList);
    /* Fixture-pinned maps: every AC table write and the trailing ct clear
     * must succeed (see firewall_test_env.h for the isolation contract). */
    EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
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
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    int ret = bpfNet->RegisterCallback(nullptr);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, LoadSystemAbility001, TestSize.Level0)
{
    /* An INVALID SA id must fail deterministically in every environment:
     * samgr missing -> -1 at the manager probe; a live samgr has no
     * ability 99999 registered -> -1 at the ability probe. The old
     * either-value-passes form could not fail at all. */
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    int ret = bpfNet->LoadSystemAbility(99999);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallRules001, TestSize.Level0)
{
    /* Fresh instance starts not-loaded: the gate must refuse both an empty
     * and a populated rule list before any map is touched. */
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();

    std::vector<sptr<NetFirewallBaseRule>> ruleList;
    int ret = bpfNet->SetFirewallRules(NetFirewallRuleType::RULE_IP, ruleList, true);
    EXPECT_EQ(ret, NETFIREWALL_ERR);

    sptr<NetFirewallIpRule> rule = (std::make_unique<NetFirewallIpRule>()).release();
    ruleList.push_back(rule);
    ret = bpfNet->SetFirewallRules(NetFirewallRuleType::RULE_IP, ruleList, true);
    EXPECT_EQ(ret, NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallCurrentUserId001, TestSize.Level0)
{
    /* Fresh instance starts not-loaded: the gate must refuse. */
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    int ret = bpfNet->SetFirewallCurrentUserId(100);
    EXPECT_EQ(ret, NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearFirewallDefaultAction002, TestSize.Level0)
{
    /* Fresh instance starts not-loaded: the gate must refuse. */
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    int ret = bpfNet->ClearFirewallDefaultAction();
    EXPECT_EQ(ret, NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallDefaultAction002, TestSize.Level0)
{
    /* Fresh instance starts not-loaded: the gate must refuse. */
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
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

HWTEST_F(NetsysBpfNetFirewallTest, SetEvalFallbackMode001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    /* invalid modes are rejected regardless of the bpf state */
    EXPECT_EQ(bpfNet->SetEvalFallbackMode(2u), NETFIREWALL_ERR);
    EXPECT_EQ(bpfNet->SetEvalFallbackMode(0xFFFFFFFFu), NETFIREWALL_ERR);
    /* fresh instance starts not-loaded: valid modes are still refused by the gate */
    EXPECT_EQ(bpfNet->SetEvalFallbackMode(EVAL_FALLBACK_OBSERVE), NETFIREWALL_ERR);
    EXPECT_EQ(bpfNet->SetEvalFallbackMode(EVAL_FALLBACK_FAIL_CLOSED), NETFIREWALL_ERR);
    {
        FirewallLoadScope loaded(bpfNet, true);
        /* Fixture-pinned maps: the mode write must succeed — a broken
         * environment fails loudly here instead of passing vacuously on a
         * conditional. */
        int ret = bpfNet->SetEvalFallbackMode(EVAL_FALLBACK_OBSERVE);
        EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
        /* restore the secure default (map default is fail-closed) — the
         * restore is checked unconditionally: an unchecked restore failing
         * on-device would leave the process in OBSERVE (fail-open) mode
         * while the test stays green. */
        EXPECT_EQ(bpfNet->SetEvalFallbackMode(EVAL_FALLBACK_FAIL_CLOSED), NETFIREWALL_SUCCESS);
    }
}

HWTEST_F(NetsysBpfNetFirewallTest, CtEntryEvaluatedBits001, TestSize.Level0)
{
    /* The two evaluated bits must live in the flag byte of ct_entry
     * (offset 4, bits 3/4, little-endian bit order) and the value size must stay
     * 16 bytes — the CT map value layout is ABI for existing entries. */
    static_assert(sizeof(struct ct_entry) == 16, "ct_entry size changed");
    static_assert(offsetof(struct ct_entry, tx_seen_flag) == 5, "flag byte must stay at offset 4");
    struct ct_entry entry = {0};
    __u8 *flagByte = reinterpret_cast<__u8 *>(&entry) + 4;
    /* cross-lock: setting the bit-field member must produce exactly the
     * access mask used by the BPF helpers (netfirewall_ct.h) */
    entry.egress_evaluated = 1;
    EXPECT_EQ(*flagByte, CT_EGRESS_EVALUATED_MASK);
    entry = {0};
    entry.ingress_evaluated = 1;
    EXPECT_EQ(*flagByte, CT_INGRESS_EVALUATED_MASK);
    entry = {0};
    entry.rx_closing_flag = 1;
    entry.tx_closing_flag = 1;
    entry.seen_non_syn = 1;
    entry.egress_evaluated = 1;
    entry.ingress_evaluated = 1;
    EXPECT_EQ(*flagByte, (__u8)(0x07 | CT_EGRESS_EVALUATED_MASK | CT_INGRESS_EVALUATED_MASK));
    EXPECT_EQ(CT_EGRESS_EVALUATED_MASK, (__u8)0x08);
    EXPECT_EQ(CT_INGRESS_EVALUATED_MASK, (__u8)0x10);
}

HWTEST_F(NetsysBpfNetFirewallTest, FirewallReservedKeyConstants001, TestSize.Level0)
{
    /* 常量共享头一致性: user-side (netfirewall_parcel.h) reserved keys must
     * match the BPF-side sentinel (netfirewall_types.h UID_UNATTRIBUTED). */
    EXPECT_EQ(FIREWALL_SYSTEM_USER_ID, 1);
    EXPECT_EQ(FIREWALL_UNATTRIBUTED_USER_ID, -1);
    EXPECT_EQ(static_cast<uint32_t>(FIREWALL_UNATTRIBUTED_USER_ID), UID_UNATTRIBUTED);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallReservedKeyAction001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    /* Reserved keys are normal write keys: rejected only when bpf is
     * not loaded, never by a userId range check. Fresh instance starts
     * not-loaded: the gate must refuse both reserved keys. */
    EXPECT_EQ(bpfNet->SetFirewallDefaultAction(FIREWALL_SYSTEM_USER_ID, FirewallRuleAction::RULE_ALLOW,
                                               FirewallRuleAction::RULE_ALLOW),
              NETFIREWALL_ERR);
    EXPECT_EQ(bpfNet->SetFirewallDefaultAction(FIREWALL_UNATTRIBUTED_USER_ID, FirewallRuleAction::RULE_ALLOW,
                                               FirewallRuleAction::RULE_ALLOW),
              NETFIREWALL_ERR);
    {
        FirewallLoadScope loaded(bpfNet, true);
        /* Fixture-pinned maps: the reserved-key write must succeed. */
        int ret = bpfNet->SetFirewallDefaultAction(FIREWALL_UNATTRIBUTED_USER_ID, FirewallRuleAction::RULE_ALLOW,
                                                   FirewallRuleAction::RULE_ALLOW);
        EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
    }
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearFirewallDefaultActionReserved001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    /* The clear no longer uses a fake -1 key — it sweeps user entries
     * and keeps the reserved ones. Fresh instance starts not-loaded: the
     * gate must refuse. */
    EXPECT_EQ(bpfNet->ClearFirewallDefaultAction(), NETFIREWALL_ERR);
    {
        FirewallLoadScope loaded(bpfNet, true);
        /* Fixture-pinned maps: the reserved-key-surviving sweep must
         * succeed (reserved-key survival itself is a device matrix item). */
        int ret = bpfNet->ClearFirewallDefaultAction();
        EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
    }
}

HWTEST_F(NetsysBpfNetFirewallTest, WriteLoopBackBpfMap001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    /* Fixture-pinned maps: both loopback entries must be written — a broken
     * environment fails loudly instead of either-value-passes. */
    int ret = bpfNet->WriteLoopBackBpfMap();
    EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
}

HWTEST_F(NetsysBpfNetFirewallTest, SetFirewallIpRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
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
    /* Fixture-pinned maps: the full-replace reconcile must succeed — a broken
     * environment fails loudly instead of either-value-passes. */
    EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearDomainRules001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NewFirewallInstance();
    /* Smoke: the full-replace clear must run and not crash in both
     * map-live and map-less environments (the legacy LPM map writes were
     * removed with their BPF readers; the AC table clear is map-less
     * tolerant by design). */
    bpfNet->ClearDomainRules();
    EXPECT_NE(bpfNet, nullptr);
}

namespace {
struct InterceptCapture {
    bool called = false;
    uint16_t localPort = 0;
    uint16_t remotePort = 0;
    uint16_t protocol = 0;
    uint8_t family = 0;
    std::string localIp;
    std::string remoteIp;
    int32_t appUid = 0;
    std::string domain;
};

InterceptEvent MakeEgressTcpEvent()
{
    InterceptEvent info{};
    info.dir = EGRESS;
    info.protocol = IPPROTO_TCP;
    info.family = AF_INET6;
    int sport = 51302;
    int dport = 12345;
    info.sport = htons(sport);
    info.dport = htons(dport);
    return info;
}

class TestInterceptCallbackStub : public OHOS::NetsysNative::NetFirewallCallbackStub {
public:
    int32_t OnIntercept(OHOS::sptr<InterceptRecord> &record) override
    {
        if (record == nullptr) {
            return -1;
        }
        called = true;
        localIp = record->localIp;
        remoteIp = record->remoteIp;
        localPort = record->localPort;
        remotePort = record->remotePort;
        return 0;
    }

    bool called = false;
    std::string localIp;
    std::string remoteIp;
    uint16_t localPort = 0;
    uint16_t remotePort = 0;
};
} // namespace

HWTEST_F(NetsysBpfNetFirewallTest, RegisterInterceptHandler001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    InterceptCapture capture;
    bpfNet->RegisterInterceptHandler([&capture](const InterceptRecord &record, uint8_t family) {
        capture.called = true;
        capture.localPort = record.localPort;
        capture.remotePort = record.remotePort;
        capture.protocol = record.protocol;
        capture.family = family;
        capture.localIp = record.localIp;
        capture.remoteIp = record.remoteIp;
        capture.appUid = record.appUid;
        capture.domain = record.domain;
    });

    OHOS::sptr<InterceptRecord> record = OHOS::sptr<InterceptRecord>::MakeSptr();
    record->localIp = "2001:db8::1";
    record->remoteIp = "2001:db8::2";
    record->localPort = 12345;
    record->remotePort = 51302;
    record->protocol = IPPROTO_TCP;
    record->appUid = 1001;
    record->time = 123;
    record->domain = "example.com";

    bpfNet->NotifyEgressIntercept(MakeEgressTcpEvent(), record);
    EXPECT_TRUE(capture.called);
    EXPECT_EQ(capture.localPort, 12345);
    EXPECT_EQ(capture.remotePort, 51302);
    EXPECT_EQ(capture.protocol, IPPROTO_TCP);
    EXPECT_EQ(capture.family, AF_INET6);
    EXPECT_EQ(capture.localIp, "2001:db8::1");
    EXPECT_EQ(capture.remoteIp, "2001:db8::2");
    EXPECT_EQ(capture.appUid, 1001);
    EXPECT_EQ(capture.domain, "example.com");
    bpfNet->RegisterInterceptHandler(nullptr);
}

HWTEST_F(NetsysBpfNetFirewallTest, NotifyInterceptEventEgress001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    InterceptCapture handlerCapture;
    bpfNet->RegisterInterceptHandler([&handlerCapture](const InterceptRecord &record, uint8_t family) {
        handlerCapture.called = true;
        handlerCapture.localPort = record.localPort;
        handlerCapture.remotePort = record.remotePort;
        handlerCapture.localIp = record.localIp;
        handlerCapture.remoteIp = record.remoteIp;
        handlerCapture.family = family;
    });
    OHOS::sptr<TestInterceptCallbackStub> callback = OHOS::sptr<TestInterceptCallbackStub>::MakeSptr();
    OHOS::sptr<OHOS::NetsysNative::INetFirewallCallback> baseCallback = callback;
    ASSERT_EQ(bpfNet->RegisterCallback(baseCallback), 0);

    InterceptEvent info{};
    info.dir = EGRESS;
    info.family = AF_INET;
    info.protocol = IPPROTO_TCP;
    info.sport = htons(443);
    info.dport = htons(51302);
    ASSERT_EQ(inet_pton(AF_INET, "93.184.216.34", &info.ipv4.saddr), 1);
    ASSERT_EQ(inet_pton(AF_INET, "192.168.1.5", &info.ipv4.daddr), 1);
    bpfNet->NotifyInterceptEvent(&info);

    EXPECT_TRUE(callback->called);
    EXPECT_EQ(callback->localIp, "192.168.1.5");
    EXPECT_EQ(callback->localPort, 51302);
    EXPECT_EQ(callback->remoteIp, "93.184.216.34");
    EXPECT_EQ(callback->remotePort, 443);
    EXPECT_TRUE(handlerCapture.called);
    EXPECT_EQ(handlerCapture.localIp, "192.168.1.5");
    EXPECT_EQ(handlerCapture.localPort, 51302);
    EXPECT_EQ(handlerCapture.remoteIp, "93.184.216.34");
    EXPECT_EQ(handlerCapture.remotePort, 443);
    EXPECT_EQ(handlerCapture.family, AF_INET);

    bpfNet->RegisterInterceptHandler(nullptr);
    bpfNet->UnregisterCallback(baseCallback);
}

HWTEST_F(NetsysBpfNetFirewallTest, NotifyInterceptEventIngress001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    bool handlerCalled = false;
    bpfNet->RegisterInterceptHandler([&handlerCalled](const InterceptRecord &, uint8_t) { handlerCalled = true; });
    OHOS::sptr<TestInterceptCallbackStub> callback = OHOS::sptr<TestInterceptCallbackStub>::MakeSptr();
    OHOS::sptr<OHOS::NetsysNative::INetFirewallCallback> baseCallback = callback;
    ASSERT_EQ(bpfNet->RegisterCallback(baseCallback), 0);

    InterceptEvent info{};
    info.dir = INGRESS;
    info.family = AF_INET;
    info.protocol = IPPROTO_TCP;
    info.sport = htons(80);
    info.dport = htons(5684);
    ASSERT_EQ(inet_pton(AF_INET, "192.168.8.116", &info.ipv4.saddr), 1);
    ASSERT_EQ(inet_pton(AF_INET, "192.168.1.5", &info.ipv4.daddr), 1);
    bpfNet->NotifyInterceptEvent(&info);

    EXPECT_TRUE(callback->called);
    EXPECT_EQ(callback->localIp, "192.168.8.116");
    EXPECT_EQ(callback->localPort, 80);
    EXPECT_EQ(callback->remoteIp, "192.168.1.5");
    EXPECT_EQ(callback->remotePort, 5684);
    EXPECT_FALSE(handlerCalled);

    bpfNet->RegisterInterceptHandler(nullptr);
    bpfNet->UnregisterCallback(baseCallback);
}

HWTEST_F(NetsysBpfNetFirewallTest, NotifyEgressInterceptIngress001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    bool called = false;
    bpfNet->RegisterInterceptHandler([&called](const InterceptRecord &, uint8_t) { called = true; });
    InterceptEvent info = MakeEgressTcpEvent();
    info.dir = INGRESS;
    bpfNet->NotifyEgressIntercept(info, OHOS::sptr<InterceptRecord>::MakeSptr());
    EXPECT_FALSE(called);
    bpfNet->RegisterInterceptHandler(nullptr);
}

HWTEST_F(NetsysBpfNetFirewallTest, NotifyEgressInterceptUdp001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    bool called = false;
    bpfNet->RegisterInterceptHandler([&called](const InterceptRecord &, uint8_t) { called = true; });
    /* Protocol-complete fast-fail (spec 2026-11-16): a UDP egress intercept
     * (the denied DNS query path — the musl resolver socket) must also reach
     * the registered handler for the netlink socket destroy, exactly like
     * TCP; no interception point may fall back to a timeout-style slow
     * failure. */
    InterceptEvent info = MakeEgressTcpEvent();
    info.protocol = IPPROTO_UDP;
    bpfNet->NotifyEgressIntercept(info, OHOS::sptr<InterceptRecord>::MakeSptr());
    EXPECT_TRUE(called);
    bpfNet->RegisterInterceptHandler(nullptr);
}

HWTEST_F(NetsysBpfNetFirewallTest, NotifyEgressInterceptNullRecord001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    bool called = false;
    bpfNet->RegisterInterceptHandler([&called](const InterceptRecord &, uint8_t) { called = true; });
    bpfNet->NotifyEgressIntercept(MakeEgressTcpEvent(), nullptr);
    EXPECT_FALSE(called);
    bpfNet->RegisterInterceptHandler(nullptr);
}

HWTEST_F(NetsysBpfNetFirewallTest, NotifyEgressInterceptNoHandler001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    bpfNet->RegisterInterceptHandler(nullptr);
    bpfNet->NotifyEgressIntercept(MakeEgressTcpEvent(), OHOS::sptr<InterceptRecord>::MakeSptr());
    EXPECT_TRUE(true);
}

HWTEST_F(NetsysBpfNetFirewallTest, RegisterInterceptHandlerOverwrite001, TestSize.Level1)
{
    auto bpfNet = NewFirewallInstance();
    bool firstCalled = false;
    bool secondCalled = false;
    bpfNet->RegisterInterceptHandler([&firstCalled](const InterceptRecord &, uint8_t) { firstCalled = true; });
    bpfNet->RegisterInterceptHandler([&secondCalled](const InterceptRecord &, uint8_t) { secondCalled = true; });
    bpfNet->NotifyEgressIntercept(MakeEgressTcpEvent(), OHOS::sptr<InterceptRecord>::MakeSptr());
    EXPECT_FALSE(firstCalled);
    EXPECT_TRUE(secondCalled);
    bpfNet->RegisterInterceptHandler(nullptr);
}

/* ===== unified tri-state L4 parser unit tests (AH unit / bounds defense /
 * matrix rows 1-3 basis). The real parser header (netfirewall_utils.h) is compiled
 * into this TU and fed crafted packets through the mock skb below. ===== */
namespace {
/* mock skb: data = buffer address, len = buffer length (see global-scope stub above) */
struct MockSkb {
    struct __sk_buff skb = {};
    std::vector<uint8_t> buf;
    void Reset(int family)
    {
        buf.clear();
        memset(&skb, 0, sizeof(skb));
        skb.family = family;
    }
    void Append(const std::vector<uint8_t> &bytes)
    {
        buf.insert(buf.end(), bytes.begin(), bytes.end());
    }
    struct __sk_buff *Done()
    {
        /* the 32-bit skb->data field cannot hold a pointer: publish the buffer
         * through the side global consumed by the stub; skb->len/family stay
         * in the struct because the parser reads them directly */
        g_mockPacketData = buf.data();
        g_mockPacketLen = static_cast<uint32_t>(buf.size());
        skb.len = static_cast<uint32_t>(buf.size());
        return &skb;
    }
};

/* IPv4 header builder: ihlBytes must be a multiple of 4 >= 20 */
std::vector<uint8_t> Ipv4Header(size_t ihlBytes, uint8_t proto, uint16_t totLen, uint16_t fragOffField)
{
    std::vector<uint8_t> h(ihlBytes, 0);
    h[0] = static_cast<uint8_t>((4U << 4) | (ihlBytes / 4)); /* version | IHL */
    h[2] = static_cast<uint8_t>(totLen >> 8);                /* tot_len (BE) */
    h[3] = static_cast<uint8_t>(totLen & 0xFF);
    h[6] = static_cast<uint8_t>(fragOffField >> 8); /* flags + frag offset (BE) */
    h[7] = static_cast<uint8_t>(fragOffField & 0xFF);
    h[8] = 64; /* ttl */
    h[9] = proto;
    return h;
}

/* 20-byte TCP header; flags byte lives at offset 13 */
std::vector<uint8_t> TcpHeader(uint16_t sport, uint16_t dport, uint8_t flags)
{
    std::vector<uint8_t> h(20, 0);
    h[0] = static_cast<uint8_t>(sport >> 8);
    h[1] = static_cast<uint8_t>(sport & 0xFF);
    h[2] = static_cast<uint8_t>(dport >> 8);
    h[3] = static_cast<uint8_t>(dport & 0xFF);
    h[13] = flags;
    return h;
}

/* 8-byte UDP header */
std::vector<uint8_t> UdpHeader(uint16_t sport, uint16_t dport)
{
    std::vector<uint8_t> h(8, 0);
    h[0] = static_cast<uint8_t>(sport >> 8);
    h[1] = static_cast<uint8_t>(sport & 0xFF);
    h[2] = static_cast<uint8_t>(dport >> 8);
    h[3] = static_cast<uint8_t>(dport & 0xFF);
    return h;
}

/* ===== DNS packet builders: real wire-format packets fed to the
 * real match_dns_query so the parse/learn chain runs end to end. ===== */

/* 12-byte DNS header (id/flags/qdcount/ancount, all big endian) */
std::vector<uint8_t> DnsHeader(uint16_t txid, uint16_t flags, uint16_t qd, uint16_t an)
{
    std::vector<uint8_t> h(12, 0);
    h[0] = static_cast<uint8_t>(txid >> 8);
    h[1] = static_cast<uint8_t>(txid & 0xFF);
    h[2] = static_cast<uint8_t>(flags >> 8);
    h[3] = static_cast<uint8_t>(flags & 0xFF);
    h[4] = static_cast<uint8_t>(qd >> 8);
    h[5] = static_cast<uint8_t>(qd & 0xFF);
    h[6] = static_cast<uint8_t>(an >> 8);
    h[7] = static_cast<uint8_t>(an & 0xFF);
    return h;
}

/* one question: labels + root + type + class IN (the sniff wire format) */
void AppendDnsQuery(std::vector<uint8_t> &p, const std::string &qname, uint16_t type)
{
    size_t start = 0;
    while (start < qname.size()) {
        size_t dot = qname.find('.', start);
        if (dot == std::string::npos) {
            dot = qname.size();
        }
        size_t labelLen = dot - start;
        p.push_back(static_cast<uint8_t>(labelLen));
        p.insert(p.end(), qname.begin() + start, qname.begin() + dot);
        start = dot + 1;
    }
    p.push_back(0); /* root label */
    p.push_back(static_cast<uint8_t>(type >> 8));
    p.push_back(static_cast<uint8_t>(type & 0xFF));
    p.push_back(0);
    p.push_back(1); /* class IN */
}

/* one answer: 2-byte name compression pointer (parse_answers skips a u16 name)
 * + type + class IN + ttl + rdlen + rdata */
void AppendDnsAnswer(std::vector<uint8_t> &p, uint16_t type, uint32_t ttl, const std::vector<uint8_t> &rdata)
{
    p.push_back(0xC0);
    p.push_back(0x0C);
    p.push_back(static_cast<uint8_t>(type >> 8));
    p.push_back(static_cast<uint8_t>(type & 0xFF));
    p.push_back(0);
    p.push_back(1); /* class IN */
    p.push_back(static_cast<uint8_t>(ttl >> 24));
    p.push_back(static_cast<uint8_t>((ttl >> 16) & 0xFF));
    p.push_back(static_cast<uint8_t>((ttl >> 8) & 0xFF));
    p.push_back(static_cast<uint8_t>(ttl & 0xFF));
    p.push_back(static_cast<uint8_t>(rdata.size() >> 8));
    p.push_back(static_cast<uint8_t>(rdata.size() & 0xFF));
    p.insert(p.end(), rdata.begin(), rdata.end());
}

/* IPv4 header carrying explicit addresses (flow assembly reads offsets 12-19) */
std::vector<uint8_t> Ipv4HeaderAddr(uint8_t proto, uint16_t totLen, uint32_t saddr, uint32_t daddr)
{
    std::vector<uint8_t> h = Ipv4Header(20, proto, totLen, 0);
    h[12] = static_cast<uint8_t>(saddr >> 24);
    h[13] = static_cast<uint8_t>((saddr >> 16) & 0xFF);
    h[14] = static_cast<uint8_t>((saddr >> 8) & 0xFF);
    h[15] = static_cast<uint8_t>(saddr & 0xFF);
    h[16] = static_cast<uint8_t>(daddr >> 24);
    h[17] = static_cast<uint8_t>((daddr >> 16) & 0xFF);
    h[18] = static_cast<uint8_t>((daddr >> 8) & 0xFF);
    h[19] = static_cast<uint8_t>(daddr & 0xFF);
    return h;
}

/* 40-byte IPv6 base header; nexthdr at byte 6 */
std::vector<uint8_t> Ipv6Header(uint8_t nexthdr, uint16_t payloadLen)
{
    std::vector<uint8_t> h(40, 0);
    h[0] = 0x60;
    h[4] = static_cast<uint8_t>(payloadLen >> 8);
    h[5] = static_cast<uint8_t>(payloadLen & 0xFF);
    h[6] = nexthdr;
    return h;
}

/* generic extension header (HBH/routing/destopts: advance = 8*(hdrlen+1)) */
std::vector<uint8_t> ExtHeader(uint8_t nexthdr, uint8_t hdrlen, size_t totalBytes)
{
    std::vector<uint8_t> h(totalBytes, 0);
    h[0] = nexthdr;
    h[1] = hdrlen;
    return h;
}

/* 8-byte IPv6 fragment header: nexthdr / reserved / frag_off(BE) / id */
std::vector<uint8_t> FragHeader(uint8_t nexthdr, uint16_t fragOffField)
{
    std::vector<uint8_t> h(8, 0);
    h[0] = nexthdr;
    h[2] = static_cast<uint8_t>(fragOffField >> 8);
    h[3] = static_cast<uint8_t>(fragOffField & 0xFF);
    return h;
}
} // namespace

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv4Basic001, TestSize.Level0)
{
    /* anchor: IHL=5, no options — the parser must locate L4 exactly like the
     * old fixed offset (offset 20) and the real ports must be read. */
    MockSkb mock;
    mock.Reset(AF_INET);
    mock.Append(Ipv4Header(20, IPPROTO_TCP, 40, 0));
    mock.Append(TcpHeader(1234, 443, 0x10));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 20U);
    EXPECT_EQ(l4.protocol, IPPROTO_TCP);
    __be16 sport = 0;
    __be16 dport = 0;
    EXPECT_TRUE(load_l4_ports(mock.Done(), l4.l4_nhoff, IPPROTO_TCP, &sport, &dport));
    EXPECT_EQ(sport, htons(1234));
    EXPECT_EQ(dport, htons(443));
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv4Options001, TestSize.Level0)
{
    /* matrix row 1 basis: IHL=6 with NOP options — the option bytes must NOT be
     * read as ports (the old fixed offset read dport = 0x0101). */
    MockSkb mock;
    mock.Reset(AF_INET);
    auto h = Ipv4Header(24, IPPROTO_TCP, 44, 0);
    h[20] = 0x01; /* NOP option bytes 20-23 */
    h[21] = 0x01;
    h[22] = 0x01;
    h[23] = 0x01;
    mock.Append(h);
    mock.Append(TcpHeader(1000, 443, 0x02));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 24U);
    __be16 sport = 0;
    __be16 dport = 0;
    EXPECT_TRUE(load_l4_ports(mock.Done(), l4.l4_nhoff, IPPROTO_TCP, &sport, &dport));
    EXPECT_EQ(sport, htons(1000));
    EXPECT_EQ(dport, htons(443));
    EXPECT_NE(dport, static_cast<__be16>(0x0101));
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv4ForgedRst001, TestSize.Level0)
{
    /* matrix row 2 basis: IHL=14 (56-byte header), a 0x04 byte planted at the OLD fixed flags
     * position (20+13=33, pure option area) — RST must come from the real header
     * at 56+13=69 (ACK only), so rst == 0 and the forged vector is dead. */
    MockSkb mock;
    mock.Reset(AF_INET);
    auto h = Ipv4Header(56, IPPROTO_TCP, 76, 0);
    h[33] = 0x04; /* forged RST bit where the old code read flags */
    mock.Append(h);
    mock.Append(TcpHeader(5000, 80, 0x10)); /* real flags: ACK, no RST */
    ASSERT_EQ(mock.buf[33], 0x04);          /* the trap is armed as designed */
    ASSERT_EQ(mock.buf[69], 0x10);
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 56U);
    __u16 rst = 1;
    EXPECT_TRUE(load_l4_header_flags(mock.Done(), l4.l4_nhoff, &rst));
    EXPECT_EQ(rst, 0U);
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv4Malformed001, TestSize.Level0)
{
    /* Bad version / IHL < 5 are UNPARSEABLE instead of being read at wrong offsets */
    MockSkb mock;
    mock.Reset(AF_INET);
    auto badVersion = Ipv4Header(20, IPPROTO_TCP, 40, 0);
    badVersion[0] = 0x65; /* version 6 */
    mock.Append(badVersion);
    mock.Append(TcpHeader(1, 2, 0));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_UNPARSEABLE);

    MockSkb mock2;
    mock2.Reset(AF_INET);
    auto badIhl = Ipv4Header(20, IPPROTO_TCP, 40, 0);
    badIhl[0] = 0x44; /* IHL=4 */
    mock2.Append(badIhl);
    mock2.Append(TcpHeader(1, 2, 0));
    struct l4_info l42 = {};
    ASSERT_TRUE(parse_l4(mock2.skb.family, mock2.Done(), &l42));
    EXPECT_EQ(l42.state, L4_PARSE_UNPARSEABLE);
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv4LengthDefense001, TestSize.Level0)
{
    /* Declared tot_len too short for the port prefix, and a truncated skb —
     * both UNPARSEABLE, never a partial misread (bounds first, reads second). */
    MockSkb mock;
    mock.Reset(AF_INET);
    mock.Append(Ipv4Header(20, IPPROTO_TCP, 22, 0)); /* tot_len 22 < 20+4 */
    mock.Append(TcpHeader(1, 2, 0));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_UNPARSEABLE);

    MockSkb mock2;
    mock2.Reset(AF_INET);
    mock2.buf.resize(10); /* shorter than the IP base header itself */
    struct l4_info l42 = {};
    ASSERT_TRUE(parse_l4(mock2.skb.family, mock2.Done(), &l42));
    EXPECT_EQ(l42.state, L4_PARSE_UNPARSEABLE);
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv4Fragment001, TestSize.Level0)
{
    /* matrix row 6 basis: non-first fragment -> NO_L4; first fragment with MF set
     * still parses (frag offset 0). */
    MockSkb mock;
    mock.Reset(AF_INET);
    mock.Append(Ipv4Header(20, IPPROTO_TCP, 40, 0x0001)); /* frag offset = 8 bytes */
    mock.Append(TcpHeader(1, 2, 0));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_NO_L4);

    MockSkb mock2;
    mock2.Reset(AF_INET);
    mock2.Append(Ipv4Header(20, IPPROTO_TCP, 40, 0x2000)); /* MF set, offset 0: first */
    mock2.Append(TcpHeader(1, 2, 0));
    struct l4_info l42 = {};
    ASSERT_TRUE(parse_l4(mock2.skb.family, mock2.Done(), &l42));
    EXPECT_EQ(l42.state, L4_PARSE_OK);
    EXPECT_EQ(l42.l4_nhoff, 20U);
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv6Basic001, TestSize.Level0)
{
    /* anchor: no extension headers — offset 40 exactly like the old code. */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv6Header(IPPROTO_UDP, 12));
    mock.Append(UdpHeader(5353, 53));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 40U);
    EXPECT_EQ(l4.protocol, IPPROTO_UDP);
    __be16 sport = 0;
    __be16 dport = 0;
    EXPECT_TRUE(load_l4_ports(mock.Done(), l4.l4_nhoff, IPPROTO_UDP, &sport, &dport));
    EXPECT_EQ(sport, htons(5353));
    EXPECT_EQ(dport, htons(53));
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv6ExtChain001, TestSize.Level0)
{
    /* matrix rows 3/5 basis: HBH(8) + routing(16) + destopts(8) then UDP — real
     * ports at 40+32; the old code read ports at 40 (inside the HBH header). */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv6Header(IPPROTO_HOPOPTS, 0));
    mock.Append(ExtHeader(IPPROTO_ROUTING, 0, 8));  /* HBH: next=routing, 8B */
    mock.Append(ExtHeader(IPPROTO_DSTOPTS, 1, 16)); /* routing: next=destopts, 8*(1+1)=16B */
    mock.Append(ExtHeader(IPPROTO_UDP, 0, 8));      /* destopts: next=UDP, 8B */
    mock.Append(UdpHeader(40000, 8080));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 72U);
    EXPECT_EQ(l4.protocol, IPPROTO_UDP);
    __be16 sport = 0;
    __be16 dport = 0;
    EXPECT_TRUE(load_l4_ports(mock.Done(), l4.l4_nhoff, IPPROTO_UDP, &sport, &dport));
    EXPECT_EQ(sport, htons(40000));
    EXPECT_EQ(dport, htons(8080));
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv6AhUnit001, TestSize.Level0)
{
    /* (the classic trap): AH advances 4*(hdrlen+2) — hdrlen=1 means 12 bytes,
     * NOT 8*(1+1)=16. HBH(8) + AH(12) + UDP puts L4 at 60; with the wrong unit
     * the parser would land at 64 and read the wrong ports. */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv6Header(IPPROTO_HOPOPTS, 0));
    mock.Append(ExtHeader(IPPROTO_AH, 0, 8));   /* HBH: next=AH, 8B */
    mock.Append(ExtHeader(IPPROTO_UDP, 1, 12)); /* AH: next=UDP, 4*(1+2)=12B */
    mock.Append(UdpHeader(7777, 9999));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 60U);
    __be16 sport = 0;
    __be16 dport = 0;
    EXPECT_TRUE(load_l4_ports(mock.Done(), l4.l4_nhoff, IPPROTO_UDP, &sport, &dport));
    EXPECT_EQ(sport, htons(7777));
    EXPECT_EQ(dport, htons(9999));
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv6Fragment001, TestSize.Level0)
{
    /* matrix row 6 basis: v6 fragment header — first fragment continues with the
     * inner nexthdr, non-first is NO_L4. */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv6Header(IPPROTO_FRAGMENT, 0));
    mock.Append(FragHeader(IPPROTO_UDP, 0x0000)); /* first fragment */
    mock.Append(UdpHeader(1111, 2222));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 48U);
    EXPECT_EQ(l4.protocol, IPPROTO_UDP);

    MockSkb mock2;
    mock2.Reset(AF_INET6);
    mock2.Append(Ipv6Header(IPPROTO_FRAGMENT, 0));
    mock2.Append(FragHeader(IPPROTO_UDP, 0x0001)); /* non-first */
    mock2.Append(UdpHeader(1111, 2222));
    struct l4_info l42 = {};
    ASSERT_TRUE(parse_l4(mock2.skb.family, mock2.Done(), &l42));
    EXPECT_EQ(l42.state, L4_PARSE_NO_L4);
    EXPECT_EQ(l42.protocol, IPPROTO_UDP); /* best-effort inner protocol */
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv6EspUnknown001, TestSize.Level0)
{
    /* matrix row 7 basis: ESP and unknown next headers are UNPARSEABLE (never
     * evaluated with misread values — decided by the chain instead). */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv6Header(IPPROTO_ESP, 16));
    mock.buf.resize(mock.buf.size() + 16, 0);
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_UNPARSEABLE);

    MockSkb mock2;
    mock2.Reset(AF_INET6);
    mock2.Append(Ipv6Header(200, 8)); /* unknown next header */
    mock2.Append(UdpHeader(1, 2));
    struct l4_info l42 = {};
    ASSERT_TRUE(parse_l4(mock2.skb.family, mock2.Done(), &l42));
    EXPECT_EQ(l42.state, L4_PARSE_UNPARSEABLE);
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv6ChainLimit001, TestSize.Level0)
{
    /* Seven HBH hops still reach the terminal protocol; eight ext headers
     * never do (bounded loop exits as UNPARSEABLE). */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv6Header(IPPROTO_HOPOPTS, 0));
    for (int i = 0; i < 6; i++) {
        mock.Append(ExtHeader(IPPROTO_HOPOPTS, 0, 8)); /* six chained HBH */
    }
    mock.Append(ExtHeader(IPPROTO_TCP, 0, 8)); /* 7th HBH: next=TCP */
    mock.Append(TcpHeader(1, 2, 0));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 40U + 7U * 8U);
    EXPECT_EQ(l4.protocol, IPPROTO_TCP);

    MockSkb mock2;
    mock2.Reset(AF_INET6);
    mock2.Append(Ipv6Header(IPPROTO_HOPOPTS, 0));
    for (int i = 0; i < 7; i++) {
        mock2.Append(ExtHeader(IPPROTO_HOPOPTS, 0, 8)); /* seven chained HBH */
    }
    mock2.Append(ExtHeader(IPPROTO_TCP, 0, 8)); /* 8th HBH: next=TCP */
    mock2.Append(TcpHeader(1, 2, 0));
    struct l4_info l42 = {};
    ASSERT_TRUE(parse_l4(mock2.skb.family, mock2.Done(), &l42));
    EXPECT_EQ(l42.state, L4_PARSE_UNPARSEABLE);
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4Ipv6OffsetCap001, TestSize.Level0)
{
    /* A 128-byte extension region is the hard cap — exactly 128 passes, 256
     * (two max-size routing headers) is UNPARSEABLE. */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv6Header(IPPROTO_ROUTING, 0));
    mock.Append(ExtHeader(IPPROTO_TCP, 15, 128)); /* 8*(15+1) = 128: at the cap */
    mock.Append(TcpHeader(1, 2, 0));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 40U + 128U);

    MockSkb mock2;
    mock2.Reset(AF_INET6);
    mock2.Append(Ipv6Header(IPPROTO_ROUTING, 0));
    mock2.Append(ExtHeader(IPPROTO_ROUTING, 15, 128));
    mock2.Append(ExtHeader(IPPROTO_TCP, 15, 128));
    mock2.Append(TcpHeader(1, 2, 0));
    struct l4_info l42 = {};
    ASSERT_TRUE(parse_l4(mock2.skb.family, mock2.Done(), &l42));
    EXPECT_EQ(l42.state, L4_PARSE_UNPARSEABLE);
}

HWTEST_F(NetsysBpfNetFirewallTest, ParseL4FamilyCorner001, TestSize.Level0)
{
    /* dual-stack v4-mapped: family AF_INET6 with a v4 version nibble parses as
     * IPv4 (documented is_ipv4_format semantics); a non-IP family is a hard
     * parse failure (the FAIL_TUPLE corner preserved from the old code). */
    MockSkb mock;
    mock.Reset(AF_INET6);
    mock.Append(Ipv4Header(20, IPPROTO_UDP, 28, 0));
    mock.Append(UdpHeader(9, 10));
    struct l4_info l4 = {};
    ASSERT_TRUE(parse_l4(mock.skb.family, mock.Done(), &l4));
    EXPECT_EQ(l4.state, L4_PARSE_OK);
    EXPECT_EQ(l4.l4_nhoff, 20U);
    EXPECT_EQ(l4.protocol, IPPROTO_UDP);

    MockSkb mock2;
    mock2.Reset(0); /* AF_UNSPEC */
    mock2.buf.resize(64, 0);
    struct l4_info l42 = {};
    EXPECT_FALSE(parse_l4(mock2.skb.family, mock2.Done(), &l42));

    struct l4_info l43 = {};
    EXPECT_FALSE(parse_l4(AF_UNSPEC, NULL, &l43));
}

/* ===== (validated learning + aging) unit tests. The real BPF
 * domain.h chain runs against the stub maps above: pure helpers are tested
 * directly (shared inline functions, not a user-space re-implementation), the
 * sniffing/learning chain is driven with real wire-format DNS packets. ===== */

namespace {
constexpr uint32_t DNS_TEST_CLIENT_IP = 0xC0A80101U; /* 192.168.1.1 */
constexpr uint32_t DNS_TEST_SERVER_IP = 0x08080808U; /* 8.8.8.8 */
constexpr uint16_t DNS_TEST_EPHEMERAL = 49152;

/* egress query: client -> server, txid 0x1234, one A question */
struct __sk_buff *MakeDnsQuery(MockSkb &mock)
{
    mock.Reset(AF_INET);
    std::vector<uint8_t> dns = DnsHeader(0x1234, 0x0000, 1, 0);
    AppendDnsQuery(dns, "www.example.com", 1);
    mock.Append(Ipv4HeaderAddr(IPPROTO_UDP, static_cast<uint16_t>(20 + 8 + dns.size()), DNS_TEST_CLIENT_IP,
                               DNS_TEST_SERVER_IP));
    mock.Append(UdpHeader(DNS_TEST_EPHEMERAL, 53));
    mock.Append(dns);
    return mock.Done();
}

/* one-answer response; ingress form is server -> client, egress form keeps
 * the same ports/direction so only the caller-chosen dir differs */
struct __sk_buff *MakeDnsResponse(MockSkb &mock, uint16_t txid, uint32_t ttl, uint32_t answerIp)
{
    mock.Reset(AF_INET);
    std::vector<uint8_t> dns = DnsHeader(txid, 0x8000, 1, 1);
    AppendDnsQuery(dns, "www.example.com", 1);
    AppendDnsAnswer(dns, 1, ttl,
                    {static_cast<uint8_t>(answerIp >> 24), static_cast<uint8_t>((answerIp >> 16) & 0xFF),
                     static_cast<uint8_t>((answerIp >> 8) & 0xFF), static_cast<uint8_t>(answerIp & 0xFF)});
    mock.Append(Ipv4HeaderAddr(IPPROTO_UDP, static_cast<uint16_t>(20 + 8 + dns.size()), DNS_TEST_SERVER_IP,
                               DNS_TEST_CLIENT_IP));
    mock.Append(UdpHeader(53, DNS_TEST_EPHEMERAL));
    mock.Append(dns);
    return mock.Done();
}

/* =====: DNS-over-TCP builders — the message follows a 2-byte
 * length prefix after the TCP header. The shared TcpHeader deliberately
 * leaves doff=0 (RST-flag tests), so a dedicated builder sets doff=5. ===== */
std::vector<uint8_t> TcpHeaderDns(uint16_t sport, uint16_t dport)
{
    std::vector<uint8_t> h = TcpHeader(sport, dport, 0x10); /* ACK */
    h[12] = 0x50;                                           /* doff = 5: 20 bytes, no options */
    return h;
}

std::vector<uint8_t> DnsTcpLenPrefix(uint16_t len)
{
    return {static_cast<uint8_t>(len >> 8), static_cast<uint8_t>(len & 0xFF)};
}

struct __sk_buff *MakeTcpDnsQuery(MockSkb &mock)
{
    mock.Reset(AF_INET);
    std::vector<uint8_t> dns = DnsHeader(0x1234, 0x0000, 1, 0);
    AppendDnsQuery(dns, "www.example.com", 1);
    mock.Append(Ipv4HeaderAddr(IPPROTO_TCP, static_cast<uint16_t>(20 + 20 + 2 + dns.size()), DNS_TEST_CLIENT_IP,
                               DNS_TEST_SERVER_IP));
    mock.Append(TcpHeaderDns(DNS_TEST_EPHEMERAL, 53));
    mock.Append(DnsTcpLenPrefix(static_cast<uint16_t>(dns.size())));
    mock.Append(dns);
    return mock.Done();
}

struct __sk_buff *MakeTcpDnsResponse(MockSkb &mock, uint16_t txid, uint32_t ttl, uint32_t answerIp)
{
    mock.Reset(AF_INET);
    std::vector<uint8_t> dns = DnsHeader(txid, 0x8000, 1, 1);
    AppendDnsQuery(dns, "www.example.com", 1);
    AppendDnsAnswer(dns, 1, ttl,
                    {static_cast<uint8_t>(answerIp >> 24), static_cast<uint8_t>((answerIp >> 16) & 0xFF),
                     static_cast<uint8_t>((answerIp >> 8) & 0xFF), static_cast<uint8_t>(answerIp & 0xFF)});
    mock.Append(Ipv4HeaderAddr(IPPROTO_TCP, static_cast<uint16_t>(20 + 20 + 2 + dns.size()), DNS_TEST_SERVER_IP,
                               DNS_TEST_CLIENT_IP));
    mock.Append(TcpHeaderDns(53, DNS_TEST_EPHEMERAL));
    mock.Append(DnsTcpLenPrefix(static_cast<uint16_t>(dns.size())));
    mock.Append(dns);
    return mock.Done();
}
} // namespace

HWTEST_F(NetsysBpfNetFirewallTest, DnsLearnTtlClamp001, TestSize.Level1)
{
    /* TTL=0 is not learned (resolver-aligned,); small TTLs
     * floor at 600s (dnscache lower bound); big TTLs cap at 24h. */
    EXPECT_EQ(dns_learn_ttl_clamp(0), 0U);
    EXPECT_EQ(dns_learn_ttl_clamp(1), 600U);
    EXPECT_EQ(dns_learn_ttl_clamp(599U), 600U);
    EXPECT_EQ(dns_learn_ttl_clamp(600U), 600U);
    EXPECT_EQ(dns_learn_ttl_clamp(601U), 601U);
    EXPECT_EQ(dns_learn_ttl_clamp(86400U), 86400U);
    EXPECT_EQ(dns_learn_ttl_clamp(86401U), 86400U);
    EXPECT_EQ(dns_learn_ttl_clamp(0xFFFFFFFFU), 86400U);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsRecordExpired001, TestSize.Level1)
{
    /* A record lives 60s from the query; the boundary instant itself
     * is still valid (strictly-greater comparison); a backwards clock is
     * fail-safe expired. */
    constexpr uint64_t SEC = 1000000000ULL;
    uint64_t ts = 1000ULL * SEC;
    EXPECT_EQ(dns_record_expired(ts, ts), 0);
    EXPECT_EQ(dns_record_expired(ts + 60ULL * SEC, ts), 0);
    EXPECT_EQ(dns_record_expired(ts + 60ULL * SEC + 1, ts), 1);
    EXPECT_EQ(dns_record_expired(ts - 1, ts), 1);
}

HWTEST_F(NetsysBpfNetFirewallTest, DomainEntryExpired001, TestSize.Level1)
{
    /* time gate math: dead without a timestamp, alive up to ttl
     * (boundary instant inclusive), dead past it, zero-ttl dead immediately,
     * clock skew keeps serving. */
    constexpr uint64_t SEC = 1000000000ULL;
    uint64_t ts = 1000ULL * SEC;
    EXPECT_EQ(domain_entry_expired(ts, 0, 600), 1);
    EXPECT_EQ(domain_entry_expired(ts + 599ULL * SEC, ts, 600), 0);
    EXPECT_EQ(domain_entry_expired(ts + 600ULL * SEC, ts, 600), 0);
    EXPECT_EQ(domain_entry_expired(ts + 600ULL * SEC + 1, ts, 600), 1);
    EXPECT_EQ(domain_entry_expired(ts + 1, ts, 0), 1);
    EXPECT_EQ(domain_entry_expired(ts - 1, ts, 600), 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsQueryRecordKeyBuild001, TestSize.Level1)
{
    /* The RESPONSE 5-tuple reverses into the query view. */
    struct dns_flow_info flow = {};
    struct dns_query_record_key key = {};
    EXPECT_EQ(build_dns_query_record_key(nullptr, &key), -1);
    EXPECT_EQ(build_dns_query_record_key(&flow, nullptr), -1);
    flow.family = AF_UNSPEC;
    EXPECT_EQ(build_dns_query_record_key(&flow, &key), -1);

    flow.protocol = IPPROTO_UDP;
    flow.family = AF_INET;
    flow.txid = 0x1234;
    flow.sport = htons(53);                 /* response src = server */
    flow.dport = htons(DNS_TEST_EPHEMERAL); /* response dst = client */
    flow.ipv4.saddr = htonl(DNS_TEST_SERVER_IP);
    flow.ipv4.daddr = htonl(DNS_TEST_CLIENT_IP);
    ASSERT_EQ(build_dns_query_record_key(&flow, &key), 0);
    EXPECT_EQ(key.txid, 0x1234);
    EXPECT_EQ(key.protocol, IPPROTO_UDP);
    EXPECT_EQ(key.family, AF_INET);
    EXPECT_EQ(key.sport, htons(DNS_TEST_EPHEMERAL)); /* query src = client */
    EXPECT_EQ(key.dport, htons(53));                 /* query dst = server */
    EXPECT_EQ(key.ipv4.saddr, htonl(DNS_TEST_CLIENT_IP));
    EXPECT_EQ(key.ipv4.daddr, htonl(DNS_TEST_SERVER_IP));

    struct in6_addr srv = {};
    struct in6_addr cli = {};
    srv.s6_addr[0] = 0x20;
    cli.s6_addr[15] = 0x01;
    flow.family = AF_INET6;
    flow.ipv6.saddr = srv;
    flow.ipv6.daddr = cli;
    ASSERT_EQ(build_dns_query_record_key(&flow, &key), 0);
    EXPECT_EQ(key.family, AF_INET6);
    EXPECT_EQ(0, memcmp(&key.ipv6.saddr, &cli, sizeof(cli)));
    EXPECT_EQ(0, memcmp(&key.ipv6.daddr, &srv, sizeof(srv)));
}

HWTEST_F(NetsysBpfNetFirewallTest, DomainValueLayout001, TestSize.Level1)
{
    /* 24 bytes with alignment — the pinned map value_size follows
     * sizeof on both sides, locked here so a field reorder cannot silently
     * change the kernel-visible layout.: byte-exact record key/value. */
    static_assert(sizeof(struct domain_value) == 24, "domain_value layout");
    EXPECT_EQ(offsetof(struct domain_value, appuid), 0U);
    EXPECT_EQ(offsetof(struct domain_value, uid), 4U);
    EXPECT_EQ(offsetof(struct domain_value, timestamp), 8U);
    EXPECT_EQ(offsetof(struct domain_value, ttl), 16U);
    static_assert(sizeof(struct dns_query_record_key) == 40, "record key layout");
    static_assert(sizeof(struct dns_query_record_value) == 16, "record value layout");
}

HWTEST_F(NetsysBpfNetFirewallTest, MatchDomainTimeGate001, TestSize.Level1)
{
    /* first layer: an expired learned entry is a miss; it is not
     * deleted on the read path (reclaim belongs to the aging sweep). */
    ResetDnsStubs();
    g_stubDeleteCalls = 0;
    struct match_tuple tuple = {};
    tuple.family = AF_INET;
    tuple.uid = 1; /* get_user_id(200100) */
    tuple.appuid = 200100;
    tuple.ipv4.saddr = htonl(0x01020304U);

    g_stubDomainV4Ptr = nullptr;
    EXPECT_FALSE(MatchDomain(&tuple));

    g_stubDomainV4Entry = {};
    g_stubDomainV4Entry.uid = 1;
    g_stubDomainV4Entry.appuid = 200100;
    g_stubDomainV4Entry.timestamp = g_stubNowNs - 1000U;
    g_stubDomainV4Entry.ttl = 600;
    g_stubDomainV4Ptr = &g_stubDomainV4Entry;
    EXPECT_TRUE(MatchDomain(&tuple));

    /* entry past its ttl: miss, still in place (no delete path exists here) */
    g_stubDomainV4Entry.timestamp = g_stubNowNs - 601ULL * 1000000000ULL;
    EXPECT_FALSE(MatchDomain(&tuple));
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);

    /* timestamp=0: dead on read — no stale writer can pin an entry open */
    g_stubDomainV4Entry.timestamp = 0;
    g_stubDomainV4Entry.ttl = 600;
    EXPECT_FALSE(MatchDomain(&tuple));

    /* appuid wildcard entry (appuid=0) with a fresh timestamp: hit */
    g_stubDomainV4Entry.appuid = 0;
    g_stubDomainV4Entry.timestamp = g_stubNowNs - 1000U;
    EXPECT_TRUE(MatchDomain(&tuple));
    /* The read path never deletes — hits and misses alike */
    EXPECT_EQ(g_stubDeleteCalls, 0);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffEgressQueryStagesRecord001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsQuery(mock);
    ASSERT_NE(skb, nullptr);
    EXPECT_EQ(match_dns_query(skb->family, skb, EGRESS, NULL), SK_PASS);
    /* The allow-listed query stages its credential — query-view key,
     * this-skb tag, egress attribution — but nothing is recorded yet (the
     * hook-exit flush decides with the final verdict). */
    EXPECT_EQ(g_stubPending.valid, 1);
    EXPECT_EQ(g_stubPending.skb_addr, static_cast<uint64_t>(reinterpret_cast<unsigned long>(skb)));
    EXPECT_EQ(g_stubPending.key.txid, 0x1234);
    EXPECT_EQ(g_stubPending.key.sport, htons(DNS_TEST_EPHEMERAL));
    EXPECT_EQ(g_stubPending.key.dport, htons(53));
    EXPECT_EQ(g_stubPending.key.ipv4.saddr, htonl(DNS_TEST_CLIENT_IP));
    EXPECT_EQ(g_stubPending.key.ipv4.daddr, htonl(DNS_TEST_SERVER_IP));
    EXPECT_EQ(g_stubPending.uid, 1);
    EXPECT_EQ(g_stubPending.appuid, 200100U);
    EXPECT_EQ(g_stubRecordUpdates.size(), 0U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffDropFastFailEgressOnly001, TestSize.Level1)
{
    /* F5 fast-fail: the sniff DROP handler destroys the EGRESS querying
     * socket (ms-level ECONNABORTED to its owner instead of the ~5-20s
     * resolver backoff) but leaves INGRESS drops alone — the socket may
     * be serving other in-flight queries. */
    ResetDnsStubs();
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsQuery(mock);
    ASSERT_NE(skb, nullptr);
    int destroyedBefore = g_stubSockDestroyed;
    EXPECT_EQ(dns_sniff_drop(AF_INET, skb, EGRESS), SK_DROP);
    EXPECT_EQ(g_stubSockDestroyed, destroyedBefore + 1); /* fast-fail fired */
    destroyedBefore = g_stubSockDestroyed;
    EXPECT_EQ(dns_sniff_drop(AF_INET, skb, INGRESS), SK_DROP);
    EXPECT_EQ(g_stubSockDestroyed, destroyedBefore); /* ingress: socket spared */
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, EgressGateClosedQueryFastFail001, TestSize.Level1)
{
    /* F5 fast-fail at the policy chain: a default-deny egress verdict on
     * a parseable DNS query (gate->is_query, staged by the egress dns
     * program) destroys the querying socket; the same DROP for a
     * non-query keeps the legacy silent behavior. */
    ResetDnsStubs();
    g_stubSocketUid = 20020152;               /* -> userId 100 */
    g_stubDefaultAction = {SK_DROP, SK_DROP}; /* both directions deny */
    g_stubDevNameOk = true;                   /* full evaluations take the real rule path (no rules -> default) */
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsQuery(mock);
    ASSERT_NE(skb, nullptr);

    struct dns_gate_ctx gate = {};
    gate.is_query = 1; /* staged by the egress dns program */
    int destroyedBefore = g_stubSockDestroyed;
    EXPECT_EQ(fw_policy_after_egress(AF_INET, skb, &gate), SK_DROP);
    EXPECT_EQ(g_stubSockDestroyed, destroyedBefore + 1); /* fast-fail fired */

    /* control: same packet, gate not a query -> legacy silent drop */
    gate.is_query = 0;
    destroyedBefore = g_stubSockDestroyed;
    EXPECT_EQ(fw_policy_after_egress(AF_INET, skb, &gate), SK_DROP);
    EXPECT_EQ(g_stubSockDestroyed, destroyedBefore); /* silent drop preserved */
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffMultiQuestionNotStaged001, TestSize.Level1)
{
    ResetDnsStubs();
    MockSkb mock;
    mock.Reset(AF_INET);
    std::vector<uint8_t> dns = DnsHeader(0x1234, 0x0000, 2, 0);
    AppendDnsQuery(dns, "www.example.com", 1);
    AppendDnsQuery(dns, "www.example.org", 1);
    mock.Append(Ipv4HeaderAddr(IPPROTO_UDP, static_cast<uint16_t>(20 + 8 + dns.size()), DNS_TEST_CLIENT_IP,
                               DNS_TEST_SERVER_IP));
    mock.Append(UdpHeader(DNS_TEST_EPHEMERAL, 53));
    mock.Append(dns);
    EXPECT_EQ(match_dns_query(mock.skb.family, mock.Done(), EGRESS, NULL), SK_PASS);
    /* qdcount != 1: neither staged nor learned (unchanged behavior) */
    EXPECT_EQ(g_stubPending.valid, 0);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffTruncatedNoCrash001, TestSize.Level1)
{
    ResetDnsStubs();
    /* (a): a name cut mid-label and an answer cut mid-rdata must not crash,
     * must not learn garbage, and the packet keeps flowing (parse aborts). */
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsResponse(mock, 0x1234, 300, 0x01020304U);
    mock.buf.resize(mock.buf.size() - 30); /* cut into the answer rdata */
    EXPECT_EQ(match_dns_query(mock.skb.family, mock.Done(), INGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);

    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsQuery(mock2);
    mock2.buf.resize(mock2.buf.size() - 10); /* cut into the question name */
    EXPECT_EQ(match_dns_query(mock2.skb.family, mock2.Done(), EGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);

    /* DNS header itself truncated: no sniffing at all */
    MockSkb mock3;
    mock3.Reset(AF_INET);
    mock3.Append(Ipv4HeaderAddr(IPPROTO_UDP, 20 + 8 + 4, DNS_TEST_CLIENT_IP, DNS_TEST_SERVER_IP));
    mock3.Append(UdpHeader(DNS_TEST_EPHEMERAL, 53));
    mock3.Append(std::vector<uint8_t>(4, 0));
    EXPECT_EQ(match_dns_query(mock3.skb.family, mock3.Done(), EGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);
    (void)skb;
    (void)skb2;
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsLearnTtlZeroNotLearned001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    /* A TTL=0 answer is never learned — the resolver does not cache
     * it either, and a zero-ttl entry could not pass the time gate. */
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsResponse(mock, 0x1234, 0, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb->family, skb, INGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);

    /* TTL=1 still learns — clamped to the 600s floor. */
    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsResponse(mock2, 0x1234, 1, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb2->family, skb2, INGRESS, NULL), SK_PASS);
    ASSERT_EQ(g_stubDomainUpdates.size(), 1U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.ttl, 600U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsLearnIngressResponse001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsResponse(mock, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb->family, skb, INGRESS, NULL), SK_PASS);
    ASSERT_EQ(g_stubDomainUpdates.size(), 1U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.ttl, 600U); /* clamp(300) = 600 */
    /* no matching record: rejected (— learning only, never a drop) */
    g_stubRecordPtr = nullptr;
    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsResponse(mock2, 0x1234, 300, 0x05060708U);
    EXPECT_EQ(match_dns_query(skb2->family, skb2, INGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 1U); /* nothing new learned */
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsFlushRecordPoint001, TestSize.Level1)
{
    ResetDnsStubs();
    struct __sk_buff skb = {};
    g_stubPending.skb_addr = reinterpret_cast<uint64_t>(&skb);
    g_stubPending.valid = 1;
    g_stubPending.key.txid = 0x1234;
    g_stubPending.uid = 1;
    g_stubPending.appuid = 200100;

    /* Final verdict PASS -> record written with the pending
     * attribution and the flush-time timestamp; slot released. */
    flush_dns_query_record(&skb, true);
    ASSERT_EQ(g_stubRecordUpdates.size(), 1U);
    EXPECT_EQ(g_stubRecordUpdates[0].key.txid, 0x1234);
    EXPECT_EQ(g_stubRecordUpdates[0].value.uid, 1U);
    EXPECT_EQ(g_stubRecordUpdates[0].value.appuid, 200100U);
    EXPECT_EQ(g_stubRecordUpdates[0].value.timestamp, g_stubNowNs);
    EXPECT_EQ(g_stubPending.valid, 0);

    /* final verdict DROP -> discarded, slot released, nothing written */
    g_stubPending.skb_addr = reinterpret_cast<uint64_t>(&skb);
    g_stubPending.valid = 1;
    flush_dns_query_record(&skb, false);
    EXPECT_EQ(g_stubRecordUpdates.size(), 1U);
    EXPECT_EQ(g_stubPending.valid, 0);

    /* Another skb's leftovers are never flushed by this skb */
    struct __sk_buff other = {};
    g_stubPending.skb_addr = reinterpret_cast<uint64_t>(&other);
    g_stubPending.valid = 1;
    flush_dns_query_record(&skb, true);
    EXPECT_EQ(g_stubRecordUpdates.size(), 1U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsLearnValidatedEndToEnd001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    /* stage via the real egress sniff, flush with the real flush point, then
     * require the ingress response to match the record through the real
     * (reversal) lookup — the keyed stub "map" only hits on a byte-exact key. */
    MockSkb qmock;
    struct __sk_buff *qskb = MakeDnsQuery(qmock);
    ASSERT_EQ(match_dns_query(qskb->family, qskb, EGRESS, NULL), SK_PASS);
    ASSERT_EQ(g_stubPending.valid, 1);
    flush_dns_query_record(qskb, true);
    ASSERT_EQ(g_stubRecordUpdates.size(), 1U);

    g_stubKeyedRecord = true;
    g_stubExpectedRecordKey = g_stubRecordUpdates[0].key;
    g_stubRecordValue = {};
    g_stubRecordValue.uid = 1;
    g_stubRecordValue.appuid = 200100;
    g_stubRecordValue.timestamp = g_stubNowNs - 1000U; /* well inside 60s */

    /* matching response (server->client, txid 0x1234, reversed to the query
     * view): learned with the RECORD's attribution and timestamp. */
    MockSkb rmock;
    struct __sk_buff *rskb = MakeDnsResponse(rmock, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(rskb->family, rskb, INGRESS, NULL), SK_PASS);
    ASSERT_EQ(g_stubDomainUpdates.size(), 1U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.uid, 1U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.appuid, 200100U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.timestamp, g_stubNowNs - 1000U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.ttl, 600U);

    /* forged txid: the reversed key misses the record -> no learning, and the
     * answer itself still flows (— a failed check is not a drop). */
    MockSkb fmock;
    struct __sk_buff *fskb = MakeDnsResponse(fmock, 0xBEEF, 300, 0x05060708U);
    EXPECT_EQ(match_dns_query(fskb->family, fskb, INGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 1U);

    /* expired record (61s old): rejected like a miss. */
    g_stubRecordValue.timestamp = g_stubNowNs - 61ULL * 1000000000ULL;
    MockSkb emock;
    struct __sk_buff *eskb = MakeDnsResponse(emock, 0x1234, 300, 0x090A0B0CU);
    EXPECT_EQ(match_dns_query(eskb->family, eskb, INGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 1U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsLearnEgressResponseLegacy001, TestSize.Level1)
{
    ResetDnsStubs();
    /* D-egress: an egress "response" (QR=1 flowing out) is not
     * parsed at all — no learning and no deny-drop in the sniffer; the packet
     * keeps flowing to the policy/ct chain, so the main vector (a local
     * app forging an outbound reply that learns) is structurally dead. */
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsResponse(mock, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb->family, skb, EGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);
    EXPECT_EQ(g_stubPending.valid, 0); /* staged nothing, learned nothing */
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffDenyQueryDrop001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(false, true);                    /* deny rule on the query qname */
    g_stubDefaultAction = {SK_DROP, SK_DROP}; /* default-deny user */
    /* Spec row 1 (2026-11-16): a deny-qname egress query is dropped right
     * at the sniffer (fast-fail: the dns program hands the verdict to
     * dns_sniff_drop — intercept event + socket destroy), the gate export
     * still happens before the short-circuit (allowrst=0: deny, not allow),
     * and nothing is staged (only allow-listed queries stage records). */
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsQuery(mock);
    struct dns_gate_ctx gate = {};
    EXPECT_EQ(match_dns_query(skb->family, skb, EGRESS, &gate), SK_DROP);
    EXPECT_EQ(gate.is_query, 1);
    EXPECT_EQ(gate.allowrst, 0);
    EXPECT_EQ(g_stubPending.valid, 0);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffDenyResponseDrop001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(false, true);                    /* deny rule on the response qname */
    g_stubDefaultAction = {SK_DROP, SK_DROP}; /* default-deny user */
    /* Spec row 1, response side: a response carrying a deny-qname record is
     * dropped at the sniffer on ingress — the client never sees the
     * resolution (the querying socket is destroyed by dns_sniff_drop). */
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsResponse(mock, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb->family, skb, INGRESS, NULL), SK_DROP);
    ResetDnsStubs();
}

/* ===== Double-hit adjudication (user ruling 2026-11-17): when an explicit
 * allow and an explicit deny rule both match the same qname, the verdict
 * follows the DEFAULT action direction — same semantics as the IP chain's
 * Conflict & Repetition Algorithm in match_action. The baidu shape: deny
 * *.example.com + allow www.example.com on qname www.example.com. ===== */

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffDoubleHitDefaultAllow001, TestSize.Level1)
{
    /* default-ALLOW + conflict: the explicit allow outranks the explicit
     * deny — the query passes, the gate opens and the record stages, as
     * if only the allow rule existed. */
    ResetDnsStubs();
    AcDnsEnvConflict();
    g_stubDefaultAction = {SK_PASS, SK_PASS};
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsQuery(mock);
    struct dns_gate_ctx gate = {};
    EXPECT_EQ(match_dns_query(skb->family, skb, EGRESS, &gate), SK_PASS);
    EXPECT_EQ(gate.is_query, 1);
    EXPECT_EQ(gate.allowrst, 1);
    EXPECT_EQ(g_stubPending.valid, 1); /* allow-listed queries stage records */
    ResetDnsStubs();

    /* Guard row (single-hit, unchanged by the ruling): under default-ALLOW
     * an UNOPPOSED explicit deny still drops — a deny rule stays effective
     * when nothing conflicts it. */
    ResetDnsStubs();
    AcDnsEnv(false, true);
    g_stubDefaultAction = {SK_PASS, SK_PASS};
    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsQuery(mock2);
    struct dns_gate_ctx gate2 = {};
    EXPECT_EQ(match_dns_query(skb2->family, skb2, EGRESS, &gate2), SK_DROP);
    EXPECT_EQ(gate2.is_query, 1);
    EXPECT_EQ(gate2.allowrst, 0);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffDoubleHitDefaultDeny001, TestSize.Level1)
{
    /* default-DENY + conflict: the explicit deny outranks the explicit
     * allow — dropped right at the sniffer (fast-fail), nothing staged;
     * the gate export still happens before the short-circuit. */
    ResetDnsStubs();
    AcDnsEnvConflict();
    g_stubDefaultAction = {SK_DROP, SK_DROP};
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsQuery(mock);
    struct dns_gate_ctx gate = {};
    EXPECT_EQ(match_dns_query(skb->family, skb, EGRESS, &gate), SK_DROP);
    EXPECT_EQ(gate.is_query, 1);
    EXPECT_EQ(gate.allowrst, 1);
    EXPECT_EQ(g_stubPending.valid, 0); /* dropped queries never stage */
    ResetDnsStubs();

    /* Guard row (single-hit, unchanged): default-DENY + allow-only passes
     * the sniffer and stages — the default verdict itself is decided
     * later by the policy chain / DNS server gate. */
    ResetDnsStubs();
    AcDnsEnv(true);
    g_stubDefaultAction = {SK_DROP, SK_DROP};
    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsQuery(mock2);
    struct dns_gate_ctx gate2 = {};
    EXPECT_EQ(match_dns_query(skb2->family, skb2, EGRESS, &gate2), SK_PASS);
    EXPECT_EQ(gate2.allowrst, 1);
    EXPECT_EQ(g_stubPending.valid, 1);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffDoubleHitResponseFollowsDefault001, TestSize.Level1)
{
    /* Response side, same matrix (parse_dns_response carries the identical
     * adjudication): default-ALLOW + conflict keeps the response flowing
     * (the client sees the resolution — the answer learning may proceed);
     * default-DENY + conflict drops it at the sniffer. */
    ResetDnsStubs();
    AcDnsEnvConflict();
    g_stubDefaultAction = {SK_PASS, SK_PASS};
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsResponse(mock, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb->family, skb, INGRESS, NULL), SK_PASS);
    ResetDnsStubs();

    ResetDnsStubs();
    AcDnsEnvConflict();
    g_stubDefaultAction = {SK_DROP, SK_DROP};
    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsResponse(mock2, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb2->family, skb2, INGRESS, NULL), SK_DROP);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsGateCtxFill001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    /* The sniffing step exports the qname verdict to the policy
     * layer. Only a parseable single-question egress query opens is_query —
     * a malformed or non-DNS packet keeps the gate closed (fail-closed). */
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsQuery(mock);
    struct dns_gate_ctx gate = {};
    EXPECT_EQ(match_dns_query(skb->family, skb, EGRESS, &gate), SK_PASS);
    EXPECT_EQ(gate.is_query, 1);
    EXPECT_EQ(gate.allowrst, 1); /* AC allow rule: the qname is allow-listed */

    /* qname hits no allow rule: query-but-not-allowed — the gate cannot open */
    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsQuery(mock2);
    struct dns_gate_ctx gate2 = {};
    AcDnsEnv(false);
    EXPECT_EQ(match_dns_query(skb2->family, skb2, EGRESS, &gate2), SK_PASS);
    EXPECT_EQ(gate2.is_query, 1);
    EXPECT_EQ(gate2.allowrst, 0);

    /* multi-question: the parse aborts before the verdict -> ctx untouched */
    MockSkb mock3;
    mock3.Reset(AF_INET);
    std::vector<uint8_t> dns3 = DnsHeader(0x1234, 0x0000, 2, 0);
    AppendDnsQuery(dns3, "www.example.com", 1);
    AppendDnsQuery(dns3, "www.example.org", 1);
    mock3.Append(Ipv4HeaderAddr(IPPROTO_UDP, static_cast<uint16_t>(20 + 8 + dns3.size()), DNS_TEST_CLIENT_IP,
                                DNS_TEST_SERVER_IP));
    mock3.Append(UdpHeader(DNS_TEST_EPHEMERAL, 53));
    mock3.Append(dns3);
    struct dns_gate_ctx gate3 = {};
    EXPECT_EQ(match_dns_query(mock3.skb.family, mock3.Done(), EGRESS, &gate3), SK_PASS);
    EXPECT_EQ(gate3.is_query, 0);
    EXPECT_EQ(gate3.allowrst, 0);

    /* non-53/853 port: not sniffed at all -> ctx untouched */
    MockSkb mock4;
    mock4.Reset(AF_INET);
    std::vector<uint8_t> dns4 = DnsHeader(0x1234, 0x0000, 1, 0);
    AppendDnsQuery(dns4, "www.example.com", 1);
    mock4.Append(Ipv4HeaderAddr(IPPROTO_UDP, static_cast<uint16_t>(20 + 8 + dns4.size()), DNS_TEST_CLIENT_IP,
                                DNS_TEST_SERVER_IP));
    mock4.Append(UdpHeader(DNS_TEST_EPHEMERAL, 5353));
    mock4.Append(dns4);
    struct dns_gate_ctx gate4 = {};
    EXPECT_EQ(match_dns_query(mock4.skb.family, mock4.Done(), EGRESS, &gate4), SK_PASS);
    EXPECT_EQ(gate4.is_query, 0);
    EXPECT_EQ(gate4.allowrst, 0);

    /* DoT (853, UDP): port-shape marking only — the payload is encrypted,
     * nothing is parsed/staged/learned, is_query is marked by the port.
     * Clear the pending-record stub first: the first sub-case (allow env)
     * legitimately staged a record and the stub keeps the last write —
     * the 853 assertion below must see a fresh, untouched stub. */
    g_stubPending = {};
    MockSkb mock5;
    mock5.Reset(AF_INET);
    std::vector<uint8_t> dot5 = {0x16, 0x03, 0x01, 0x00, 0x20}; /* TLS-ish bytes: never parsed */
    mock5.Append(Ipv4HeaderAddr(IPPROTO_UDP, static_cast<uint16_t>(20 + 8 + dot5.size()), DNS_TEST_CLIENT_IP,
                                DNS_TEST_SERVER_IP));
    mock5.Append(UdpHeader(DNS_TEST_EPHEMERAL, DNS_OVER_TLS_PORT));
    mock5.Append(dot5);
    struct dns_gate_ctx gate5 = {};
    EXPECT_EQ(match_dns_query(mock5.skb.family, mock5.Done(), EGRESS, &gate5), SK_PASS);
    EXPECT_EQ(gate5.is_query, 1);
    EXPECT_EQ(gate5.allowrst, 0);
    EXPECT_EQ(g_stubPending.valid, 0);

    /* DoT (853, TCP): same port-shape marking on the TCP face. The header
     * must be built with TcpHeaderDns: its 0x50 sits on the data-offset
     * byte (offset 12), which the parser reads before anything else — the
     * plain TcpHeader helper writes its third argument to the flags byte
     * (offset 13), leaving doff == 0 and short-circuiting the parse. */
    MockSkb mock6;
    mock6.Reset(AF_INET);
    std::vector<uint8_t> dot6 = {0x16, 0x03, 0x01, 0x00, 0x20};
    mock6.Append(Ipv4HeaderAddr(IPPROTO_TCP, static_cast<uint16_t>(20 + 20 + dot6.size()), DNS_TEST_CLIENT_IP,
                                DNS_TEST_SERVER_IP));
    mock6.Append(TcpHeaderDns(DNS_TEST_EPHEMERAL, DNS_OVER_TLS_PORT));
    mock6.Append(dot6);
    struct dns_gate_ctx gate6 = {};
    EXPECT_EQ(match_dns_query(mock6.skb.family, mock6.Done(), EGRESS, &gate6), SK_PASS);
    EXPECT_EQ(gate6.is_query, 1);
    EXPECT_EQ(gate6.allowrst, 0);
    EXPECT_EQ(g_stubPending.valid, 0);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffTcpQueryStagesRecord001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    /* DNS-over-TCP query — the 2-byte length prefix is skipped, the
     * gate ctx is filled and the record is staged with protocol = TCP. */
    MockSkb mock;
    struct __sk_buff *skb = MakeTcpDnsQuery(mock);
    struct dns_gate_ctx gate = {};
    EXPECT_EQ(match_dns_query(skb->family, skb, EGRESS, &gate), SK_PASS);
    EXPECT_EQ(gate.is_query, 1);
    EXPECT_EQ(gate.allowrst, 1);
    EXPECT_EQ(g_stubPending.valid, 1);
    EXPECT_EQ(g_stubPending.skb_addr, static_cast<uint64_t>(reinterpret_cast<unsigned long>(skb)));
    EXPECT_EQ(g_stubPending.key.txid, 0x1234);
    EXPECT_EQ(g_stubPending.key.protocol, IPPROTO_TCP);
    EXPECT_EQ(g_stubPending.key.sport, htons(DNS_TEST_EPHEMERAL));
    EXPECT_EQ(g_stubPending.key.dport, htons(53));
    EXPECT_EQ(g_stubPending.key.ipv4.saddr, htonl(DNS_TEST_CLIENT_IP));
    EXPECT_EQ(g_stubPending.key.ipv4.daddr, htonl(DNS_TEST_SERVER_IP));
    EXPECT_EQ(g_stubPending.uid, 1);
    EXPECT_EQ(g_stubPending.appuid, 200100U);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffTcpBadDoff001, TestSize.Level1)
{
    ResetDnsStubs();
    /* doff=0 (malformed TCP header): the sniff aborts fail-closed — nothing
     * staged, the gate stays closed, the policy chain still decides. */
    MockSkb mock;
    mock.Reset(AF_INET);
    std::vector<uint8_t> dns = DnsHeader(0x1234, 0x0000, 1, 0);
    AppendDnsQuery(dns, "www.example.com", 1);
    mock.Append(Ipv4HeaderAddr(IPPROTO_TCP, static_cast<uint16_t>(20 + 20 + 2 + dns.size()), DNS_TEST_CLIENT_IP,
                               DNS_TEST_SERVER_IP));
    mock.Append(TcpHeader(DNS_TEST_EPHEMERAL, 53, 0x10)); /* doff byte stays 0 */
    mock.Append(DnsTcpLenPrefix(static_cast<uint16_t>(dns.size())));
    mock.Append(dns);
    struct dns_gate_ctx gate = {};
    EXPECT_EQ(match_dns_query(mock.skb.family, mock.Done(), EGRESS, &gate), SK_PASS);
    EXPECT_EQ(gate.is_query, 0);
    EXPECT_EQ(gate.allowrst, 0);
    EXPECT_EQ(g_stubPending.valid, 0);
    EXPECT_EQ(g_stubDomainUpdates.size(), 0U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsSniffTcpResponseLearn001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    /* round trip: a staged TCP query's record is matched by the TCP
     * response (protocol is part of the record key) and learns like UDP. */
    MockSkb qmock;
    struct __sk_buff *qskb = MakeTcpDnsQuery(qmock);
    EXPECT_EQ(match_dns_query(qskb->family, qskb, EGRESS, NULL), SK_PASS);
    flush_dns_query_record(qskb, true);
    ASSERT_EQ(g_stubRecordUpdates.size(), 1U);
    EXPECT_EQ(g_stubRecordUpdates[0].key.protocol, IPPROTO_TCP);

    g_stubKeyedRecord = true;
    g_stubExpectedRecordKey = g_stubRecordUpdates[0].key;
    g_stubRecordValue = {};
    g_stubRecordValue.uid = 1;
    g_stubRecordValue.appuid = 200100;
    g_stubRecordValue.timestamp = g_stubNowNs - 1000U;

    MockSkb rmock;
    struct __sk_buff *rskb = MakeTcpDnsResponse(rmock, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(rskb->family, rskb, INGRESS, NULL), SK_PASS);
    ASSERT_EQ(g_stubDomainUpdates.size(), 1U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.uid, 1U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.appuid, 200100U);
    EXPECT_EQ(g_stubDomainUpdates[0].value.ttl, 600U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsLearnO2ServerCheck001, TestSize.Level1)
{
    ResetDnsStubs();
    AcDnsEnv(true);
    /* (adopted): a deny-default user — exactly the managed
     * population and the DNS_SERVER map population (§5.1; allowrst in the
     * learning path already proves the domain-allow half) — only learns from
     * an authorized server. A default-allow user holds no map entries and
     * keeps the plain credential path. */
    g_stubDefaultAction = {SK_DROP, SK_DROP};

    /* managed user, authorized server (the response's src = 8.8.8.8): learn */
    struct dns_server_v4_key srv = {};
    srv.uid = 1;
    srv.ip = htonl(DNS_TEST_SERVER_IP);
    g_stubDnsServerV4Keys.push_back(srv);
    MockSkb mock;
    struct __sk_buff *skb = MakeDnsResponse(mock, 0x1234, 300, 0x01020304U);
    EXPECT_EQ(match_dns_query(skb->family, skb, INGRESS, NULL), SK_PASS);
    ASSERT_EQ(g_stubDomainUpdates.size(), 1U);

    /* managed user, unauthorized server (map miss): skip learning only —
     * the response itself still flows */
    g_stubDnsServerV4Keys.clear();
    MockSkb mock2;
    struct __sk_buff *skb2 = MakeDnsResponse(mock2, 0x1234, 300, 0x05060708U);
    EXPECT_EQ(match_dns_query(skb2->family, skb2, INGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 1U);

    /* default-allow user, same map miss: does not apply, learns per  */
    g_stubDefaultAction = {SK_PASS, SK_PASS};
    MockSkb mock3;
    struct __sk_buff *skb3 = MakeDnsResponse(mock3, 0x1234, 300, 0x090A0B0CU);
    EXPECT_EQ(match_dns_query(skb3->family, skb3, INGRESS, NULL), SK_PASS);
    EXPECT_EQ(g_stubDomainUpdates.size(), 2U);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, GetUserIdFolding001, TestSize.Level1)
{
    /* F6: real system-identity sockets (uid < USER_ID_DIVIDOR — system
     * apps' appId, root, shell, system daemons) fold to the foreground
     * user via CURRENT_UID_MAP (factory spec restored); map miss ->
     * DEFAULT_USER_ID; the 0xFFFFFFFF sentinel keeps UID_UNATTRIBUTED;
     * real third-party uids keep the divisor math. */
    g_stubCurrentUidPresent = false;
    EXPECT_EQ(get_user_id(0U), DEFAULT_USER_ID); /* map miss -> 100 */
    EXPECT_EQ(get_user_id(2000U), DEFAULT_USER_ID);
    EXPECT_EQ(get_user_id(199999U), DEFAULT_USER_ID);
    g_stubCurrentUidPresent = true;
    g_stubCurrentUid = 101; /* foreground user 101 */
    EXPECT_EQ(get_user_id(0U), 101U);
    EXPECT_EQ(get_user_id(2000U), 101U);
    EXPECT_EQ(get_user_id(12345U), 101U); /* a system app's appId */
    g_stubCurrentUid = 100;               /* foreground user 100 */
    EXPECT_EQ(get_user_id(0U), 100U);
    EXPECT_EQ(get_user_id(199999U), 100U);
    EXPECT_EQ(get_user_id(200000U), 1U); /* user 1's own uid range */
    EXPECT_EQ(get_user_id(200100U), 1U);
    EXPECT_EQ(get_user_id(20199999U), 100U);
    EXPECT_EQ(get_user_id(0xFFFFFFFFU), UID_UNATTRIBUTED);
    g_stubCurrentUidPresent = false;
}

HWTEST_F(NetsysBpfNetFirewallTest, SystemIdentityFoldsToForeground001, TestSize.Level1)
{
    /* F6 hook-level: root's socket (uid 0) is governed by the foreground
     * user's policy — a default-deny-out foreground user drops root's
     * egress (the ICMP "ping escapes rules" defect closes); a
     * default-allow foreground user passes it. */
    ResetDnsStubs();
    g_stubDevNameOk = true; /* full evaluations take the real rule path (no rules -> default) */
    g_stubCurrentUidPresent = true;
    g_stubCurrentUid = 100;
    g_stubSocketUid = 0;                       /* root — folds to the foreground user (100) */
    constexpr uint32_t REMOTE_IP = 0xB465312C; /* 180.101.49.44 */
    constexpr uint16_t LOCAL_PORT = 12345;
    constexpr uint32_t LOCAL_IP = 0x0A000001; /* 10.0.0.1 */

    g_stubDefaultAction = {SK_PASS, SK_DROP}; /* user 100: out deny */
    MockSkb deny;
    deny.Reset(AF_INET);
    deny.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, LOCAL_IP, REMOTE_IP));
    deny.Append(TcpHeader(LOCAL_PORT, 80, 0x02));
    EXPECT_EQ(fw_policy_after_egress(AF_INET, deny.Done(), NULL), SK_DROP);

    g_stubDefaultAction = {SK_PASS, SK_PASS}; /* user 100: out allow */
    MockSkb allow;
    allow.Reset(AF_INET);
    allow.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, LOCAL_IP, REMOTE_IP));
    allow.Append(TcpHeader(LOCAL_PORT, 80, 0x02));
    EXPECT_EQ(fw_policy_after_egress(AF_INET, allow.Done(), NULL), SK_PASS);

    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, MatchDnsServerLookup001, TestSize.Level1)
{
    ResetDnsStubs();
    /* condition 3: {uid, server ip} presence in the authorization
     * maps — wrong uid, wrong ip, wrong family or a null v6 pointer all miss. */
    struct dns_server_v4_key v4 = {};
    v4.uid = 1;
    v4.ip = htonl(DNS_TEST_SERVER_IP);
    g_stubDnsServerV4Keys.push_back(v4);
    EXPECT_TRUE(MatchDnsServer(1, AF_INET, htonl(DNS_TEST_SERVER_IP), nullptr));
    EXPECT_FALSE(MatchDnsServer(2, AF_INET, htonl(DNS_TEST_SERVER_IP), nullptr));
    EXPECT_FALSE(MatchDnsServer(1, AF_INET, htonl(0x01010101U), nullptr));

    struct in6_addr srv6 = {};
    srv6.s6_addr[0] = 0x20;
    struct dns_server_v6_key v6 = {};
    v6.uid = 1;
    memcpy(&v6.ip, &srv6, sizeof(srv6));
    g_stubDnsServerV6Keys.push_back(v6);
    EXPECT_TRUE(MatchDnsServer(1, AF_INET6, 0, &srv6));
    EXPECT_FALSE(MatchDnsServer(1, AF_INET6, 0, nullptr)); /* null v6 pointer */
    EXPECT_FALSE(MatchDnsServer(1, AF_UNSPEC, 0, &srv6));  /* unknown family */
    ResetDnsStubs();
}

/* shared builder: the post-swap egress query tuple (server side = saddr/sport)
 * + the armed gate ctx + the authorized server map entry. */
static void MakeGateTuple(struct match_tuple &tuple, struct dns_gate_ctx &gate, struct bitmap &key)
{
    memset(&tuple, 0, sizeof(tuple));
    tuple.dir = EGRESS;
    tuple.family = AF_INET;
    tuple.protocol = IPPROTO_UDP;
    tuple.uid = 1; /* get_user_id(200100) */
    tuple.appuid = 200100;
    tuple.sport = htons(53); /* server side after the egress port swap */
    tuple.dport = htons(DNS_TEST_EPHEMERAL);
    tuple.ipv4.saddr = htonl(DNS_TEST_SERVER_IP);
    tuple.ipv4.daddr = htonl(DNS_TEST_CLIENT_IP);

    gate.is_query = 1;
    gate.allowrst = 1;
    memset(&key, 0, sizeof(key)); /* no rule bit: the no-rule branch */

    g_stubDefaultAction = {SK_DROP, SK_DROP}; /* managed: default deny */
    struct dns_server_v4_key srv = {};
    srv.uid = 1;
    srv.ip = htonl(DNS_TEST_SERVER_IP);
    g_stubDnsServerV4Keys.push_back(srv);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsGateMatrix001, TestSize.Level1)
{
    ResetDnsStubs();
    /* The four gate conditions — all must hold to rewrite a
     * default-DROP into PASS; each missing one keeps the DROP. */
    struct match_tuple tuple;
    struct dns_gate_ctx gate;
    struct bitmap key;
    MakeGateTuple(tuple, gate, key);
    EXPECT_EQ(match_action(&tuple, &key, &gate), FW_PASS);

    /* condition 4 missing: qname hit no allow rule */
    struct dns_gate_ctx noAllow = gate;
    noAllow.allowrst = 0;
    EXPECT_EQ(match_action(&tuple, &key, &noAllow), FW_DROP);

    /* not a parseable query (malformed DNS on :53 — fail-closed) */
    struct dns_gate_ctx notQuery = gate;
    notQuery.is_query = 0;
    EXPECT_EQ(match_action(&tuple, &key, &notQuery), FW_DROP);

    /* condition 3 missing: server not authorized */
    g_stubDnsServerV4Keys.clear();
    EXPECT_EQ(match_action(&tuple, &key, &gate), FW_DROP);
    struct dns_server_v4_key srv = {};
    srv.uid = 1;
    srv.ip = htonl(DNS_TEST_SERVER_IP);
    g_stubDnsServerV4Keys.push_back(srv);

    /* condition 2 missing: neither port 53 nor 853 */
    struct match_tuple tPort = tuple;
    tPort.sport = htons(5353);
    EXPECT_EQ(match_action(&tPort, &key, &gate), FW_DROP);

    /* DoT (853): the payload is encrypted, the qname is physically
     * unverifiable — the gate opens on server authorization alone even
     * with allowrst=0 (asymmetric condition, spec 2026-11-16), while a
     * 53 query with the same unverified qname stays dropped. */
    struct match_tuple tDot = tuple;
    tDot.sport = htons(DNS_OVER_TLS_PORT);
    struct dns_gate_ctx noAllowDot = gate;
    noAllowDot.allowrst = 0;
    EXPECT_EQ(match_action(&tDot, &key, &noAllowDot), FW_PASS);
    EXPECT_EQ(match_action(&tuple, &key, &noAllowDot), FW_DROP);

    /* condition 2 missing: transport neither UDP nor TCP */
    struct match_tuple tProto = tuple;
    tProto.protocol = IPPROTO_ESP;
    EXPECT_EQ(match_action(&tProto, &key, &gate), FW_DROP);

    /* direction: the gate is an EGRESS query fallback only */
    struct match_tuple tDir = tuple;
    tDir.dir = INGRESS;
    EXPECT_EQ(match_action(&tDir, &key, &gate), FW_DROP);

    /* condition 1 missing: default-allow user — PASS without the gate (the
     * gate may never downgrade a PASS) */
    g_stubDefaultAction = {SK_PASS, SK_PASS};
    EXPECT_EQ(match_action(&tuple, &key, &gate), FW_PASS);
    struct dns_gate_ctx closed = {};
    EXPECT_EQ(match_action(&tuple, &key, &closed), FW_PASS);

    /* NULL gate (ingress caller / flag-off shape): pure default action */
    g_stubDefaultAction = {SK_DROP, SK_DROP};
    EXPECT_EQ(match_action(&tuple, &key, nullptr), FW_DROP);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsGateExplicitRulesFirst001, TestSize.Level1)
{
    ResetDnsStubs();
    /* Red lines: an explicit rule always wins — the gate lives in the
     * no-rule branch and can never override a conflict-algorithm verdict. */
    struct match_tuple tuple;
    struct dns_gate_ctx gate;
    struct bitmap key;
    MakeGateTuple(tuple, gate, key);
    g_stubActionBitmapPtr = &g_stubActionBitmap; /* the rule map exists */

    /* An explicit DENY 53 rule (rule bit 0 matched by the packet and set
     * in the deny action bitmap) — DROP even with the gate fully armed. */
    key.val[0] = 0x1;
    g_stubActionBitmap.val[0] = 0x1;
    EXPECT_EQ(match_action(&tuple, &key, &gate), FW_DROP);

    /* An explicit ALLOW 53 rule (bit 0 matched but not a deny rule) —
     * PASS through the conflict algorithm with the gate CLOSED. */
    g_stubActionBitmap.val[0] = 0x0;
    struct dns_gate_ctx closed = {};
    EXPECT_EQ(match_action(&tuple, &key, &closed), FW_PASS);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, MatchActionConflictMatrix001, TestSize.Level1)
{
    ResetDnsStubs();
    g_stubActionBitmapPtr = &g_stubActionBitmap; /* the rule map exists */
    /* Action bitmap: a SET bit marks a DENY rule row — rule 0 is a deny
     * rule, rule 1 is an allow rule (see the Conflict & Repetition
     * Algorithm comment in match_action: the AND family decides over the
     * matched-rules bitmap in key). Locks the FULL default-action x
     * hit-set matrix of the IP-rule chain so it can never drift from the
     * DNS-domain-gate adjudication (DnsSniffDoubleHit*): on a double hit
     * the verdict follows the DEFAULT action direction — deny wins under
     * default-DENY, allow wins under default-ALLOW. The two chains are
     * deliberately kept in the same matrix; when touching either, run
     * both suites. */
    g_stubActionBitmap.val[0] = 0x1; /* rule 0: deny; rule 1: allow */

    /* Plain egress v4 flow — deliberately NOT DNS-shaped: the conflict
     * algorithm is rule-driven, the tuple only keys the default-action
     * lookup (no domain-cache entries are staged: MatchDomain stays a
     * miss on every row, the cache rescue never interferes). */
    struct match_tuple tuple;
    memset(&tuple, 0, sizeof(tuple));
    tuple.dir = EGRESS;
    tuple.family = AF_INET;
    tuple.protocol = IPPROTO_TCP;
    tuple.uid = 1; /* get_user_id(200100) */
    tuple.appuid = 200100;
    tuple.sport = htons(12345);
    tuple.dport = htons(443);

    /* bitmap_and / bitmap_and_inv MUTATE key->val in place — EVERY call
     * below builds its matched-rules bitmap fresh (never reuse a struct
     * across rows): bit 0 = matched the deny rule, bit 1 = matched the
     * allow rule. */

    /* DEFAULT-DENY rows: matched ∩ deny decides — */
    g_stubDefaultAction = {SK_DROP, SK_DROP};
    /* allow-only hit: the AND clears every bit (no matched rule is a
     * deny rule) — the explicit allow RESCUES the default drop. */
    struct bitmap allowOnly = {};
    allowOnly.val[0] = 0x2;
    EXPECT_EQ(match_action(&tuple, &allowOnly, NULL), FW_PASS);
    /* deny-only hit: the AND keeps the deny bit — DROP. */
    struct bitmap denyOnly = {};
    denyOnly.val[0] = 0x1;
    EXPECT_EQ(match_action(&tuple, &denyOnly, NULL), FW_DROP);
    /* double hit: the AND keeps the deny bit — DROP, the explicit deny
     * outranks the opposing allow (default direction wins). */
    struct bitmap doubleHit = {};
    doubleHit.val[0] = 0x3;
    EXPECT_EQ(match_action(&tuple, &doubleHit, NULL), FW_DROP);

    /* DEFAULT-ALLOW rows: matched ∩ ~deny (the matched allow bits)
     * decides — */
    g_stubDefaultAction = {SK_PASS, SK_PASS};
    /* deny-only hit: no matched allow bit and no domain-cache entry —
     * DROP, the unopposed deny stays effective. */
    struct bitmap denyOnly2 = {};
    denyOnly2.val[0] = 0x1;
    EXPECT_EQ(match_action(&tuple, &denyOnly2, NULL), FW_DROP);
    /* allow-only hit: the matched allow bit survives — PASS. */
    struct bitmap allowOnly2 = {};
    allowOnly2.val[0] = 0x2;
    EXPECT_EQ(match_action(&tuple, &allowOnly2, NULL), FW_PASS);
    /* double hit: the matched allow bit survives — PASS, the explicit
     * allow outranks the opposing deny (default direction wins). */
    struct bitmap doubleHit2 = {};
    doubleHit2.val[0] = 0x3;
    EXPECT_EQ(match_action(&tuple, &doubleHit2, NULL), FW_PASS);
    ResetDnsStubs();
}

HWTEST_F(NetsysBpfNetFirewallTest, SetBuiltInFirewallDefaultActionsNotLoaded001, TestSize.Level1)
{
    /* With the maps not loaded the probe fails cleanly with
     * NETFIREWALL_ERR and never crashes — the deterministic not-loaded
     * shape. A fresh instance starts not-loaded, so no toggle is needed
     * (the loaded-path write is covered by the fixture's SetBpfLoaded(true)
     * init hook and the SetFirewallReservedKeyAction001 loaded branch). */
    auto fw = NewFirewallInstance();
    EXPECT_EQ(fw->SetBuiltInFirewallDefaultActions(), NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsPrewarmTriggerCoalescing001, TestSize.Level1)
{
    /* Resolver-change bursts (network
     * flapping) must coalesce to one in-flight sweep plus a single merged
     * re-run — never N concurrent seconds-long resolver sweeps. Part 1
     * (always runs): with no work the trigger stays inert and must never
     * wedge the in-flight slot. Part 2 (pin-present debug image): seeded
     * prewarm work + a blocking counting resolver; 2 triggers landing while
     * sweep A is blocked must produce exactly 2 resolver invocations
     * (A + one coalesced re-run), not 3 concurrent sweeps. On a retail
     * image (M10-class, no pinned maps) the domain-rule seeding aborts at
     * the trailing ct clear and the case self-skips — count assertions run
     * in the same batch as the pin-device items.
     *
     * SINGLETON EXCEPTION: the prewarm engine is production-bound to the
     * singleton — RunDnsPrewarmTask and the periodic aging thread both
     * resolve through GetInstance(), so a fresh instance would register
     * its resolver where the sweep never looks. This case therefore uses
     * the singleton (like the fixture) and keeps its explicit cleanup:
     * resolver de-registration + user-registry wipe leave the singleton
     * inert for later cases. */
    auto fw = NetsysBpfNetFirewall::GetInstance();

    /* part 1: inert trigger — no resolver registered, nothing to wedge */
    fw->RegisterDnsPrewarmResolver(nullptr);
    fw->TriggerDnsPrewarm();
    fw->TriggerDnsPrewarm();

    /* seed: one exact allow domain for user 100. The trailing Trigger at the
     * seeding end still sees a null resolver and stays inert. */
    std::vector<sptr<NetFirewallDomainRule>> ruleList;
    sptr<NetFirewallDomainRule> rule = (std::make_unique<NetFirewallDomainRule>()).release();
    rule->userId = 100;
    rule->appUid = 1000;
    rule->ruleAction = FirewallRuleAction::RULE_ALLOW;
    NetFirewallDomainParam param;
    param.isWildcard = false;
    param.domain = "prewarm.example.com";
    rule->domains.push_back(param);
    ruleList.push_back(rule);
    int32_t seedRet = fw->SetFirewallDomainRules(ruleList);
    if (seedRet != NETFIREWALL_SUCCESS) {
        /* retail image: no pinned firewall maps (ct clear fails) — same
         * GTEST_SKIP form as the pin-device family. */
        fw->RemoveDnsServerUser(100);
        GTEST_SKIP() << "pinned firewall maps absent (seed ret=" << seedRet << ")";
    }

    /* managed user: default deny + allow-domain registries make
     * DnsServerSyncNeeded(100) true (their trailing triggers still see no
     * resolver — inert) */
    fw->NotifyDnsRuleChanged(100, true);
    fw->NotifyDnsDefaultActionChanged(100, true);

    /* blocking counting resolver: every call counts, the first blocks until
     * released so sweep A is observably in flight. The probe state is heap
     * shared (captured by value) — a straggler sweep running after this test
     * scope only touches the probe, never dangling locals. */
    struct PrewarmProbeState {
        std::mutex mtx;
        int calls = 0;
        std::atomic<bool> blocked{true};
        std::atomic<bool> entered{false};
    };
    auto probe = std::make_shared<PrewarmProbeState>();
    fw->RegisterDnsPrewarmResolver([probe](const std::string &, std::vector<std::string> &ips) -> int32_t {
        {
            std::lock_guard<std::mutex> guard(probe->mtx);
            ++probe->calls;
        }
        probe->entered.store(true);
        while (probe->blocked.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ips = {"192.0.2.1"};
        return NETFIREWALL_SUCCESS;
    });

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    fw->TriggerDnsPrewarm(); /* sweep A starts, blocks inside the resolver */
    while (!probe->entered.load() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(probe->entered.load()) << "sweep A never started";

    fw->TriggerDnsPrewarm();     /* mid-run: must fold into pending, not spawn */
    fw->TriggerDnsPrewarm();     /* mid-run: must fold into pending, not spawn */
    probe->blocked.store(false); /* release A — the tail honors pending exactly once */

    deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    bool secondSweep = false;
    while (!(secondSweep = [&] {
        std::lock_guard<std::mutex> guard(probe->mtx);
        return probe->calls >= 2;
    }()) && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(secondSweep) << "coalesced re-run never started";
    /* settle window: a third (un-coalesced) sweep must never appear */
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::lock_guard<std::mutex> guard(probe->mtx);
        EXPECT_EQ(probe->calls, 2); /* exactly A + one merged re-run */
    }

    /* cleanup: de-register the resolver and wipe the seeded user registries
     * so later tests and later triggers stay inert */
    fw->RegisterDnsPrewarmResolver(nullptr);
    fw->RemoveDnsServerUser(100);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsPrewarmDefaultAllowEligibility001, TestSize.Level1)
{
    /* A default-allow user with a domain-allow rule must be pre-warmed too:
     * the learned entry is what carries the domain allow past explicit deny
     * rules, and without the warm cache a resolver-cache hit (no fresh DNS
     * transit to learn from) leaves the domain allow inoperative. The user
     * is seeded WITHOUT the default-deny registry entry, so the strict
     * server-sync condition stays false while the prewarm sweep must still
     * run. Pin-present images assert the resolver invocation; retail images
     * self-skip at the seeding step.
     *
     * SINGLETON EXCEPTION: same as DnsPrewarmTriggerCoalescing001 — the
     * ffrt prewarm task resolves through GetInstance(), so this case uses
     * the singleton and relies on its explicit cleanup (resolver
     * de-registration + user-registry wipe) to stay residue-free. */
    auto fw = NetsysBpfNetFirewall::GetInstance();
    fw->RegisterDnsPrewarmResolver(nullptr);

    std::vector<sptr<NetFirewallDomainRule>> ruleList;
    sptr<NetFirewallDomainRule> rule = (std::make_unique<NetFirewallDomainRule>()).release();
    rule->userId = 101;
    rule->appUid = 1000;
    rule->ruleAction = FirewallRuleAction::RULE_ALLOW;
    NetFirewallDomainParam param;
    param.isWildcard = false;
    param.domain = "default-allow-prewarm.example.com";
    rule->domains.push_back(param);
    ruleList.push_back(rule);
    int32_t seedRet = fw->SetFirewallDomainRules(ruleList);
    if (seedRet != NETFIREWALL_SUCCESS) {
        fw->RemoveDnsServerUser(101);
        GTEST_SKIP() << "pinned firewall maps absent (seed ret=" << seedRet << ")";
    }
    /* keep the default-allow posture explicit: no default-deny registry
     * entry, so the strict sync condition must stay false. */
    fw->NotifyDnsDefaultActionChanged(101, false);
    EXPECT_FALSE(fw->DnsServerSyncNeeded(101));

    /* counting resolver: heap-shared probe, so a straggler sweep running
     * after this test scope only touches the probe, never dangling locals. */
    struct PrewarmProbeState {
        std::mutex mtx;
        int calls = 0;
        std::atomic<bool> entered{false};
    };
    auto probe = std::make_shared<PrewarmProbeState>();
    fw->RegisterDnsPrewarmResolver([probe](const std::string &, std::vector<std::string> &ips) -> int32_t {
        {
            std::lock_guard<std::mutex> guard(probe->mtx);
            ++probe->calls;
        }
        probe->entered.store(true);
        ips = {"192.0.2.2"};
        return NETFIREWALL_SUCCESS;
    });
    fw->TriggerDnsPrewarm();

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!probe->entered.load() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(probe->entered.load()) << "default-allow user never swept";

    /* settle window: let the sweep finish its cache write before cleanup */
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::lock_guard<std::mutex> guard(probe->mtx);
        EXPECT_GE(probe->calls, 1);
    }

    fw->RegisterDnsPrewarmResolver(nullptr);
    fw->RemoveDnsServerUser(101);
}

HWTEST_F(NetsysBpfNetFirewallTest, DomainCacheAgingSweepConsistency001, TestSize.Level1)
{
    /* One sweep must be consistent and never crash. The fixture guarantees
     * the pinned maps, so the sweep must complete — a broken environment
     * fails loudly instead of passing vacuously on either value. */
    int32_t ret = NewFirewallInstance()->DomainCacheAgingSweep();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearDnsQueryRecordMapNoCrash001, TestSize.Level1)
{
    /* The record-table clear must be safe in every configuration —
     * without the pinned map it is a silent no-op, with it the table is
     * simply emptied. */
    NewFirewallInstance()->ClearDnsQueryRecordMap();
}

/* ===== DNS server authorization maps + pre-warm engine =====
 * Pure-function cases run everywhere; map cases branch on a BpfMapper probe
 * because the M10-class retail image never pins the maps (an open failure
 * must surface as NETFIREWALL_ERR, never a crash). */

HWTEST_F(NetsysBpfNetFirewallTest, BuildDnsServerKeys001, TestSize.Level1)
{
    /* 完整性校验: valid mix kept, duplicates collapsed, empty and
     * malformed entries skipped — never a partial or garbage key. */
    std::vector<std::string> servers = {"8.8.8.8", "2001:4860:4860::8888", "8.8.8.8", "", "not-an-ip", "8.8.4.4"};
    std::vector<DnsServerV4Key> v4Keys;
    std::vector<DnsServerV6Key> v6Keys;
    EXPECT_EQ(NetsysBpfNetFirewall::BuildDnsServerKeys(servers, 100, v4Keys, v6Keys), NETFIREWALL_SUCCESS);
    EXPECT_EQ(v4Keys.size(), 2U);
    EXPECT_EQ(v6Keys.size(), 1U);
    EXPECT_EQ(v4Keys[0].uid, 100U);
    struct in_addr expect = {};
    ASSERT_EQ(inet_pton(AF_INET, "8.8.8.8", &expect), 1);
    EXPECT_EQ(v4Keys[0].ip, expect.s_addr);
    ASSERT_EQ(inet_pton(AF_INET, "8.8.4.4", &expect), 1);
    EXPECT_EQ(v4Keys[1].ip, expect.s_addr);

    /* empty list -> zero keys is a valid (full-delete) sync set */
    v4Keys.clear();
    v6Keys.clear();
    std::vector<std::string> none;
    EXPECT_EQ(NetsysBpfNetFirewall::BuildDnsServerKeys(none, 100, v4Keys, v6Keys), NETFIREWALL_SUCCESS);
    EXPECT_TRUE(v4Keys.empty());
    EXPECT_TRUE(v6Keys.empty());
}

HWTEST_F(NetsysBpfNetFirewallTest, BuildDnsServerKeysCapacity002, TestSize.Level1)
{
    /* Every user is capped at DNS_SERVER_MAX_PER_USER (8) COMBINED
     * across both families — the map can never be flooded per user. */
    std::vector<std::string> servers;
    for (int i = 1; i <= 5; i++) {
        servers.push_back("10.0.0." + std::to_string(i));
    }
    for (int i = 1; i <= 5; i++) {
        servers.push_back("2001:db8::" + std::to_string(i));
    }
    std::vector<DnsServerV4Key> v4Keys;
    std::vector<DnsServerV6Key> v6Keys;
    EXPECT_EQ(NetsysBpfNetFirewall::BuildDnsServerKeys(servers, 100, v4Keys, v6Keys), NETFIREWALL_SUCCESS);
    EXPECT_EQ(v4Keys.size() + v6Keys.size(), 8U);
    EXPECT_EQ(v4Keys.size(), 5U); /* first five valid entries are all v4 */
    EXPECT_EQ(v6Keys.size(), 3U); /* only three v6 fit after the cap */

    /* invalid userId is rejected before anything is built */
    EXPECT_EQ(NetsysBpfNetFirewall::BuildDnsServerKeys(servers, -1, v4Keys, v6Keys), NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsServerKeyLayout001, TestSize.Level1)
{
    /* byte-exact layout (device is little-endian): uid host order, ip network
     * order — exactly the bytes the data-plane lookup will build from the
     * packet. No padding may appear (hash keys must be byte-stable). */
    EXPECT_EQ(sizeof(DnsServerV4Key), 8U);
    EXPECT_EQ(sizeof(DnsServerV6Key), 20U);
    DnsServerV4Key k4 = {};
    k4.uid = 100;
    ASSERT_EQ(inet_pton(AF_INET, "8.8.8.8", &k4.ip), 1);
    unsigned char expectV4[8] = {100, 0, 0, 0, 8, 8, 8, 8};
    EXPECT_EQ(memcmp(&k4, expectV4, sizeof(k4)), 0);
    DnsServerV6Key k6 = {};
    k6.uid = 100;
    ASSERT_EQ(inet_pton(AF_INET6, "2001:db8::1", &k6.ip), 1);
    unsigned char expectV6[20] = {100, 0, 0, 0, 0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01};
    EXPECT_EQ(memcmp(&k6, expectV6, sizeof(k6)), 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, DnsServerSyncNeeded001, TestSize.Level1)
{
    /* Write condition matrix: entries exist ONLY for (>=1 domain
     * allow rule) x (egress default deny). Isolated userId so the singleton
     * registry never leaks into other cases; removed again at the end. */
    const int32_t userId = 990001;
    auto bpf = NewFirewallInstance();
    EXPECT_FALSE(bpf->DnsServerSyncNeeded(userId));
    bpf->NotifyDnsRuleChanged(userId, true);
    EXPECT_FALSE(bpf->DnsServerSyncNeeded(userId)); /* default not deny yet */
    bpf->NotifyDnsDefaultActionChanged(userId, true);
    EXPECT_TRUE(bpf->DnsServerSyncNeeded(userId));     /* condition met */
    bpf->NotifyDnsDefaultActionChanged(userId, false); /* flipped to allow */
    EXPECT_FALSE(bpf->DnsServerSyncNeeded(userId));
    bpf->NotifyDnsDefaultActionChanged(userId, true);
    EXPECT_TRUE(bpf->DnsServerSyncNeeded(userId));
    bpf->NotifyDnsRuleChanged(userId, false); /* allow rules gone */
    EXPECT_FALSE(bpf->DnsServerSyncNeeded(userId));
    bpf->NotifyDnsRuleChanged(userId, true);
    bpf->NotifyDnsDefaultActionChanged(userId, true);
    EXPECT_TRUE(bpf->DnsServerSyncNeeded(userId));
    bpf->RemoveDnsServerUser(userId); /* user removal drops everything */
    EXPECT_FALSE(bpf->DnsServerSyncNeeded(userId));
    /* negative userId is ignored entirely (no registry pollution) */
    bpf->NotifyDnsRuleChanged(-1, true);
    EXPECT_FALSE(bpf->DnsServerSyncNeeded(-1));
}

HWTEST_F(NetsysBpfNetFirewallTest, SetDnsServerMaps001, TestSize.Level1)
{
    /* probe 容错: M10-class image is flag-off and pins no DNS server map —
     * the interface must report NETFIREWALL_ERR, never crash; with pins the
     * delta write (add, then shrink) must reconcile exactly. */
    const int32_t userId = 990002;
    auto bpf = NewFirewallInstance();
    BpfMapper<DnsServerV4Key, DnsServerVal> probe(MAP_PATH(DNS_SERVER_V4_MAP), BPF_F_RDONLY);
    bool pinned = probe.IsValid();
    std::vector<std::string> servers = {"8.8.8.8", "2001:4860:4860::8888"};
    int32_t ret = bpf->SetDnsServerMaps(userId, servers);
    if (pinned) {
        EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
        BpfMapper<DnsServerV4Key, DnsServerVal> rdMap(MAP_PATH(DNS_SERVER_V4_MAP), BPF_F_RDONLY);
        DnsServerV4Key k4 = {};
        k4.uid = static_cast<uint32_t>(userId);
        ASSERT_EQ(inet_pton(AF_INET, "8.8.8.8", &k4.ip), 1);
        DnsServerVal val = {};
        ASSERT_EQ(rdMap.Read(k4, val), 0);
        EXPECT_EQ(val.managed, 1);
        /* shrink the set: 8.8.8.8 must be deleted (delta, not append) */
        std::vector<std::string> onlyV6 = {"2001:4860:4860::8888"};
        EXPECT_EQ(bpf->SetDnsServerMaps(userId, onlyV6), NETFIREWALL_SUCCESS);
        DnsServerVal gone = {};
        EXPECT_NE(rdMap.Read(k4, gone), 0);
    } else {
        EXPECT_EQ(ret, NETFIREWALL_ERR);
    }
    EXPECT_EQ(bpf->ClearDnsServerMaps(userId), pinned ? NETFIREWALL_SUCCESS : NETFIREWALL_ERR);
}

HWTEST_F(NetsysBpfNetFirewallTest, AddPrewarmDomainCache001, TestSize.Level1)
{
    /* Wildcard/empty/negative-uid rejected (完整性校验); with pins
     * the learned entry carries the full pre-warm value spec — uid = rule
     * userId, appuid = 0, ttl = 3600, fresh CLOCK_MONOTONIC timestamp. */
    const int32_t userId = 990003;
    auto bpf = NewFirewallInstance();
    EXPECT_EQ(bpf->AddPrewarmDomainCache(userId, "*.example.com", {"8.8.8.8"}), NETFIREWALL_ERR);
    EXPECT_EQ(bpf->AddPrewarmDomainCache(userId, "", {"8.8.8.8"}), NETFIREWALL_ERR);
    EXPECT_EQ(bpf->AddPrewarmDomainCache(-1, "a.example.com", {"8.8.8.8"}), NETFIREWALL_ERR);

    BpfMapper<Ipv4LpmKey, DomainValue> probe(MAP_PATH(DOMAIN_IPV4_MAP), BPF_F_RDONLY);
    bool pinned = probe.IsValid();
    std::vector<std::string> ips = {"203.0.113.10", "2001:db8::99", "bogus-ip"};
    int32_t ret = bpf->AddPrewarmDomainCache(userId, "prewarm.example.com", ips);
    if (pinned) {
        EXPECT_EQ(ret, NETFIREWALL_SUCCESS);
        BpfMapper<Ipv4LpmKey, DomainValue> rd4(MAP_PATH(DOMAIN_IPV4_MAP), BPF_F_RDONLY);
        Ipv4LpmKey k4 = {};
        k4.prefixlen = 32;
        ASSERT_EQ(inet_pton(AF_INET, "203.0.113.10", &k4.data), 1);
        DomainValue v4 = {};
        ASSERT_EQ(rd4.Read(k4, v4), 0);
        EXPECT_EQ(v4.uid, static_cast<uint32_t>(userId));
        EXPECT_EQ(v4.appuid, 0U);
        EXPECT_EQ(v4.ttl, 3600U);
        EXPECT_GT(v4.timestamp, 0U);
        BpfMapper<Ipv6LpmKey, DomainValue> rd6(MAP_PATH(DOMAIN_IPV6_MAP), BPF_F_RDONLY);
        Ipv6LpmKey k6 = {};
        k6.prefixlen = 128;
        ASSERT_EQ(inet_pton(AF_INET6, "2001:db8::99", &k6.data), 1);
        DomainValue v6 = {};
        ASSERT_EQ(rd6.Read(k6, v6), 0);
        EXPECT_EQ(v6.uid, static_cast<uint32_t>(userId));
        EXPECT_EQ(v6.appuid, 0U);
        /* clean up so the learned map stays pristine for the other cases */
        BpfMapper<Ipv4LpmKey, DomainValue> wr4(MAP_PATH(DOMAIN_IPV4_MAP), BPF_F_WRONLY);
        EXPECT_EQ(wr4.Delete(k4), 0);
        BpfMapper<Ipv6LpmKey, DomainValue> wr6(MAP_PATH(DOMAIN_IPV6_MAP), BPF_F_WRONLY);
        EXPECT_EQ(wr6.Delete(k6), 0);
    } else {
        EXPECT_EQ(ret, NETFIREWALL_ERR);
    }
}

/* ===================== A' AC builder tests =====================
 * Wire-encoding, differential (vs the legacy GetDomainHashKey
 * normalization and vs the real BPF-side ac_fold) and user-space walk
 * tests. The walk tests run the REAL frozen scan subprograms
 * (ac_prefix_scan / ac_suffix_scan from netfirewall_domain.h) against mock
 * wire names and the builder's emitted tables installed in the AC stub
 * maps — the builder and its BPF consumer are exercised against each
 * other, never against a re-implementation. */
namespace {

/* Wire-format name from a dotted string (an independent label-split
 * implementation: it must agree with the builder's internal split — the
 * differential tests assert exactly that). */
std::vector<uint8_t> AcWireName(const std::string &dotted, bool terminate = true)
{
    std::vector<uint8_t> p;
    size_t start = 0;
    while (start < dotted.size()) {
        size_t dot = dotted.find('.', start);
        if (dot == std::string::npos) {
            dot = dotted.size();
        }
        p.push_back(static_cast<uint8_t>(dot - start));
        p.insert(p.end(), dotted.begin() + start, dotted.begin() + dot);
        start = dot + 1;
    }
    if (terminate) {
        p.push_back(0); /* root label terminator */
    }
    return p;
}

/* Folded dotted form of a rule domain ('*' stripped) — the expected
 * decode of every builder pattern. */
std::string AcFoldDotted(const std::string &domain)
{
    std::string out;
    for (char ch : domain) {
        if (ch == '*') {
            continue;
        }
        out += (ch == '.') ? '.' : static_cast<char>(AcFoldByte(static_cast<uint8_t>(ch)));
    }
    return out;
}

std::string AcReverseString(const std::string &s)
{
    std::string r;
    for (size_t i = s.size(); i-- > 0;) {
        r += s[i];
    }
    return r;
}

/* Decode a pattern back to its folded dotted form. Both encodings follow
 * the builder's BuildPattern emission order, which is the exact class
 * sequence the BPF scans consume: prefix = LEN(n), n chars, ... anchored
 * at the name start; suffix = TERM, then per label (last first) the
 * reversed content chars followed by its LEN, anchored at the terminator.
 * The wildcard tails decode with their '*' back: the MORE class
 * ("*.x.com") terminates the suffix stream, a truncated char group
 * without its closing LEN renders the leading '*' ("*w.x.com"), and a
 * SKIP class renders the trailing '*' ("x.f*" / "x.*").
 * Returns "?" when the structural expectations do not hold (a mismatch
 * always fails the caller's EXPECT_EQ against the folded input). */
std::string AcDecodePattern(const std::vector<uint32_t> &cls, bool isSuffix)
{
    std::vector<std::string> labels;
    bool leadingFragment = false; /* "*f.x.com": truncated tail, no closing LEN */
    bool trailingStar = false;    /* "x.f*" / "x.*": SKIP + (optional) fragment */
    std::string frag;
    size_t i = 0;
    if (isSuffix) {
        if (cls.empty() || cls[0] != AC_CLS_TERM) {
            return "?";
        }
        i = 1;
    }
    while (i < cls.size()) {
        std::string label;
        if (isSuffix) {
            /* backward encoding: chars first (reversed inside the
             * label), then the LEN that closes the group */
            while (i < cls.size() && (cls[i] & 0x100U) != 0 && cls[i] != AC_CLS_TERM && cls[i] != AC_BUILDER_CLS_MORE) {
                label += static_cast<char>(cls[i] & 0xFFU);
                ++i;
            }
            if (i >= cls.size()) {
                /* truncated tail: a leading-star fragment ("*w.x.com") —
                 * >=1 content chars that never meet their closing LEN */
                if (label.empty()) {
                    return "?";
                }
                labels.push_back(AcReverseString(label));
                leadingFragment = true;
                break;
            }
            size_t n = cls[i];
            if (n == 0 || n > DNS_LABEL_MAX_LEN || n != label.size()) {
                return "?";
            }
            ++i;
            labels.push_back(AcReverseString(label));
            if (i < cls.size() && cls[i] == AC_BUILDER_CLS_MORE) {
                /* subdomain-only anchor tail ("*.x.com"): one MORE class
                 * demands at least one further label BEFORE the pattern's
                 * first label — a structural terminator, nothing may
                 * follow it */
                if (i + 1 != cls.size()) {
                    return "?";
                }
                ++i;
                break;
            }
        } else {
            /* forward encoding: LEN(n) first, then the label's chars; a
             * SKIP class opens the trailing-star tail (the fragment
             * prefixing the final label, or nothing for "x.*") */
            if (cls[i] == AC_BUILDER_CLS_SKIP) {
                ++i;
                trailingStar = true;
                while (i < cls.size()) {
                    if ((cls[i] & 0x100U) == 0) {
                        return "?";
                    }
                    frag += static_cast<char>(cls[i] & 0xFFU);
                    ++i;
                }
                break;
            }
            if (cls[i] == 0 || cls[i] > DNS_LABEL_MAX_LEN) {
                return "?"; /* a length class must be [1,63] */
            }
            size_t n = cls[i];
            ++i;
            if (i + n > cls.size()) {
                return "?";
            }
            for (size_t j = 0; j < n; ++j) {
                if ((cls[i + j] & 0x100U) == 0) {
                    return "?";
                }
                label += static_cast<char>(cls[i + j] & 0xFFU);
            }
            i += n;
            labels.push_back(label);
        }
    }
    std::string out;
    if (isSuffix) {
        /* the suffix pattern emits the LAST label first: decode order is
         * reversed for the dotted form (the leading-star fragment is the
         * last decoded group, so it lands at the front) */
        for (size_t k = labels.size(); k-- > 0;) {
            if (k + 1 < labels.size()) {
                out += '.';
            }
            out += labels[k];
        }
        if (leadingFragment) {
            out = "*" + out;
        }
    } else {
        for (size_t k = 0; k < labels.size(); ++k) {
            if (k > 0) {
                out += '.';
            }
            out += labels[k];
        }
        if (trailingStar) {
            if (!labels.empty()) {
                out += '.';
            }
            out += frag + "*";
        }
    }
    return out;
}

struct AcRuleSpec {
    uint32_t uid;
    uint32_t appuid; /* 0 = the wild namespace */
    bool isAllow;
    const char *domain;
};

AcDomainTableBuilder AcMakeBuilder(const std::vector<AcRuleSpec> &specs)
{
    AcDomainTableBuilder builder;
    for (const auto &s : specs) {
        EXPECT_TRUE(builder.AddRule(s.uid, s.appuid, s.isAllow, s.domain)) << "spec domain: " << s.domain;
    }
    return builder;
}

/* Install a builder's emitted tables into the stub maps under generation
 * `gen` (the production "write the inactive generation" step). */
bool AcInstallTables(AcDomainTableBuilder &builder, uint32_t gen)
{
    std::vector<AcTransEntry> prefix;
    std::vector<AcTransEntry> suffix;
    if (!builder.Emit(gen, prefix, suffix)) {
        return false;
    }
    for (const auto &e : prefix) {
        g_stubAcPrefixMap[e.key] = e.val;
    }
    for (const auto &e : suffix) {
        g_stubAcSuffixMap[e.key] = e.val;
    }
    g_stubAcGen = gen;
    g_stubAcGenPresent = true;
    return true;
}

/* The four scans of one name, exactly in the parse_dns_query calling
 * order (exact appuid, then wild appuid=0, per anchor) — combined and
 * adjudicated with the same masks the BPF caller uses. */
struct AcQuadVerdict {
    uint32_t prefixExact = 0;
    uint32_t prefixWild = 0;
    uint32_t suffixExact = 0;
    uint32_t suffixWild = 0;
    uint32_t Combined() const
    {
        return prefixExact | prefixWild | suffixExact | suffixWild;
    }
    bool Denied() const
    {
        return (Combined() & (AC_FLAG_PREFIX_DENY | AC_FLAG_SUFFIX_DENY)) != 0;
    }
    bool Allowed() const
    {
        return (Combined() & (AC_FLAG_PREFIX_ALLOW | AC_FLAG_SUFFIX_ALLOW)) != 0;
    }
};

AcQuadVerdict AcScanName(MockSkb &mock, uint32_t uid, uint32_t appuid)
{
    struct domain_hash_key key = {};
    struct domain_hash_key *kp = &key;
    AcQuadVerdict v;
    kp->uid = uid;
    kp->appuid = appuid;
    v.prefixExact = ac_prefix_scan(mock.Done(), 0, kp);
    kp->appuid = 0;
    v.prefixWild = ac_prefix_scan(mock.Done(), 0, kp);
    kp->appuid = appuid;
    v.suffixExact = ac_suffix_scan(mock.Done(), 0, kp);
    kp->appuid = 0;
    v.suffixWild = ac_suffix_scan(mock.Done(), 0, kp);
    return v;
}

MockSkb AcMockFor(const std::string &dotted, bool terminate = true)
{
    MockSkb mock;
    mock.Reset(AF_INET);
    mock.Append(AcWireName(dotted, terminate));
    mock.Done();
    return mock;
}

/* One-shot scan helper: AcScanName takes a mutable MockSkb reference,
 * a temporary does not bind — build it here under a name first. */
AcQuadVerdict AcScanQuery(const std::string &dotted, uint32_t uid, uint32_t appuid)
{
    MockSkb mock = AcMockFor(dotted);
    return AcScanName(mock, uid, appuid);
}
} // namespace

HWTEST_F(NetsysBpfNetFirewallTest, AcFoldDifferential001, TestSize.Level0)
{
    /* The control-plane fold must be byte-identical to the REAL BPF-side
     * ac_fold (included from netfirewall_domain.h) for every input byte. */
    for (uint32_t c = 0; c <= 0xFF; ++c) {
        uint8_t in = static_cast<uint8_t>(c);
        EXPECT_EQ(AcFoldByte(in), ac_fold(in)) << "byte " << c;
    }
    EXPECT_EQ(AcFoldByte('A'), static_cast<uint8_t>('a'));
    EXPECT_EQ(AcFoldByte('Z'), static_cast<uint8_t>('z'));
    EXPECT_EQ(AcFoldByte('a'), static_cast<uint8_t>('a'));
    EXPECT_EQ(AcFoldByte('0'), static_cast<uint8_t>('0'));
    EXPECT_EQ(AcFoldByte(0x00), 0x00);
    EXPECT_EQ(AcFoldByte(0x80), 0x80);
}

HWTEST_F(NetsysBpfNetFirewallTest, AcPatternSuffixEncode001, TestSize.Level0)
{
    std::vector<uint32_t> cls;
    /* "example.com" -> TERM, m o c, LEN(3), e l p m a x e, LEN(7):
     * the exact class sequence ac_suffix_scan consumes walking a matching
     * name backwards from the terminator. */
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("example.com", false, cls));
    EXPECT_EQ(cls,
              (std::vector<uint32_t>{AC_CLS_TERM, AC_CLS_CHAR('m'), AC_CLS_CHAR('o'), AC_CLS_CHAR('c'), AC_CLS_LEN(3),
                                     AC_CLS_CHAR('e'), AC_CLS_CHAR('l'), AC_CLS_CHAR('p'), AC_CLS_CHAR('m'),
                                     AC_CLS_CHAR('a'), AC_CLS_CHAR('x'), AC_CLS_CHAR('e'), AC_CLS_LEN(7)}));

    /* single label "com" -> TERM, m o c, LEN(3) */
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("com", false, cls));
    EXPECT_EQ(
        cls, (std::vector<uint32_t>{AC_CLS_TERM, AC_CLS_CHAR('m'), AC_CLS_CHAR('o'), AC_CLS_CHAR('c'), AC_CLS_LEN(3)}));

    /* mixed case folds to the same pattern */
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("ExAmPlE.CoM", false, cls));
    std::vector<uint32_t> folded;
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("example.com", false, folded));
    EXPECT_EQ(cls, folded);
}

HWTEST_F(NetsysBpfNetFirewallTest, AcPatternPrefixEncode001, TestSize.Level0)
{
    std::vector<uint32_t> cls;
    /* "www.example*" -> LEN(3), w w w, SKIP, e x a m p l e — anchored at
     * the name start, no terminator class. The trailing-star family: the
     * SKIP consumes the final label's LEN (any 1..63) and the fragment
     * "example" PREFIXES that label (an allow verdict is confined to it,
     * a deny verdict is right-unconstrained — see
     * AcScanTrailingStarSemantics001). The legacy label-exact encoding
     * (LEN(3),www,LEN(7),example — third label exactly "example",
     * anything to the right) is the plain no-star prefix form. */
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("www.example*", true, cls));
    EXPECT_EQ(cls, (std::vector<uint32_t>{AC_CLS_LEN(3), AC_CLS_CHAR('w'), AC_CLS_CHAR('w'), AC_CLS_CHAR('w'),
                                          AC_BUILDER_CLS_SKIP, AC_CLS_CHAR('e'), AC_CLS_CHAR('x'), AC_CLS_CHAR('a'),
                                          AC_CLS_CHAR('m'), AC_CLS_CHAR('p'), AC_CLS_CHAR('l'), AC_CLS_CHAR('e')}));

    /* mixed case folds identically */
    std::vector<uint32_t> folded;
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("WWW.Example*", true, folded));
    EXPECT_EQ(cls, folded);

    /* "www.example.c*" — a partial final label ("c" prefixes "com"):
     * full labels www+example exact, SKIP, then the fragment. Legacy
     * stripped the '*' and demanded the third label be the literal "c"
     * (measured on-device: never fired, the admin's intent was
     * www.example.com). */
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("www.example.c*", true, cls));
    EXPECT_EQ(cls, (std::vector<uint32_t>{AC_CLS_LEN(3), AC_CLS_CHAR('w'), AC_CLS_CHAR('w'), AC_CLS_CHAR('w'),
                                          AC_CLS_LEN(7), AC_CLS_CHAR('e'), AC_CLS_CHAR('x'), AC_CLS_CHAR('a'),
                                          AC_CLS_CHAR('m'), AC_CLS_CHAR('p'), AC_CLS_CHAR('l'), AC_CLS_CHAR('e'),
                                          AC_BUILDER_CLS_SKIP, AC_CLS_CHAR('c')}));
    EXPECT_EQ(AcDecodePattern(cls, false), "www.example.c*");
}

HWTEST_F(NetsysBpfNetFirewallTest, AcPatternReject001, TestSize.Level0)
{
    std::vector<uint32_t> cls;
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("", false, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*", false, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("***", true, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("a..b", false, cls));               /* empty label */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("a.", false, cls));                 /* trailing dot */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(".a", false, cls));                 /* leading dot */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(std::string(64, 'a'), false, cls)); /* label > 63 */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(std::string(64, 'a') + ".com", false, cls));
    /* text length 127 with valid labels (63 + 1 + 63): the pattern cannot
     * fit the 128-position scan window (prefix 128 > 127 content
     * positions, suffix 129 > 128) — rejected, legacy accepted it as a
     * never-matchable rule. */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(std::string(63, 'a') + "." + std::string(63, 'b'), false, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(std::string(63, 'a') + "." + std::string(63, 'b'), true, cls));
    /* text length 126 (63 + 1 + 62) is the largest representable form:
     * prefix 127 classes, suffix 128 classes. */
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern(std::string(63, 'a') + "." + std::string(62, 'b'), true, cls));
    EXPECT_EQ(cls.size(), 127U);
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern(std::string(63, 'a') + "." + std::string(62, 'b'), false, cls));
    EXPECT_EQ(cls.size(), 128U);

    /* Star-form gate (fail-visible): a mid-domain, duplicated or dual-end
     * '*' is unrepresentable — the legacy normalization stripped them
     * silently and armed a DIFFERENT rule than the admin wrote. */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("www.*example.com", false, cls)); /* mid-domain star */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*bai*du*.com", false, cls));     /* multi star */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*.example.*", false, cls));      /* dual end */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*w.example.c*", false, cls));    /* dual end */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(".*", true, cls));                /* bare ".*" */
    /* The whole-final-label trailing form is the NEW capability (the
     * legacy path rejected it through the empty trailing label). */
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("www.example.*", true, cls));
    EXPECT_EQ(AcDecodePattern(cls, false), "www.example.*");
}

HWTEST_F(NetsysBpfNetFirewallTest, AcPatternDifferentialVsLegacy001, TestSize.Level0)
{
    /* Differential against the legacy GetDomainHashKey normalization:
     * same accept/reject surface (modulo the AC-only label/fit checks) and
     * the same text content — the builder's label split loses nothing and
     * folds exactly what the scan will fold. */
    auto bpfNet = std::make_shared<NetsysBpfNetFirewall>();
    const std::vector<std::string> agreed = {"example.com",
                                             "www.example.com",
                                             "a.b.c.d.example.com",
                                             "com",
                                             "x",
                                             "www.example*",
                                             "www.example.c*",
                                             "*w.example.com",
                                             "ExAmPlE.CoM",
                                             "WWW.Example*",
                                             std::string(63, 'a') + "." + std::string(62, 'b')};
    for (const auto &d : agreed) {
        DomainHashKey key = {};
        bool legacySuffix = bpfNet->GetDomainHashKey(d, key, true);
        bool legacyPrefix = bpfNet->GetDomainHashKey(d, key, false);
        std::vector<uint32_t> suffixCls;
        std::vector<uint32_t> prefixCls;
        EXPECT_EQ(AcDomainTableBuilder::BuildPattern(d, false, suffixCls), legacySuffix) << "suffix accept: " << d;
        EXPECT_EQ(AcDomainTableBuilder::BuildPattern(d, true, prefixCls), legacyPrefix) << "prefix accept: " << d;
        if (suffixCls.empty()) {
            continue;
        }
        /* decoded patterns == folded dotted input (both directions) —
         * star-free domains only: the partial-star forms re-encode their
         * '*' (SKIP class / truncated fragment), see
         * AcPatternTrailingStarEncode001 and AcPatternLeadingStarEncode001.
         * The CHAR-sequence checks below hold for every form: the SKIP /
         * MORE / LEN classes are not CHARs and the fragment keeps the
         * text's char order. */
        if (d.find('*') == std::string::npos) {
            EXPECT_EQ(AcDecodePattern(prefixCls, false), AcFoldDotted(d)) << "prefix decode: " << d;
            EXPECT_EQ(AcDecodePattern(suffixCls, true), AcFoldDotted(d)) << "suffix decode: " << d;
        }
        /* the CHAR class sequence is the folded text (dots dropped); the
         * suffix walk sees the same bytes in reverse — the legacy
         * reversed-key view of the same rule */
        std::string textNoDots;
        std::string revTextNoDots;
        for (char ch : AcFoldDotted(d)) {
            if (ch != '.') {
                textNoDots += ch;
            }
        }
        revTextNoDots = AcReverseString(textNoDots);
        std::string gotFwd;
        for (uint32_t c : prefixCls) {
            /* the pseudo classes carry the 0x100 bit too (SKIP 0x301,
             * MORE 0x300) — without the explicit exclusion a trailing-star
             * pattern's SKIP row leaked '\x01' into the extracted chars */
            if ((c & 0x100U) && c != AC_BUILDER_CLS_SKIP && c != AC_BUILDER_CLS_MORE) {
                gotFwd += static_cast<char>(c & 0xFFU);
            }
        }
        std::string gotRev;
        for (uint32_t c : suffixCls) {
            if ((c & 0x100U) && c != AC_BUILDER_CLS_SKIP && c != AC_BUILDER_CLS_MORE) {
                gotRev += static_cast<char>(c & 0xFFU);
            }
        }
        EXPECT_EQ(gotFwd, textNoDots) << "prefix chars: " << d;
        EXPECT_EQ(gotRev, revTextNoDots) << "suffix chars: " << d;
    }
    /* Divergence (documented): a 127-char text with valid labels is
     * accepted by the legacy normalization but cannot fit the AC scan
     * window — the builder rejects it (skip + log in production). */
    std::string longDomain = std::string(63, 'a') + "." + std::string(63, 'b');
    DomainHashKey key = {};
    EXPECT_TRUE(bpfNet->GetDomainHashKey(longDomain, key, true));
    std::vector<uint32_t> cls;
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(longDomain, false, cls));

    /* Divergence (documented, F1): the subdomain anchor "*.example.com".
     * Suffix family: both accept — the legacy reversed-LPM kept the leading
     * dot as a one-more-label requirement; the builder strips it and
     * re-expresses the requirement as the MORE tail class (decode loses the
     * dot, semantics identical). Prefix family: the legacy key
     * "..example.com" is a never-matching dead key; the builder rejects a
     * leading-dot prefix outright — no wire name starts with a dot. */
    DomainHashKey anchorKey = {};
    EXPECT_TRUE(bpfNet->GetDomainHashKey("*.example.com", anchorKey, true));
    std::vector<uint32_t> anchorCls;
    EXPECT_TRUE(AcDomainTableBuilder::BuildPattern("*.example.com", false, anchorCls));
    EXPECT_EQ(AcDecodePattern(anchorCls, true), "example.com");
    EXPECT_EQ(anchorCls.back(), AC_BUILDER_CLS_MORE);
    EXPECT_TRUE(bpfNet->GetDomainHashKey("*.example.com", anchorKey, false));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*.example.com", true, anchorCls));
}

HWTEST_F(NetsysBpfNetFirewallTest, AcPatternLeadingStarEncode001, TestSize.Level0)
{
    /* F1: the subdomain anchor "*.x.com". Before the fix the '*' strip left
     * a leading dot, the empty first label made the pattern unrepresentable
     * and the builder silently dropped the rule (on-device:
     * "skip invalid domain: ... *.baidu.com"). Now the dot is stripped and
     * the "at least one more label" requirement is re-expressed as the MORE
     * tail class — scan-side semantics identical to the legacy reversed-LPM
     * leading-dot anchor (1535d64b: subdomains only). */
    std::vector<uint32_t> cls;
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("*.example.com", false, cls));
    EXPECT_EQ(cls, (std::vector<uint32_t>{AC_CLS_TERM, AC_CLS_CHAR('m'), AC_CLS_CHAR('o'), AC_CLS_CHAR('c'),
                                          AC_CLS_LEN(3), AC_CLS_CHAR('e'), AC_CLS_CHAR('l'), AC_CLS_CHAR('p'),
                                          AC_CLS_CHAR('m'), AC_CLS_CHAR('a'), AC_CLS_CHAR('x'), AC_CLS_CHAR('e'),
                                          AC_CLS_LEN(7), AC_BUILDER_CLS_MORE}));
    EXPECT_EQ(AcDecodePattern(cls, true), "example.com");

    /* mixed case folds identically */
    std::vector<uint32_t> folded;
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("*.ExAmPlE.CoM", false, folded));
    EXPECT_EQ(cls, folded);

    /* single-label anchor */
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("*.com", false, cls));
    EXPECT_EQ(AcDecodePattern(cls, true), "com");
    EXPECT_EQ(cls.back(), AC_BUILDER_CLS_MORE);

    /* a star that opens a NON-EMPTY first label is the leading-fragment
     * form: the label right before the pattern body ends with the
     * fragment, everything further left is the glob '*' — the name
     * itself matches (glob '*' = empty). Encoded as a TRUNCATED backward
     * pattern: the fragment's reversed chars with no closing LEN and no
     * MORE (the legacy normalization stripped the star and downgraded
     * the rule to the bare domain + subdomains — measured on-device,
     * never the admin's intent). */
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("*w.example.com", false, cls));
    EXPECT_EQ(cls, (std::vector<uint32_t>{AC_CLS_TERM, AC_CLS_CHAR('m'), AC_CLS_CHAR('o'), AC_CLS_CHAR('c'),
                                          AC_CLS_LEN(3), AC_CLS_CHAR('e'), AC_CLS_CHAR('l'), AC_CLS_CHAR('p'),
                                          AC_CLS_CHAR('m'), AC_CLS_CHAR('a'), AC_CLS_CHAR('x'), AC_CLS_CHAR('e'),
                                          AC_CLS_LEN(7), AC_CLS_CHAR('w')}));
    EXPECT_EQ(AcDecodePattern(cls, true), "*w.example.com");
    EXPECT_NE(cls.back(), AC_BUILDER_CLS_MORE);
    /* single-label fragment ("*example.com": the label before "com" ends
     * with "example") */
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("*example.com", false, cls));
    EXPECT_EQ(AcDecodePattern(cls, true), "*example.com");
    EXPECT_NE(cls.back(), AC_BUILDER_CLS_MORE);
    /* lone-fragment form ("*baiducom": the LAST label ends with it) */
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("*baiducom", false, cls));
    EXPECT_EQ(AcDecodePattern(cls, true), "*baiducom");

    /* rejects: bare anchor, star-only, empty label behind the anchor,
     * literal leading dot, and the prefix family never takes the anchor */
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*.", false, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*", false, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*.a..b", false, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern(".a", false, cls));
    EXPECT_FALSE(AcDomainTableBuilder::BuildPattern("*.example.com", true, cls));
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanLeadingStarSuffix001, TestSize.Level0)
{
    /* F1 at the scan level: "*.example.com" covers SUBDOMAINS ONLY; the
     * bare domain needs the no-star spelling. */
    ResetDnsStubs();
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "*.example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));

    MockSkb m1 = AcMockFor("a.example.com");
    EXPECT_TRUE(AcScanName(m1, 100, 12345).Allowed());
    MockSkb m2 = AcMockFor("www.example.com");
    EXPECT_TRUE(AcScanName(m2, 100, 12345).Allowed());
    MockSkb m3 = AcMockFor("a.b.example.com");
    EXPECT_TRUE(AcScanName(m3, 100, 12345).Allowed());

    /* the bare domain itself is NOT covered by the anchored form */
    MockSkb m4 = AcMockFor("example.com");
    EXPECT_FALSE(AcScanName(m4, 100, 12345).Allowed());
    /* suffix anchor: super-domains and label look-alikes miss */
    MockSkb m5 = AcMockFor("example.com.evil.net");
    EXPECT_FALSE(AcScanName(m5, 100, 12345).Allowed());
    MockSkb m6 = AcMockFor("notexample.com");
    EXPECT_FALSE(AcScanName(m6, 100, 12345).Allowed());

    /* the no-star spelling still covers bare + subdomains (parity anchor) */
    ResetDnsStubs();
    AcDomainTableBuilder builder2 = AcMakeBuilder({{100, 0, true, "example.com"}});
    ASSERT_TRUE(AcInstallTables(builder2, 0));
    MockSkb m7 = AcMockFor("example.com");
    EXPECT_TRUE(AcScanName(m7, 100, 12345).Allowed());
    MockSkb m8 = AcMockFor("a.example.com");
    EXPECT_TRUE(AcScanName(m8, 100, 12345).Allowed());

    /* DENY anchor is isomorphic */
    ResetDnsStubs();
    AcDomainTableBuilder builder3 = AcMakeBuilder({{100, 0, false, "*.example.com"}});
    ASSERT_TRUE(AcInstallTables(builder3, 0));
    MockSkb m9 = AcMockFor("a.example.com");
    EXPECT_TRUE(AcScanName(m9, 100, 12345).Denied());
    MockSkb m10 = AcMockFor("example.com");
    EXPECT_FALSE(AcScanName(m10, 100, 12345).Denied());

    /* anchored and non-anchored rules share the trie without cross-talk:
     * the bare rule completes at its last LEN, the anchored rule needs the
     * MORE tail (one further name class) — different completion edges */
    ResetDnsStubs();
    AcDomainTableBuilder builder4 = AcMakeBuilder({{100, 0, true, "example.com"}, {100, 0, false, "*.example.com"}});
    ASSERT_TRUE(AcInstallTables(builder4, 0));
    MockSkb m11 = AcMockFor("example.com");
    AcQuadVerdict v11 = AcScanName(m11, 100, 12345);
    EXPECT_TRUE(v11.Allowed());
    EXPECT_FALSE(v11.Denied()); /* anchored deny needs a further label: no hit */
    EXPECT_EQ(v11.suffixWild, AC_FLAG_SUFFIX_ALLOW);
    MockSkb m12 = AcMockFor("a.example.com");
    AcQuadVerdict v12 = AcScanName(m12, 100, 12345);
    EXPECT_TRUE(v12.Allowed());
    EXPECT_TRUE(v12.Denied()); /* both complete; deny wins at adjudication */
    EXPECT_EQ(v12.suffixWild, AC_FLAG_SUFFIX_ALLOW | AC_FLAG_SUFFIX_DENY);
    MockSkb m13 = AcMockFor("b.example.org");
    EXPECT_FALSE(AcScanName(m13, 100, 12345).Allowed());
    EXPECT_FALSE(AcScanName(m13, 100, 12345).Denied());
}

HWTEST_F(NetsysBpfNetFirewallTest, CtHookReturnPathInherit001, TestSize.Level0)
{
    /* F2: return-path inheritance on non-loopback ct entries. Matrix from
     * the on-device TC-E regression — "inbound default deny + outbound
     * allow": pre-P4 exempted every non-NEW ct (status != CT_NEW → PASS);
     * P4 re-adjudicated the SYN-ACK against the inbound default action and
     * killed each allowed handshake (DROP + TCP reset). The fix: a
     * non-loopback ct whose OPPOSITE direction passed the full evaluation
     * passes the return traffic too; loopback stays per-direction (P4
     * cross-space isolation). */
    constexpr uint32_t LOCAL_IP = 0x0A000002;  /* 10.0.0.2 */
    constexpr uint32_t REMOTE_IP = 0xB465312C; /* 180.101.49.44 */
    constexpr uint16_t LOCAL_PORT = 12345;

    /* --- 1+2: the fix's main assertion — egress SYN passes, return
     * SYN-ACK inherits and passes without a reset --- */
    ResetDnsStubs();
    g_stubSocketUid = 20020152;               /* -> userId 100 */
    g_stubDefaultAction = {SK_DROP, SK_PASS}; /* inaction deny + outaction allow */
    g_stubDevNameOk = true;                   /* full evaluations take the real rule path (no rules -> default) */

    MockSkb syn;
    syn.Reset(AF_INET);
    syn.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, LOCAL_IP, REMOTE_IP));
    syn.Append(TcpHeader(LOCAL_PORT, 80, 0x02));
    EXPECT_EQ(fw_policy_after_egress(AF_INET, syn.Done(), NULL), SK_PASS);
    ASSERT_EQ(g_stubCtMap.size(), 1U);
    struct ct_entry created = g_stubCtMap.begin()->second;
    EXPECT_EQ(created.egress_evaluated, 1);
    EXPECT_EQ(created.ingress_evaluated, 0);

    int resetsBefore = g_stubResetSent;
    MockSkb synack;
    synack.Reset(AF_INET);
    synack.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, REMOTE_IP, LOCAL_IP));
    synack.Append(TcpHeader(80, LOCAL_PORT, 0x12));
    EXPECT_EQ(fw_policy_after_ingress(AF_INET, synack.Done()), SK_PASS);
    EXPECT_EQ(g_stubResetSent, resetsBefore); /* inheritance: no reset */
    /* the return packet never marks its own direction (it was not fully
     * evaluated here) — every return packet re-checks the opposite bit */
    EXPECT_EQ(g_stubCtMap.begin()->second.ingress_evaluated, 0);
    EXPECT_EQ(g_stubCtMap.begin()->second.egress_evaluated, 1);

    /* --- 3: control — same ct but the opposite bit NOT set: full
     * re-adjudication against the inbound default -> DROP + reset --- */
    ResetDnsStubs();
    g_stubSocketUid = 20020152;
    g_stubDefaultAction = {SK_DROP, SK_PASS};
    g_stubDevNameOk = true;
    MockSkb syn2;
    syn2.Reset(AF_INET);
    syn2.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, LOCAL_IP, REMOTE_IP));
    syn2.Append(TcpHeader(LOCAL_PORT, 80, 0x02));
    EXPECT_EQ(fw_policy_after_egress(AF_INET, syn2.Done(), NULL), SK_PASS);
    ASSERT_EQ(g_stubCtMap.size(), 1U);
    g_stubCtMap.begin()->second.egress_evaluated = 0; /* sabotage the opposite bit */

    resetsBefore = g_stubResetSent;
    MockSkb synack2;
    synack2.Reset(AF_INET);
    synack2.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, REMOTE_IP, LOCAL_IP));
    synack2.Append(TcpHeader(80, LOCAL_PORT, 0x12));
    EXPECT_EQ(fw_policy_after_ingress(AF_INET, synack2.Done()), SK_DROP);
    EXPECT_EQ(g_stubResetSent, resetsBefore + 1); /* intercepted: reset sent */

    /* --- 4: control — no ct at all (nothing initiated): pure inbound
     * falls to the inbound default deny --- */
    ResetDnsStubs();
    g_stubSocketUid = 20020152;
    g_stubDefaultAction = {SK_DROP, SK_PASS};
    g_stubDevNameOk = true;
    MockSkb synack3;
    synack3.Reset(AF_INET);
    synack3.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, REMOTE_IP, LOCAL_IP));
    synack3.Append(TcpHeader(80, LOCAL_PORT, 0x12));
    EXPECT_EQ(fw_policy_after_ingress(AF_INET, synack3.Done()), SK_DROP);
    EXPECT_EQ(g_stubCtMap.size(), 0U); /* DROP never creates ct */

    /* --- 5: loopback — egress-created ct does NOT inherit to ingress
     * (P4 cross-space isolation preserved); the receiver's inaction is
     * adjudicated on the first ingress packet --- */
    ResetDnsStubs();
    g_stubSocketUid = 20020152;
    g_stubDefaultAction = {SK_DROP, SK_PASS};
    g_stubDevNameOk = true;
    g_stubLoopbackPresent = 1;
    constexpr uint32_t LO_A = 0x7F000001; /* 127.0.0.1 */
    constexpr uint32_t LO_B = 0x7F000002; /* 127.0.0.2 */

    MockSkb loSyn;
    loSyn.Reset(AF_INET);
    loSyn.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, LO_A, LO_B));
    loSyn.Append(TcpHeader(LOCAL_PORT, 80, 0x02));
    EXPECT_EQ(fw_policy_after_egress(AF_INET, loSyn.Done(), NULL), SK_PASS);
    ASSERT_EQ(g_stubCtMap.size(), 1U);
    EXPECT_EQ(g_stubCtMap.begin()->second.egress_evaluated, 1);

    resetsBefore = g_stubResetSent;
    MockSkb loSynack;
    loSynack.Reset(AF_INET);
    loSynack.Append(Ipv4HeaderAddr(IPPROTO_TCP, 40, LO_B, LO_A));
    loSynack.Append(TcpHeader(80, LOCAL_PORT, 0x12));
    EXPECT_EQ(fw_policy_after_ingress(AF_INET, loSynack.Done()), SK_DROP);
    EXPECT_EQ(g_stubResetSent, resetsBefore + 1); /* loopback: no inheritance, inbound deny fires */
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanSuffixExact001, TestSize.Level0)
{
    ResetDnsStubs();
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));

    /* suffix rule = the domain and every subdomain */
    MockSkb m1 = AcMockFor("example.com");
    EXPECT_TRUE(AcScanName(m1, 100, 12345).Allowed());
    MockSkb m2 = AcMockFor("www.example.com");
    EXPECT_TRUE(AcScanName(m2, 100, 12345).Allowed());
    MockSkb m3 = AcMockFor("a.b.example.com");
    EXPECT_TRUE(AcScanName(m3, 100, 12345).Allowed());

    /* label alignment: "badexample.com" must NOT hit (legacy LPM did) */
    MockSkb m4 = AcMockFor("badexample.com");
    AcQuadVerdict v4 = AcScanName(m4, 100, 12345);
    EXPECT_FALSE(v4.Allowed());
    EXPECT_FALSE(v4.Denied());
    /* a longer first label breaks the pattern's first-label LEN anchor */
    MockSkb m5 = AcMockFor("notexample.com");
    EXPECT_FALSE(AcScanName(m5, 100, 12345).Allowed());
    /* the suffix anchor is the name END: a super-domain does not match */
    MockSkb m6 = AcMockFor("example.com.evil.net");
    EXPECT_FALSE(AcScanName(m6, 100, 12345).Allowed());
    /* wire-case folding */
    MockSkb m7 = AcMockFor("EXAMPLE.com");
    EXPECT_TRUE(AcScanName(m7, 100, 12345).Allowed());
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanSuffixOverlap001, TestSize.Level0)
{
    ResetDnsStubs();
    /* overlapping suffix rules share the trie path: both must latch
     * through the shared rows (the "com"+"example.com" double hit). */
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, false, "com"}, {100, 0, true, "example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));

    MockSkb m = AcMockFor("www.example.com");
    AcQuadVerdict v = AcScanName(m, 100, 12345);
    EXPECT_TRUE(v.Allowed());
    EXPECT_TRUE(v.Denied()); /* deny wins at the BPF adjudication */
    EXPECT_EQ(v.suffixWild, AC_FLAG_SUFFIX_ALLOW | AC_FLAG_SUFFIX_DENY);

    MockSkb m2 = AcMockFor("a.example.org");
    AcQuadVerdict v2 = AcScanName(m2, 100, 12345);
    EXPECT_FALSE(v2.Allowed());
    EXPECT_FALSE(v2.Denied());
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanPrefixWild001, TestSize.Level0)
{
    ResetDnsStubs();
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "www.example*"}, {100, 0, false, "ads*"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));

    /* Trailing-star allow rules are LABEL-CONFINED (the '*' completes the
     * last label): "www.example*" completes inside the SECOND label
     * ("example" prefixes it) and the "com" boundary drops the
     * provisional verdict. The legacy label-exact form also matched
     * cross-label continuations — that spelling is "www.example.*"
     * below; the plain cross-label allow is gone ON PURPOSE (an allow
     * must not leak past the last label: www.example.evil.com is
     * attacker-controlled). */
    MockSkb m1 = AcMockFor("www.example.com");
    EXPECT_FALSE(AcScanName(m1, 100, 12345).Allowed());
    /* the pattern completes before the terminator: a name ending at the
     * pattern still matches */
    MockSkb m2 = AcMockFor("www.example");
    EXPECT_TRUE(AcScanName(m2, 100, 12345).Allowed());
    /* the fragment may complete mid-label (glob '*'): "examples" starts
     * with "example" and is the name's last label */
    MockSkb m2b = AcMockFor("www.examples");
    EXPECT_TRUE(AcScanName(m2b, 100, 12345).Allowed());
    /* anchored at the name start */
    MockSkb m3 = AcMockFor("a.www.example.com");
    EXPECT_FALSE(AcScanName(m3, 100, 12345).Allowed());
    /* first-label length mismatch */
    MockSkb m4 = AcMockFor("wwww.example.com");
    EXPECT_FALSE(AcScanName(m4, 100, 12345).Allowed());
    MockSkb m5 = AcMockFor("www3.example.com");
    EXPECT_FALSE(AcScanName(m5, 100, 12345).Allowed());
    /* label alignment of the full labels: "examplez" is fine as a
     * last-label completion (m2b), but as a NON-last label the boundary
     * after it drops the verdict */
    MockSkb m6 = AcMockFor("www.examplez.com");
    EXPECT_FALSE(AcScanName(m6, 100, 12345).Allowed());
    /* deny prefix rule: right-unconstrained on purpose (fail-closed) */
    MockSkb m7 = AcMockFor("ads.example.com");
    EXPECT_TRUE(AcScanName(m7, 100, 12345).Denied());

    /* the whole-final-label spelling carries the legacy intent: exactly
     * one more label after www.example, any content, allow-confined. */
    ResetDnsStubs();
    AcDomainTableBuilder builder2 = AcMakeBuilder({{100, 0, true, "www.example.*"}});
    ASSERT_TRUE(AcInstallTables(builder2, 0));
    MockSkb m8 = AcMockFor("www.example.com");
    EXPECT_TRUE(AcScanName(m8, 100, 12345).Allowed());
    MockSkb m9 = AcMockFor("www.example.xyz");
    EXPECT_TRUE(AcScanName(m9, 100, 12345).Allowed());
    MockSkb m10 = AcMockFor("www.example.evil.com");
    EXPECT_FALSE(AcScanName(m10, 100, 12345).Allowed());
    MockSkb m11 = AcMockFor("www.example");
    EXPECT_FALSE(AcScanName(m11, 100, 12345).Allowed()); /* SKIP never eats TERM */
}

/* SKIP-edge collision: the deny "ads*" SKIP expansion and the allow
 * "www.example*" concrete first-label edge share the (root, LEN(3)) key —
 * before the Insert-side absorption the single-writer map kept only the
 * last row and one of the two rules silently vanished (measured: the
 * allow lost its very first transition, m2/m2b above). The trie now
 * holds the union of both continuations under the SKIP target, in
 * either insertion order. */
HWTEST_F(NetsysBpfNetFirewallTest, AcScanSkipEdgeCollision001, TestSize.Level0)
{
    /* deny first: the allow's concrete label-length class routes through
     * the SKIP target that already exists at the root */
    ResetDnsStubs();
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, false, "ads*"}, {100, 0, true, "www.example*"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));
    MockSkb m1 = AcMockFor("www.example");
    EXPECT_TRUE(AcScanName(m1, 100, 12345).Allowed());
    MockSkb m1b = AcMockFor("www.examples");
    EXPECT_TRUE(AcScanName(m1b, 100, 12345).Allowed());
    MockSkb m2 = AcMockFor("www.example.com");
    EXPECT_FALSE(AcScanName(m2, 100, 12345).Allowed()); /* label-confined allow */
    MockSkb m3 = AcMockFor("ads.example.com");
    EXPECT_TRUE(AcScanName(m3, 100, 12345).Denied());
    MockSkb m4 = AcMockFor("ads.example");
    EXPECT_TRUE(AcScanName(m4, 100, 12345).Denied());
    MockSkb m5 = AcMockFor("adsx");
    EXPECT_TRUE(AcScanName(m5, 100, 12345).Denied());

    /* allow first: the SKIP edge absorbs the concrete label-length
     * edge's subtrie at creation */
    ResetDnsStubs();
    AcDomainTableBuilder builder2 = AcMakeBuilder({{100, 0, true, "www.example*"}, {100, 0, false, "ads*"}});
    ASSERT_TRUE(AcInstallTables(builder2, 0));
    MockSkb m6 = AcMockFor("www.example");
    EXPECT_TRUE(AcScanName(m6, 100, 12345).Allowed());
    MockSkb m6b = AcMockFor("www.examples");
    EXPECT_TRUE(AcScanName(m6b, 100, 12345).Allowed());
    MockSkb m7 = AcMockFor("ads.example.com");
    EXPECT_TRUE(AcScanName(m7, 100, 12345).Denied());
}

/* MORE-edge collision: "*.example.com" (MORE tail) and "*w.example.com"
 * (leading-fragment CHAR tail) share the post-label node — the MORE
 * expansion used to emit a CHAR('w') row that clobbered the fragment
 * rule's concrete row (same map key, last writer wins). The concrete row
 * now wins the class and the anchor's completion flags ride along. */
HWTEST_F(NetsysBpfNetFirewallTest, AcScanMoreEdgeCollision001, TestSize.Level0)
{
    ResetDnsStubs();
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "*.example.com"}, {100, 0, true, "*w.example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));
    MockSkb m1 = AcMockFor("a.example.com");
    EXPECT_TRUE(AcScanName(m1, 100, 12345).Allowed());
    MockSkb m2 = AcMockFor("w.example.com");
    EXPECT_TRUE(AcScanName(m2, 100, 12345).Allowed());
    MockSkb m3 = AcMockFor("xw.example.com");
    EXPECT_TRUE(AcScanName(m3, 100, 12345).Allowed());
    MockSkb m4 = AcMockFor("zw.example.com");
    EXPECT_TRUE(AcScanName(m4, 100, 12345).Allowed());
    MockSkb m5 = AcMockFor("example.com");
    EXPECT_FALSE(AcScanName(m5, 100, 12345).Allowed()); /* anchor: subdomains only */
    MockSkb m6 = AcMockFor("aw.example.org");
    EXPECT_FALSE(AcScanName(m6, 100, 12345).Allowed()); /* suffix mismatch */
    MockSkb m7 = AcMockFor("ax.example.org");
    EXPECT_FALSE(AcScanName(m7, 100, 12345).Allowed()); /* label does not end with the fragment */
}

HWTEST_F(NetsysBpfNetFirewallTest, AcPatternTrailingStarEncode001, TestSize.Level0)
{
    /* Remaining trailing-star encodings: the whole-final-label form
     * ("x.*" — SKIP right after the full labels) and the single-label
     * form ("f*" — SKIP first, then the fragment). */
    std::vector<uint32_t> cls;
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("www.example.*", true, cls));
    EXPECT_EQ(cls,
              (std::vector<uint32_t>{AC_CLS_LEN(3), AC_CLS_CHAR('w'), AC_CLS_CHAR('w'), AC_CLS_CHAR('w'), AC_CLS_LEN(7),
                                     AC_CLS_CHAR('e'), AC_CLS_CHAR('x'), AC_CLS_CHAR('a'), AC_CLS_CHAR('m'),
                                     AC_CLS_CHAR('p'), AC_CLS_CHAR('l'), AC_CLS_CHAR('e'), AC_BUILDER_CLS_SKIP}));
    EXPECT_EQ(AcDecodePattern(cls, false), "www.example.*");

    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("baidu*", true, cls));
    EXPECT_EQ(cls, (std::vector<uint32_t>{AC_BUILDER_CLS_SKIP, AC_CLS_CHAR('b'), AC_CLS_CHAR('a'), AC_CLS_CHAR('i'),
                                          AC_CLS_CHAR('d'), AC_CLS_CHAR('u')}));
    EXPECT_EQ(AcDecodePattern(cls, false), "baidu*");

    /* mixed case folds identically */
    std::vector<uint32_t> folded;
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("WWW.Example.*", true, folded));
    cls.clear();
    ASSERT_TRUE(AcDomainTableBuilder::BuildPattern("www.example.*", true, cls));
    EXPECT_EQ(cls, folded);

    /* Emit expansion accounting: one "www.example.c*" rule in a fresh
     * namespace = 12 concrete class edges + 63 SKIP-expanded LEN rows +
     * 1 fragment char = 76 prefix rows, exactly one completion row and
     * it carries the label-confined allow bit. */
    AcDomainTableBuilder single = AcMakeBuilder({{100, 0, true, "www.example.c*"}});
    std::vector<AcTransEntry> prefix;
    std::vector<AcTransEntry> suffix;
    ASSERT_TRUE(single.Emit(0, prefix, suffix));
    EXPECT_EQ(prefix.size(), 76U);
    EXPECT_EQ(suffix.size(), 0U);
    size_t completions = 0;
    for (const auto &e : prefix) {
        if (e.val.flags != 0) {
            ++completions;
            EXPECT_EQ(e.val.flags, AC_FLAG_PREFIX_ALLOW_LC);
            EXPECT_EQ(e.key.cls, AC_CLS_CHAR('c'));
        }
        /* the SKIP pseudo-class never reaches the emitted rows */
        EXPECT_NE(e.key.cls, AC_BUILDER_CLS_SKIP);
        EXPECT_TRUE(e.key.cls == AC_CLS_TERM || e.key.cls <= DNS_LABEL_MAX_LEN || (e.key.cls & 0x100U));
    }
    EXPECT_EQ(completions, 1U);

    /* the deny spelling takes the plain PREFIX_DENY bit (right-
     * unconstrained on purpose) */
    AcDomainTableBuilder denyBuilder = AcMakeBuilder({{100, 0, false, "www.example.c*"}});
    std::vector<AcTransEntry> p2;
    std::vector<AcTransEntry> s2;
    ASSERT_TRUE(denyBuilder.Emit(0, p2, s2));
    size_t denyCompletions = 0;
    for (const auto &e : p2) {
        if (e.val.flags != 0) {
            ++denyCompletions;
            EXPECT_EQ(e.val.flags, AC_FLAG_PREFIX_DENY);
        }
    }
    EXPECT_EQ(denyCompletions, 1U);
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanTrailingStarSemantics001, TestSize.Level0)
{
    /* C semantics at the scan level: an ALLOW trailing-star rule is
     * confined to the LAST label (the provisional verdict drops at any
     * later label boundary — "www.example.c*" opens www.example.com but
     * never www.example.c.evil.com), a DENY rule is right-unconstrained
     * (fail-closed: it keeps blocking everything to the right). */
    ResetDnsStubs();
    AcDomainTableBuilder allowBuilder = AcMakeBuilder({{100, 0, true, "www.example.c*"}});
    ASSERT_TRUE(AcInstallTables(allowBuilder, 0));
    EXPECT_TRUE(AcScanQuery("www.example.com", 100, 12345).Allowed());
    EXPECT_TRUE(AcScanQuery("www.example.cn", 100, 12345).Allowed());
    EXPECT_TRUE(AcScanQuery("www.example.co", 100, 12345).Allowed());
    EXPECT_TRUE(AcScanQuery("www.example.c", 100, 12345).Allowed());           /* fragment = whole label */
    EXPECT_FALSE(AcScanQuery("www.example.c.evil.com", 100, 12345).Allowed()); /* boundary drop */
    EXPECT_FALSE(AcScanQuery("www.example.evil.com", 100, 12345).Allowed());   /* 'e' != 'c' */
    EXPECT_FALSE(AcScanQuery("www.examplez.com", 100, 12345).Allowed());       /* 'e' != 'c' */
    EXPECT_FALSE(AcScanQuery("image.example.com", 100, 12345).Allowed());      /* anchor at start */

    ResetDnsStubs();
    AcDomainTableBuilder denyBuilder = AcMakeBuilder({{100, 0, false, "www.example.c*"}});
    ASSERT_TRUE(AcInstallTables(denyBuilder, 0));
    EXPECT_TRUE(AcScanQuery("www.example.com", 100, 12345).Denied());
    EXPECT_TRUE(AcScanQuery("www.example.c.evil.com", 100, 12345).Denied()); /* deny survives */
    EXPECT_FALSE(AcScanQuery("www.example.evil.com", 100, 12345).Denied());  /* 'e' != 'c' */

    /* the deny broadening is a label-PREFIX: "adsense" starts with
     * "ads" (blocked), "xads" does not (the SKIP consumed its LEN, the
     * fragment then misses at the first char) */
    ResetDnsStubs();
    AcDomainTableBuilder adsBuilder = AcMakeBuilder({{100, 0, false, "ads*"}});
    ASSERT_TRUE(AcInstallTables(adsBuilder, 0));
    EXPECT_TRUE(AcScanQuery("ads.example.com", 100, 12345).Denied());
    EXPECT_TRUE(AcScanQuery("ads", 100, 12345).Denied());
    EXPECT_TRUE(AcScanQuery("adsense.example.com", 100, 12345).Denied());
    EXPECT_FALSE(AcScanQuery("xads.example.com", 100, 12345).Denied());

    /* the allow side of the same rule stays confined */
    ResetDnsStubs();
    AcDomainTableBuilder adsAllow = AcMakeBuilder({{100, 0, true, "ads*"}});
    ASSERT_TRUE(AcInstallTables(adsAllow, 0));
    EXPECT_TRUE(AcScanQuery("ads", 100, 12345).Allowed());
    EXPECT_FALSE(AcScanQuery("ads.example.com", 100, 12345).Allowed());
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanLeadingFragmentSuffix001, TestSize.Level0)
{
    /* "*w.example.com" (suffixed truncated pattern): the label right
     * before "example.com" ends with "w", everything further left is the
     * glob '*' — the name itself matches, deeper left labels match (still
     * inside example.com), and the label-aligned body keeps look-alike
     * registrable domains out. */
    ResetDnsStubs();
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "*w.example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));
    EXPECT_TRUE(AcScanQuery("www.example.com", 100, 12345).Allowed());
    EXPECT_TRUE(AcScanQuery("w.example.com", 100, 12345).Allowed());    /* name itself */
    EXPECT_TRUE(AcScanQuery("a.bw.example.com", 100, 12345).Allowed()); /* glob left */
    EXPECT_TRUE(AcScanQuery("foow.example.com", 100, 12345).Allowed());
    EXPECT_TRUE(AcScanQuery("xw.example.com", 100, 12345).Allowed());
    EXPECT_FALSE(AcScanQuery("image.example.com", 100, 12345).Allowed()); /* 'e' != 'w' */
    EXPECT_FALSE(AcScanQuery("music.example.com", 100, 12345).Allowed()); /* 'c' != 'w' (假锚) */
    EXPECT_FALSE(AcScanQuery("example.com", 100, 12345).Allowed());       /* no label before example */
    EXPECT_FALSE(AcScanQuery("www.example.org", 100, 12345).Allowed());   /* body must align */
    /* look-alike single label: the body demands the label before "com"
     * be exactly "example" (LEN class), "xwexample" never fits */
    EXPECT_FALSE(AcScanQuery("xwexample.com", 100, 12345).Allowed());
    EXPECT_FALSE(AcScanQuery("www.example.com.evil.com", 100, 12345).Allowed()); /* TERM anchor */

    /* deny variant: identical matching surface, SUFFIX_DENY bit */
    ResetDnsStubs();
    AcDomainTableBuilder denyBuilder = AcMakeBuilder({{100, 0, false, "*w.example.com"}});
    ASSERT_TRUE(AcInstallTables(denyBuilder, 0));
    EXPECT_TRUE(AcScanQuery("www.example.com", 100, 12345).Denied());
    EXPECT_FALSE(AcScanQuery("image.example.com", 100, 12345).Denied());
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanCaseMixed001, TestSize.Level0)
{
    ResetDnsStubs();
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "Example.COM"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));

    for (const auto &name : std::vector<std::string>{"example.com", "EXAMPLE.COM", "ExAmPlE.cOm"}) {
        MockSkb m = AcMockFor(name);
        EXPECT_TRUE(AcScanName(m, 100, 42).Allowed()) << "name: " << name;
    }
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanNamespaceIsolation001, TestSize.Level0)
{
    ResetDnsStubs();
    /* exact (100,12345) allow, wild (100,0) deny, another user's wild
     * allow — three independent tries in one table set. */
    AcDomainTableBuilder builder =
        AcMakeBuilder({{100, 12345, true, "example.com"}, {100, 0, false, "evil.com"}, {101, 0, true, "example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));

    MockSkb allow = AcMockFor("example.com");
    MockSkb deny = AcMockFor("evil.com");
    /* the attributed app hits its exact table */
    EXPECT_TRUE(AcScanName(allow, 100, 12345).Allowed());
    EXPECT_TRUE(AcScanName(deny, 100, 12345).Denied());
    /* a different app of the same user: no exact table, only the wild
     * namespace applies */
    AcQuadVerdict other = AcScanName(allow, 100, 99999);
    EXPECT_FALSE(other.Allowed());
    EXPECT_TRUE(AcScanName(deny, 100, 99999).Denied());
    /* another user's namespace is fully isolated */
    EXPECT_TRUE(AcScanName(allow, 101, 777).Allowed());
    EXPECT_FALSE(AcScanName(deny, 101, 777).Denied());
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanCombinedQuadrant001, TestSize.Level0)
{
    ResetDnsStubs();
    /* prefix deny + suffix allow on one name: the four scans OR into a
     * combined verdict with both bits — the BPF adjudication (deny
     * priority for default-allow users) then drops. */
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, false, "ads*"}, {100, 0, true, "example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));

    MockSkb m = AcMockFor("ads.example.com");
    AcQuadVerdict v = AcScanName(m, 100, 12345);
    EXPECT_EQ(v.prefixWild, AC_FLAG_PREFIX_DENY);
    EXPECT_EQ(v.suffixWild, AC_FLAG_SUFFIX_ALLOW);
    EXPECT_TRUE(v.Denied());
    EXPECT_TRUE(v.Allowed());

    MockSkb m2 = AcMockFor("www.example.com");
    AcQuadVerdict v2 = AcScanName(m2, 100, 12345);
    EXPECT_EQ(v2.prefixWild, 0U);
    EXPECT_EQ(v2.suffixWild, AC_FLAG_SUFFIX_ALLOW);
    EXPECT_TRUE(v2.Allowed());
    EXPECT_FALSE(v2.Denied());
}

HWTEST_F(NetsysBpfNetFirewallTest, AcScanFailClosed001, TestSize.Level0)
{
    ResetDnsStubs();
    /* no generation entry (fresh boot / map-less): every scan fails —
     * no verdict, the packet keeps flowing through the policy chain */
    MockSkb m = AcMockFor("www.example.com");
    AcQuadVerdict v = AcScanName(m, 100, 12345);
    EXPECT_EQ(v.prefixExact, AC_PARSE_FAIL);
    EXPECT_EQ(v.prefixWild, AC_PARSE_FAIL);
    EXPECT_EQ(v.suffixExact, AC_PARSE_FAIL);
    EXPECT_EQ(v.suffixWild, AC_PARSE_FAIL);

    /* empty tables with a present generation: clean misses, 0 flags */
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "example.com"}});
    ASSERT_TRUE(AcInstallTables(builder, 0));
    g_stubAcPrefixMap.clear();
    g_stubAcSuffixMap.clear();
    AcQuadVerdict v2 = AcScanName(m, 100, 12345);
    EXPECT_EQ(v2.Combined(), 0U);
    EXPECT_FALSE(v2.Allowed());
    EXPECT_FALSE(v2.Denied());

    /* label length byte > 63: the forward scan aborts the parse */
    MockSkb m3 = AcMockFor(std::string(64, 'a') + ".com");
    EXPECT_EQ(ac_prefix_scan(m3.Done(), 0, &g_stubScratch[0]), AC_PARSE_FAIL);
    /* no terminator inside the 128-position window: parse failure */
    MockSkb m4 = AcMockFor(std::string(63, 'a') + "." + std::string(63, 'b'), false);
    EXPECT_EQ(ac_prefix_scan(m4.Done(), 0, &g_stubScratch[0]), AC_PARSE_FAIL);
}

HWTEST_F(NetsysBpfNetFirewallTest, AcGenerationFlip001, TestSize.Level0)
{
    ResetDnsStubs();
    /* The flip protocol on the stub surface: write the new generation
     * (both coexist, the key gen disambiguates), the old generation keeps
     * answering until the single flip, the retired rows are swept after.
     * In-flight scans read the generation once and stay consistent. */
    AcDomainTableBuilder gen0 = AcMakeBuilder({{100, 0, false, "example.com"}});
    ASSERT_TRUE(AcInstallTables(gen0, 0));

    MockSkb m = AcMockFor("www.example.com");
    EXPECT_TRUE(AcScanName(m, 100, 12345).Denied());

    /* build generation 1 (rule flipped to allow) and write it WITHOUT
     * flipping — generation 0 must keep answering unchanged */
    AcDomainTableBuilder gen1 = AcMakeBuilder({{100, 0, true, "example.com"}});
    std::vector<AcTransEntry> prefix1;
    std::vector<AcTransEntry> suffix1;
    ASSERT_TRUE(gen1.Emit(1, prefix1, suffix1));
    for (const auto &e : prefix1) {
        g_stubAcPrefixMap[e.key] = e.val;
    }
    for (const auto &e : suffix1) {
        g_stubAcSuffixMap[e.key] = e.val;
    }
    EXPECT_EQ(g_stubAcGen, 0U);
    EXPECT_TRUE(AcScanName(m, 100, 12345).Denied()); /* still the old table */

    /* the flip: one write, every later scan uses the new generation */
    g_stubAcGen = 1;
    EXPECT_TRUE(AcScanName(m, 100, 12345).Allowed());
    EXPECT_FALSE(AcScanName(m, 100, 12345).Denied());

    /* retired-generation sweep: rows of gen 0 go, gen 1 keeps answering */
    for (auto it = g_stubAcPrefixMap.begin(); it != g_stubAcPrefixMap.end();) {
        it = (it->first.gen == 0) ? g_stubAcPrefixMap.erase(it) : std::next(it);
    }
    for (auto it = g_stubAcSuffixMap.begin(); it != g_stubAcSuffixMap.end();) {
        it = (it->first.gen == 0) ? g_stubAcSuffixMap.erase(it) : std::next(it);
    }
    EXPECT_TRUE(AcScanName(m, 100, 12345).Allowed());
}

HWTEST_F(NetsysBpfNetFirewallTest, AcBuildCapacityFailFast001, TestSize.Level0)
{
    /* A rule set whose suffix table exceeds AC_TRANS_MAX_ENTRIES must be
     * rejected by Emit BEFORE any map write (fail-fast, no silent
     * truncation). The label generator is a stateful LCG advanced across
     * positions: a stateless i·M + j·K mix only shifts the alphabet by a
     * constant per rule (period 26 in i — measured: 2200 rules collapsed
     * to 26 distinct patterns and the trie stayed at ~5k transitions),
     * so it never reaches the cap. */
    auto lcgLabel = [](uint32_t seed, size_t len) {
        std::string s;
        uint32_t state = seed * 2654435761U + 1U;
        for (size_t j = 0; j < len; ++j) {
            state = state * 1103515245U + 12345U;
            s += static_cast<char>('a' + ((state >> 16) % 26U));
        }
        return s;
    };
    AcDomainTableBuilder big;
    for (uint32_t i = 0; i < 2200; ++i) {
        /* text length 125 (62+1+62): suffix pattern = 127 classes */
        ASSERT_TRUE(big.AddRule(100, 0, false, lcgLabel(i, 62) + "." + lcgLabel(i + 1, 62)));
    }
    std::vector<AcTransEntry> prefix;
    std::vector<AcTransEntry> suffix;
    EXPECT_FALSE(big.Emit(0, prefix, suffix));
    EXPECT_GT(suffix.size(), static_cast<size_t>(AC_TRANS_MAX_ENTRIES));

    /* a normal-size set stays well under the cap */
    AcDomainTableBuilder small;
    for (uint32_t i = 0; i < 100; ++i) {
        ASSERT_TRUE(small.AddRule(100, 0, true, lcgLabel(i, 10) + "." + lcgLabel(i + 1, 10)));
    }
    std::vector<AcTransEntry> p2;
    std::vector<AcTransEntry> s2;
    EXPECT_TRUE(small.Emit(0, p2, s2));
    EXPECT_LE(s2.size(), static_cast<size_t>(AC_TRANS_MAX_ENTRIES));
}

HWTEST_F(NetsysBpfNetFirewallTest, AcEntryInvariants001, TestSize.Level0)
{
    ResetDnsStubs();
    /* duplicate rule (no extra rows), conflicting allow/deny on one domain
     * (both bits on the shared completion row), two namespaces, both rule
     * families. */
    AcDomainTableBuilder builder = AcMakeBuilder({{100, 0, true, "example.com"},
                                                  {100, 0, true, "example.com"},  /* exact duplicate */
                                                  {100, 0, false, "example.com"}, /* conflicting action */
                                                  {100, 0, true, "www.example*"},
                                                  {100, 12345, false, "com"},
                                                  {101, 0, true, "a.b.c"}});
    std::vector<AcTransEntry> prefix;
    std::vector<AcTransEntry> suffix;
    ASSERT_TRUE(builder.Emit(7, prefix, suffix));
    ASSERT_FALSE(prefix.empty());
    ASSERT_FALSE(suffix.empty());

    size_t prefixCompletions = 0;
    size_t suffixCompletions = 0;
    for (const auto &e : prefix) {
        EXPECT_EQ(e.val.rem, 0U); /* the scans never consume rem */
        EXPECT_EQ(e.key.gen, 7U);
        EXPECT_NE(e.key.state, AC_PARSE_FAIL); /* the dead marker is never a row */
        EXPECT_NE(e.val.next, AC_PARSE_FAIL);
        EXPECT_LT(e.val.next, prefix.size() + suffix.size());
        EXPECT_TRUE(e.key.cls == AC_CLS_TERM || e.key.cls <= DNS_LABEL_MAX_LEN || (e.key.cls & 0x100U))
            << "cls " << e.key.cls;
        if (e.val.flags != 0) {
            ++prefixCompletions;
            /* only prefix bits in the prefix table — the label-confined
             * allow bit included (trailing-star allow rules, e.g. the
             * "www.example*" row below) */
            EXPECT_EQ(e.val.flags & ~(AC_PREFIX_MASK | AC_FLAG_PREFIX_ALLOW_LC), 0U);
        }
    }
    for (const auto &e : suffix) {
        EXPECT_EQ(e.val.rem, 0U);
        EXPECT_EQ(e.key.gen, 7U);
        EXPECT_NE(e.key.state, AC_PARSE_FAIL);
        EXPECT_NE(e.val.next, AC_PARSE_FAIL);
        if (e.val.flags != 0) {
            ++suffixCompletions;
            EXPECT_EQ(e.val.flags & ~AC_SUFFIX_MASK, 0U);
        }
    }
    /* one completion row per distinct pattern per table: prefix has
     * "www.example*" (uid 100), suffix has "example.com" (both bits) and
     * "com" (uid 100 appuid 12345) and "a.b.c" (uid 101) — the duplicate
     * added nothing. */
    EXPECT_EQ(prefixCompletions, 1U);
    EXPECT_EQ(suffixCompletions, 3U);
    /* the conflicting domain carries both verdict bits on one row. */
    bool bothBits = false;
    for (const auto &e : suffix) {
        if (e.key.uid == 100 && e.key.appuid == 0 && e.val.flags == (AC_FLAG_SUFFIX_ALLOW | AC_FLAG_SUFFIX_DENY)) {
            bothBits = true;
        }
    }
    EXPECT_TRUE(bothBits);
    /* determinism: a rebuild emits the identical sequence. */
    AcDomainTableBuilder again = AcMakeBuilder({{100, 0, true, "example.com"},
                                                {100, 0, true, "example.com"},
                                                {100, 0, false, "example.com"},
                                                {100, 0, true, "www.example*"},
                                                {100, 12345, false, "com"},
                                                {101, 0, true, "a.b.c"}});
    std::vector<AcTransEntry> p3;
    std::vector<AcTransEntry> s3;
    ASSERT_TRUE(again.Emit(7, p3, s3));
    EXPECT_EQ(p3.size(), prefix.size());
    EXPECT_EQ(s3.size(), suffix.size());
    EXPECT_EQ(memcmp(p3.data(), prefix.data(), p3.size() * sizeof(AcTransEntry)), 0);
    EXPECT_EQ(memcmp(s3.data(), suffix.data(), s3.size() * sizeof(AcTransEntry)), 0);
}

HWTEST_F(NetsysBpfNetFirewallTest, AcBuildInvalidRulesOnly001, TestSize.Level0)
{
    /* every rule invalid: the builder ends empty — the production path
     * flips to the cleared tables (full-replace to "no rules"). */
    AcDomainTableBuilder builder;
    EXPECT_FALSE(builder.AddRule(100, 0, true, ""));
    EXPECT_FALSE(builder.AddRule(100, 0, true, "*"));
    EXPECT_FALSE(builder.AddRule(100, 0, true, "a..b"));
    EXPECT_FALSE(builder.AddRule(100, 0, true, std::string(64, 'x')));
    EXPECT_TRUE(builder.Empty());
    std::vector<AcTransEntry> prefix;
    std::vector<AcTransEntry> suffix;
    EXPECT_TRUE(builder.Emit(0, prefix, suffix));
    EXPECT_TRUE(prefix.empty());
    EXPECT_TRUE(suffix.empty());
}
} // namespace NetManagerStandard
} // namespace OHOS
