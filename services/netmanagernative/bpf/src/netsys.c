/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/string.h>
#include <stddef.h>
#include <stdint.h>

#include "bpf/bpf_helpers.h"
#include "bpf_def.h"

#define SEC(NAME) __attribute__((section(NAME), used))

// network stats begin
bpf_map_def SEC("maps") iface_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uint64_t),
    .value_size = sizeof(iface_stats_value),
    .max_entries = IFACE_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_uid_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(uint64_t),
    .value_size = sizeof(app_uid_stats_value),
    .max_entries = APP_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_uid_sim_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(app_uid_sim_stats_key),
    .value_size = sizeof(app_uid_sim_stats_value),
    .max_entries = APP_STATS_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_uid_if_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(app_uid_if_stats_key),
    .value_size = sizeof(app_uid_if_stats_value),
    .max_entries = IFACE_NAME_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") app_cookie_stats_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(socket_cookie_stats_key),
    .value_size = sizeof(app_cookie_stats_value),
    .max_entries = IFACE_NAME_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

SEC("socket/iface/stats")
int socket_iface_stats(struct __sk_buff *skb)
{
    if (skb == NULL) {
        return 1;
    }

    if (skb->pkt_type == PACKET_LOOPBACK) {
        return 1;
    }

    uint64_t ifindex = skb->ifindex;
    iface_stats_value *value_if = bpf_map_lookup_elem(&iface_stats_map, &ifindex);
    if (value_if == NULL) {
        iface_stats_value newValue = {};
        bpf_map_update_elem(&iface_stats_map, &ifindex, &newValue, BPF_NOEXIST);
        value_if = bpf_map_lookup_elem(&iface_stats_map, &ifindex);
    }

    if (skb->pkt_type == PACKET_OUTGOING) {
        if (value_if != NULL) {
            __sync_fetch_and_add(&value_if->txPackets, 1);
            __sync_fetch_and_add(&value_if->txBytes, skb->len);
        }
    } else {
        if (value_if != NULL) {
            __sync_fetch_and_add(&value_if->rxPackets, 1);
            __sync_fetch_and_add(&value_if->rxBytes, skb->len);
        }
    }
    return 1;
}

bpf_map_def SEC("maps") app_uid_access_policy_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(app_uid_key),
    .value_size = sizeof(uid_access_policy_value),
    .max_entries = UID_ACCESS_POLICY_ARRAY_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

SEC("cgroup_skb/uid/ingress")
int bpf_cgroup_skb_uid_ingress(struct __sk_buff *skb)
{
    if (skb == NULL || skb->pkt_type == PACKET_LOOPBACK) {
        return 1;
    }
    uint64_t sock_uid = bpf_get_socket_uid(skb);
    app_uid_stats_value *value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    if (value == NULL) {
        app_uid_stats_value newValue = {};
        bpf_map_update_elem(&app_uid_stats_map, &sock_uid, &newValue, BPF_NOEXIST);
        value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    }
    if (value != NULL) {
        __sync_fetch_and_add(&value->rxPackets, 1);
        __sync_fetch_and_add(&value->rxBytes, skb->len);
    }
    app_uid_if_stats_key key = {.uId = sock_uid, .ifIndex = skb->ifindex};
    app_uid_if_stats_value *value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    if (value_uid_if == NULL) {
        app_uid_if_stats_value newValue = {};
        bpf_map_update_elem(&app_uid_if_stats_map, &key, &newValue, BPF_NOEXIST);
        value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    }
    if (value_uid_if != NULL) {
        __sync_fetch_and_add(&value_uid_if->rxPackets, 1);
        __sync_fetch_and_add(&value_uid_if->rxBytes, skb->len);
    }
    socket_cookie_stats_key sock_cookie = bpf_get_socket_cookie(skb);
    app_cookie_stats_value *value_cookie = bpf_map_lookup_elem(&app_cookie_stats_map, &sock_cookie);
    if (value_cookie == NULL) {
        app_cookie_stats_value newValue = {};
        bpf_map_update_elem(&app_cookie_stats_map, &sock_cookie, &newValue, BPF_NOEXIST);
        value_cookie = bpf_map_lookup_elem(&app_cookie_stats_map, &sock_cookie);
    }
    if (value_cookie != NULL) {
        __sync_fetch_and_add(&value_cookie->rxPackets, 1);
        __sync_fetch_and_add(&value_cookie->rxBytes, skb->len);
    }
    return 1;
}

SEC("cgroup_skb/uid/egress")
int bpf_cgroup_skb_uid_egress(struct __sk_buff *skb)
{
    if (skb == NULL || skb->pkt_type == PACKET_LOOPBACK) {
        return 1;
    }
    uint64_t sock_uid = bpf_get_socket_uid(skb);
    uid_access_policy_value *netAccessPolicyValue = bpf_map_lookup_elem(&app_uid_access_policy_map, &sock_uid);
    if (netAccessPolicyValue != NULL) {
        if ((netAccessPolicyValue->netIfIndex == NETWORK_BEARER_TYPE_CELLULAR) &&
            (!netAccessPolicyValue->cellularPolicy)) {
            return 0;
        }
        if ((netAccessPolicyValue->netIfIndex == NETWORK_BEARER_TYPE_WIFI) && (!netAccessPolicyValue->wifiPolicy)) {
            return 0;
        }
    }

    app_uid_stats_value *value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    if (value == NULL) {
        app_uid_stats_value newValue = {};
        bpf_map_update_elem(&app_uid_stats_map, &sock_uid, &newValue, BPF_NOEXIST);
        value = bpf_map_lookup_elem(&app_uid_stats_map, &sock_uid);
    }
    if (value != NULL) {
        __sync_fetch_and_add(&value->txPackets, 1);
        __sync_fetch_and_add(&value->txBytes, skb->len);
    }
    app_uid_if_stats_key key = {.uId = sock_uid, .ifIndex = skb->ifindex};
    app_uid_if_stats_value *value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    if (value_uid_if == NULL) {
        app_uid_if_stats_value newValue = {};
        bpf_map_update_elem(&app_uid_if_stats_map, &key, &newValue, BPF_NOEXIST);
        value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
    }
    if (value_uid_if != NULL) {
        __sync_fetch_and_add(&value_uid_if->txPackets, 1);
        __sync_fetch_and_add(&value_uid_if->txBytes, skb->len);
    }
    socket_cookie_stats_key sock_cookie = bpf_get_socket_cookie(skb);
    app_cookie_stats_value *value_cookie = bpf_map_lookup_elem(&app_cookie_stats_map, &sock_cookie);
    if (value_cookie == NULL) {
        app_cookie_stats_value newValue = {};
        bpf_map_update_elem(&app_cookie_stats_map, &sock_cookie, &newValue, BPF_NOEXIST);
        value_cookie = bpf_map_lookup_elem(&app_cookie_stats_map, &sock_cookie);
    }
    if (value_cookie != NULL) {
        __sync_fetch_and_add(&value_cookie->txPackets, 1);
        __sync_fetch_and_add(&value_cookie->txBytes, skb->len);
    }
    return 1;
}
// network stats end

// internet permission begin
bpf_map_def SEC("maps") oh_sock_permission_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(sock_permission_key),
    .value_size = sizeof(sock_permission_value),
    .max_entries = OH_SOCK_PERMISSION_MAP_SIZE,
};

bpf_map_def SEC("maps") broker_sock_permission_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(sock_permission_key),
    .value_size = sizeof(sock_permission_value),
    .max_entries = BROKER_SOCK_PERMISSION_MAP_SIZE,
};

SEC("cgroup_sock/inet_create_socket")
int inet_create_socket(struct bpf_sock *sk)
{
    void *map_ptr = &oh_sock_permission_map;
    if (bpf_get_netns_cookie(sk) != bpf_get_netns_cookie(NULL)) {
        map_ptr = &broker_sock_permission_map;
    }

    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    sock_permission_value *value = bpf_map_lookup_elem(map_ptr, &uid);
    // value == NULL means that the process attached to this uid is not a hap process which started by appspawn
    // it is a native process, native process should have this permission
    if (value == NULL) {
        return 1;
    }
    // *value == 0 means no permission
    if (*value == 0) {
        return 0;
    }
    return 1;
}
// internet permission end
bpf_map_def SEC("maps") net_bear_type_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(net_bear_id_key),
    .value_size = sizeof(net_bear_type_map_value),
    .max_entries = IFACE_NAME_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") ringbuf_map = {
    .type = BPF_MAP_TYPE_RINGBUF,
    .max_entries = 256 * 1024 /* 256 KB */,
};

SEC("cgroup_addr/bind4")
static int inet_check_bind4(struct bpf_sock_addr *ctx)
{
    void *map_ptr = &app_uid_access_policy_map;

    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    uid_access_policy_value *value = bpf_map_lookup_elem(map_ptr, &uid);
    // value == NULL means that the process attached to this uid is not a hap process which has a default configuration
    if (value == NULL) {
        return 1;
    }

    void *net_bear_map_ptr = &net_bear_type_map;
    net_bear_id_key net_bear_id = DEFAULT_NETWORK_BEARER_MAP_KEY;

    net_bear_type_map_value *net_bear_type = bpf_map_lookup_elem(net_bear_map_ptr, &net_bear_id);
    if (net_bear_type == NULL) {
        return 1;
    }

    if (((*net_bear_type == NETWORK_BEARER_TYPE_WIFI) && (!value->wifiPolicy)) ||
        ((*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR) && (!value->cellularPolicy))) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag ||
            ((value->netIfIndex != NETWORK_BEARER_TYPE_INITIAL) && (value->netIfIndex != *net_bear_type))) {
            uint32_t *e;
            e = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*e), 0);
            if (e) {
                *e = uid;
                bpf_ringbuf_submit(e, 0);
                value->diagAckFlag = 1;
            }
        }
        value->netIfIndex = *net_bear_type;
        bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
        return 0;
    }

    value->netIfIndex = *net_bear_type;
    bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
    return 1;
}

SEC("cgroup_addr/bind6")
static int inet_check_bind6(struct bpf_sock_addr *ctx)
{
    void *map_ptr = &app_uid_access_policy_map;

    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    uid_access_policy_value *value = bpf_map_lookup_elem(map_ptr, &uid);
    // value == NULL means that the process attached to this uid is not a hap process which has a default configuration
    if (value == NULL) {
        return 1;
    }

    void *net_bear_map_ptr = &net_bear_type_map;
    net_bear_id_key net_bear_id = DEFAULT_NETWORK_BEARER_MAP_KEY;

    net_bear_type_map_value *net_bear_type = bpf_map_lookup_elem(net_bear_map_ptr, &net_bear_id);
    if (net_bear_type == NULL) {
        return 1;
    }

    if (((*net_bear_type == NETWORK_BEARER_TYPE_WIFI) && (!value->wifiPolicy)) ||
        ((*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR) && (!value->cellularPolicy))) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag ||
            ((value->netIfIndex != NETWORK_BEARER_TYPE_INITIAL) && (value->netIfIndex != *net_bear_type))) {
            uint32_t *e;
            e = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*e), 0);
            if (e) {
                *e = uid;
                bpf_ringbuf_submit(e, 0);
                value->diagAckFlag = 1;
            }
        }
        value->netIfIndex = *net_bear_type;
        bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
        return 0;
    }

    value->netIfIndex = *net_bear_type;
    bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
    return 1;
}

SEC("cgroup_addr/connect4")
static int inet_check_connect4(struct bpf_sock_addr *ctx)
{
    void *map_ptr = &app_uid_access_policy_map;

    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    uid_access_policy_value *value = bpf_map_lookup_elem(map_ptr, &uid);
    // value == NULL means that the process attached to this uid is not a hap process which has a default configuration
    if (value == NULL) {
        return 1;
    }

    void *net_bear_map_ptr = &net_bear_type_map;
    net_bear_id_key net_bear_id = DEFAULT_NETWORK_BEARER_MAP_KEY;

    net_bear_type_map_value *net_bear_type = bpf_map_lookup_elem(net_bear_map_ptr, &net_bear_id);
    if (net_bear_type == NULL) {
        return 1;
    }

    if (((*net_bear_type == NETWORK_BEARER_TYPE_WIFI) && (!value->wifiPolicy)) ||
        ((*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR) && (!value->cellularPolicy))) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag ||
            ((value->netIfIndex != NETWORK_BEARER_TYPE_INITIAL) && (value->netIfIndex != *net_bear_type))) {
            uint32_t *e;
            e = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*e), 0);
            if (e) {
                *e = uid;
                bpf_ringbuf_submit(e, 0);
                value->diagAckFlag = 1;
            }
        }
        value->netIfIndex = *net_bear_type;
        bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
        return 0;
    }

    value->netIfIndex = *net_bear_type;
    bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
    return 1;
}


SEC("cgroup_addr/connect6")
static int inet_check_connect6(struct bpf_sock_addr *ctx)
{
    void *map_ptr = &app_uid_access_policy_map;

    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    uid_access_policy_value *value = bpf_map_lookup_elem(map_ptr, &uid);
    // value == NULL means that the process attached to this uid is not a hap process which has a default configuration
    if (value == NULL) {
        return 1;
    }

    void *net_bear_map_ptr = &net_bear_type_map;
    net_bear_id_key net_bear_id = DEFAULT_NETWORK_BEARER_MAP_KEY;

    net_bear_type_map_value *net_bear_type = bpf_map_lookup_elem(net_bear_map_ptr, &net_bear_id);
    if (net_bear_type == NULL) {
        return 1;
    }

    if (((*net_bear_type == NETWORK_BEARER_TYPE_WIFI) && (!value->wifiPolicy)) ||
        ((*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR) && (!value->cellularPolicy))) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag ||
            ((value->netIfIndex != NETWORK_BEARER_TYPE_INITIAL) && (value->netIfIndex != *net_bear_type))) {
            uint32_t *e;
            e = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*e), 0);
            if (e) {
                *e = uid;
                bpf_ringbuf_submit(e, 0);
                value->diagAckFlag = 1;
            }
        }
        value->netIfIndex = *net_bear_type;
        bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
        return 0;
    }

    value->netIfIndex = *net_bear_type;
    bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
    return 1;
}

SEC("cgroup_addr/sendmsg4")
static int inet_check_sendmsg4(struct bpf_sock_addr *ctx)
{
    void *map_ptr = &app_uid_access_policy_map;

    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    uid_access_policy_value *value = bpf_map_lookup_elem(map_ptr, &uid);
    // value == NULL means that the process attached to this uid is not a hap process which has a default configuration
    if (value == NULL) {
        return 1;
    }

    void *net_bear_map_ptr = &net_bear_type_map;
    net_bear_id_key net_bear_id = DEFAULT_NETWORK_BEARER_MAP_KEY;

    net_bear_type_map_value *net_bear_type = bpf_map_lookup_elem(net_bear_map_ptr, &net_bear_id);
    if (net_bear_type == NULL) {
        return 1;
    }

    if (((*net_bear_type == NETWORK_BEARER_TYPE_WIFI) && (!value->wifiPolicy)) ||
        ((*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR) && (!value->cellularPolicy))) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag ||
            ((value->netIfIndex != NETWORK_BEARER_TYPE_INITIAL) && (value->netIfIndex != *net_bear_type))) {
            uint32_t *e;
            e = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*e), 0);
            if (e) {
                *e = uid;
                bpf_ringbuf_submit(e, 0);
                value->diagAckFlag = 1;
            }
        }
        value->netIfIndex = *net_bear_type;
        bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
        return 0;
    }

    value->netIfIndex = *net_bear_type;
    bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
    return 1;
}

SEC("cgroup_addr/sendmsg6")
static int inet_check_sendmsg6(struct bpf_sock_addr *ctx)
{
    void *map_ptr = &app_uid_access_policy_map;

    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    uid_access_policy_value *value = bpf_map_lookup_elem(map_ptr, &uid);
    // value == NULL means that the process attached to this uid is not a hap process which has a default configuration
    if (value == NULL) {
        return 1;
    }

    void *net_bear_map_ptr = &net_bear_type_map;
    net_bear_id_key net_bear_id = DEFAULT_NETWORK_BEARER_MAP_KEY;

    net_bear_type_map_value *net_bear_type = bpf_map_lookup_elem(net_bear_map_ptr, &net_bear_id);
    if (net_bear_type == NULL) {
        return 1;
    }

    if (((*net_bear_type == NETWORK_BEARER_TYPE_WIFI) && (!value->wifiPolicy)) ||
        ((*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR) && (!value->cellularPolicy))) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag ||
            ((value->netIfIndex != NETWORK_BEARER_TYPE_INITIAL) && (value->netIfIndex != *net_bear_type))) {
            uint32_t *e;
            e = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*e), 0);
            if (e) {
                *e = uid;
                bpf_ringbuf_submit(e, 0);
                value->diagAckFlag = 1;
            }
        }
        value->netIfIndex = *net_bear_type;
        bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
        return 0;
    }

    value->netIfIndex = *net_bear_type;
    bpf_map_update_elem(map_ptr, &uid, value, BPF_NOEXIST);
    return 1;
}

char g_license[] SEC("license") = "GPL";
