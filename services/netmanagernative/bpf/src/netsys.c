/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/bpf.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tunnel.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/mpls.h>
#include <stddef.h>
#include <stdint.h>

#include "bpf/bpf_endian.h"
#include "bpf/bpf_helpers.h"
#include "bpf_def.h"

#define SEC(NAME) __attribute__((section(NAME), used))

bpf_map_def SEC("maps") iface_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uint64_t),
    .value_size = sizeof(stats_value),
    .max_entries = IFACE_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_uid_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uint64_t),
    .value_size = sizeof(stats_value),
    .max_entries = APP_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_uid_if_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(stats_key),
    .value_size = sizeof(stats_value),
    .max_entries = IFACE_NAME_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

SEC("cgroup_skb/uid/ingress")
int bpf_cgroup_skb_uid_ingress(struct __sk_buff *skb) {
    if (skb == NULL) {
        return 1;
    }
    uint64_t sock_uid = bpf_get_socket_uid(skb);
    stats_value *value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    if (value == NULL) {
        stats_value newValue = {};
        bpf_map_update_elem(&app_uid_stats_map, &sock_uid, &newValue, BPF_NOEXIST);
        value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    }
    if (value != NULL) {
        __sync_fetch_and_add(&value->rxPackets, 1);
        __sync_fetch_and_add(&value->rxBytes, skb->len);
    }
    stats_key key = {.uId = sock_uid, .ifIndex = skb->ifindex};
    stats_value *value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    if (value_uid_if == NULL) {
        stats_value newValue = {};
        bpf_map_update_elem(&app_uid_if_stats_map, &key, &newValue, BPF_NOEXIST);
        value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    }
    if (value_uid_if != NULL) {
        __sync_fetch_and_add(&value_uid_if->rxPackets, 1);
        __sync_fetch_and_add(&value_uid_if->rxBytes, skb->len);
    }
    uint64_t ifindex = skb->ifindex;
    stats_value *value_if = bpf_map_lookup_elem(&iface_stats_map, &ifindex);
    if (value_if == NULL) {
        stats_value newValue = {};
        bpf_map_update_elem(&iface_stats_map, &ifindex, &newValue, BPF_NOEXIST);
        value_if = bpf_map_lookup_elem(&iface_stats_map, &ifindex);
    }
    if (value_if != NULL) {
        __sync_fetch_and_add(&value_if->rxPackets, 1);
        __sync_fetch_and_add(&value_if->rxBytes, skb->len);
    }
    return 1;
}

SEC("cgroup_skb/uid/egress")
int bpf_cgroup_skb_uid_egress(struct __sk_buff *skb) {
    if (skb == NULL) {
        return 1;
    }
    uint64_t sock_uid = bpf_get_socket_uid(skb);
    stats_value *value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    if (value == NULL) {
        stats_value newValue = {};
        bpf_map_update_elem(&app_uid_stats_map, &sock_uid, &newValue, BPF_NOEXIST);
        value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    }
    if (value != NULL) {
        __sync_fetch_and_add(&value->txPackets, 1);
        __sync_fetch_and_add(&value->txBytes, skb->len);
    }
    stats_key key = {.uId = sock_uid, .ifIndex = skb->ifindex};
    stats_value *value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    if (value_uid_if == NULL) {
        stats_value newValue = {};
        bpf_map_update_elem(&app_uid_if_stats_map, &key, &newValue, BPF_NOEXIST);
        value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    }
    if (value_uid_if != NULL) {
        __sync_fetch_and_add(&value_uid_if->txPackets, 1);
        __sync_fetch_and_add(&value_uid_if->txBytes, skb->len);
    }
    uint64_t ifindex = skb->ifindex;
    stats_value *value_if = bpf_map_lookup_elem(&iface_stats_map, &ifindex);
    if (value_if == NULL) {
        stats_value newValue = {};
        bpf_map_update_elem(&iface_stats_map, &ifindex, &newValue, BPF_NOEXIST);
        value_if = bpf_map_lookup_elem(&iface_stats_map, &ifindex);
    }
    if (value_if != NULL) {
        __sync_fetch_and_add(&value_if->txPackets, 1);
        __sync_fetch_and_add(&value_if->txBytes, skb->len);
    }
    return 1;
}

char g_license[] SEC("license") = "GPL";