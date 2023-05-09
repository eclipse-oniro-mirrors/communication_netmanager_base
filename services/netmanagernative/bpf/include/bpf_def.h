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

#ifndef NETMANAGER_BASE_BPF_DEF_H
#define NETMANAGER_BASE_BPF_DEF_H

#include <linux/bpf.h>

static const int32_t APP_STATS_MAP_SIZE = 5000;
static const int32_t IFACE_STATS_MAP_SIZE = 1000;
static const int32_t IFACE_NAME_MAP_SIZE = 1000;
static const int32_t IFNAME_SIZE = 32;

typedef struct {
    enum bpf_map_type type;
    __u32 key_size;
    __u32 value_size;
    __u32 max_entries;
    __u32 map_flags;
    __u32 inner_map_idx;
    __u32 numa_node;
} bpf_map_def;

typedef struct {
    __u32 uId;
    __u32 ifIndex;
} stats_key;

typedef struct {
    __u64 rxPackets;
    __u64 rxBytes;
    __u64 txPackets;
    __u64 txBytes;
} stats_value;

typedef struct {
    char name[IFNAME_SIZE];
} iface_name;

typedef __u64 iface_stats_key;
typedef stats_value iface_stats_value;

typedef __u64 app_uid_stats_key;
typedef stats_value app_uid_stats_value;

typedef stats_key app_uid_if_stats_key;
typedef stats_value app_uid_if_stats_value;
#endif /* NETMANAGER_BASE_BPF_DEF_H */
