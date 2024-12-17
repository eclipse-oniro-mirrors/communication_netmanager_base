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
#ifndef NET_FIREWALL_DOMAIN_H
#define NET_FIREWALL_DOMAIN_H

#include <bpf/bpf_helpers.h>
#include <linux/bpf.h>
#include <linux/udp.h>

#include "netfirewall_event.h"
#include "netfirewall_match.h"
#include "netfirewall_map.h"
#include "netfirewall_types.h"
#include "netfirewall_utils.h"

static __always_inline __u32 get_current_uid(struct __sk_buff *skb)
{
    if (skb == NULL) {
        return DEFAULT_USER_ID;
    }
    __u32 sock_uid = bpf_get_socket_uid(skb);
    return get_user_id(sock_uid);
}

static __always_inline bool add_domain_cache(struct __sk_buff *skb, const __u8 *payload, const __u32 family)
{
    bool ret = false;
    struct domain_value value = { 0 };
    value.appuid = bpf_get_socket_uid(skb);
    value.uid = get_current_uid(skb);
    if (family == AF_INET) {
        struct ipv4_lpm_key lpm_key = {
            .prefixlen = IPV4_MAX_PREFIXLEN,
        };
        memcpy(&(lpm_key.data), payload, sizeof(lpm_key.data));
        ret = bpf_map_update_elem(&DOMAIN_IPV4_MAP, &lpm_key, &value, 0) == 0;
    } else if (family == AF_INET6) {
        struct ipv6_lpm_key lpm_key = {
            .prefixlen = IPV6_MAX_PREFIXLEN,
        };
        memcpy(&(lpm_key.data), payload, sizeof(lpm_key.data));
        ret = bpf_map_update_elem(&DOMAIN_IPV6_MAP, &lpm_key, &value, 0) == 0;
    }
    return ret;
}

static __always_inline __u16 parse_queries_name(struct __sk_buff *skb, __u16 dns_qry_off, __u8 *key_data,
    __u16 *key_len)
{
    __u8 offset = dns_qry_off;
    __u8 i = 0;

    for (; i < DNS_DOMAIN_LEN; i++) {
        bpf_skb_load_bytes(skb, offset, key_data + i, sizeof(__u8));
        offset += 1;
        if (*(key_data + i) == 0) {
            *key_len = i;
            break;
        }
    }
    if (i == DNS_DOMAIN_LEN) {
        return 0;
    }

    return offset;
}

static __always_inline __u16 parse_queries(struct __sk_buff *skb, __u16 dns_qry_off, __u8 *key_data,
    __u16 *key_len)
{
    __u16 offset = parse_queries_name(skb, dns_qry_off, key_data, key_len);
    if (offset == 0) {
        return 0;
    }
    // Type & Class
    offset += sizeof(__u32);

    return offset;
}

static __always_inline __u16 parse_answers(struct __sk_buff *skb, __u16 dns_qry_off, __u8 save_ip)
{
    __u16 type;
    __u16 str_len;
    __u16 offset = dns_qry_off;

    // Name : u16
    offset += sizeof(__u16);
    // Type : u16
    bpf_skb_load_bytes(skb, offset, &type, sizeof(__u16));
    type = bpf_ntohs(type);
    offset += sizeof(__u16);
    // Class : u16
    offset += sizeof(__u16);
    // Ttl : u32
    offset += sizeof(__u32);
    // Data len : u16
    bpf_skb_load_bytes(skb, offset, &str_len, sizeof(__u16));
    str_len = bpf_ntohs(str_len);
    offset += sizeof(__u16);
    if (str_len == 0) {
        return offset;
    }
    // ipv4 or ipv6 addr
    if (type == DNS_QRS_IPV4_TYPE && str_len == DNS_QRS_IPV4_LEN) {
        __u32 addr;
        bpf_skb_load_bytes(skb, offset, &addr, sizeof(__u32));
        if (save_ip) {
            add_domain_cache(skb, (__u8*)&addr, AF_INET);
        }
    } else if (type == DNS_QRS_IPV6_TYPE && str_len == DNS_QRS_IPV6_LEN) {
        ip6_key ip6_addr;
        bpf_skb_load_bytes(skb, offset, &ip6_addr, sizeof(ip6_key));
        if (save_ip) {
            add_domain_cache(skb, (__u8*)&ip6_addr, AF_INET6);
        }
    }
    offset += str_len;

    return offset;
}

static __always_inline bool match_domain_value(struct __sk_buff *skb, const struct domain_value *value)
{
    if (skb == NULL || value == NULL) {
        return false;
    }

    if (value->uid == get_current_uid(skb) &&
        (value->appuid == bpf_get_socket_uid(skb) || value->appuid == 0)) {
        return true;
    }
    return false;
}

static __always_inline __u8 parse_dns_query(struct __sk_buff *skb, __u16 dns_qry_off, __u16 qu_num)
{
    if (qu_num == 1) {
        __u16 res;
        __u16 key_len = 0;
        struct domain_hash_key key = { 0 };

        res = parse_queries(skb, dns_qry_off, (__u8*)&key.data, &key_len);
        if (res == 0) {
            return 0;
        }

        struct domain_value *deny_value = bpf_map_lookup_elem(&DOMAIN_DENY_MAP, &key);

        if (deny_value != NULL && match_domain_value(skb, deny_value)) {
            return 1;
        }
    }
    return 0;
}

static __always_inline __u16 parse_dns_response(struct __sk_buff *skb, __u16 dns_qry_off, __u16 qu_num,
    __u16 as_num)
{
    __u16 offset;
    bool is_in_pass = 0;

    if (qu_num == 1) {
        __u16 key_len = 0;
        struct domain_hash_key key = { 0 };

        offset = parse_queries(skb, dns_qry_off, (__u8*)&key.data, &key_len);
        if (offset == 0) {
            return 0;
        }

        struct domain_value *deny_value = bpf_map_lookup_elem(&DOMAIN_DENY_MAP, &key);
        if (deny_value != NULL && match_domain_value(skb, deny_value)) {
            return 1;
        }

        struct domain_value *allow_value = bpf_map_lookup_elem(&DOMAIN_PASS_MAP, &key);
        __u32 current_uid = get_current_uid(skb);
        struct defalut_action_value *default_value = bpf_map_lookup_elem(&DEFAULT_ACTION_MAP, &current_uid);
        enum sk_action sk_act = SK_PASS;
        if (default_value) {
            sk_act = default_value->outaction;
        }

        if ((allow_value != NULL && match_domain_value(skb, allow_value)) && (sk_act == SK_DROP)) {
            is_in_pass = 1;
            log_dbg(DBG_MATCH_DOMAIN_ACTION, EGRESS, SK_PASS);
        } else {
            return 0;
        }
    } else {
        // not support multi questions
        return 0;
    }

    for (__u16 i = 0; i < DNS_ANSWER_CNT; i++) {
        if (i >= as_num) {
            break;
        }
        offset = parse_answers(skb, offset, is_in_pass);
    }

    return 0;
}

static __always_inline enum sk_action match_dns_query(struct __sk_buff *skb)
{
    if (!skb) {
        return SK_PASS;
    }

    __u8 protocol = 0;
    __u32 l4_nhoff = 0;
    if (skb->family == AF_INET) {
        struct iphdr iph = { 0 };
        bpf_skb_load_bytes(skb, 0, &iph, sizeof(struct iphdr));
        protocol = iph.protocol;
        l4_nhoff = sizeof(struct iphdr);
    } else if (skb->family == AF_INET6) {
        struct ipv6hdr ip6h = { 0 };
        bpf_skb_load_bytes(skb, 0, &ip6h, sizeof(struct ipv6hdr));
        protocol = ip6h.nexthdr;
        l4_nhoff = sizeof(struct ipv6hdr);
    }

    if (protocol == IPPROTO_UDP) {
        __u16 src_port;
        __u16 dst_port;
        struct udphdr udph = { 0 };
        bpf_skb_load_bytes(skb, l4_nhoff, &udph, sizeof(struct udphdr));

        src_port = bpf_ntohs(udph.source);
        dst_port = bpf_ntohs(udph.dest);
        if (src_port == DNS_PORT || dst_port == DNS_PORT) {
            struct dnshdr dnsh = { 0 };
            bpf_skb_load_bytes(skb, l4_nhoff + sizeof(struct udphdr), &dnsh, sizeof(struct dnshdr));
            __u16 qu_num = bpf_ntohs(dnsh.qdcount);
            __u16 as_num = bpf_ntohs(dnsh.ancount);

            __u16 dns_qry_off = l4_nhoff + sizeof(struct udphdr) + sizeof(struct dnshdr);
            __u16 flag = bpf_ntohs(dnsh.flag);
            __u16 bit_mask = (1 << DNS_QR_DEFALUT_MASK);
            __u16 qr = (flag & bit_mask);
            __u16 res = 0;
            if (qr == bit_mask) {
                res = parse_dns_response(skb, dns_qry_off, qu_num, as_num);
            } else {
                res = parse_dns_query(skb, dns_qry_off, qu_num);
            }
            if (res == 1) {
                log_dbg(DBG_MATCH_DOMAIN_ACTION, EGRESS, SK_DROP);
                return SK_DROP;
            }
        }
        return SK_PASS;
    }
    return SK_PASS;
}

#endif // NET_FIREWALL_DOMAIN_H