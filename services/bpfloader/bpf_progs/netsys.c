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

#include <stdint.h>
#include <linux/bpf.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tunnel.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/mpls.h>

#define SEC(NAME) __attribute__((section(NAME), used))

static void *(*BpfMapLookupElem)(void *map, const void *key) = (void *)BPF_FUNC_map_lookup_elem;
static long (*BpfMapUpdateElem)(void *map, const void *key, const void *value, __u64 flags) =
    (void *)BPF_FUNC_map_update_elem;
static long (*BpfTracePrintk)(const char *fmt, __u32 fmt_size, ...) = (void *)BPF_FUNC_trace_printk;
static __u32 (*BpfGetSocketUid)(struct __sk_buff *skb) = (void *)BPF_FUNC_get_socket_uid;

static const int APP_STATS_MAP_SIZE = 5000;
static const int IFACE_STATS_MAP_SIZE = 1000;
static const int IFACE_NAM_MAP_SIZE = 1000;
static const int IFNAM_SIZE = 32;

typedef struct {
    enum bpf_map_type type;
    unsigned int key_size;
    unsigned int value_size;
    unsigned int max_entries;
    unsigned int map_flags;
    unsigned int inner_map_idx;
    unsigned int numa_node;
} bpf_map_def;

typedef struct {
    uint32_t uId;
    uint32_t ifIndex;
} StatsKey;

typedef struct {
    uint64_t rxPackets;
    uint64_t rxBytes;
    uint64_t txPackets;
    uint64_t txBytes;
} StatsValue;

typedef struct {
    char name[IFNAM_SIZE];
} IfaceName;

bpf_map_def SEC("maps") iface_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uint64_t),
    .value_size = sizeof(StatsValue),
    .max_entries = IFACE_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") iface_name_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uint64_t),
    .value_size = sizeof(IfaceName),
    .max_entries = IFACE_NAM_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_uid_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uint64_t),
    .value_size = sizeof(StatsValue),
    .max_entries = APP_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_uid_if_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(StatsKey),
    .value_size = sizeof(StatsValue),
    .max_entries = APP_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

SEC("cgroup_skb/uid/ingress")
int bpf_cgroup_skb_uid_ingress(struct __sk_buff *skb)
{
    return 1;
}

SEC("cgroup_skb/uid/egress")
int bpf_cgroup_skb_uid_egress(struct __sk_buff *skb)
{
    return 1;
}

char g_license[] SEC("license") = "GPL";