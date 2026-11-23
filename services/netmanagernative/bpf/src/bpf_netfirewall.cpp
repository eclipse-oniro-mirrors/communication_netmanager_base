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

#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <ctime>
#include <libbpf.h>
#include <linux/bpf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <regex>
#include <securec.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>

#include "bpf_loader.h"
#include "bpf_netfirewall.h"
#include "bpf_ring_buffer.h"
#include "ffrt_inner.h"
#include "iservice_registry.h"
#include "netnative_log_wrapper.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::NetsysNative;

namespace OHOS {
namespace NetManagerStandard {
std::shared_ptr<NetsysBpfNetFirewall> NetsysBpfNetFirewall::instance_ = nullptr;
std::atomic<bool> NetsysBpfNetFirewall::keepListen_{false};
std::atomic<bool> NetsysBpfNetFirewall::keepGc_{false};
std::atomic<bool> NetsysBpfNetFirewall::keepAging_{false};
std::atomic<bool> NetsysBpfNetFirewall::isBpfLoaded_{false};
std::unique_ptr<BpfMapper<CtKey, CtVaule>> NetsysBpfNetFirewall::ctRdMap_ = nullptr;
std::unique_ptr<BpfMapper<CtKey, CtVaule>> NetsysBpfNetFirewall::ctWrMap_ = nullptr;

namespace {
/* RunDnsServerSyncTask sentinel: full authorization reset (rule/default
 * action clear — no user can meet the write condition afterwards, 绝对隔离). */
constexpr int32_t DNS_SERVER_SYNC_RESET = -1;

/* Key equality: in6_addr is a kernel union without operator==, so v6 keys
 * compare bytewise (both sides are zero-init then filled, no padding). */
inline bool SameServerKey(const DnsServerV4Key &lhs, const DnsServerV4Key &rhs)
{
    return lhs.uid == rhs.uid && lhs.ip == rhs.ip;
}

inline bool SameServerKey(const DnsServerV6Key &lhs, const DnsServerV6Key &rhs)
{
    return lhs.uid == rhs.uid && memcmp(&lhs.ip, &rhs.ip, sizeof(lhs.ip)) == 0;
}

template <typename Key> bool ContainsServerKey(const std::vector<Key> &keys, const Key &key)
{
    for (const Key &k : keys) {
        if (SameServerKey(k, key)) {
            return true;
        }
    }
    return false;
}

template <typename Key> int32_t CollectUserServerKeys(const char *path, uint32_t uid, std::vector<Key> &keys)
{
    BpfMapper<Key, DnsServerVal> rdMap(path, BPF_F_RDONLY);
    if (!rdMap.IsValid()) {
        NETNATIVE_LOGE("CollectUserServerKeys: rdMap invalid: %{public}s", path);
        return NETFIREWALL_ERR;
    }
    for (const Key &k : rdMap.GetAllKeys()) {
        if (k.uid == uid) {
            keys.push_back(k);
        }
    }
    return NETFIREWALL_SUCCESS;
}

/* Delta-apply one family: the user's stale keys are deleted, the desired set
 * is (re)written — SetDnsServerMaps semantics = "exactly this server set". */
template <typename Key>
int32_t ApplyUserServerKeys(const char *path, uint32_t uid, const std::vector<Key> &desired, uint32_t &written,
                            uint32_t &deleted)
{
    std::vector<Key> existing;
    if (CollectUserServerKeys<Key>(path, uid, existing) != NETFIREWALL_SUCCESS) {
        return NETFIREWALL_ERR;
    }
    BpfMapper<Key, DnsServerVal> wrMap(path, BPF_F_WRONLY);
    if (!wrMap.IsValid()) {
        NETNATIVE_LOGE("ApplyUserServerKeys: wrMap invalid: %{public}s", path);
        return NETFIREWALL_ERR;
    }
    for (const Key &k : existing) {
        if (!ContainsServerKey(desired, k) && wrMap.Delete(k) == 0) {
            deleted++;
        }
    }
    DnsServerVal val = {};
    val.managed = 1;
    for (const Key &k : desired) {
        if (wrMap.Write(k, val, BPF_ANY) == 0) {
            written++;
        }
    }
    return NETFIREWALL_SUCCESS;
}

template <typename Key> int32_t ClearUserServerKeys(const char *path, uint32_t uid, uint32_t &deleted)
{
    std::vector<Key> keys;
    if (CollectUserServerKeys<Key>(path, uid, keys) != NETFIREWALL_SUCCESS) {
        return NETFIREWALL_ERR;
    }
    if (keys.empty()) {
        return NETFIREWALL_SUCCESS;
    }
    BpfMapper<Key, DnsServerVal> wrMap(path, BPF_F_WRONLY);
    if (!wrMap.IsValid()) {
        NETNATIVE_LOGE("ClearUserServerKeys: wrMap invalid: %{public}s", path);
        return NETFIREWALL_ERR;
    }
    for (const Key &k : keys) {
        if (wrMap.Delete(k) == 0) {
            deleted++;
        }
    }
    return NETFIREWALL_SUCCESS;
}

/* Full reset sweep: every entry of one family goes (rule reset paths). */
template <typename Key> int32_t ResetServerMapKeys(const char *path, uint32_t &deleted)
{
    BpfMapper<Key, DnsServerVal> rdMap(path, BPF_F_RDONLY);
    if (!rdMap.IsValid()) {
        NETNATIVE_LOGE("ResetServerMapKeys: rdMap invalid: %{public}s", path);
        return NETFIREWALL_ERR;
    }
    std::vector<Key> keys = rdMap.GetAllKeys();
    if (keys.empty()) {
        return NETFIREWALL_SUCCESS;
    }
    BpfMapper<Key, DnsServerVal> wrMap(path, BPF_F_WRONLY);
    if (!wrMap.IsValid()) {
        NETNATIVE_LOGE("ResetServerMapKeys: wrMap invalid: %{public}s", path);
        return NETFIREWALL_ERR;
    }
    for (const Key &k : keys) {
        if (wrMap.Delete(k) == 0) {
            deleted++;
        }
    }
    return NETFIREWALL_SUCCESS;
}
} // namespace

NetsysBpfNetFirewall::NetsysBpfNetFirewall()
{
    NETNATIVE_LOG_D("NetsysBpfNetFirewall construct");
    isBpfLoaded_ = false;
}

std::shared_ptr<NetsysBpfNetFirewall> NetsysBpfNetFirewall::GetInstance()
{
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> guard(instanceMutex);
    if (instance_ == nullptr) {
        instance_.reset(new NetsysBpfNetFirewall());
        return instance_;
    }
    return instance_;
}

void NetsysBpfNetFirewall::ConntrackGcTask()
{
    NETNATIVE_LOG_D("ConntrackGcTask: running");
    std::vector<CtKey> keys = ctRdMap_->GetAllKeys();
    if (keys.empty()) {
        NETNATIVE_LOG_D("GcConntrackCb: key is empty");
        return;
    }

    timespec now = {0};
    // bpf_ktime_get_ns: CLOCK_MONOTONIC
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        NETNATIVE_LOGE("ConntrackGcTask: clock_gettime failed");
        return;
    }
    for (const CtKey &k : keys) {
        CtVaule v = {};
        if (ctRdMap_->Read(k, v) < 0) {
            NETNATIVE_LOGE("GcConntrackCb: read failed");
            continue;
        }

        if (v.lifetime < now.tv_sec) {
            if (ctWrMap_->Delete(k) != 0) {
                NETNATIVE_LOGE("GcConntrackCb: delete failed");
                continue;
            }
        }
    }
}

void NetsysBpfNetFirewall::RingBufferListenThread(void)
{
    if (keepListen_) {
        NETNATIVE_LOG_D("under listening");
        return;
    }

    int mapFd = NetsysBpfRingBuffer::GetRingbufFd(MAP_PATH(EVENT_MAP), 0);
    if (mapFd < 0) {
        NETNATIVE_LOGE("failed to get ring buffer fd: errno=%{public}d", errno);
        return;
    }
    ring_buffer *rb = ring_buffer__new(mapFd, NetsysBpfNetFirewall::HandleEvent, NULL, NULL);
    if (!rb) {
        NETNATIVE_LOGE("failed to create ring buffer: errno=%{public}d", errno);
        return;
    }

    keepListen_ = true;
    while (keepListen_) {
        if (ffrt::this_task::get_id() != 0) {
            ffrt::sync_io(mapFd);
        }
        int err = ring_buffer__poll(rb, RING_BUFFER_POLL_TIME_OUT_MS);
        if (err < 0) {
            NETNATIVE_LOGE("Error polling ring buffer: errno=%{public}d", errno);
            keepListen_ = false;
            break;
        }
    }

    NETNATIVE_LOGE("Could not get bpf event ring buffer map");
    ring_buffer__free(rb);
}

int32_t NetsysBpfNetFirewall::StartListener()
{
    if (!isBpfLoaded_) {
        NETNATIVE_LOG_D("bfp is not loaded");
        return -1;
    }
    ctRdMap_ = std::make_unique<BpfMapper<CtKey, CtVaule>>(MAP_PATH(CT_MAP), BPF_F_RDONLY);
    ctWrMap_ = std::make_unique<BpfMapper<CtKey, CtVaule>>(MAP_PATH(CT_MAP), BPF_F_WRONLY);

    ffrt::submit(RingBufferListenThread, {}, {}, ffrt::task_attr().name("RingBufferListen"));
    ffrt::submit(StartConntrackGcThread, {&ctRdMap_}, {&ctWrMap_});
    /* Learned-entry reclaim runs beside the conntrack GC with the
     * same 60s cadence; correctness never depends on it (BPF read-time gate). */
    ffrt::submit(StartDomainCacheAgingThread, {}, {}, ffrt::task_attr().name("DomainCacheAging"));
    return 0;
}

int32_t NetsysBpfNetFirewall::StopListener()
{
    keepListen_ = false;
    StopConntrackGc();
    keepAging_ = false;
    return 0;
}

void NetsysBpfNetFirewall::StartConntrackGcThread(void)
{
    if (keepGc_) {
        NETNATIVE_LOG_D("under keepGc");
        return;
    }
    if (!ctRdMap_->IsValid()) {
        NETNATIVE_LOGE("GcConntrackCb: ctRdMap is invalid");
        return;
    }

    if (!ctWrMap_->IsValid()) {
        NETNATIVE_LOGE("GcConntrackCb: ctWrMap is invalid");
        return;
    }

    keepGc_ = true;

    int rdMapFd = NetsysBpfRingBuffer::GetRingbufFd(MAP_PATH(CT_MAP), BPF_F_RDONLY);
    int wrMapFd = NetsysBpfRingBuffer::GetRingbufFd(MAP_PATH(CT_MAP), BPF_F_WRONLY);
    if (rdMapFd < 0 || wrMapFd < 0) {
        NETNATIVE_LOGE("failed to get rdMapFd or wrMapFd: errno=%{public}d", errno);
        return;
    }

    while (keepGc_) {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(CONNTRACK_GC_INTTERVAL_MS));
        if (ffrt::this_task::get_id() != 0) {
            ffrt::sync_io(rdMapFd);
            ffrt::sync_io(wrMapFd);
        }
        ConntrackGcTask();
    }
}

void NetsysBpfNetFirewall::StopConntrackGc()
{
    keepGc_ = false;
}

void NetsysBpfNetFirewall::SetBpfLoaded(bool load)
{
    isBpfLoaded_ = load;
    InvalidateAllSnapshots();
    if (isBpfLoaded_) {
        WriteLoopBackBpfMap();
        /* The reserved DEFAULT_ACTION_MAP entries: the unattributed
         * fallback (get_user_id -> UID_UNATTRIBUTED for 0xFFFFFFFF) and the
         * now-vestigial system key (F6: real system-identity sockets fold
         * to the foreground user instead). Best effort at this point (a
         * failure is reported via NET_FIREWALL_INIT_FAULT and does not abort
         * the load — the keys stay writable through SetFirewallDefaultAction). */
        if (SetBuiltInFirewallDefaultActions() != NETFIREWALL_SUCCESS) {
            NETNATIVE_LOGE("SetBpfLoaded: built-in default actions failed (see NET_FIREWALL_INIT_FAULT)");
        }
    }
}

void NetsysBpfNetFirewall::InvalidateIpSnapshots(NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    std::lock_guard<std::mutex> snapshotGuard(snapshotMutex_);
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, saddr));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, saddr6));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, daddr));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, daddr6));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, sport));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, dport));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, proto));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, appuid));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, uid));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, action));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, iface));
    appliedSnapshots_.erase(GET_MAP_PATH(ingress, wildcard));
}

void NetsysBpfNetFirewall::InvalidateAllSnapshots()
{
    std::lock_guard<std::mutex> snapshotGuard(snapshotMutex_);
    appliedSnapshots_.clear();
}

int32_t NetsysBpfNetFirewall::WriteLoopBackBpfMap()
{
    Ipv4LpmKey ip4Key = {};
    LoopbackValue loopbackVal = 1;
    ip4Key.prefixlen = LOOP_BACK_IPV4_PREFIXLEN;
    inet_pton(AF_INET, LOOP_BACK_IPV4, &ip4Key.data);
    if (WriteBpfMap(MAP_PATH(LOOP_BACK_IPV4_MAP), ip4Key, loopbackVal) != 0) {
        NETNATIVE_LOGE("WriteLoopBackBpfMap: ipv4 failed");
        return NETFIREWALL_ERR;
    }
    Ipv6LpmKey ip6Key = {};
    ip6Key.prefixlen = LOOP_BACK_IPV6_PREFIXLEN;
    inet_pton(AF_INET6, LOOP_BACK_IPV6, &ip6Key.data);
    if (WriteBpfMap(MAP_PATH(LOOP_BACK_IPV6_MAP), ip6Key, loopbackVal) != 0) {
        NETNATIVE_LOGE("WriteLoopBackBpfMap: ipv6 failed");
        return NETFIREWALL_ERR;
    }
    return NETFIREWALL_SUCCESS;
}

void NetsysBpfNetFirewall::ClearBpfFirewallRules(NetFirewallRuleDirection direction)
{
    Ipv4LpmKey ip4Key = {};
    Ipv6LpmKey ip6Key = {};
    PortKey portKey = {};
    ProtoKey protoKey = 0;
    interface_key ifaceKey = {};
    AppUidKey appIdKey = 0;
    UidKey uidKey = 0;
    ActionKey actKey = 1;
    ActionValue actVal;
    RuleCode ruleCode;
    WildcardKey wildcardKey = 0;
    CtKey ctKey;
    CtVaule ctVal;

    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    int res = 0;
    res += ClearBpfMap(GET_MAP_PATH(ingress, saddr), ip4Key, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, saddr6), ip6Key, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, daddr), ip4Key, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, daddr6), ip6Key, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, sport), portKey, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, dport), portKey, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, proto), protoKey, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, appuid), appIdKey, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, uid), uidKey, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, action), actKey, actVal);
    res += ClearBpfMap(GET_MAP_PATH(ingress, iface), ifaceKey, ruleCode);
    res += ClearBpfMap(GET_MAP_PATH(ingress, wildcard), wildcardKey, ruleCode);
    res += ClearBpfMap(MAP_PATH(CT_MAP), ctKey, ctVal);
    if (res) {
        NETNATIVE_LOGE("ClearBpfFirewallRules: dir=%{public}d, res=%{public}d", direction, res);
    }
}

int32_t NetsysBpfNetFirewall::ClearFirewallRules(NetFirewallRuleType type)
{
    {
        std::lock_guard<std::mutex> guard(rulesMutex_);
        switch (type) {
            case NetFirewallRuleType::RULE_IP: {
                firewallIpRules_.clear();
                ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_IN);
                ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_OUT);
                InvalidateIpSnapshots(NetFirewallRuleDirection::RULE_IN);
                InvalidateIpSnapshots(NetFirewallRuleDirection::RULE_OUT);
                break;
            }
            case NetFirewallRuleType::RULE_DOMAIN: {
                firewallDomainRules_.clear();
                ClearDomainRules();
                /* Domain rule removal also invalidates verdicts. */
                if (ClearConntrackMap() != NETFIREWALL_SUCCESS) {
                    NETNATIVE_LOGE("ClearFirewallRules: clear ct map failed");
                }
                /* With every domain rule gone no user meets the write
                 * condition (绝对隔离) — the authorization maps must end up empty. */
                {
                    std::lock_guard<std::mutex> guard(dnsServerMutex_);
                    dnsUserHasAllowDomain_.clear();
                    dnsPrewarmDomains_.clear();
                }
                ffrt::submit([]() { RunDnsServerSyncTask(DNS_SERVER_SYNC_RESET); }, {}, {},
                             ffrt::task_attr().name("DnsSrvSync"));
                break;
            }
            case NetFirewallRuleType::RULE_DEFAULT_ACTION: {
                ClearFirewallDefaultAction();
                break;
            }
            case NetFirewallRuleType::RULE_ALL: {
                firewallIpRules_.clear();
                ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_IN);
                ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_OUT);
                InvalidateAllSnapshots();
                firewallDomainRules_.clear();
                ClearDomainRules();
                ClearFirewallDefaultAction();
                /* Full rule reset also invalidates verdicts. */
                if (ClearConntrackMap() != NETFIREWALL_SUCCESS) {
                    NETNATIVE_LOGE("ClearFirewallRules: clear ct map failed");
                }
                /* Same reset as RULE_DOMAIN above; ClearFirewallDefaultAction's own
                 * hook clears the default-deny registry, a second reset sweep is a
                 * harmless no-op. */
                {
                    std::lock_guard<std::mutex> guard(dnsServerMutex_);
                    dnsUserHasAllowDomain_.clear();
                    dnsPrewarmDomains_.clear();
                }
                ffrt::submit([]() { RunDnsServerSyncTask(DNS_SERVER_SYNC_RESET); }, {}, {},
                             ffrt::task_attr().name("DnsSrvSync"));
                break;
            }
            default:
                break;
        }
    }
    /* Verdict flip (rules gone): cached allow-era answers must not linger.
     * Called OUTSIDE rulesMutex_ — see FlushDnsCacheAfterRuleChange. */
    FlushDnsCacheAfterRuleChange();
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetBpfFirewallRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList,
                                                  NetFirewallRuleDirection direction)
{
    BitmapManager manager;
    int32_t ret = manager.BuildBitmapMap(ruleList);
    if (ret) {
        NETNATIVE_LOGE("SetBpfFirewallRules: BuildBitmapMap failed: %{public}d", ret);
        return ret;
    }
    int res = 0;
    res += WriteSrcIpv4BpfMap(manager, direction);
    res += WriteSrcIpv6BpfMap(manager, direction);
    res += WriteDstIpv4BpfMap(manager, direction);
    res += WriteDstIpv6BpfMap(manager, direction);
    res += WriteSrcPortBpfMap(manager, direction);
    res += WriteDstPortBpfMap(manager, direction);
    res += WriteProtoBpfMap(manager, direction);
    res += WriteAppUidBpfMap(manager, direction);
    res += WriteUidBpfMap(manager, direction);
    res += WriteActionBpfMap(manager, direction);
    res += WriteInterfaceBpfMap(manager, direction);
    res += WriteWildcardBpfMap(manager, direction);
    if (res) {
        NETNATIVE_LOGE("SetBpfFirewallRules: dir=%{public}d, res=%{public}d", direction, res);
        return NETFIREWALL_ERR;
    }
    /* Same log-only downgrade as SetFirewallDomainRules:
     * the rule maps are already written, a failed ct clear only delays
     * re-evaluation until ct expiry — not worth a new error code to ext. */
    if (ClearConntrackMap() != NETFIREWALL_SUCCESS) {
        NETNATIVE_LOGE("SetBpfFirewallRules: clear ct map failed (log-only)");
    }
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetFirewallRules(NetFirewallRuleType type,
                                               const std::vector<sptr<NetFirewallBaseRule>> &ruleList, bool isFinish)
{
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("SetFirewallRules: bpf not loaded");
        return NETFIREWALL_ERR;
    }
    if (ruleList.empty()) {
        NETNATIVE_LOGE("SetFirewallRules: rules is empty");
        return NETFIREWALL_ERR;
    }
    int32_t ret = NETFIREWALL_SUCCESS;
    bool dnsCacheFlushNeeded = false;
    {
        std::lock_guard<std::mutex> guard(rulesMutex_);
        switch (type) {
            case NetFirewallRuleType::RULE_IP: {
                for (const auto &rule : ruleList) {
                    firewallIpRules_.emplace_back(firewall_rule_cast<NetFirewallIpRule>(rule));
                }
                if (isFinish) {
                    ret = SetFirewallIpRules(firewallIpRules_);
                    firewallIpRules_.clear();
                }
                break;
            }
            case NetFirewallRuleType::RULE_DOMAIN: {
                for (const auto &rule : ruleList) {
                    firewallDomainRules_.emplace_back(firewall_rule_cast<NetFirewallDomainRule>(rule));
                }
                if (isFinish) {
                    ret = SetFirewallDomainRules(firewallDomainRules_);
                    firewallDomainRules_.clear();
                }
                break;
            }
            default:
                break;
        }
        dnsCacheFlushNeeded = isFinish && (ret == NETFIREWALL_SUCCESS);
    }
    NETNATIVE_LOGI("SetFirewallRules: ret=%{public}d size=%{public}zu isFinish=%{public}" PRId32, ret, ruleList.size(),
                   isFinish);
    /* Rule application flips verdicts; cached allow-era DNS answers would
     * serve without re-adjudication. Flush OUTSIDE rulesMutex_ — see
     * FlushDnsCacheAfterRuleChange for the cross-layer lock-order note. */
    if (dnsCacheFlushNeeded) {
        FlushDnsCacheAfterRuleChange();
    }
    return ret;
}

int32_t NetsysBpfNetFirewall::SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList)
{
    if (ruleList.empty()) {
        NETNATIVE_LOGE("SetFirewallDomainRules: rules is empty");
        return NETFIREWALL_ERR;
    }
    /* clearAcTables=false: the AC rebuild below is a full replace with an
     * atomic generation flip — flipping to an empty table first would only
     * open a no-verdict window between the two flips. */
    ClearDomainRules(false);
    /* Compile the rule set into the AC trie tables — the only BPF-side
     * consumer of domain rules. Aggregated build + one generation flip; on
     * any failure the flip never happens — the previous table stays online
     * and the error propagates, so a deny rule can never silently look
     * applied while being unenforced. */
    int ret = BuildAcDomainTables(ruleList);
    if (ret != NETFIREWALL_SUCCESS) {
        NETNATIVE_LOGE("SetFirewallDomainRules: AC table build failed, previous AC generation stays online");
        ret = NETFIREWALL_ERR;
    }
    /* Domain rule changes must invalidate conntrack entries —
     * the per-direction evaluated bits would otherwise keep stale verdicts for up to 6
     * hours. Clear failure is log-only, aligned with SetFirewallDefaultAction
     * (same rationale): the rule maps are already authoritative, a failed
     * clear only delays re-evaluation until ct expiry, and a new error code
     * here would leak to ext callers for a non-fatal condition. */
    if (ClearConntrackMap() != NETFIREWALL_SUCCESS) {
        NETNATIVE_LOGE("SetFirewallDomainRules: clear ct map failed (log-only)");
    }
    /* Domain rules are full-replace semantics (ClearDomainRules
     * above wipes the maps), so the sync registries follow the same replace.
     * Only registry update + async scheduling run under rulesMutex_: the map
     * I/O and the dns-layer provider (cacheMutex_) happen on the async task —
     * taking cacheMutex_ under rulesMutex_ would reverse the established
     * cacheMutex_ -> rulesMutex_ ordering (deadlock). */
    {
        std::map<int32_t, bool> hasAllow;
        std::map<int32_t, std::set<std::string>> exactDomains;
        for (const auto &rule : ruleList) {
            if (rule == nullptr || rule->ruleAction != FirewallRuleAction::RULE_ALLOW) {
                continue;
            }
            hasAllow[rule->userId] = true;
            for (const auto &param : rule->domains) {
                /* Wildcard patterns cannot be resolved as literal names —
                 * any '*' (leading subdomain anchor "*.x.com", trailing
                 * prefix "www.x*", or misplaced) is unresolvable garbage
                 * upstream ("no such name" loops, measured on-device). The
                 * pre-warm engine only takes exact domains. */
                if (!param.domain.empty() && param.domain.find('*') == std::string::npos) {
                    exactDomains[rule->userId].insert(param.domain);
                }
            }
        }
        std::vector<int32_t> resyncUsers;
        {
            std::lock_guard<std::mutex> guard(dnsServerMutex_);
            for (const auto &entry : dnsUserHasAllowDomain_) {
                resyncUsers.push_back(entry.first); /* users that may lose the condition */
            }
            for (const auto &entry : hasAllow) {
                resyncUsers.push_back(entry.first); /* users that may gain it */
            }
            std::sort(resyncUsers.begin(), resyncUsers.end());
            resyncUsers.erase(std::unique(resyncUsers.begin(), resyncUsers.end()), resyncUsers.end());
            dnsUserHasAllowDomain_ = std::move(hasAllow);
            dnsPrewarmDomains_ = std::move(exactDomains);
        }
        for (int32_t userId : resyncUsers) {
            ffrt::submit([userId]() { RunDnsServerSyncTask(userId); }, {}, {}, ffrt::task_attr().name("DnsSrvSync"));
        }
    }
    TriggerDnsPrewarm();
    return ret;
}

bool NetsysBpfNetFirewall::GetDomainHashKey(const std::string &domain, DomainHashKey &out, bool isReversed)
{
    if (domain.empty()) {
        NETNATIVE_LOGE("GetDomainHashKey: domain is empty");
        return false;
    }
    std::string text(domain);
    text.erase(std::remove(text.begin(), text.end(), '*'), text.end());
    if (text.empty()) {
        NETNATIVE_LOGE("GetDomainHashKey: domain is empty after strip");
        return false;
    }
    int strLen = static_cast<int>(text.length());
    if (strLen >= DNS_DOMAIN_LEN) {
        NETNATIVE_LOGE("GetDomainHashKey: domain length exceeds DNS_DOMAIN_LEN");
        return false;
    }
    if (isReversed) {
        std::reverse(text.begin(), text.end());
        if (memcpy_s(out.data, DNS_DOMAIN_LEN, (uint8_t *)text.c_str(), strLen) != EOK) {
            NETNATIVE_LOGE("GetDomainHashKey: isReversed memcpy_s failed");
            return false;
        }
        out.prefixlen = static_cast<uint32_t>((sizeof(out.uid) + sizeof(out.appuid) + strLen) * BIT_PER_BYTE);
    } else {
        out.data[0] = '.';
        if (memcpy_s(out.data + 1, DNS_DOMAIN_LEN - 1, (uint8_t *)text.c_str(), strLen) != EOK) {
            NETNATIVE_LOGE("GetDomainHashKey: noReversed memcpy_s failed");
            return false;
        }
        out.prefixlen = static_cast<uint32_t>((sizeof(out.uid) + sizeof(out.appuid) + strLen + 1) * BIT_PER_BYTE);
    }
    return true;
}

void NetsysBpfNetFirewall::ClearDomainRules(bool clearAcTables)
{
    NETNATIVE_LOG_D("ClearDomainRules");
    ClearDomainCache();
    /* The record table follows the domain rules — a rule change voids
     * the old allow verdicts, so pending query credentials are void too. */
    ClearDnsQueryRecordMap();
    /* A' trie tables follow the same full-replace semantics. Skipped when
     * the caller rebuilds them right after (SetFirewallDomainRules): the
     * rebuild's generation flip is atomic, an intermediate empty flip would
     * only open a no-verdict window. */
    if (clearAcTables) {
        ClearAcDomainTables();
    }
}

void NetsysBpfNetFirewall::ClearDomainRules()
{
    /* Legacy ABI-compat entry point — see the header declaration. */
    ClearDomainRules(true);
}

/* ===== A' AC trie tables: aggregated build + generation switch =====
 * Protocol (see AcDomainTableBuilder for the wire encoding):
 *   1. compile every rule in memory (invalid rules are skipped + logged,
 *      mirroring the legacy GetDomainHashKey skip);
 *   2. read the active generation, target the inactive one;
 *   3. pre-sweep the inactive generation (heals any residue a failed or
 *      crashed build left behind — a partial table must never go live);
 *   4. write the full table; a failed write rolls the partial rows back and
 *      keeps the old generation online;
 *   5. flip AC_GEN_MAP — the single atomic step; every scan reads the
 *      generation once and walks one consistent table view per name;
 *   6. sweep the retired generation (log-only: leftovers are unreachable
 *      through the active generation, they only occupy capacity).
 * Capacity overflow (AC_TRANS_MAX_ENTRIES) fails fast before any write. */
int32_t NetsysBpfNetFirewall::BuildAcDomainTables(const std::vector<sptr<NetFirewallDomainRule>> &ruleList)
{
    AcDomainTableBuilder builder;
    for (const auto &rule : ruleList) {
        if (rule == nullptr) {
            continue;
        }
        uint32_t uid = static_cast<uint32_t>(rule->userId);
        uint32_t appuid = static_cast<uint32_t>(rule->appUid);
        bool isAllow = rule->ruleAction == FirewallRuleAction::RULE_ALLOW;
        for (const auto &param : rule->domains) {
            if (param.domain.empty()) {
                continue;
            }
            if (!builder.AddRule(uid, appuid, isAllow, param.domain)) {
                /* Fail-visible: name the violated star form when that is
                 * the reason — the legacy behavior stripped '*' silently
                 * and armed rules with different semantics than the admin
                 * wrote (measured on-device: "*w.baidu.com" never fired). */
                const char *starReason = nullptr;
                AcDomainTableBuilder::CheckStarForm(param.domain, &starReason);
                NETNATIVE_LOGE(
                    "BuildAcDomainTables: skip invalid domain: uid=%{public}u appuid=%{public}u "
                    "domain=%{public}s reason=%{public}s",
                    uid, appuid, param.domain.c_str(),
                    starReason != nullptr ? starReason : "unrepresentable (empty label / length cap / scan window)");
            }
        }
    }
    if (builder.Empty()) {
        /* Nothing representable survived validation: full-replace semantics
         * — the online table must end up empty. */
        ClearAcDomainTables();
        return NETFIREWALL_SUCCESS;
    }
    uint32_t zero = 0;
    BpfMapper<__u32, __u32> rdGen(MAP_PATH(AC_GEN_MAP), BPF_F_RDONLY);
    if (!rdGen.IsValid()) {
        /* No AC maps at all (map-less environment / ELF without the A'
         * tables): the BPF side no longer reads the legacy LPM maps, so
         * returning success here would leave deny rules silently
         * unenforced — fail loudly instead. */
        NETNATIVE_LOGE("BuildAcDomainTables: ac_gen_map unavailable, AC tables not maintained");
        return NETFIREWALL_ERR;
    }
    uint32_t curGen = 0;
    uint32_t inactive = 1;
    if (rdGen.Read(zero, curGen) == 0) {
        inactive = curGen ^ 1U;
    }
    /* else: no generation entry yet (fresh maps): build generation 1; the
     * pre-sweep below clears anything left in it. */
    std::vector<AcTransEntry> prefixEntries;
    std::vector<AcTransEntry> suffixEntries;
    if (!builder.Emit(inactive, prefixEntries, suffixEntries)) {
        NETNATIVE_LOGE(
            "BuildAcDomainTables: entries exceed AC_TRANS_MAX_ENTRIES (%{public}zu prefix, "
            "%{public}zu suffix), build aborted",
            prefixEntries.size(), suffixEntries.size());
        return NETFIREWALL_ERR;
    }
    BpfMapper<AcTransKey, AcTransVal> prefixWr(MAP_PATH(AC_PREFIX_TRANS_MAP), BPF_F_WRONLY);
    BpfMapper<AcTransKey, AcTransVal> suffixWr(MAP_PATH(AC_SUFFIX_TRANS_MAP), BPF_F_WRONLY);
    if (!prefixWr.IsValid() || !suffixWr.IsValid()) {
        NETNATIVE_LOGE("BuildAcDomainTables: trans map unavailable, previous generation stays online");
        return NETFIREWALL_ERR;
    }
    /* Pre-sweep the inactive generation: never write on top of unknown rows. */
    if (SweepAcGenEntries(MAP_PATH(AC_PREFIX_TRANS_MAP), inactive, true) != NETFIREWALL_SUCCESS ||
        SweepAcGenEntries(MAP_PATH(AC_SUFFIX_TRANS_MAP), inactive, true) != NETFIREWALL_SUCCESS) {
        NETNATIVE_LOGE("BuildAcDomainTables: inactive generation pre-sweep failed, build aborted");
        return NETFIREWALL_ERR;
    }
    std::vector<AcTransKey> writtenPrefix;
    std::vector<AcTransKey> writtenSuffix;
    int32_t ret = NETFIREWALL_SUCCESS;
    for (const auto &e : prefixEntries) {
        if (prefixWr.Write(e.key, e.val, BPF_ANY) != 0) {
            NETNATIVE_LOGE(
                "BuildAcDomainTables: prefix transition write failed errno=%{public}d "
                "key={uid=%{public}u,appuid=%{public}u,state=%{public}u,cls=0x%{public}x,gen=%{public}u}",
                errno, e.key.uid, e.key.appuid, e.key.state, e.key.cls, e.key.gen);
            ret = NETFIREWALL_ERR;
            break;
        }
        writtenPrefix.push_back(e.key);
    }
    if (ret == NETFIREWALL_SUCCESS) {
        for (const auto &e : suffixEntries) {
            if (suffixWr.Write(e.key, e.val, BPF_ANY) != 0) {
                NETNATIVE_LOGE(
                    "BuildAcDomainTables: suffix transition write failed errno=%{public}d "
                    "key={uid=%{public}u,appuid=%{public}u,state=%{public}u,cls=0x%{public}x,gen=%{public}u}",
                    errno, e.key.uid, e.key.appuid, e.key.state, e.key.cls, e.key.gen);
                ret = NETFIREWALL_ERR;
                break;
            }
            writtenSuffix.push_back(e.key);
        }
    }
    if (ret != NETFIREWALL_SUCCESS) {
        NETNATIVE_LOGE("BuildAcDomainTables: transition write failed, rolling back the partial generation");
        for (const auto &k : writtenPrefix) {
            if (prefixWr.Delete(k) != 0) {
                NETNATIVE_LOGE("BuildAcDomainTables: rollback delete failed (healed by the next build's pre-sweep)");
            }
        }
        for (const auto &k : writtenSuffix) {
            if (suffixWr.Delete(k) != 0) {
                NETNATIVE_LOGE("BuildAcDomainTables: rollback delete failed (healed by the next build's pre-sweep)");
            }
        }
        return NETFIREWALL_ERR;
    }
    /* The flip: one array write. In-flight scans already read the old value
     * and keep walking the old table for the rest of their name. */
    uint32_t newGen = inactive;
    if (WriteBpfMap(MAP_PATH(AC_GEN_MAP), zero, newGen) != 0) {
        NETNATIVE_LOGE("BuildAcDomainTables: generation flip failed, previous generation stays online");
        return NETFIREWALL_ERR;
    }
    /* Retired generation sweep — log-only. */
    if (SweepAcGenEntries(MAP_PATH(AC_PREFIX_TRANS_MAP), newGen, false) != NETFIREWALL_SUCCESS ||
        SweepAcGenEntries(MAP_PATH(AC_SUFFIX_TRANS_MAP), newGen, false) != NETFIREWALL_SUCCESS) {
        NETNATIVE_LOGE("BuildAcDomainTables: retired generation sweep incomplete (log-only)");
    }
    NETNATIVE_LOG_D("BuildAcDomainTables: gen=%{public}u prefix=%{public}zu suffix=%{public}zu", newGen,
                    prefixEntries.size(), suffixEntries.size());
    return NETFIREWALL_SUCCESS;
}

void NetsysBpfNetFirewall::ClearAcDomainTables()
{
    NETNATIVE_LOG_D("ClearAcDomainTables");
    AcTransKey key = {0};
    AcTransVal val = {0};
    /* Both generations go: whichever row set the active generation addresses
     * disappears and every scan misses (no verdict — the packet keeps
     * flowing through the policy chain, the same outcome as an empty rule
     * table). No generation flip is needed — a present entry keeps
     * addressing a now-empty table, an absent entry keeps the scans on the
     * fail-closed path; both end in "no rules". Map-less environments are
     * silent by design (ClearDnsQueryRecordMap pattern). */
    ClearBpfMap(MAP_PATH(AC_PREFIX_TRANS_MAP), key, val);
    ClearBpfMap(MAP_PATH(AC_SUFFIX_TRANS_MAP), key, val);
}

int32_t NetsysBpfNetFirewall::SweepAcGenEntries(const char *path, uint32_t gen, bool match)
{
    BpfMapper<AcTransKey, AcTransVal> rdMap(path, BPF_F_RDONLY);
    if (!rdMap.IsValid()) {
        NETNATIVE_LOGE("SweepAcGenEntries: rdMap invalid: %{public}s", path);
        return NETFIREWALL_ERR;
    }
    std::vector<AcTransKey> keys = rdMap.GetAllKeys();
    if (keys.empty()) {
        return NETFIREWALL_SUCCESS;
    }
    BpfMapper<AcTransKey, AcTransVal> wrMap(path, BPF_F_WRONLY);
    if (!wrMap.IsValid()) {
        NETNATIVE_LOGE("SweepAcGenEntries: wrMap invalid: %{public}s", path);
        return NETFIREWALL_ERR;
    }
    int32_t ret = NETFIREWALL_SUCCESS;
    for (const auto &k : keys) {
        if ((k.gen == gen) != match) {
            continue;
        }
        if (wrMap.Delete(k) != 0) {
            NETNATIVE_LOGE("SweepAcGenEntries: delete failed, stale row kept (healed by the next pre-sweep)");
            ret = NETFIREWALL_ERR;
        }
    }
    return ret;
}

void NetsysBpfNetFirewall::ClearDnsQueryRecordMap()
{
    NETNATIVE_LOG_D("ClearDnsQueryRecordMap");
    DnsQueryRecordKey key = {0};
    DnsQueryRecordValue value = {0};
    /* With the validation feature compiled out the pinned map does not exist;
     * the open failure is silent by design (ClearBpfMap only logs write-side
     * failures) — no log spam on flag-off builds. */
    ClearBpfMap(MAP_PATH(DNS_QUERY_RECORD_MAP), key, value);
}

int32_t NetsysBpfNetFirewall::SetFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList)
{
    std::vector<sptr<NetFirewallIpRule>> inRules;
    std::vector<sptr<NetFirewallIpRule>> outRules;

    for (const auto &rule : ruleList) {
        if (rule->ruleDirection == NetFirewallRuleDirection::RULE_IN) {
            if (rule->protocol == NetworkProtocol::ICMP || rule->protocol == NetworkProtocol::ICMPV6) {
                outRules.emplace_back(rule);
            } else {
                inRules.emplace_back(rule);
            }
        }
        if (rule->ruleDirection == NetFirewallRuleDirection::RULE_OUT) {
            outRules.emplace_back(rule);
        }
    }

    int32_t ret = NETFIREWALL_SUCCESS;
    if (!inRules.empty()) {
        ret = SetBpfFirewallRules(inRules, NetFirewallRuleDirection::RULE_IN);
    }
    if (!outRules.empty()) {
        ret += SetBpfFirewallRules(outRules, NetFirewallRuleDirection::RULE_OUT);
    }
    return ret;
}

int32_t NetsysBpfNetFirewall::ClearFirewallDefaultAction()
{
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("ClearFirewallDefaultAction: bpf not loaded");
        return NETFIREWALL_ERR;
    }
    /* The clear is an explicit full sweep of user-configured
     * entries — the old code passed a fake -1 (0xFFFFFFFF) key to the ignore-key
     * ClearBpfMap helper, colliding with the reserved unattributed key. The
     * reserved entries (FIREWALL_SYSTEM_USER_ID / FIREWALL_UNATTRIBUTED_USER_ID)
     * are managed through SetFirewallDefaultAction / the built-in writer and
     * must survive this clear. */
    BpfMapper<uid_key, defalut_action_value> rdMap(MAP_PATH(DEFAULT_ACTION_MAP), BPF_F_RDONLY);
    if (!rdMap.IsValid()) {
        NETNATIVE_LOGE("ClearFirewallDefaultAction: rdMap is invalid");
        return NETFIREWALL_ERR;
    }
    std::vector<uid_key> keys = rdMap.GetAllKeys();
    /* GetAllKeys -> Clear is not atomic vs a concurrent SetFirewallDefaultAction
     * (this class has no rules-level lock): a default-action write racing inside
     * this window can be swept away while its own call still reports success.
     * Accepted for now: the firewall manager (ext) is the sole writer and pushes
     * rule/default-action batches sequentially over one IPC session. */
    std::vector<uid_key> removableKeys;
    for (const uid_key &key : keys) {
        if (key == static_cast<uid_key>(FIREWALL_UNATTRIBUTED_USER_ID) ||
            key == static_cast<uid_key>(FIREWALL_SYSTEM_USER_ID)) {
            continue;
        }
        removableKeys.push_back(key);
    }
    /* With no user-configured default actions left, no user meets
     * the write condition — the authorization maps must end up empty. The
     * early return below would skip a late reset, so it runs here once for
     * both exits (an empty sweep is a no-op). */
    {
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        dnsUserDefaultDeny_.clear();
    }
    ffrt::submit([]() { RunDnsServerSyncTask(DNS_SERVER_SYNC_RESET); }, {}, {}, ffrt::task_attr().name("DnsSrvSync"));
    if (removableKeys.empty()) {
        return NETFIREWALL_SUCCESS;
    }
    BpfMapper<uid_key, defalut_action_value> wrMap(MAP_PATH(DEFAULT_ACTION_MAP), BPF_F_WRONLY);
    if (!wrMap.IsValid()) {
        NETNATIVE_LOGE("ClearFirewallDefaultAction: wrMap is invalid");
        return NETFIREWALL_ERR;
    }
    if (wrMap.Clear(removableKeys) != 0) {
        NETNATIVE_LOGE("ClearFirewallDefaultAction: clear failed");
        return NETFIREWALL_ERR;
    }
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::ClearConntrackMap()
{
    CtKey ctKey;
    CtVaule ctVal;
    if (ClearBpfMap(MAP_PATH(CT_MAP), ctKey, ctVal) != 0) {
        NETNATIVE_LOGE("ClearConntrackMap: clear ct map failed");
        return NETFIREWALL_ERR;
    }
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetFirewallDefaultAction(int32_t userId, FirewallRuleAction inDefault,
                                                       FirewallRuleAction outDefault)
{
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("SetFirewallDefaultAction: bpf not loaded");
        return NETFIREWALL_ERR;
    }
    defalut_action_value val = {SK_PASS};
    val.inaction = (inDefault == FirewallRuleAction::RULE_ALLOW) ? SK_PASS : SK_DROP;
    val.outaction = (outDefault == FirewallRuleAction::RULE_ALLOW) ? SK_PASS : SK_DROP;
    /* Reserved keys (FIREWALL_SYSTEM_USER_ID / FIREWALL_UNATTRIBUTED_USER_ID) are
     * normal write keys here: the stub layer applies no userId range
     * check and WriteBpfMap writes the raw key, so ext configures the reserved
     * fallback actions through this same command. The ct clear below also kills
     * the per-direction evaluated bits so existing flows re-evaluate under the new defaults. */
    if (WriteBpfMap(MAP_PATH(DEFAULT_ACTION_MAP), (uid_key)userId, val) != 0) {
        NETNATIVE_LOGE("SetFirewallDefaultAction: write failed");
        return NETFIREWALL_ERR;
    }
    /* sync timing: a default-action flip re-evaluates the DNS server
     * write condition for this user (deny->allow clears the entries,
     * allow->deny arms them) and re-runs the pre-warm engine. */
    NotifyDnsDefaultActionChanged(userId, outDefault == FirewallRuleAction::RULE_DENY);
    if (ClearConntrackMap() != NETFIREWALL_SUCCESS) {
        /* Not fatal — the write succeeded and the pre-existing IPC contract
         * returns success here — but the per-direction verdicts of existing connections
         * now survive until ct gc; this must be observable in the log. */
        NETNATIVE_LOGE("SetFirewallDefaultAction: clear ct map failed, stale verdicts may persist");
    }
    /* Default-action flip changes the verdict baseline; flush cached
     * allow-era DNS answers so deny baselines are not bypassed by cache
     * hits (no rulesMutex_ is held on this path). */
    FlushDnsCacheAfterRuleChange();
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetBuiltInFirewallDefaultActions()
{
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("SetBuiltInFirewallDefaultActions: bpf not loaded");
        return NETFIREWALL_ERR;
    }
    /* Both reserved entries in one pass — after F6 no live traffic folds
     * into FIREWALL_SYSTEM_USER_ID (real system-identity sockets go
     * through CURRENT_UID_MAP foreground folding); the entry stays as a
     * writable reserved/governance key. The unattributed fallback
     * (0xFFFFFFFF: socketless kernel packets, ingress attribution
     * failures) lands on DEFAULT_ACTION_MAP[UNATTRIBUTED], which must
     * exist and read allow. The keys stay writable through
     * SetFirewallDefaultAction afterwards. */
    defalut_action_value val = {SK_PASS};
    val.inaction = SK_PASS;
    val.outaction = SK_PASS;

    if (WriteBpfMap(MAP_PATH(DEFAULT_ACTION_MAP), (uid_key)FIREWALL_SYSTEM_USER_ID, val) != 0) {
        NETNATIVE_LOGE("SetBuiltInFirewallDefaultActions: SYSTEM write failed, errno=%{public}d", errno);
        return NETFIREWALL_ERR;
    }
    if (WriteBpfMap(MAP_PATH(DEFAULT_ACTION_MAP), (uid_key)FIREWALL_UNATTRIBUTED_USER_ID, val) != 0) {
        NETNATIVE_LOGE("SetBuiltInFirewallDefaultActions: UNATTRIBUTED write failed, errno=%{public}d", errno);
        return NETFIREWALL_ERR;
    }
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetEvalFallbackMode(uint32_t mode)
{
    if (mode != EVAL_FALLBACK_OBSERVE && mode != EVAL_FALLBACK_FAIL_CLOSED) {
        NETNATIVE_LOGE("SetEvalFallbackMode: invalid mode %{public}u", mode);
        return NETFIREWALL_ERR;
    }
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("SetEvalFallbackMode: bpf not loaded");
        return NETFIREWALL_ERR;
    }
    /* Both modes are expressed as a single-entry write: eval_error treats any value
     * other than OBSERVE as fail-closed, so writing FAIL_CLOSED(1) is semantically
     * equal to deleting the entry (secure default), no single-entry delete needed. */
    uint32_t key = 0;
    uint32_t val = mode;
    if (WriteBpfMap(MAP_PATH(EVAL_FALLBACK_MODE_MAP), key, val) != 0) {
        NETNATIVE_LOGE("SetEvalFallbackMode: write failed, mode %{public}u", mode);
        return NETFIREWALL_ERR;
    }
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetFirewallCurrentUserId(int32_t userId)
{
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("SetFirewallCurrentUserId: bpf not loaded");
        return NETFIREWALL_ERR;
    }

    CurrentUserIdKey key = CURRENT_USER_ID_KEY;
    UidKey val = (UidKey)userId;
    if (WriteBpfMap(MAP_PATH(CURRENT_UID_MAP), key, val) != 0) {
        NETNATIVE_LOGE("SetFirewallCurrentUserId: write bpf map failed");
        return NETFIREWALL_ERR;
    }
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::WriteSrcIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeLpmMap<Ipv4LpmKey>(manager.GetSrcIp4Map(), &Ip4RuleBitmap::mask, GET_MAP_PATH(ingress, saddr),
                                           "src_ip4_map");
}

int32_t NetsysBpfNetFirewall::WriteSrcIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeLpmMap<Ipv6LpmKey>(manager.GetSrcIp6Map(), &Ip6RuleBitmap::prefixlen,
                                           GET_MAP_PATH(ingress, saddr6), "src_ip6_map");
}

int32_t NetsysBpfNetFirewall::WriteDstIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeLpmMap<Ipv4LpmKey>(manager.GetDstIp4Map(), &Ip4RuleBitmap::mask, GET_MAP_PATH(ingress, daddr),
                                           "dst_ip4_map");
}

int32_t NetsysBpfNetFirewall::WriteDstIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeLpmMap<Ipv6LpmKey>(manager.GetDstIp6Map(), &Ip6RuleBitmap::prefixlen,
                                           GET_MAP_PATH(ingress, daddr6), "dst_ip6_map");
}

int32_t NetsysBpfNetFirewall::WritePortBpfMap(BpfPortMap &portMap, const char *path)
{
    return WriteRuleCodeLpmMap<PortKey>(portMap.Get(), &portRuleBitmap::prefixlen, path, "port_map");
}

int32_t NetsysBpfNetFirewall::WriteSrcPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WritePortBpfMap(manager.GetSrcPortMap(), GET_MAP_PATH(ingress, sport));
}

int32_t NetsysBpfNetFirewall::WriteDstPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WritePortBpfMap(manager.GetDstPortMap(), GET_MAP_PATH(ingress, dport));
}

int32_t NetsysBpfNetFirewall::WriteProtoBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeMap<ProtoKey>(manager.GetProtoMap(), GET_MAP_PATH(ingress, proto), "proto_map");
}

int32_t NetsysBpfNetFirewall::WriteAppUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeMap<AppUidKey>(manager.GetAppIdMap(), GET_MAP_PATH(ingress, appuid), "appuid_map");
}

int32_t NetsysBpfNetFirewall::WriteUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeMap<UidKey>(manager.GetUidMap(), GET_MAP_PATH(ingress, uid), "uid_map");
}

int32_t NetsysBpfNetFirewall::WriteActionBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    return WriteRuleCodeMap<ActionKey>(manager.GetActionMap(), GET_MAP_PATH(ingress, action), "action_map");
}

int32_t NetsysBpfNetFirewall::WriteInterfaceBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    /* See the Write* common funnel (bpf_netfirewall.h): empty desired set
     * clears the map. */
    BpfInterfaceMap &interfaceMap = manager.GetInterfaceMap();

    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    std::vector<std::pair<interface_key, RuleCode>> desired;
    desired.reserve(interfaceMap.Get().size());
    for (const auto &pair : interfaceMap.Get()) {
        const std::string &devName = pair.first;
        const Bitmap &val = pair.second;
        if (devName.length() >= INTERFACE_NAME_MAX_LEN) {
            NETNATIVE_LOGE("WriteInterfaceBpfMap: interface name too long: %{public}s, bitmap=%{public}u",
                           devName.c_str(), val.Get()[0]);
            continue;
        }
        interface_key ifKey = {};
        if (strncpy_s(ifKey.name, INTERFACE_NAME_MAX_LEN, devName.c_str(), devName.length()) != EOK) {
            NETNATIVE_LOGE("WriteInterfaceBpfMap: devName: %{public}s, bitmap=%{public}u", devName.c_str(),
                           val.Get()[0]);
            continue;
        }

        RuleCode rule = {0};
        if (memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode)) != EOK) {
            NETNATIVE_LOGE("WriteInterfaceBpfMap: memcpy_s failed, devName: %{public}s", devName.c_str());
            return -1;
        }
        desired.emplace_back(ifKey, rule);
    }
    return ReconcileBpfMap(GET_MAP_PATH(ingress, iface), desired, false);
}

int32_t NetsysBpfNetFirewall::WriteWildcardBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    /* fix: write all 8 dimension-role entries every rebuild. Roles with no
     * unconstrained rule keep an all-zero bitmap so the fallback AND kills every
     * rule bit (equivalent to the former empty sentinel entry). */
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    Bitmap *wildcardStore = manager.GetWildcardStore();
    std::vector<std::pair<WildcardKey, RuleCode>> desired;
    desired.reserve(WILDCARD_ROLE_MAX);
    for (uint32_t role = 0; role < WILDCARD_ROLE_MAX; ++role) {
        WildcardKey key = static_cast<WildcardKey>(role);
        RuleCode rule = {0};
        if (memcpy_s(rule.val, sizeof(RuleCode), wildcardStore[role].Get(), sizeof(RuleCode)) != EOK) {
            NETNATIVE_LOGE("WriteWildcardBpfMap: memcpy_s failed");
            return -1;
        }
        desired.emplace_back(key, rule);
    }
    return ReconcileBpfMap(GET_MAP_PATH(ingress, wildcard), desired, false);
}

int32_t NetsysBpfNetFirewall::RegisterCallback(const sptr<NetsysNative::INetFirewallCallback> &callback)
{
    if (!callback) {
        return -1;
    }

    std::lock_guard<std::mutex> guard(callbackMutex_);
    callbacks_.emplace_back(callback);

    return 0;
}
int32_t NetsysBpfNetFirewall::UnregisterCallback(const sptr<NetsysNative::INetFirewallCallback> &callback)
{
    if (!callback) {
        return -1;
    }

    std::lock_guard<std::mutex> guard(callbackMutex_);
    for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it) {
        if (*it == callback) {
            callbacks_.erase(it);
            return 0;
        }
    }
    return -1;
}

void NetsysBpfNetFirewall::RegisterInterceptHandler(std::function<void(const InterceptRecord &, uint8_t)> handler)
{
    std::lock_guard<std::mutex> guard(handlerMutex_);
    interceptHandler_ = std::move(handler);
}

void NetsysBpfNetFirewall::NotifyEgressIntercept(const InterceptEvent &info, const sptr<InterceptRecord> &record)
{
    /* Fast-fail (reset/destroy) coverage is protocol-complete (spec 2026-11-16):
     * TCP intercepts (app connections, DoT/TCP-53 queries) and UDP intercepts
     * (denied DNS queries — the musl resolver path) both reach the registered
     * handler for the netlink socket destroy. Any other protocol seen here is
     * a programming error — silently ignored. */
    if (info.dir != EGRESS || (info.protocol != IPPROTO_TCP && info.protocol != IPPROTO_UDP) || record == nullptr) {
        return;
    }
    std::function<void(const InterceptRecord &, uint8_t)> handler;
    {
        std::lock_guard<std::mutex> guard(handlerMutex_);
        handler = interceptHandler_;
    }
    if (handler == nullptr) {
        return;
    }
    // Only for EGRESS: the packet source is the local socket endpoint, so the record tuple can be used directly.
    InterceptRecord destroyRecord = *record;
    handler(destroyRecord, static_cast<uint8_t>(info.family));
}

void NetsysBpfNetFirewall::NotifyInterceptEvent(InterceptEvent *info)
{
    if (!info) {
        return;
    }
    sptr<InterceptRecord> record = sptr<InterceptRecord>::MakeSptr();
    record->time = static_cast<decltype(record->time)>(GetNowMs());
    record->protocol = static_cast<uint16_t>(info->protocol);
    record->appUid = (int32_t)info->appuid;
    std::string srcIp;
    std::string dstIp;
    if (info->family == AF_INET) {
        char ip4[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &(info->ipv4.saddr), ip4, INET_ADDRSTRLEN);
        srcIp = ip4;
        memset_s(ip4, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(info->ipv4.daddr), ip4, INET_ADDRSTRLEN);
        dstIp = ip4;
    } else {
        char ip6[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, &(info->ipv6.saddr), ip6, INET6_ADDRSTRLEN);
        srcIp = ip6;
        memset_s(ip6, INET6_ADDRSTRLEN, 0, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &(info->ipv6.daddr), ip6, INET6_ADDRSTRLEN);
        dstIp = ip6;
    }
    if (info->dir == INGRESS) {
        record->localIp = srcIp;
        record->remoteIp = dstIp;
        record->localPort = BitmapManager::Nstohl(info->sport);
        record->remotePort = BitmapManager::Nstohl(info->dport);
    } else {
        // The BPF tuple of EGRESS has swapped both addresses and ports, so the destination is the local end.
        record->localIp = dstIp;
        record->remoteIp = srcIp;
        record->localPort = BitmapManager::Nstohl(info->dport);
        record->remotePort = BitmapManager::Nstohl(info->sport);
    }
    /* record->domain stays empty: the deny-qname intercept payload no
     * longer carries the qname (the AC-trie rewrite dropped the per-skb
     * domain staging map); the innerkits field is kept for the parcel
     * contract. */
    NotifyEgressIntercept(*info, record);
    std::lock_guard<std::mutex> guard(callbackMutex_);
    for (auto callback : callbacks_) {
        callback->OnIntercept(record);
    }
}

uint64_t NetsysBpfNetFirewall::GetNowMs()
{
    struct timespec ts;
    return (clock_gettime(CLOCK_REALTIME, &ts) == 0)
               ? static_cast<uint64_t>(ts.tv_sec) * MILLIS_PER_SEC + static_cast<uint64_t>(ts.tv_nsec) / NANOS_PER_MILLI
               : static_cast<uint64_t>(time(nullptr)) * MILLIS_PER_SEC;
}

void NetsysBpfNetFirewall::HandleTupleEvent(TupleEvent *ev)
{
    NETNATIVE_LOG_D(
        "%{public}s tuple: sport=%{public}u dport=%{public}u protocol=%{public}u appuid=%{public}u uid=%{public}u",
        (ev->dir == INGRESS) ? "> ingress" : "< egress", ntohs(ev->sport), ntohs(ev->dport), ev->protocol, ev->appuid,
        ev->uid);
    NETNATIVE_LOG_D("\trstpacket=%{public}u", ev->rst);
}

void NetsysBpfNetFirewall::HandleInterceptEvent(InterceptEvent *ev)
{
    GetInstance()->NotifyInterceptEvent(ev);

    NETNATIVE_LOGI("%{public}s intercept: sport=%{public}u dport=%{public}u protocol=%{public}u appuid=%{public}u",
                   (ev->dir == INGRESS) ? "ingress" : "egress", ntohs(ev->sport), ntohs(ev->dport), ev->protocol,
                   ev->appuid);
}

void NetsysBpfNetFirewall::HandleDebugEvent(DebugEvent *ev)
{
    const char *direction = ev->dir == INGRESS ? ">" : "<";
    switch (ev->type) {
        case DBG_MATCH_SPORT:
            NETNATIVE_LOG_D("%{public}s sport: %{public}u bitmap: %{public}x", direction, ntohs(ev->arg1), ev->arg2);
            break;
        case DBG_MATCH_DPORT:
            NETNATIVE_LOG_D("%{public}s dport: %{public}u bitmap: %{public}x", direction, ntohs(ev->arg1), ev->arg2);
            break;
        case DBG_MATCH_PROTO:
            NETNATIVE_LOG_D("%{public}s protocol: %{public}u bitmap: %{public}x", direction, ev->arg1, ev->arg2);
            break;
        case DBG_MATCH_APPUID:
            NETNATIVE_LOG_D("%{public}s appuid: %{public}u bitmap: %{public}x", direction, ev->arg1, ev->arg2);
            break;
        case DBG_MATCH_UID:
            NETNATIVE_LOG_D("%{public}s uid: %{public}u bitmap: %{public}x", direction, ev->arg1, ev->arg2);
            break;
        case DBG_ACTION_KEY:
            NETNATIVE_LOG_D("%{public}s actionkey: %{public}x", direction, ev->arg1);
            break;
        case DBG_MATCH_ACTION:
            NETNATIVE_LOG_D("%{public}s    action: %{public}s", direction, (ev->arg1 == SK_PASS ? "PASS" : "DROP"));
            break;
        case DBG_CT_LOOKUP:
            NETNATIVE_LOG_D("%{public}s ct lookup status: %{public}u", direction, ev->arg1);
            break;
        case DBG_MATCH_DOMAIN:
            NETNATIVE_LOG_D("egress match domain, action PASS");
            break;
        case DBG_MATCH_DOMAIN_ACTION:
            NETNATIVE_LOG_D("%{public}s match domain action: %{public}s", direction,
                            (ev->arg1 == SK_PASS ? "PASS" : "DROP"));
            break;
        case DBG_MATCH_INTERFACE:
            NETNATIVE_LOG_D("%{public}s interface: %{public}u bitmap: %{public}x", direction, ev->arg1, ev->arg2);
            break;
        case DBG_DNS_RECORD_WRITE:
            /* A query record was flushed at the egress hook exit. */
            NETNATIVE_LOG_D("dns query record written, txid=%{public}u", ev->arg1);
            break;
        case DBG_DNS_LEARN_REJECT:
            /* Learning rejected (no/expired record or non-allow qname) —
             * the response itself keeps flowing. */
            NETNATIVE_LOG_D("dns learn rejected: no matching query record");
            break;
        default:
            break;
    }
}

int NetsysBpfNetFirewall::HandleEvent(void *ctx, void *data, size_t len)
{
    if (data != NULL && len >= sizeof(Event)) {
        Event *ev = (Event *)data;

        switch (ev->type) {
            case EVENT_DEBUG: {
                HandleDebugEvent(&(ev->debug));
                break;
            }
            case EVENT_INTERCEPT: {
                HandleInterceptEvent(&(ev->intercept));
                break;
            }
            case EVENT_TUPLE_DEBUG: {
                HandleTupleEvent(&(ev->tuple));
                break;
            }
            default:
                break;
        }
    }
    return 0;
}

int32_t NetsysBpfNetFirewall::LoadSystemAbility(int32_t systemAbilityId)
{
    NETNATIVE_LOG_D("GetSystemAbility: [%{public}d]", systemAbilityId);
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        NETNATIVE_LOGE("GetCmProxy registry is null");
        return -1;
    }

    auto object = saManager->CheckSystemAbility(systemAbilityId);
    if (object != nullptr) {
        return 0;
    }
    sptr<OHOS::IRemoteObject> ret = saManager->GetSystemAbility(systemAbilityId);
    if (ret == nullptr || !ret->IsProxyObject()) {
        NETNATIVE_LOGE("ret is nullptr");
        return -1;
    }

    return 0;
}

/* The former AddDomainCache(NetAddrInfo) was dead control-plane
 * code (no callers, all-zero value) and was removed with the domain_value
 * timestamp/ttl extension — see bpf_netfirewall.h. */

void NetsysBpfNetFirewall::ClearDomainCache()
{
    NETNATIVE_LOG_D("ClearDomainCache");
    Ipv4LpmKey ip4Key = {};
    Ipv6LpmKey ip6Key = {};
    domain_value value{0};
    ClearBpfMap(MAP_PATH(DOMAIN_IPV4_MAP), ip4Key, value);
    ClearBpfMap(MAP_PATH(DOMAIN_IPV6_MAP), ip6Key, value);
}

namespace {
/* Walk one learned-entry map and reclaim entries past their TTL.
 * Mirrors the ConntrackGcTask shape (GetAllKeys + Read + Delete). A single
 * failed delete is logged and skipped — the next sweep retries it; a map that
 * cannot be opened aborts the family's sweep only. Entries with timestamp=0
 * cannot pass the BPF gate (domain_entry_expired treats them as expired) so
 * they are reclaimed here as dead weight. */
template <typename Key> int32_t AgeDomainMapEntries(const char *path, uint64_t nowNs, uint32_t &deleted)
{
    BpfMapper<Key, DomainValue> rdMap(path, BPF_F_RDONLY);
    if (!rdMap.IsValid()) {
        return -1;
    }
    std::vector<Key> keys = rdMap.GetAllKeys();
    if (keys.empty()) {
        return 0;
    }
    BpfMapper<Key, DomainValue> wrMap(path, BPF_F_WRONLY);
    if (!wrMap.IsValid()) {
        NETNATIVE_LOGE("AgeDomainMapEntries: wrMap is invalid: %{public}s", path);
        return -1;
    }
    for (const Key &k : keys) {
        DomainValue v = {};
        if (rdMap.Read(k, v) < 0) {
            NETNATIVE_LOGE("AgeDomainMapEntries: read failed, skip this round");
            continue;
        }
        if (domain_entry_expired(nowNs, v.timestamp, v.ttl) == 0) {
            continue;
        }
        if (wrMap.Delete(k) != 0) {
            NETNATIVE_LOGE("AgeDomainMapEntries: delete failed, retried next sweep");
            continue;
        }
        deleted++;
    }
    return 0;
}
} // namespace

int32_t NetsysBpfNetFirewall::DomainCacheAgingSweep()
{
    timespec now = {0};
    /* bpf_ktime_get_ns is CLOCK_MONOTONIC — the user-space aging task MUST use
     * the same clock domain, otherwise entries get misjudged as (un)expired
     * (same convention as ConntrackGcTask). */
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        NETNATIVE_LOGE("DomainCacheAgingSweep: clock_gettime failed");
        return -1;
    }
    uint64_t nowNs = static_cast<uint64_t>(now.tv_sec) * DNS_NS_PER_SEC + static_cast<uint64_t>(now.tv_nsec);

    int32_t ret = 0;
    uint32_t deleted = 0;
    if (AgeDomainMapEntries<Ipv4LpmKey>(MAP_PATH(DOMAIN_IPV4_MAP), nowNs, deleted) != 0) {
        ret = -1;
    }
    deleted = 0;
    if (AgeDomainMapEntries<Ipv6LpmKey>(MAP_PATH(DOMAIN_IPV6_MAP), nowNs, deleted) != 0) {
        ret = -1;
    }
    return ret;
}

void NetsysBpfNetFirewall::StartDomainCacheAgingThread(void)
{
    if (keepAging_) {
        NETNATIVE_LOG_D("DomainCacheAging: already running");
        return;
    }
    keepAging_ = true;
    int prewarmSweeps = 0;
    while (keepAging_) {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(DOMAIN_CACHE_AGING_INTERVAL_MS));
        if (!keepAging_) {
            break;
        }
        (void)GetInstance()->DomainCacheAgingSweep();
        /* Periodic re-resolve refreshes pre-warm entries
         * long before the fixed 1h TTL expires (10 sweeps × 60s = 10min); a
         * no-op when nothing is registered (retail, unit tests). */
        if (++prewarmSweeps >= DNS_PREWARM_INTERVAL_SWEEPS) {
            prewarmSweeps = 0;
            GetInstance()->TriggerDnsPrewarm();
        }
    }
}

/* ===== DNS server authorization maps + pre-warm engine ===== */

int32_t NetsysBpfNetFirewall::BuildDnsServerKeys(const std::vector<std::string> &servers, int32_t userId,
                                                 std::vector<DnsServerV4Key> &v4Keys,
                                                 std::vector<DnsServerV6Key> &v6Keys)
{
    v4Keys.clear();
    v6Keys.clear();
    if (userId < 0) {
        NETNATIVE_LOGE("BuildDnsServerKeys: invalid userId %{public}d", userId);
        return NETFIREWALL_ERR;
    }
    std::set<std::string> seen;
    for (const auto &server : servers) {
        /* 每用户 ≤8 条容量控制：primary/standby + 冗余之外的条目
         * 一律截断，map 侧永远不可能超发。 */
        if (v4Keys.size() + v6Keys.size() >= DNS_SERVER_MAX_PER_USER) {
            break;
        }
        if (server.empty() || !seen.insert(server).second) {
            continue;
        }
        struct in_addr v4 = {};
        if (inet_pton(AF_INET, server.c_str(), &v4) == 1) {
            DnsServerV4Key key = {};
            key.uid = static_cast<uint32_t>(userId);
            key.ip = v4.s_addr; /* network order — exactly what the data plane sees */
            v4Keys.push_back(key);
            continue;
        }
        struct in6_addr v6 = {};
        if (inet_pton(AF_INET6, server.c_str(), &v6) == 1) {
            DnsServerV6Key key = {};
            key.uid = static_cast<uint32_t>(userId);
            key.ip = v6;
            v6Keys.push_back(key);
            continue;
        }
        /* Never write a partial or malformed key — skip and let
         * the caller observe the (smaller) count in the sync event. */
        NETNATIVE_LOGE("BuildDnsServerKeys: invalid server skipped");
    }
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetDnsServerMaps(int32_t userId, const std::vector<std::string> &servers)
{
    if (userId < 0) {
        return NETFIREWALL_ERR;
    }
    std::vector<DnsServerV4Key> v4Keys;
    std::vector<DnsServerV6Key> v6Keys;
    int32_t ret = BuildDnsServerKeys(servers, userId, v4Keys, v6Keys);
    if (ret != NETFIREWALL_SUCCESS) {
        return ret;
    }
    uint32_t written = 0;
    uint32_t deleted = 0;
    ret = NETFIREWALL_SUCCESS;
    if (ApplyUserServerKeys<DnsServerV4Key>(MAP_PATH(DNS_SERVER_V4_MAP), static_cast<uint32_t>(userId), v4Keys, written,
                                            deleted) != NETFIREWALL_SUCCESS) {
        ret = NETFIREWALL_ERR;
    }
    if (ApplyUserServerKeys<DnsServerV6Key>(MAP_PATH(DNS_SERVER_V6_MAP), static_cast<uint32_t>(userId), v6Keys, written,
                                            deleted) != NETFIREWALL_SUCCESS) {
        ret = NETFIREWALL_ERR;
    }
    NETNATIVE_LOGI("SetDnsServerMaps: user=%{public}d entries=%{public}zu written=%{public}u deleted=%{public}u",
                   userId, v4Keys.size() + v6Keys.size(), written, deleted);
    return ret;
}

int32_t NetsysBpfNetFirewall::ClearDnsServerMaps(int32_t userId)
{
    if (userId < 0) {
        return NETFIREWALL_ERR;
    }
    uint32_t deleted = 0;
    int32_t ret = NETFIREWALL_SUCCESS;
    if (ClearUserServerKeys<DnsServerV4Key>(MAP_PATH(DNS_SERVER_V4_MAP), static_cast<uint32_t>(userId), deleted) !=
        NETFIREWALL_SUCCESS) {
        ret = NETFIREWALL_ERR;
    }
    if (ClearUserServerKeys<DnsServerV6Key>(MAP_PATH(DNS_SERVER_V6_MAP), static_cast<uint32_t>(userId), deleted) !=
        NETFIREWALL_SUCCESS) {
        ret = NETFIREWALL_ERR;
    }
    NETNATIVE_LOGI("ClearDnsServerMaps: user=%{public}d deleted=%{public}u ret=%{public}d", userId, deleted, ret);
    return ret;
}

bool NetsysBpfNetFirewall::DnsServerSyncNeeded(int32_t userId)
{
    /* Write condition: egress default deny AND >= 1 domain allow
     * rule — anything else must hold no authorization entry (绝对隔离). */
    std::lock_guard<std::mutex> guard(dnsServerMutex_);
    auto hasAllow = dnsUserHasAllowDomain_.find(userId);
    auto defaultDeny = dnsUserDefaultDeny_.find(userId);
    return hasAllow != dnsUserHasAllowDomain_.end() && hasAllow->second && defaultDeny != dnsUserDefaultDeny_.end() &&
           defaultDeny->second;
}

bool NetsysBpfNetFirewall::DnsPrewarmNeeded(int32_t userId)
{
    /* Eligibility: >= 1 domain allow rule remains — no default-action
     * precondition. A default-allow user's learned entry is what carries
     * the domain allow past explicit deny rules (the rule-hit override), so
     * those users need the warmed cache as well; the server authorization
     * maps keep the stricter condition above. */
    std::lock_guard<std::mutex> guard(dnsServerMutex_);
    auto hasAllow = dnsUserHasAllowDomain_.find(userId);
    return hasAllow != dnsUserHasAllowDomain_.end() && hasAllow->second;
}

int32_t NetsysBpfNetFirewall::SyncUserDnsServerMaps(int32_t userId)
{
    if (userId < 0) {
        return NETFIREWALL_ERR;
    }
    if (!DnsServerSyncNeeded(userId)) {
        return ClearDnsServerMaps(userId);
    }
    std::function<int32_t(std::vector<std::string> &)> provider;
    {
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        provider = dnsServerSetProvider_;
    }
    if (!provider) {
        NETNATIVE_LOGE("SyncUserDnsServerMaps: no server-set provider registered");
        return NETFIREWALL_ERR;
    }
    std::vector<std::string> servers;
    /* The provider returns the BASE resolver chain only — the
     * RULE_DNS per-app substitution never leaks into the authorization map. */
    if (provider(servers) != NETFIREWALL_SUCCESS) {
        NETNATIVE_LOGE("SyncUserDnsServerMaps: provider failed");
        return NETFIREWALL_ERR;
    }
    int32_t ret = SetDnsServerMaps(userId, servers);
    /* SET/RESET sync tasks run unserialized — a rule clear may
     * have flipped the condition to false while this SET was building and
     * applying, which would leave stale authorization entries with no
     * future trigger to remove them. Re-check after the write and converge
     * to the cleared state. */
    if (ret == NETFIREWALL_SUCCESS && !DnsServerSyncNeeded(userId)) {
        NETNATIVE_LOGI("SyncUserDnsServerMaps: condition lost mid-apply, clearing user %{public}d", userId);
        return ClearDnsServerMaps(userId);
    }
    return ret;
}

void NetsysBpfNetFirewall::RunDnsServerSyncTask(int32_t userId)
{
    if (userId == DNS_SERVER_SYNC_RESET) {
        uint32_t deleted = 0;
        int32_t ret = NETFIREWALL_SUCCESS;
        if (ResetServerMapKeys<DnsServerV4Key>(MAP_PATH(DNS_SERVER_V4_MAP), deleted) != NETFIREWALL_SUCCESS) {
            ret = NETFIREWALL_ERR;
        }
        if (ResetServerMapKeys<DnsServerV6Key>(MAP_PATH(DNS_SERVER_V6_MAP), deleted) != NETFIREWALL_SUCCESS) {
            ret = NETFIREWALL_ERR;
        }
        NETNATIVE_LOGI("RunDnsServerSyncTask: reset deleted=%{public}u ret=%{public}d", deleted, ret);
        return;
    }
    (void)GetInstance()->SyncUserDnsServerMaps(userId);
}

void NetsysBpfNetFirewall::NotifyDnsRuleChanged(int32_t userId, bool hasAllowDomain)
{
    if (userId < 0) {
        return;
    }
    {
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        if (hasAllowDomain) {
            dnsUserHasAllowDomain_[userId] = true;
        } else {
            dnsUserHasAllowDomain_.erase(userId);
        }
    }
    ffrt::submit([userId]() { RunDnsServerSyncTask(userId); }, {}, {}, ffrt::task_attr().name("DnsSrvSync"));
    if (hasAllowDomain) {
        TriggerDnsPrewarm();
    }
}

void NetsysBpfNetFirewall::NotifyDnsDefaultActionChanged(int32_t userId, bool defaultDeny)
{
    if (userId < 0) {
        return;
    }
    {
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        if (defaultDeny) {
            dnsUserDefaultDeny_[userId] = true;
        } else {
            dnsUserDefaultDeny_.erase(userId);
        }
    }
    ffrt::submit([userId]() { RunDnsServerSyncTask(userId); }, {}, {}, ffrt::task_attr().name("DnsSrvSync"));
    /* Prewarm eligibility no longer depends on the default action (users
     * with allow domains warm in both modes); the re-arm only refreshes
     * entries alongside the server-map sync and coalesces with any sweep
     * already in flight. */
    TriggerDnsPrewarm();
}

void NetsysBpfNetFirewall::NotifyDnsResolverChanged()
{
    std::vector<int32_t> users;
    {
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        for (const auto &entry : dnsUserHasAllowDomain_) {
            if (entry.second) {
                users.push_back(entry.first);
            }
        }
    }
    for (int32_t userId : users) {
        ffrt::submit([userId]() { RunDnsServerSyncTask(userId); }, {}, {}, ffrt::task_attr().name("DnsSrvSync"));
    }
    TriggerDnsPrewarm();
}

void NetsysBpfNetFirewall::RemoveDnsServerUser(int32_t userId)
{
    if (userId < 0) {
        return;
    }
    {
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        dnsUserHasAllowDomain_.erase(userId);
        dnsUserDefaultDeny_.erase(userId);
        dnsPrewarmDomains_.erase(userId);
    }
    ffrt::submit([userId]() { (void)GetInstance()->ClearDnsServerMaps(userId); }, {}, {},
                 ffrt::task_attr().name("DnsSrvSync"));
}

int32_t NetsysBpfNetFirewall::AddPrewarmDomainCache(int32_t userId, const std::string &domain,
                                                    const std::vector<std::string> &ips)
{
    /* Only exact domains are pre-resolved — a '*' wildcard (leading or
     * trailing) cannot be enumerated, its appearance here is a caller bug. */
    if (userId < 0 || domain.empty() || domain.find('*') != std::string::npos) {
        NETNATIVE_LOGE("AddPrewarmDomainCache: invalid args");
        return NETFIREWALL_ERR;
    }
    struct timespec now = {};
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        NETNATIVE_LOGE("AddPrewarmDomainCache: clock_gettime failed");
        return NETFIREWALL_ERR;
    }
    DomainValue val = {};
    /* Same clock domain as bpf_ktime_get_ns: the BPF read-time gate
     * judges pre-warm entries with the same arithmetic as learned ones.
     * appuid = 0 — the MatchDomain relaxation makes one entry serve every
     * app of the rule's user. GetAddrInfo exposes no answer TTL, so a fixed
     * 1h window is used and the periodic re-resolve refreshes it. */
    val.uid = static_cast<uint32_t>(userId);
    val.appuid = 0;
    val.timestamp = static_cast<uint64_t>(now.tv_sec) * 1000000000ULL + static_cast<uint64_t>(now.tv_nsec);
    val.ttl = static_cast<uint32_t>(DNS_PREWARM_TTL_SECONDS);
    /* One WRONLY mapper per family for the whole call — the
     * per-IP open/close inside WriteBpfMap was N_domains × M_ips map opens
     * per pre-warm sweep. Failure semantics unchanged: any write failure
     * still reports NETFIREWALL_ERR (an invalid mapper fails every write). */
    BpfMapper<Ipv4LpmKey, DomainValue> v4Map(MAP_PATH(DOMAIN_IPV4_MAP), BPF_F_WRONLY);
    BpfMapper<Ipv6LpmKey, DomainValue> v6Map(MAP_PATH(DOMAIN_IPV6_MAP), BPF_F_WRONLY);
    int32_t ret = NETFIREWALL_SUCCESS;
    for (const auto &ip : ips) {
        struct in_addr v4 = {};
        if (inet_pton(AF_INET, ip.c_str(), &v4) == 1) {
            Ipv4LpmKey key = {};
            key.prefixlen = 32;
            key.data = v4.s_addr;
            if (!v4Map.IsValid() || v4Map.Write(key, val, BPF_ANY) != 0) {
                ret = NETFIREWALL_ERR;
            }
            continue;
        }
        struct in6_addr v6 = {};
        if (inet_pton(AF_INET6, ip.c_str(), &v6) == 1) {
            Ipv6LpmKey key = {};
            key.prefixlen = 128;
            key.data = v6;
            if (!v6Map.IsValid() || v6Map.Write(key, val, BPF_ANY) != 0) {
                ret = NETFIREWALL_ERR;
            }
            continue;
        }
        NETNATIVE_LOGE("AddPrewarmDomainCache: invalid ip skipped");
    }
    return ret;
}

void NetsysBpfNetFirewall::TriggerDnsPrewarm()
{
    bool hasWork = false;
    {
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        hasWork = !dnsPrewarmDomains_.empty() && dnsPrewarmResolver_;
    }
    if (!hasWork) {
        return;
    }
    /* Coalesce (debounce) — at most one sweep in
     * flight; triggers arriving mid-run fold into a single pending re-sweep
     * that re-snapshots the current domain set (merge semantics). Without
     * this, resolver-change bursts (network flapping) fan out concurrent
     * full resolver sweeps — pure-cost concurrency over the same space. */
    if (dnsPrewarmInFlight_.exchange(true)) {
        dnsPrewarmPending_.store(true);
        return;
    }
    ffrt::submit(RunDnsPrewarmTask, {}, {}, ffrt::task_attr().name("DnsPrewarm"));
}

void NetsysBpfNetFirewall::RunDnsPrewarmTask()
{
    auto self = GetInstance();
    /* Snapshot under the lock, resolve and write lock-free: a resolver call
     * is seconds-long network I/O and must never stretch a critical section. */
    std::map<int32_t, std::set<std::string>> domains;
    std::function<int32_t(const std::string &, std::vector<std::string> &)> resolver;
    {
        std::lock_guard<std::mutex> guard(self->dnsServerMutex_);
        domains = self->dnsPrewarmDomains_;
        resolver = self->dnsPrewarmResolver_;
    }
    if (!resolver) {
        /* Every exit path must release the in-flight slot — a wedged
         * slot would silently swallow every later trigger. */
        self->dnsPrewarmInFlight_.store(false);
        return;
    }
    for (const auto &entry : domains) {
        int32_t userId = entry.first;
        /* The snapshot only holds users with at least one exact allow
         * domain (wildcard-only rules are not enumerable) — every entry is
         * pre-warm eligible regardless of the user's default action: the
         * learned entry is what lets a domain allow override an explicit
         * deny rule, so default-allow users need the warmed cache too. */
        for (const auto &domain : entry.second) {
            std::vector<std::string> ips;
            if (resolver(domain, ips) != NETFIREWALL_SUCCESS || ips.empty()) {
                NETNATIVE_LOGW("RunDnsPrewarmTask: resolve failed: %{public}s", domain.c_str());
                continue;
            }
            /* The resolve above is seconds-long — a rule clear
             * may have dropped this user's allow-domain condition mid-flight,
             * and a write now would stage a stale allow entry (1h TTL) with
             * no later trigger to remove it. Re-check per write; skip the
             * user's remaining domains once the condition is gone. */
            if (!self->DnsPrewarmNeeded(userId)) {
                NETNATIVE_LOGI("RunDnsPrewarmTask: allow-domain condition lost mid-run, skip user %{public}d", userId);
                break;
            }
            if (self->AddPrewarmDomainCache(userId, domain, ips) != NETFIREWALL_SUCCESS) {
                NETNATIVE_LOGW("RunDnsPrewarmTask: prewarm write failed: %{public}s", domain.c_str());
            }
        }
    }
    /* Release the slot, then honor the coalesced pending
     * trigger with at most one re-sweep — it re-snapshots the then-current
     * domain set, so changes that arrived mid-run are not lost. A pending
     * store landing in the window between the two atomics below is
     * consumed by the next sweep tail instead (deferred, never lost; the
     * <=10min periodic sweeper backstops). */
    self->dnsPrewarmInFlight_.store(false);
    if (self->dnsPrewarmPending_.exchange(false)) {
        self->TriggerDnsPrewarm();
    }
}

void NetsysBpfNetFirewall::RegisterDnsServerSetProvider(std::function<int32_t(std::vector<std::string> &)> provider)
{
    std::lock_guard<std::mutex> guard(dnsServerMutex_);
    dnsServerSetProvider_ = std::move(provider);
}

void NetsysBpfNetFirewall::RegisterDnsPrewarmResolver(
    std::function<int32_t(const std::string &, std::vector<std::string> &)> resolver)
{
    std::lock_guard<std::mutex> guard(dnsServerMutex_);
    dnsPrewarmResolver_ = std::move(resolver);
}

void NetsysBpfNetFirewall::RegisterDnsCacheFlushProvider(std::function<int32_t()> provider)
{
    std::lock_guard<std::mutex> guard(dnsServerMutex_);
    dnsCacheFlushProvider_ = std::move(provider);
}

void NetsysBpfNetFirewall::FlushDnsCacheAfterRuleChange()
{
    std::function<int32_t()> flush;
    {
        /* Copy the provider under the registration lock, call OUTSIDE it:
         * the provider takes the dns-layer cache mutex, and the only
         * established cross-layer order is cacheMutex_ -> dnsServerMutex_. */
        std::lock_guard<std::mutex> guard(dnsServerMutex_);
        flush = dnsCacheFlushProvider_;
    }
    if (!flush) {
        return;
    }
    int32_t flushed = flush();
    if (flushed < 0) {
        NETNATIVE_LOGE("FlushDnsCacheAfterRuleChange: flush provider failed: %{public}d", flushed);
    } else {
        NETNATIVE_LOGI("FlushDnsCacheAfterRuleChange: flushed dns cache of %{public}d netId(s)", flushed);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS