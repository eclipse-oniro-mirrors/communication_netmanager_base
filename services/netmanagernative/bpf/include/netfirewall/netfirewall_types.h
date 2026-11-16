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
#ifndef NET_FIREWALL_TYPES_H
#define NET_FIREWALL_TYPES_H

#include <linux/bpf.h>
#include <linux/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
#include <netinet/in.h>
#else
#include <linux/in6.h>
#endif

#define USER_ID_DIVIDOR 200000
#define DEFAULT_USER_ID 100
#define BITMAP_LEN 63
#define IPV4_MAX_PREFIXLEN 32
#define IPV6_MAX_PREFIXLEN 128
#define PORT_MAX_MASK 16
#define DNS_PORT 53
#define DNS_OVER_TLS_PORT 853
#define DNS_QR_DEFALUT_MASK 15
#define DNS_QRS_IPV4_TYPE 1
#define DNS_QRS_IPV4_LEN 4
#define DNS_QRS_IPV6_TYPE 28
#define DNS_QRS_IPV6_LEN 16
#define BITS_PER_BYTE 8
#define DNS_DOMAIN_LEN 128
/* RFC 1035 label length field is 6 bits: the two top bits of a name byte
 * are the compression-pointer/extension markers, never label data. Used
 * by the label-boundary walk to reject garbage strides. */
#define DNS_LABEL_MAX_LEN 63
#define DNS_ANSWER_CNT 32
/* DNS query record table (txid + reversed 5-tuple credential check for
 * ingress learning). LRU: a flood of real queries evicts the oldest entry
 * instead of failing the write, so table stuffing cannot become a DoS. */
#define DNS_QUERY_RECORD_MAX_ENTRIES 1024
/* A record is valid only this long after the query passed (60s, aligned
 * with the non-TCP conntrack timeout). */
#define DNS_RECORD_EXPIRE_NS (60ULL * 1000ULL * 1000ULL * 1000ULL)
/* Learned-entry lifetime bounds, in seconds. The lower bound matches
 * the resolver cache floor (dnscache DEFAULT_DELAYED_COUNT) so a firewall
 * entry never dies before the resolver cache that still points at the IP. */
#define DNS_LEARN_TTL_MIN_SEC 600
#define DNS_LEARN_TTL_MAX_SEC (24ULL * 3600ULL)
#define DNS_NS_PER_SEC (1000000000ULL)
#define PROTOCOL_SAT_EXPAK 64
#define INTERFACE_NAME_MAX_LEN 16

struct bitmap {
    __u32 val[BITMAP_LEN];
};

typedef __u32 *bitmap_ptr;
typedef __u32 bitmap_t[BITMAP_LEN];
#define BITMAP_BITS (BITMAP_LEN * 32)

enum stream_dir {
    INVALID = -1,
    INGRESS = 1,
    EGRESS,
};

enum event_type {
    EVENT_INTERCEPT = 1,
    EVENT_DEBUG,
    EVENT_TUPLE_DEBUG,
};

enum debug_type {
    DBG_GENERIC, /* Generic, no message, useful to dump random integers */
    DBG_MATCH_SPORT,
    DBG_MATCH_DPORT,
    DBG_MATCH_PROTO,
    DBG_MATCH_APPUID,
    DBG_MATCH_UID,
    DBG_ACTION_KEY,
    DBG_MATCH_ACTION,
    DBG_CT_LOOKUP,
    DBG_MATCH_DOMAIN,
    DBG_MATCH_DOMAIN_ACTION,
    DBG_MATCH_INTERFACE,
    /* Record-table write flushed at the egress hook exit / learning
     * rejected (record mismatch / expired / non-allow). */
    DBG_DNS_RECORD_WRITE,
    DBG_DNS_LEARN_REJECT,
};

struct domain_hash_key {
    __u32 prefixlen;
    __u32 uid;
    __u32 appuid;
    __u8 data[DNS_DOMAIN_LEN];
};

/* ===== A' stub experiment: label-anchored trie (two anchored scans) =====
 * Prefix rules walk FORWARD from name position 0 (single descent, anchored
 * at the name start); suffix rules walk BACKWARD from the terminator
 * (single descent, anchored at the name end — every suffix pattern shares
 * that anchor, so one trie path covers all overlapping rules, the map
 * counterpart of today's reverse-string LPM semantics).
 * Class encoding keeps label length bytes and content bytes disjoint:
 * a content digit ('0'-'9' == 0x30-0x39) can never masquerade as a length
 * class (1-63), which is what makes label alignment structural instead
 * of rescued by the terminator.
 * Capacity is a standing kernel-memory budget (prealloc pool costs
 * ~106B/entry eagerly at map create, measured on HM 1.15 — see
 * netfirewall_map.h AC table comment): 32768 entries ≈ 3.4MB per table,
 * ≈ 6.9MB for both tables, covering ~100 wildcard patterns or ~2000
 * bare domains (pool shared across all (uid, appuid) namespaces).
 * Overflow fails fast before the generation flip: the previous table
 * stays online (graceful rejection). */
#define AC_TRANS_MAX_ENTRIES 32768
/* Scan bound == DNS_DOMAIN_LEN: the wire name (127 bytes + terminator)
 * fits in 128 positions, exactly the legacy parse capacity. */
#define AC_NAME_SCAN_MAX 128
#define AC_FLAG_PREFIX_ALLOW 0x1U
#define AC_FLAG_PREFIX_DENY 0x2U
#define AC_FLAG_SUFFIX_ALLOW 0x4U
#define AC_FLAG_SUFFIX_DENY 0x8U
#define AC_PREFIX_MASK (AC_FLAG_PREFIX_ALLOW | AC_FLAG_PREFIX_DENY)
#define AC_SUFFIX_MASK (AC_FLAG_SUFFIX_ALLOW | AC_FLAG_SUFFIX_DENY)
/* Label-confined allow completion bit (trailing-star prefix rules,
 * "www.example.c*" as an ALLOW rule): the '*' completes the LAST label —
 * the verdict is provisional, ac_prefix_scan drops it at any later label
 * boundary ("www.example.c*" must not allow www.example.c.evil.com) and
 * promotes the survivor to AC_FLAG_PREFIX_ALLOW at the terminator, so
 * every downstream consumer keeps reading AC_PREFIX_MASK only. DENY
 * trailing-star rules use the plain AC_FLAG_PREFIX_DENY: a deny is
 * right-unconstrained on purpose (fail-closed both ways — an allow must
 * not leak past the last label, a deny may block everything to the
 * right). Builder/table-side only; deliberately NOT part of
 * AC_PREFIX_MASK. */
#define AC_FLAG_PREFIX_ALLOW_LC 0x10U
#define AC_PARSE_FAIL 0xFFFFFFFFU
#define AC_CLS_LEN(c) ((__u32)(c))
#define AC_CLS_CHAR(c) (0x100U | (__u32)(c))
#define AC_CLS_TERM 0x200U

struct ac_trans_key {
    __u32 uid;
    __u32 appuid;
    __u32 state; /* trie node, root = 0; an opaque map scalar, never an index */
    __u32 cls;   /* AC_CLS_LEN / AC_CLS_CHAR / AC_CLS_TERM */
    __u32 gen;   /* table generation: one read, whole scan uses it */
};

struct ac_trans_val {
    __u32 next;
    __u32 flags; /* AC_FLAG_* completion bits */
    __u32 rem;   /* reserved for the builder (unused by the scans: the
                  * backward scan takes label boundaries from the forward
                  * scan's bitmap — a per-transition rem cannot
                  * disambiguate patterns that diverge mid-label) */
};

struct defalut_action_value {
    enum sk_action inaction;
    enum sk_action outaction;
};

/* ===== evaluation three-state: sk_action is kernel UAPI (SK_DROP/SK_PASS) and must
 * never be extended to carry an error state, hence a dedicated enum. ===== */
enum fw_eval_result {
    FW_PASS = 1,   /* evaluation succeeded and verdict is pass */
    FW_DROP,       /* evaluation succeeded and verdict is drop */
    FW_EVAL_ERROR, /* evaluation failed: hand over to eval_error fallback */
};

/* Evaluation fail points (diagnostic dimension of eval fault events; append-only, never
 * renumber existing values). */
enum eval_fail_point {
    FAIL_NONE = 0,
    FAIL_TUPLE = 1,          /* get_match_tuple failed (incl. unparseable-L4 exit) */
    FAIL_CT_TUPLE = 2,       /* get_ct_tuple failed (defensive branch) */
    FAIL_SCRATCH = 3,        /* NF_BITMAP_SCRATCH lookup failed */
    FAIL_ACTION_KEY = 4,     /* match_action_key false (realistic source: dev_name failure) */
    FAIL_ACTION_ARGS = 5,    /* match_action null args (defensive branch) */
    FAIL_UID_ATTR = 6,       /* uid attribution failed (reserved; carried by UNATTRIBUTED semantics) */
    FAIL_L4_NO_L4 = 7,       /* non-first fragment, no L4 header to read (structural, not a fault) */
    FAIL_L4_UNPARSEABLE = 8, /* ESP/malformed/overlong ext chain, L4 not safely locatable */
};

/* Unified tri-state L4 parse result: the firewall's L4 view must match the stack's
 * (IHL / IPv6 extension header chain), otherwise attacker-controlled option bytes are
 * read as ports/RST. A packet that is NO_L4/UNPARSEABLE never enters the 7-dimension
 * rule AND and never skips dimensions — it is decided by the chain
 * (ct -> default action -> MatchDomain) in the policy hooks. */
enum l4_parse_state {
    L4_PARSE_OK = 0,          /* real L4 located: l4_info.l4_nhoff/protocol valid */
    L4_PARSE_NO_L4 = 1,       /* structurally no L4 header: non-first fragment (frag_off != 0) */
    L4_PARSE_UNPARSEABLE = 2, /* cannot safely locate L4: ESP, IHL < 5, bad version, overlong chain */
};

/* uid attribution state: 0xFFFFFFFF can never collide with a real userId, which would
 * require sock_uid >= 0xFFFFFFFF * 200000 (u32 overflow). The former behaviour folded
 * 0xFFFFFFFF to 21473, a theoretically valid userId. */
#define UID_UNATTRIBUTED 0xFFFFFFFFu

/* Reserved system key (user-space twin: FIREWALL_SYSTEM_USER_ID in
 * netfirewall_parcel.h, constexpr — the single source of the value now).
 * After F6 nothing folds into it: real system-identity sockets
 * (sock_uid < USER_ID_DIVIDOR) go through CURRENT_UID_MAP foreground
 * folding, and socketless kernel packets land on UID_UNATTRIBUTED. The
 * DEFAULT_ACTION_MAP entry written at startup (SetBuiltInFirewallDefaultActions,
 * keyed by FIREWALL_SYSTEM_USER_ID) is kept as a writable reserved/governance
 * key — no live traffic hits it. Value 1 is outside the real multi-user
 * range (real users start at 100). */

/* Runtime fallback mode (EVAL_FALLBACK_MODE_MAP value). No entry = FAIL_CLOSED (secure
 * default); OBSERVE is the explicit escape hatch restoring the legacy pass-through. */
enum eval_fallback_mode {
    EVAL_FALLBACK_OBSERVE = 0,
    EVAL_FALLBACK_FAIL_CLOSED = 1,
};

/* Same-batch invariant: the learned-entry value carries the
 * write timestamp and the answer TTL consumed by the two-layer aging — the
 * BPF read-time gate (MatchDomain) and the user-space reclaim task. value_size
 * change (8 -> 24) rebuilds the pinned map on image upgrade; the learned cache
 * starts empty, which is a one-time re-learn cost, not a compatibility break. */
struct domain_value {
    __u32 appuid;
    __u32 uid;
    __u64 timestamp; /* write time, bpf_ktime_get_ns (CLOCK_MONOTONIC) domain */
    __u32 ttl;       /* clamped answer TTL, seconds; 0 entries never pass the gate */
};

/* ===== DNS query record structures ===== */

/* The DNS flow identity as the packet itself describes it (no egress swap
 * semantics): assembled once in match_dns_query and carried into the parse
 * functions. sport/dport/saddr/daddr stay in network order. */
struct dns_flow_info {
    __u8 protocol; /* IPPROTO_UDP / IPPROTO_TCP — TCP sniffing is live */
    __u8 family;   /* AF_INET / AF_INET6 */
    __u16 txid;    /* dnshdr.id, host order */
    __be16 sport;  /* packet source port, network order */
    __be16 dport;  /* packet destination port, network order */
    union {
        struct {
            __be32 saddr;
            __be32 daddr;
        } ipv4;
        struct {
            struct in6_addr saddr;
            struct in6_addr daddr;
        } ipv6;
    };
};

/* Record-table key — always in the QUERY direction view (client ip:port ->
 * server ip:53). sizeof = 40, no padding (byte-exact hash key on both sides).
 * Key design: uid is deliberately NOT part of the key. A
 * cross-uid collision needs two apps sharing (txid, client port, server, local
 * address) inside the same 60s window — UDP ephemeral-port uniqueness makes
 * that unreachable in practice, and the failure mode is attribution confusion
 * (the response side still re-checks allowrst per socket), never an
 * unauthorized pass. Adding uid would double the miss surface for nothing. */
struct dns_query_record_key {
    __u16 txid;    /* host order (same conversion rule on write and lookup) */
    __u8 protocol; /* IPPROTO_UDP */
    __u8 family;   /* AF_INET / AF_INET6 */
    __be16 sport;  /* query source port (client ephemeral port) */
    __be16 dport;  /* query destination port (53) */
    union {
        struct {
            __be32 saddr; /* client ip */
            __be32 daddr; /* dns server ip */
        } ipv4;
        struct {
            struct in6_addr saddr;
            struct in6_addr daddr;
        } ipv6;
    };
};

/* Record-table value — the authoritative uid/appuid source for learning:
 * captured at the egress query, never taken from the response packet. */
struct dns_query_record_value {
    __u32 uid;       /* query originator userId (get_current_uid) */
    __u32 appuid;    /* query originator app uid (bpf_get_socket_uid) */
    __u64 timestamp; /* record write time, bpf_ktime_get_ns */
};

/* Egress query staging slot (percpu scratch): written by the sniffing step,
 * flushed (or dropped) at the hook exit once the final verdict is known. The
 * skb pointer address tags the staging so a later packet can never flush a
 * dropped packet's leftovers. */
struct dns_query_pending {
    __u64 skb_addr; /* this packet's tag */
    __u16 valid;    /* 1 = a record is staged */
    struct dns_query_record_key key;
    __u32 uid;
    __u32 appuid;
};

/* ===== Pure helpers: shared by the BPF side and user-space tests.
 * NO BPF helper calls here — this header is also included from plain C++
 * translation units. int is used instead of bool to stay valid in both the
 * C (BPF) and C++ (user) compilations of this header. ===== */

/* Reverse the response (server -> client) 5-tuple into the query
 * (client -> server) view used by the record key. 0 on success. */
static inline int build_dns_query_record_key(const struct dns_flow_info *flow, struct dns_query_record_key *key)
{
    if (flow == 0 || key == 0) {
        return -1;
    }
    key->txid = flow->txid;
    key->protocol = flow->protocol;
    key->family = flow->family;
    /* response dst port (client ephemeral) = query src port; response src
     * port (53) = query dst port; same swap for the addresses. */
    key->sport = flow->dport;
    key->dport = flow->sport;
    if (flow->family == AF_INET) {
        key->ipv4.saddr = flow->ipv4.daddr;
        key->ipv4.daddr = flow->ipv4.saddr;
    } else if (flow->family == AF_INET6) {
        key->ipv6.saddr = flow->ipv6.daddr;
        key->ipv6.daddr = flow->ipv6.saddr;
    } else {
        return -1;
    }
    return 0;
}

/* Records older than DNS_RECORD_EXPIRE_NS are treated as absent.
 * A record apparently written in the future (clock moved backwards) is also
 * treated as absent — fail-safe closes the acceptance window. Deliberately
 * the OPPOSITE rollback policy of domain_entry_expired below:
 * a record is a seconds-scale credential, closing it is cheap fail-safe;
 * a learned allow entry gates live traffic, so that one keeps serving. */
static inline int dns_record_expired(__u64 now, __u64 record_timestamp)
{
    if (now < record_timestamp) {
        return 1;
    }
    return (now - record_timestamp) > DNS_RECORD_EXPIRE_NS;
}

/* Clamp the answer TTL into [DNS_LEARN_TTL_MIN_SEC, DNS_LEARN_TTL_MAX_SEC].
 * TTL=0 returns 0 — such answers are not learned (the resolver does
 * not cache TTL=0 answers either, data plane and resolver stay aligned). */
static inline __u32 dns_learn_ttl_clamp(__u32 ttl)
{
    if (ttl == 0) {
        return 0;
    }
    if (ttl < DNS_LEARN_TTL_MIN_SEC) {
        return DNS_LEARN_TTL_MIN_SEC;
    }
    if (ttl > (__u32)DNS_LEARN_TTL_MAX_SEC) {
        return (__u32)DNS_LEARN_TTL_MAX_SEC;
    }
    return ttl;
}

/* First-layer time gate: an expired learned entry is a miss. It is
 * never deleted here (reclaim belongs to the user-space aging task); when
 * that task is stuck, expired entries only occupy capacity and never
 * authorize traffic. timestamp=0 cannot occur in this revision (every writer
 * fills it) and is treated as expired so no stale writer can pin an entry. */
static inline int domain_entry_expired(__u64 now, __u64 timestamp, __u32 ttl)
{
    if (timestamp == 0) {
        return 1;
    }
    if (now < timestamp) {
        /* monotonic clock cannot move backwards within a boot; if it ever
         * does, keep serving and let the aging task decide on reclaim.
         * Opposite of dns_record_expired (see its header for the rationale):
         * closing a live allow entry would cut traffic. */
        return 0;
    }
    return (now - timestamp) > ((__u64)ttl * DNS_NS_PER_SEC);
}

struct debug_event {
    enum debug_type type;
    enum stream_dir dir;
    __u32 arg1;
    __u32 arg2;
    __u32 arg3;
    __u32 arg4;
    __u32 arg5;
};

struct intercept_event {
    enum stream_dir dir;
    __u32 family;
    __u8 protocol;
    union {
        struct {
            __be32 saddr;
            __be32 daddr;
        } ipv4;
        struct {
            struct in6_addr saddr;
            struct in6_addr daddr;
        } ipv6;
    };
    __be16 sport;
    __be16 dport;
    __u32 appuid;
};

struct match_tuple {
    enum stream_dir dir;
    __u32 family;
    __u8 protocol;
    union {
        struct {
            __be32 saddr;
            __be32 daddr;
        } ipv4;
        struct {
            struct in6_addr saddr;
            struct in6_addr daddr;
        } ipv6;
    };
    __be16 sport;
    __be16 dport;
    __u32 appuid;
    __u32 uid;
    __u16 rst;
    __u32 ifindex;
    /* L4 parse state of this packet. Ports/rst are loaded only on L4_PARSE_OK
     * (from the parsed l4_nhoff); NO_L4/UNPARSEABLE tuples carry zero ports and are
     * handed to the chain. */
    enum l4_parse_state l4_state;
};

/* ===== Query-layer server-authorization gate ===== */

/* Gate context: filled by the sniffing step for an egress DNS query and
 * consumed by match_action's no-rule branch. is_query=1 when the packet is
 * DNS-query-shaped: a parseable single-question UDP/TCP 53 query (parsed by
 * the sniffer), or a UDP/TCP packet addressed to port 853 (DoT — port-shape
 * only: the payload is encrypted, nothing to sniff or learn). A malformed
 * or non-DNS packet on port 53 keeps is_query=0 so the gate stays closed
 * (fail-closed). allowrst carries the 53-side qname verdict (1 = the qname
 * hit this uid's domain allow rule); it is never set for 853 (encrypted
 * payload — the gate opens on server authorization alone). */
struct dns_gate_ctx {
    __u8 is_query; /* 1 = egress DNS query shape (53 parsed / 853 by port) */
    __u8 allowrst; /* 1 = the qname hit this uid's domain allow rule (53 only) */
};

struct event {
    enum event_type type;
    union {
        struct debug_event debug;
        struct intercept_event intercept;
        struct match_tuple tuple;
    };
    __u32 len;
};

typedef __u8 loop_back_val;
typedef __be32 ip4_key;
typedef struct in6_addr ip6_key;
typedef __u8 action_key;
typedef struct bitmap action_val;
typedef __be16 port_key_val;
typedef __u8 proto_key;
typedef __u32 appuid_key;
typedef __u32 uid_key;
typedef struct {
    char name[INTERFACE_NAME_MAX_LEN];
} interface_key;

/* Wildcard entry key: the dimension role the entry falls back for. Role values 0-7
 * are structurally disjoint from every dimension rule key space (ip/port/proto/uid),
 * so a rule value can never merge into a wildcard entry. */
typedef __u32 wildcard_key;

typedef enum {
    WILDCARD_SRC_ADDR = 0, /* covers both saddr(v4) and saddr6(v6) fallback */
    WILDCARD_DST_ADDR,
    WILDCARD_SPORT,
    WILDCARD_DPORT,
    WILDCARD_PROTO,
    WILDCARD_APPUID,
    WILDCARD_UID,
    WILDCARD_IFACE,
    WILDCARD_ROLE_MAX,
} wildcard_role;

typedef enum {
    CURRENT_USER_ID_KEY = 1,
} current_user_id_key;

typedef enum {
    DEFAULT_ACT_IN_KEY = 1,
    DEFAULT_ACT_OUT_KEY = 2,
} default_action_key;

/* ===== DNS server authorization maps =====
 *
 * Key design: {uid, ip} — the per-domain gate authorizes a
 * DNS query only when "destination IP ∈ this user's system resolver set ×
 * qname ∈ allow-domain rules". The map is NOT a pass rule (it never joins the
 * rule bitmaps) and carries no wildcard merging; the control plane is the sole
 * writer, the BPF side is read-only.
 * Capacity: every user is capped at DNS_SERVER_MAX_PER_USER entries
 * (primary/standby + redundancy, enforced by the control-plane writer), so
 * DNS_SERVER_MAP_MAX_ENTRIES = 64 users × 8. */
#define DNS_SERVER_MAX_PER_USER 8
#define DNS_SERVER_MAP_MAX_ENTRIES 512
/* v4 key: 8 bytes, no padding — ip stays in network order exactly as the
 * data plane will see it in the packet header. */
struct dns_server_v4_key {
    __u32 uid;
    __u32 ip;
};
/* v6 key: 4 + 16 = 20 bytes, in6_addr is 4-byte aligned so there is no
 * implicit padding — the user-space writer must zero-init then fill (
 * lesson: no garbage bytes in hash keys). */
struct dns_server_v6_key {
    __u32 uid;
    struct in6_addr ip;
};
/* value = flag word: presence in the map means "authorized resolver target
 * for this user"; the byte is kept for future flags (e.g. primary/standby). */
struct dns_server_val {
    __u8 managed;
};

struct ipv4_lpm_key {
    __u32 prefixlen;
    ip4_key data;
};

struct ipv6_lpm_key {
    __u32 prefixlen;
    ip6_key data;
};

struct port_key {
    __u32 prefixlen;
    __be16 data;
};

struct dnshdr {
    __be16 id;
    __be16 flag;
    __be16 qdcount;
    __be16 ancount;
    __be16 nscount;
    __be16 arcount;
};

#endif // NET_FIREWALL_TYPES_H
