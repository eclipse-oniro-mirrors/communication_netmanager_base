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
#ifndef NET_FIREWALL_H
#define NET_FIREWALL_H

#include <linux/bpf.h>

#include "netfirewall_ct.h"
#include "netfirewall_def.h"
#include "netfirewall_domain.h"
#include "netfirewall_event.h"
#include "netfirewall_match.h"

/**
 * @brief if tcp socket was intercepted, need send reset packet to peer
 *
 * @param tuple match tuple of skb meta data
 * @param skb struct __sk_buff
 * @param dir enum stream_dir
 * @return 0 if no error, -1 if an error occurred
 */
static __always_inline int send_sock_tcp_reset(struct match_tuple *tuple, struct __sk_buff *skb, enum stream_dir dir)
{
    if (!skb || !tuple) {
        return -1;
    }
    if (tuple->protocol == IPPROTO_TCP) {
        if (dir == INGRESS) {
            bpf_sock_tcp_send_reset(skb);
        }
        return 0;
    }
    return -1;
}

/**
 * @brief Get the packet rst on tuple
 *
 * @param tuple struct match_tuple
 * @return true if success or false if an error occurred
 */
static __always_inline bool get_packet_rst_flag(struct match_tuple *tuple)
{
    if (!tuple) {
        return false;
    }

    if (tuple->rst == 1) {
        return true;
    }

    return false;
}

/**
 * @brief Get the ct tuple from match tuple
 *
 * @param match_tpl struct match_tuple
 * @param ct_tpl struct ct_tuple
 * @return true if success or false if an error occurred
 */
static __always_inline bool get_ct_tuple(struct match_tuple *match_tpl, struct ct_tuple *ct_tpl)
{
    if (!match_tpl || !ct_tpl) {
        return false;
    }

    ct_tpl->uid = match_tpl->uid;
    ct_tpl->family = match_tpl->family;
    ct_tpl->protocol = match_tpl->protocol;
    ct_tpl->sport = match_tpl->sport;
    ct_tpl->dport = match_tpl->dport;

    if (match_tpl->family == AF_INET) {
        ct_tpl->ipv4.saddr = match_tpl->ipv4.saddr;
        ct_tpl->ipv4.daddr = match_tpl->ipv4.daddr;
    } else {
        ct_tpl->ipv6.saddr = match_tpl->ipv6.saddr;
        ct_tpl->ipv6.daddr = match_tpl->ipv6.daddr;
    }

    return true;
}

/**
 * @brief verdict chain for packets whose L4 header could not be located
 *        (non-first fragment / ESP / malformed / overlong ext chain): such packets
 *        never enter the 7-dimension rule AND (that would evaluate misread values)
 *        and never skip dimensions (that would make port-constrained rules
 *        falsely hit). Chain: ct lookup -> throttled report -> per-uid default
 *        action -> MatchDomain escape under default deny.
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff
 * @param tuple struct match_tuple (family/addrs/uid/protocol valid; ports are zero)
 * @param dir enum stream_dir
 * @return SK_DROP or SK_PASS
 */
static __always_inline enum sk_action handle_unparseable(__u32 family, struct __sk_buff *skb,
    struct match_tuple *tuple, enum stream_dir dir)
{
    /* 1) ct: a hit means this weak-key flow already carries a PASS verdict. The
     * zero-port key cannot collide with real-port entries (different key by
     * construction), so in practice this serves symmetric loopback re-delivery
     * and port-0 flows; complete fragment verdict chaining is the first-fragment
     * decision cache. Unparseable packets never create ct entries (the ct is
     * query-only for such flows).
     * The evaluated bit is deliberately NOT consulted here (unlike the early
     * exit in the OK path): unparseable packets never enter rule evaluation by
     * construction, so no evaluated-flag asymmetry exists; a ct hit IS the
     * verdict carried from the fully evaluated counterpart packet
     * (revisit with the ct rework). */
    struct ct_tuple ct_tpl = {};
    if (get_ct_tuple(tuple, &ct_tpl)) {
        struct ct_entry *cte = NULL;
        enum ct_status status =
            ct_map_lookup_entry(family, skb, &ct_tpl, (dir == INGRESS) ? CT_INGRESS : CT_EGRESS, match_loopback(*tuple), &cte);
        if (status != CT_NEW && cte != NULL) {
            return SK_PASS;
        }
    }

    /* 2) default action; no entry = unconfigured user = PASS */
    struct defalut_action_value *default_value = bpf_map_lookup_elem(&DEFAULT_ACTION_MAP, &tuple->uid);
    if (default_value == NULL) {
        return SK_PASS;
    }
    enum sk_action act = (dir == INGRESS) ? default_value->inaction : default_value->outaction;
    if (act == SK_PASS) {
        return SK_PASS;
    }

    /* 4) default deny: MatchDomain escape — the IP dimension is unaffected by the
     * L4 parse, so learned IPs of domain-allowed connections still pass. */
    if (MatchDomain(tuple)) {
        return SK_PASS;
    }
    log_intercept(tuple);
    return SK_DROP;
}

/* DNS-sniff DROP path, shared by the dns programs (netsys.c): parses
 * the tuple for the intercept event exactly like the pre-split hook did.
 * Unified with the main path: default-deny users are still dropped,
 * default-allow users are relaxed from DROP to default action (behavior
 * matrix row 9, review-visible change).
 * F5 fast-fail, sniff side (EGRESS only): a deny-qname egress query dies
 * HERE — destroy the querying socket in-kernel for an ms-level
 * ECONNABORTED to the owner instead of the seconds-long resolver backoff.
 * Idempotent with the user-space netlink destroy (DestroySocketByTuple,
 * reached via the same intercept event logged below — the user-space side
 * filters dir != EGRESS identically): whichever lands first wins, the
 * second finds the socket gone. INGRESS drops (a deny-qname response
 * arriving) deliberately spare the socket: it may still be serving other
 * in-flight queries on the same resolver socket. */
static __always_inline enum sk_action dns_sniff_drop(__u32 family, struct __sk_buff *skb,
    enum stream_dir dir)
{
    struct match_tuple tuple = { (enum stream_dir)0 };
    if (!get_match_tuple(family, skb, &tuple, dir)) {
        return eval_error(skb, NULL, dir, FAIL_TUPLE);
    }
    log_intercept(&tuple);
    if (dir == EGRESS) {
        (void)bpf_skb_sock_destroy(skb);
    }
    return SK_DROP;
}

/**
 * @brief Ingress policy chain for the attached main program.
 *
 * Runs inline in the attached main program (the second attachment after
 * the dns program — see netsys.c): the dual-attach split gives the dns
 * program (DNS chain, ~13.5k verifier insns measured) and this policy
 * chain (174k ingress / 253k egress measured) each their own verifier
 * budget instead of the shared 1M limit that the monolithic program
 * blew. The DNS sniff itself lives in the dns program (netsys.c); no
 * gate context on ingress: the per-domain gate is an EGRESS query
 * fallback, ingress responses ride the ct + record validation.
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff
 * @return SK_DROP if intercepted or SK_PASS if not
 */
static __always_inline enum sk_action fw_policy_after_ingress(__u32 family, struct __sk_buff *skb)
{
    /* { (enum stream_dir)0 } keeps the whole-struct zero-init valid in BOTH
     * the BPF C build and the host C++17 test TU (plain { 0 } narrows int →
     * enum as a hard error in C++ list-initialization). */
    struct match_tuple tuple = { (enum stream_dir)0 };
    if (!get_match_tuple(family, skb, &tuple, INGRESS)) {
        return eval_error(skb, NULL, INGRESS, FAIL_TUPLE);
    }

    log_tuple(&tuple);

    if (tuple.l4_state != L4_PARSE_OK) {
        /* Non-first fragment / ESP / malformed — decided by the
         * ct -> default action -> MatchDomain chain, never by misread ports. */
        return handle_unparseable(family, skb, &tuple, INGRESS);
    }

    struct ct_tuple ct_tpl = {};
    if (!get_ct_tuple(&tuple, &ct_tpl)) {
        return eval_error(skb, &tuple, INGRESS, FAIL_CT_TUPLE);
    }

    struct ct_entry *cte = NULL;
    bool loopback = match_loopback(tuple);
    enum ct_status status = ct_map_lookup_entry(family, skb, &ct_tpl, CT_INGRESS, loopback, &cte);
    log_dbg(DBG_CT_LOOKUP, INGRESS, status);
    /* Theoretical, declared assumption: same LRU-lifetime assumption as
     * the egress path — cte is only written at ct_set_evaluated below, after
     * the bounded scratch/action-key lookups. */
    if (status != CT_NEW && cte != NULL) {
        if (ct_evaluated(cte, CT_INGRESS)) {
            /* Steady state — this direction already passed the full evaluation
             * (per-direction once; the receiver's inaction is evaluated on the
             * first ingress packet even for a loopback ct created by egress). */
            return SK_PASS;
        }
        /* Return-path inheritance (non-loopback only): the entry was created
         * by an opposite-direction packet that passed its FULL evaluation
         * (ct_create_entry only runs on the PASS path), so the return traffic
         * of an allowed connection must not be re-adjudicated against this
         * direction's rules — restores the pre-P4 stateful exemption.
         * Without it, "inbound default deny + outbound-only allow" drops
         * every SYN-ACK of an allowed connection (the handshake never
         * completes; measured: browser preconnect retried until
         * CONNECTION_ABORTED). Loopback stays per-direction: the receiver's
         * inaction is still evaluated on the first ingress packet of a
         * loopback ct created by egress (P4 cross-space isolation). */
        if (!loopback && ct_evaluated(cte, CT_EGRESS)) {
            return SK_PASS;
        }
    }

    if (get_packet_rst_flag(&tuple)) {
        return SK_PASS;
    }

    __u32 nf_scratch_key = 0;
    struct bitmap *key = bpf_map_lookup_elem(&NF_BITMAP_SCRATCH, &nf_scratch_key);
    if (key == NULL) {
        return eval_error(skb, &tuple, INGRESS, FAIL_SCRATCH);
    }
    __builtin_memset(key, 0, sizeof(*key));
    if (!match_action_key(skb, &tuple, key)) {
        /* Includes dev_name failure: must NOT be treated as "no iface constraint",
         * which would bypass iface deny rules. */
        return eval_error(skb, &tuple, INGRESS, FAIL_ACTION_KEY);
    }

    enum fw_eval_result act = match_action(&tuple, key, NULL);
    if (act == FW_EVAL_ERROR) {
        return eval_error(skb, &tuple, INGRESS, FAIL_ACTION_ARGS);
    }
    if (act == FW_DROP) {
        log_intercept(&tuple);
        send_sock_tcp_reset(&tuple, skb, INGRESS);
        return SK_DROP;
    }

    if (status == CT_NEW) {
        /* Ct_create_entry sets ingress_evaluated (this packet just passed). */
        ct_create_entry(&ct_tpl, skb, CT_INGRESS);
    } else if (cte != NULL) {
        /* First packet of this direction passed the full evaluation. */
        ct_set_evaluated(cte, CT_INGRESS);
    }

    return SK_PASS;
}

/**
 * @brief Egress policy chain for the attached main program.
 *
 * Same split rationale as fw_policy_after_ingress. The per-domain gate
 * moved with the DNS program: the egress dns program (attached first)
 * fills a gate ctx in the per-CPU scratch for an egress DNS query and
 * match_action's no-rule branch consumes it together with the DNS_SERVER
 * map hit; NULL keeps the gate closed (scratch lookup failure — a stale
 * per-CPU slot must never leak across packets).
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff
 * @param gate per-domain gate ctx staged by the egress dns program, or NULL
 * @return SK_DROP if intercepted or SK_PASS if not
 */
static __always_inline enum sk_action fw_policy_after_egress(__u32 family, struct __sk_buff *skb,
    const struct dns_gate_ctx *gate)
{
    /* { (enum stream_dir)0 } keeps the whole-struct zero-init valid in BOTH
     * the BPF C build and the host C++17 test TU (plain { 0 } narrows int →
     * enum as a hard error in C++ list-initialization). */
    struct match_tuple tuple = { (enum stream_dir)0 };
    if (!get_match_tuple(family, skb, &tuple, EGRESS)) {
        return eval_error(skb, NULL, EGRESS, FAIL_TUPLE);
    }

    log_tuple(&tuple);

    if (tuple.l4_state != L4_PARSE_OK) {
        /* Non-first fragment / ESP / malformed — decided by the
         * ct -> default action -> MatchDomain chain, never by misread ports. */
        return handle_unparseable(family, skb, &tuple, EGRESS);
    }

    if (get_packet_rst_flag(&tuple)) {
        return SK_PASS;
    }

    struct ct_tuple ct_tpl = {};
    if (!get_ct_tuple(&tuple, &ct_tpl)) {
        return eval_error(skb, &tuple, EGRESS, FAIL_CT_TUPLE);
    }

    struct ct_entry *cte = NULL;
    bool loopback = match_loopback(tuple);
    enum ct_status status = ct_map_lookup_entry(family, skb, &ct_tpl, CT_EGRESS, loopback, &cte);
    log_dbg(DBG_CT_LOOKUP, EGRESS, status);
    /* Theoretical, declared assumption: cte stays raw across the scratch/
     * action-key map lookups below until ct_set_evaluated — the CT map is
     * LRU_HASH, so invalidating this entry inside the µs-scale window needs
     * another CPU to churn a full LRU capacity (~5000 inserts); revisit if
     * the capacity shrinks or a long map walk lands between lookup and write. */
    if (status != CT_NEW && cte != NULL) {
        if (ct_evaluated(cte, CT_EGRESS)) {
            /* Steady state — this direction already passed the full evaluation. */
            return SK_PASS;
        }
        /* Return-path inheritance (non-loopback only), mirror of the ingress
         * hook: the opposite direction passed its full evaluation, so this
         * direction's reply traffic is not re-adjudicated (inbound-allowed
         * connections must be able to answer). Loopback stays per-direction
         * (P4 cross-space isolation, unchanged). */
        if (!loopback && ct_evaluated(cte, CT_INGRESS)) {
            return SK_PASS;
        }
    }

    __u32 nf_scratch_key = 0;
    struct bitmap *key = bpf_map_lookup_elem(&NF_BITMAP_SCRATCH, &nf_scratch_key);
    if (key == NULL) {
        return eval_error(skb, &tuple, EGRESS, FAIL_SCRATCH);
    }
    __builtin_memset(key, 0, sizeof(*key));
    if (!match_action_key(skb, &tuple, key)) {
        /* Includes dev_name failure: fail-closed, never bypass iface deny rules. */
        return eval_error(skb, &tuple, EGRESS, FAIL_ACTION_KEY);
    }

    enum fw_eval_result act = match_action(&tuple, key, gate);
    if (act == FW_EVAL_ERROR) {
        return eval_error(skb, &tuple, EGRESS, FAIL_ACTION_ARGS);
    }
    /* No outbound DNS release hatch — outbound 53/853 and any
     * other destination fall to the pure adjudication chain. An authorized
     * query passed above through the §4.3 gate INSIDE match_action (same-
     * layer fallback under the no-rule branch), so this DROP is either an
     * explicit deny hit or the default action of a user with no
     * authorization — both must drop (red lines 2/3). The gate-passed
     * query keeps walking to the ct create + the hook-exit record flush. */
    if (act == FW_DROP) {
        log_intercept(&tuple);
        /* F5 fast-fail for the query the gate did not release: a
         * default-deny user's DNS query (gate->is_query, staged by the
         * egress dns program for a parseable 53 query or an 853 port
         * shape) dies HERE on the policy side: no server authorization,
         * or a non-allow qname that failed gate condition 4 (deny and
         * unrelated domains alike — the deny-qname query is normally
         * already gone at the sniff side, dns_sniff_drop; this path is
         * the sniff-side verdict ANDed with the policy verdict).
         * Destroy the querying socket for ms-level ECONNABORTED to the
         * owner vs. the seconds-long resolver backoff. Non-query drops
         * keep the legacy silent behavior. */
        if (gate != NULL && gate->is_query == 1) {
            (void)bpf_skb_sock_destroy(skb);
        }
        send_sock_tcp_reset(&tuple, skb, EGRESS);
        return SK_DROP;
    }

    if (status == CT_NEW) {
        /* Ct_create_entry sets egress_evaluated (this packet just passed). */
        ct_create_entry(&ct_tpl, skb, CT_EGRESS);
    } else if (cte != NULL) {
        /* First packet of this direction passed the full evaluation. */
        ct_set_evaluated(cte, CT_EGRESS);
    }

    return SK_PASS;
}

#endif // NET_FIREWALL_H