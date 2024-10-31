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

#ifndef NETMANAGER_EXT_NET_FIREWALL_BITMAP_MANAGER_H
#define NETMANAGER_EXT_NET_FIREWALL_BITMAP_MANAGER_H

#include <securec.h>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <utility>

#include "netfirewall/netfirewall_def.h"
#include "netfirewall_parcel.h"

namespace OHOS::NetManagerStandard {
enum NetFirewallError {
    NETFIREWALL_SUCCESS = 0,
    NETFIREWALL_ERR,
    NETFIREWALL_IP_STR_ERR,
    NETFIREWALL_MASK_ERR,
    NETFIREWALL_FAMILY_ERR,
    NETFIREWALL_EMPTY_ERR,
};

class Bitmap {
public:
    Bitmap();

    explicit Bitmap(uint32_t n);

    Bitmap(const Bitmap &other);

    ~Bitmap() = default;

    void Clear();

    /**
     * set bit of index n to 1
     *
     * @param n bit index
     */
    void Set(uint32_t n);

    /**
     * get bitmap hash code
     *
     * @return hash code
     */
    uint64_t SpecialHash() const;

    /**
     * get bitmap memory address
     *
     * @return address
     */
    uint32_t *Get();

    /**
     * and by bit
     *
     * @param other rule bitmap
     */
    void And(const Bitmap &other);

    /**
     * or by bit
     *
     * @param other rule bitmap
     */
    void Or(const Bitmap &other);

    bool operator == (const Bitmap &other) const;

    Bitmap &operator = (const Bitmap &other);

private:
    /**
     * get uin32_t hash, use thomas Wang's 32 bit Mix Function
     *
     * @param key input uin32_t number
     * @return hash number
     */
    uint32_t GetHash(uint32_t key) const;

private:
    bitmap_t bitmap_;
};

template <class T> class BpfUnorderedMap {
public:
    /**
     * if key is not exist in map insert value, or get value or with input value
     *
     * @param key input key
     * @param val rule bitmap
     */
    void OrInsert(const T &key, const Bitmap &val)
    {
        auto it = map_.find(key);
        if (it == map_.end()) {
            map_.insert(std::make_pair(key, Bitmap(val)));
        } else {
            it->second.Or(val);
        }
    }

    /**
     * set all value of map or with input bitmap
     *
     * @param other rule bitmap
     */
    void OrForEach(const Bitmap &other)
    {
        auto it = map_.begin();
        for (; it != map_.end(); ++it) {
            it->second.Or(other);
        }
    }

    int32_t Delete(const T &key)
    {
        return map_.erase(key);
    }

    void Clear()
    {
        map_.clear();
    }

    std::unordered_map<T, Bitmap> &Get()
    {
        return map_;
    }

    bool Empty()
    {
        return map_.empty();
    }

private:
    std::unordered_map<T, Bitmap> map_;
};

struct BitmapHash {
    uint64_t operator () (const Bitmap &bitmap) const
    {
        return bitmap.SpecialHash();
    }
};

const uint32_t BIT_PER_BYTE = 8;
const uint32_t IPV6_BIT_COUNT = 128;
const uint32_t IPV4_BIT_COUNT = 32;
const uint32_t IPV6_BYTE_COUNT = 16;
const uint32_t IPV6_SEGMENT_COUNT = 8;
const uint32_t VALUE_ONE = 1;

struct Ip4Data {
    uint32_t mask;
    uint32_t data; // Host Long ip
};

struct Ip6Data {
    uint32_t prefixlen;
    in6_addr data;
};

class IpParamParser {
public:
    IpParamParser() = default;

    /**
     * convert ip4segment to ip4 and mask list
     *
     * @param startAddr start ip
     * @param endAddr end ip
     * @param list output vector
     * @return success:return NETFIREWALL_SUCCESS, otherwise return error code
     */
    static int32_t GetIp4AndMask(const in_addr &startAddr, const in_addr &endAddr, std::vector<Ip4Data> &list);

    /**
     * convert ip4 string to uint32_t of network byte order
     *
     * @param address ip4 string
     * @param ipInt ip4
     * @return success:NETFIREWALL_SUCCESS, fail:NETFIREWALL_IP_STR_ERR
     */
    static int32_t GetIpUint32(const std::string &address, uint32_t &ipInt);

    static std::string Ip4ToStr(uint32_t ip);

    /**
     * save ip4 and mask to vector
     *
     * @param ip uint32_t of Network byte order
     * @param mask ip4 mask
     * @param ip4Vec out put vector
     */

    static void AddIp(uint32_t ip, uint32_t mask, std::vector<Ip4Data> &ip4Vec);

    /**
     * get biggest mask from startIp and endIp
     *
     * @param startIp start ip
     * @param endIp end ip
     * @return ip mask
     */

    static uint32_t GetMask(uint32_t startIp, uint32_t endIp);

    /**
     * find value of bit from ip4 uint32_t from right to left
     *
     * @param ip uint32_t of Network byte order
     * @param start start index
     * @param end  end index
     * @param value find value 0 or 1
     * @return if founded return bit index of ip, otherwise return IPV4_BIT_COUNT
     */
    static uint32_t Rfind(uint32_t ip, uint32_t start, uint32_t end, uint32_t value);

    /**
     * find value of bit from ip4 uint32_t from right to left
     *
     * @param ip uint32_t of Network byte order
     * @param start start index
     * @param value find value 0 or 1
     * @return if founded return bit index of ip, otherwise return IPV4_BIT_COUNT
     */
    static uint32_t Find(uint32_t ip, uint32_t start, uint32_t value);

    /**
     * get broadcast ip from mask and ip, set ip to next ip of broadcast
     *
     * @param mask ip4 mask
     * @param ip input and output
     */
    static void ChangeStart(uint32_t mask, uint32_t &ip);

    /**
     * convert ip6segment to ip6 and mask list
     *
     * @param addr6Start start ip
     * @param addr6End end ip
     * @param list output vector
     * @return if successed:return NETFIREWALL_SUCCESS, otherwise return error code
     */
    static int32_t GetIp6AndMask(const in6_addr &addr6Start, const in6_addr &addr6End, std::vector<Ip6Data> &list);

    static std::string Addr6ToStr(const in6_addr &v6Addr);

    /**
     * convert ip6 string to in6_addr of Network byte order
     *
     * @param ipStr ip6 string
     * @param addr output ip6
     * @return success:NETFIREWALL_SUCCESS, fail:NETFIREWALL_IP_STR_ERR
     */
    static int32_t GetInAddr6(const std::string &ipStr, in6_addr &addr);

    /**
     * get biggest prefixlen from start ip and end ip
     *
     * @param start start ip
     * @param end end ip
     * @return ip6 prefixlen
     */
    static uint32_t GetIp6Prefixlen(const in6_addr &start, const in6_addr &end);

    /**
     * save ip6 and prefixlen to vector
     *
     * @param addr ip6 data
     * @param prefixlen ip6 prefixlen
     * @param list output vector
     */
    static void AddIp6(const in6_addr &addr, uint32_t prefixlen, std::vector<Ip6Data> &list);

    /**
     * get broadcast ip6 from ip6 and start bit of ip6, set ip to next ip of broadcast
     *
     * @param startBit start bit of ip6
     * @param start input and output
     */
    static void ChangeIp6Start(uint32_t startBit, in6_addr &start);

    /**
     * find value of bit from ip6 from right to left
     *
     * @param addr in6_addr of Network byte order
     * @param startBit start index of bit
     * @param endBit  end index of bit
     * @param value find value 0 or 1
     * @return if founded return index of bit of addr, otherwise return IPV6_BIT_COUNT
     */
    static uint32_t RfindIp6(const in6_addr &addr, uint32_t startBit, uint32_t endBit, uint8_t value);

    /**
     * find value of bit from ip6 from right to left
     *
     * @param addr in6_addr of Network byte order
     * @param startBit start index of bit
     * @param value find value 0 or 1
     * @return if founded return bit index of addr, otherwise return IPV6_BIT_COUNT
     */
    static uint32_t FindIp6(const in6_addr &addr, uint32_t startBit, uint8_t value);
};

struct Ip4RuleBitmap {
    uint32_t mask;
    uint32_t data; // Network ip
    Bitmap bitmap;
};

struct Ip6RuleBitmap {
    uint32_t prefixlen;
    in6_addr data;
    Bitmap bitmap;
};

using Ip4RuleBitmapVector = std::vector<Ip4RuleBitmap>;
using Ip6RuleBitmapVector = std::vector<Ip6RuleBitmap>;

class Ip4RuleMap {
public:
    /**
     * set all bitmap of vector or with input bitmap
     *
     * @param bitmap rule bitmap
     */
    void OrForEach(const Bitmap &bitmap)
    {
        auto it = ruleBitmapVec_.begin();
        for (; it != ruleBitmapVec_.end(); ++it) {
            it->bitmap.Or(bitmap);
        }
    }

    /**
     * if addr and mask not exist in vector, save value to vector, otherwise or bitmap
     *
     * @param addr Network ip
     * @param mask ip mask
     * @param bitmap rule bitmap
     */
    void OrInsert(uint32_t addr, uint32_t mask, Bitmap &bitmap)
    {
        std::vector<Ip4RuleBitmapVector::iterator> matches;
        uint32_t networkAddr = GetNetworkAddress(addr, mask);
        for (auto it = ruleBitmapVec_.begin(); it != ruleBitmapVec_.end(); ++it) {
            if (it->data == htonl(addr) || GetNetworkAddress(ntohl(it->data), it->mask) == networkAddr) {
                matches.emplace_back(it);
            }
        }
        if (matches.empty()) {
            Ip4RuleBitmap ruleBitmap;
            ruleBitmap.data = htonl(addr);
            ruleBitmap.mask = mask;
            ruleBitmap.bitmap = bitmap;
            ruleBitmapVec_.emplace_back(std::move(ruleBitmap));
        } else {
            for (const auto &it : matches) {
                it->bitmap.Or(bitmap);
            }
        }
    }

    void Clear()
    {
        ruleBitmapVec_.clear();
    }

    std::vector<Ip4RuleBitmap> &Get()
    {
        return ruleBitmapVec_;
    }

private:
    /**
     * get value from ip & mask by network byte order
     *
     * @param addr ip
     * @param mask ip mask
     * @return mask uint32 value by network byte order
     */
    inline uint32_t GetNetworkAddress(uint32_t addr, uint32_t mask)
    {
        return addr & (0xFFFFFFFF << (IPV4_MAX_PREFIXLEN - mask));
    }

private:
    std::vector<Ip4RuleBitmap> ruleBitmapVec_;
};

class Ip6RuleMap {
public:
    /**
     * set all bitmap of vector or with input bitmap
     *
     * @param bitmap rule bitmap
     */
    void OrForEach(const Bitmap &bitmap)
    {
        auto it = ruleBitmapVec_.begin();
        for (; it != ruleBitmapVec_.end(); ++it) {
            it->bitmap.Or(bitmap);
        }
    }

    /**
     * if addr and prefixlen not exist in vector, save value to vector, otherwise or bitmap
     *
     * @param addr ip6
     * @param prefixlen ip6 prefixlen
     * @param bitmap rule bitmap
     */
    void OrInsert(const in6_addr &addr, uint32_t prefixlen, Bitmap &bitmap)
    {
        std::vector<Ip6RuleBitmapVector::iterator> matches;
        in6_addr networkAddr = {};
        GetNetworkAddress(addr, prefixlen, networkAddr);
        for (auto it = ruleBitmapVec_.begin(); it != ruleBitmapVec_.end(); ++it) {
            in6_addr otherNetworkAddr = {};
            GetNetworkAddress(it->data, it->prefixlen, otherNetworkAddr);
            if (!memcmp(&addr, &(it->data), sizeof(in6_addr)) ||
                !memcmp(&networkAddr, &otherNetworkAddr, sizeof(in6_addr))) {
                matches.emplace_back(it);
            }
        }
        if (matches.empty()) {
            Ip6RuleBitmap ruleBitmap;
            ruleBitmap.data = addr;
            ruleBitmap.prefixlen = prefixlen;
            ruleBitmap.bitmap = bitmap;
            ruleBitmapVec_.emplace_back(std::move(ruleBitmap));
        } else {
            for (const auto &it : matches) {
                it->bitmap.Or(bitmap);
            }
        }
    }

    void Clear()
    {
        ruleBitmapVec_.clear();
    }

    std::vector<Ip6RuleBitmap> &Get()
    {
        return ruleBitmapVec_;
    }

private:
    void GetNetworkAddress(in6_addr addr, int prefixLen, in6_addr &out)
    {
        int quotient = prefixLen / 8;
        int remainder = prefixLen % 8;
        for (int i = 0; i < quotient; i++) {
            out.s6_addr[i] = addr.s6_addr[i] & 0xff;
        }
        if (remainder) {
            out.s6_addr[quotient] = addr.s6_addr[quotient] & (~(0xff >> remainder));
        }
    }

private:
    std::vector<Ip6RuleBitmap> ruleBitmapVec_;
};

struct SegmentBitmap {
    uint16_t start;
    uint16_t end;
    Bitmap bitmap;
};

class SegmentBitmapMap {
public:
    SegmentBitmapMap() = default;

    /**
     * add segment and rule bitmap map
     *
     * @param start start of segment
     * @param end end of segment
     * @param bitmap rule itmap
     */
    void AddMap(uint16_t start, uint16_t end, const Bitmap &bitmap)
    {
        std::vector<uint32_t> indexs;
        SearchIntersection(start, end, indexs);
        if (indexs.empty()) {
            Insert(start, end, bitmap);
            return;
        }
        std::vector<SegmentBitmap> list;
        GetMapList(start, end, bitmap, indexs, list);
        DeleteSegBitmap(indexs);
        AddSegBitmapMap(list);
    }

    std::vector<SegmentBitmap> &GetMap()
    {
        return mapList_;
    }

private:
    /**
     * search input segment intersection in exist map list
     *
     * @param start start of segment
     * @param end end of segment
     * @param indexs has intersection index value in exist map
     */
    void SearchIntersection(uint16_t start, uint16_t end, std::vector<uint32_t> &indexs)
    {
        for (size_t i = 0; i < mapList_.size(); ++i) {
            if (((start >= mapList_[i].start) && (start <= mapList_[i].end)) ||
                ((mapList_[i].start >= start) && (mapList_[i].start <= end))) {
                indexs.push_back(i);
            }
        }
    }

    void DeleteSegBitmap(std::vector<uint32_t> &indexs)
    {
        if (indexs.empty()) {
            return;
        }
        auto it = indexs.rbegin();
        for (; it != indexs.rend(); ++it) {
            mapList_.erase(mapList_.begin() + *it);
        }
    }

    /**
     * insert segment and bitmap into map lsit
     *
     * @param start start of segment
     * @param end end of segment
     * @param bitmap rule bitmap
     */
    void Insert(uint16_t start, uint16_t end, const Bitmap &bitmap)
    {
        SegmentBitmap segBitmap;
        segBitmap.start = start;
        segBitmap.end = end;
        segBitmap.bitmap = bitmap;
        std::vector<SegmentBitmap>::iterator it = mapList_.begin();
        for (; it != mapList_.end(); ++it) {
            if (start < it->start) {
                mapList_.insert(it, segBitmap);
                return;
            }
        }
        mapList_.insert(mapList_.end(), segBitmap);
    }

    /**
     * add segment and bitmap map to vector
     *
     * @param start start of segment
     * @param end end of segment
     * @param bitmap rule bitmap
     * @param mapList map list
     */
    void AddSegBitmap(uint16_t start, uint16_t end, const Bitmap &bitmap, std::vector<SegmentBitmap> &mapList)
    {
        if (start > end) {
            return;
        }
        SegmentBitmap tmpSegBitmap;
        tmpSegBitmap.start = start;
        tmpSegBitmap.end = end;
        tmpSegBitmap.bitmap = bitmap;
        mapList.emplace_back(tmpSegBitmap);
    }

    void AddSegBitmapMap(const std::vector<SegmentBitmap> &mapList)
    {
        auto it = mapList.begin();
        for (; it != mapList.end(); ++it) {
            Insert(it->start, it->end, it->bitmap);
        }
    }

    /**
     * merge segment and bitmap map with exit map into new maps
     *
     * @param start start of segment
     * @param end end of segment
     * @param bitmap rule bitmap
     * @param indexs intersection indexs
     * @param list segment and rule bitmap map list
     */
    void GetMapList(uint16_t start, uint16_t end, const Bitmap &bitmap, std::vector<uint32_t> &indexs,
        std::vector<SegmentBitmap> &list)
    {
        uint32_t tmpStart = start;
        for (auto index : indexs) {
            SegmentBitmap &segBitmap = mapList_[index];
            if (tmpStart < segBitmap.start) {
                AddSegBitmap(tmpStart, segBitmap.start - 1, bitmap, list);
                Bitmap bmap(bitmap);
                if (end <= segBitmap.end) {
                    bmap.Or(segBitmap.bitmap);
                    AddSegBitmap(segBitmap.start, end, bmap, list);
                    AddSegBitmap(end + 1, segBitmap.end, segBitmap.bitmap, list);
                    tmpStart = end + 1;
                    break;
                } else {
                    bmap.Or(segBitmap.bitmap);
                    AddSegBitmap(segBitmap.start, segBitmap.end, bmap, list);
                    tmpStart = segBitmap.end + 1;
                }
            } else {
                if (tmpStart > segBitmap.start) {
                    AddSegBitmap(segBitmap.start, tmpStart - 1, segBitmap.bitmap, list);
                }
                Bitmap bmap(bitmap);
                if (end <= segBitmap.end) {
                    bmap.Or(segBitmap.bitmap);
                    AddSegBitmap(tmpStart, end, bmap, list);
                    AddSegBitmap(end + 1, segBitmap.end, segBitmap.bitmap, list);
                    tmpStart = end + 1;
                    break;
                } else {
                    bmap.Or(segBitmap.bitmap);
                    AddSegBitmap(tmpStart, segBitmap.end, bmap, list);
                    tmpStart = segBitmap.end + 1;
                }
            }
        }
        if (tmpStart <= end) {
            AddSegBitmap(tmpStart, end, bitmap, list);
        }
    }

private:
    std::vector<SegmentBitmap> mapList_;
};

using PortKey = port_key;
using ProtoKey = proto_key;
using AppUidKey = appuid_key;
using UidKey = uid_key;
using ActionValue = action_val;

using BpfStrMap = BpfUnorderedMap<std::string>;
using BpfPortMap = BpfUnorderedMap<PortKey>;
using BpfProtoMap = BpfUnorderedMap<ProtoKey>;
using BpfAppUidMap = BpfUnorderedMap<AppUidKey>;
using BpfUidMap = BpfUnorderedMap<UidKey>;
using BpfActionMap = BpfUnorderedMap<action_key>;

class BitmapManager {
public:
    BitmapManager() {}

    ~BitmapManager() = default;

    /**
     * build firewall rule bitmap map
     *
     * @param ruleList fire wall list
     * @return success: return NETFIREWALL_SUCCESS, otherwise return error code
     */
    int32_t BuildBitmapMap(const std::vector<sptr<NetFirewallIpRule>> &ruleList);

    Ip4RuleBitmapVector &GetSrcIp4Map()
    {
        return srcIp4Map_.Get();
    }

    Ip6RuleBitmapVector &GetSrcIp6Map()
    {
        return srcIp6Map_.Get();
    }

    Ip4RuleBitmapVector &GetDstIp4Map()
    {
        return dstIp4Map_.Get();
    }

    Ip6RuleBitmapVector &GetDstIp6Map()
    {
        return dstIp6Map_.Get();
    }

    BpfPortMap &GetSrcPortMap()
    {
        return srcPortMap_;
    }

    BpfPortMap &GetDstPortMap()
    {
        return dstPortMap_;
    }

    BpfProtoMap &GetProtoMap()
    {
        return protoMap_;
    }

    BpfAppUidMap &GetAppIdMap()
    {
        return appUidMap_;
    }

    BpfUidMap &GetUidMap()
    {
        return uidMap_;
    }

    BpfActionMap &GetActionMap()
    {
        return actionMap_;
    }

    static uint16_t Hltons(uint32_t n);

    static uint16_t Nstohl(uint16_t n);

private:
    void Clear();

    /**
     * build firewall rule bitmap map, with element seted
     *
     * @param ruleList fire wall list
     * @return success: return NETFIREWALL_SUCCESS, otherwise return error code
     */
    int32_t BuildMarkBitmap(const std::vector<sptr<NetFirewallIpRule>> &ruleList);

    /**
     * build firewall rule bitmap map, with element not seted
     *
     * @param ruleList fire wall list
     * @return success: return NETFIREWALL_SUCCESS, otherwise return error code
     */
    void BuildNoMarkBitmap(const std::vector<sptr<NetFirewallIpRule>> &ruleList);

    /**
     * insert ip and rule bitmap map
     *
     * @param ipInfo ip info
     * @param isSrc true: Source, false: local
     * @param bitmap rule bitmap
     * @return success: return NETFIREWALL_SUCCESS, otherwise return error code
     */
    int32_t InsertIpBitmap(const std::vector<NetFirewallIpParam> &ipInfo, bool isSrc, Bitmap &bitmap);

    /**
     * convect port segement map to single port map
     *
     * @param portSegMap segment port and rule bitmap map
     * @param portMap output single port and rule bitmap map
     * @return success: return NETFIREWALL_SUCCESS, otherwise return error code
     */
    void OrInsertPortBitmap(SegmentBitmapMap &portSegMap, BpfUnorderedMap<PortKey> &portMap);

    /**
     * judge protocols if need port map
     *
     * @param protocol transform protoco
     * @return true: not need; false: needed
     */
    bool IsNotNeedPort(NetworkProtocol protocol);

    /**
     * insert ip6 segment and bitmap map
     *
     * @param item ip info
     * @param bitmap rule bitmap
     * @param ip6Map ip6 and rule bitmap map
     * @return success: return NETFIREWALL_SUCCESS, otherwise return error code
     */
    int32_t InsertIp6SegBitmap(const NetFirewallIpParam &item, Bitmap &bitmap, Ip6RuleMap *ip6Map);

    /**
     * insert ip4 segment and bitmap map
     *
     * @param item ip info
     * @param bitmap rule bitmap
     * @param ip4Map ip4 and rule bitmap map
     * @return success: return NETFIREWALL_SUCCESS, otherwise return error code
     */
    int32_t InsertIp4SegBitmap(const NetFirewallIpParam &item, Bitmap &bitmap, Ip4RuleMap *ip4Map);

    /**
     * save port segment and bitmap map to map list
     *
     * @param port port info
     * @param bitmap rule bitmap
     * @param portMap port segment and bitmap map list
     */
    void AddPortBitmap(const std::vector<NetFirewallPortParam> &port, Bitmap &bitmap, SegmentBitmapMap &portMap);

private:
    Ip4RuleMap srcIp4Map_;
    Ip4RuleMap dstIp4Map_;
    Ip6RuleMap srcIp6Map_;
    Ip6RuleMap dstIp6Map_;
    BpfPortMap srcPortMap_;
    BpfPortMap dstPortMap_;
    BpfProtoMap protoMap_;
    BpfAppUidMap appUidMap_;
    BpfUidMap uidMap_;
    BpfActionMap actionMap_;
};
} // namespace OHOS::NetManagerStandard
#endif /* NETMANAGER_EXT_NET_FIREWALL_BITMAP_MANAGER_H */
