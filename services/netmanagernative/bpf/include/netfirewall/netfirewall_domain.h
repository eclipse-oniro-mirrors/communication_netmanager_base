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
#ifndef NET_FIREWALL_DOMAIN_H
#define NET_FIREWALL_DOMAIN_H

/* User-space test/fuzz TUs include this chain without libbpf's helper
 * declarations: they declare their own stubs (bpf_map_lookup_elem, the byte
 * loader, ...) before the include and define a mirror bpf_map_def instead
 * (same pattern as netfirewall_utils.h). The real BPF build defines __BPF__. */
#ifdef __BPF__
#include <bpf_helpers.h>
#endif

#include "netfirewall_event.h"
#include "netfirewall_map.h"
#include "netfirewall_match.h"
#include "netfirewall_types.h"
#include "netfirewall_utils.h"

static __always_inline __u32 get_current_uid(struct __sk_buff *skb)
{
    if (skb == NULL) {
        return DEFAULT_USER_ID;
    }
    __u32 sock_uid = bpf_get_socket_uid(skb);
    return get_user_id(sock_uid);
}

/* Attribution (uid/appuid) and lifetime (timestamp/ttl) are
 * parameterized — the validated ingress path passes the record-table values
 * (authoritative), the legacy path passes skb-derived values. The record
 * pointer, when present, donates its timestamp so the learned entry and the
 * query record share one time base. ttl comes in already clamped
 * (dns_learn_ttl_clamp); ttl=0 (TTL=0 answers) is never learned. */
static __always_inline bool add_domain_cache(const __u8 *payload, const __u32 family, __u32 appuid, __u32 uid,
                                             __u32 ttl, const struct dns_query_record_value *record)
{
    bool ret = false;
    struct domain_value value = {0};
    if (ttl == 0) {
        /* The resolver does not cache TTL=0 answers either — aligned
         * behavior, and a zero-ttl entry could never pass the time gate. */
        return false;
    }
    value.appuid = appuid;
    value.uid = uid;
    if (value.uid == UID_UNATTRIBUTED) {
        /* Never learn domains for unattributed queries — there is no app
         * to bind the entry to; learning would poison the [0xFFFFFFFF] key. */
        return false;
    }
    value.ttl = ttl;
    value.timestamp = (record != NULL) ? record->timestamp : bpf_ktime_get_ns();
    if (family == AF_INET) {
        struct ipv4_lpm_key lpm_key = {
            .prefixlen = IPV4_MAX_PREFIXLEN,
        };
        memcpy(&(lpm_key.data), payload, sizeof(lpm_key.data));
        ret = bpf_map_update_elem(&DOMAIN_IPV4_MAP, &lpm_key, &value, 0) == 0;
    } else if (family == AF_INET6) {
        struct ipv6_lpm_key lpm_key = {
            .prefixlen = IPV6_MAX_PREFIXLEN,
        };
        memcpy(&(lpm_key.data), payload, sizeof(lpm_key.data));
        ret = bpf_map_update_elem(&DOMAIN_IPV6_MAP, &lpm_key, &value, 0) == 0;
    }
    return ret;
}

/* The answer TTL is read (one load + ntohl) instead of skipped and
 * handed to add_domain_cache via dns_learn_ttl_clamp. CNAME records are not
 * learned (unchanged) so their TTL is simply not consumed. Load
 * failures abort the answer walk; the address locals are zero-initialized so
 * an uninitialized stack value can never reach the map. */
static __always_inline __u16 parse_answers(struct __sk_buff *skb, __u16 dns_qry_off, __u8 save_ip, __u32 appuid,
                                           __u32 uid, const struct dns_query_record_value *record)
{
    __u16 type = 0;
    __u16 str_len = 0;
    __u32 ttl_raw = 0;
    __u16 offset = dns_qry_off;

    // Name : u16
    offset += sizeof(__u16);
    // Type : u16
    if (bpf_skb_load_bytes(skb, offset, &type, sizeof(__u16)) != 0) {
        return 0;
    }
    type = bpf_ntohs(type);
    offset += sizeof(__u16);
    // Class : u16
    offset += sizeof(__u16);
    // Ttl : u32
    if (bpf_skb_load_bytes(skb, offset, &ttl_raw, sizeof(__u32)) != 0) {
        return 0;
    }
    offset += sizeof(__u32);
    // Data len : u16
    if (bpf_skb_load_bytes(skb, offset, &str_len, sizeof(__u16)) != 0) {
        return 0;
    }
    str_len = bpf_ntohs(str_len);
    offset += sizeof(__u16);
    if (str_len == 0) {
        return offset;
    }
    // ipv4 or ipv6 addr
    if (type == DNS_QRS_IPV4_TYPE && str_len == DNS_QRS_IPV4_LEN) {
        __u32 addr = 0;
        if (bpf_skb_load_bytes(skb, offset, &addr, sizeof(__u32)) != 0) {
            return 0;
        }
        if (save_ip) {
            add_domain_cache((__u8 *)&addr, AF_INET, appuid, uid, dns_learn_ttl_clamp(bpf_ntohl(ttl_raw)), record);
        }
    } else if (type == DNS_QRS_IPV6_TYPE && str_len == DNS_QRS_IPV6_LEN) {
        ip6_key ip6_addr = {{{0}}}; /* in6_addr wraps s6_addr[16]: three brace levels keep C++ -Wmissing-braces quiet */
        if (bpf_skb_load_bytes(skb, offset, &ip6_addr, sizeof(ip6_key)) != 0) {
            return 0;
        }
        if (save_ip) {
            add_domain_cache((__u8 *)&ip6_addr, AF_INET6, appuid, uid, dns_learn_ttl_clamp(bpf_ntohl(ttl_raw)), record);
        }
    }
    /* u16 wrap guard — a corrupt rdlength that would wrap the
     * offset aborts the walk (0) instead of re-reading from a bogus offset. */
    if (offset > 0xFFFF - str_len) {
        return 0;
    }
    offset += str_len;

    return offset;
}

/* ===== A' label-anchored trie, two anchored scans =====
 * The production domain-matching path (the pre-AC walk + reverse-swap +
 * LPM chain was removed with its four BPF maps — the AC tables are the
 * only consumer of domain rules since 2026-11).
 * The forward scan anchors ALL prefix rules at the name start
 * (single descent, no restart); the backward scan anchors ALL suffix
 * rules at the terminator — every suffix pattern shares that anchor, so
 * overlapping rules ("com" and "example.com") both hit through the
 * shared trie path, the map counterpart of the reverse-string LPM
 * semantics. Label alignment is structural: the class space keeps
 * length bytes (cls 1-63), content bytes (0x100|fold) and the
 * terminator (0x200) disjoint, so a content digit can never masquerade
 * as a length class.
 * Design note: the backward scan takes label boundaries from the
 * FORWARD scan's bitmap instead of a per-transition rem — two patterns
 * that diverge mid-label (e.g. "example.com" and "tinyexample.com"
 * share TERM,'m','o','c',LEN(3) and seven chars, then one expects
 * LEN(7) while the other expects CHAR('y')) would need two different
 * rem values on the same shared transition, which a map cannot store. */

/* Case-fold a wire byte: the trie is built folded, so matching becomes
 * case-insensitive on the BPF side (aligned with the user-space trie;
 * the legacy LPM path was case-sensitive — semantic change, reported). */
static __always_inline __u8 ac_fold(__u8 c)
{
    return (c >= 'A' && c <= 'Z') ? (__u8)(c + ('a' - 'A')) : c;
}

/* noinline (bpf-to-bpf subprogram, 3 args): the scan is explored once
 * per distinct entry state; uid/appuid travel via the scratch key (map
 * memory), so the exact and wild calls from one caller share a single
 * entry state. The subprogram boundary is a ~100x state-space saver
 * (195 states measured vs 27k+ with the chain inlined) — the DNS chain
 * must stay subprograms. The dns programs carry no bpf_tail_call, so
 * mixing subprogram calls there is legal (see netsys.c). */
static __attribute__((noinline)) __u32 ac_prefix_scan(struct __sk_buff *skb, __u16 off, struct domain_hash_key *key)
{
    __u32 one = 1;
    struct domain_hash_key *origin_key = bpf_map_lookup_elem(&DOMAIN_SCRATCH_MAP, &one);
    if (origin_key == NULL) {
        return AC_PARSE_FAIL;
    }
    /* Label-boundary bitmap (bits over positions 0..127, data[0..15] of
     * the origin slot). Cleared every scan: the slot may carry a
     * previous packet's residue. */
    __u8 *boundary = origin_key->data;
    __builtin_memset(boundary, 0, AC_NAME_SCAN_MAX / 8);
    /* Transition key overlaid on the caller's scratch key at data[12..]:
     * a stack local would push the bpf-to-bpf chain's combined frame
     * past the verifier's 512-byte limit; the overlay region is private
     * to the scans (the gate export below only touches uid/appuid).
     * Bytes 0..11 (prefixlen/uid/appuid) stay untouched. */
    struct ac_trans_key *k = (struct ac_trans_key *)(void *)((__u8 *)key + 24);
    __builtin_memset(k, 0, sizeof(*k));
    __u32 zero = 0;
    __u32 *genp = bpf_map_lookup_elem(&AC_GEN_MAP, &zero);
    if (genp == NULL) {
        return AC_PARSE_FAIL;
    }
    k->uid = key->uid;
    k->appuid = key->appuid;
    k->gen = *genp;
    __u32 state = 0;
    __u32 rem = 0;
    __u32 flags = 0;
    /* Provisional label-confined allow verdicts (trailing-star rules,
     * AC_FLAG_PREFIX_ALLOW_LC): kept apart from `flags` so a later label
     * boundary can drop them without touching the plain verdicts — a
     * plain deny of the same family deliberately survives to the right. */
    __u32 lcAllow = 0;
    /* Flat concrete-stride loop (the name-driven rem never mixes with
     * the induction variable, so its tnum stays stable across
     * iterations). rem is laundered through the scratch prefixlen on
     * every content byte: a plain decrement would let the verifier
     * relate rem to the length byte it came from and drift the loop
     * state, which is what exhausted the complexity budget on the old
     * walk (boundary = j + labelLen + 1 shifted per iteration). */
    for (__u32 i = 0; i < AC_NAME_SCAN_MAX; i++) {
        __u8 c = 0;
        if (bpf_skb_load_bytes(skb, (__u32)off + i, &c, sizeof(__u8)) != 0) {
            return AC_PARSE_FAIL;
        }
        if (rem == 0) {
            if (c == 0) {
                /* Terminator: last TERM transition, stage the name
                 * length (positions consumed, terminator included) for
                 * the suffix scan and the answers offset. Staged even
                 * on a dead walk — the suffix scan still runs. */
                k->state = state;
                k->cls = AC_CLS_TERM;
                struct ac_trans_val *tv = bpf_map_lookup_elem(&AC_PREFIX_TRANS_MAP, k);
                if (tv != NULL) {
                    flags |= tv->flags & ~AC_FLAG_PREFIX_ALLOW_LC;
                    lcAllow |= tv->flags & AC_FLAG_PREFIX_ALLOW_LC;
                }
                /* A label-confined allow verdict survived every later
                 * label boundary: the completed fragment really sits in
                 * the name's LAST label — promote it to the standard
                 * allow bit so every downstream consumer keeps reading
                 * AC_PREFIX_MASK only. */
                if (lcAllow != 0) {
                    flags |= AC_FLAG_PREFIX_ALLOW;
                }
                *(volatile __u16 *)&key->prefixlen = (__u16)(i + 1);
                return flags & AC_PREFIX_MASK;
            }
            /* RFC 1035: a label length is 6 bits; anything above 63 is
             * garbage — abort the whole parse (fail-closed: no verdict,
             * the packet keeps flowing through the policy chain). */
            if (c > DNS_LABEL_MAX_LEN) {
                return AC_PARSE_FAIL;
            }
            rem = c;
            k->cls = AC_CLS_LEN(c);
            boundary[i >> 3] |= (__u8)(1u << (i & 7));
        } else {
            k->cls = AC_CLS_CHAR(ac_fold(c));
            *(volatile __u32 *)&key->prefixlen = rem - 1;
            rem = *(volatile __u32 *)&key->prefixlen;
        }
        /* Label confinement (trailing-star allow rules): the verdict is
         * provisional — a LEN class AFTER the completion opens a new
         * label to the right, so the fragment did not sit in the name's
         * last label; drop it ("www.example.c*" must not allow
         * "www.example.c.evil.com"). Checked BEFORE this step's
         * transition so a verdict the current class completes is never
         * eaten by its own boundary — the SKIP edge consuming this LEN
         * completes the pattern later, at the fragment. LEN classes are
         * 1..63 (< 0x100), CHAR classes carry 0x100 — the class space
         * itself tells the boundary. Deny verdicts are plain bits and
         * unaffected: a deny stays valid to the right on purpose. */
        if (lcAllow != 0 && k->cls < AC_CLS_CHAR(0)) {
            lcAllow = 0;
        }
        k->state = state;
        struct ac_trans_val *tv = bpf_map_lookup_elem(&AC_PREFIX_TRANS_MAP, k);
        if (tv == NULL) {
            /* Miss: dead terminal state. The marker is never a
             * builder-written node, so every later lookup misses too —
             * uniform code shape, no extra branch. Name parsing
             * continues regardless (structure + name length must still
             * be produced for the suffix scan). */
            state = AC_PARSE_FAIL;
        } else {
            state = tv->next;
            flags |= tv->flags & ~AC_FLAG_PREFIX_ALLOW_LC;
            lcAllow |= tv->flags & AC_FLAG_PREFIX_ALLOW_LC;
        }
    }
    /* No terminator within the scan bound — malformed name. */
    return AC_PARSE_FAIL;
}

/* noinline (bpf-to-bpf subprogram, 3 args): see ac_prefix_scan. */
static __attribute__((noinline)) __u32 ac_suffix_scan(struct __sk_buff *skb, __u16 off, struct domain_hash_key *key)
{
    __u32 one = 1;
    struct domain_hash_key *origin_key = bpf_map_lookup_elem(&DOMAIN_SCRATCH_MAP, &one);
    if (origin_key == NULL) {
        return AC_PARSE_FAIL;
    }
    const __u8 *boundary = origin_key->data;
    struct ac_trans_key *k = (struct ac_trans_key *)(void *)((__u8 *)key + 24);
    __builtin_memset(k, 0, sizeof(*k));
    __u32 zero = 0;
    __u32 *genp = bpf_map_lookup_elem(&AC_GEN_MAP, &zero);
    if (genp == NULL) {
        return AC_PARSE_FAIL;
    }
    k->uid = key->uid;
    k->appuid = key->appuid;
    k->gen = *genp;

    /* Name length staged by the forward scan. The u16 round-trip keeps
     * it an opaque [0,65535] scalar; the DIRECT compares below refine
     * the reloaded register itself to [1, AC_NAME_SCAN_MAX] so the
     * position arithmetic stays provably in range. */
    __u16 name_len = *(volatile __u16 *)&key->prefixlen;
    if (name_len < 1 || name_len > AC_NAME_SCAN_MAX) {
        /* Unreachable at runtime: the forward scan staged a value in
         * range or failed the whole parse before this call. */
        return AC_PARSE_FAIL;
    }

    __u32 state = 0;
    __u32 flags = 0;
    /* Peeled terminator step (every suffix pattern anchors here): a
     * miss means this (uid, appuid, gen) has no suffix rules at all —
     * no verdict, no walk. */
    __u8 c = 0;
    *(volatile __u16 *)&key->prefixlen = (__u16)(name_len - 1);
    __u16 idx = *(volatile __u16 *)&key->prefixlen;
    if (idx >= AC_NAME_SCAN_MAX) {
        /* Unreachable at runtime: name_len >= 1 keeps idx in [0,127].
         * No restore needed: every AC_PARSE_FAIL exit ORs all-ones into
         * the caller's combined verdict, which bails before anyone
         * reads the staged length again. */
        return AC_PARSE_FAIL;
    }
    if (bpf_skb_load_bytes(skb, (__u32)off + idx, &c, sizeof(__u8)) != 0) {
        return AC_PARSE_FAIL;
    }
    k->state = 0;
    k->cls = AC_CLS_TERM;
    struct ac_trans_val *tv = bpf_map_lookup_elem(&AC_SUFFIX_TRANS_MAP, k);
    if (tv == NULL) {
        /* The staged name length is CONTRACT state, not scratch: the
         * caller runs this scan up to twice (exact appuid, then wild
         * appuid=0) plus the response path reads it again after the
         * pair for the answers offset. Every NON-FAIL exit must leave
         * the staged value intact — laundering idx through prefixlen is
         * scan-internal only. Leaving the laundered residue here made
         * the wild scan read a corrupted name_len (dead anchor walk);
         * a completed walk left 0xFFFF which the next scan turned into
         * AC_PARSE_FAIL, and the caller OR-eated the whole verdict
         * (measured on-device: deny rules never fired). */
        *(volatile __u16 *)&key->prefixlen = name_len;
        return 0;
    }
    state = tv->next;
    flags |= tv->flags;

    /* Backward walk from name_len-2 down to 0. The loop is driven by a
     * concrete counter (progress: an all-opaque induction made every
     * back-edge state identical to the previous one and the verifier
     * rejected the loop as infinite — measured EINVAL "infinite loop
     * detected"); the position itself stays an opaque u16 through the
     * scratch prefixlen round-trip (a plain register decrement leaves a
     * negative-capable value that kills the bitmap access of the next
     * iteration). The internal range check refines idx to
     * [0, AC_NAME_SCAN_MAX) before the bitmap read; the 0xFFFF wrap
     * past position 0 exits. Class comes from the forward scan's
     * boundary bitmap: the name's own structure decides length-byte vs
     * content position, which is what makes the match label-aligned. */
    *(volatile __u16 *)&key->prefixlen = (__u16)(name_len - 2);
    idx = *(volatile __u16 *)&key->prefixlen;
    for (__u32 n = 0; n < AC_NAME_SCAN_MAX; n++) {
        if (idx >= AC_NAME_SCAN_MAX) {
            break;
        }
        if (bpf_skb_load_bytes(skb, (__u32)off + idx, &c, sizeof(__u8)) != 0) {
            /* No restore: AC_PARSE_FAIL paths are OR-eaten upstream
             * before the staged length is read again. */
            return AC_PARSE_FAIL;
        }
        __u8 bm = boundary[idx >> 3];
        __u32 is_len = (__u32)((bm >> (idx & 7)) & 1);
        k->cls = is_len ? AC_CLS_LEN(c) : AC_CLS_CHAR(ac_fold(c));
        k->state = state;
        tv = bpf_map_lookup_elem(&AC_SUFFIX_TRANS_MAP, k);
        if (tv == NULL) {
            /* Dead terminal state — see ac_prefix_scan. No structural
             * FAIL here: the forward scan already validated the name,
             * and a live walk's bitmap classes are true classes by
             * construction; on a dead walk the class is irrelevant
             * (every lookup misses). */
            state = AC_PARSE_FAIL;
        } else {
            state = tv->next;
            flags |= tv->flags;
        }
        *(volatile __u16 *)&key->prefixlen = (__u16)(idx - 1);
        idx = *(volatile __u16 *)&key->prefixlen;
    }
    /* Restore the staged name length before returning — see the peel
     * comment: the residue (0xFFFF after a full walk) must never escape
     * this scan; the wild re-scan and the response-side answers-offset
     * read both consume the staged value after this call. */
    *(volatile __u16 *)&key->prefixlen = name_len;
    return flags & AC_SUFFIX_MASK;
}

/* Stage the query credential (record-table key + egress-side
 * attribution) in the percpu slot. The skb pointer address tags the staging so
 * a later packet can never flush a dropped packet's leftovers. */
static __always_inline void record_dns_query_pending(struct __sk_buff *skb, const struct dns_flow_info *flow)
{
    __u32 zero = 0;
    struct dns_query_pending *pending = bpf_map_lookup_elem(&DNS_QUERY_SCRATCH_MAP, &zero);
    if (pending == NULL || flow == NULL) {
        return;
    }
    pending->skb_addr = (__u64)(unsigned long)skb;
    pending->valid = 1;
    /* The egress query's flow is ALREADY in the query view — copy it verbatim.
     * (build_dns_query_record_key reverses a RESPONSE flow and is used only by
     * the lookup side; applying it here would store a response-view key that no
     * lookup could ever hit.) */
    pending->key.txid = flow->txid;
    pending->key.protocol = flow->protocol;
    pending->key.family = flow->family;
    pending->key.sport = flow->sport;
    pending->key.dport = flow->dport;
    if (flow->family == AF_INET) {
        pending->key.ipv4.saddr = flow->ipv4.saddr;
        pending->key.ipv4.daddr = flow->ipv4.daddr;
    } else if (flow->family == AF_INET6) {
        pending->key.ipv6.saddr = flow->ipv6.saddr;
        pending->key.ipv6.daddr = flow->ipv6.daddr;
    } else {
        pending->valid = 0;
        return;
    }
    pending->uid = get_current_uid(skb);
    pending->appuid = bpf_get_socket_uid(skb);
}

/* Flush point — called at the egress hook exit with the FINAL verdict.
 * Only queries that ultimately PASS are recorded; a dropped query's staging is
 * discarded. pending->valid is cleared on both paths so the slot never leaks. */
static __always_inline void flush_dns_query_record(struct __sk_buff *skb, bool passed)
{
    __u32 zero = 0;
    struct dns_query_pending *pending = bpf_map_lookup_elem(&DNS_QUERY_SCRATCH_MAP, &zero);
    if (pending == NULL || pending->valid != 1 || pending->skb_addr != (__u64)(unsigned long)skb) {
        return;
    }
    if (passed) {
        struct dns_query_record_value value = {
            .uid = pending->uid,
            .appuid = pending->appuid,
            .timestamp = bpf_ktime_get_ns(),
        };
        if (bpf_map_update_elem(&DNS_QUERY_RECORD_MAP, &pending->key, &value, BPF_ANY) == 0) {
            log_dbg(DBG_DNS_RECORD_WRITE, EGRESS, pending->key.txid);
        }
    }
    pending->valid = 0;
}

/* The ingress learning credential — the reversed 5-tuple + txid must
 * hit a record no older than DNS_RECORD_EXPIRE_NS. dns_record_expired also
 * rejects a record whose timestamp is in the future (fail-safe). */
static __always_inline const struct dns_query_record_value *lookup_dns_query_record(const struct dns_flow_info *flow)
{
    struct dns_query_record_key key = {0};
    __u64 now = bpf_ktime_get_ns();
    if (build_dns_query_record_key(flow, &key) != 0) {
        return NULL;
    }
    struct dns_query_record_value *record = bpf_map_lookup_elem(&DNS_QUERY_RECORD_MAP, &key);
    if (record == NULL) {
        return NULL;
    }
    if (dns_record_expired(now, record->timestamp)) {
        return NULL;
    }
    return record;
}

/* flow != NULL only for an egress query: the query credential is staged
 * (allowrst only) and flushed at the hook exit if the final verdict is
 * PASS. Ingress queries pass NULL and keep exactly the
 * current behavior.
 * gate != NULL only for an egress query: the sniffing exports the qname verdict
 * (condition 4 of the per-domain gate) to the policy layer. It is filled only
 * for a parseable single-question query — anything else keeps is_query=0 so
 * the gate stays closed (fail-closed). (Port 853 is exempt from the parse
 * requirement: match_dns_query marks is_query by port shape there — the DoT
 * payload is encrypted; see its own comment.) */
/* noinline (nested bpf-to-bpf subprogram): the qname parse chain (scan,
 * walk, reverse swap) rides as its own subprogram explored once per
 * distinct argument state — the ~100x state-space saver that keeps the
 * DNS chain inside the verifier budget (13.5k insns as subprograms vs
 * >1M inlined, measured on-device). See match_dns_query below. */
static __attribute__((noinline)) __u8 parse_dns_query(struct __sk_buff *skb, __u16 dns_qry_off, __u32 query_meta,
                                                      const struct dns_flow_info *flow, struct dns_gate_ctx *gate)
{
    /* bpf-to-bpf calls take at most 5 args (R1-R5): quNum and
     * defaultactionrst travel packed in the high/low half of query_meta. */
    __u16 quNum = (__u16)(query_meta >> 16);
    __u16 defaultactionrst = (__u16)query_meta;
    if (quNum == 1) {
        __u32 zero = 0;
        struct domain_hash_key *key = bpf_map_lookup_elem(&DOMAIN_SCRATCH_MAP, &zero);
        if (key == NULL) {
            return 0;
        }
        /* The scratch slot may carry a previous packet's residue. */
        __builtin_memset(key, 0, sizeof(*key));

        /* A' anchored scans: exact (appuid attributed) then wild
         * (appuid 0) per anchor, verdict = union, deny wins at the
         * adjudication below. uid/appuid travel via the scratch key —
         * map memory reaches the subprograms as opaque scalars, so the
         * exact and wild calls share one entry state per subprogram. */
        __u32 appuid_exact = bpf_get_socket_uid(skb);
        key->uid = get_current_uid(skb);
        key->appuid = appuid_exact;
        __u32 vp = ac_prefix_scan(skb, dns_qry_off, key);
        key->appuid = 0;
        __u32 vp_wild = ac_prefix_scan(skb, dns_qry_off, key);
        key->appuid = appuid_exact;
        __u32 vs = ac_suffix_scan(skb, dns_qry_off, key);
        key->appuid = 0;
        __u32 vs_wild = ac_suffix_scan(skb, dns_qry_off, key);
        /* A parse failure ORs the whole marker in (all-ones); flags
         * only ever use the low four bits, so the equality test is
         * exact. Fail-closed: no verdict, the packet keeps flowing
         * through the policy chain. */
        __u32 combined = vp | vp_wild | vs | vs_wild;
        if (combined == AC_PARSE_FAIL) {
            return 0;
        }
        __u16 denyrst = ((combined & (AC_FLAG_PREFIX_DENY | AC_FLAG_SUFFIX_DENY)) != 0) ? 1 : 0;
        __u16 allowrst = ((combined & (AC_FLAG_PREFIX_ALLOW | AC_FLAG_SUFFIX_ALLOW)) != 0) ? 1 : 0;

        /* Export the parsed qname verdict to the policy layer (gate
         * condition 4). Set before the deny short-circuits: a deny-qname query
         * is dropped right here by the sniffer and never consults the
         * gate, and a pass-through ctx with allowrst=0 fails the gate anyway.
         * The two bytes travel through an opaque u32 round-trip on the
         * scratch key: exact 0/1 bytes would fork every caller
         * continuation per sniff outcome and the egress policy chain
         * gets re-verified per distinct state that reaches it. Opaque
         * scalars keep the chain's entry shape single; runtime values
         * are bit-identical. key->uid is scratch here — the AC scans are
         * done with it by now, the policy layer re-derives uid itself. */
        if (gate != NULL) {
            *(volatile __u32 *)&key->uid = ((__u32)1 << 8) | ((allowrst == 1) ? 1u : 0u);
            __u32 gateBits = *(volatile __u32 *)&key->uid;
            gate->is_query = (__u8)(gateBits >> 8);
            gate->allowrst = (__u8)gateBits;
        }

        /* Double-hit conflict resolution follows the DEFAULT action
         * direction — same semantics as the IP chain's Conflict &
         * Repetition Algorithm in match_action: under DEFAULT-DENY
         * (defaultactionrst == SK_DROP) any deny hit drops — an explicit
         * deny outranks an opposing allow; under DEFAULT-ALLOW
         * (defaultactionrst == SK_PASS) a deny hit drops only when
         * UNOPPOSED — an explicit allow outranks an opposing deny.
         * Polarity note: defaultactionrst carries the RAW sk_action enum
         * (SK_DROP=0, SK_PASS=1) — it is NOT a boolean "default is deny";
         * !defaultactionrst means the default action is DROP. */
        if (!defaultactionrst && denyrst) {
            return 1;
        }

        if (defaultactionrst && denyrst && !allowrst) {
            return 1;
        }

        /* Stage the query record when the qname is allow-
         * listed. Only allow queries occupy record-table capacity. A
         * later DROP verdict at the hook exit discards the staging. */
        if (allowrst == 1 && flow != NULL) {
            record_dns_query_pending(skb, flow);
        }
    }
    return 0;
}

/* flow != NULL only for an INGRESS response: the learning credential must
 * match the egress query record (txid + reversed 5-tuple + 60s) and
 * uid/appuid come from that record, never from the response packet.
 * flow == NULL (defensive only: the only caller passes a flow) keeps the
 * skb-derived path. */
/* noinline (nested bpf-to-bpf subprogram): see parse_dns_query above. */
static __attribute__((noinline)) __u16 parse_dns_response(struct __sk_buff *skb, __u16 dns_qry_off, __u32 answer_meta,
                                                          __u16 defaultactionrst, const struct dns_flow_info *flow)
{
    /* bpf-to-bpf calls take at most 5 args (R1-R5): qu_num and asNum
     * travel packed in the high/low half of answer_meta. */
    __u16 qu_num = (__u16)(answer_meta >> 16);
    __u16 asNum = (__u16)answer_meta;
    __u16 offset;
    bool is_in_pass = 0;
    __u32 appuid = 0;
    __u32 uid = 0;
    const struct dns_query_record_value *record = NULL;

    if (qu_num == 1) {
        __u32 zero = 0;
        struct domain_hash_key *key = bpf_map_lookup_elem(&DOMAIN_SCRATCH_MAP, &zero);
        if (key == NULL) {
            return 0;
        }
        /* The scratch slot may carry a previous packet's residue. */
        __builtin_memset(key, 0, sizeof(*key));

        /* A' anchored scans — see parse_dns_query. */
        __u32 appuid_tmp = bpf_get_socket_uid(skb);
        key->uid = get_current_uid(skb);
        key->appuid = appuid_tmp;
        __u32 vp = ac_prefix_scan(skb, dns_qry_off, key);
        key->appuid = 0;
        __u32 vp_wild = ac_prefix_scan(skb, dns_qry_off, key);
        key->appuid = appuid_tmp;
        __u32 vs = ac_suffix_scan(skb, dns_qry_off, key);
        key->appuid = 0;
        __u32 vs_wild = ac_suffix_scan(skb, dns_qry_off, key);
        __u32 combined = vp | vp_wild | vs | vs_wild;
        if (combined == AC_PARSE_FAIL) {
            return 0;
        }
        __u16 denyrst = ((combined & (AC_FLAG_PREFIX_DENY | AC_FLAG_SUFFIX_DENY)) != 0) ? 1 : 0;
        __u16 allowrst = ((combined & (AC_FLAG_PREFIX_ALLOW | AC_FLAG_SUFFIX_ALLOW)) != 0) ? 1 : 0;

        /* Position after the query name: the forward scan staged the
         * consumed length (terminator included); skip TYPE(2)+CLASS(2).
         * The reload keeps the answers offset an opaque scalar so the
         * answer loop's entry state does not fork per name length. */
        __u16 name_len = *(volatile __u16 *)&key->prefixlen;
        if (name_len < 1 || name_len > AC_NAME_SCAN_MAX) {
            /* Unreachable at runtime: the scan staged it in range or
             * failed the whole parse above. */
            return 0;
        }
        offset = (__u16)((__u32)dns_qry_off + name_len + sizeof(__u32));

        /* Double-hit conflict resolution follows the DEFAULT action
         * direction — the same matrix as the query side (parse_dns_query)
         * and the IP chain's Conflict & Repetition Algorithm in
         * match_action. Polarity note: defaultactionrst carries the RAW
         * sk_action enum (SK_DROP=0, SK_PASS=1) — !defaultactionrst means
         * the default action is DROP (default-DENY). */
        if (!defaultactionrst && denyrst) {
            return 1;
        }

        if (defaultactionrst && denyrst && !allowrst) {
            return 1;
        }

        if (!allowrst) {
            return 0;
        }

        if (flow != NULL) {
            record = lookup_dns_query_record(flow);
            if (record == NULL) {
                /* -: a failed credential check only skips learning —
                 * the response itself keeps going through the policy/ct path,
                 * otherwise the attack turns into a DoS on legitimate
                 * resolution (record evicted by LRU, address change, ...). */
                log_dbg(DBG_DNS_LEARN_REJECT, INGRESS, 0);
                return 0;
            }
            uid = record->uid;
            appuid = record->appuid;
            /* Deny-default users — exactly the managed
             * population and the DNS_SERVER map population (allowrst
             * above already proves the domain-allow half) — only learn from
             * an authorized server. Closes the window where the server set
             * changed between the query and the response. Default-allow users
             * hold no map entries and keep the plain credential path. */
            struct defalut_action_value *o2Default = bpf_map_lookup_elem(&DEFAULT_ACTION_MAP, &record->uid);
            if (o2Default != NULL && o2Default->outaction == SK_DROP &&
                !MatchDnsServer(record->uid, flow->family, flow->ipv4.saddr, &flow->ipv6.saddr)) {
                log_dbg(DBG_DNS_LEARN_REJECT, INGRESS, 1);
                return 0;
            }
        } else {
            /* flow == NULL (defensive only): skb-derived attribution. */
            uid = get_current_uid(skb);
            appuid = appuid_tmp;
        }
        is_in_pass = 1;
        log_dbg(DBG_MATCH_DOMAIN_ACTION, EGRESS, SK_PASS);
    } else {
        // not support multi questions
        return 0;
    }

    for (__u16 i = 0; i < DNS_ANSWER_CNT; i++) {
        if (i >= asNum) {
            break;
        }
        offset = parse_answers(skb, offset, is_in_pass, appuid, uid, record);
        /* A failed bounded load inside parse_answers returns
         * 0 — stop instead of re-walking from the header on the next pass. */
        if (offset == 0) {
            break;
        }
    }

    return 0;
}

#ifndef _GNU_SOURCE
#define source uh_sport
#define dest uh_dport
#endif

static __always_inline __u16 MatchDefaultActionMap(struct __sk_buff *skb)
{
    __u32 currentUid = get_current_uid(skb);
    /* UID_UNATTRIBUTED lands on the reserved DEFAULT_ACTION_MAP[UID_UNATTRIBUTED]
     * entry here: the plain lookup below is the reserved-key
     * fallback; a miss keeps SK_PASS. */
    struct defalut_action_value *defaultValue = bpf_map_lookup_elem(&DEFAULT_ACTION_MAP, &currentUid);
    enum sk_action sk_act = SK_PASS;
    if (defaultValue) {
        sk_act = defaultValue->outaction;
    }
    return sk_act;
}

/* Shared DNS-message sniffing for both transports (UDP and TCP
 * 53). dns_hdr_off points at the DNS message itself — for TCP the caller has
 * already skipped the 2-byte length prefix. wire_sport/wire_dport are the raw
 * __be16 L4 ports of the packet (query view), used verbatim for the flow /
 * record identity. */
static __always_inline enum sk_action sniff_dns_message(__u32 family, struct __sk_buff *skb, enum stream_dir dir,
                                                        __u8 protocol, __u16 dns_hdr_off, __be16 wire_sport,
                                                        __be16 wire_dport, struct dns_gate_ctx *gate)
{
    struct dnshdr dnsh = {0};
    /* A header load failure aborts the sniffing — no partially
     * loaded DNS header reaches the parse functions. */
    if (bpf_skb_load_bytes(skb, dns_hdr_off, &dnsh, sizeof(struct dnshdr)) != 0) {
        return SK_PASS;
    }
    __u16 qu_num = bpf_ntohs(dnsh.qdcount);
    __u16 as_num = bpf_ntohs(dnsh.ancount);

    __u16 dns_qry_off = dns_hdr_off + sizeof(struct dnshdr);
    __u16 flag = bpf_ntohs(dnsh.flag);
    __u16 bit_mask = (1 << DNS_QR_DEFALUT_MASK);
    __u16 qr = (flag & bit_mask);
    __u16 res = 0;

    __u16 defaultactionrst = MatchDefaultActionMap(skb);
    /* The flow identity is taken from the packet itself (no
     * match_tuple swap semantics), addresses in network order. Address
     * load failures are practically unreachable (the 12-byte dnsh load
     * above already succeeded, which bounds-guarantees the earlier L3
     * address region, and every L4 length check keeps the payload within
     * the packet) — and a failed load now degrades to a learning-skip
     * instead of staging a zero-address dead record.
     * The flow is assembled in a percpu scratch slot instead of a stack
     * local: a 40-byte local here inflates the bpf-to-bpf call chain's
     * combined stack frame past the verifier's 512-byte limit, and exact
     * stack contents would also split the parse subprogram entry states
     * per L3/L4 shape. Through the map every field reaches the callee as
     * an opaque scalar (single packet lifetime on one CPU, same
     * rationale as DNS_QUERY_SCRATCH_MAP). A missing slot (unreachable
     * in practice for a 1-entry percpu array) aborts the sniffing: no
     * partial flow reaches the parse functions. */
    __u32 zero = 0;
    struct dns_flow_info *flow = bpf_map_lookup_elem(&DNS_FLOW_SCRATCH_MAP, &zero);
    if (flow == NULL) {
        return SK_PASS;
    }
    __builtin_memset(flow, 0, sizeof(*flow));
    /* Opaque u16 round-trip for dns_qry_off through the scratch (the
     * txid slot is overwritten with the real txid right below; the
     * laundered offset lives on in a register): dns_qry_off otherwise
     * carries a different tnum per L3/L4 shape (v4/v6 x UDP/TCP x
     * ext-header chains) into the parse subprogram, and the verifier
     * re-explores the whole parse chain per distinct argument state. The
     * round-trip hands every path the same [0, 65535] scalar so the
     * chain is verified once; the runtime value is bit-identical. */
    *(volatile __u16 *)&flow->txid = dns_qry_off;
    dns_qry_off = *(volatile __u16 *)&flow->txid;
    flow->protocol = protocol;
    flow->txid = bpf_ntohs(dnsh.id);
    flow->sport = wire_sport;
    flow->dport = wire_dport;
    /* Decide the flow family from the packet itself, mirroring
     * the policy side (is_ipv4_format) — a dual-stack socket presents
     * v4 packets with family=AF_INET6, and the old raw-family branch read
     * 32 bytes from offset 8 (garbage key), so the record never matched and
     * that population's learning silently failed. */
    __u8 flow_addr_ok = 1;
    if (is_ipv4_format(family, skb)) {
        flow->family = AF_INET;
        if (bpf_skb_load_bytes(skb, L3_NHOFF + 12, &flow->ipv4.saddr, sizeof(__be32) * 2) != 0) {
            flow_addr_ok = 0;
        }
    } else {
        flow->family = AF_INET6;
        if (bpf_skb_load_bytes(skb, L3_NHOFF + 8, &flow->ipv6.saddr, sizeof(struct in6_addr) * 2) != 0) {
            flow_addr_ok = 0;
        }
    }
    if (qr == bit_mask) { /* response */
        /* D-egress: only the ingress side parses responses. An
         * egress "response" (a local app forging an outbound reply) is not
         * parsed at all — the main vector is structurally dead; the packet
         * keeps flowing through the policy/ct chain untouched. */
        if (dir == INGRESS) {
            res = parse_dns_response(skb, dns_qry_off, ((__u32)qu_num << 16) | as_num, defaultactionrst, flow);
        }
    } else { /* query */
        /* Only egress queries can stage a record: the matching ingress
         * response will arrive with the reversed 5-tuple. flow_addr_ok=0
         * (unreachable in practice): stage no record — the
         * gate ctx is still filled below, resolution is unaffected. */
        res = parse_dns_query(skb, dns_qry_off, ((__u32)qu_num << 16) | defaultactionrst,
                              (dir == EGRESS && flow_addr_ok) ? flow : NULL, (dir == EGRESS) ? gate : NULL);
    }
    if (res == 1) {
        log_dbg(DBG_MATCH_DOMAIN_ACTION, EGRESS, SK_DROP);
        return SK_DROP;
    }
    return SK_PASS;
}

/* Direction-aware sniffing over UDP AND TCP 53.
 * Egress queries stage the record credential (allow-listed qname — the
 * deny short-circuits return before staging) and fill the per-domain gate
 * context (is_query + allowrst); ingress responses validate against
 * the record table (plus the authorized-server check); egress "responses"
 * are not parsed at all (D-egress). Port 853 is gate-only: the payload is
 * encrypted (DoT), so the sniffer never parses it — it just marks the gate
 * ctx by port shape. gate is only ever non-NULL from the egress policy hook;
 * every other caller passes NULL. */
/* noinline (a bpf-to-bpf subprogram call): the DNS parse chain is the
 * dominant contributor to the verifier's state space when INLINED
 * (26.7k+ states, >1M processed insns — measured 2026-11 on-device with
 * the chain fully inlined). As a subprogram the verifier explores it
 * once per distinct argument state at the call boundary instead
 * (13.5k insns / 195 states measured), a ~100x state-space saving.
 * Runtime semantics are unchanged; the call costs a few cycles per
 * packet against a once-per-boot verification win. Requires the loader
 * to merge the .text section into each prog (dead-strip: only the
 * functions reachable from THIS prog are appended — see bpf_loader.cpp).
 * Callers: the two dns programs in netsys.c (+ unit tests, NULL gate). */
static __attribute__((noinline)) enum sk_action match_dns_query(__u32 family, struct __sk_buff *skb,
                                                                enum stream_dir dir, struct dns_gate_ctx *gate)
{
    if (!skb) {
        return SK_PASS;
    }

    __u8 protocol = 0;
    __u32 l4_nhoff = 0;
    /* The duplicate fixed-offset parse (20/40 + nexthdr) is deleted — sniffing
     * shares the single tri-state parser, so IPv6 ext-header DNS queries are sniffed
     * again (matrix row 4). Non-OK states have no readable UDP header: no sniffing,
     * the main policy path decides them via the chain. */
    struct l4_info l4 = {L4_PARSE_OK}; /* zero-fill: L4_PARSE_OK == 0, C++-safe first-member init */
    if (!parse_l4(family, skb, &l4) || l4.state != L4_PARSE_OK) {
        return SK_PASS;
    }
    protocol = l4.protocol;
    l4_nhoff = l4.l4_nhoff;

    if (protocol == IPPROTO_UDP) {
        __u16 src_port;
        __u16 dst_port;
        struct udphdr udph = {0};
        /* A header load failure aborts the sniffing — no partially
         * loaded DNS header reaches the parse functions. */
        if (bpf_skb_load_bytes(skb, l4_nhoff, &udph, sizeof(struct udphdr)) != 0) {
            return SK_PASS;
        }

        src_port = bpf_ntohs(udph.source);
        dst_port = bpf_ntohs(udph.dest);
        if (src_port == DNS_PORT || dst_port == DNS_PORT) {
            return sniff_dns_message(family, skb, dir, protocol, (__u16)(l4_nhoff + sizeof(struct udphdr)), udph.source,
                                     udph.dest, gate);
        }
        /* DoT (853), gate-only face: the payload is encrypted — no parse,
         * no record staging, no learning. Mark the gate ctx by port shape
         * so the policy-layer gate can release an authorized-server query
         * (spec: 53 or 853). Egress direction only (a query is addressed
         * TO the resolver port; a forged local "response", src 853, never
         * marks). The volatile store keeps the write opaque to the
         * verifier so the dns program's exit state stays single (same
         * rationale as the gate export inside parse_dns_query). */
        if (dst_port == DNS_OVER_TLS_PORT && dir == EGRESS && gate != NULL) {
            *(volatile __u8 *)&gate->is_query = 1;
        }
        return SK_PASS;
    }
    if (protocol == IPPROTO_TCP) {
        /* DNS-over-TCP — the message follows a 2-byte length
         * prefix after the (possibly option-bearing) TCP header, so the DNS
         * header sits at l4_nhoff + doff*4 + 2. The header is read as raw
         * bytes on purpose: the udph source/dest shim above (musl's BPF-side
         * member-name remap) would clobber any tcph .source/.dest access.
         * Doff comes from the real header; a malformed doff (or one
         * pushing the DNS header past the packet) aborts the sniffing
         * fail-closed — the policy chain still decides the packet. */
        __u8 tcph_raw[sizeof(struct tcphdr)] = {0};
        if (bpf_skb_load_bytes(skb, l4_nhoff, tcph_raw, sizeof(tcph_raw)) != 0) {
            return SK_PASS;
        }
        __u16 tcp_hdr_len = (__u16)((tcph_raw[12] >> 4) * 4); /* doff = high nibble of byte 12 */
        if (tcp_hdr_len < sizeof(struct tcphdr) || tcp_hdr_len > 60) {
            return SK_PASS;
        }

        /* memcpy load == the udph struct-load convention: the wire values are
         * passed to the sniffer in exactly the form a struct member read
         * would give (byte-identical keys on any endianness). */
        __be16 wire_sport = 0;
        __be16 wire_dport = 0;
        __builtin_memcpy(&wire_sport, tcph_raw, sizeof(wire_sport));
        __builtin_memcpy(&wire_dport, tcph_raw + 2, sizeof(wire_dport));
        if (bpf_ntohs(wire_sport) == DNS_PORT || bpf_ntohs(wire_dport) == DNS_PORT) {
            return sniff_dns_message(family, skb, dir, protocol, (__u16)(l4_nhoff + tcp_hdr_len + 2), wire_sport,
                                     wire_dport, gate);
        }
        /* DoT (853), gate-only face — see the UDP branch above. */
        if (bpf_ntohs(wire_dport) == DNS_OVER_TLS_PORT && dir == EGRESS && gate != NULL) {
            *(volatile __u8 *)&gate->is_query = 1;
        }
        return SK_PASS;
    }
    return SK_PASS;
}

#endif // NET_FIREWALL_DOMAIN_H