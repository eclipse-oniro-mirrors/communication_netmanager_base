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
#ifndef NET_FIREWALL_CT_H
#define NET_FIREWALL_CT_H

#include "netfirewall_utils.h"
#include "netfirewall_ct_def.h"
#include "netfirewall_ct_map.h"

/* __typeof__ (not plain typeof): the BPF C build runs in gnu mode where
 * both spellings work, but the host C++ unit-test TU compiles this header
 * under -std=c++17 where plain typeof is not a keyword; __typeof__ is
 * accepted by clang in every language mode. */
#define READ_ONCE(x) (*(volatile __typeof__(x) *)&(x))
#define WRITE_ONCE(x, v) (*(volatile __typeof__(x) *)&(x)) = (v)

/* The evaluated bits are bit-field members, which cannot be addressed
 * (READ_ONCE/WRITE_ONCE take &(x)), so the hooks access them through the
 * enclosing flag byte: offsetof(tx_seen_flag) - 1 is the byte holding the
 * bit-fields (little-endian, single __u8 unit). Masks are defined beside the
 * bit-field layout in netfirewall_ct_def.h (single source of truth). */
#define CT_FLAGS_BYTE(entry) ((__u8 *)(entry) + __builtin_offsetof(struct ct_entry, tx_seen_flag) - 1)

static __always_inline bool ct_evaluated(const struct ct_entry *entry, enum ct_dir dir)
{
    __u8 flags = READ_ONCE(*CT_FLAGS_BYTE(entry));
    return (flags & (dir == CT_EGRESS ? CT_EGRESS_EVALUATED_MASK : CT_INGRESS_EVALUATED_MASK)) != 0;
}

static __always_inline void ct_set_evaluated(struct ct_entry *entry, enum ct_dir dir)
{
    __u8 *flagsByte = CT_FLAGS_BYTE(entry);
    __u8 flags = READ_ONCE(*flagsByte);
    WRITE_ONCE(*flagsByte, flags | (dir == CT_EGRESS ? CT_EGRESS_EVALUATED_MASK : CT_INGRESS_EVALUATED_MASK));
}

static __always_inline void reset_seen_flags(struct ct_entry *entry)
{
    entry->rx_seen_flag = 0;
    entry->tx_seen_flag = 0;
}

static __always_inline void reset_closing_flags(struct ct_entry *entry)
{
    entry->rx_closing_flag = 0;
    entry->tx_closing_flag = 0;
}

static __always_inline bool is_conn_alive(const struct ct_entry *entry)
{
    return !entry->rx_closing_flag || !entry->tx_closing_flag;
}

static __always_inline bool is_conn_closing(const struct ct_entry *entry)
{
    return entry->tx_closing_flag || entry->rx_closing_flag;
}

static __always_inline enum ct_action get_tcp_conn_action(union tcp_flags flags)
{
    if (flags.value & (TCP_FLAG_RST | TCP_FLAG_FIN)) {
        return CT_ACTION_CLOSE;
    }

    if (flags.value & TCP_FLAG_SYN) {
        return CT_ACTION_CREATE;
    }

    return CT_ACTION_UNSPEC;
}

static __always_inline bool is_seen_both_syns(const struct ct_entry *entry)
{
    bool rx_syn = entry->rx_seen_flag & TCP_FLAG_SYN;
    bool tx_syn = entry->tx_seen_flag & TCP_FLAG_SYN;

    return rx_syn && tx_syn;
}

static __always_inline bool update_timeout_inner(struct ct_entry *entry, __u32 lifetime, enum ct_dir dir,
    union tcp_flags flags)
{
    __u32 now = bpf_ktime_get_ns() / NS_PER_SEC;
    __u8 last_seen_flags;
    __u8 seen_flags = flags.lower_bits & REPORT_FLAGS;
    __u32 last_report;

    WRITE_ONCE(entry->lifetime, now + lifetime);

    if (dir == CT_INGRESS) {
        last_seen_flags = READ_ONCE(entry->rx_seen_flag);
        last_report = READ_ONCE(entry->last_rx_report);
    } else {
        last_seen_flags = READ_ONCE(entry->tx_seen_flag);
        last_report = READ_ONCE(entry->last_tx_report);
    }
    seen_flags |= last_seen_flags;
    if ((last_report + REPORT_INTERVAL_SEC) < now || last_seen_flags != seen_flags) {
        if (dir == CT_INGRESS) {
            WRITE_ONCE(entry->rx_seen_flag, seen_flags);
            WRITE_ONCE(entry->last_rx_report, now);
        } else {
            WRITE_ONCE(entry->tx_seen_flag, seen_flags);
            WRITE_ONCE(entry->last_tx_report, now);
        }
        return true;
    }
    return false;
}

/**
 * @brief Update the CT timeouts for the specified entry.
 *
 * @param entry struct ct_entry
 * @param tcp tcp connection
 * @param dir enum ct_dir
 * @param seen_flags union tcp_flags
 * @return If REPORT_INTERVAL_SEC has elapsed since the last update, updates the last_updated timestamp
 *     and returns true. Otherwise returns false.
 */
static __always_inline bool ct_update_timeout(struct ct_entry *entry, bool tcp, enum ct_dir dir,
    union tcp_flags seen_flags)
{
    __u32 timeout = NONTCP_CONN_TIMEOUT_SEC;
    bool syn = seen_flags.value & TCP_FLAG_SYN;

    if (tcp) {
        entry->seen_non_syn |= !syn;
        if (entry->seen_non_syn) {
            timeout = TCP_CONN_TIMEOUT_SEC;
        } else {
            timeout = TCP_SYN_TIMEOUT_SEC;
        }
    }

    return update_timeout_inner(entry, timeout, dir, seen_flags);
}

/**
 * @brief create a key pair of ct_tuple/ct_entry and add to ct map
 *
 * @param tuple struct ct_tuple
 * @param skb struct __sk_buff
 * @param dir enum ct_dir
 * @return true if success or false if an error occurred
 */
static __always_inline bool ct_create_entry(struct ct_tuple *tuple, struct __sk_buff *skb, const enum ct_dir dir)
{
    struct ct_entry entry = { 0 };
    bool is_tcp = (tuple->protocol == IPPROTO_TCP);
    union tcp_flags seen_flags = {
        .value = 0
    };

    seen_flags.value |= is_tcp ? TCP_FLAG_SYN : 0;
    ct_update_timeout(&entry, is_tcp, dir, seen_flags);
    /* The creating direction has just passed the full rule evaluation
     * (ct_create_entry is only called on the PASS path of a CT_NEW packet). */
    if (dir == CT_EGRESS) {
        entry.egress_evaluated = 1;
    } else {
        entry.ingress_evaluated = 1;
    }

    return bpf_map_update_elem(&CT_MAP, tuple, &entry, 0) == 0;
}

/**
 * @brief lookup from ct map by ct_tuple if found then update lifetime of connection
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff
 * @param tuple struct ct_tuple
 * @param dir enum ct_dir
 * @param entry_out optional out param: the hit ct_entry; NULL when the
 *        entry is deleted inside (stale pointer) or on CT_NEW. The loopback swap
 *        in ct_map_lookup_entry only mutates the tuple, never the value.
 * @return CT_NEW if not found, otherwise CT_RELATED, CT_REOPENED or CT_ESTABLISHED
 */
static __always_inline enum ct_status ct_lookup_entry(__u32 family, struct __sk_buff *skb, const struct ct_tuple *tuple,
    enum ct_dir dir, struct ct_entry **entry_out)
{
    if (entry_out != NULL) {
        *entry_out = NULL;
    }
    struct ct_entry *entry = bpf_map_lookup_elem(&CT_MAP, tuple);
    if (entry) {
        if (entry_out != NULL) {
            *entry_out = entry;
        }
        __u32 l3_nhoff = get_l3_nhoff(skb);
        bool is_tcp = is_l4_protocol(family, skb, l3_nhoff, IPPROTO_TCP);
        union tcp_flags seen_flags = {};
        if (is_tcp) {
            __u32 l4_nhoff = get_l4_nhoff(family, skb);
            if (load_tcp_flags(skb, l4_nhoff, &seen_flags) < 0) {
                return CT_RELATED;
            }
        }
        if (is_conn_alive(entry)) {
            ct_update_timeout(entry, is_tcp, dir, seen_flags);
        }
        enum ct_action action = get_tcp_conn_action(seen_flags);
        switch (action) {
            case CT_ACTION_CREATE:
                if (is_conn_closing(entry)) {
                    reset_closing_flags(entry);
                    reset_seen_flags(entry);
                    entry->seen_non_syn = false;
                    ct_update_timeout(entry, is_tcp, dir, seen_flags);
                    return CT_REOPENED;
                }
                break;
            case CT_ACTION_CLOSE:
                if (!is_seen_both_syns(entry) && (seen_flags.value & TCP_FLAG_RST)) {
                    entry->rx_closing_flag = 1;
                    entry->tx_closing_flag = 1;
                } else if (dir == CT_INGRESS) {
                    entry->rx_closing_flag = 1;
                } else {
                    entry->tx_closing_flag = 1;
                }

                if (is_conn_alive(entry)) {
                    break;
                }
                bpf_map_delete_elem(&CT_MAP, tuple);
                if (entry_out != NULL) {
                    *entry_out = NULL; /* entry deleted: pointer would be stale */
                }
                break;
            default:
                break;
        }
        return CT_ESTABLISHED;
    }

    return CT_NEW;
}

/**
 * @brief swap tuple ports at egress direction
 *
 * @param tuple struct match_tuple
 */
static __always_inline void swap_ct_tuple_ports(struct ct_tuple *tuple)
{
    __be16 tmp = tuple->sport;
    tuple->sport = tuple->dport;
    tuple->dport = tmp;
}

/**
 * @brief swap tuple addrs at egress direction
 *
 * @param tuple struct match_tuple
 */
static __always_inline void swap_ct_tuple_addrs(struct ct_tuple *tuple)
{
    if (tuple->family == AF_INET) {
        __be32 tmp = tuple->ipv4.saddr;
        tuple->ipv4.saddr = tuple->ipv4.daddr;
        tuple->ipv4.daddr = tmp;
    } else {
        struct in6_addr tmp = tuple->ipv6.saddr;
        tuple->ipv6.saddr = tuple->ipv6.daddr;
        tuple->ipv6.daddr = tmp;
    }
}

/**
 * @brief lookup from ct map by ct_tuple if found then update lifetime of connection
 *
 * @param family socket family, read once at program entry (see skb_family())
 * @param skb struct __sk_buff
 * @param tuple struct ct_tuple (swapped in place for the loopback retry)
 * @param dir enum ct_dir
 * @param is_loopback whether to retry with a swapped tuple (loopback direction match)
 * @param entry_out optional out param passed to ct_lookup_entry: the hit entry on
 * return, NULL on CT_NEW or when the entry was deleted inside
 * @return CT_NEW if not found, otherwise CT_RELATED, CT_REOPENED or CT_ESTABLISHED
 */
static __always_inline enum ct_status ct_map_lookup_entry(__u32 family, struct __sk_buff *skb, struct ct_tuple *tuple,
    enum ct_dir dir, bool is_loopback, struct ct_entry **entry_out)
{
    enum ct_status status = ct_lookup_entry(family, skb, tuple, dir, entry_out);
    if (status == CT_NEW && is_loopback) {
        swap_ct_tuple_addrs(tuple);
        swap_ct_tuple_ports(tuple);
        status = ct_lookup_entry(family, skb, tuple, dir, entry_out);
    }
    return status;
}

#endif // NET_FIREWALL_CT_H
