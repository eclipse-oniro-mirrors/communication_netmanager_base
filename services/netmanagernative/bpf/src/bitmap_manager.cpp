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

#include "bitmap_manager.h"

#include <arpa/inet.h>
#include <cstdio>
#include <netdb.h>
#include <net/if.h>
#include <securec.h>
#include <string>
#include <sys/socket.h>
#include <vector>

#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "netsys_net_dns_result_data.h"

using namespace OHOS::NetsysNative;

namespace OHOS {
namespace NetManagerStandard {
const uint32_t BITMAP_BIT_COUNT = (BITMAP_LEN * sizeof(uint32_t) * BIT_PER_BYTE);
static constexpr const uint32_t IPV4_MAX_VAL = 0xFFFFFFFF;

Bitmap::Bitmap()
{
    Clear();
}

Bitmap::Bitmap(uint32_t n)
{
    Clear();
    Set(n);
}

Bitmap::Bitmap(const Bitmap &other)
{
    memcpy_s(bitmap_, sizeof(bitmap_), other.bitmap_, sizeof(bitmap_));
}

void Bitmap::Clear()
{
    memset_s(bitmap_, sizeof(bitmap_), 0, sizeof(bitmap_));
}

const int BIT_OFFSET = 5;       // n/32
const int BIT_REMAINDER = 0x1f; // n%32
void Bitmap::Set(uint32_t n)
{
    if (n < BITMAP_BIT_COUNT) {
        int32_t i = n >> BIT_OFFSET;
        int32_t j = n & BIT_REMAINDER;
        bitmap_[i] |= 1 << j;
    }
}

uint64_t Bitmap::SpecialHash() const
{
    uint64_t h = 0;
    for (int32_t i = 0; i < BITMAP_LEN; i++) {
        if (bitmap_[i]) {
            h = GetHash(bitmap_[i]);
            h = h << i;
            return h;
        }
    }
    return h;
}

void Bitmap::And(const Bitmap &other)
{
    for (int32_t i = 0; i < BITMAP_LEN; i++) {
        bitmap_[i] &= other.bitmap_[i];
    }
}

void Bitmap::Or(const Bitmap &other)
{
    for (int32_t i = 0; i < BITMAP_LEN; i++) {
        bitmap_[i] |= other.bitmap_[i];
    }
}

bool Bitmap::operator == (const Bitmap &other) const
{
    return (memcmp(bitmap_, other.bitmap_, sizeof(bitmap_)) == 0);
}

Bitmap &Bitmap::operator = (const Bitmap &other)
{
    memcpy_s(bitmap_, sizeof(bitmap_), other.bitmap_, sizeof(bitmap_));
    return *this;
}

// Thomas Wang's 32 bit Mix Function
uint32_t Bitmap::GetHash(uint32_t key) const
{
    const int THOMAS_INDEX_0 = 15;
    const int THOMAS_INDEX_1 = 10;
    const int THOMAS_INDEX_2 = 3;
    const int THOMAS_INDEX_3 = 6;
    const int THOMAS_INDEX_4 = 11;
    const int THOMAS_INDEX_5 = 16;
    key += ~(key << THOMAS_INDEX_0);
    key ^= (key >> THOMAS_INDEX_1);
    key += (key << THOMAS_INDEX_2);
    key ^= (key >> THOMAS_INDEX_3);
    key += ~(key << THOMAS_INDEX_4);
    key ^= (key >> THOMAS_INDEX_5);
    return key;
}

uint32_t *Bitmap::Get()
{
    return bitmap_;
}

void Ip4RuleMap::OrForEach(const Bitmap &bitmap)
{
    for (auto &item : ruleBitmapVec_) {
        item.bitmap.Or(bitmap);
    }
}

uint32_t Ip4RuleMap::GetNetworkAddress(uint32_t addr, uint32_t mask)
{
    if (mask == 0) {
        return 0;
    }
    return addr & (0xFFFFFFFF << (IPV4_MAX_PREFIXLEN - mask));
}

void Ip4RuleMap::CollectCandidates(Ip4RuleInsertContext &ctx, std::vector<size_t> &candidates, bool &shouldInsert)
{
    auto exactIt = exactIndex_.find(ctx.exactKey);
    if (exactIt != exactIndex_.end()) {
        candidates.emplace_back(exactIt->second);
        shouldInsert = false;
    }
    for (uint32_t prefix = 0; prefix < ctx.mask; ++prefix) {
        uint32_t net = GetNetworkAddress(ctx.addr, prefix);
        auto it = byPrefix_[prefix].find(net);
        if (it != byPrefix_[prefix].end()) {
            candidates.insert(candidates.end(), it->second.begin(), it->second.end());
        }
    }
    auto containedIt = containedAt_[ctx.mask].find(ctx.networkAddr);
    if (containedIt != containedAt_[ctx.mask].end()) {
        for (size_t index : containedIt->second) {
            if (ruleBitmapVec_[index].mask > ctx.mask) {
                candidates.emplace_back(index);
            }
        }
    }
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
}

void Ip4RuleMap::MergeCandidates(
    Ip4RuleInsertContext &ctx, Bitmap &bitmapCopy, const std::vector<size_t> &candidates, bool &shouldInsert)
{
    for (size_t index : candidates) {
        auto &item = ruleBitmapVec_[index];
        uint32_t existingIp = ntohl(item.data);
        uint32_t existingNetworkAddr = GetNetworkAddress(existingIp, item.mask);
        if (item.data == ctx.netOrderAddr && item.mask == ctx.mask) {
            item.bitmap.Or(bitmapCopy);
            shouldInsert = false;
        }
        if (ctx.mask < item.mask && GetNetworkAddress(existingIp, ctx.mask) == ctx.networkAddr) {
            item.bitmap.Or(bitmapCopy);
        }
        if (ctx.mask > item.mask && GetNetworkAddress(existingIp, item.mask) == existingNetworkAddr) {
            bitmapCopy.Or(item.bitmap);
        }
    }
}

void Ip4RuleMap::InsertRuleBitmap(Ip4RuleInsertContext &ctx, Bitmap &bitmapCopy)
{
    Ip4RuleBitmap ruleBitmap;
    ruleBitmap.data = ctx.netOrderAddr;
    ruleBitmap.mask = ctx.mask;
    ruleBitmap.bitmap = std::move(bitmapCopy);
    ruleBitmapVec_.emplace_back(std::move(ruleBitmap));
    size_t index = ruleBitmapVec_.size() - 1;
    exactIndex_[ctx.exactKey] = index;
    byPrefix_[ctx.mask][ctx.networkAddr].emplace_back(index);
    for (uint32_t prefix = 0; prefix <= ctx.mask; ++prefix) {
        containedAt_[prefix][GetNetworkAddress(ctx.addr, prefix)].emplace_back(index);
    }
}

void Ip4RuleMap::OrInsert(uint32_t addr, uint32_t mask, Bitmap &bitmap)
{
    if (mask > IPV4_MAX_PREFIXLEN) {
        mask = IPV4_MAX_PREFIXLEN;
    }
    uint32_t netOrderAddr = htonl(addr);
    uint32_tnetworkAddr = GetNetworkAddress(addr, mask);
    bool shouldInsert = true;
    Bitmap bitmapCopy(bitmap);
    
    Ip4RuleInsertContext ctx = {
        addr,
        mask,
        netOrderAddr,
        networkAddr,
        (static_cast<uint64_t>(mask) << 32) | netOrderAddr,
    };

    std::vector<size_t> candidates;
    CollectCandidates(ctx, candidates, shouldInsert);
    MergeCandidates(ctx, bitmapCopy, candidates, shouldInsert);
    if (shouldInsert) {
        InsertRuleBitmap(ctx, bitmapCopy);
    }
}

void Ip4RuleMap::Clear()
{
    ruleBitmapVec_.clear();
    exactIndex_.clear();
    for (auto &bucket : byPrefix_) {
        bucket.clear();
    }
    for (auto &bucket : containedAt_) {
        bucket.clear();
    }
}

std::vector<Ip4RuleBitmap> &Ip4RuleMap::Get()
{
    return ruleBitmapVec_;
}

void Ip6RuleMap::OrForEach(const Bitmap &bitmap)
{
    for (auto &item : ruleBitmapVec_) {
        item.bitmap.Or(bitmap);
    }
}

void Ip6RuleMap::GetNetworkAddress(in6_addr addr, int prefixLen, in6_addr &out)
{
    if (prefixLen < 0 || prefixLen > IPV6_MAX_PREFIXLEN) {
        return;
    }
    if (prefixLen == 0) {
        return;
    }
    int quotient = prefixLen / 8;
    int remainder = prefixLen % 8;
    for (int i = 0; i < quotient; i++) {
        out.s6_addr[i] = addr.s6_addr[i] & 0xff;
    }
    if (remainder) {
        out.s6_addr[quotient] = addr.s6_addr[quotient] & (~(0xff >> remainder));
    }
}

static Ip6ExactKey MakeIp6ExactKey(const in6_addr &networkAddr, uint32_t prefixlen)
{
    Ip6ExactKey key;
    key.prefixlen = prefixlen;
    if (memcpy_s(key.net.data, sizeof(key.net.data), networkAddr.s6_addr, sizeof(networkAddr.s6_addr)) != EOK) {
        return key;
    }
    return key;
}

void Ip6RuleMap::OrInsert(const in6_addr &addr, uint32_t prefixlen, Bitmap &bitmap)
{
    if (prefixlen > IPV6_MAX_PREFIXLEN) {
        prefixlen = IPV6_MAX_PREFIXLEN;
    }
    in6_addr networkAddr = {};
    GetNetworkAddress(addr, static_cast<int>(prefixlen), networkAddr);
    Ip6ExactKey key = MakeIp6ExactKey(networkAddr, prefixlen);
    auto it = exactIndex_.find(key);
    if (it != exactIndex_.end()) {
        ruleBitmapVec_[it->second].bitmap.Or(bitmap);
        return;
    }
    Ip6RuleBitmap ruleBitmap;
    ruleBitmap.data = networkAddr;
    ruleBitmap.prefixlen = prefixlen;
    ruleBitmap.bitmap = bitmap;
    ruleBitmapVec_.emplace_back(std::move(ruleBitmap));
    exactIndex_[key] = ruleBitmapVec_.size() - 1;
    if (!activePrefix_[prefixlen]) {
        activePrefix_[prefixlen] = true;
        ++activePrefixCount_;
    }
    finalized_ = false;
}

void Ip6RuleMap::Finalize()
{
    if (finalized_) {
        return;
    }
    if (activePrefixCount_ <= 1) {
        finalized_ = true;
        return;
    }
    for (auto &item : ruleBitmapVec_) {
        for (uint32_t prefix = 0; prefix < item.prefixlen; ++prefix) {
            if (!activePrefix_[prefix]) {
                continue;
            }
            in6_addr ancestorNet = {};
            GetNetworkAddress(item.data, static_cast<int>(prefix), ancestorNet);
            auto it = exactIndex_.find(MakeIp6ExactKey(ancestorNet, prefix));
            if (it != exactIndex_.end()) {
                item.bitmap.Or(ruleBitmapVec_[it->second].bitmap);
            }
        }
    }
    finalized_ = true;
}

void Ip6RuleMap::Clear()
{
    ruleBitmapVec_.clear();
    exactIndex_.clear();
    activePrefix_.fill(false);
    activePrefixCount_ = 0;
    finalized_ = false;
}

std::vector<Ip6RuleBitmap> &Ip6RuleMap::Get()
{
    Finalize();
    return ruleBitmapVec_;
}

void BpfPortMap::OrForEach(const Bitmap &bitmap)
{
    for (auto &item : ruleBitmapVec_) {
        item.bitmap.Or(bitmap);
    }
}

void BpfPortMap::OrInsert(uint16_t start, uint32_t mask, Bitmap &bitmap)
{
    uint64_t key = (static_cast<uint64_t>(mask) << 16) | start;
    auto it = index_.find(key);
    if (it == index_.end()) {
        portRuleBitmap tmp;
        tmp.prefixlen = mask;
        tmp.data = start;
        tmp.bitmap = bitmap;
        size_t index = ruleBitmapVec_.size();
        ruleBitmapVec_.push_back(std::move(tmp));
        index_[key] = index;
    } else {
        ruleBitmapVec_[it->second].bitmap.Or(bitmap);
    }
}

void BpfPortMap::Clear()
{
    ruleBitmapVec_.clear();
    index_.clear();
}

std::vector<portRuleBitmap> &BpfPortMap::Get()
{
    return ruleBitmapVec_;
}

void SegmentBitmapMap::AddMap(uint16_t start, uint16_t end, const Bitmap& bitmap)
{
    if (start > end) {
        return;
    }
    std::vector<size_t> intersectIndices;
    CollectIntersectIndices(start, end, intersectIndices);

    std::vector<SegmentBitmap> newSegments;
    ProcessIntersections(start, end, bitmap, intersectIndices, newSegments);

    RemoveSegments(intersectIndices);
    InsertNewSegments(newSegments);
}

std::vector<SegmentBitmap>& SegmentBitmapMap::GetMap()
{
    return segments_;
}

void SegmentBitmapMap::CollectIntersectIndices(uint16_t start, uint16_t end, std::vector<size_t>& indices) const
{
    for (size_t i = 0; i < segments_.size(); ++i) {
        const auto& seg = segments_[i];
        if (!(end < seg.start || start > seg.end)) {
            indices.push_back(i);
        }
    }
}

void SegmentBitmapMap::RemoveSegments(const std::vector<size_t>& indices)
{
    std::vector<size_t> sortedIndices = indices;
    std::sort(sortedIndices.rbegin(), sortedIndices.rend());

    for (size_t idx : sortedIndices) {
        if (idx < segments_.size()) {
            segments_.erase(segments_.begin() + idx);
        }
    }
}

void SegmentBitmapMap::ProcessIntersections(uint16_t targetStart, uint16_t targetEnd, const Bitmap& targetBitmap,
                                            const std::vector<size_t>& intersectIndices,
                                            std::vector<SegmentBitmap>& newSegments)
{
    std::vector<SegmentBitmap> intersectSegs;
    for (size_t idx : intersectIndices) {
        intersectSegs.push_back(segments_[idx]);
    }
    std::sort(intersectSegs.begin(), intersectSegs.end(),
              [](const SegmentBitmap& a, const SegmentBitmap& b) {
                  return a.start < b.start;
              });

    uint16_t currentPos = targetStart;
    for (const auto& seg : intersectSegs) {
        if (currentPos < seg.start) {
            addSegment(currentPos, seg.start - 1, targetBitmap, newSegments);
            currentPos = seg.start;
        }

        if (seg.start < currentPos) {
            addSegment(seg.start, currentPos - 1, seg.bitmap, newSegments);
        }

        uint16_t overlapEnd = std::min(targetEnd, seg.end);
        Bitmap mergedBitmap = seg.bitmap;
        mergedBitmap.Or(targetBitmap);
        addSegment(currentPos, overlapEnd, mergedBitmap, newSegments);

        if (overlapEnd == UINT16_MAX) {
            return;
        }

        currentPos = overlapEnd + 1;

        if (currentPos <= seg.end) {
            addSegment(currentPos, seg.end, seg.bitmap, newSegments);
        }

        if (currentPos > targetEnd) {
            break;
        }
    }

    if (currentPos <= targetEnd) {
        addSegment(currentPos, targetEnd, targetBitmap, newSegments);
    }
}

void SegmentBitmapMap::addSegment(uint16_t start, uint16_t end, const Bitmap& bitmap,
                                  std::vector<SegmentBitmap>& segments)
{
    if (start > end) {
        return;
    }
    segments.push_back({start, end, bitmap});
}

void SegmentBitmapMap::InsertNewSegments(const std::vector<SegmentBitmap>& newSegments)
{
    for (const auto& seg : newSegments) {
        if (!seg.isValid()) {
            continue;
        }
        auto it = std::lower_bound(segments_.begin(), segments_.end(), seg,
            [](const SegmentBitmap& a, const SegmentBitmap& b) {
                return a.start < b.start;
            });
        segments_.insert(it, seg);
    }
}

uint16_t BitmapManager::Hltons(uint32_t n)
{
    return htons((uint16_t)(n & 0x0000ffff));
}

uint16_t BitmapManager::Nstohl(uint16_t n)
{
    uint16_t m = 0;
    return m | ntohs(n);
}

int32_t BitmapManager::BuildBitmapMap(const std::vector<sptr<NetFirewallIpRule>> &ruleList)
{
    Clear();
    int32_t ret = BuildMarkBitmap(ruleList);
    if (ret != NETFIREWALL_SUCCESS) {
        return ret;
    }

    Insert();
    BuildNoMarkBitmap(ruleList);
    return NETFIREWALL_SUCCESS;
}

void BitmapManager::Insert()
{
    std::lock_guard<std::mutex> guard(mutex_);
    Bitmap bitmap;
    srcIp4Map_.OrInsert(OTHER_IP4_KEY, IPV4_MAX_PREFIXLEN, bitmap);
    dstIp4Map_.OrInsert(OTHER_IP4_KEY, IPV4_MAX_PREFIXLEN, bitmap);
    in6_addr otherIp6Key;
    memset_s(&otherIp6Key, sizeof(in6_addr), 0xff, sizeof(in6_addr));
    srcIp6Map_.OrInsert(otherIp6Key, IPV6_MAX_PREFIXLEN, bitmap);
    dstIp6Map_.OrInsert(otherIp6Key, IPV6_MAX_PREFIXLEN, bitmap);
    srcPortMap_.OrInsert(OTHER_PORT_KEY, PORT_MAX_MASK, bitmap);
    dstPortMap_.OrInsert(OTHER_PORT_KEY, PORT_MAX_MASK, bitmap);
    protoMap_.OrInsert(OTHER_PROTO_KEY, Bitmap());
    appUidMap_.OrInsert(OTHER_APPUID_KEY, Bitmap());
    uidMap_.OrInsert(OTHER_UID_KEY, Bitmap());
    action_key Key = 1;
    actionMap_.OrInsert(Key, Bitmap());
    interfaceMap_.OrInsert(std::string(""), Bitmap());
}

void BitmapManager::Clear()
{
    std::lock_guard<std::mutex> guard(mutex_);
    srcIp4Map_.Clear();
    srcIp6Map_.Clear();
    dstIp4Map_.Clear();
    dstIp6Map_.Clear();
    srcPortMap_.Clear();
    dstPortMap_.Clear();
    protoMap_.Clear();
    appUidMap_.Clear();
    uidMap_.Clear();
    actionMap_.Clear();
    interfaceMap_.Clear();
}

int32_t BitmapManager::InsertIp4SegBitmap(const NetFirewallIpParam &item, Bitmap &bitmap, Ip4RuleMap *ip4Map)
{
    if (ip4Map == nullptr) {
        return NETFIREWALL_ERR;
    }
    if (item.type == SINGLE_IP) {
        if (item.mask > IPV4_MAX_PREFIXLEN) {
            NETNATIVE_LOGE("InsertIp4SegBitmap: invalid mask %{public}u", item.mask);
            return NETFIREWALL_ERR;
        }
        uint32_t ipInt = item.ipv4.startIp.s_addr;
        ip4Map->OrInsert(ntohl(ipInt), static_cast<uint32_t>(item.mask), bitmap);
        NETNATIVE_LOG_D("InsertIpBitmap ipp[%{public}u] ipn[%{public}s] mask[%{public}u]", ipInt,
            item.GetStartIp().c_str(), item.mask);
    } else if (item.type == MULTIPLE_IP) {
        std::vector<Ip4Data> ips;
        int32_t ret = IpParamParser::GetIp4AndMask(item.ipv4.startIp, item.ipv4.endIp, ips);
        if (ret != NETFIREWALL_SUCCESS) {
            return ret;
        }
        for (auto &ipData : ips) {
            ip4Map->OrInsert(ipData.data, ipData.mask, bitmap);
            NETNATIVE_LOG_D("InsertIpBitmap ip[%{public}u], mask[%{public}u]", htonl(ipData.data), ipData.mask);
        }
    }
    return NETFIREWALL_SUCCESS;
}


int32_t BitmapManager::InsertIp6SegBitmap(const NetFirewallIpParam &item, Bitmap &bitmap, Ip6RuleMap *ip6Map)
{
    if (ip6Map == nullptr) {
        return NETFIREWALL_ERR;
    }
    if (item.type == SINGLE_IP) {
        if (item.mask > IPV6_MAX_PREFIXLEN) {
            NETNATIVE_LOGE("InsertIp6SegBitmap: invalid mask %{public}u", item.mask);
            return NETFIREWALL_ERR;
        }
        ip6Map->OrInsert(item.ipv6.startIp, static_cast<uint32_t>(item.mask), bitmap);
        std::string addrStr = IpParamParser::Addr6ToStr(item.ipv6.startIp);
        NETNATIVE_LOG_D("InsertIp6SegBitmap ip[%{public}s], mask[%{public}u]",
            CommonUtils::ToAnonymousIp(addrStr).c_str(), item.mask);
    } else if (item.type == MULTIPLE_IP) {
        std::vector<Ip6Data> ips;
        int32_t ret = IpParamParser::GetIp6AndMask(item.ipv6.startIp, item.ipv6.endIp, ips);
        if (ret != NETFIREWALL_SUCCESS) {
            NETNATIVE_LOGW("InsertIp6SegBitmap GetIp6AndMask fail ret=%{public}d", ret);
            return ret;
        }
        for (auto &ipData : ips) {
            ip6Map->OrInsert(ipData.data, ipData.prefixlen, bitmap);
        }
    }
    return NETFIREWALL_SUCCESS;
}

int32_t BitmapManager::InsertIpBitmap(const std::vector<NetFirewallIpParam> &ipInfo, bool isSrc, Bitmap &bitmap)
{
    for (const NetFirewallIpParam &item : ipInfo) {
        Ip4RuleMap *ip4Map = nullptr;
        Ip6RuleMap *ip6Map = nullptr;
        if (isSrc) {
            if (item.family == FAMILY_IPV4) {
                ip4Map = &srcIp4Map_;
            } else if (item.family == FAMILY_IPV6) {
                ip6Map = &srcIp6Map_;
            }
        } else {
            if (item.family == FAMILY_IPV4) {
                ip4Map = &dstIp4Map_;
            } else if (item.family == FAMILY_IPV6) {
                ip6Map = &dstIp6Map_;
            }
        }

        int32_t ret = NETFIREWALL_SUCCESS;
        if (item.family == FAMILY_IPV4) {
            int32_t ret = InsertIp4SegBitmap(item, bitmap, ip4Map);
            if (ret != NETFIREWALL_SUCCESS) {
                return ret;
            }
        } else {
            ret = InsertIp6SegBitmap(item, bitmap, ip6Map);
            if (ret != NETFIREWALL_SUCCESS) {
                return ret;
            }
        }
    }
    return NETFIREWALL_SUCCESS;
}

void BitmapManager::OrInsertPortBitmap(SegmentBitmapMap &portSegMap, BpfPortMap &portMap)
{
    auto &segMap = portSegMap.GetMap();
    for (auto &item : segMap) {
        uint16_t keyStart = item.start;
        uint16_t keyEnd = item.end;
        NETNATIVE_LOG_D("Segment: start=%{public}u(0x%{public}04X), end=%{public}u(0x%{public}04X)",
            keyStart, keyStart, keyEnd, keyEnd);
        std::vector<portRule> portRules;
        int32_t ret = GetPortRangeAndMask(keyStart, keyEnd, portRules);
        if (ret != 0) {
            continue;
        }

        for (auto &rule : portRules) {
            if (rule.prefixlen > PORT_MAX_MASK) {
                continue;
            }
            uint16_t netOrderPort = htons(rule.data);
            NETNATIVE_LOG_D("OrInsert: port=0x%{public}04X(%{public}u)/%{public}u",
                            netOrderPort, ntohs(netOrderPort), rule.prefixlen);
            portMap.OrInsert(netOrderPort, static_cast<uint32_t>(rule.prefixlen), item.bitmap);
        }
    }
}

int32_t BitmapManager::GetPortRangeAndMask(uint16_t startPort, uint16_t endPort, std::vector<portRule> &list)
{
    if (startPort > endPort) {
        return -1;
    }
    uint32_t start = static_cast<uint32_t>(startPort);
    uint32_t end = static_cast<uint32_t>(endPort);
    while (start <= end) {
        uint32_t suffixZeros = 0;
        uint32_t temp = start;
        while (temp > 0 && (temp & 1) == 0) {
            suffixZeros++;
            temp >>= 1;
        }

        uint32_t maxBlock = 1U << suffixZeros;
        uint32_t remaining = end - start + 1;
        uint32_t blockSize = maxBlock;
        while (blockSize > remaining) {
            blockSize >>= 1;
        }

        uint32_t mask = PORT_MAX_MASK;
        uint32_t tempSize = blockSize;
        while (tempSize > 1) {
            mask--;
            tempSize >>= 1;
        }

        portRule rule;
        rule.data = static_cast<uint16_t>(start);
        rule.prefixlen = static_cast<uint16_t>(mask);
        list.push_back(rule);

        uint16_t blockEnd = rule.data + blockSize - 1;

        start += blockSize;
    }
    return 0;
}

void BitmapManager::AddPortBitmap(const std::vector<NetFirewallPortParam> &port, Bitmap &bitmap,
    SegmentBitmapMap &portMap)
{
    for (const NetFirewallPortParam &item : port) {
        uint16_t startPort = item.startPort;
        if (startPort == 0) {
            continue;
        }
        if (item.startPort <= item.endPort) {
            portMap.AddMap(item.startPort, item.endPort, bitmap);
        }
    }
}

int32_t BitmapManager::BuildMarkBitmap(const std::vector<sptr<NetFirewallIpRule>> &ruleList)
{
    SegmentBitmapMap srcPortMap;
    SegmentBitmapMap dstPortMap;
    uint32_t index = 0;
    int32_t ret;
    for (const auto &rule : ruleList) {
        if (rule == nullptr) {
            NETNATIVE_LOGW("BuildMarkBitmap: rule is nullptr, skip");
            continue;
        }
        Bitmap bitmap(index);
        ret = InsertIpBitmap(rule->remoteIps, true, bitmap);
        if (ret) {
            NETNATIVE_LOGW("BuildMarkBitmap InsertIpBitmap remoteIps fail ret=%{public}d", ret);
            return NETFIREWALL_ERR;
        }
        ret = InsertIpBitmap(rule->localIps, false, bitmap);
        if (ret) {
            NETNATIVE_LOGW("BuildMarkBitmap InsertIpBitmap localIps fail ret=%{public}d", ret);
            return NETFIREWALL_ERR;
        }
        if (!IsNotNeedPort(rule->protocol)) {
            AddPortBitmap(rule->remotePorts, bitmap, srcPortMap);
            AddPortBitmap(rule->localPorts, bitmap, dstPortMap);
        }

        if (rule->protocol != (NetworkProtocol)0) {
            ProtoKey proto = (ProtoKey)rule->protocol;
            protoMap_.OrInsert(proto, bitmap);
        }

        if (rule->appUid > 0) {
            appUidMap_.OrInsert((AppUidKey)rule->appUid, bitmap);
        }

        if (rule->userId > 0) {
            uidMap_.OrInsert((UidKey)rule->userId, bitmap);
        }

        if (rule->ruleAction != FirewallRuleAction::RULE_ALLOW) {
            action_key Key = 1;
            actionMap_.OrInsert(Key, bitmap);
        }

        if (!rule->interface.empty()) {
            interfaceMap_.OrInsert(rule->interface, bitmap);
        }
        index++;
    }

    OrInsertPortBitmap(srcPortMap, srcPortMap_);
    OrInsertPortBitmap(dstPortMap, dstPortMap_);
    return NETFIREWALL_SUCCESS;
}

void BitmapManager::BuildNoMarkBitmap(const std::vector<sptr<NetFirewallIpRule>> &ruleList)
{
    uint32_t index = 0;
    for (const auto &rule : ruleList) {
        Bitmap bitmap(index);
        if (rule->remoteIps.empty()) {
            srcIp4Map_.OrForEach(bitmap);
            srcIp6Map_.OrForEach(bitmap);
        }
        if (rule->localIps.empty()) {
            dstIp4Map_.OrForEach(bitmap);
            dstIp6Map_.OrForEach(bitmap);
        }
        if (rule->remotePorts.empty() || IsNotNeedPort(rule->protocol)) {
            srcPortMap_.OrForEach(bitmap);
        }
        if (rule->localPorts.empty() || IsNotNeedPort(rule->protocol)) {
            dstPortMap_.OrForEach(bitmap);
        }
        if (rule->protocol == (NetworkProtocol)0) {
            protoMap_.OrForEach(bitmap);
        }
        if (rule->appUid < 1) {
            appUidMap_.OrForEach(bitmap);
        }
        if (rule->userId < 1) {
            uidMap_.OrForEach(bitmap);
        }
        if (rule->interface.empty()) {
            interfaceMap_.OrForEach(bitmap);
        }
        index++;
    }
}

bool BitmapManager::IsNotNeedPort(NetworkProtocol protocol)
{
    if (protocol == NetworkProtocol::ICMPV6 || protocol == NetworkProtocol::ICMP) {
        return true;
    }
    return false;
}

int32_t IpParamParser::GetIpUint32(const std::string &address, uint32_t &ipInt)
{
    in_addr out = { 0 };
    memset_s(&out, sizeof(out), 0, sizeof(out));
    if (inet_pton(AF_INET, address.c_str(), &out) != 1) {
        return NETFIREWALL_IP_STR_ERR;
    }

    ipInt = out.s_addr;
    return NETFIREWALL_SUCCESS;
}

void IpParamParser::AddIp(uint32_t ip, uint32_t mask, std::vector<Ip4Data> &ip4Vec)
{
    Ip4Data info;
    info.data = ip;
    info.mask = mask;
    ip4Vec.emplace_back(info);
}

std::string IpParamParser::Ip4ToStr(uint32_t ip)
{
    char str[INET_ADDRSTRLEN] = {0};
    in_addr ipv4;
    memset_s(&ipv4, sizeof(ipv4), 0, sizeof(ipv4));
    ipv4.s_addr = ntohl(ip);

    if (inet_ntop(AF_INET, &ipv4, str, INET_ADDRSTRLEN) == NULL) {
        return "error ip";
    }
    return std::string(str);
}

uint32_t IpParamParser::GetCidrBlockBits(uint64_t cidrSize)
{
    uint32_t count = 0;
    while (cidrSize >>= 1) {
        ++count;
    }
    return count;
}
 
uint32_t IpParamParser::GetSuffixZeroLength(uint32_t ip)
{
    if (ip == 0) {
        return IPV4_BIT_COUNT;
    }
    uint32_t count = 0;
    while ((ip & 1) == 0) {
        ++count;
        ip >>= 1;
    }
    return count;
}

int32_t IpParamParser::GetIp4AndMask(const in_addr &startAddr, const in_addr &endAddr, std::vector<Ip4Data> &list)
{
    uint32_t startIpInt = ntohl(startAddr.s_addr);
    uint32_t endIpInt = ntohl(endAddr.s_addr);
    if (startIpInt > endIpInt) {
        return NETFIREWALL_ERR;
    }
    uint32_t suffixZeros = 0;
    uint64_t maxCidrSize = 0;
    uint64_t remainingCidrSize = 0;
    uint32_t cidrBits = 0;
    while (startIpInt <= endIpInt) {
        suffixZeros = GetSuffixZeroLength(startIpInt);
        maxCidrSize = (1ULL << suffixZeros);
        remainingCidrSize = static_cast<uint64_t>(endIpInt) - startIpInt + 1;
        cidrBits = GetCidrBlockBits(std::min(maxCidrSize, remainingCidrSize));
        AddIp(startIpInt, IPV4_BIT_COUNT - cidrBits, list);
        uint64_t nextStart = static_cast<uint64_t>(startIpInt) + (1ULL << cidrBits);
        if (nextStart > endIpInt) {
            break;
        }
        startIpInt = static_cast<uint32_t>(nextStart);
    }
    return NETFIREWALL_SUCCESS;
}

int32_t IpParamParser::GetInAddr6(const std::string &ipStr, in6_addr &addr)
{
    int32_t ret = inet_pton(AF_INET6, ipStr.c_str(), &addr);
    if (ret <= 0) {
        return NETFIREWALL_IP_STR_ERR;
    }
    return NETFIREWALL_SUCCESS;
}

std::string IpParamParser::Addr6ToStr(const in6_addr &v6Addr)
{
    char buf[INET6_ADDRSTRLEN] = { 0 };
    if (inet_ntop(AF_INET6, &v6Addr, buf, INET6_ADDRSTRLEN) == NULL) {
        return "error in6_addr";
    }
    return std::string(buf);
}

uint32_t IpParamParser::GetIp6Prefixlen(const in6_addr &start, const in6_addr &end)
{
    uint32_t prefixlen = IPV6_MAX_PREFIXLEN;
    for (uint32_t i = 0; i < IPV6_BYTE_COUNT; i++) {
        if (start.s6_addr[i] == end.s6_addr[i]) {
            continue;
        }
        for (int32_t j = static_cast<int32_t>(BIT_PER_BYTE - 1); j >= 0; --j) {
            uint8_t byte = (1 << j);
            if ((start.s6_addr[i] & byte) == (end.s6_addr[i] & byte)) {
                continue;
            } else {
                prefixlen = i * BIT_PER_BYTE + BIT_PER_BYTE - static_cast<uint32_t>(j) - 1;
                return prefixlen;
            }
        }
    }
    return prefixlen;
}

uint32_t IpParamParser::RfindIp6(const in6_addr &addr, uint32_t startBit, uint32_t endBit, uint8_t value)
{
    if (startBit > endBit) {
        return IPV6_BIT_COUNT;
    }
    uint32_t startBits = startBit % BIT_PER_BYTE;
    uint32_t startBytes = startBit / BIT_PER_BYTE;
    uint32_t endBits = endBit % BIT_PER_BYTE;
    uint32_t endBytes = endBit / BIT_PER_BYTE;
    for (uint32_t i = endBytes; i >= startBytes; --i) {
        uint32_t j = (i == endBytes) ? (BIT_PER_BYTE - endBits - 1) : 0;
        uint32_t k = (i == startBytes) ? (BIT_PER_BYTE - startBits - 1) : (BIT_PER_BYTE - 1);
        for (; j <= k; ++j) {
            uint8_t byte = (1 << j);
            uint8_t tmp = (addr.s6_addr[i] & byte) >> j;
            if (tmp == value) {
                return i * BIT_PER_BYTE + BIT_PER_BYTE - j - 1;
            }
        }
        if (i == startBytes) {
            break;
        }
    }
    return IPV6_BIT_COUNT;
}

uint32_t IpParamParser::FindIp6(const in6_addr &addr, uint32_t startBit, uint8_t value)
{
    if (startBit >= IPV6_BIT_COUNT) {
        return IPV6_BIT_COUNT;
    }
    uint32_t startBits = startBit % BIT_PER_BYTE;
    uint32_t startBytes = startBit / BIT_PER_BYTE;
    for (uint32_t i = startBytes; i < IPV6_BYTE_COUNT; ++i) {
        int32_t j = static_cast<int32_t>((i == startBytes) ? (BIT_PER_BYTE - startBits - 1) : (BIT_PER_BYTE - 1));
        for (; j >= 0; --j) {
            uint8_t tmp = ((addr.s6_addr[i] >> j) & VALUE_ONE);
            if (tmp == value) {
                return i * BIT_PER_BYTE + BIT_PER_BYTE - static_cast<uint32_t>(j) - 1;
            }
        }
    }
    return IPV6_BIT_COUNT;
}

void IpParamParser::AddIp6(const in6_addr &addr, uint32_t prefixlen, std::vector<Ip6Data> &list)
{
    Ip6Data info;
    info.prefixlen = prefixlen;
    info.data = addr;
    list.emplace_back(info);

    std::string startIpStr = IpParamParser::Addr6ToStr(info.data);
    NETNATIVE_LOG_D("AddIp6 ip[%{public}s], mask[%{public}u]",
        CommonUtils::ToAnonymousIp(startIpStr).c_str(), info.prefixlen);
}

void IpParamParser::ChangeIp6Start(uint32_t startBit, in6_addr &addr)
{
    uint32_t bits = startBit % BIT_PER_BYTE;
    uint32_t bytes = startBit / BIT_PER_BYTE;
    if (bytes < IPV6_BYTE_COUNT - 1) {
        memset_s(addr.s6_addr + bytes + 1, IPV6_BYTE_COUNT - bytes - 1, 0, IPV6_BYTE_COUNT - bytes - 1);
    }

    bool needSetZero = true;
    if (bytes >= IPV6_BYTE_COUNT) {
        bytes = IPV6_BYTE_COUNT - 1;
        bits = BIT_PER_BYTE - 1;
        needSetZero = false;
    }

    uint32_t off = BIT_PER_BYTE - bits - 1;
    for (int32_t i = static_cast<int32_t>(bytes); i >= 0; --i) {
        for (uint32_t j = 0; j < BIT_PER_BYTE; ++j) {
            uint8_t byte = (1 << j);
            if (needSetZero && (i == static_cast<int32_t>(bytes) && j <= off)) {
                addr.s6_addr[i] &= (~byte);
                continue;
            }
            if (addr.s6_addr[i] & byte) {
                addr.s6_addr[i] &= (~byte);
                continue;
            } else {
                addr.s6_addr[i] |= byte;
                return;
            }
        }
    }
}

namespace {
constexpr size_t IP6_RANGE_CACHE_MAX = 1024;

struct Ip6RangeKey {
    uint8_t start[IPV6_BYTE_COUNT] = {};
    uint8_t end[IPV6_BYTE_COUNT] = {};

    bool operator == (const Ip6RangeKey &other) const
    {
        return memcmp(start, other.start, sizeof(start)) == 0 &&
            memcmp(end, other.end, sizeof(end)) == 0;
    }
};

struct Ip6RangeKeyHash {
    size_t operator () (const Ip6RangeKey &key) const
    {
        size_t hash = FNV_OFFSET_BASIS_64;
        const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&key);
        for (size_t i = 0; i < sizeof(key.start) + sizeof(key.end); ++i) {
            hash ^= bytes[i];
            hash *= FNV_PRIME_64;
        }
        return hash;
    }
};

std::mutex g_ip6RangeCacheMutex;
std::unordered_map<Ip6RangeKey, std::vector<Ip6Data>, Ip6RangeKeyHash> g_ip6RangeCache;
} // namespace

static int32_t ComputeIp6AndMask(const in6_addr &addr6Start, const in6_addr &addr6End, std::vector<Ip6Data> &list)
{
    int32_t ret = memcmp(&addr6Start, &addr6End, sizeof(addr6Start));
    if (ret > 0) {
        NETNATIVE_LOGW("GetIp6AndMask fail ret=%{public}d", ret);
        return NETFIREWALL_ERR;
    } else if (ret == 0) {
        IpParamParser::AddIp6(addr6Start, IPV6_MAX_PREFIXLEN, list);
        return NETFIREWALL_SUCCESS;
    }
    uint32_t prefixlen = IpParamParser::GetIp6Prefixlen(addr6Start, addr6End);
    in6_addr tmpAddr;
    memcpy_s(&tmpAddr, sizeof(tmpAddr), &addr6Start, sizeof(addr6Start));
    uint32_t off = IpParamParser::RfindIp6(addr6Start, prefixlen, IPV6_BIT_COUNT - 1, 1);
    if (off != IPV6_BIT_COUNT) {
        IpParamParser::AddIp6(tmpAddr, off + 1, list);
        IpParamParser::ChangeIp6Start(off + 1, tmpAddr);
        off = IpParamParser::RfindIp6(addr6Start, prefixlen, off, 0);
        while (off != IPV6_BIT_COUNT && off != prefixlen) {
            IpParamParser::AddIp6(tmpAddr, off + 1, list);
            IpParamParser::ChangeIp6Start(off + 1, tmpAddr);
            off = IpParamParser::RfindIp6(addr6Start, prefixlen, off - 1, 0);
        }
    } else if (off == IPV6_BIT_COUNT) {
        if (IpParamParser::RfindIp6(addr6End, prefixlen, IPV6_BIT_COUNT - 1, 0) == IPV6_BIT_COUNT) {
            IpParamParser::AddIp6(addr6Start, prefixlen, list);
            return NETFIREWALL_SUCCESS;
        }
    }
    off = IpParamParser::FindIp6(addr6End, prefixlen, 1);
    if (off == IPV6_BIT_COUNT) {
        NETNATIVE_LOGW("GetIp6AndMask off equal 128");
        return NETFIREWALL_ERR;
    }
    off = IpParamParser::FindIp6(addr6End, off + 1, 1);
    while (off != IPV6_BIT_COUNT) {
        IpParamParser::AddIp6(tmpAddr, off + 1, list);
        IpParamParser::ChangeIp6Start(off + 1, tmpAddr);
        off = IpParamParser::FindIp6(addr6End, off + 1, 1);
    }
    ret = memcmp(&tmpAddr, &addr6End, sizeof(addr6Start));
    if (ret == 0) {
        IpParamParser::AddIp6(tmpAddr, IPV6_MAX_PREFIXLEN, list);
    }
    return NETFIREWALL_SUCCESS;
}

int32_t IpParamParser::GetIp6AndMask(const in6_addr &addr6Start, const in6_addr &addr6End, std::vector<Ip6Data> &list)
{
    Ip6RangeKey key;
    if (memcpy_s(key.start, sizeof(key.start), addr6Start.s6_addr, sizeof(addr6Start.s6_addr)) != EOK ||
        memcpy_s(key.end, sizeof(key.end), addr6End.s6_addr, sizeof(addr6End.s6_addr)) != EOK) {
        return ComputeIp6AndMask(addr6Start, addr6End, list);
    }

    {
        std::lock_guard<std::mutex> guard(g_ip6RangeCacheMutex);
        auto it = g_ip6RangeCache.find(key);
        if (it != g_ip6RangeCache.end()) {
            list.insert(list.end(), it->second.begin(), it->second.end());
            return NETFIREWALL_SUCCESS;
        }
    }

    size_t offset = list.size();
    int32_t ret = ComputeIp6AndMask(addr6Start, addr6End, list);
    if (ret != NETFIREWALL_SUCCESS) {
        return ret;
    }

    std::vector<Ip6Data> expanded(list.begin() + offset, list.end());
    std::lock_guard<std::mutex> guard(g_ip6RangeCacheMutex);
    if (g_ip6RangeCache.size() >= IP6_RANGE_CACHE_MAX) {
        g_ip6RangeCache.clear();
    }
    g_ip6RangeCache.emplace(key, std::move(expanded));
    return NETFIREWALL_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS