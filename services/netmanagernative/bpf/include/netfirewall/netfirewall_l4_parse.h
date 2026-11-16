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
#ifndef NET_FIREWALL_L4_PARSE_H
#define NET_FIREWALL_L4_PARSE_H

/* Unified tri-state L4 parser. Kept free of any struct iphdr/ipv6hdr/tcphdr
 * dependency on purpose: the whole header reads raw bytes, so it compiles into
 * user-space test/fuzzer TUs where the musl netinet structs (netinet/ip.h,
 * tcp.h, ip_icmp.h) clash with the kernel UAPI ones (C++ redefinition), while
 * <netfirewall_utils.h> (which needs those structs for its loaders) stays
 * BPF/test-side only.
 *
 * This header does not declare bpf_skb_load_bytes itself: the BPF build gets
 * it from bpf_helpers.h (included ahead by consumers), user-space TUs must
 * declare their mock stub before including this header. */

#include "netfirewall_types.h"

#define L3_NHOFF 0

#define VERSION_IPV4 4
#define VERSION_OFFSET 4

/* ===== Parser constants ===== */
#define IPV4_FRAG_OFFSET_MASK 0x1fff
#define IPV6_EXT_MAX_HOPS 8
#define IPV6_EXT_MAX_BYTES 128
#define IPV6_BASE_HEADER_LEN 40
/* shared TCP/UDP port prefix (sport+dport) the firewall reads from the L4
 * header; 4 bytes keeps minimal legal UDP/ICMP packets (IHL*4 + 8) parseable. */
#define L4_MIN_HDR_LEN 4

/* byte offsets inside the 20-byte IPv4 base header */
#define IPV4_HDR_VERSION_IHL 0
#define IPV4_HDR_TOT_LEN 2
#define IPV4_HDR_FRAG_OFF 6
#define IPV4_HDR_PROTOCOL 9

/*
 * skb->family MUST be read exactly once per program entry (skb_family() in
 * netsys.c) and threaded down to every consumer as a __u32 scalar.
 * Rationale: clang -O2 CSEs the address &skb->family (ctx + 88) across all
 * inlined readers; under register pressure the modified ctx pointer is spilled
 * to the stack and reloaded for the dereference, and the BPF verifier rejects
 * exactly that construct ("dereference of a modified ctx ptr", program load
 * fails with EACCES). A volatile read does NOT prevent it — the GEP address
 * computation is a pure instruction and is CSE'd regardless (device-verified).
 * One read per program keeps the offset folded into the LDX immediate, so no
 * ctx+N pointer is ever materialized; scalar values spill and reload freely.
 */
/**
 * @brief judget given skb is ipv4 packet, even if its family is AF_INET6
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff
 * @return true if it is ipv4 packet, otherwise false
 */
static __always_inline bool is_ipv4_format(__u32 family, const struct __sk_buff *skb)
{
    if (!skb) {
        return false;
    }
    if (family == AF_INET) {
        return true;
    }
    if (family == AF_INET6) {
        __u8 version = { 0 };
        bpf_skb_load_bytes(skb, 0, &version, sizeof(__u8));
        return ((version >> VERSION_OFFSET) == VERSION_IPV4);
    }
    return false;
}

/* Parser output. */
struct l4_info {
    enum l4_parse_state state;
    __u32 l4_nhoff; /* valid when state == L4_PARSE_OK */
    __u8 protocol;  /* valid when state == L4_PARSE_OK (terminal L4 protocol) */
};

/**
 * @brief locate the real layer 4 header. IPv4: validate version/IHL/total
 *        length, then l4_nhoff = IHL*4; non-zero fragment offset -> NO_L4
 *        (first fragment with MF set still parses). IPv6: walk the extension
 *        header chain, at most IPV6_EXT_MAX_HOPS hops and IPV6_EXT_MAX_BYTES
 *        bytes of headers; HBH/routing/destination options advance
 *        8*(hdrlen+1), AH advances 4*(hdrlen+2) (different unit and base — the
 *        classic trap), the fragment header decides first (continue with inner
 *        nexthdr) vs non-first (NO_L4); ESP and unknown types, or any
 *        out-of-bounds hop, end as UNPARSEABLE. Bounds first, reads second
 *        (historical failure c3d1d423): every load is offset-checked and its
 *        failure mapped to a state, never a partial misread.
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff
 * @param out struct l4_info
 * @return false only when the skb is not an IP packet we understand at all
 *         (family neither IPv4-format nor AF_INET6 — same corner
 *         get_match_tuple historically failed on); true otherwise, with the
 *         verdict in out->state.
 */
static __always_inline bool parse_l4(__u32 family, struct __sk_buff *skb, struct l4_info *out)
{
    if (!skb || !out) {
        return false;
    }
    out->state = L4_PARSE_UNPARSEABLE;
    out->l4_nhoff = 0;
    out->protocol = 0;

    if (is_ipv4_format(family, skb)) {
        /* bytes 0 (version|IHL), 2-3 (tot_len), 6-7 (frag_off), 9 (protocol) */
        __u8 v4_hdr[10] = { 0 };
        if (bpf_skb_load_bytes(skb, L3_NHOFF, v4_hdr, sizeof(v4_hdr)) < 0) {
            return true;
        }
        __u8 ihl = v4_hdr[IPV4_HDR_VERSION_IHL] & 0x0F;
        __u8 version = v4_hdr[IPV4_HDR_VERSION_IHL] >> VERSION_OFFSET;
        if (version != VERSION_IPV4 || ihl < 5) {
            return true;
        }
        __u32 l4_nhoff = L3_NHOFF + (__u32)ihl * 4;
        __u16 tot_len = ((__u16)v4_hdr[IPV4_HDR_TOT_LEN] << 8) | (__u16)v4_hdr[IPV4_HDR_TOT_LEN + 1];
        /* the port prefix must fit both the declared total length and the real
         * skb length before any consumer reads it */
        if (l4_nhoff + L4_MIN_HDR_LEN > tot_len || l4_nhoff + L4_MIN_HDR_LEN > skb->len) {
            return true;
        }
        __u16 frag_off = ((__u16)v4_hdr[IPV4_HDR_FRAG_OFF] << 8) | (__u16)v4_hdr[IPV4_HDR_FRAG_OFF + 1];
        if (frag_off & IPV4_FRAG_OFFSET_MASK) {
            out->state = L4_PARSE_NO_L4;
            out->protocol = v4_hdr[IPV4_HDR_PROTOCOL]; /* real protocol: IP header is in every fragment */
            return true;
        }
        out->state = L4_PARSE_OK;
        out->l4_nhoff = l4_nhoff;
        out->protocol = v4_hdr[IPV4_HDR_PROTOCOL];
        return true;
    }

    if (family != AF_INET6) {
        return false;
    }

    /* bytes 0 (version|tc) and 6 (nexthdr) of the 40-byte base header */
    __u8 v6_hdr[8] = { 0 };
    if (bpf_skb_load_bytes(skb, L3_NHOFF, v6_hdr, sizeof(v6_hdr)) < 0) {
        return true;
    }
    __u8 nexthdr = v6_hdr[6];
    __u32 offset = L3_NHOFF + IPV6_BASE_HEADER_LEN;
    for (int hop = 0; hop < IPV6_EXT_MAX_HOPS; hop++) {
        switch (nexthdr) {
            case IPPROTO_TCP:
            case IPPROTO_UDP:
            case IPPROTO_ICMP:
            case IPPROTO_ICMPV6:
                if (offset + L4_MIN_HDR_LEN > skb->len) {
                    return true;
                }
                out->state = L4_PARSE_OK;
                out->l4_nhoff = offset;
                out->protocol = nexthdr;
                return true;
            case IPPROTO_HOPOPTS:
            case IPPROTO_ROUTING:
            case IPPROTO_DSTOPTS: {
                __u8 ext[2] = { 0, 0 };
                if (bpf_skb_load_bytes(skb, offset, ext, sizeof(ext)) < 0) {
                    return true;
                }
                nexthdr = ext[0];
                offset += 8 * ((__u32)ext[1] + 1);
                break;
            }
            case IPPROTO_AH: {
                __u8 ext[2] = { 0, 0 };
                if (bpf_skb_load_bytes(skb, offset, ext, sizeof(ext)) < 0) {
                    return true;
                }
                nexthdr = ext[0];
                /* AH length field counts 4-byte units excluding 8 fixed bytes:
                 * (hdrlen + 2) * 4 — NOT the 8*(hdrlen+1) of the other headers. */
                offset += 4 * ((__u32)ext[1] + 2);
                break;
            }
            case IPPROTO_FRAGMENT: {
                __u8 frag[8] = { 0 };
                if (bpf_skb_load_bytes(skb, offset, frag, sizeof(frag)) < 0) {
                    return true;
                }
                __u16 frag_off = ((__u16)frag[2] << 8) | (__u16)frag[3];
                if (frag_off & IPV4_FRAG_OFFSET_MASK) {
                    out->state = L4_PARSE_NO_L4;
                    out->protocol = frag[0]; /* best-effort inner protocol (diagnostics) */
                    return true;
                }
                /* first fragment: the real L4 follows the 8-byte fragment header */
                nexthdr = frag[0];
                offset += 8;
                break;
            }
            default:
                /* ESP (IPPROTO_ESP) and unknown types: cannot safely locate L4 */
                return true;
        }
        if (offset - (L3_NHOFF + IPV6_BASE_HEADER_LEN) > IPV6_EXT_MAX_BYTES) {
            return true;
        }
    }
    /* more than IPV6_EXT_MAX_HOPS extension headers */
    return true;
}

#endif // NET_FIREWALL_L4_PARSE_H
