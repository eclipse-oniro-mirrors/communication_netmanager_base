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

#define GET_IP_SEGMENT(ip, seg) (((ip) >> (((seg)-1) * 8)) & 0xFF)
#define IS_MATCHED_IP(ip, target) \
    GET_IP_SEGMENT(ip, 1) == (target)[0] && GET_IP_SEGMENT(ip, 2) == (target)[1] && \
    GET_IP_SEGMENT(ip, 3) == (target)[2] && GET_IP_SEGMENT(ip, 4) == (target)[3]    \

static const uint32_t WLAN_IPv4[4] = {172, 17, 1, 2};
static const uint32_t CELLULAR_IPv4[4] = {172, 17, 0, 2};
static const int32_t IFACE_TYPE_CELLULAR = 1;
static const int32_t IFACE_TYPE_WIFI = 2;
static const int32_t APP_STATS_MAP_SIZE = 5000;
static const int32_t IFACE_STATS_MAP_SIZE = 1000;
static const int32_t IFACE_NAME_MAP_SIZE = 1000;
static const int32_t OH_SOCK_PERMISSION_MAP_SIZE = 1000;
static const int32_t BROKER_SOCK_PERMISSION_MAP_SIZE = 1000;
static const int32_t UID_ACCESS_POLICY_ARRAY_SIZE = 65535;
static const int32_t NET_NS_MAP_SIZE = 5000;
static const uint64_t SOCK_COOKIE_ID_NULL = UINT64_MAX;
static const int32_t SIM_UID_MAX = 20000;
static const int32_t SIM_UID_MIN = 10000;
static const uint64_t DEFAULT_BROKER_UID_KEY = 65536;
enum { IFNAME_SIZE = 32 };
enum { DEFAULT_NETWORK_BEARER_MAP_KEY = 0 };

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
    __u32 ifType;
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

typedef struct {
    __u8 wifiPolicy;
    __u8 cellularPolicy;
    __u8 configSetFromFlag;
    __u8 diagAckFlag;
    __u32 netIfIndex;
} uid_access_policy_value;

enum network_bearer_type {
    NETWORK_BEARER_TYPE_INITIAL = 0,
    NETWORK_BEARER_TYPE_CELLULAR,
    NETWORK_BEARER_TYPE_WIFI,
};

// network stats begin
typedef __u64 iface_stats_key;
typedef stats_value iface_stats_value;

typedef __u64 app_uid_stats_key;
typedef stats_value app_uid_stats_value;

typedef __u64 sock_netns_key;
typedef __u64 sock_netns_value;

typedef stats_key app_uid_sim_stats_key;
typedef stats_value app_uid_sim_stats_value;

typedef stats_key app_uid_if_stats_key;
typedef stats_value app_uid_if_stats_value;

typedef __u64 socket_cookie_stats_key;
typedef stats_value app_cookie_stats_value;
// network stats end

// internet permission begin
typedef __u32 sock_permission_key;
typedef __u8 sock_permission_value;
// internet permission end

typedef __u32 net_bear_id_key;
typedef __u32 net_bear_type_map_value;

typedef __u16 net_index;
typedef __u8 net_interface_name_id;

typedef __u32 app_uid_key;
#endif /* NETMANAGER_BASE_BPF_DEF_H */
