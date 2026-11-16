/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/* Fuzz the REAL BPF DNS sniffing chain (match_dns_query ->
 * parse_dns_query/parse_dns_response -> ac_prefix_scan/ac_suffix_scan ->
 * parse_answers -> add_domain_cache) with fuzz-built L3 packets. Contract
 * under fuzz: never crash, never read out of bounds (the stub enforces
 * skb->len like the kernel helper), never return anything but
 * SK_PASS/SK_DROP.
 * Env note: the AC tables are NOT stubbed here — every scan misses
 * cleanly, so the allow/deny verdicts stay 0 and the answer-learning
 * deep path is not reached (the fuzz surface is the parse chain itself:
 * L3/L4/DNS header decode, the two anchored name scans and their
 * bounds-checked loads). Restoring verdict-driven deep paths needs an
 * AC table stub keyed to arbitrary fuzz qnames — follow-up item.
 *
 * This is a SEPARATE translation unit from netsys_netfirewall_fuzzer.cpp on
 * purpose: that TU's include chain (clatd/netinet) conflicts with the kernel
 * UAPI structs in netfirewall_utils.h, so the DNS chain cannot be reached
 * from there. All stubs here are static — the sibling TU already defines
 * bpf_skb_load_bytes with external linkage. */

#include <cstdint>
#include <cstring>
#include <sys/socket.h>

#include <linux/bpf.h>

/* user-space mirror of the libbpf bits the domain.h chain expects: the map
 * definition struct for the "maps" globals and the byte-order macros (SEC
 * comes from netfirewall_map_def.h itself). */
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
/* udphdr member names: this TU builds with _GNU_SOURCE, under which OHOS musl
 * netinet/udp.h renames the uh_* fields to source/dest at preprocessing time
 * (#define uh_sport source) — exactly what the headers' udph.source accesses
 * expect. No source/dest remap may be defined here: it would ping-pong with
 * musl's own macros and corrupt later struct definitions (linux/tcp.h). The
 * domain.h remap only exists for the BPF build, which has no _GNU_SOURCE. */

/* typed-pointer proxy: see bpf_netfirewall_test.cpp (StubBpfValue) — the C
 * header bodies assign the helper result directly to typed pointers, which
 * C++ rejects from a void* return. */
struct StubBpfValue {
    void *ptr;
    explicit StubBpfValue(void *p) : ptr(p) {}
    template <typename T> operator T *() const
    {
        return static_cast<T *>(ptr);
    }
};

static const uint8_t *g_dnsFuzzData = nullptr;
static uint32_t g_dnsFuzzLen = 0;

static long bpf_skb_load_bytes(const struct __sk_buff *skb, __u32 offset, void *to, __u32 len);
static StubBpfValue bpf_map_lookup_elem(const void *map, const void *key);
static long bpf_map_update_elem(void *map, const void *key, const void *value, __u64 flags);
static long bpf_map_delete_elem(void *map, const void *key);
static __u32 bpf_get_socket_uid(struct __sk_buff *skb);
static __u64 bpf_ktime_get_ns(void);
static StubBpfValue bpf_ringbuf_reserve(const void *ringbuf, __u64 size, __u64 flags);
static void bpf_ringbuf_submit(void *data, __u64 flags);
static long bpf_skb_get_dev_name(struct __sk_buff *skb, char *name, __u32 name_size);

#include "netfirewall/netfirewall_domain.h"

/* fixed, deterministic stub state: allow-all domain rules and a fresh record
 * drive the fuzz traffic through the DEEPEST paths (answer parsing and
 * learning) instead of the early allow-miss exits. */
static struct domain_hash_key g_fuzzScratch[2] = {};
static struct defalut_action_value g_fuzzDefaultAction = {SK_PASS, SK_PASS};
static struct dns_query_pending g_fuzzPending = {};
static struct dns_flow_info g_fuzzFlow = {};
static struct dns_query_record_value g_fuzzRecord = {};

static long bpf_skb_load_bytes(const struct __sk_buff *skb, __u32 offset, void *to, __u32 len)
{
    if (skb == NULL || to == NULL || g_dnsFuzzData == nullptr) {
        return -1;
    }
    if (static_cast<uint64_t>(offset) + len > g_dnsFuzzLen) {
        return -1;
    }
    if (memcpy(to, g_dnsFuzzData + offset, len) == NULL) {
        return -1;
    }
    return 0;
}

static StubBpfValue bpf_map_lookup_elem(const void *map, const void *key)
{
    if (map == &DOMAIN_SCRATCH_MAP) {
        __u32 idx = *static_cast<const __u32 *>(key);
        return StubBpfValue((idx == 1) ? &g_fuzzScratch[1] : &g_fuzzScratch[0]);
    }
    if (map == &DEFAULT_ACTION_MAP) {
        return StubBpfValue(&g_fuzzDefaultAction);
    }
    if (map == &DNS_QUERY_SCRATCH_MAP) {
        return StubBpfValue(&g_fuzzPending);
    }
    if (map == &DNS_FLOW_SCRATCH_MAP) {
        return StubBpfValue(&g_fuzzFlow);
    }
    if (map == &DNS_QUERY_RECORD_MAP) {
        /* a permanently-fresh record: the validated path always matches */
        g_fuzzRecord.timestamp = bpf_ktime_get_ns() - 1000U;
        return StubBpfValue(&g_fuzzRecord);
    }
    return StubBpfValue(nullptr); /* DENY maps, CURRENT_UID_MAP, ...: miss */
}

static long bpf_map_update_elem(void *map, const void *key, const void *value, __u64 flags)
{
    /* learning writes go nowhere: the fuzz contract is memory safety, not
     * observable state. */
    (void)map;
    (void)key;
    (void)value;
    (void)flags;
    return 0;
}

static long bpf_map_delete_elem(void *map, const void *key)
{
    (void)map;
    (void)key;
    return 0;
}

static __u32 bpf_get_socket_uid(struct __sk_buff *skb)
{
    (void)skb;
    return 200100; /* >= USER_ID_DIVIDOR: a normal attributed userId (1) */
}

static __u64 bpf_ktime_get_ns(void)
{
    return 1000ULL * 1000ULL * 1000ULL * 1000ULL;
}

static StubBpfValue bpf_ringbuf_reserve(const void *ringbuf, __u64 size, __u64 flags)
{
    (void)ringbuf;
    (void)size;
    (void)flags;
    return StubBpfValue(nullptr); /* debug output is dropped in user space */
}

static void bpf_ringbuf_submit(void *data, __u64 flags)
{
    (void)data;
    (void)flags;
}

static long bpf_skb_get_dev_name(struct __sk_buff *skb, char *name, __u32 name_size)
{
    (void)skb;
    (void)name;
    (void)name_size;
    return -1; /* no device context off-device: every interface lookup misses */
}

namespace OHOS {
namespace NetManagerStandard {

/* byte 0 selects the environment: bit0 family (odd = AF_INET6, even =
 * AF_INET), bit1 direction (set = EGRESS, clear = INGRESS); the rest is the
 * raw packet starting at L3. */
void DnsSniffFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < 2) {
        return;
    }
    struct __sk_buff skb = {};
    uint8_t selector = data[0];
    skb.family = (selector & 1) ? AF_INET6 : AF_INET;
    enum stream_dir dir = (selector & 2) ? EGRESS : INGRESS;
    g_dnsFuzzData = data + 1;
    g_dnsFuzzLen = static_cast<uint32_t>(size - 1);
    skb.len = g_dnsFuzzLen;
    /* Pass a real gate ctx so the egress-query qname export runs under
     * fuzz too; the ctx content itself is not a fuzz contract. */
    struct dns_gate_ctx gate = {};
    (void)match_dns_query(skb.family, &skb, dir, &gate);
}
} // namespace NetManagerStandard
} // namespace OHOS
