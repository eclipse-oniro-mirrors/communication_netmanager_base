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

#ifndef NET_FIREWALL_MAP_DEF_H
#define NET_FIREWALL_MAP_DEF_H

#ifndef SEC
#define SEC(NAME) __attribute__((section(NAME), used))
#endif // SEC

#define MAP_MAX_ENTRIES 5000
#define MAP_MAX_PORT_ENTRIES 65535

#define INGRESS_SADDR_MAP in_saddr_map
#define INGRESS_SADDR6_MAP in_saddr6_map
#define INGRESS_DADDR_MAP in_daddr_map
#define INGRESS_DADDR6_MAP in_daddr6_map
#define INGRESS_SPORT_MAP in_sport_map
#define INGRESS_DPORT_MAP in_dport_map
#define INGRESS_PROTO_MAP in_proto_map
#define INGRESS_APPUID_MAP in_appuid_map
#define INGRESS_UID_MAP in_uid_map
#define INGRESS_ACTION_MAP in_action_map
#define INGRESS_INTERFACE_MAP in_iface_map
#define INGRESS_WILDCARD_MAP in_wildcard_map

#define EGRESS_SADDR_MAP out_saddr_map
#define EGRESS_SADDR6_MAP out_saddr6_map
#define EGRESS_DADDR_MAP out_daddr_map
#define EGRESS_DADDR6_MAP out_daddr6_map
#define EGRESS_SPORT_MAP out_sport_map
#define EGRESS_DPORT_MAP out_dport_map
#define EGRESS_PROTO_MAP out_proto_map
#define EGRESS_APPUID_MAP out_appuid_map
#define EGRESS_UID_MAP out_uid_map
#define EGRESS_ACTION_MAP out_action_map
#define EGRESS_INTERFACE_MAP out_iface_map
#define EGRESS_WILDCARD_MAP out_wildcard_map

#define EVENT_MAP event_map
#define DEFAULT_ACTION_MAP def_act_map
#define ZERO_BITMAP_MAP zero_bitmap_map
#define CT_MAP ct_map
#define CURRENT_UID_MAP current_uid_map
#define DOMAIN_IPV4_MAP domain_ipv4_map
#define DOMAIN_IPV6_MAP domain_ipv6_map
#define LOOP_BACK_IPV4_MAP loop_ipv4_map
#define LOOP_BACK_IPV6_MAP loop_ipv6_map
#define DOMAIN_SCRATCH_MAP domain_scratch_map
/* DNS query record table (pinned by the loader) and the percpu
 * staging slot (BPF-internal only, not consumed from user space). */
#define DNS_QUERY_RECORD_MAP dns_query_record_map
#define DNS_QUERY_SCRATCH_MAP dns_query_scratch_map
#define DNS_FLOW_SCRATCH_MAP dns_flow_scrat_map
/* A' label-anchored trie: one forward-anchored table (prefix rules) and
 * one backward-anchored table (suffix rules), both plain HASH with the
 * composite (uid, appuid, state, cls, gen) key — the custom loader has no
 * HASH_OF_MAPS, so the trie nodes live in one map. Generation switch:
 * the builder writes the inactive generation, flips this single-entry
 * array, then clears the old one. */
#define AC_PREFIX_TRANS_MAP ac_prefix_trans_map
#define AC_SUFFIX_TRANS_MAP ac_suffix_trans_map
#define AC_GEN_MAP ac_gen_map
/* Per-CPU staging for the egress per-domain gate ctx: the dns program
 * fills it and the main program consumes it — they are separate
 * attachments (dual-attach split), so a stack-local gate ctx cannot
 * cross the program boundary. */
#define DNS_GATE_SCRATCH_MAP dns_gate_scrat_map
/* DNS server authorization maps — control-plane written, consumed by
 * the per-domain gate. Pinned by the loader like every other map. */
#define DNS_SERVER_V4_MAP dns_server_v4_map
#define DNS_SERVER_V6_MAP dns_server_v6_map
#define NF_BITMAP_SCRATCH nf_bm_scrat_map
#define EVAL_FALLBACK_MODE_MAP eval_fallback_mode_map

#define MAPS_DIR() "/sys/fs/bpf/netsys/maps/"
#define STR(x) #x
#define MAP_NAME(x) STR(x)
#define MAP_PATH(name) MAPS_DIR() MAP_NAME(name)
#define GET_MAP_PATH(ingress, name) ((ingress) ? MAPS_DIR() "in_" #name "_map" : MAPS_DIR() "out_" #name "_map")
#define GET_MAP(ingress, name) ((ingress) ? &in_##name##_map : &out_##name##_map)

#endif // NET_FIREWALL_MAP_DEF_H