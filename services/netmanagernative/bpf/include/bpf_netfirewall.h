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

#ifndef NETMANAGER_EXT_BPF_NET_FIREWALL_H
#define NETMANAGER_EXT_BPF_NET_FIREWALL_H

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <netdb.h>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "bitmap_manager.h"
#include "bpf_mapper.h"
#include "i_netfirewall_callback.h"
#include "netfirewall/netfirewall_def.h"
#include "netfirewall_parcel.h"
#include "system_ability.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS::NetManagerStandard {
// Only used for unittest code currently
static constexpr const char *FIREWALL_BPF_PATH = "/system/etc/bpf/netsys.o";

static constexpr const int CONNTRACK_GC_INTTERVAL_MS = 60000;
static constexpr const int RING_BUFFER_POLL_TIME_OUT_MS = -1;
/* Learned-entry aging sweep period. */
static constexpr const int DOMAIN_CACHE_AGING_INTERVAL_MS = 60000;

static constexpr const char *LOOP_BACK_IPV4 = "127.0.0.0";
static constexpr const int LOOP_BACK_IPV4_PREFIXLEN = 8;
static constexpr const char *LOOP_BACK_IPV6 = "::1";
static constexpr const int LOOP_BACK_IPV6_PREFIXLEN = 128;
// convert ebpf types from unix style style to CPP's
using Ip4Key = ip4_key;
using Ip6Key = ip6_key;
using Ipv4LpmKey = struct ipv4_lpm_key;
using Ipv6LpmKey = struct ipv6_lpm_key;
using PortKey = port_key;
using ProtoKey = proto_key;
using AppUidKey = appuid_key;
using DefaultActionKey = default_action_key;
using CurrentUserIdKey = current_user_id_key;
using ActionKey = action_key;
using ActionValue = action_val;
using WildcardKey = wildcard_key;
using RuleCode = struct bitmap;
using StreamDir = enum stream_dir;
using EventType = enum event_type;
using Event = struct event;
using InterceptEvent = struct intercept_event;
using DebugEvent = struct debug_event;
using TupleEvent = struct match_tuple;
using DebugType = enum debug_type;
using EvalFailPoint = enum eval_fail_point;

using CtKey = struct ct_tuple;
using CtVaule = struct ct_entry;

using DomainHashKey = struct domain_hash_key;
using DomainValue = domain_value;
using LoopbackValue = loop_back_val;
/* User space only ever READS/clears the record table (written by the
 * BPF flush point), these aliases exist for the BpfMapper instantiations. */
using DnsQueryRecordKey = struct dns_query_record_key;
using DnsQueryRecordValue = struct dns_query_record_value;
/* DNS server authorization map key/value aliases (control-plane
 * writer side). */
using DnsServerV4Key = struct dns_server_v4_key;
using DnsServerV6Key = struct dns_server_v6_key;
using DnsServerVal = struct dns_server_val;

/* ===== A' label-anchored AC trie builder (control plane) =====
 *
 * Compiles the domain firewall rules into the sparse transition tables the
 * BPF side walks (ac_prefix_scan / ac_suffix_scan in netfirewall_domain.h —
 * the consumers are frozen and device-validated; this builder is their sole
 * writer). Two tables: the prefix table (rules with a trailing '*', anchored
 * at the name start, forward encoding) and the suffix table (exact domains,
 * anchored at the name terminator, backward encoding). Wire patterns:
 *
 *   suffix  "example.com"   TERM, CHAR(m) CHAR(o) CHAR(c), LEN(3),
 *                           CHAR(e) CHAR(l) CHAR(p) CHAR(m) CHAR(a) CHAR(x) CHAR(e), LEN(7)
 *   prefix  "www.example*"  LEN(3), CHAR(w) CHAR(w) CHAR(w),
 *                           LEN(7), CHAR(e) ... CHAR(e)
 *
 * TERM=0x200, LEN(c)=c (label length byte), CHAR(c)=0x100|fold(c) (content
 * byte) — the class space is disjoint, see netfirewall_types.h. Every
 * (uid, appuid) namespace gets its own trie with its own BFS state
 * numbering from root=0 (the state travels inside the hash key, namespaces
 * never share rows). Completion flags (AC_FLAG_*_ALLOW/DENY) latch onto the
 * LAST transition of each pattern; overlapping rules sharing a trie path
 * ("com" and "example.com") both latch through the shared rows — the map
 * counterpart of the legacy reverse-string LPM overlap semantics.
 * ac_trans_val.rem is written 0: the scans never consume it (the backward
 * scan takes label boundaries from the forward scan's boundary bitmap).
 *
 * Pure, in-memory, unit-testable; the map I/O and the generation switch
 * live in NetsysBpfNetFirewall::BuildAcDomainTables. */

using AcTransKey = struct ac_trans_key;
using AcTransVal = struct ac_trans_val;

/* Control-plane mirror of the BPF-side ac_fold (netfirewall_domain.h): the
 * trie is built folded and the scan folds the wire bytes — both sides must
 * fold identically or every mixed-case match silently breaks. The
 * differential test asserts byte-equality against the real ac_fold. */
inline uint8_t AcFoldByte(uint8_t c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<uint8_t>(c + ('a' - 'A')) : c;
}

struct AcTransEntry {
    AcTransKey key = {};
    AcTransVal val = {};
};

/* Builder-private pseudo-class for the legacy subdomain anchor ("*.x.com"
 * matches subdomains only, never the bare domain — legacy reversed-LPM
 * keyed the leading dot, see the spec-widening commit). Appended as the
 * LAST class of a suffix pattern started by "*." it encodes "at least one
 * more class follows the pattern" — in the backward class stream every
 * label's LEN is followed by >=1 content CHAR (labels are >=1 byte), so
 * >=1 class after the first label's LEN <=> >=1 preceding label. Emit
 * expands one MORE edge into a full row of real-class transitions, so the
 * value never reaches the BPF-side class space (LEN 1..63 / CHAR
 * 0x100..0x1FF / TERM 0x200) and netsys.o is untouched by this constant. */
static constexpr uint32_t AC_BUILDER_CLS_MORE = 0x300U;

/* Builder-private pseudo-class for the trailing-star (label-anchored)
 * prefix rules: "www.example.c*" / "www.example.*" / "ads*". Inserted
 * where the pattern must consume ONE label-LEN class of any value — the
 * length of the name's next label, which the builder cannot know. Emit
 * expands one SKIP edge into the 63 real AC_CLS_LEN rows (the whole
 * legal length range, the MORE expansion pattern LEN-only), so the value
 * never reaches the BPF-side class space and the match path stays plain
 * map lookups. Label semantics: the fragment after the SKIP (nothing,
 * for the "x.*" whole-label form) completes the LAST label — an ALLOW
 * verdict is provisional (AC_FLAG_PREFIX_ALLOW_LC, cleared by
 * ac_prefix_scan at any later label boundary), a DENY verdict is
 * right-unconstrained on purpose (fail-closed both ways). */
static constexpr uint32_t AC_BUILDER_CLS_SKIP = 0x301U;

class AcDomainTableBuilder {
public:
    AcDomainTableBuilder() = default;

    /* Add one domain rule into the (uid, appuid) namespace trie (appuid 0 =
     * the wild namespace; NetFirewallDomainParam.isWildcard is ignored —
     * the wild-namespace row covers it). Returns false on an
     * unrepresentable domain
     * (skip + log at the call site): empty, '*' only, empty label ("a..b"),
     * label longer than DNS_LABEL_MAX_LEN (the BPF scan aborts on c > 63),
     * a pattern that cannot fit the scan window, or an unsupported '*'
     * form (see CheckStarForm). Supported forms —
     *   "x.com"              bare domain + every subdomain;
     *   "*.x.com"            subdomains only (legacy spec-widening anchor);
     *   "*f.x.com"           the label before "x.com" ends with f,
     *                        everything further left is the '*' (glob
     *                        left; the label-aligned body keeps every
     *                        match inside the written suffix);
     *   "x.f*" / "x.*" / "f*" the '*' completes the LAST label: an allow
     *                        verdict is confined to it (no new labels to
     *                        the right), a deny verdict is
     *                        right-unconstrained (fail-closed both ways). */
    bool AddRule(uint32_t uid, uint32_t appuid, bool isAllow, const std::string &domain);

    /* Star-form gate (fail-visible validation, the single source of truth
     * for BuildPattern and the caller's skip log): true when the domain's
     * '*' placement is representable — at most one star, at the very
     * front or the very back, never mid-domain, never duplicated. On
     * rejection *reason (optional) names the violated form. The star
     * count / last position are exported for BuildPattern's form
     * routing (meaningful only when true is returned). */
    static bool CheckStarForm(const std::string &domain, const char **reason = nullptr, size_t *starCount = nullptr,
                              size_t *lastStar = nullptr);

    bool Empty() const
    {
        return namespaces_.empty();
    }

    /* Emit the transition entries of generation `gen`, BFS-numbered per
     * namespace (prefix rules -> prefixOut, suffix rules -> suffixOut).
     * Deterministic (ordered maps). Returns false when either table would
     * exceed AC_TRANS_MAX_ENTRIES — the caller must fail fast and must NOT
     * flip the generation. */
    bool Emit(uint32_t gen, std::vector<AcTransEntry> &prefixOut, std::vector<AcTransEntry> &suffixOut) const;

    /* Wire pattern generation (public for the differential tests).
     * isPrefixRule: the domain ends with '*' — prefix rules anchor at the
     * name start, suffix rules at the terminator. Star forms: see
     * CheckStarForm; the new label-anchored forms are "*f.x.com"
     * (truncated suffix pattern) and the trailing-star family
     * ("x.f*" / "x.*" / "f*", SKIP class + fragment — see
     * AC_BUILDER_CLS_SKIP). */
    static bool BuildPattern(const std::string &domain, bool isPrefixRule, std::vector<uint32_t> &cls);

private:
    /* One trie per (table, namespace). Node 0 is the root; edges[node] maps
     * the class of the next consumed byte to the child node. Every non-root
     * node has exactly one entering edge, so the completion flags of the
     * edge entering node n are stored per node (enterFlags[n]). */
    struct AcTrie {
        std::vector<std::map<uint32_t, uint32_t>> edges = {std::map<uint32_t, uint32_t>()};
        std::vector<uint32_t> enterFlags = {0};

        void Insert(const std::vector<uint32_t> &pattern, uint32_t doneFlags);
        void Emit(uint32_t uid, uint32_t appuid, uint32_t gen, std::vector<AcTransEntry> &out) const;

    private:
        void InsertAt(const std::vector<uint32_t> &pattern, size_t pos, uint32_t cur, uint32_t doneFlags);
        void MergeInto(uint32_t src, uint32_t dst);
        void CopySubtree(uint32_t src, uint32_t dstParent, uint32_t cls);
    };
    struct AcNamespace {
        AcTrie prefix;
        AcTrie suffix;
    };

    std::map<std::pair<uint32_t, uint32_t>, AcNamespace> namespaces_;
};

inline void AcDomainTableBuilder::AcTrie::Insert(const std::vector<uint32_t> &pattern, uint32_t doneFlags)
{
    InsertAt(pattern, 0, 0, doneFlags);
}

/* A SKIP edge intercepts EVERY label length at its node: its 63-row
 * expansion shares the map key (state, LEN(c)) with any concrete
 * label-length edge another rule placed at the same node, and the
 * single-writer hash map keeps only the last row — one of the two
 * rules silently vanishes (measured: allow "www.example*" + deny
 * "ads*" in one namespace clobbered the allow at (root, LEN(3)),
 * AcScanPrefixWild001 m2/m2b). The invariant that keeps both alive:
 * at a node with a SKIP edge, the SKIP target holds the UNION of
 * every label-length continuation — the concrete ones are deep-merged
 * into it when the SKIP edge is created, later concrete classes route
 * through it, and the absorbed concrete edges are erased so Emit
 * only ever emits the SKIP expansion for label lengths. Sound
 * because a concrete continuation demands its label's exact content
 * before the next boundary: reached via any SKIP row it self-filters
 * to the very same labels. */
inline void AcDomainTableBuilder::AcTrie::InsertAt(const std::vector<uint32_t> &pattern, size_t pos, uint32_t cur,
                                                   uint32_t doneFlags)
{
    while (pos < pattern.size()) {
        const uint32_t cls = pattern[pos];
        if (cls == AC_BUILDER_CLS_SKIP) {
            auto it = edges[cur].find(AC_BUILDER_CLS_SKIP);
            uint32_t target;
            if (it == edges[cur].end()) {
                target = static_cast<uint32_t>(edges.size());
                edges.emplace_back();
                enterFlags.push_back(0);
                edges[cur].emplace(AC_BUILDER_CLS_SKIP, target);
                /* Absorb the concrete label-length continuations already
                 * at this node into the fresh SKIP target, then drop the
                 * concrete edges (indices only — never hold a reference
                 * into edges across the subtree merges below, the vector
                 * reallocates). */
                std::vector<std::pair<uint32_t, uint32_t>> absorbed;
                for (const auto &edge : edges[cur]) {
                    if (edge.first >= AC_CLS_LEN(1) && edge.first <= AC_CLS_LEN(DNS_LABEL_MAX_LEN)) {
                        absorbed.emplace_back(edge.first, edge.second);
                    }
                }
                for (const auto &edge : absorbed) {
                    MergeInto(edge.second, target);
                    edges[cur].erase(edge.first);
                }
            } else {
                target = it->second;
            }
            InsertAt(pattern, pos + 1, target, doneFlags);
            return;
        }
        if (cls >= AC_CLS_LEN(1) && cls <= AC_CLS_LEN(DNS_LABEL_MAX_LEN)) {
            /* A concrete label length at a SKIP node: the SKIP expansion
             * is the only row set emitted for label lengths here, so the
             * continuation must live under the SKIP target (the length
             * class itself is consumed by the expansion row). */
            auto it = edges[cur].find(AC_BUILDER_CLS_SKIP);
            if (it != edges[cur].end()) {
                InsertAt(pattern, pos + 1, it->second, doneFlags);
                return;
            }
        }
        auto it = edges[cur].find(cls);
        if (it == edges[cur].end()) {
            uint32_t child = static_cast<uint32_t>(edges.size());
            edges.emplace_back();
            enterFlags.push_back(0);
            cur = edges[cur].emplace(cls, child).first->second;
        } else {
            cur = it->second;
        }
        ++pos;
    }
    /* Completion flags latch onto the edge that consumed the pattern's last
     * class — OR-ed, so one edge can complete several rules, and an allow
     * and a deny rule on the same domain keep both bits (the legacy two-map
     * lookup pair produced exactly the same combined verdict). */
    enterFlags[cur] |= doneFlags;
}

/* Union-merge the subtrie rooted at src into the node dst: shared edge
 * classes merge recursively (completion flags OR at every level), new
 * classes deep-copy. The trie stays a tree — every node keeps exactly
 * one entering edge, so per-node enterFlags remains sound. */
inline void AcDomainTableBuilder::AcTrie::MergeInto(uint32_t src, uint32_t dst)
{
    enterFlags[dst] |= enterFlags[src];
    std::vector<std::pair<uint32_t, uint32_t>> list(edges[src].begin(), edges[src].end());
    for (const auto &edge : list) {
        auto it = edges[dst].find(edge.first);
        if (it == edges[dst].end()) {
            CopySubtree(edge.second, dst, edge.first);
        } else {
            MergeInto(edge.second, it->second);
        }
    }
}

inline void AcDomainTableBuilder::AcTrie::CopySubtree(uint32_t src, uint32_t dstParent, uint32_t cls)
{
    uint32_t fresh = static_cast<uint32_t>(edges.size());
    edges.emplace_back();
    enterFlags.push_back(enterFlags[src]);
    edges[dstParent].emplace(cls, fresh);
    std::vector<std::pair<uint32_t, uint32_t>> list(edges[src].begin(), edges[src].end());
    for (const auto &edge : list) {
        CopySubtree(edge.second, fresh, edge.first);
    }
}

inline void AcDomainTableBuilder::AcTrie::Emit(uint32_t uid, uint32_t appuid, uint32_t gen,
                                               std::vector<AcTransEntry> &out) const
{
    if (edges.size() <= 1) {
        return; /* empty trie: no rows, the scan misses at the root */
    }
    /* BFS numbering from root=0: children are numbered in class order as
     * the frontier advances. The relabeling is cosmetic — the state is an
     * opaque hash-key scalar on the BPF side, never an index; no emitted id
     * can ever collide with the AC_PARSE_FAIL dead marker (ids < entry
     * count <= AC_TRANS_MAX_ENTRIES). */
    std::vector<uint32_t> newId(edges.size(), 0);
    std::vector<uint32_t> queue;
    queue.reserve(edges.size());
    queue.push_back(0);
    uint32_t next = 1;
    for (size_t head = 0; head < queue.size(); ++head) {
        for (const auto &edge : edges[queue[head]]) {
            if (next >= edges.size()) {
                break; /* unreachable: every child is discovered exactly once */
            }
            newId[edge.second] = next++;
            queue.push_back(edge.second);
        }
    }
    /* Orphan pruning: absorbed subtrees (see InsertAt) keep their nodes in
     * the vector but lost their entering edge — the BFS above never numbers
     * them (newId stays 0) and they must not be emitted: their rows would
     * key at state 0 and clobber the root's real rows. */
    std::vector<bool> visited(edges.size(), false);
    for (uint32_t n : queue) {
        visited[n] = true;
    }
    for (size_t node = 0; node < edges.size(); ++node) {
        if (!visited[node]) {
            continue;
        }
        const auto &nodeEdges = edges[node];
        const bool hasSkip = nodeEdges.find(AC_BUILDER_CLS_SKIP) != nodeEdges.end();
        const auto moreIt = nodeEdges.find(AC_BUILDER_CLS_MORE);
        for (const auto &edge : nodeEdges) {
            if (hasSkip && edge.first >= AC_CLS_LEN(1) && edge.first <= AC_CLS_LEN(DNS_LABEL_MAX_LEN)) {
                /* Defense-in-depth: a concrete label-length edge at a SKIP
                 * node was absorbed into the SKIP target at Insert time
                 * (see InsertAt) — emitting it would collide with the SKIP
                 * expansion below and silently clobber one of the two
                 * rules. */
                continue;
            }
            if (edge.first == AC_BUILDER_CLS_SKIP) {
                /* Wildcard-LEN expansion (the MORE pattern, LEN-only): the
                 * trailing-star pattern's SKIP consumes the next label's
                 * LEN whose value the builder cannot know — one SKIP edge
                 * becomes the 63 real AC_CLS_LEN rows (the walk's whole
                 * legal length range; anything above DNS_LABEL_MAX_LEN
                 * aborts the parse upstream). ~0.2% of
                 * AC_TRANS_MAX_ENTRIES per rule; the BPF match path stays
                 * plain map lookups either way. The allow-side label
                 * confinement is NOT expressed here — it lives in
                 * ac_prefix_scan's AC_FLAG_PREFIX_ALLOW_LC clearing. */
                for (uint32_t c = 1; c <= DNS_LABEL_MAX_LEN; ++c) {
                    AcTransEntry entry;
                    entry.key.uid = uid;
                    entry.key.appuid = appuid;
                    entry.key.state = newId[node];
                    entry.key.cls = AC_CLS_LEN(c);
                    entry.key.gen = gen;
                    entry.val.next = newId[edge.second];
                    entry.val.flags = enterFlags[edge.second];
                    entry.val.rem = 0;
                    out.push_back(entry);
                }
                continue;
            }
            if (edge.first == AC_BUILDER_CLS_MORE) {
                /* Wildcard edge expansion: the walk position after a label
                 * LEN is always a content CHAR of the preceding label, so
                 * the real class consumed here can be any CHAR; LEN rows
                 * are cheap defense (TERM was already consumed at the
                 * anchor and cannot recur mid-walk). One MORE edge becomes
                 * 63 + 256 transition rows — ~1% of AC_TRANS_MAX_ENTRIES
                 * per anchored rule, and the BPF-side scan is untouched
                 * (plain map lookups either way).
                 *
                 * Collision guard: a CONCRETE edge with the same class at
                 * this node would share the expansion row's key — the
                 * single-writer map keeps only the last row and one of the
                 * two rules silently vanishes. The concrete row wins the
                 * class (it continues into the other rule's subtrie) and
                 * the anchored rule's completion rides along on it below,
                 * so the expansion just skips the covered classes (the
                 * MORE target is always a leaf — MORE is terminal —
                 * nothing of a continuation is lost). */
                for (uint32_t c = 1; c <= DNS_LABEL_MAX_LEN; ++c) {
                    if (nodeEdges.find(AC_CLS_LEN(c)) != nodeEdges.end()) {
                        continue;
                    }
                    AcTransEntry entry;
                    entry.key.uid = uid;
                    entry.key.appuid = appuid;
                    entry.key.state = newId[node];
                    entry.key.cls = AC_CLS_LEN(c);
                    entry.key.gen = gen;
                    entry.val.next = newId[edge.second];
                    entry.val.flags = enterFlags[edge.second];
                    entry.val.rem = 0;
                    out.push_back(entry);
                }
                for (uint32_t c = 0; c <= 0xFF; ++c) {
                    if (nodeEdges.find(AC_CLS_CHAR(c)) != nodeEdges.end()) {
                        continue;
                    }
                    AcTransEntry entry;
                    entry.key.uid = uid;
                    entry.key.appuid = appuid;
                    entry.key.state = newId[node];
                    entry.key.cls = AC_CLS_CHAR(c);
                    entry.key.gen = gen;
                    entry.val.next = newId[edge.second];
                    entry.val.flags = enterFlags[edge.second];
                    entry.val.rem = 0;
                    out.push_back(entry);
                }
                continue;
            }
            AcTransEntry entry;
            entry.key.uid = uid;
            entry.key.appuid = appuid;
            entry.key.state = newId[node];
            entry.key.cls = edge.first;
            entry.key.gen = gen;
            entry.val.next = newId[edge.second];
            entry.val.flags = enterFlags[edge.second];
            if (moreIt != nodeEdges.end()) {
                /* This concrete class is one the MORE expansion covers
                 * (see the collision guard above): the anchored rule
                 * completes on this very consumed class, so its
                 * completion flags ride along on the concrete row. */
                entry.val.flags |= enterFlags[moreIt->second];
            }
            entry.val.rem = 0; /* reserved for the builder — the scans take
                                * label boundaries from the forward bitmap */
            out.push_back(entry);
        }
    }
}

inline bool AcDomainTableBuilder::CheckStarForm(const std::string &domain, const char **reason, size_t *starCount,
                                                size_t *lastStar)
{
    if (reason != nullptr) {
        *reason = nullptr;
    }
    size_t count = 0;
    size_t last = 0;
    for (size_t i = 0; i < domain.size(); ++i) {
        if (domain[i] == '*') {
            ++count;
            last = i;
        }
    }
    if (starCount != nullptr) {
        *starCount = count;
    }
    if (lastStar != nullptr) {
        *lastStar = last;
    }
    if (count == 0 || (count == 1 && (last == 0 || last + 1 == domain.size()))) {
        return true;
    }
    if (reason != nullptr) {
        *reason = (count > 1) ? "more than one '*'" : "the single '*' must be at the front or the back";
    }
    return false;
}

inline bool AcDomainTableBuilder::BuildPattern(const std::string &domain, bool isPrefixRule, std::vector<uint32_t> &cls)
{
    cls.clear();
    if (domain.empty()) {
        return false;
    }
    /* Star-form gate (fail-visible, CheckStarForm is the single source of
     * truth): at most one '*', at the very front or the very back. A
     * mid-label or duplicated star is unrepresentable in the label-aligned
     * class space — the legacy normalization stripped every '*' silently,
     * which downgraded "*w.example.com" to the bare domain
     * "w.example.com" and "www.example.c*" to a pattern whose last label
     * had to be the literal "c" (measured on-device: both forms fell
     * through to the default deny while AddRule reported success, zero
     * log). Every unsupported star form is now rejected so the caller
     * skips + logs it instead of arming a rule with different semantics
     * than the admin wrote. */
    size_t starCount = 0;
    size_t lastStar = 0;
    if (!CheckStarForm(domain, nullptr, &starCount, &lastStar)) {
        return false;
    }
    /* Same normalization as GetDomainHashKey: strip every '*', then the
     * DNS_DOMAIN_LEN cap. The label checks below are the structural ones
     * the BPF scan enforces (label > 63 aborts the parse, an empty label
     * can never appear in a wire name). */
    std::string text;
    text.reserve(domain.size());
    for (char ch : domain) {
        if (ch != '*') {
            text.push_back(ch);
        }
    }
    if (text.empty() || text.length() >= DNS_DOMAIN_LEN) {
        return false;
    }
    const bool frontStar = starCount == 1 && lastStar == 0;
    const bool backStar = starCount == 1 && lastStar + 1 == domain.size();
    /* Leading-label fragment: a suffix rule whose '*' opens a NON-EMPTY
     * first label ("*w.example.com"): the label right before the
     * pattern's labels ends with the fragment, everything further left is
     * the glob '*' — the label-aligned pattern body keeps every match
     * inside the written suffix, no escape to a look-alike registrable
     * domain. Encoded as a truncated backward pattern (see the suffix
     * branch). Evaluated BEFORE the subdomain-anchor erase below: the
     * anchor form "*.example.com" also has a non-dot front AFTER the
     * erase ("example..."), which used to misclassify it here — the
     * anchor lost its first full label's LEN class and the MORE tail
     * then swallowed the label boundary, matching the bare domain
     * ("example.com" hit "*.example.com", AcScanLeadingStarSuffix001). */
    const bool leadingFragment = !isPrefixRule && frontStar && text.front() != '.';
    /* Legacy subdomain anchor: a suffix rule whose '*' leaves a LEADING dot
     * ("*.baidu.com" -> ".baidu.com") is the spec-widening wildcard form —
     * legacy reversed-LPM kept the dot as a one-more-label requirement
     * (subdomains only). Strip it here and re-express the requirement as
     * the MORE tail class below; without this the empty first label made
     * the whole rule unrepresentable and the builder silently dropped it
     * (measured on-device: "*.baidu.com" fuzzy rules never fired). */
    bool subdomainOnly = !isPrefixRule && frontStar && text.front() == '.';
    if (subdomainOnly) {
        text.erase(0, 1);
        if (text.empty()) { /* the domain was a bare "*." */
            return false;
        }
    }
    /* Trailing-star prefix rules ("www.example.c*" / "www.example.*" /
     * "ads*"): the '*' completes the LAST label — the SKIP consumes the
     * next label's LEN of any value, the fragment after it (nothing, for
     * the whole-label form) prefixes that label's content. Verdict
     * semantics: AC_BUILDER_CLS_SKIP / AC_FLAG_PREFIX_ALLOW_LC. */
    const bool trailingStar = isPrefixRule && backStar;
    bool trailingFullLabel = false;
    if (trailingStar && text.back() == '.') {
        /* "www.example.*": the star stands for a WHOLE final label — the
         * trailing dot is the separator the SKIP consumes, not an empty
         * label; drop it so the split sees full labels only
         * ("www.example..*" still fails on the empty label). */
        text.pop_back();
        if (text.empty()) { /* the domain was a bare ".*" */
            return false;
        }
        trailingFullLabel = true;
    }
    std::vector<std::string> labels;
    size_t start = 0;
    while (true) {
        size_t dot = text.find('.', start);
        std::string label = text.substr(start, (dot == std::string::npos) ? std::string::npos : dot - start);
        if (label.empty() || label.length() > DNS_LABEL_MAX_LEN) {
            return false;
        }
        labels.push_back(std::move(label));
        if (dot == std::string::npos) {
            break;
        }
        start = dot + 1;
    }
    if (trailingStar) {
        /* Forward encoding, anchored at the name start (single descent
         * from root, position 0): the full labels are exact, the SKIP
         * consumes the final label's LEN (any 1..63), the fragment that
         * follows (nothing, for "x.*") must PREFIX that label's content.
         * The pattern must fit the scan window before the terminator
         * position: at most AC_NAME_SCAN_MAX - 1 content positions. */
        size_t fullCount = trailingFullLabel ? labels.size() : labels.size() - 1;
        for (size_t i = 0; i < fullCount; ++i) {
            const auto &label = labels[i];
            cls.push_back(AC_CLS_LEN(static_cast<uint32_t>(label.length())));
            for (char ch : label) {
                cls.push_back(AC_CLS_CHAR(static_cast<uint32_t>(AcFoldByte(static_cast<uint8_t>(ch)))));
            }
        }
        cls.push_back(AC_BUILDER_CLS_SKIP);
        if (!trailingFullLabel) {
            for (char ch : labels.back()) {
                cls.push_back(AC_CLS_CHAR(static_cast<uint32_t>(AcFoldByte(static_cast<uint8_t>(ch)))));
            }
        }
        if (cls.size() > AC_NAME_SCAN_MAX - 1) {
            return false;
        }
    } else if (isPrefixRule) {
        /* Plain forward encoding (no trailing star): the legacy
         * label-exact prefix form, unchanged. */
        for (const auto &label : labels) {
            cls.push_back(AC_CLS_LEN(static_cast<uint32_t>(label.length())));
            for (char ch : label) {
                cls.push_back(AC_CLS_CHAR(static_cast<uint32_t>(AcFoldByte(static_cast<uint8_t>(ch)))));
            }
        }
        if (cls.size() > AC_NAME_SCAN_MAX - 1) {
            return false;
        }
    } else {
        /* Backward encoding, anchored at the name terminator: TERM, then
         * the LAST label's content reversed, its LEN, ... down to the
         * FIRST full label's LEN — the exact class sequence
         * ac_suffix_scan consumes walking a matching name backwards. */
        size_t firstFull = leadingFragment ? 1 : 0;
        cls.push_back(AC_CLS_TERM);
        for (size_t i = labels.size(); i-- > firstFull;) {
            const auto &label = labels[i];
            for (size_t j = label.length(); j-- > 0;) {
                cls.push_back(AC_CLS_CHAR(static_cast<uint32_t>(AcFoldByte(static_cast<uint8_t>(label[j])))));
            }
            cls.push_back(AC_CLS_LEN(static_cast<uint32_t>(label.length())));
        }
        if (leadingFragment) {
            /* Truncated tail: the fragment's reversed chars with NO closing
             * LEN and no MORE. The AC occurrence semantics complete the
             * pattern inside the label preceding the pattern body (that
             * label ends with the fragment) and the latched flags survive
             * the dead remainder of the walk — anything further left is
             * the glob '*', including the name "w.example.com" itself
             * (glob '*' = empty). A fragment longer than that label runs
             * into its LEN class and misses (class disjointness). */
            const auto &frag = labels.front();
            for (size_t j = frag.length(); j-- > 0;) {
                cls.push_back(AC_CLS_CHAR(static_cast<uint32_t>(AcFoldByte(static_cast<uint8_t>(frag[j])))));
            }
        }
        /* The anchor tail: one MORE class demands at least one more class
         * (hence one more label) BEFORE the pattern's first label — the
         * "subdomains only" requirement. Bare-domain names end the
         * backward stream right at the first label's LEN, so the MORE
         * edge is never consumed and they do not match. */
        if (subdomainOnly) {
            cls.push_back(AC_BUILDER_CLS_MORE);
        }
        if (cls.size() > AC_NAME_SCAN_MAX) {
            return false;
        }
    }
    return true;
}

inline bool AcDomainTableBuilder::AddRule(uint32_t uid, uint32_t appuid, bool isAllow, const std::string &domain)
{
    if (domain.empty()) {
        return false;
    }
    bool isPrefixRule = domain.back() == '*';
    std::vector<uint32_t> cls;
    if (!BuildPattern(domain, isPrefixRule, cls)) {
        return false;
    }
    /* Trailing-star prefix rules carry the SKIP class (the '*' completes
     * the last label). A DENY verdict takes the plain PREFIX_DENY bit:
     * right-unconstrained on purpose — a deny stays valid past the last
     * label (fail-closed: "ads*" keeps blocking ads.example.com). An
     * ALLOW verdict must not leak past the last label ("www.example.c*"
     * must not allow www.example.c.evil.com): it latches the provisional
     * AC_FLAG_PREFIX_ALLOW_LC bit, which ac_prefix_scan clears at any
     * later label boundary and promotes to the standard allow bit at the
     * terminator. */
    bool labelConfinedAllow = false;
    if (isPrefixRule && isAllow) {
        for (uint32_t c : cls) {
            if (c == AC_BUILDER_CLS_SKIP) {
                labelConfinedAllow = true;
                break;
            }
        }
    }
    uint32_t done = 0;
    if (isPrefixRule) {
        if (isAllow) {
            done = labelConfinedAllow ? AC_FLAG_PREFIX_ALLOW_LC : AC_FLAG_PREFIX_ALLOW;
        } else {
            done = AC_FLAG_PREFIX_DENY;
        }
    } else {
        done = isAllow ? AC_FLAG_SUFFIX_ALLOW : AC_FLAG_SUFFIX_DENY;
    }
    AcNamespace &ns = namespaces_[std::make_pair(uid, appuid)];
    AcTrie &trie = isPrefixRule ? ns.prefix : ns.suffix;
    trie.Insert(cls, done);
    return true;
}

inline bool AcDomainTableBuilder::Emit(uint32_t gen, std::vector<AcTransEntry> &prefixOut,
                                       std::vector<AcTransEntry> &suffixOut) const
{
    prefixOut.clear();
    suffixOut.clear();
    for (const auto &ns : namespaces_) {
        ns.second.prefix.Emit(ns.first.first, ns.first.second, gen, prefixOut);
        ns.second.suffix.Emit(ns.first.first, ns.first.second, gen, suffixOut);
    }
    if (prefixOut.size() > AC_TRANS_MAX_ENTRIES || suffixOut.size() > AC_TRANS_MAX_ENTRIES) {
        return false; /* fail fast: the caller must not flip the generation */
    }
    return true;
}

/**
 * Class for setup bpf maps and poll event from bpf ring buffer
 */
class NetsysBpfNetFirewall : public NoCopyable {
public:
    static std::shared_ptr<NetsysBpfNetFirewall> GetInstance();

    /**
     * start to listen bpf ring buffer
     *
     * @return 0 if success or -1 if an error occurred
     */
    int32_t StartListener();

    /* *
     * @brief stop listen bpf ring buffer
     *
     * @return 0 if success or -1 if an error occurred
     */
    int32_t StopListener();

    /**
     * Set firewall rules to native
     *
     * @param type type of NetFirewallRuleType
     * @param ruleList list of NetFirewallIpRule
     * @param isFinish transmit finish or not
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallRules(NetFirewallRuleType type, const std::vector<sptr<NetFirewallBaseRule>> &ruleList,
                             bool isFinish);

    /**
     * Set firewall default action
     *
     * @param userId user id
     * @param inDefault  Default action of NetFirewallRuleDirection:RULE_IN
     * @param outDefault Default action of NetFirewallRuleDirection:RULE_OUT
     * @return  0 if success or -1 if an error occurred
     */
    int32_t SetFirewallDefaultAction(int32_t userId, FirewallRuleAction inDefault, FirewallRuleAction outDefault);

    /**
     * Set firewall current user id
     *
     * @param userId current user id
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallCurrentUserId(int32_t userId);

    /**
     * Set the evaluation fallback mode: OBSERVE restores the legacy pass-through
     * behavior on evaluation failures; FAIL_CLOSED (map default, no entry) falls back
     * to the per-uid default action. Local management entry (open item), no new IPC
     * command code.
     *
     * @param mode 0 = EVAL_FALLBACK_OBSERVE, 1 = EVAL_FALLBACK_FAIL_CLOSED
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetEvalFallbackMode(uint32_t mode);

    /**
     * Clear all bpf maps
     *
     * @param type type of NetFirewallRuleType
     * @return  0 if success or -1 if an error occurred
     */
    int32_t ClearFirewallRules(NetFirewallRuleType type);

    /**
     * Register callback for recevie intercept event
     *
     * @param callback implement of INetFirewallCallback
     * @return 0 if success or -1 if an error occurred
     */
    int32_t RegisterCallback(const sptr<NetsysNative::INetFirewallCallback> &callback);

    /**
     * Unregister callback for recevie intercept event
     *
     * @param callback register callback for recevie intercept event
     * @return 0 if success or -1 if an error occurred
     */
    int32_t UnregisterCallback(const sptr<NetsysNative::INetFirewallCallback> &callback);

    /**
     * Register an internal handler invoked for egress TCP intercept events.
     * Only used inside netmanagernative to close the intercepted socket.
     *
     * @param handler handler invoked with the parsed intercept record and address family
     */
    void RegisterInterceptHandler(std::function<void(const InterceptRecord &, uint8_t family)> handler);

    /**
     * Load SA on demand
     *
     * @param systemAbilityId id of SA want to load
     * @return 0 if success or -1 if an error occurred
     */
    int32_t LoadSystemAbility(int32_t systemAbilityId);

    /**
     * Set bpf prog load state
     *
     * @param load true if load success or false if an error occurred
     */
    void SetBpfLoaded(bool load);

    /**
     * Get bpf prog load state
     */
    bool IsBpfLoaded()
    {
        return isBpfLoaded_;
    }

    /* The dead AddDomainCache(NetAddrInfo) control-plane entry was
     * removed with the domain_value timestamp/ttl extension — it wrote an
     * all-zero value (uid=0/appuid=0) and had no callers. The pre-warm
     * engine gets a proper writer that fills uid/appuid/timestamp/ttl. */

    void ClearDomainCache();

    /* *
     * @brief Full DNS query record table clear (user space only
     * ever reads/clears it — the BPF flush point writes). Hooked into
     * ClearDomainRules: a domain-rule change voids the old allow verdicts, so
     * the pending query credentials are void too. Public for unit tests
     * (mirrors DomainCacheAgingSweep).
     */
    void ClearDnsQueryRecordMap();

    /**
     * second-layer aging: one deterministic reclaim sweep over the
     * learned-entry DOMAIN maps. Public for unit tests (retail devices without
     * the firewall maps report openFailed and delete nothing — first-layer
     * read-time invalidation keeps correctness independent of this sweep).
     *
     * @return 0 if both maps were swept (possibly empty), -1 if a map could
     *         not be opened (the other map may still have been swept)
     */
    int32_t DomainCacheAgingSweep();

    /**
     * clear firewall default action
     *
     * @return 0 if success or -1 if an error occurred
     */
    int32_t ClearFirewallDefaultAction();

    /**
     * clear conntrack map
     *
     * @return 0 if success or -1 if an error occurred
     */
    int32_t ClearConntrackMap();

    /* ===== DNS server authorization map maintenance ===== */

    /* Parse/validate/dedupe/cap a resolver server list into map keys. Pure
     * function (unit-testable): userId < 0 rejected, malformed addresses
     * skipped, duplicates collapsed, combined cap DNS_SERVER_MAX_PER_USER
     * (never write partial or garbage keys). */
    static int32_t BuildDnsServerKeys(const std::vector<std::string> &servers, int32_t userId,
                                      std::vector<DnsServerV4Key> &v4Keys, std::vector<DnsServerV6Key> &v6Keys);

    /* Make the user's authorized server set exactly `servers` (delta write:
     * stale keys deleted, new keys written). 0 on full success. */
    int32_t SetDnsServerMaps(int32_t userId, const std::vector<std::string> &servers);

    /* Remove every authorization entry of the user (rule clear / user removal
     * / default action no longer deny). */
    int32_t ClearDnsServerMaps(int32_t userId);

    /* Sync triggers (timing table). The registry update is
     * synchronous; the map reconciliation is rescheduled asynchronously so no
     * dns-layer lock is ever acquired under rulesMutex_ (lock ordering:
     * cacheMutex_ -> rulesMutex_ already exists and must not be reversed). */
    void NotifyDnsRuleChanged(int32_t userId, bool hasAllowDomain);
    void NotifyDnsDefaultActionChanged(int32_t userId, bool defaultDeny);
    /* Resolver set changed (network switch / DHCP renew / network destroyed):
     * re-sync every managed user and re-run the pre-warm engine. */
    void NotifyDnsResolverChanged();
    /* User space removal (ext orchestration): drop registries and entries. */
    void RemoveDnsServerUser(int32_t userId);

    /* Whether the user currently meets the write condition (default
     * deny AND >= 1 domain allow rule) — the unit-test surface of the sync
     * condition. */
    bool DnsServerSyncNeeded(int32_t userId);

    /* Pre-warm eligibility: >= 1 domain allow rule remains, with NO
     * default-action precondition — a default-allow user's learned entry is
     * what carries the domain allow past explicit deny rules, so those users
     * need the warmed cache too. The DNS server authorization maps keep the
     * stricter condition above. */
    bool DnsPrewarmNeeded(int32_t userId);

    /* Synchronous one-user reconciliation (public for unit tests; production
     * flow enters through the async Notify* triggers). */
    int32_t SyncUserDnsServerMaps(int32_t userId);

    /* ===== Pre-warm engine ===== */

    /* Write one pre-resolved domain's addresses into the learned DOMAIN maps
     * with system attribution: value = {uid: rule userId, appuid: 0,
     * timestamp, ttl} — the 0-appuid relaxation in MatchDomain makes the
     * entries valid for every app of that user. */
    int32_t AddPrewarmDomainCache(int32_t userId, const std::string &domain, const std::vector<std::string> &ips);

    /* Async re-resolve of every registered exact allow domain (triggers:
     * rule change / resolver set change / TTL refresh). */
    void TriggerDnsPrewarm();

    /* dns-layer providers, registered by DnsManager at construction: the
     * base-chain server set (GetResolverConfig without the RULE_DNS
     * substitution) and the system-identity resolver. */
    void RegisterDnsServerSetProvider(std::function<int32_t(std::vector<std::string> &)> provider);
    void RegisterDnsPrewarmResolver(std::function<int32_t(const std::string &, std::vector<std::string> &)> resolver);
    /* dns-layer cache flush provider, registered by DnsManager at
     * construction: every completed rule/default mutation calls it so
     * stale allow-era cached DNS answers cannot bypass a freshly set deny
     * (the cache2 query path serves hits without egress re-adjudication).
     * Provider returns the number of flushed netIds, negative on failure. */
    void RegisterDnsCacheFlushProvider(std::function<int32_t()> provider);

    /* ===== Built-in reserved default actions ===== */

    /* Write the two reserved DEFAULT_ACTION_MAP entries the system identity
     * contract needs — FIREWALL_SYSTEM_USER_ID = allow (uid < USER_ID_DIVIDOR
     * traffic, never folded to the foreground user) and
     * FIREWALL_UNATTRIBUTED_USER_ID = allow (fail-open baseline for
     * unattributed packets, now explicit and dumpable). Called once after the
     * BPF program loads; a write failure reports NET_FIREWALL_INIT_FAULT and
     * returns non-zero. Both keys remain writable through
     * SetFirewallDefaultAction (ext keeps an escape hatch). */
    int32_t SetBuiltInFirewallDefaultActions();

private:
    template <typename Key, typename Value> int ClearBpfMap(const char *path, const Key &key, Value &val)
    {
        (void)key;
        (void)val;
        BpfMapper<Key, Value> rdMap(path, BPF_F_RDONLY);
        if (!rdMap.IsValid()) {
            return -1;
        }
        std::vector<Key> keys = rdMap.GetAllKeys();
        if (keys.empty()) {
            return 0;
        }
        BpfMapper<Key, Value> wrMap(path, BPF_F_WRONLY);
        if (!wrMap.IsValid()) {
            NETNATIVE_LOGE("ClearBpfMap: wrMap is invalid");
            return -1;
        }
        if (wrMap.Clear(keys) != 0) {
            NETNATIVE_LOGE("ClearBpfMap: clear failed");
            return -1;
        }

        return 0;
    }

    template <typename Key, typename Value> int WriteBpfMap(const char *path, const Key &key, Value &val)
    {
        BpfMapper<Key, Value> map(path, BPF_F_WRONLY);
        if (!map.IsValid()) {
            NETNATIVE_LOGE("WriteBpfMap: map invalid: %{public}s", path);
            return -1;
        }

        if (map.Write(key, val, BPF_ANY) != 0) {
            NETNATIVE_LOGE("WriteBpfMap: map write failed");
            return -1;
        }

        return 0;
    }

    struct BpfMapSnapshot {
        bool valid = false;
        std::unordered_map<std::string, std::string> entries;
    };

    template <typename Key> static std::string KeyToBytes(const Key &key)
    {
        return std::string(reinterpret_cast<const char *>(&key), sizeof(Key));
    }

    template <typename Key> static Key KeyFromBytes(const std::string &bytes)
    {
        Key key{};
        if (bytes.size() == sizeof(Key)) {
            (void)memcpy_s(&key, sizeof(Key), bytes.data(), sizeof(Key));
        }
        return key;
    }

    template <typename Value> static std::string ValueToBytes(const Value &value)
    {
        return std::string(reinterpret_cast<const char *>(&value), sizeof(Value));
    }

    /* ===== Rule-dimension map writers (common funnel) =====
     * The per-dimension Write*BpfMap bodies were identical except for the
     * container, key type and map path — both container shapes (unordered
     * scalar-key maps, vector-of-nodes LPM/port maps) funnel through these
     * two templates into ReconcileBpfMap. Empty desired set legitimately
     * clears the target map (dimension unconstrained by any rule); the
     * wildcard map carries the fallback entries. RuleCode marshalling
     * checks memcpy_s on every entry (the former scalar writers ignored
     * the return value — unified here). */
    template <typename Key, typename DimMap> int32_t WriteRuleCodeMap(DimMap &dimMap, const char *path, const char *tag)
    {
        std::vector<std::pair<Key, RuleCode>> desired;
        desired.reserve(dimMap.Get().size());
        for (const auto &entry : dimMap.Get()) {
            RuleCode rule = {0};
            if (memcpy_s(rule.val, sizeof(RuleCode), entry.second.Get(), sizeof(RuleCode)) != EOK) {
                NETNATIVE_LOGE("%{public}s: memcpy_s failed", tag);
                return -1;
            }
            NETNATIVE_LOG_D("%{public}s key=%{public}u", tag, static_cast<uint32_t>(entry.first));
            desired.emplace_back(entry.first, rule);
        }
        return ReconcileBpfMap(path, desired, false);
    }

    template <typename LpmKey, typename Node>
    int32_t WriteRuleCodeLpmMap(const std::vector<Node> &nodes, uint32_t Node::*plenMember, const char *path,
                                const char *tag)
    {
        std::vector<std::pair<LpmKey, RuleCode>> desired;
        desired.reserve(nodes.size());
        for (const auto &node : nodes) {
            RuleCode rule = {0};
            if (memcpy_s(rule.val, sizeof(RuleCode), node.bitmap.Get(), sizeof(RuleCode)) != EOK) {
                NETNATIVE_LOGE("%{public}s: memcpy_s failed", tag);
                return -1;
            }
            LpmKey key = {0};
            key.prefixlen = node.*plenMember;
            key.data = static_cast<decltype(key.data)>(node.data);
            desired.emplace_back(key, rule);
        }
        return ReconcileBpfMap(path, desired, false);
    }

    template <typename Key, typename Value>
    int32_t ReconcileBpfMap(const char *path, const std::vector<std::pair<Key, Value>> &desired, bool forceFull)
    {
        if (path == nullptr || *path == '\0') {
            return -1;
        }
        std::lock_guard<std::mutex> snapshotGuard(snapshotMutex_);
        BpfMapSnapshot &snapshot = appliedSnapshots_[path];
        BpfMapper<Key, Value> map(path, 0);
        if (!map.IsValid()) {
            NETNATIVE_LOGE("ReconcileBpfMap: map invalid: %{public}s", path);
            return -1;
        }
        if (!snapshot.valid || forceFull) {
            return ReconcileBpfMapFull(map, path, desired, snapshot);
        }
        return ReconcileBpfMapDelta(map, path, desired, snapshot);
    }

    template <typename Key, typename Value>
    int32_t ReconcileBpfMapFull(BpfMapper<Key, Value> &map, const char *path,
                                const std::vector<std::pair<Key, Value>> &desired, BpfMapSnapshot &snapshot)
    {
        std::vector<Key> oldKeys = map.GetAllKeys();
        if (!oldKeys.empty() && map.Clear(oldKeys) != 0) {
            NETNATIVE_LOGE("ReconcileBpfMap: clear failed: %{public}s", path);
            return -1;
        }
        std::unordered_map<std::string, std::string> entries;
        entries.reserve(desired.size());
        for (const auto &item : desired) {
            if (map.Write(item.first, item.second, BPF_ANY) != 0) {
                NETNATIVE_LOGE("ReconcileBpfMap: write failed: %{public}s", path);
                return -1;
            }
            entries[KeyToBytes(item.first)] = ValueToBytes(item.second);
        }
        snapshot.entries = std::move(entries);
        snapshot.valid = true;
        return 0;
    }

    template <typename Key, typename Value>
    int32_t ReconcileBpfMapDelta(BpfMapper<Key, Value> &map, const char *path,
                                 const std::vector<std::pair<Key, Value>> &desired, BpfMapSnapshot &snapshot)
    {
        std::unordered_set<std::string> desiredKeys;
        desiredKeys.reserve(desired.size());
        for (const auto &item : desired) {
            std::string keyBytes = KeyToBytes(item.first);
            std::string valueBytes = ValueToBytes(item.second);
            desiredKeys.emplace(keyBytes);
            auto it = snapshot.entries.find(keyBytes);
            if (it == snapshot.entries.end() || it->second != valueBytes) {
                if (map.Write(item.first, item.second, BPF_ANY) != 0) {
                    NETNATIVE_LOGE("ReconcileBpfMap: update failed: %{public}s", path);
                    return -1;
                }
                snapshot.entries[keyBytes] = std::move(valueBytes);
            }
        }
        return ReconcileBpfMapDeleteStale(map, path, desiredKeys, snapshot);
    }

    template <typename Key, typename Value>
    int32_t ReconcileBpfMapDeleteStale(BpfMapper<Key, Value> &map, const char *path,
                                       const std::unordered_set<std::string> &desiredKeys, BpfMapSnapshot &snapshot)
    {
        for (auto it = snapshot.entries.begin(); it != snapshot.entries.end();) {
            if (desiredKeys.find(it->first) == desiredKeys.end()) {
                Key key = KeyFromBytes<Key>(it->first);
                if (map.Delete(key) != 0) {
                    NETNATIVE_LOGE("ReconcileBpfMap: delete failed: %{public}s", path);
                    return -1;
                }
                it = snapshot.entries.erase(it);
            } else {
                ++it;
            }
        }
        return 0;
    }

    NetsysBpfNetFirewall();

    static void StartConntrackGcThread(void);

    static void RingBufferListenThread(void);

    void StopConntrackGc();

    /* Periodic learned-entry reclaim thread (60s). Plain BpfMapper
     * walking, no locks held, no callbacks — a stuck or failing sweep never
     * affects correctness (the BPF read-time gate is the safety layer). */
    static void StartDomainCacheAgingThread(void);

    static int HandleEvent(void *ctx, void *data, size_t len);

    static void HandleTupleEvent(TupleEvent *ev);

    static void HandleInterceptEvent(InterceptEvent *ev);

    static void HandleDebugEvent(DebugEvent *ev);

    void NotifyInterceptEvent(InterceptEvent *info);

    void NotifyEgressIntercept(const InterceptEvent &info, const sptr<InterceptRecord> &record);

    static void ConntrackGcTask();

    uint64_t GetNowMs();

    void ClearBpfFirewallRules(NetFirewallRuleDirection direction);

    void InvalidateIpSnapshots(NetFirewallRuleDirection direction);

    void InvalidateAllSnapshots();

    int32_t WriteSrcIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteSrcIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteDstIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteDstIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WritePortBpfMap(BpfPortMap &portMap, const char *path);

    int32_t WriteSrcPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteDstPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteProtoBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteAppUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteActionBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteInterfaceBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t WriteWildcardBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t SetBpfFirewallRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList,
                                NetFirewallRuleDirection direction);

    int32_t SetFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList);

    int32_t SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList);

    /* Legacy LPM normalization kept as the differential baseline for the
     * AC builder tests (BuildPattern vs GetDomainHashKey byte-equality);
     * no map write rides on it anymore — the AC tables are the only
     * BPF-side consumer of domain rules. */
    bool GetDomainHashKey(const std::string &domain, DomainHashKey &out, bool isReversed = false);

    /* clearAcTables: skip the A' trie table clear when the caller rebuilds
     * them in full right after (SetFirewallDomainRules) — the rebuild's
     * generation flip is atomic, an intermediate empty flip would only open
     * a no-verdict window for no benefit. */
    void ClearDomainRules(bool clearAcTables);
    /* Legacy no-arg overload: keeps the pre-AC build's exported symbol
     * (ClearDomainRulesEv) alive for binary compatibility across a
     * so-only upgrade — same semantics as ClearDomainRules(true). */
    void ClearDomainRules();

    /* ===== A' AC trie table maintenance (see AcDomainTableBuilder) =====
     * Map I/O follows the existing domain-rule path: synchronous, under the
     * caller's rulesMutex_, never touching any dns-layer lock (the
     * cacheMutex_ -> rulesMutex_ ordering must not be reversed). */

    /* Aggregate every domain rule, compile the AC tables and switch them in
     * one generation flip (full-replace — the BPF side consumes only these
     * tables). Fail-fast protocol: capacity
     * overflow or any map failure aborts BEFORE the flip, the previous
     * generation stays online and the error propagates (a silently
     * unenforced deny rule must never look applied). */
    int32_t BuildAcDomainTables(const std::vector<sptr<NetFirewallDomainRule>> &ruleList);

    /* Full clear of both transition tables (rule clear / full-replace to
     * empty): every scan misses afterwards — no verdict, the packet keeps
     * flowing through the policy chain. Map-less environments are silent
     * by design (ClearDnsQueryRecordMap pattern). */
    void ClearAcDomainTables();

    /* Delete transition rows of one generation: match=true deletes exactly
     * `gen` (pre-sweep of the build target — never write on top of unknown
     * rows, this heals any residue a failed/crashed build left behind);
     * match=false deletes every generation but `gen` (retired-generation
     * sweep after the flip). Per-row delete failures are log-only (a stale
     * row is unreachable through the active generation, the next build's
     * pre-sweep heals it); the return is non-zero only when the map cannot
     * be opened or walked. */
    int32_t SweepAcGenEntries(const char *path, uint32_t gen, bool match);

    int32_t WriteLoopBackBpfMap();

    static std::shared_ptr<NetsysBpfNetFirewall> instance_;
    static std::atomic<bool> isBpfLoaded_;
    static std::atomic<bool> keepListen_;
    std::unique_ptr<std::thread> thread_;
    std::vector<sptr<NetsysNative::INetFirewallCallback>> callbacks_;
    std::mutex callbackMutex_;
    std::function<void(const InterceptRecord &, uint8_t)> interceptHandler_;
    std::mutex handlerMutex_;
    static std::atomic<bool> keepGc_;
    static std::atomic<bool> keepAging_;
    std::unique_ptr<std::thread> gcThread_;
    static std::unique_ptr<BpfMapper<CtKey, CtVaule>> ctRdMap_, ctWrMap_;
    std::vector<sptr<NetFirewallIpRule>> firewallIpRules_;
    std::vector<sptr<NetFirewallDomainRule>> firewallDomainRules_;
    std::mutex rulesMutex_;
    std::mutex snapshotMutex_;
    std::unordered_map<std::string, BpfMapSnapshot> appliedSnapshots_;
    /* ===== Control-plane state =====
     * Guarded by dnsServerMutex_ (never taken while holding a dns-layer
     * lock; acquired only one way: caller locks -> here, never here -> out).
     * The map reconciliation itself runs lock-free on the async paths so
     * slow map I/O never blocks rule application. */
    mutable std::mutex dnsServerMutex_;
    std::map<int32_t, bool> dnsUserHasAllowDomain_;              /* >= 1 allow domain rule */
    std::map<int32_t, bool> dnsUserDefaultDeny_;                 /* egress default action is deny */
    std::map<int32_t, std::set<std::string>> dnsPrewarmDomains_; /* exact allow domains */
    std::function<int32_t(std::vector<std::string> &)> dnsServerSetProvider_;
    std::function<int32_t(const std::string &, std::vector<std::string> &)> dnsPrewarmResolver_;
    std::function<int32_t()> dnsCacheFlushProvider_;

    /* Post-mutation DNS cache flush (rulesMutex_-free call site required:
     * the provider takes the dns-layer cache mutex, and the only
     * established cross-layer order is cacheMutex_ -> dnsServerMutex_). */
    void FlushDnsCacheAfterRuleChange();
    /* Pre-warm trigger coalescing (debounce) —
     * resolver-change bursts (network flapping) see at most one sweep in
     * flight plus a single merged re-run (which re-snapshots the then-
     * current state) instead of N concurrent seconds-long resolver sweeps.
     * Invariants: dnsPrewarmPending_ is only ever set while
     * dnsPrewarmInFlight_ is true, and each store of pending is consumed
     * at most once (atomic exchange) by a sweep tail. Narrow window
     * A pending store landing between the
     * releasing sweep's slot store and its pending check is consumed by
     * the NEXT sweep tail instead — deferred, never lost; the periodic
     * sweeper (<=10min) backstops freshness long inside the 1h TTL. */
    std::atomic<bool> dnsPrewarmInFlight_ = false;
    std::atomic<bool> dnsPrewarmPending_ = false;
    /* Pre-warm entry lifetime: GetAddrInfo exposes no answer TTL, so a fixed
     * 1h window is used and the periodic re-resolve (every
     * DNS_PREWARM_INTERVAL_SWEEPS aging sweeps, 60s each) refreshes the
     * entries long before expiry (clamp keeps the BPF gate consistent). */
    static constexpr int DNS_PREWARM_TTL_SECONDS = 3600;
    static constexpr int DNS_PREWARM_INTERVAL_SWEEPS = 10;
    static void RunDnsPrewarmTask();
    static void RunDnsServerSyncTask(int32_t userId);
};
} // namespace OHOS::NetManagerStandard
#endif /* NETMANAGER_EXT_BPF_NET_FIREWALL_H */
