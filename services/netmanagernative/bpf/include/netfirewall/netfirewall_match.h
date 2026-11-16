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
#ifndef NET_FIREWALL_MATCH_H
#define NET_FIREWALL_MATCH_H

/* User-space test/fuzz TUs include this chain without libbpf's helper
 * declarations: they provide their own stubs before the include instead
 * (same pattern as netfirewall_utils.h). The real BPF build defines __BPF__. */
#ifdef __BPF__
#include <bpf_helpers.h>
#endif

#include "netfirewall_utils.h"
#include "netfirewall_bitmap.h"
#include "netfirewall_map.h"
#include "netfirewall_event.h"
#include "netfirewall_def.h"

/**
 * @brief Get the user id from sock_uid
 *
 * @param sock_uid bpf_get_socket_uid
 * @return user id with type __u32
 */
static __always_inline __u32 get_user_id(__u32 sock_uid)
{
    if (sock_uid == 0xFFFFFFFFu) {
        /* No socket attribution: never fold to 21473 (a theoretically valid userId);
         * carried to DEFAULT_ACTION_MAP lookup as UID_UNATTRIBUTED. */
        return UID_UNATTRIBUTED;
    }
    __u32 user_id = sock_uid / USER_ID_DIVIDOR;
    if (user_id > 0) {
        return user_id;
    }
    /* Real system-identity socket (sock_uid < USER_ID_DIVIDOR: system apps'
     * appId, root, shell, system daemons) folds to the foreground user via
     * CURRENT_UID_MAP — the foreground user's rules and default actions
     * govern system identities too (factory spec; F6 restores it). Map miss
     * -> DEFAULT_USER_ID. The map is written by user space
     * (SetFirewallCurrentUserId) on foreground user switch. */
    current_user_id_key key = CURRENT_USER_ID_KEY;
    uid_key *current_user_id = bpf_map_lookup_elem(&CURRENT_UID_MAP, &key);
    if (current_user_id == NULL) {
        return DEFAULT_USER_ID;
    }
    return *current_user_id;
}

/**
 * @brief swap tuple ports at egress direction
 *
 * @param tuple struct match_tuple
 */
static __always_inline void swap_tuple_ports(struct match_tuple *tuple)
{
    __be16 tmp = tuple->sport;
    tuple->sport = tuple->dport;
    tuple->dport = tmp;
}

/**
 * @brief swap tuple addrs at egress direction
 *
 * @param tuple struct match_tuple
 */
static __always_inline void swap_tuple_addrs(struct match_tuple *tuple)
{
    if (tuple->family == AF_INET) {
        __be32 tmp = tuple->ipv4.saddr;
        tuple->ipv4.saddr = tuple->ipv4.daddr;
        tuple->ipv4.daddr = tmp;
    } else {
        struct in6_addr tmp = tuple->ipv6.saddr;
        tuple->ipv6.saddr = tuple->ipv6.daddr;
        tuple->ipv6.daddr = tmp;
    }
}

/**
 * @brief Get the match tuple from skb
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff of packet
 * @param tuple struct match_tuple
 * @param dir enum stream_dir
 * @return true if success or false if an error occurred
 */
static __always_inline bool get_match_tuple(__u32 family, struct __sk_buff *skb, struct match_tuple *tuple,
    enum stream_dir dir)
{
    if (!skb || !tuple) {
        return false;
    }

    __u32 l3_nhoff = get_l3_nhoff(skb);
    if (is_ipv4_format(family, skb)) {
        load_l3_v4_addrs(family, skb, l3_nhoff, &(tuple->ipv4.saddr), &(tuple->ipv4.daddr));
    } else {
        load_l3_v6_addrs(family, skb, l3_nhoff, &(tuple->ipv6.saddr), &(tuple->ipv6.daddr));
    }
    /* Single tri-state parser — l4_nhoff follows IHL / the IPv6 extension header
     * chain, so ports/RST are read from the real L4 header instead of attacker-
     * controlled option bytes. NO_L4/UNPARSEABLE still builds the tuple (family/
     * addrs/uid/protocol are unaffected) with l4_state carried for the chain. */
    struct l4_info l4 = { L4_PARSE_OK }; /* zero-fill: L4_PARSE_OK == 0, C++-safe first-member init */
    if (!parse_l4(family, skb, &l4)) {
        return false;
    }
    tuple->dir = dir;
    tuple->family = family;
    if (tuple->family != AF_INET && is_ipv4_format(family, skb)) {
        tuple->family = AF_INET;
    }
    __u32 sock_uid = bpf_get_socket_uid(skb);
    tuple->appuid = sock_uid;
    tuple->uid = get_user_id(sock_uid);
    /* resolved as NOT implementable: active receiver-side attribution
     * via bpf_sk_lookup_tcp/udp cannot produce a uid — struct bpf_sock exposes no
     * uid field (libbpf vendored UAPI and kernel 6.6/5.10 prebuilt UAPIs all lack
     * it) and no helper maps a looked-up socket back to its owner. Unattributed
     * ingress keeps UID_UNATTRIBUTED and is decided by the reserved
     * DEFAULT_ACTION_MAP[UID_UNATTRIBUTED] fallback plus the
     * uid-dimension wildcard rules installed by the rule sync. Degraded form of the ideal design. */
    tuple->l4_state = l4.state;
    tuple->protocol = l4.protocol;

    if (l4.state == L4_PARSE_OK && (l4.protocol == IPPROTO_TCP || l4.protocol == IPPROTO_UDP)) {
        load_l4_ports(skb, l4.l4_nhoff, l4.protocol, &(tuple->sport), &(tuple->dport));
        if (l4.protocol == IPPROTO_TCP) {
            /* RST flags come from the real header — the forged-RST-via-options
             * vector is eliminated (the real-RST bypass itself is a separate concern). */
            load_l4_header_flags(skb, l4.l4_nhoff, &(tuple->rst));
        }
    }
    if (dir == EGRESS) {
        swap_tuple_addrs(tuple);
        swap_tuple_ports(tuple);
    }
    tuple->ifindex = skb->ifindex;
    return true;
}

/**
 * @brief lookup key from the dimension map, falling back to the direction's wildcard
 * map keyed by dimension role. Both misses fall back to the all-zero bitmap so the
 * dimension kills every rule bit (fail-closed) instead of being skipped.
 *
 * @param map dimension bpf map pointer
 * @param key exact key built from the packet
 * @param wildcard_map direction's wildcard map (in_wildcard_map / out_wildcard_map)
 * @param role dimension role (wildcard_role) used as the wildcard map key
 * @return value with type struct bitmap of the key, wildcard entry, or zero bitmap
 */
static __always_inline struct bitmap *lookup_dim_map(void *map, void *key,
                                                     void *wildcard_map, wildcard_key role)
{
    struct bitmap *result = bpf_map_lookup_elem(map, key);
    if (!result) {
        result = bpf_map_lookup_elem(wildcard_map, &role);
    }
    if (!result) {
        __u32 zero_key = 0;
        result = bpf_map_lookup_elem(&ZERO_BITMAP_MAP, &zero_key);
    }
    return result;
}

/**
 * @brief match packet is loopback or not
 *
 * @param match_tpl struct match_tuple
 * @return true is loopback packet or false if not
 */
static __always_inline bool match_loopback(struct match_tuple match_tpl)
{
    bool is_loopback = false;
    if (match_tpl.protocol == PROTOCOL_SAT_EXPAK && match_tpl.ifindex == 1) {
        is_loopback = true;
    } else {
        loop_back_val *result = NULL;
        if (match_tpl.family == AF_INET) {
            // ipv4 127.0.0.1
            struct ipv4_lpm_key lpm_key = {
                .prefixlen = IPV4_MAX_PREFIXLEN,
                .data = match_tpl.ipv4.saddr,
            };
            result = bpf_map_lookup_elem(&LOOP_BACK_IPV4_MAP, &lpm_key);
            if (result != NULL) {
                lpm_key.data = match_tpl.ipv4.daddr;
                result = bpf_map_lookup_elem(&LOOP_BACK_IPV4_MAP, &lpm_key);
            }
        } else {
            // ipv6 ::1/128
            struct ipv6_lpm_key lpm_key = {
                .prefixlen = IPV6_MAX_PREFIXLEN,
            };
            memcpy(&(lpm_key.data), &(match_tpl.ipv6.saddr), sizeof(lpm_key.data));
            result = bpf_map_lookup_elem(&LOOP_BACK_IPV6_MAP, &lpm_key);
            if (result != NULL) {
                memcpy(&(lpm_key.data), &(match_tpl.ipv6.daddr), sizeof(lpm_key.data));
                result = bpf_map_lookup_elem(&LOOP_BACK_IPV6_MAP, &lpm_key);
            }
        }
        if (result != NULL) {
            is_loopback = true;
        }
    }
    return is_loopback;
}

/**
 * @brief lookup addr bitmap use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @return true if success or false if an error occurred
 */
static __always_inline bool match_addrs(struct match_tuple *tuple, struct bitmap *key)
{
    if (!tuple || !key) {
        return false;
    }

    struct bitmap *result = NULL;
    bool ingress = tuple->dir == INGRESS;

    if (tuple->family == AF_INET) {
        struct ipv4_lpm_key lpm_key = {
            .prefixlen = 32,
            .data = tuple->ipv4.saddr,
        };

        result = lookup_dim_map(GET_MAP(ingress, saddr), &lpm_key, GET_MAP(ingress, wildcard),
                                WILDCARD_SRC_ADDR);
        if (result) {
            bitmap_and(key->val, result->val);
            result = NULL;
        }

        lpm_key.data = tuple->ipv4.daddr;
        result = lookup_dim_map(GET_MAP(ingress, daddr), &lpm_key, GET_MAP(ingress, wildcard),
                                WILDCARD_DST_ADDR);
        if (result) {
            bitmap_and(key->val, result->val);
        }
    } else {
        struct ipv6_lpm_key lpm_key = {
            .prefixlen = 128,
        };

        memcpy(&(lpm_key.data), &(tuple->ipv6.saddr), sizeof(lpm_key.data));
        result = lookup_dim_map(GET_MAP(ingress, saddr6), &lpm_key, GET_MAP(ingress, wildcard),
                                WILDCARD_SRC_ADDR);
        if (result) {
            bitmap_and(key->val, result->val);
            result = NULL;
        }

        memcpy(&(lpm_key.data), &(tuple->ipv6.daddr), sizeof(lpm_key.data));
        result = lookup_dim_map(GET_MAP(ingress, daddr6), &lpm_key, GET_MAP(ingress, wildcard),
                                WILDCARD_DST_ADDR);
        if (result) {
            bitmap_and(key->val, result->val);
        }
    }
    return true;
}

/**
 * @brief bitmap use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @return true if success or false if an error occurred
 */
static __always_inline bool match_ports(struct match_tuple *tuple, struct bitmap *key)
{
    if (!tuple || !key) {
        return false;
    }
    struct port_key exact_key;
    exact_key.data = tuple->sport;
    exact_key.prefixlen = PORT_MAX_MASK;

    bool ingress = tuple->dir == INGRESS;
    struct bitmap *result = NULL;

    result = lookup_dim_map(GET_MAP(ingress, sport), &exact_key, GET_MAP(ingress, wildcard),
                            WILDCARD_SPORT);
    if (result) {
        log_dbg2(DBG_MATCH_SPORT, tuple->dir, (__u32)tuple->sport, result->val[0]);
        bitmap_and(key->val, result->val);
        result = NULL;
    }
    exact_key.data = tuple->dport;
    exact_key.prefixlen = PORT_MAX_MASK;
    result = lookup_dim_map(GET_MAP(ingress, dport), &exact_key, GET_MAP(ingress, wildcard),
                            WILDCARD_DPORT);
    if (result) {
        log_dbg2(DBG_MATCH_DPORT, tuple->dir, (__u32)tuple->dport, result->val[0]);
        bitmap_and(key->val, result->val);
    }
    return true;
}

/**
 * @brief lookup protocol bitmap use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @return true if success or false if an error occurred
 */
static __always_inline bool match_protocol(struct match_tuple *tuple, struct bitmap *key)
{
    if (!tuple || !key) {
        return false;
    }

    bool ingress = tuple->dir == INGRESS;
    struct bitmap *result = NULL;

    result = lookup_dim_map(GET_MAP(ingress, proto), &(tuple->protocol), GET_MAP(ingress, wildcard),
                            WILDCARD_PROTO);
    if (result) {
        log_dbg2(DBG_MATCH_PROTO, tuple->dir, (__u32)tuple->protocol, result->val[0]);
        bitmap_and(key->val, result->val);
    }

    return true;
}

/**
 * @brief lookup appuid bitmap use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @return true if success or false if an error occurred
 */
static __always_inline bool match_appuid(struct match_tuple *tuple, struct bitmap *key)
{
    if (!tuple || !key) {
        return false;
    }

    bool ingress = tuple->dir == INGRESS;
    struct bitmap *result = NULL;

    result = lookup_dim_map(GET_MAP(ingress, appuid), &(tuple->appuid), GET_MAP(ingress, wildcard),
                            WILDCARD_APPUID);
    if (result) {
        log_dbg2(DBG_MATCH_APPUID, tuple->dir, tuple->appuid, result->val[0]);
        bitmap_and(key->val, result->val);
    }

    return true;
}

/**
 * @brief lookup user_id bitmap use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @return true if success or false if an error occurred
 */
static __always_inline bool match_uid(struct match_tuple *tuple, struct bitmap *key)
{
    if (!tuple || !key) {
        return false;
    }

    bool ingress = tuple->dir == INGRESS;
    struct bitmap *result = NULL;

    result = lookup_dim_map(GET_MAP(ingress, uid), &(tuple->uid), GET_MAP(ingress, wildcard),
                            WILDCARD_UID);
    if (result) {
        log_dbg2(DBG_MATCH_UID, tuple->dir, tuple->uid, result->val[0]);
        bitmap_and(key->val, result->val);
    }
    return true;
}

/**
 * @brief lookup interface bitmap use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @return true if success or false if an error occurred
 */
static __always_inline bool match_interface(struct __sk_buff *skb, struct match_tuple *tuple, struct bitmap *key)
{
    if (!skb || !tuple || !key) {
        return false;
    }

    bool ingress = tuple->dir == INGRESS;
    interface_key iface_key = {};
    long ret = bpf_skb_get_dev_name(skb, iface_key.name, sizeof(iface_key.name));
    if (ret < 0) {
        log_dbg(DBG_MATCH_INTERFACE, tuple->dir, ret);
        return false;
    }
    struct bitmap *result = lookup_dim_map(GET_MAP(ingress, iface), &iface_key, GET_MAP(ingress, wildcard),
                                           WILDCARD_IFACE);
    if (result) {
        log_dbg2(DBG_MATCH_INTERFACE, tuple->dir, iface_key.name, result->val[0]);
        bitmap_and(key->val, result->val);
    }

    return true;
}

/**
 * @brief lookup action key bitmap use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @return true if success or false if an error occurred
 */
/* noinline (bpf-to-bpf subprogram): see match_action below — the rule
 * bitmap assembly is re-walked per parent state when inlined. */
static __attribute__((noinline)) bool match_action_key(struct __sk_buff *skb, struct match_tuple *tuple, struct bitmap *key)
{
    if (!skb || !tuple || !key) {
        return false;
    }

    memset(key, 0xff, sizeof(struct bitmap));

    if (!match_addrs(tuple, key)) {
        return false;
    }

    if (!match_protocol(tuple, key)) {
        return false;
    }

    if (!match_ports(tuple, key)) {
        return false;
    }

    if (!match_appuid(tuple, key)) {
        return false;
    }

    if (!match_uid(tuple, key)) {
        return false;
    }

    if (!match_interface(skb, tuple, key)) {
        return false;
    }

    log_dbg(DBG_ACTION_KEY, tuple->dir, key->val[0]);
    return true;
}

static __always_inline bool MatchDomain(const struct match_tuple *tuple)
{
    if (!tuple) {
        return false;
    }
    struct domain_value *result = NULL;
    if (tuple->family == AF_INET) {
        struct ipv4_lpm_key key = {
            .prefixlen = IPV4_MAX_PREFIXLEN,
            .data = tuple->ipv4.saddr,
        };
        result = bpf_map_lookup_elem(&DOMAIN_IPV4_MAP, &key);
    } else {
        struct ipv6_lpm_key key = {
            .prefixlen = IPV6_MAX_PREFIXLEN,
            .data = tuple->ipv6.saddr,
        };
        result = bpf_map_lookup_elem(&DOMAIN_IPV6_MAP, &key);
    }
    bool matchAction = false;
    if (result != NULL && tuple->uid == result->uid &&
        (tuple->appuid == result->appuid || result->appuid == 0)) {
        /* First-layer aging: a learned entry past its TTL is treated
         * as a miss. The entry is NOT deleted here (capacity reclaim belongs
         * to the user-space aging task); if that task is ever stuck, expired
         * entries only occupy capacity and never authorize traffic
         * (fail-safe). Cost: one ktime call only on the uid/appuid-hit path. */
        if (domain_entry_expired(bpf_ktime_get_ns(), result->timestamp, result->ttl) == 0) {
            matchAction = true;
        }
    } else {
        matchAction = false;
    }
    return matchAction;
}

/**
 * @brief The DNS server authorization lookup —
 *        {uid, server ip} must exist in DNS_SERVER_V4/V6_MAP (control-plane
 * written). "Server ip" is the DNS-speaking endpoint side: the
 *        egress query tuple's saddr (post-swap) or the ingress response
 *        flow's saddr. Presence alone means authorization — the map value's
 *        managed byte is reserved for future flags.
 *
 * @param uid userId of the rule owner (query originator)
 * @param family AF_INET / AF_INET6
 * @param saddr4 server ipv4 (network order) — read only when family == AF_INET
 * @param saddr6 server ipv6 — read only when family == AF_INET6
 * @return true on a map hit
 */
static __always_inline bool MatchDnsServer(__u32 uid, __u32 family, __be32 saddr4, const struct in6_addr *saddr6)
{
    if (family == AF_INET) {
        struct dns_server_v4_key key = { 0 };
        key.uid = uid;
        key.ip = saddr4;
        /* typed-pointer assignment first: StubBpfValue (the C++ test/fuzz
         * proxy) has no comparison operators — the BPF build is unaffected. */
        struct dns_server_val *hit = bpf_map_lookup_elem(&DNS_SERVER_V4_MAP, &key);
        return hit != NULL;
    }
    if (family == AF_INET6 && saddr6 != NULL) {
        struct dns_server_v6_key key = { 0 }; /* zero-init first: no garbage bytes in a hash key */
        key.uid = uid;
        __builtin_memcpy(&key.ip, saddr6, sizeof(key.ip));
        struct dns_server_val *hit = bpf_map_lookup_elem(&DNS_SERVER_V6_MAP, &key);
        return hit != NULL;
    }
    return false;
}

/**
 * @brief lookup action with action_key use the given tuple
 *
 * @param tuple struct match_tuple get from skb
 * @param key out param for lookup result
 * @param gate per-domain gate context (egress queries; NULL when the
 *             caller has none — ingress). Consumed only inside the no-rule
 *             branch.
 * @return FW_PASS/FW_DROP on success; FW_EVAL_ERROR on null args (handed to eval_error)
 */
/* noinline (bpf-to-bpf subprogram): the verifier re-walks this whole
 * chain once per distinct parent state that reaches it (~28 structural
 * parents: family x protocol x ct x sniff continuations), and each walk
 * costs ~300 processed insns of unrolled bitmap work — that product is
 * what exhausts the egress complexity budget. As a subprogram the chain
 * is entered through fixed argument registers and explored once per
 * entry state instead of being inlined into every caller path. */
static __attribute__((noinline)) enum fw_eval_result match_action(struct match_tuple *tuple, struct bitmap *key,
    const struct dns_gate_ctx *gate)
{
    if (!tuple || !key) {
        return FW_EVAL_ERROR;
    }
    bool ingress = tuple->dir == INGRESS;
    /* UID_UNATTRIBUTED (0xFFFFFFFF) is a normal
     * input here, not an evaluation failure — the plain lookup below already
     * lands on the reserved DEFAULT_ACTION_MAP[UID_UNATTRIBUTED] entry when the
     * uid is the sentinel, and a miss keeps SK_PASS (unconfigured users are not
     * killed). The uid-dimension bitmap miss above already fell back to the
     * wildcard map, so uid-unconstrained rules still apply to unattributed
     * traffic. SYSTEM (FIREWALL_SYSTEM_USER_ID) joins the same path.
     * No dedicated branch is needed for either. */
    struct defalut_action_value *default_value = bpf_map_lookup_elem(&DEFAULT_ACTION_MAP, &tuple->uid);
    enum sk_action sk_act = SK_PASS;
    if (default_value) {
        sk_act = ingress ? default_value->inaction : default_value->outaction;
    }

    action_key akey = 1;
    struct bitmap *action_bitmap = bpf_map_lookup_elem(GET_MAP(ingress, action), &akey);
    /*
     * Conflict & Repetition Algorithm
     * eg: matched 0110, action 1100 : 1:drop 0:pass
     * 1 default drop: Match the rule with the action's bitmap bit by and, and if any bit is 1, it is drop
     * (0110&1100->0100)
     * 2 default pass: 2.1 Reverse the action, 0011(1:pass, 0:drop) 2.2 Match results bit by and, and if
     * any bit is 1, it is pass(0110&0011->0010)
     */
    if (action_bitmap && bitmap_positive(key->val)) {
        if (sk_act == SK_DROP) {
            bitmap_and(key->val, action_bitmap->val);
            if (!bitmap_positive(key->val) || MatchDomain(tuple)) {
                sk_act = SK_PASS;
            }
        } else {
            bitmap_and_inv(key->val, action_bitmap->val);
            if (!bitmap_positive(key->val) && !MatchDomain(tuple)) {
                sk_act = SK_DROP;
            }
        }
        // If the outbound does not match the IP rule, check if there are any domain name rules
    } else if (MatchDomain(tuple)) {
        log_dbg(DBG_MATCH_DOMAIN, tuple->dir, sk_act);
        sk_act = SK_PASS;
        /* Query-layer server-authorization gate — a same-layer fallback INSIDE
         * the no-rule branch. Rule fidelity is free: an explicit 53/853 port rule
         * is decided by the conflict algorithm in the rule-hit branch above and
         * can never reach this gate; the gate only rewrites a DEFAULT-DROP
         * verdict. Conditions (all must hold): ① default action DROP (no rule
         * hit), ② egress UDP/TCP with the destination port 53 (plaintext DNS)
         * or 853 (DoT) — the egress tuple is port-swapped, so the destination
         * port is sport, ③ DNS_SERVER map hit {uid, server ip} — the tunnel
         * face stays closed: only configured servers are gated open, never any
         * :53/:853, ④ for port 53: the sniffer verified the qname hits this
         * uid's domain allow rule (gate->allowrst — a deny or unrelated qname
         * fails the gate and the query is dropped by the default action with
         * the F5 fast-fail). Port 853 is deliberately exempt from ④: the DoT
         * payload is encrypted, the qname can never be sniffed or verified —
         * server authorization is the only check physically possible (spec
         * 2026-11-16). A gate pass keeps walking the normal ct-create +
         * record-flush path (netfirewall.h). */
    } else if (gate != NULL && gate->is_query == 1 && sk_act == SK_DROP && tuple->dir == EGRESS &&
               (tuple->protocol == IPPROTO_UDP || tuple->protocol == IPPROTO_TCP) &&
               ((bpf_htons(tuple->sport) == DNS_PORT && gate->allowrst == 1) ||
                bpf_htons(tuple->sport) == DNS_OVER_TLS_PORT) &&
               MatchDnsServer(tuple->uid, tuple->family, tuple->ipv4.saddr, &tuple->ipv6.saddr)) {
        log_dbg(DBG_MATCH_ACTION, tuple->dir, SK_PASS);
        sk_act = SK_PASS;
    }
    log_dbg(DBG_MATCH_ACTION, tuple->dir, sk_act);
    return (sk_act == SK_PASS) ? FW_PASS : FW_DROP;
}

/**
 * @brief unified fallback for evaluation failures: salvaged uid -> throttled fault
 *        report -> mode switch -> per-uid default action (entry miss = PASS, preserved
 *        design semantics for unconfigured users)
 *
 * @param skb struct __sk_buff (used for uid salvage when tuple is NULL)
 * @param tuple struct match_tuple (NULL when extraction failed, e.g. FAIL_TUPLE)
 * @param dir enum stream_dir (explicit parameter: tuple may be NULL)
 * @param fail_point enum eval_fail_point
 * @return SK_DROP or SK_PASS per the default action of the salvaged uid
 */
static __always_inline enum sk_action eval_error(struct __sk_buff *skb, const struct match_tuple *tuple,
    enum stream_dir dir, enum eval_fail_point fail_point)
{
    /* 1) uid salvage: prefer tuple (always populated on build-success paths); else take
     *    from socket again, independent of packet parsing (0xFFFFFFFF -> UID_UNATTRIBUTED) */
    __u32 uid = UID_UNATTRIBUTED;
    if (tuple) {
        uid = tuple->uid;
    } else if (skb) {
        uid = get_user_id(bpf_get_socket_uid(skb));
    }

    /* 2) mode switch: OBSERVE = legacy pass-through (escape hatch);
     *    no entry = FAIL_CLOSED (secure default) */
    __u32 zero = 0;
    __u32 *mode = bpf_map_lookup_elem(&EVAL_FALLBACK_MODE_MAP, &zero);
    if (mode != NULL && *mode == EVAL_FALLBACK_OBSERVE) {
        return SK_PASS;
    }

    /* 3) fail-closed fallback: per-uid default action; no entry = unconfigured user =
     *    PASS (single lookup only, no nested error handling).
     *    The fields are kernel sk_action values (the writer side only stores
     *    SK_DROP/SK_PASS), but the verifier cannot see that invariant through
     *    a raw map read: a cgroup_skb program's R0 must be provably in {0, 1}
     *    at exit, and returning the unmasked u32 makes BPF_PROG_LOAD fail with
     *    EINVAL ("At program exit the register R0 ... should have been in
     *    (0x0; 0x1)"). Collapse the value explicitly; anything but SK_PASS
     *    (i.e. a corrupted entry) fail-closes to DROP. */
    struct defalut_action_value *default_value = bpf_map_lookup_elem(&DEFAULT_ACTION_MAP, &uid);
    if (default_value == NULL) {
        return SK_PASS;
    }
    enum sk_action fallback = (dir == INGRESS) ? default_value->inaction : default_value->outaction;
    return (fallback == SK_PASS) ? SK_PASS : SK_DROP;
}
#endif // NET_FIREWALL_MATCH_H
