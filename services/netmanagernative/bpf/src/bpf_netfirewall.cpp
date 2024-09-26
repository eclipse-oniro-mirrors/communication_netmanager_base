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

#include <arpa/inet.h>
#include <cstdio>
#include <libbpf.h>
#include <linux/bpf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <securec.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
#include <ctime>

#include "bpf_loader.h"
#include "bpf_netfirewall.h"
#include "netnative_log_wrapper.h"
#include "iservice_registry.h"
#include "bpf_ring_buffer.h"
#include "ffrt_inner.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::NetsysNative;

namespace OHOS {
namespace NetManagerStandard {
std::shared_ptr<NetsysBpfNetFirewall> NetsysBpfNetFirewall::instance_ = nullptr;
bool NetsysBpfNetFirewall::keepListen_ = false;
bool NetsysBpfNetFirewall::keepGc_ = false;
bool NetsysBpfNetFirewall::isBpfLoaded_ = false;
std::unique_ptr<BpfMapper<CtKey, CtVaule>> NetsysBpfNetFirewall::ctRdMap_ = nullptr;
std::unique_ptr<BpfMapper<CtKey, CtVaule>> NetsysBpfNetFirewall::ctWrMap_ = nullptr;

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

    timespec now = { 0 };
    // bpf_ktime_get_ns: CLOCK_MONOTONIC
    if (!clock_gettime(CLOCK_MONOTONIC, &now)) {
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
    ffrt::submit(StartConntrackGcThread, { &ctRdMap_ }, { &ctWrMap_ });
    return 0;
}

int32_t NetsysBpfNetFirewall::StopListener()
{
    keepListen_ = false;
    StopConntrackGc();
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
}

void NetsysBpfNetFirewall::ClearBpfFirewallRules(NetFirewallRuleDirection direction)
{
    NETNATIVE_LOG_D("ClearBpfFirewallRules: direction=%{public}d", direction);
    Ipv4LpmKey ip4Key = {};
    Ipv6LpmKey ip6Key = {};
    PortKey portKey = 0;
    ProtoKey protoKey = 0;
    AppUidKey appIdKey = 0;
    UidKey uidKey = 0;
    ActionKey actKey = 1;
    ActionValue actVal;
    RuleCode ruleCode;
    CtKey ctKey;
    CtVaule ctVal;

    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    ClearBpfMap(GET_MAP_PATH(ingress, saddr), ip4Key, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, saddr6), ip6Key, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, daddr), ip4Key, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, daddr6), ip6Key, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, sport), portKey, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, dport), portKey, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, proto), protoKey, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, appuid), appIdKey, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, uid), uidKey, ruleCode);
    ClearBpfMap(GET_MAP_PATH(ingress, action), actKey, actVal);
    ClearBpfMap(MAP_PATH(CT_MAP), ctKey, ctVal);
}

int32_t NetsysBpfNetFirewall::ClearFirewallRules()
{
    firewallIpRules_.clear();
    ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_IN);
    ClearBpfFirewallRules(NetFirewallRuleDirection::RULE_OUT);
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

    ClearBpfFirewallRules(direction);
    WriteSrcIpv4BpfMap(manager, direction);
    WriteSrcIpv6BpfMap(manager, direction);
    WriteDstIpv4BpfMap(manager, direction);
    WriteDstIpv6BpfMap(manager, direction);
    WriteSrcPortBpfMap(manager, direction);
    WriteDstPortBpfMap(manager, direction);
    WriteProtoBpfMap(manager, direction);
    WriteAppUidBpfMap(manager, direction);
    WriteUidBpfMap(manager, direction);
    WriteActionBpfMap(manager, direction);
    return NETFIREWALL_SUCCESS;
}

int32_t NetsysBpfNetFirewall::SetFirewallRules(const std::vector<sptr<NetFirewallBaseRule>> &ruleList, bool isFinish)
{
    NETNATIVE_LOGI("SetFirewallRules: size=%{public}zu isFinish=%{public}" PRId32, ruleList.size(), isFinish);
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("SetFirewallRules: bpf not loaded");
        return NETFIREWALL_ERR;
    }
    if (ruleList.empty()) {
        NETNATIVE_LOGE("SetFirewallRules: rules is empty");
        return NETFIREWALL_ERR;
    }
    for (const auto &rule : ruleList) {
        firewallIpRules_.emplace_back(firewall_rule_cast<NetFirewallIpRule>(rule));
    }
    int32_t ret = NETFIREWALL_SUCCESS;
    if (isFinish) {
        ret = SetFirewallIpRules(firewallIpRules_);
        firewallIpRules_.clear();
    }
    return ret;
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

int32_t NetsysBpfNetFirewall::SetFirewallDefaultAction(FirewallRuleAction inDefault, FirewallRuleAction outDefault)
{
    if (!isBpfLoaded_) {
        NETNATIVE_LOGE("SetFirewallDefaultAction: bpf not loaded");
        return NETFIREWALL_ERR;
    }
    DefaultActionKey key = DEFAULT_ACT_IN_KEY;
    enum sk_action val = (inDefault == FirewallRuleAction::RULE_ALLOW) ? SK_PASS : SK_DROP;
    WriteBpfMap(MAP_PATH(DEFAULT_ACTION_MAP), key, val);

    key = DEFAULT_ACT_OUT_KEY;
    val = (outDefault == FirewallRuleAction::RULE_ALLOW) ? SK_PASS : SK_DROP;
    WriteBpfMap(MAP_PATH(DEFAULT_ACTION_MAP), key, val);
    CtKey ctKey;
    CtVaule ctVal;
    ClearBpfMap(MAP_PATH(CT_MAP), ctKey, ctVal);
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
    WriteBpfMap(MAP_PATH(CURRENT_UID_MAP), key, val);
    return NETFIREWALL_SUCCESS;
}

void NetsysBpfNetFirewall::WriteSrcIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    std::vector<Ip4RuleBitmap> &srcIp4Map = manager.GetSrcIp4Map();
    if (srcIp4Map.empty()) {
        NETNATIVE_LOGE("WriteSrcIpv4BpfMap: srcIp4Map is empty");
        return;
    }
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    for (const auto &node : srcIp4Map) {
        Bitmap val = node.bitmap;
        RuleCode rule;
        memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));

        Ipv4LpmKey key = { 0 };
        key.prefixlen = node.mask;
        key.data = static_cast<Ip4Key>(node.data);
        WriteBpfMap(GET_MAP_PATH(ingress, saddr), key, rule);
    }
}

void NetsysBpfNetFirewall::WriteSrcIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    std::vector<Ip6RuleBitmap> &srcIp6Map = manager.GetSrcIp6Map();
    if (srcIp6Map.empty()) {
        NETNATIVE_LOGE("WriteSrcIpv6BpfMap: srcIp6Map is empty");
        return;
    }
    bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
    for (const auto &node : srcIp6Map) {
        Bitmap val = node.bitmap;
        RuleCode rule;
        memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));

        Ipv6LpmKey key = { 0 };
        key.prefixlen = node.prefixlen;
        key.data = static_cast<Ip6Key>(node.data);
        WriteBpfMap(GET_MAP_PATH(ingress, saddr6), key, rule);
    }
}

void NetsysBpfNetFirewall::WriteDstIpv4BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    std::vector<Ip4RuleBitmap> &dstIp4Map = manager.GetDstIp4Map();
    if (dstIp4Map.empty()) {
        NETNATIVE_LOGE("WriteDstIp4BpfMap: dstIp4Map is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &node : dstIp4Map) {
            Bitmap val = node.bitmap;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));

            Ipv4LpmKey key = { 0 };
            key.prefixlen = node.mask;
            key.data = static_cast<Ip4Key>(node.data);
            WriteBpfMap(GET_MAP_PATH(ingress, daddr), key, rule);
        }
    }
}

void NetsysBpfNetFirewall::WriteDstIpv6BpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    std::vector<Ip6RuleBitmap> &dstIp6Map = manager.GetDstIp6Map();
    if (dstIp6Map.empty()) {
        NETNATIVE_LOGE("WriteDstIp6BpfMap: dstIp6Map is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &node : dstIp6Map) {
            Bitmap val = node.bitmap;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));

            Ipv6LpmKey key = { 0 };
            key.prefixlen = node.prefixlen;
            key.data = static_cast<Ip6Key>(node.data);
            WriteBpfMap(GET_MAP_PATH(ingress, daddr6), key, rule);
        }
    }
}

void NetsysBpfNetFirewall::WriteSrcPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    BpfPortMap &srcPortMap = manager.GetSrcPortMap();
    if (srcPortMap.Empty()) {
        NETNATIVE_LOGE("WriteSrcPortBpfMap: srcPortMap is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &pair : srcPortMap.Get()) {
            PortKey key = pair.first;
            Bitmap val = pair.second;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));
            NETNATIVE_LOG_D("sport_map=%{public}d", key);
            WriteBpfMap(GET_MAP_PATH(ingress, sport), key, rule);
        }
    }
}

void NetsysBpfNetFirewall::WriteDstPortBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    BpfPortMap &dstPortMap = manager.GetDstPortMap();
    if (dstPortMap.Empty()) {
        NETNATIVE_LOGE("WriteDstPortBpfMap: dstPortMap is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &pair : dstPortMap.Get()) {
            PortKey key = pair.first;
            Bitmap val = pair.second;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));
            NETNATIVE_LOG_D("dport_map=%{public}d", key);
            WriteBpfMap(GET_MAP_PATH(ingress, dport), key, rule);
        }
    }
}

void NetsysBpfNetFirewall::WriteProtoBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    BpfProtoMap &protoMap = manager.GetProtoMap();
    if (protoMap.Empty()) {
        NETNATIVE_LOGE("WriteProtoBpfMap: protoMap is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &pair : protoMap.Get()) {
            ProtoKey key = pair.first;
            Bitmap val = pair.second;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));
            NETNATIVE_LOG_D("proto_map=%{public}d", key);
            WriteBpfMap(GET_MAP_PATH(ingress, proto), key, rule);
        }
    }
}

void NetsysBpfNetFirewall::WriteAppUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    BpfAppUidMap &appIdMap = manager.GetAppIdMap();
    if (appIdMap.Empty()) {
        NETNATIVE_LOGE("WriteAppUidBpfMap: appIdMap is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &pair : appIdMap.Get()) {
            AppUidKey key = pair.first;
            Bitmap val = pair.second;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));
            NETNATIVE_LOG_D("appuid_map=%{public}d", key);
            WriteBpfMap(GET_MAP_PATH(ingress, appuid), key, rule);
        }
    }
}

void NetsysBpfNetFirewall::WriteUidBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    BpfUidMap &uidMap = manager.GetUidMap();
    if (uidMap.Empty()) {
        NETNATIVE_LOGE("WriteUidBpfMap: uidMap is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &pair : uidMap.Get()) {
            UidKey key = pair.first;
            Bitmap val = pair.second;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));
            NETNATIVE_LOG_D("uidMap=%{public}d", key);
            WriteBpfMap(GET_MAP_PATH(ingress, uid), key, rule);
        }
    }
}

void NetsysBpfNetFirewall::WriteActionBpfMap(BitmapManager &manager, NetFirewallRuleDirection direction)
{
    BpfActionMap &actionMap = manager.GetActionMap();
    if (actionMap.Empty()) {
        NETNATIVE_LOGE("WriteActionBpfMap: actionMap is empty");
    } else {
        bool ingress = (direction == NetFirewallRuleDirection::RULE_IN);
        for (const auto &pair : actionMap.Get()) {
            ActionKey key = pair.first;
            Bitmap val = pair.second;
            RuleCode rule;
            memcpy_s(rule.val, sizeof(RuleCode), val.Get(), sizeof(RuleCode));
            NETNATIVE_LOG_D("action_map=%{public}d", val.Get()[0]);
            WriteBpfMap(GET_MAP_PATH(ingress, action), key, rule);
        }
    }
}

int32_t NetsysBpfNetFirewall::RegisterCallback(const sptr<NetsysNative::INetFirewallCallback> &callback)
{
    if (!callback) {
        return -1;
    }

    callbacks_.emplace_back(callback);

    return 0;
}
int32_t NetsysBpfNetFirewall::UnregisterCallback(const sptr<NetsysNative::INetFirewallCallback> &callback)
{
    if (!callback) {
        return -1;
    }

    for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it) {
        if (*it == callback) {
            callbacks_.erase(it);
            return 0;
        }
    }
    return -1;
}

bool NetsysBpfNetFirewall::ShouldSkipNotify(sptr<InterceptRecord> record)
{
    if (!record) {
        return true;
    }
    if (oldRecord_ != nullptr && (record->time - oldRecord_->time) < INTERCEPT_BUFF_INTERVAL_SEC) {
        if (record->localIp == oldRecord_->localIp && record->remoteIp == oldRecord_->remoteIp &&
            record->localPort == oldRecord_->localPort && record->remotePort == oldRecord_->remotePort &&
            record->protocol == oldRecord_->protocol && record->appUid == oldRecord_->appUid) {
            return true;
        }
    }
    oldRecord_ = record;
    return false;
}

void NetsysBpfNetFirewall::NotifyInterceptEvent(InterceptEvent *info)
{
    if (!info) {
        return;
    }
    sptr<InterceptRecord> record = new (std::nothrow) InterceptRecord();
    record->time = (int32_t)time(NULL);
    record->localPort = BitmapManager::Nstohl(info->sport);
    record->remotePort = BitmapManager::Nstohl(info->dport);
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
    } else {
        record->localIp = dstIp;
        record->remoteIp = srcIp;
    }
    if (ShouldSkipNotify(record)) {
        return;
    }
    for (auto callback : callbacks_) {
        callback->OnIntercept(record);
    }
}

void NetsysBpfNetFirewall::HandleTupleEvent(TupleEvent *ev)
{
    NETNATIVE_LOG_D(
        "%{public}s tuple: sport=%{public}u dport=%{public}u protocol=%{public}u appuid=%{public}u uid=%{public}u",
        (ev->dir == INGRESS) ? "> ingress" : "< egress", ntohs(ev->sport), ntohs(ev->dport), ev->protocol, ev->appuid,
        ev->uid);
    if (ev->family == AF_INET) {
        in_addr in;
        in.s_addr = ev->ipv4.saddr;
        NETNATIVE_LOG_D("\tsaddr=%{public}s", inet_ntoa(in));

        in.s_addr = ev->ipv4.daddr;
        NETNATIVE_LOG_D("\tdaddr=%{public}s", inet_ntoa(in));
    } else {
        char buf[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, &(ev->ipv6.saddr), buf, INET6_ADDRSTRLEN);
        NETNATIVE_LOG_D("\tsaddr6=%{public}s", buf);

        memset_s(buf, INET6_ADDRSTRLEN, 0, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &(ev->ipv6.daddr), buf, INET6_ADDRSTRLEN);
        NETNATIVE_LOG_D("\tdaddr6=%{public}s", buf);
    }
    NETNATIVE_LOG_D("\trstpacket=%{public}u", ev->rst);
}

void NetsysBpfNetFirewall::HandleInterceptEvent(InterceptEvent *ev)
{
    GetInstance()->NotifyInterceptEvent(ev);

    NETNATIVE_LOGI("%{public}s intercept: sport=%{public}u dport=%{public}u protocol=%{public}u appuid=%{public}u",
        (ev->dir == INGRESS) ? "ingress" : "egress", ntohs(ev->sport), ntohs(ev->dport), ev->protocol, ev->appuid);
    if (ev->family == AF_INET) {
        in_addr in;
        in.s_addr = ev->ipv4.saddr;
        NETNATIVE_LOGI("\tsaddr=%{public}s", inet_ntoa(in));

        in.s_addr = ev->ipv4.daddr;
        NETNATIVE_LOGI("\tdaddr=%{public}s", inet_ntoa(in));
    } else {
        char buf[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, &(ev->ipv6.saddr), buf, INET6_ADDRSTRLEN);
        NETNATIVE_LOGI("\tsaddr6=%{public}s", buf);

        memset_s(buf, INET6_ADDRSTRLEN, 0, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &(ev->ipv6.daddr), buf, INET6_ADDRSTRLEN);
        NETNATIVE_LOGI("\tdaddr6=%{public}s", buf);
    }
}

void NetsysBpfNetFirewall::HandleDebugEvent(DebugEvent *ev)
{
    const char *direction = ev->dir == INGRESS ? ">" : "<";
    switch (ev->type) {
        case DBG_LOOKUP_FAIL:
            NETNATIVE_LOG_D("bpf map lookup: fail");
            break;
        case DBG_MATCH_SADDR: {
            in_addr in;
            in.s_addr = ev->arg1;
            NETNATIVE_LOG_D("%{public}s saddr: %{public}s bitmap: %{public}x", direction, inet_ntoa(in), ev->arg2);
            break;
        }
        case DBG_MATCH_DADDR: {
            in_addr in;
            in.s_addr = ev->arg1;
            NETNATIVE_LOG_D("%{public}s daddr: %{public}s bitmap: %{public}x", direction, inet_ntoa(in), ev->arg2);
            break;
        }
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
        default:
            break;
    }
}

int NetsysBpfNetFirewall::HandleEvent(void *ctx, void *data, size_t len)
{
    if (data && len > 0) {
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

void OnDemandLoadManagerCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    NETNATIVE_LOG_D("OnLoadSystemAbilitySuccess systemAbilityId: [%{public}d]", systemAbilityId);
}

void OnDemandLoadManagerCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    NETNATIVE_LOG_D("OnLoadSystemAbilityFail: [%{public}d]", systemAbilityId);
}

int32_t NetsysBpfNetFirewall::LoadSystemAbility(int32_t systemAbilityId)
{
    NETNATIVE_LOG_D("LoadSystemAbility: [%{public}d]", systemAbilityId);
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        NETNATIVE_LOGE("GetCmProxy registry is null");
        return -1;
    }

    auto object = saManager->CheckSystemAbility(systemAbilityId);
    if (object != nullptr) {
        return 0;
    }

    sptr<OnDemandLoadManagerCallback> loadCallBack = new (std::nothrow) OnDemandLoadManagerCallback();
    if (loadCallBack == nullptr) {
        NETNATIVE_LOGE("new OnDemandLoadCertManagerCallback failed");
        return -1;
    }

    int32_t ret = saManager->LoadSystemAbility(systemAbilityId, loadCallBack);
    if (ret != ERR_OK) {
        NETNATIVE_LOGE("systemAbilityId:%d load failed,result code:%d", systemAbilityId, ret);
        return -1;
    }
    return 0;
}

void NetsysBpfNetFirewall::AddDomainCache(const NetAddrInfo &addrInfo)
{
    NETNATIVE_LOGI("AddDomainCache");
    domain_value value = 1;
    if (addrInfo.aiFamily == AF_INET) {
        Ipv4LpmKey key = { 0 };
        key.prefixlen = IPV4_MAX_PREFIXLEN;
        key.data = addrInfo.aiAddr.sin.s_addr;
        WriteBpfMap(MAP_PATH(DOMAIN_IPV4_MAP), key, value);
    } else {
        Ipv6LpmKey key = { 0 };
        key.prefixlen = IPV6_MAX_PREFIXLEN;
        key.data = addrInfo.aiAddr.sin6;
        WriteBpfMap(MAP_PATH(DOMAIN_IPV6_MAP), key, value);
    }
}

void NetsysBpfNetFirewall::ClearDomainCache()
{
    NETNATIVE_LOG_D("ClearDomainCache");
    Ipv4LpmKey ip4Key = {};
    Ipv6LpmKey ip6Key = {};
    domain_value value;
    ClearBpfMap(MAP_PATH(DOMAIN_IPV4_MAP), ip4Key, value);
    ClearBpfMap(MAP_PATH(DOMAIN_IPV6_MAP), ip6Key, value);
}
} // namespace NetManagerStandard
} // namespace OHOS