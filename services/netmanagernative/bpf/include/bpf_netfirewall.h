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
#include <string>
#include <thread>
#include <unordered_map>
#include <chrono>
#include <functional>

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

struct NetAddrInfo {
    uint32_t aiFamily;
    union {
        struct in_addr sin;
        struct in6_addr sin6;
    } aiAddr;
};

/**
 * @brief Callback impl for LoadSystemAbility
 */
class OnDemandLoadManagerCallback : public SystemAbilityLoadCallbackStub {
public:
    /**
     * called when load SA success
     *
     * @param systemAbilityId id of SA which was loaded
     * @param remoteObject poniter of IRemoteObject
     */
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;

    /**
     * called when load SA fail
     *
     * @param systemAbilityId id of SA which was loaded
     */
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
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
     * @param ruleList list of NetFirewallIpRule
     * @param isFinish transmit finish or not
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallRules(const std::vector<sptr<NetFirewallBaseRule>> &ruleList, bool isFinish);

    /**
     * Set firewall default action
     *
     * @param inDefault  Default action of NetFirewallRuleDirection:RULE_IN
     * @param outDefault Default action of NetFirewallRuleDirection:RULE_OUT
     * @return  0 if success or -1 if an error occurred
     */
    int32_t SetFirewallDefaultAction(FirewallRuleAction inDefault, FirewallRuleAction outDefault);

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
     * @return  0 if success or -1 if an error occurred
     */
    int32_t ClearFirewallRules();

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

    NetsysBpfNetFirewall();

    static void StartConntrackGcThread(void);

    static void RingBufferListenThread(void);

    void StopConntrackGc();

    static int HandleEvent(void *ctx, void *data, size_t len);

    static void HandleTupleEvent(TupleEvent *ev);

    static void HandleInterceptEvent(InterceptEvent *ev);

    static void HandleDebugEvent(DebugEvent *ev);

    bool ShouldSkipNotify(sptr<InterceptRecord> record);

    void NotifyInterceptEvent(InterceptEvent *info);

    static void ConntrackGcTask();

    void ClearBpfFirewallRules(NetFirewallRuleDirection direction);

    void WriteSrcIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteSrcIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteDstIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteDstIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteSrcPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteDstPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteProtoBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteAppUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    void WriteActionBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction);

    int32_t SetBpfFirewallRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList,
        NetFirewallRuleDirection direction);

    int32_t SetFirewallIpRules(const std::vector<sptr<NetFirewallIpRule>> &ruleList);

    static std::shared_ptr<NetsysBpfNetFirewall> instance_;
    static bool isBpfLoaded_;
    static bool keepListen_;
    std::unique_ptr<std::thread> thread_;
    std::vector<sptr<NetsysNative::INetFirewallCallback>> callbacks_;
    sptr<InterceptRecord> oldRecord_ = nullptr;
    static bool keepGc_;
    std::unique_ptr<std::thread> gcThread_;
    static std::unique_ptr<BpfMapper<CtKey, CtVaule>> ctRdMap_, ctWrMap_;
    std::vector<sptr<NetFirewallIpRule>> firewallIpRules_;
    std::vector<NetAddrInfo> domainCache_;
};
} // namespace OHOS::NetManagerStandard
#endif /* NETMANAGER_EXT_BPF_NET_FIREWALL_H */
