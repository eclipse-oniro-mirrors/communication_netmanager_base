/*
* Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.

* The netsys.c is dual licensed: you can use it either under the terms of
* the GPL V2, or the 3-Clause BSD license, at your option.
* See the LICENSE file in the root of this repository for complete details.
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

#ifdef FEATURE_NET_FIREWALL_ENABLE
#include "netfirewall/netfirewall.h"
#endif //FEATURE_NET_FIREWALL_ENABLE

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

bpf_map_def SEC("maps") sock_netns_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(sock_netns_key),
    .value_size = sizeof(sock_netns_value),
    .max_entries = NET_NS_MAP_SIZE,
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

static inline __u32 get_data_len(struct __sk_buff *skb)
{
    __u32 length = skb->len;
    if (skb->vlan_present == 1) {
        length += VLAN_HEADER_LENGTH;
    }
    if (skb->family == AF_INET) {
        length += IPV4_HEADERS_LENGTH;
    }
    if (skb->family == AF_INET6) {
        length += IPV6_HEADERS_LENGTH;
    }
    return length;
}

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
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") broker_uid_access_policy_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(app_uid_key),
    .value_size = sizeof(app_uid_key),
    .max_entries = 1,
    .map_flags = BPF_F_NO_PREALLOC,
    .inner_map_idx = 0,
    .numa_node = 0,
};

bpf_map_def SEC("maps") net_index_and_iface_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(net_index),
    .value_size = sizeof(net_interface_name_id),
    .max_entries = 5,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

static inline net_bear_type_map_value check_socket_fwmark(__u32 mark)
{
    __u8 explicitlySelected = (mark >> 16) & (0x1);
    net_bear_type_map_value net_bear_mark_type = NETWORK_BEARER_TYPE_INITIAL;
    // explicitlySelected == 1 means the socket fwmark is set
    if (explicitlySelected == 1) {
        void *ifaceC_map_ptr = &net_index_and_iface_map;
        __u16 TmpnetId = mark & (0x0000FFFF);
        net_interface_name_id *ifaceC = bpf_map_lookup_elem(ifaceC_map_ptr, &TmpnetId);
        // ifaceC == NULL, default bear type (*net_bear_type) is used.
        if (ifaceC != NULL) {
            net_bear_mark_type = *ifaceC;
        }
    }
    return net_bear_mark_type;
}

bpf_map_def SEC("maps") net_bear_type_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(net_bear_id_key),
    .value_size = sizeof(net_bear_type_map_value),
    .max_entries = IFACE_NAME_MAP_SIZE,
    .map_flags = 0,
    .inner_map_idx = 0,
    .numa_node = 0,
};

static inline __u8 check_network_policy(net_bear_type_map_value net_bear_mark_type,
                                        uid_access_policy_value *netAccessPolicyValue)
{
    if (((net_bear_mark_type == NETWORK_BEARER_TYPE_CELLULAR) ||
         (netAccessPolicyValue->netIfIndex == NETWORK_BEARER_TYPE_CELLULAR)) &&
        (!netAccessPolicyValue->cellularPolicy)) {
        return 0;
    }
    if (((net_bear_mark_type == NETWORK_BEARER_TYPE_WIFI) ||
         (netAccessPolicyValue->netIfIndex == NETWORK_BEARER_TYPE_WIFI)) &&
        (!netAccessPolicyValue->wifiPolicy)) {
        return 0;
    }
    if (netAccessPolicyValue->netIfIndex == NETWORK_BEARER_TYPE_INITIAL) {
        void *net_bear_map_ptr = &net_bear_type_map;
        net_bear_id_key net_bear_id = DEFAULT_NETWORK_BEARER_MAP_KEY;
        net_bear_type_map_value *net_bear_type = bpf_map_lookup_elem(net_bear_map_ptr, &net_bear_id);
        if (net_bear_type == NULL) {
            return 1;
        }

        if (((*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR)) && (!netAccessPolicyValue->cellularPolicy)) {
            return 0;
        }
        if (((*net_bear_type == NETWORK_BEARER_TYPE_WIFI)) && (!netAccessPolicyValue->wifiPolicy)) {
            return 0;
        }
    }
    return 1;
}

static inline __u64 check_broker_policy(uint64_t uid)
{
    uint64_t network_access_uid = uid;
    void* broker_map_ptr = &broker_uid_access_policy_map;
    __u32 broker_default_uid = DEFAULT_BROKER_UID_KEY;
    app_uid_key *broker_uid_value = bpf_map_lookup_elem(broker_map_ptr, &broker_default_uid);
    if (broker_uid_value != NULL) {
        network_access_uid = *broker_uid_value;
    }
    return network_access_uid;
}

static inline __u32 filter_sim_stats(__u32 ipv4)
{
    if (IS_MATCHED_IP(ipv4, WLAN_IPv4) || IS_MATCHED_IP(ipv4, CELLULAR_IPv4)) {
        return 1;
    }
    return 0;
}

static inline __u32 get_iface_type(__u32 ipv4)
{
    if (IS_MATCHED_IP(ipv4, WLAN_IPv4)) {
        return IFACE_TYPE_WIFI;
    }
    if (IS_MATCHED_IP(ipv4, CELLULAR_IPv4)) {
        return IFACE_TYPE_CELLULAR;
    }
    return 0;
}

SEC("cgroup_skb/uid/ingress")
int bpf_cgroup_skb_uid_ingress(struct __sk_buff *skb)
{
#ifdef FEATURE_NET_FIREWALL_ENABLE
    if (skb == NULL) {
        return 1;
    }
    if (netfirewall_policy_ingress(skb) != SK_PASS) {
        return SK_DROP;
    }
    if (skb->pkt_type == PACKET_LOOPBACK) {
        return 1;
    }
#else
    if (skb == NULL || skb->pkt_type == PACKET_LOOPBACK) {
        return 1;
    }
#endif

    sock_netns_key key_sock_netns1 = bpf_get_socket_cookie(skb);
    sock_netns_value *value_sock_netns1 = bpf_map_lookup_elem(&sock_netns_map, &key_sock_netns1);
    sock_netns_key key_sock_netns2 = SOCK_COOKIE_ID_NULL;
    sock_netns_value *value_sock_netns2 = bpf_map_lookup_elem(&sock_netns_map, &key_sock_netns2);
    uint64_t sock_uid = bpf_get_socket_uid(skb);
    uint64_t network_access_uid = sock_uid;
    if (value_sock_netns1 != NULL && value_sock_netns2 != NULL && *value_sock_netns1 != *value_sock_netns2) {
        network_access_uid = check_broker_policy(sock_uid);
    }

    uid_access_policy_value *netAccessPolicyValue =
        bpf_map_lookup_elem(&app_uid_access_policy_map, &network_access_uid);
    if (netAccessPolicyValue != NULL) {
        net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(skb->mark);
        if (check_network_policy(net_bear_mark_type, netAccessPolicyValue) == 0) {
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
        __sync_fetch_and_add(&value->rxPackets, 1);
        __sync_fetch_and_add(&value->rxBytes, skb->len);
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

    if ((sock_uid >= SIM_UID_MIN && sock_uid < SIM_UID_MAX) ||
        (value_sock_netns1 != NULL && value_sock_netns2 != NULL && *value_sock_netns1 != *value_sock_netns2)) {
        if (filter_sim_stats(skb->local_ip4) == 1) {
            app_uid_sim_stats_key key_sim = {.uId = sock_uid, .ifIndex = skb->ifindex,
                                             .ifType = get_iface_type(skb->local_ip4)};
            app_uid_sim_stats_value *value_uid_sim = bpf_map_lookup_elem(&app_uid_sim_stats_map, &key_sim);
            if (value_uid_sim == NULL) {
                app_uid_sim_stats_value newValue = {};
                bpf_map_update_elem(&app_uid_sim_stats_map, &key_sim, &newValue, BPF_NOEXIST);
                value_uid_sim = bpf_map_lookup_elem(&app_uid_sim_stats_map, &key_sim);
            }
            if (value_uid_sim != NULL) {
                __sync_fetch_and_add(&value_uid_sim->rxPackets, 1);
                __sync_fetch_and_add(&value_uid_sim->rxBytes, get_data_len(skb));
            }
        }
    } else {
        app_uid_if_stats_key key = {.uId = sock_uid, .ifIndex = skb->ifindex};
        app_uid_if_stats_value *value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
        if (value_uid_if == NULL) {
            app_uid_if_stats_value newValue = {};
            bpf_map_update_elem(&app_uid_if_stats_map, &key, &newValue, BPF_NOEXIST);
            value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
        }
        if (value_uid_if != NULL) {
            __sync_fetch_and_add(&value_uid_if->rxPackets, 1);
            __sync_fetch_and_add(&value_uid_if->rxBytes, get_data_len(skb));
        }
    }
    return 1;
}

SEC("cgroup_skb/uid/egress")
int bpf_cgroup_skb_uid_egress(struct __sk_buff *skb)
{
#ifdef FEATURE_NET_FIREWALL_ENABLE
    if (skb == NULL) {
        return 1;
    }
    if (netfirewall_policy_egress(skb) != SK_PASS) {
        return SK_DROP;
    }
    if (skb->pkt_type == PACKET_LOOPBACK) {
        return 1;
    }
#else
    if (skb == NULL || skb->pkt_type == PACKET_LOOPBACK) {
        return 1;
    }
#endif

    sock_netns_key key_sock_netns1 = bpf_get_socket_cookie(skb);
    sock_netns_value *value_sock_netns1 = bpf_map_lookup_elem(&sock_netns_map, &key_sock_netns1);
    sock_netns_key key_sock_netns2 = SOCK_COOKIE_ID_NULL;
    sock_netns_value *value_sock_netns2 = bpf_map_lookup_elem(&sock_netns_map, &key_sock_netns2);
    uint64_t sock_uid = bpf_get_socket_uid(skb);
    uint64_t network_access_uid = sock_uid;
    if (value_sock_netns1 != NULL && value_sock_netns2 != NULL && *value_sock_netns1 != *value_sock_netns2) {
        network_access_uid = check_broker_policy(sock_uid);
    }
    uid_access_policy_value *netAccessPolicyValue =
        bpf_map_lookup_elem(&app_uid_access_policy_map, &network_access_uid);
    if (netAccessPolicyValue != NULL) {
        net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(skb->mark);
        if (check_network_policy(net_bear_mark_type, netAccessPolicyValue) == 0) {
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

    if ((sock_uid >= SIM_UID_MIN && sock_uid < SIM_UID_MAX) ||
        (value_sock_netns1 != NULL && value_sock_netns2 != NULL && *value_sock_netns1 != *value_sock_netns2)) {
        if (filter_sim_stats(skb->local_ip4) == 1) {
            app_uid_sim_stats_key key_sim = {.uId = sock_uid, .ifIndex = skb->ifindex,
                                             .ifType = get_iface_type(skb->local_ip4)};
            app_uid_sim_stats_value *value_uid_sim = bpf_map_lookup_elem(&app_uid_sim_stats_map, &key_sim);
            if (value_uid_sim == NULL) {
                app_uid_sim_stats_value newValue = {};
                bpf_map_update_elem(&app_uid_sim_stats_map, &key_sim, &newValue, BPF_NOEXIST);
                value_uid_sim = bpf_map_lookup_elem(&app_uid_sim_stats_map, &key_sim);
            }
            if (value_uid_sim != NULL) {
                __sync_fetch_and_add(&value_uid_sim->txPackets, 1);
                __sync_fetch_and_add(&value_uid_sim->txBytes, get_data_len(skb));
            }
        }
    } else {
        app_uid_if_stats_key key = {.uId = sock_uid, .ifIndex = skb->ifindex};
        app_uid_if_stats_value *value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
        if (value_uid_if == NULL) {
            app_uid_if_stats_value newValue = {};
            bpf_map_update_elem(&app_uid_if_stats_map, &key, &newValue, BPF_NOEXIST);
            value_uid_if = bpf_map_lookup_elem(&app_uid_if_stats_map, &key);
        }
        if (value_uid_if != NULL) {
            __sync_fetch_and_add(&value_uid_if->txPackets, 1);
            __sync_fetch_and_add(&value_uid_if->txBytes, get_data_len(skb));
        }
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
    sock_netns_key key_sock_netns1 = bpf_get_socket_cookie(sk);
    sock_netns_value value_sock_netns1 = bpf_get_netns_cookie(sk);
    bpf_map_update_elem(&sock_netns_map, &key_sock_netns1, &value_sock_netns1, BPF_NOEXIST);
    sock_netns_key key_sock_netns2 = SOCK_COOKIE_ID_NULL;
    sock_netns_value value_sock_netns2 = bpf_get_netns_cookie(NULL);
    bpf_map_update_elem(&sock_netns_map, &key_sock_netns2, &value_sock_netns2, BPF_NOEXIST);

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

SEC("cgroup_sock/inet_release_socket")
int inet_release_socket(struct bpf_sock *sk)
{
    sock_netns_key key_sock_netns = bpf_get_socket_cookie(sk);
    bpf_map_delete_elem(&sock_netns_map, &key_sock_netns);

    socket_cookie_stats_key key_sock_cookie = bpf_get_socket_cookie(sk);
    bpf_map_delete_elem(&app_cookie_stats_map, &key_sock_cookie);
    return 1;
}
// internet permission end

bpf_map_def SEC("maps") ringbuf_map = {
    .type = BPF_MAP_TYPE_RINGBUF,
    .max_entries = 256 * 1024 /* 256 KB */,
};

static inline __u8 socket_check_network_policy(net_bear_type_map_value net_bear_mark_type,
                                               net_bear_type_map_value *net_bear_type, uid_access_policy_value *value)
{
    if ((((net_bear_mark_type == NETWORK_BEARER_TYPE_WIFI) || (*net_bear_type == NETWORK_BEARER_TYPE_WIFI)) &&
         (!value->wifiPolicy)) ||
        (((net_bear_mark_type == NETWORK_BEARER_TYPE_CELLULAR) || (*net_bear_type == NETWORK_BEARER_TYPE_CELLULAR)) &&
         (!value->cellularPolicy))) {
        return 0;
    }
    return 1;
}

static inline __u8 socket_ringbuf_event_submit(__u32 uid)
{
    uint32_t *e;
    e = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*e), 0);
    if (e) {
        *e = uid;
        bpf_ringbuf_submit(e, 0);
        return 1;
    }
    return 0;
}

SEC("cgroup_addr/bind4")
static int inet_check_bind4(struct bpf_sock_addr *ctx)
{
    void *map_ptr = &app_uid_access_policy_map;
    __u64 uid_gid = bpf_get_current_uid_gid();
    __u32 uid = (__u32)(uid_gid & 0x00000000FFFFFFFF);
    if (bpf_get_netns_cookie(ctx) != bpf_get_netns_cookie(NULL)) {
        uid = check_broker_policy(uid);
    }

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

    struct bpf_sock *sk = ctx->sk;
    net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(sk->mark);
    if (socket_check_network_policy(net_bear_mark_type, net_bear_type, value) == 0) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag) {
            if (socket_ringbuf_event_submit(uid) != 0) {
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
    if (bpf_get_netns_cookie(ctx) != bpf_get_netns_cookie(NULL)) {
        uid = check_broker_policy(uid);
    }

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

    struct bpf_sock *sk = ctx->sk;
    net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(sk->mark);
    if (socket_check_network_policy(net_bear_mark_type, net_bear_type, value) == 0) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag) {
            if (socket_ringbuf_event_submit(uid) != 0) {
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
    if (bpf_get_netns_cookie(ctx) != bpf_get_netns_cookie(NULL)) {
        uid = check_broker_policy(uid);
    }

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

    struct bpf_sock *sk = ctx->sk;
    net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(sk->mark);
    if (socket_check_network_policy(net_bear_mark_type, net_bear_type, value) == 0) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag) {
            if (socket_ringbuf_event_submit(uid) != 0) {
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
    if (bpf_get_netns_cookie(ctx) != bpf_get_netns_cookie(NULL)) {
        uid = check_broker_policy(uid);
    }

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

    struct bpf_sock *sk = ctx->sk;
    net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(sk->mark);
    if (socket_check_network_policy(net_bear_mark_type, net_bear_type, value) == 0) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag) {
            if (socket_ringbuf_event_submit(uid) != 0) {
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
    if (bpf_get_netns_cookie(ctx) != bpf_get_netns_cookie(NULL)) {
        uid = check_broker_policy(uid);
    }

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

    struct bpf_sock *sk = ctx->sk;
    net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(sk->mark);
    if (socket_check_network_policy(net_bear_mark_type, net_bear_type, value) == 0) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag) {
            if (socket_ringbuf_event_submit(uid) != 0) {
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
    if (bpf_get_netns_cookie(ctx) != bpf_get_netns_cookie(NULL)) {
        uid = check_broker_policy(uid);
    }

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

    struct bpf_sock *sk = ctx->sk;
    net_bear_type_map_value net_bear_mark_type = check_socket_fwmark(sk->mark);
    if (socket_check_network_policy(net_bear_mark_type, net_bear_type, value) == 0) {
        if (value->diagAckFlag) {
            return 0;
        }

        // the policy configuration needs to be reconfirmed or the network bearer changes
        if (value->configSetFromFlag) {
            if (socket_ringbuf_event_submit(uid) != 0) {
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
