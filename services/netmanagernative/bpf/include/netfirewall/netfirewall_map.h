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
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") INGRESS_DPORT_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_PROTO_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(proto_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_ACTION_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(action_key),
    .value_size = sizeof(action_val),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") INGRESS_APPUID_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(appuid_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
// ingress map end
// egress map begin
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
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_DPORT_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(port_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_PROTO_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(proto_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_ACTION_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(action_key),
    .value_size = sizeof(action_val),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};
bpf_map_def SEC("maps") EGRESS_APPUID_MAP = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(appuid_key),
    .value_size = sizeof(struct bitmap),
    .max_entries = MAP_MAX_ENTRIES,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") DEFAULT_ACTION_MAP = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(default_action_key),
    .value_size = sizeof(action_val),
    .max_entries = 2,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

#endif // NET_FIREWALL_MAP_H