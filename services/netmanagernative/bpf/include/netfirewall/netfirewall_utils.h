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
#ifndef NET_FIREWALL_SKB_UTILS_H
#define NET_FIREWALL_SKB_UTILS_H

#include <stdint.h>
#include <sys/socket.h>
#include <bpf/bpf_endian.h>
#include <linux/bpf.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <linux/if_tunnel.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tc_act/tc_bpf.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <stdbool.h>

#define TCP_FLAGS_OFFSET 12
#define TCP_FLAGS_SIZE 2

#define L3_NHOFF 0
#define L4_NHOFF(ipv4) (L3_NHOFF + ((ipv4) ? sizeof(struct iphdr) : sizeof(struct ipv6hdr)))

struct grehdr {
    __be16 flags;
    __be16 proto;
};

union tcp_flags {
    struct {
        __u8 upper_bits;
        __u8 lower_bits;
        __u16 pad;
    };
    __u32 value;
};

/**
 * @brief judget given skb is a layler 4 protocol or not
 *
 * @param skb struct __sk_buff
 * @param l3_nhoff layer 3 network header offset
 * @param protocol layer 4 protocol number
 * @return true if match, otherwise false
 */
static __always_inline bool is_l4_protocol(struct __sk_buff *skb, __u32 l3_nhoff, __u8 protocol)
{
    if (skb->family == AF_INET) {
        struct iphdr iph = { 0 };
        bpf_skb_load_bytes(skb, l3_nhoff, &iph, sizeof(struct iphdr));

        return iph.protocol == protocol;
    }
    if (skb->family == AF_INET6) {
        struct ipv6hdr ip6h = { 0 };
        bpf_skb_load_bytes(skb, l3_nhoff, &ip6h, sizeof(struct ipv6hdr));

        return ip6h.nexthdr == protocol;
    }

    return false;
}

/**
 * @brief load gre info from given skb
 *
 * @param skb struct __sk_buff
 * @param inner_l3_nhoff layer 3 network header offset
 * @param eth_proto gre inner layer3 protocol
 * @return true if success or false if an error occurred
 */
static __always_inline bool load_gre_info(struct __sk_buff *skb, __u32 *inner_l3_nhoff, __u16 *eth_proto)
{
    if (!is_l4_protocol(skb, L3_NHOFF, IPPROTO_GRE)) {
        return false;
    }

    __u32 nh_off = L4_NHOFF(skb->family == AF_INET);
    struct grehdr greh = { 0 };
    bpf_skb_load_bytes(skb, nh_off, &greh, sizeof(struct grehdr));

    *eth_proto = bpf_ntohs(greh.proto);
    __u16 gre_flags = bpf_ntohs(greh.flags);

    nh_off += sizeof(struct grehdr);
    if (gre_flags & GRE_CSUM) {
        nh_off += 4;
    }
    if (gre_flags & GRE_KEY) {
        nh_off += 4;
    }
    if (gre_flags & GRE_SEQ) {
        nh_off += 4;
    }
    *inner_l3_nhoff = nh_off;
    return true;
}

/**
 * Get the layer 3 network header offset and if first layer 3 is gre protocol then get gre inner layer 3 network header
 * offset
 *
 * @param skb struct __sk_buff
 * @return layer 3 network header offset
 */
static __always_inline __u32 get_l3_nhoff(struct __sk_buff *skb)
{
    __u32 inner_l3_nhoff = 0;
    __u16 inner_eth_proto = 0;
    bool is_gre = load_gre_info(skb, &inner_l3_nhoff, &inner_eth_proto);
    if (is_gre) {
        return inner_l3_nhoff;
    }

    return L3_NHOFF;
}

/**
 * Get the layer 4 network header offset and if first layer 4 is gre protocol then get gre inner layer 4 network header
 * offset
 *
 * @param skb struct __sk_buff
 * @return layer 4 network header offset
 */
static __always_inline __u32 get_l4_nhoff(struct __sk_buff *skb)
{
    __u32 l4_nhoff = 0;
    __u32 inner_l3_nhoff = 0;
    __u16 inner_eth_proto = 0;
    bool is_gre = load_gre_info(skb, &inner_l3_nhoff, &inner_eth_proto);
    if (is_gre) {
        if (inner_eth_proto == ETH_P_IP) {
            l4_nhoff = inner_l3_nhoff + sizeof(struct iphdr);
        } else if (inner_eth_proto == ETH_P_IPV6) {
            l4_nhoff = inner_l3_nhoff + sizeof(struct ipv6hdr);
        }
        return l4_nhoff;
    }

    return L4_NHOFF(skb->family == AF_INET);
}

/**
 * @brief load tcp flags from given skb
 *
 * @param skb struct __sk_buff
 * @param l4_nhoff layer 4 network header offset
 * @param flags union tcp_flags
 * @return __always_inline
 */
static __always_inline int load_tcp_flags(struct __sk_buff *skb, __u32 l4_nhoff, union tcp_flags *flags)
{
    return bpf_skb_load_bytes(skb, l4_nhoff + TCP_FLAGS_OFFSET, flags, TCP_FLAGS_SIZE);
}

/**
 * @brief load layer 4 protocol from given skb
 *
 * @param skb struct __sk_buff
 * @param l3_nhoff layer 3 network header offset
 * @param protocol out layer 4 protocol
 * @return true if success or false if an error occurred
 */
static __always_inline bool load_l4_protocol(const struct __sk_buff *skb, __u32 l3_nhoff, __u8 *protocol)
{
    if (skb->family == AF_INET) {
        struct iphdr iph = { 0 };
        bpf_skb_load_bytes(skb, l3_nhoff, &iph, sizeof(struct iphdr));

        *protocol = iph.protocol;
        return true;
    } else if (skb->family == AF_INET6) {
        struct ipv6hdr ip6h = { 0 };
        bpf_skb_load_bytes(skb, l3_nhoff, &ip6h, sizeof(struct ipv6hdr));

        *protocol = ip6h.nexthdr;
        return true;
    }

    return false;
}

/**
 * @brief load layer 3 ipv4 addrs from given skb
 *
 * @param skb struct __sk_buff
 * @param l3_nhoff layer 3 network header offset
 * @param saddr out saddr
 * @param daddr out daddr
 * @return true if success or false if an error occurred
 */
static __always_inline bool load_l3_v4_addrs(const struct __sk_buff *skb, __u32 l3_nhoff, __be32 *saddr, __be32 *daddr)
{
    if (skb->family != AF_INET) {
        return false;
    }

    struct iphdr iph = { 0 };
    bpf_skb_load_bytes(skb, l3_nhoff, &iph, sizeof(struct iphdr));

    *saddr = iph.saddr;
    *daddr = iph.daddr;

    return true;
}

/**
 * @brief load layer 3 ipv6 addrs from given skb
 *
 * @param skb struct __sk_buff
 * @param l3_nhoff layer 3 network header offset
 * @param saddr out saddr
 * @param daddr out daddr
 * @return true if success or false if an error occurred
 */
static __always_inline bool load_l3_v6_addrs(const struct __sk_buff *skb, __u32 l3_nhoff, struct in6_addr *saddr,
    struct in6_addr *daddr)
{
    if (skb->family != AF_INET6) {
        return false;
    }

    struct ipv6hdr ip6h = { 0 };
    bpf_skb_load_bytes(skb, l3_nhoff, &ip6h, sizeof(struct ipv6hdr));

    *saddr = ip6h.saddr;
    *daddr = ip6h.daddr;

    return true;
}

/**
 * @brief load layer 4 icmp info from given skb
 *
 * @param skb struct __sk_buff
 * @param l4_nhoff layer 4 network header offset
 * @param protocol IPPROTO_ICMP or IPPROTO_ICMPV6
 * @param type out icmp type
 * @param code out icmp code
 * @return true if success or false if an error occurred
 */
static __always_inline bool load_icmp_info(const struct __sk_buff *skb, __u32 l4_nhoff, __u8 protocol, __u8 *type,
    __u8 *code)
{
    bool res = true;
    switch (protocol) {
        case IPPROTO_ICMP: {
            struct icmphdr icmph = { 0 };
            bpf_skb_load_bytes(skb, l4_nhoff, &icmph, sizeof(struct icmphdr));

            *type = icmph.type;
            *code = icmph.code;
        } break;
        case IPPROTO_ICMPV6: {
            struct icmp6hdr icmph = { 0 };
            bpf_skb_load_bytes(skb, l4_nhoff, &icmph, sizeof(struct icmp6hdr));

            *type = icmph.icmp6_type;
            *code = icmph.icmp6_code;
        } break;
        default:
            res = false;
            break;
    }

    return res;
}

/**
 * @brief load layer 4 ports from given skb
 *
 * @param skb struct __sk_buff
 * @param l4_nhoff layer 4 network header offset
 * @param protocol IPPROTO_TCP or IPPROTO_UDP
 * @param sport out sport
 * @param dport out dport
 * @return true if success or false if an error occurred
 */
static __always_inline bool load_l4_ports(const struct __sk_buff *skb, __u32 l4_nhoff, __u8 protocol, __be16 *sport,
    __be16 *dport)
{
    bool res = true;
    switch (protocol) {
        case IPPROTO_TCP: {
            struct tcphdr tcph = { 0 };
            bpf_skb_load_bytes(skb, l4_nhoff, &tcph, sizeof(struct tcphdr));
            *sport = tcph.source;
            *dport = tcph.dest;
        } break;
        case IPPROTO_UDP: {
            struct udphdr udph = { 0 };
            bpf_skb_load_bytes(skb, l4_nhoff, &udph, sizeof(struct udphdr));
            *sport = udph.source;
            *dport = udph.dest;
        } break;
        default:
            res = false;
            break;
    }

    return res;
}

/**
 * @brief load layer 4 rst  from given skb
 *
 * @param skb struct __sk_buff
 * @param l4_nhoff layer 4 network header offset
 * @param rst out rst packet
 * @return true if success or false if an error occurred
 */
static __always_inline bool load_l4_header_flags(const struct __sk_buff *skb, __u32 l4_nhoff, __u16 *rst)
{
    struct tcphdr tcph = { 0 };
    bpf_skb_load_bytes(skb, l4_nhoff, &tcph, sizeof(struct tcphdr));
    *rst = tcph.rst;
    return true;
}

#endif // NET_FIREWALL_SKB_UTILS_H