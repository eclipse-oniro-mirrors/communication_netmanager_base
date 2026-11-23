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

#ifndef NET_FIREWALL_MAP_H
#define NET_FIREWALL_MAP_H

#include <linux/bpf.h>

#include "netfirewall_types.h"
#include "netfirewall_map_def.h"

// ingress map begin
bpf_map_def SEC("maps") INGRESS_SADDR_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv4_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_SADDR6_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv6_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") INGRESS_DADDR_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv4_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_DADDR6_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv6_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") INGRESS_SPORT_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_PORT_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") INGRESS_DPORT_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_PORT_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_PROTO_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(proto_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_ACTION_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(action_key),
    .value_size = sizeof(action_val),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_APPUID_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(appuid_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_UID_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uid_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_INTERFACE_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(interface_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
/* Holds the "rules with no constraint on this dimension" bitmaps, keyed by dimension
 * role (wildcard_role). Replaces the per-dimension sentinel entries: the key space
 * (role 0-7) can never collide with any rule key value. */
bpf_map_def SEC("maps") INGRESS_WILDCARD_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(wildcard_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = 16,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") EGRESS_SADDR_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv4_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_SADDR6_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv6_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") EGRESS_DADDR_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv4_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_DADDR6_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv6_lpm_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_SPORT_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_PORT_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_DPORT_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_PORT_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_PROTO_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(proto_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_ACTION_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(action_key),
    .value_size = sizeof(action_val),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_APPUID_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(appuid_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_UID_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uid_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") EGRESS_INTERFACE_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(interface_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
/* See INGRESS_WILDCARD_MAP. */
bpf_map_def SEC("maps") EGRESS_WILDCARD_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(wildcard_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = 16,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") DEFAULT_ACTION_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uid_key),
    .value_size = sizeof(struct defalut_action_value),
    .max_entries = MAP_MAX_ENTRIES,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") CURRENT_UID_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(current_user_id_key),
    .value_size = sizeof(uid_key),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") DOMAIN_IPV4_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv4_lpm_key),
    .value_size = sizeof(struct domain_value),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") DOMAIN_IPV6_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv6_lpm_key),
    .value_size = sizeof(struct domain_value),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") LOOP_BACK_IPV4_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv4_lpm_key),
    .value_size = sizeof(loop_back_val),
    .max_entries = 1,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") LOOP_BACK_IPV6_MAP = {
    .type = BPF_MAP_TYPE_LPM_TRIE,
    .key_size = sizeof(struct ipv6_lpm_key),
    .value_size = sizeof(loop_back_val),
    .max_entries = 1,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") DOMAIN_SCRATCH_MAP = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(__u32),
    .value_size = sizeof(struct domain_hash_key),
    .max_entries = 2,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* DNS query record table — the credential an ingress response must
 * match (txid + reversed 5-tuple + 60s) before its answers are learned.
 * LRU_HASH (same pattern as CT_MAP): a flood of real queries evicts the
 * oldest record instead of failing the write, so stuffing the table cannot
 * become a persistent availability attack. */
bpf_map_def SEC("maps") DNS_QUERY_RECORD_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(struct dns_query_record_key),
    .value_size = sizeof(struct dns_query_record_value),
    .max_entries = DNS_QUERY_RECORD_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* Egress query staging slot — written at the sniffing step (hook
 * head) and read at the hook exit, single packet lifetime on one CPU, so a
 * percpu single-entry array has no cross-CPU race window. */
bpf_map_def SEC("maps") DNS_QUERY_SCRATCH_MAP = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(__u32),
    .value_size = sizeof(struct dns_query_pending),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* DNS flow scratch — the sniffer assembles the 5-tuple here instead of
 * a stack local and hands the map pointer to the parse subprograms:
 * a 40-byte stack local would push the bpf-to-bpf call chain's combined
 * frame past the verifier's 512-byte limit, and through the map every
 * field reaches the callee as an opaque scalar (single packet lifetime
 * on one CPU, same rationale as DNS_QUERY_SCRATCH_MAP). BPF-internal
 * only, not consumed from user space. */
bpf_map_def SEC("maps") DNS_FLOW_SCRATCH_MAP = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(__u32),
    .value_size = sizeof(struct dns_flow_info),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* A' label-anchored trie transition tables. Plain prealloc HASH (no
 * BPF_F_NO_PREALLOC): on this kernel (HM 1.15) NO_PREALLOC element-pool
 * replenishment is unreliable — a sticky cap of ~4-87 elements observed
 * with 30x in-place retries ineffective, while wildcard rule sets need
 * ~320 transitions per pattern and always hit ENOMEM. Prealloc costs
 * an EAGER full pool at create time (~106B/entry measured: 262144 ->
 * ~26.5MB, 16384 -> ~1.6MB, linear), does not grow with usage, reuses
 * freed entries inside the pool (generation flips never grow it), and
 * returns everything on map destroy. AC_TRANS_MAX_ENTRIES is therefore
 * a standing kernel-memory budget decision, not just a capacity cap.
 * Builder overflow fails fast: the generation flip never happens and
 * the previous table stays online (graceful rejection). Measured on
 * device 2026-11-15. The key is the composite (uid, appuid, state,
 * cls, gen) — see netfirewall_types.h for the class encoding.
 * BPF-internal only, not consumed from user space except by the rule
 * builder. */
bpf_map_def SEC("maps") AC_PREFIX_TRANS_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(struct ac_trans_key),
    .value_size = sizeof(struct ac_trans_val),
    .max_entries = AC_TRANS_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") AC_SUFFIX_TRANS_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(struct ac_trans_key),
    .value_size = sizeof(struct ac_trans_val),
    .max_entries = AC_TRANS_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* Active trie generation, single entry keyed 0. The builder writes the
 * inactive generation's transitions, flips this value, then clears the
 * old generation — every scan reads it once and uses one consistent
 * table view for its whole walk. */
bpf_map_def SEC("maps") AC_GEN_MAP = {
    .type = BPF_MAP_TYPE_ARRAY,
    .key_size = sizeof(__u32),
    .value_size = sizeof(__u32),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* Egress per-domain gate staging across the dns->main program boundary
 * (dual-attach split: the two programs are separate attachments on the
 * same CPU and the same packet). The dns program clears and fills the
 * slot for every egress packet; only the main program reads it (always
 * right after the dns program ran on the same CPU — BPF programs are
 * non-preemptible, so the slot cannot be torn). BPF-internal only. */
bpf_map_def SEC("maps") DNS_GATE_SCRATCH_MAP = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(__u32),
    .value_size = sizeof(struct dns_gate_ctx),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* DNS server authorization maps. Plain HASH (not LRU): the control
 * plane owns the content completely and a lost entry is a config bug to be
 * observed, not something the kernel should silently evict. */
bpf_map_def SEC("maps") DNS_SERVER_V4_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(struct dns_server_v4_key),
    .value_size = sizeof(struct dns_server_val),
    .max_entries = DNS_SERVER_MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") DNS_SERVER_V6_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(struct dns_server_v6_key),
    .value_size = sizeof(struct dns_server_val),
    .max_entries = DNS_SERVER_MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") NF_BITMAP_SCRATCH = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(__u32),
    .value_size = sizeof(struct bitmap),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
/* Defensive all-zero bitmap for dimension fallback misses. Never written by anyone,
 * so a lookup always yields a zero bitmap and the packet is decided by the default
 * action (fail-closed) instead of silently skipping the dimension. Implemented as a
 * percpu array because the custom ELF loader only resolves map symbol relocations. */
bpf_map_def SEC("maps") ZERO_BITMAP_MAP = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(__u32),
    .value_size = sizeof(struct bitmap),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

/* Evaluation fallback mode switch: single entry keyed 0.
 * No entry = FAIL_CLOSED (secure default); OBSERVE(0) = legacy pass-through escape hatch.
 * Fail-closed is equivalently expressed by writing EVAL_FALLBACK_FAIL_CLOSED(1). */
bpf_map_def SEC("maps") EVAL_FALLBACK_MODE_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(__u32),
    .value_size = sizeof(__u32),
    .max_entries = 1,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
#endif // NET_FIREWALL_MAP_H