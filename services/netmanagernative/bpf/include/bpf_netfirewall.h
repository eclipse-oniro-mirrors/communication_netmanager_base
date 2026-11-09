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

#include <netdb.h>
#include <array>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <functional>
#include <atomic>
#include <mutex>

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
using RuleCode = struct bitmap;
using StreamDir = enum stream_dir;
using EventType = enum event_type;
using Event = struct event;
using InterceptEvent = struct intercept_event;
using DebugEvent = struct debug_event;
using TupleEvent = struct match_tuple;
using DebugType = enum debug_type;

using CtKey = struct ct_tuple;
using CtVaule = struct ct_entry;

using DomainHashKey = struct domain_hash_key;
using DomainValue = domain_value;
using LoopbackValue = loop_back_val;

struct NetAddrInfo {
    uint32_t aiFamily;
    union {
        struct in_addr sin;
        struct in6_addr sin6;
    } aiAddr;
};

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

    void AddDomainCache(const NetAddrInfo &addrInfo);
    void ClearDomainCache();

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

    int32_t SetBpfFirewallRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList,
        NetFirewallRuleDirection direction);

    int32_t SetFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList);

    int32_t SetFirewallDomainRules(const std::vector<sptr<NetFirewallDomainRule>> &ruleList);

    bool GetDomainHashKey(const std::string &domain, DomainHashKey &out, bool isReversed = false);

    int32_t SetBpfFirewallDomainRules(FirewallRuleAction action, DomainHashKey &key, DomainValue value,
        bool isWildcard);

    int32_t SetBpfFirewallDomainPrefixRules(FirewallRuleAction action, DomainHashKey &key, DomainValue value,
 	         bool isWildcard);

    int32_t SetSingleDomainRule(FirewallRuleAction action, const NetFirewallDomainParam &param, DomainValue value,
        int32_t &ret);

    void ClearDomainRules();

    int32_t WriteLoopBackBpfMap();

    std::string DecodeDomainFromKey(const DomainHashKey &key);

    static std::shared_ptr<NetsysBpfNetFirewall> instance_;
    static std::atomic<bool> isBpfLoaded_;
    static std::atomic<bool> keepListen_;
    std::unique_ptr<std::thread> thread_;
    std::vector<sptr<NetsysNative::INetFirewallCallback>> callbacks_;
    std::mutex callbackMutex_;
    std::function<void(const InterceptRecord &, uint8_t)> interceptHandler_;
    std::mutex handlerMutex_;
    static std::atomic<bool> keepGc_;
    std::unique_ptr<std::thread> gcThread_;
    static std::unique_ptr<BpfMapper<CtKey, CtVaule>> ctRdMap_, ctWrMap_;
    std::vector<sptr<NetFirewallIpRule>> firewallIpRules_;
    std::vector<sptr<NetFirewallDomainRule>> firewallDomainRules_;
    std::mutex rulesMutex_;
    std::mutex snapshotMutex_;
    std::unordered_map<std::string, BpfMapSnapshot> appliedSnapshots_;
};
} // namespace OHOS::NetManagerStandard
#endif /* NETMANAGER_EXT_BPF_NET_FIREWALL_H */
