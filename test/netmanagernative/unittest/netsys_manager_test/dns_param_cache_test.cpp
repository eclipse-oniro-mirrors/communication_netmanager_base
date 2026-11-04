/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <linux/bpf.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "bpf_def.h"
#include "bpf_mapper.h"
#include "bpf_path.h"
#include "dns_config_client.h"
#include "dns_param_cache.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;
class DNSParamCacheTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DNSParamCacheTest::SetUpTestCase() {}

void DNSParamCacheTest::TearDownTestCase() {}

void DNSParamCacheTest::SetUp() {}

void DNSParamCacheTest::TearDown() {}

HWTEST_F(DNSParamCacheTest, SetResolverConfigTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetResolverConfigTest001 enter");
    DnsParamCache dnsParCache;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    servers.resize(MAX_SERVER_NUM + 1);
    uint16_t netId = 1;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t ret = dnsParCache.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    EXPECT_EQ(ret, -ENOENT);
}

HWTEST_F(DNSParamCacheTest, SetResolverConfigTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("SetResolverConfigTest002 enter");
    DnsParamCache dnsParCache;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    servers.resize(MAX_SERVER_NUM + 1);
    std::string hostName = "hoseName";
    AddrInfo addrInfo;
    for (size_t i = 0; i < MAX_SERVER_NUM; i++) {
        dnsParCache.CreateCacheForNet(i + 1);
        dnsParCache.SetDnsCache(i, 0, hostName.append(std::to_string(i)), addrInfo);
        servers.emplace_back(hostName);
    }
    uint16_t netId = 1;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t ret = dnsParCache.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, SetResolverConfigTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("SetResolverConfigTest003 enter");
    DnsParamCache dnsParCache;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    servers.resize(MAX_SERVER_NUM + 1);
    uint16_t netId = 1;
    std::string hostName = "hoseName";
    dnsParCache.GetDnsCache(netId, 0, hostName);
    AddrInfo addrInfo;
    addrInfo.aiFlags = 100;
    for (size_t i = 0; i < MAX_SERVER_NUM; i++) {
        dnsParCache.CreateCacheForNet(i);
        dnsParCache.SetDnsCache(i, 0, hostName.append(std::to_string(i)), addrInfo);
        servers.emplace_back(hostName.append(std::to_string(i)));
    }
    
    uint16_t baseTimeoutMsec = 100;
    uint8_t retryCount = 2;
    int32_t ret = dnsParCache.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    EXPECT_EQ(ret, 0);
    netId = 100;
    ret = dnsParCache.GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, -ENOENT);
    netId = 1;
    ret = dnsParCache.GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, 0);
    std::string info;
    dnsParCache.GetDumpInfo(info);
}

HWTEST_F(DNSParamCacheTest, CreateCacheForNetTest, TestSize.Level1)
{
    NETNATIVE_LOGI("CreateCacheForNetTest enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.SetDefaultNetwork(netId);
    int32_t ret = dnsParCache.CreateCacheForNet(netId);
    EXPECT_EQ(ret, 0);
    ret = dnsParCache.CreateCacheForNet(netId);
    EXPECT_EQ(ret, -EEXIST);
    netId = 0;
    std::string hostName = "hostName";
    dnsParCache.SetCacheDelayed(netId, 0, hostName);
    netId = 2;
    dnsParCache.SetCacheDelayed(netId, 0, hostName);
}

HWTEST_F(DNSParamCacheTest, DestroyNetworkCacheTest, TestSize.Level1)
{
    NETNATIVE_LOGI("DestroyNetworkCacheTest enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    int32_t ret = dnsParCache.DestroyNetworkCache(netId);
    EXPECT_EQ(ret, -ENOENT);
    dnsParCache.SetDefaultNetwork(netId);
    dnsParCache.CreateCacheForNet(netId);
    ret = dnsParCache.DestroyNetworkCache(netId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, EnableIpv6Test01, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.EnableIpv6(netId);
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(netId));
}

HWTEST_F(DNSParamCacheTest, EnableIpv6Test02, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);

    dnsParCache.EnableIpv6(netId);
    EXPECT_TRUE(dnsParCache.IsIpv6Enable(netId));

    dnsParCache.EnableIpv6(netId, false);
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(netId));
}

HWTEST_F(DNSParamCacheTest, IsIpv6EnableTest01, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint16_t netId = 0;
    EXPECT_TRUE(netId == dnsParCache.defaultNetId_);

    bool ret = dnsParCache.IsIpv6Enable(netId);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, EnableIpv4Test01, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.SetClatDnsEnableIpv4(netId, true);
    EXPECT_TRUE(dnsParCache.IsIpv4Enable(netId));
    dnsParCache.SetClatDnsEnableIpv4(netId, false);
    EXPECT_FALSE(dnsParCache.IsIpv4Enable(netId));
    dnsParCache.EnableIpv4(netId);
    EXPECT_TRUE(dnsParCache.IsIpv4Enable(netId));
    dnsParCache.defaultNetId_ = netId;
    bool res = dnsParCache.IsIpv4Enable(0);
    EXPECT_TRUE(res);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetNetworkDnsFamilyCapabilityTest001, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    constexpr uint16_t netId = 100;
    EXPECT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(netId, true, true), -ENOENT);
    ASSERT_EQ(dnsParCache.CreateCacheForNet(netId), 0);
    EXPECT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(netId, true, false), 0);
    EXPECT_TRUE(dnsParCache.IsIpv4Enable(netId));
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(netId));
    EXPECT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(netId, false, true), 0);
    EXPECT_FALSE(dnsParCache.IsIpv4Enable(netId));
    EXPECT_TRUE(dnsParCache.IsIpv6Enable(netId));
}

HWTEST_F(DNSParamCacheTest, DnsFamilyCapabilitySelectVpnByUidTest001, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    constexpr uint16_t defaultNetId = 100;
    constexpr uint16_t vpnNetId = 200;
    constexpr uint32_t vpnUid = 1500;
    ASSERT_EQ(dnsParCache.CreateCacheForNet(defaultNetId), 0);
    ASSERT_EQ(dnsParCache.CreateCacheForNet(vpnNetId, true), 0);
    dnsParCache.SetDefaultNetwork(defaultNetId);
    ASSERT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(defaultNetId, true, false), 0);
    ASSERT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(vpnNetId, false, true), 0);
    std::vector<NetManagerStandard::UidRange> ranges = {{1000, 2000, 1, vpnNetId}};
    ASSERT_EQ(dnsParCache.AddUidRange(vpnNetId, ranges), 0);

    EXPECT_FALSE(dnsParCache.IsIpv4Enable(0, vpnUid));
    EXPECT_TRUE(dnsParCache.IsIpv6Enable(0, vpnUid));
    EXPECT_TRUE(dnsParCache.IsIpv4Enable(0, 999));
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(0, 999));
    EXPECT_TRUE(dnsParCache.IsIpv4Enable(defaultNetId, vpnUid));
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(defaultNetId, vpnUid));
}

HWTEST_F(DNSParamCacheTest, DnsFamilyCapabilitySelectVpnPriorityTest001, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    constexpr uint16_t lowPriorityNetId = 200;
    constexpr uint16_t highPriorityNetId = 201;
    ASSERT_EQ(dnsParCache.CreateCacheForNet(lowPriorityNetId, true), 0);
    ASSERT_EQ(dnsParCache.CreateCacheForNet(highPriorityNetId, true), 0);
    ASSERT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(lowPriorityNetId, true, false), 0);
    ASSERT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(highPriorityNetId, false, true), 0);
    ASSERT_EQ(dnsParCache.AddUidRange(lowPriorityNetId, {{1000, 2000, 1, lowPriorityNetId}}), 0);
    ASSERT_EQ(dnsParCache.AddUidRange(highPriorityNetId, {{1000, 2000, 2, highPriorityNetId}}), 0);

    EXPECT_FALSE(dnsParCache.IsIpv4Enable(0, 1500));
    EXPECT_TRUE(dnsParCache.IsIpv6Enable(0, 1500));
}

HWTEST_F(DNSParamCacheTest, DnsFamilyCapabilityCacheInvalidationTest001, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    constexpr uint16_t netId = 100;
    constexpr uint32_t uid = 1000;
    const std::string host = "vpn.example";
    ASSERT_EQ(dnsParCache.CreateCacheForNet(netId), 0);
    ASSERT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(netId, true, false), 0);

    AddrInfo addrInfo{};
    dnsParCache.SetDnsCache(netId, uid, host, addrInfo);
    dnsParCache.SetNodataCache(netId, uid, host);
    std::vector<int32_t> netIds = {netId};
    dnsParCache.SetIpv6UidBlackList(netIds, uid);
    ASSERT_FALSE(dnsParCache.GetDnsCache(netId, uid, host).empty());
    ASSERT_TRUE(dnsParCache.IsInNodataCache(netId, uid, host));
    ASSERT_TRUE(dnsParCache.IsInIpv6UidBlackList(netId, uid));

    EXPECT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(netId, true, false), 0);
    EXPECT_FALSE(dnsParCache.GetDnsCache(netId, uid, host).empty());
    EXPECT_TRUE(dnsParCache.IsInNodataCache(netId, uid, host));
    EXPECT_TRUE(dnsParCache.IsInIpv6UidBlackList(netId, uid));

    EXPECT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(netId, false, true), 0);
    EXPECT_TRUE(dnsParCache.GetDnsCache(netId, uid, host).empty());
    EXPECT_FALSE(dnsParCache.IsInNodataCache(netId, uid, host));
    EXPECT_FALSE(dnsParCache.IsInIpv6UidBlackList(netId, uid));
}

HWTEST_F(DNSParamCacheTest, DnsFamilyCapabilityMissingVpnFallbackTest001, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    constexpr uint16_t defaultNetId = 100;
    constexpr uint16_t missingVpnNetId = 200;
    constexpr uint32_t vpnUid = 1500;
    ASSERT_EQ(dnsParCache.CreateCacheForNet(defaultNetId), 0);
    dnsParCache.SetDefaultNetwork(defaultNetId);
    ASSERT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(defaultNetId, true, false), 0);
    ASSERT_EQ(dnsParCache.AddUidRange(missingVpnNetId, {{1000, 2000, 1, missingVpnNetId}}), 0);

    EXPECT_TRUE(dnsParCache.IsIpv4Enable(0, vpnUid));
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(0, vpnUid));
    ASSERT_EQ(dnsParCache.DestroyNetworkCache(defaultNetId), 0);
    EXPECT_FALSE(dnsParCache.IsIpv4Enable(0, vpnUid));
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(0, vpnUid));
}

HWTEST_F(DNSParamCacheTest, DnsFamilyCapabilityUidBoundariesAndClatTest001, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    constexpr uint16_t vpnNetId = 200;
    ASSERT_EQ(dnsParCache.CreateCacheForNet(vpnNetId, true), 0);
    ASSERT_EQ(dnsParCache.SetNetworkDnsFamilyCapability(vpnNetId, false, true), 0);
    dnsParCache.SetClatDnsEnableIpv4(vpnNetId, true);
    ASSERT_EQ(dnsParCache.AddUidRange(vpnNetId, {{1000, 2000, 1, vpnNetId}}), 0);

    for (uint32_t uid : {1000U, 2000U}) {
        EXPECT_TRUE(dnsParCache.IsIpv4Enable(0, uid));
        EXPECT_TRUE(dnsParCache.IsIpv6Enable(0, uid));
    }
    EXPECT_FALSE(dnsParCache.IsIpv4Enable(0, 999));
    EXPECT_FALSE(dnsParCache.IsIpv6Enable(0, 2001));
    EXPECT_TRUE(dnsParCache.IsIpv4Enable(vpnNetId, 999));
    EXPECT_TRUE(dnsParCache.IsIpv6Enable(vpnNetId, 999));
}

HWTEST_F(DNSParamCacheTest, DnsFamilyCapabilityIpv4AndClatFallbackMatrix, TestSize.Level1)
{
    DnsParamCache cache;
    constexpr uint16_t defaultNetId = 60126;
    constexpr uint16_t vpnNetId = 60127;
    constexpr uint32_t vpnUid = 1500;
    ASSERT_EQ(cache.CreateCacheForNet(defaultNetId), 0);
    ASSERT_EQ(cache.CreateCacheForNet(vpnNetId, true), 0);
    cache.SetDefaultNetwork(defaultNetId);
    ASSERT_EQ(cache.AddUidRange(vpnNetId, {{1000, 2000, 1, vpnNetId}}), 0);

    for (bool ipv4 : {false, true}) {
        for (bool clat : {false, true}) {
            SCOPED_TRACE(ipv4);
            SCOPED_TRACE(clat);
            ASSERT_EQ(cache.SetNetworkDnsFamilyCapability(defaultNetId, ipv4, false), 0);
            ASSERT_EQ(cache.SetNetworkDnsFamilyCapability(vpnNetId, ipv4, true), 0);
            cache.SetClatDnsEnableIpv4(defaultNetId, clat);
            cache.SetClatDnsEnableIpv4(vpnNetId, clat);
            EXPECT_EQ(cache.IsIpv4Enable(0, vpnUid), ipv4 || clat);
            EXPECT_EQ(cache.IsIpv4Enable(0, 999), ipv4 || clat);
            EXPECT_EQ(cache.IsIpv4Enable(0, 2001), ipv4 || clat);
            EXPECT_EQ(cache.IsIpv4Enable(defaultNetId, vpnUid), ipv4 || clat);
            EXPECT_TRUE(cache.IsIpv6Enable(0, vpnUid));
            EXPECT_FALSE(cache.IsIpv6Enable(0, 999));
        }
    }
    ASSERT_EQ(cache.SetNetworkDnsFamilyCapability(defaultNetId, true, true), 0);
    EXPECT_TRUE(cache.IsIpv6Enable(0, 999));
    EXPECT_TRUE(cache.IsIpv6Enable(0, UINT32_MAX));
    EXPECT_TRUE(cache.IsIpv4Enable(0, UINT32_MAX));
}

HWTEST_F(DNSParamCacheTest, GetResolverConfigTest05, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    servers.resize(MAX_SERVER_NUM + 1);
    uint16_t netId = 0;
    std::string hostName = "hoseName";
    dnsParCache.GetDnsCache(netId, 0, hostName);
    AddrInfo addrInfo;
    addrInfo.aiFlags = 100;
    for (size_t i = 0; i < MAX_SERVER_NUM; i++) {
        dnsParCache.CreateCacheForNet(i);
        dnsParCache.SetDnsCache(i, 0, hostName.append(std::to_string(i)), addrInfo);
        servers.emplace_back(hostName.append(std::to_string(i)));
    }

    uint16_t baseTimeoutMsec = 100;
    uint8_t retryCount = 2;
    int32_t ret = dnsParCache.GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_TRUE(netId == dnsParCache.defaultNetId_);

    uint32_t uid = 1;
    ret = dnsParCache.GetResolverConfig(netId, uid, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_TRUE(netId == dnsParCache.defaultNetId_);
    ret = dnsParCache.GetResolverConfig(netId, uid, servers, domains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, GetDnsCacheTest01, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint16_t netId = 0;
    std::string hostName = "";
    auto ret = dnsParCache.GetDnsCache(netId, 0, hostName);
    EXPECT_TRUE(netId == dnsParCache.defaultNetId_);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedServerFlagTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedServerFlagTest001 enter");
    DnsParamCache dnsParCache;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, 0);
    uidRanges.push_back(uidrange1);
    uint16_t netId = 0;
    bool flag = false;
    dnsParCache.SetDefaultNetwork(netId);
    dnsParCache.CreateCacheForNet(netId);
    int32_t ret = dnsParCache.GetUserDefinedServerFlag(netId, flag, 10001);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedServerFlagTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedServerFlagTest002 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 0;
    bool flag = false;
    dnsParCache.SetDefaultNetwork(netId);
    dnsParCache.CreateCacheForNet(netId);
    int32_t ret = dnsParCache.GetUserDefinedServerFlag(netId, flag);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedServerFlagTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedServerFlagTest003 enter");
    DnsParamCache dnsParCache;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, 0);
    uidRanges.push_back(uidrange1);
    uint16_t netId = 1;
    bool flag = false;
    dnsParCache.SetDefaultNetwork(netId);
    dnsParCache.CreateCacheForNet(netId);
    int32_t ret = dnsParCache.GetUserDefinedServerFlag(netId, flag, 9999);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedServerFlagTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedServerFlagTest004 enter");
    DnsParamCache dnsParCache;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, 0);
    uidRanges.push_back(uidrange1);
    uint16_t netId = 0;
    dnsParCache.SetDefaultNetwork(1);
    dnsParCache.CreateCacheForNet(1);
    bool flag = false;
    int32_t ret = dnsParCache.GetUserDefinedServerFlag(netId, flag, 9999);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedServerFlagTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedServerFlagTest005 enter");
    DnsParamCache dnsParCache;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, 0);
    uidRanges.push_back(uidrange1);
    uint16_t netId = 1;
    bool flag = false;
    int32_t ret = dnsParCache.GetUserDefinedServerFlag(netId, flag, 9999);
    EXPECT_EQ(ret, -ENOENT);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedServerFlagTest006, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedServerFlagTest006 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 0;
    bool flag = false;
    dnsParCache.SetDefaultNetwork(1);
    dnsParCache.CreateCacheForNet(1);
    int32_t ret = dnsParCache.GetUserDefinedServerFlag(netId, flag);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedServerFlagTest007, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedServerFlagTest007 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    bool flag = false;
    int32_t ret = dnsParCache.GetUserDefinedServerFlag(netId, flag);
    EXPECT_EQ(ret, -ENOENT);
}

HWTEST_F(DNSParamCacheTest, IsUseVpnDnsTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsUseVpnDnsTest007 enter");
    DnsParamCache dnsParCache;
    bool ret = dnsParCache.IsUseVpnDns(0);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, IsUseVpnDnsTest002, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint32_t vpnNetId = 100;
    std::vector<OHOS::NetManagerStandard::UidRange> uidRanges = {{1000, 2000, vpnNetId, 1}};
    dnsParCache.AddUidRange(vpnNetId, uidRanges);
    dnsParCache.CreateCacheForNet(vpnNetId, true);
    bool ret = dnsParCache.IsUseVpnDns(1500);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, IsUseVpnDnsTest003, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint32_t vpnNetId = 100;
    std::vector<OHOS::NetManagerStandard::UidRange> uidRanges = {{1000, 2000, vpnNetId, 1}};
    dnsParCache.AddUidRange(vpnNetId, uidRanges);
    bool ret = dnsParCache.IsUseVpnDns(1500);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, IsUseVpnDnsTest004, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    uint32_t vpnNetId = 100;
    std::vector<OHOS::NetManagerStandard::UidRange> uidRanges = {{1000, 2000, vpnNetId, 1}};
    dnsParCache.AddUidRange(vpnNetId, uidRanges);
    dnsParCache.CreateCacheForNet(vpnNetId, true);
    bool ret = dnsParCache.IsUseVpnDns(500);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, CreateCacheForNetTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("CreateCacheForNetTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;

    dnsParCache.SetDefaultNetwork(netId);

    int32_t ret = dnsParCache.CreateCacheForNet(netId, false);
    EXPECT_EQ(ret, 0);

    uint16_t netId2 = 2;
    ret = dnsParCache.CreateCacheForNet(netId2, false);
    EXPECT_EQ(ret, 0);

    auto it1 = dnsParCache.serverConfigMap_.find(netId);
    auto it2 = dnsParCache.serverConfigMap_.find(netId2);
    EXPECT_NE(it1, dnsParCache.serverConfigMap_.end());
    EXPECT_NE(it2, dnsParCache.serverConfigMap_.end());

    uint16_t vpnNetId = 3;
    ret = dnsParCache.CreateCacheForNet(vpnNetId, true);
    EXPECT_EQ(ret, 0);

    auto it3 = dnsParCache.serverConfigMap_.find(vpnNetId);
    EXPECT_NE(it3, dnsParCache.serverConfigMap_.end());
}

HWTEST_F(DNSParamCacheTest, DestroyNetworkCacheTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("DestroyNetworkCacheTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;

    dnsParCache.SetDefaultNetwork(netId);
    dnsParCache.CreateCacheForNet(netId);

    int32_t ret = dnsParCache.DestroyNetworkCache(netId, false);
    EXPECT_EQ(ret, 0);

    uint16_t netId2 = 2;
    ret = dnsParCache.DestroyNetworkCache(netId2, false);
    EXPECT_GE(ret, -2);
}

HWTEST_F(DNSParamCacheTest, SetUserDefinedServerFlagTest, TestSize.Level1)
{
    NETNATIVE_LOGI("CreateCacheForNetTest enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    int32_t ret = dnsParCache.SetUserDefinedServerFlag(netId, true);
    EXPECT_LE(ret, -2);
}

HWTEST_F(DNSParamCacheTest, FlushDnsCacheTest, TestSize.Level1)
{
    NETNATIVE_LOGI("FlushDnsCacheTest enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    int32_t ret = dnsParCache.FlushDnsCache(netId);
    EXPECT_EQ(ret, -2);
}

HWTEST_F(DNSParamCacheTest, FlushDnsCacheTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("FlushDnsCacheTest enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.defaultNetId_ = netId;
    dnsParCache.CreateCacheForNet(netId);
    int32_t ret = dnsParCache.FlushDnsCache(netId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, FlushDnsCacheTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("FlushDnsCacheTest enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 0;
    dnsParCache.defaultNetId_ = 1;
    dnsParCache.CreateCacheForNet(dnsParCache.defaultNetId_);
    int32_t ret = dnsParCache.FlushDnsCache(netId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, RemoveDuplicateNameserversTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("RemoveDuplicateNameserversTest enter");
    DnsParamCache dnsParCache;
    std::vector<std::string> servers;
    servers.emplace_back("1.1.1.1");
    servers.emplace_back("1.1.1.1");
    servers.emplace_back("2.2.2.2");
    servers.emplace_back("3.3.3.3");
    std::vector<std::string> res = dnsParCache.RemoveDuplicateNameservers(servers);
    EXPECT_FALSE(res.empty());
}

HWTEST_F(DNSParamCacheTest, SetDnsCacheTest001, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    dnsParCache.defaultNetId_ = 1;
    std::string hostName = "test";
    AddrInfo addrInfo;
    dnsParCache.SetDnsCache(0, 0, hostName, addrInfo);

    dnsParCache.CreateCacheForNet(1);
    dnsParCache.SetDnsCache(1, 0, hostName, addrInfo);

    AddrInfo addrInfoV6;
    addrInfoV6.aiFamily = 10;
    dnsParCache.SetDnsCache(1, 0, hostName, addrInfoV6);

    auto res = dnsParCache.GetDnsCache(1, 0, hostName);
    EXPECT_NE(res.size(), 0);
    dnsParCache.DestroyNetworkCache(1);
}

HWTEST_F(DNSParamCacheTest, SetDnsCacheTest002, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    dnsParCache.CreateCacheForNet(1);
    std::string hostName = "test";
    dnsParCache.SetCacheDelayed(1, 0, hostName);

    AddrInfo addrInfo;
    addrInfo.aiFamily = 2;
    dnsParCache.SetDnsCache(1, 0, hostName, addrInfo);
    dnsParCache.SetCacheDelayed(1, 0, hostName);

    auto res = dnsParCache.GetDnsCache(1, 0, hostName);
    EXPECT_NE(res.size(), 0);
    dnsParCache.DestroyNetworkCache(1);
}

HWTEST_F(DNSParamCacheTest, SetDnsCacheTest003, TestSize.Level1)
{
    DnsParamCache dnsParCache;
    dnsParCache.defaultNetId_ = 1;
    std::string hostName = "test";
    AddrInfoWithTtl addrInfo;
    dnsParCache.SetDnsCache(0, 0, hostName, addrInfo);

    dnsParCache.CreateCacheForNet(1);
    addrInfo.addrInfo.aiFamily = 2;
    addrInfo.ttl = 0;
    dnsParCache.SetDnsCache(0, 0, hostName, addrInfo);

    addrInfo.ttl = 50;
    dnsParCache.SetDnsCache(1, 0, hostName, addrInfo);

    auto res = dnsParCache.GetDnsCache(1, 0, hostName);
    EXPECT_NE(res.size(), 0);
    dnsParCache.DestroyNetworkCache(1);
}

HWTEST_F(DNSParamCacheTest, SetDnsCacheUidIsolationTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCacheUidIsolationTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uidA = 20000001;
    uint32_t uidB = 20000002;
    std::string hostName = "isolation.example.com";
    dnsParCache.CreateCacheForNet(netId);

    AddrInfo addrInfo;
    addrInfo.aiFamily = AF_INET;
    dnsParCache.SetDnsCache(netId, uidA, hostName, addrInfo);

    // cache entries are keyed by uid, uidB cannot read entries written by uidA
    auto resUidB = dnsParCache.GetDnsCache(netId, uidB, hostName);
    EXPECT_EQ(resUidB.size(), 0);
    // uidA still hits its own entry
    auto resUidA = dnsParCache.GetDnsCache(netId, uidA, hostName);
    EXPECT_NE(resUidA.size(), 0);

    AddrInfoWithTtl addrInfoWithTtl;
    addrInfoWithTtl.addrInfo.aiFamily = AF_INET;
    addrInfoWithTtl.ttl = 50;
    dnsParCache.SetDnsCache(netId, uidA, hostName, addrInfoWithTtl);
    EXPECT_EQ(dnsParCache.GetDnsCache(netId, uidB, hostName).size(), 0);
    EXPECT_NE(dnsParCache.GetDnsCache(netId, uidA, hostName).size(), 0);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetCacheDelayedUidIsolationTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetCacheDelayedUidIsolationTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uidA = 20000001;
    uint32_t uidB = 20000002;
    std::string hostName = "delayed.example.com";
    dnsParCache.CreateCacheForNet(netId);

    AddrInfo addrInfo;
    addrInfo.aiFamily = AF_INET;
    dnsParCache.SetDnsCache(netId, uidA, hostName, addrInfo);
    dnsParCache.SetCacheDelayed(netId, uidA, hostName);

    // delayed removal is tracked per uid, uidB has no entry for the same host
    EXPECT_EQ(dnsParCache.GetDnsCache(netId, uidB, hostName).size(), 0);
    EXPECT_NE(dnsParCache.GetDnsCache(netId, uidA, hostName).size(), 0);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, GetDefaultNetworkTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetDefaultNetworkTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.SetDefaultNetwork(netId);
    int32_t ret = dnsParCache.GetDefaultNetwork();
    EXPECT_EQ(ret, netId);
}

HWTEST_F(DNSParamCacheTest, GetDefaultNetworkTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("GetDefaultNetworkTest002 enter");
    DnsParamCache dnsParCache;
    int32_t ret = dnsParCache.GetDefaultNetwork();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, AddUidRangeTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("AddUidRangeTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 1;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, netId, 0);
    uidRanges.push_back(uidrange1);
    int32_t ret = dnsParCache.AddUidRange(netId, uidRanges);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, AddUidRangeTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("AddUidRangeTest002 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 1;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, netId, 0);
    uidRanges.push_back(uidrange1);
    NetManagerStandard::UidRange uidrange2(30000, 40000, netId, 0);
    uidRanges.push_back(uidrange2);
    int32_t ret = dnsParCache.AddUidRange(netId, uidRanges);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, DelUidRangeTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("DelUidRangeTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 1;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, netId, 0);
    uidRanges.push_back(uidrange1);
    dnsParCache.AddUidRange(netId, uidRanges);
    int32_t ret = dnsParCache.DelUidRange(netId, uidRanges);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DNSParamCacheTest, IsVpnOpenTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsVpnOpenTest001 enter");
    DnsParamCache dnsParCache;
    bool ret = dnsParCache.IsVpnOpen();
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, IsVpnOpenTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("IsVpnOpenTest002 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 1;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, netId, 0);
    uidRanges.push_back(uidrange1);
    dnsParCache.AddUidRange(netId, uidRanges);
    bool ret = dnsParCache.IsVpnOpen();
    EXPECT_TRUE(ret);
}

HWTEST_F(DNSParamCacheTest, SetNodataCacheTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetNodataCacheTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::string hostName = "test.example.com";
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    dnsParCache.SetNodataCache(netId, 0, hostName);
    bool ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_TRUE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetNodataCacheTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("SetNodataCacheTest002 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::string hostName = "test.example.com";
    dnsParCache.SetNodataCache(netId, 0, hostName);
    bool ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, IsInNodataCacheTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInNodataCacheTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::string hostName = "test.example.com";
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    bool ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_FALSE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, IsInNodataCacheTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInNodataCacheTest002 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::string hostName = "test.example.com";
    bool ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, SetNodataCacheTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("SetNodataCacheTest003 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::string hostName = "test.example.com";
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    // Set cache first time
    dnsParCache.SetNodataCache(netId, 0, hostName);
    bool ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_TRUE(ret);
    // Update existing cache entry - covers DnsResolvConfig::SetNodataCache branch for updating existing entry
    dnsParCache.SetNodataCache(netId, 0, hostName);
    ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_TRUE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, IsInNodataCacheTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInNodataCacheTest003 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::string hostName = "test.example.com";
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    dnsParCache.SetNodataCache(netId, 0, hostName);
    bool ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_TRUE(ret);
    // Clear the cache to simulate expiration - covers DnsResolvConfig::IsInNodataCache branch for cache not exist
    dnsParCache.FlushDnsCache(netId);
    ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_FALSE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetNodataCacheTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("SetNodataCacheTest004 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    // Not enable IPv4, SetNodataCache should return directly without setting cache
    // This covers DnsResolvConfig::SetNodataCache branch: if (!IsIpv4Enable()) return;
    std::string hostName = "test.example.com";
    dnsParCache.SetNodataCache(netId, 0, hostName);
    bool ret = dnsParCache.IsInNodataCache(netId, 0, hostName);
    EXPECT_FALSE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetNodataCacheTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("SetNodataCacheTest005 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    // Fill cache to MAX_NODATA_CACHE_SIZE (100) and verify oldest is removed
    // This covers DnsResolvConfig::SetNodataCache branch: if (nodataCache_.size() >= MAX_NODATA_CACHE_SIZE)
    for (size_t i = 0; i < MAX_NODATA_CACHE_SIZE + 1; ++i) {
        std::string hostName = "host" + std::to_string(i) + ".example.com";
        dnsParCache.SetNodataCache(netId, 0, hostName);
    }
    // The first entry should be removed due to cache size limit
    std::string firstHostName = "host0.example.com";
    EXPECT_FALSE(dnsParCache.IsInNodataCache(netId, 0, firstHostName));
    // The last entry should still exist
    std::string lastHostName = "host100.example.com";
    EXPECT_TRUE(dnsParCache.IsInNodataCache(netId, 0, lastHostName));
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, NodataCacheUidIsolationTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NodataCacheUidIsolationTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uidA = 20000001;
    uint32_t uidB = 20000002;
    std::string hostName = "nodata.example.com";
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);

    dnsParCache.SetNodataCache(netId, uidA, hostName);
    // NODATA cache entries are keyed by uid
    EXPECT_TRUE(dnsParCache.IsInNodataCache(netId, uidA, hostName));
    EXPECT_FALSE(dnsParCache.IsInNodataCache(netId, uidB, hostName));
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, GetDumpInfoTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetDumpInfoTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    std::vector<std::string> servers = {"8.8.8.8"};
    std::vector<std::string> domains = {"example.com"};
    dnsParCache.SetResolverConfig(netId, 1000, 3, servers, domains);
    std::string info;
    dnsParCache.GetDumpInfo(info);
    EXPECT_FALSE(info.empty());
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, AddUidRangeDedupTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("AddUidRangeDedupTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 100;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId);
    uidRanges.push_back(uidrange1);
    dnsParCache.AddUidRange(netId, uidRanges);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 1u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 1u);
    dnsParCache.AddUidRange(netId, uidRanges);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 1u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 1u);
}

HWTEST_F(DNSParamCacheTest, AddUidRangeDedupTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("AddUidRangeDedupTest002 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    uint32_t netId2 = 200;
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    std::vector<NetManagerStandard::UidRange> uidRanges2;
    NetManagerStandard::UidRange uidrange2(30000, 40000, 0, netId2);
    uidRanges2.push_back(uidrange2);
    dnsParCache.AddUidRange(netId1, uidRanges1);
    dnsParCache.AddUidRange(netId2, uidRanges2);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 2u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 2u);
}

HWTEST_F(DNSParamCacheTest, AddUidRangeDedupTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("AddUidRangeDedupTest003 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 100;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId);
    NetManagerStandard::UidRange uidrange2(30000, 40000, 0, netId);
    uidRanges.push_back(uidrange1);
    uidRanges.push_back(uidrange2);
    dnsParCache.AddUidRange(netId, uidRanges);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 2u);
    std::vector<NetManagerStandard::UidRange> uidRangesDup;
    uidRangesDup.push_back(uidrange1);
    dnsParCache.AddUidRange(netId, uidRangesDup);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 1u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 1u);
}

HWTEST_F(DNSParamCacheTest, AddUidRangeDedupTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("AddUidRangeDedupTest004 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 100;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId);
    uidRanges.push_back(uidrange1);
    dnsParCache.AddUidRange(netId, uidRanges);
    std::vector<NetManagerStandard::UidRange> emptyRanges;
    dnsParCache.AddUidRange(netId, emptyRanges);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 1u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 0u);
}

HWTEST_F(DNSParamCacheTest, DelUidRangeRemoveAllTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("DelUidRangeRemoveAllTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 100;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId);
    uidRanges.push_back(uidrange1);
    dnsParCache.AddUidRange(netId, uidRanges);
    dnsParCache.AddUidRange(netId, uidRanges);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 1u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 1u);
    dnsParCache.DelUidRange(netId, uidRanges);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 0u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 0u);
}

HWTEST_F(DNSParamCacheTest, DelUidRangeRemoveAllTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("DelUidRangeRemoveAllTest002 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    uint32_t netId2 = 200;
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    std::vector<NetManagerStandard::UidRange> uidRanges2;
    NetManagerStandard::UidRange uidrange2(30000, 40000, 0, netId2);
    uidRanges2.push_back(uidrange2);
    dnsParCache.AddUidRange(netId1, uidRanges1);
    dnsParCache.AddUidRange(netId2, uidRanges2);
    dnsParCache.DelUidRange(netId1, uidRanges1);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 1u);
    EXPECT_EQ(dnsParCache.vpnNetId_.front(), netId2);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 1u);
}

HWTEST_F(DNSParamCacheTest, DelUidRangeRemoveAllTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("DelUidRangeRemoveAllTest003 enter");
    DnsParamCache dnsParCache;
    uint32_t netId = 999;
    std::vector<NetManagerStandard::UidRange> uidRanges;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId);
    uidRanges.push_back(uidrange1);
    dnsParCache.DelUidRange(netId, uidRanges);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 0u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 0u);
}

HWTEST_F(DNSParamCacheTest, GetVpnResolverConfigContinueTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetVpnResolverConfigContinueTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    uint32_t netId2 = 200;
    dnsParCache.CreateCacheForNet(netId2, true);
    std::vector<std::string> servers = {"8.8.8.8"};
    std::vector<std::string> domains = {"vpn.example.com"};
    dnsParCache.SetResolverConfig(netId2, 1000, 2, servers, domains);
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    dnsParCache.vpnNetId_.push_back(netId1);
    dnsParCache.vpnNetId_.push_back(netId2);
    dnsParCache.vpnUidRanges_.push_back(uidrange1);
    std::vector<NetManagerStandard::UidRange> uidRanges2;
    NetManagerStandard::UidRange uidrange2(10000, 20000, 0, netId2);
    uidRanges2.push_back(uidrange2);
    dnsParCache.vpnUidRanges_.push_back(uidrange2);
    std::vector<std::string> outServers;
    std::vector<std::string> outDomains;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t ret = dnsParCache.GetVpnResolverConfig(15000, outServers, outDomains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(outServers.size(), 1u);
    EXPECT_EQ(outServers[0], "8.8.8.8");
}

HWTEST_F(DNSParamCacheTest, GetVpnResolverConfigContinueTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("GetVpnResolverConfigContinueTest002 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    dnsParCache.vpnNetId_.push_back(netId1);
    dnsParCache.vpnUidRanges_.push_back(uidrange1);
    std::vector<std::string> outServers;
    std::vector<std::string> outDomains;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t ret = dnsParCache.GetVpnResolverConfig(15000, outServers, outDomains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedVpnServerFlagContinueTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedVpnServerFlagContinueTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    uint32_t netId2 = 200;
    dnsParCache.CreateCacheForNet(netId2, true);
    dnsParCache.SetUserDefinedServerFlag(netId2, true);
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    dnsParCache.vpnNetId_.push_back(netId1);
    dnsParCache.vpnNetId_.push_back(netId2);
    dnsParCache.vpnUidRanges_.push_back(uidrange1);
    std::vector<NetManagerStandard::UidRange> uidRanges2;
    NetManagerStandard::UidRange uidrange2(10000, 20000, 0, netId2);
    uidRanges2.push_back(uidrange2);
    dnsParCache.vpnUidRanges_.push_back(uidrange2);
    bool flag = false;
    int32_t ret = dnsParCache.GetUserDefinedVpnServerFlag(15000, flag);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(flag);
}

HWTEST_F(DNSParamCacheTest, GetUserDefinedVpnServerFlagContinueTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("GetUserDefinedVpnServerFlagContinueTest002 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    dnsParCache.vpnNetId_.push_back(netId1);
    dnsParCache.vpnUidRanges_.push_back(uidrange1);
    bool flag = false;
    int32_t ret = dnsParCache.GetUserDefinedVpnServerFlag(15000, flag);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(DNSParamCacheTest, IsUseVpnDnsContinueTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsUseVpnDnsContinueTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    uint32_t netId2 = 200;
    dnsParCache.CreateCacheForNet(netId2, true);
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    dnsParCache.vpnUidRanges_.push_back(uidrange1);
    std::vector<NetManagerStandard::UidRange> uidRanges2;
    NetManagerStandard::UidRange uidrange2(10000, 20000, 0, netId2);
    uidRanges2.push_back(uidrange2);
    dnsParCache.vpnUidRanges_.push_back(uidrange2);
    bool ret = dnsParCache.IsUseVpnDns(15000);
    EXPECT_TRUE(ret);
}

HWTEST_F(DNSParamCacheTest, IsUseVpnDnsContinueTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("IsUseVpnDnsContinueTest002 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    dnsParCache.vpnUidRanges_.push_back(uidrange1);
    bool ret = dnsParCache.IsUseVpnDns(15000);
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, VpnDnsResidueScenarioTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("VpnDnsResidueScenarioTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t netId1 = 100;
    uint32_t netId2 = 200;
    std::vector<NetManagerStandard::UidRange> uidRanges1;
    NetManagerStandard::UidRange uidrange1(10000, 20000, 0, netId1);
    uidRanges1.push_back(uidrange1);
    dnsParCache.CreateCacheForNet(netId1, true);
    std::vector<std::string> servers1 = {"1.1.1.1"};
    std::vector<std::string> domains1 = {"vpn1.example.com"};
    dnsParCache.SetResolverConfig(netId1, 500, 1, servers1, domains1);
    dnsParCache.AddUidRange(netId1, uidRanges1);
    dnsParCache.AddUidRange(netId1, uidRanges1);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 1u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 1u);
    dnsParCache.DestroyNetworkCache(netId1, true);
    dnsParCache.DelUidRange(netId1, uidRanges1);
    EXPECT_EQ(dnsParCache.vpnNetId_.size(), 0u);
    EXPECT_EQ(dnsParCache.vpnUidRanges_.size(), 0u);
    dnsParCache.CreateCacheForNet(netId2, true);
    std::vector<std::string> servers2 = {"2.2.2.2"};
    std::vector<std::string> domains2 = {"vpn2.example.com"};
    dnsParCache.SetResolverConfig(netId2, 600, 2, servers2, domains2);
    std::vector<NetManagerStandard::UidRange> uidRanges2;
    NetManagerStandard::UidRange uidrange2(10000, 20000, 0, netId2);
    uidRanges2.push_back(uidrange2);
    dnsParCache.AddUidRange(netId2, uidRanges2);
    std::vector<std::string> outServers;
    std::vector<std::string> outDomains;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t ret = dnsParCache.GetVpnResolverConfig(15000, outServers, outDomains, baseTimeoutMsec, retryCount);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(outServers.size(), 1u);
    EXPECT_EQ(outServers[0], "2.2.2.2");
}

HWTEST_F(DNSParamCacheTest, SetIpv6UidBlackListTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetIpv6UidBlackListTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::vector<int32_t> netIds;
    netIds.emplace_back(netId);
    uint32_t uid = 1;
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    dnsParCache.SetIpv6UidBlackList(netIds, uid);
    bool ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_TRUE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}
 
HWTEST_F(DNSParamCacheTest, SetIpv6UidBlackListTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("SetIpv6UidBlackListTest002 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::vector<int32_t> netIds;
    netIds.emplace_back(netId);
    uint32_t uid = 1;
    dnsParCache.SetIpv6UidBlackList(netIds, uid);
    bool ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_FALSE(ret);
}
 
HWTEST_F(DNSParamCacheTest, SetIpv6UidBlackListTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("SetIpv6UidBlackListTest003 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::vector<int32_t> netIds;
    netIds.emplace_back(netId);
    uint32_t uid = 1;
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    // Set ipv6 uid black list first time
    dnsParCache.SetIpv6UidBlackList(netIds, uid);
    bool ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_TRUE(ret);
    // Update existing uid - covers DnsResolvConfig::SetIpv6UidBlackList branch for updating existing entry
    dnsParCache.SetIpv6UidBlackList(netIds, uid);
    ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_TRUE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}
 
HWTEST_F(DNSParamCacheTest, SetIpv6UidBlackListTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("SetIpv6UidBlackListTest004 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    // Not enable IPv4, SetIpv6UidBlackList should return directly without setting Ipv6 uid black list
    // This covers DnsResolvConfig::SetIpv6UidBlackList branch: if (!IsIpv4Enable()) return;
    std::vector<int32_t> netIds;
    netIds.emplace_back(netId);
    uint32_t uid = 1;
    dnsParCache.SetIpv6UidBlackList(netIds, uid);
    bool ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_FALSE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}
 
HWTEST_F(DNSParamCacheTest, SetIpv6UidBlackListTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("SetIpv6UidBlackListTest005 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    std::vector<int32_t> netIds;
    netIds.emplace_back(netId);
    // Fill uid black list to MAX_IPV6_UID_BLACK_LIST_SIZE (32) and verify oldest is removed
    // This covers DnsResolvConfig::SetIpv6UidBlackList branch: ipv6UidBlackList_.size >= MAX_IPV6_UID_BLACK_LIST_SIZE
    for (size_t i = 0; i < MAX_IPV6_UID_BLACK_LIST_SIZE + 1; ++i) {
        uint32_t uid = i;
        dnsParCache.SetIpv6UidBlackList(netIds, uid);
    }
    // The first entry should be removed due to Ipv6 black list size limit
    uint32_t firstUid = 0;
    EXPECT_FALSE(dnsParCache.IsInIpv6UidBlackList(netId, firstUid));
    // The last entry should still exist
    uint32_t lastUid = MAX_IPV6_UID_BLACK_LIST_SIZE;
    EXPECT_TRUE(dnsParCache.IsInIpv6UidBlackList(netId, lastUid));
    dnsParCache.DestroyNetworkCache(netId);
}
 
HWTEST_F(DNSParamCacheTest, IsInIpv6UidBlackListTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInIpv6UidBlackListTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    uint32_t uid = 1;
    bool ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_FALSE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}
 
HWTEST_F(DNSParamCacheTest, IsInIpv6UidBlackListTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInIpv6UidBlackListTest002 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 1;
    bool ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_FALSE(ret);
}
 
HWTEST_F(DNSParamCacheTest, IsInIpv6UidBlackListTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInIpv6UidBlackListTest003 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    std::vector<int32_t> netIds;
    netIds.emplace_back(netId);
    dnsParCache.CreateCacheForNet(netId);
    dnsParCache.EnableIpv4(netId);
    uint32_t uid = 1;
    dnsParCache.SetIpv6UidBlackList(netIds, uid);
    bool ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_TRUE(ret);
    // Clear the cache to simulate expiration - covers DnsResolvConfig::IsInIpv6UidBlackList branch for cache not exist
    dnsParCache.FlushDnsCache(netId);
    ret = dnsParCache.IsInIpv6UidBlackList(netId, uid);
    EXPECT_FALSE(ret);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetPresetDomainsTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetPresetDomainsTest001 enter");
    DnsParamCache dnsParCache;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com", "test.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    const auto &retDomains = dnsParCache.GetPresetDomains();
    EXPECT_EQ(retDomains.size(), 1u);
    auto iter = retDomains.find(20000001);
    ASSERT_NE(iter, retDomains.end());
    EXPECT_EQ(iter->second.size(), 2u);
    EXPECT_EQ(iter->second[0], "example.com");
    EXPECT_EQ(iter->second[1], "test.com");
}

HWTEST_F(DNSParamCacheTest, SetPresetDomainsTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("SetPresetDomainsTest002 enter");
    DnsParamCache dnsParCache;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    // Overwrite with new data - covers SetPresetDomains assignment branch
    std::map<uint32_t, std::vector<std::string>> newPresetDomains;
    newPresetDomains[20000002] = {"new.example.com"};
    dnsParCache.SetPresetDomains(newPresetDomains);
    const auto &retDomains = dnsParCache.GetPresetDomains();
    EXPECT_EQ(retDomains.size(), 1u);
    EXPECT_EQ(retDomains.find(20000001), retDomains.end());
    auto iter = retDomains.find(20000002);
    ASSERT_NE(iter, retDomains.end());
    EXPECT_EQ(iter->second[0], "new.example.com");
}

HWTEST_F(DNSParamCacheTest, SetPresetDomainsTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("SetPresetDomainsTest003 enter");
    DnsParamCache dnsParCache;
    // Invalid domains are filtered out and never reside in presetDomains_
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    // Oversized domain: one char longer than DNS_DOMAIN_NAME_SIZE
    presetDomains[20000001] = {"example.com", std::string(DNS_DOMAIN_NAME_SIZE + 1, 'a')};
    // All domains invalid: the uid entry itself must be dropped
    presetDomains[20000002] = {"exam_ple.com", "example.com/path"};
    dnsParCache.SetPresetDomains(presetDomains);
    const auto &retDomains = dnsParCache.GetPresetDomains();
    EXPECT_EQ(retDomains.size(), 1u);
    EXPECT_EQ(retDomains.find(20000002), retDomains.end());
    auto iter = retDomains.find(20000001);
    ASSERT_NE(iter, retDomains.end());
    EXPECT_EQ(iter->second, std::vector<std::string>({"example.com"}));
    // The filtered domains do not match any host
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000001, std::string(DNS_DOMAIN_NAME_SIZE + 1, 'a')));
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000002, "example.com/path"));
}

HWTEST_F(DNSParamCacheTest, IsValidPresetDomainTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsValidPresetDomainTest001 enter");
    // Valid: letters, digits, '.' and '-'
    EXPECT_TRUE(DnsParamCache::IsValidPresetDomain("example.com"));
    EXPECT_TRUE(DnsParamCache::IsValidPresetDomain("www.example-1.com"));
    EXPECT_TRUE(DnsParamCache::IsValidPresetDomain(std::string(DNS_DOMAIN_NAME_SIZE, 'a')));
    // Invalid: empty, oversized, illegal characters
    EXPECT_FALSE(DnsParamCache::IsValidPresetDomain(""));
    EXPECT_FALSE(DnsParamCache::IsValidPresetDomain(std::string(DNS_DOMAIN_NAME_SIZE + 1, 'a')));
    EXPECT_FALSE(DnsParamCache::IsValidPresetDomain("exam_ple.com"));
    EXPECT_FALSE(DnsParamCache::IsValidPresetDomain("example.com/path"));
    EXPECT_FALSE(DnsParamCache::IsValidPresetDomain("example com"));
}

HWTEST_F(DNSParamCacheTest, IsPresetDomainTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsPresetDomainTest001 enter");
    DnsParamCache dnsParCache;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com", "test.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    // Exact match
    EXPECT_TRUE(dnsParCache.IsPresetDomain(20000001, "example.com"));
    // Uid not in preset domains - covers IsPresetDomain branch: uidIter == end
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000002, "example.com"));
    // Host not in preset domain list
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000001, "notpreset.com"));
}

HWTEST_F(DNSParamCacheTest, IsPresetDomainTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("IsPresetDomainTest002 enter");
    DnsParamCache dnsParCache;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    // Suffix match with dot separator - covers IsPresetDomain suffix branch
    EXPECT_TRUE(dnsParCache.IsPresetDomain(20000001, "www.example.com"));
    EXPECT_TRUE(dnsParCache.IsPresetDomain(20000001, "a.b.example.com"));
    // Trailing dot of an FQDN is trimmed before matching
    EXPECT_TRUE(dnsParCache.IsPresetDomain(20000001, "www.example.com."));
    // Plain substring must not match: preset "example.com" must not hit these hosts
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000001, "example.com.cn"));
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000001, "evil-example.com.attacker.com"));
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000001, "www.example.com.attacker.com"));
    EXPECT_FALSE(dnsParCache.IsPresetDomain(20000001, "notexample.com"));
}

HWTEST_F(DNSParamCacheTest, IsInSkipIpv6UidPerHostTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInSkipIpv6UidPerHostTest001 enter");
    DnsParamCache dnsParCache;
    // No preset domains set - covers IsInSkipIpv6UidPerHost branch: uidIter == end
    bool ret = dnsParCache.IsInSkipIpv6UidPerHost(60000, 20000001, "www.example.com");
    EXPECT_FALSE(ret);
}

HWTEST_F(DNSParamCacheTest, IsInSkipIpv6UidPerHostTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInSkipIpv6UidPerHostTest002 enter");
    DnsParamCache dnsParCache;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    // Use a netId which is not registered in net_index map, bearer check is skipped and preset domain matched
    bool ret = dnsParCache.IsInSkipIpv6UidPerHost(60000, 20000001, "www.example.com");
    EXPECT_TRUE(ret);
}

HWTEST_F(DNSParamCacheTest, IsInSkipIpv6UidPerHostTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInSkipIpv6UidPerHostTest003 enter");
    DnsParamCache dnsParCache;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    // Host not matched
    bool ret = dnsParCache.IsInSkipIpv6UidPerHost(60000, 20000001, "www.notpreset.com");
    EXPECT_FALSE(ret);
    // netId is 0, covers branch: netId = defaultNetId_
    ret = dnsParCache.IsInSkipIpv6UidPerHost(0, 20000001, "www.example.com");
    EXPECT_TRUE(ret);
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // Covers SetDnsCacheCommon with AF_INET addr and preset domain matched
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfo);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    EXPECT_EQ(retAddrInfo[0].aiFamily, AF_INET);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest002 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // Covers SetDnsCacheCommon with AF_INET6 addr and preset domain matched
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET6;
    addrInfo.aiAddr.sin6.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "fe80::1", &addrInfo.aiAddr.sin6.sin6_addr);
    AddrInfoWithTtl addrInfoWithTtl = {};
    addrInfoWithTtl.addrInfo = addrInfo;
    addrInfoWithTtl.ttl = 300;
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfoWithTtl);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    EXPECT_EQ(retAddrInfo[0].aiFamily, AF_INET6);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest003, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest003 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // NetId has no cache created - covers SetDnsCacheCommon branch: serverConfigMap_ find failed
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfo);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 0u);
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest004 enter");
    DnsParamCache dnsParCache;
    int32_t defaultNetId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    dnsParCache.SetDefaultNetwork(defaultNetId);
    dnsParCache.CreateCacheForNet(static_cast<uint16_t>(defaultNetId));
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // NetId is 0 - covers SetDnsCacheCommon branch: netId = defaultNetId_
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    dnsParCache.SetDnsCache(0, uid, hostName, addrInfo);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(static_cast<uint16_t>(defaultNetId), uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    dnsParCache.DestroyNetworkCache(static_cast<uint16_t>(defaultNetId));
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest005 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // Ttl is 0 - covers SetDnsCache(AddrInfoWithTtl) branch: ttl == 0 return
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    AddrInfoWithTtl addrInfoWithTtl = {};
    addrInfoWithTtl.addrInfo = addrInfo;
    addrInfoWithTtl.ttl = 0;
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfoWithTtl);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 0u);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest006, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest006 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // Ttl is less than DEFAULT_DELAYED_COUNT - covers SetDnsCache(AddrInfoWithTtl) branch: ttl <= DEFAULT
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    AddrInfoWithTtl addrInfoWithTtl = {};
    addrInfoWithTtl.addrInfo = addrInfo;
    addrInfoWithTtl.ttl = 1;
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfoWithTtl);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest007, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest007 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // AiFamily is neither AF_INET nor AF_INET6 - covers SetDnsCacheCommon branch: no ip write
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_UNSPEC;
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfo);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, SetDnsCachePresetDomainTest008, TestSize.Level1)
{
    NETNATIVE_LOGI("SetDnsCachePresetDomainTest008 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.notpreset.example.com";
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // Host is not preset domain - covers SetDnsCacheCommon branch: IsPresetDomain returns false
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfo);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    dnsParCache.DestroyNetworkCache(netId);
}

HWTEST_F(DNSParamCacheTest, IsInSkipIpv6UidPerHostTest004, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInSkipIpv6UidPerHostTest004 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 60000;
    uint32_t uid = 20000001;
    // Uid is in preset domains but domain list is empty - covers loop end without match
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {};
    dnsParCache.SetPresetDomains(presetDomains);
    bool ret = dnsParCache.IsInSkipIpv6UidPerHost(netId, uid, "www.example.com");
    EXPECT_FALSE(ret);
    EXPECT_FALSE(dnsParCache.IsPresetDomain(uid, "www.example.com"));
}

HWTEST_F(DNSParamCacheTest, IsInSkipIpv6UidPerHostTest005, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInSkipIpv6UidPerHostTest005 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 60001;
    uint32_t uid = 20000001;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // Write a bearer type which is neither wifi nor cellular for netId - covers
    // IsInSkipIpv6UidPerHost branch: early return false before preset domain check
    OHOS::NetManagerStandard::BpfMapper<net_index, net_interface_name_id> netIdAndIfaceMap(
        OHOS::NetManagerStandard::NET_INDEX_AND_IFACE_MAP_PATH, BPF_ANY);
    if (!netIdAndIfaceMap.IsValid()) {
        GTEST_LOG_(INFO) << "net index and iface map is invalid, skip bear type branch check";
    }
    net_index key = static_cast<net_index>(netId);
    net_interface_name_id bearType = NETWORK_BEARER_TYPE_INITIAL;
    int32_t writeRet = netIdAndIfaceMap.Write(key, bearType, BPF_ANY);
    bool ret = dnsParCache.IsInSkipIpv6UidPerHost(netId, uid, "www.example.com");
    if (writeRet == 0) {
        EXPECT_FALSE(ret);
        netIdAndIfaceMap.Delete(key);
    } else {
        // Map write failed, bear type of netId is unknown, host matches preset domain
        EXPECT_TRUE(ret);
    }
}

HWTEST_F(DNSParamCacheTest, IsInSkipIpv6UidPerHostTest006, TestSize.Level1)
{
    NETNATIVE_LOGI("IsInSkipIpv6UidPerHostTest006 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 60002;
    uint32_t uid = 20000001;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // Write a wifi bearer type for netId - covers IsInSkipIpv6UidPerHost branch:
    // bear type is wifi or cellular, fall through to preset domain check and match
    OHOS::NetManagerStandard::BpfMapper<net_index, net_interface_name_id> netIdAndIfaceMap(
        OHOS::NetManagerStandard::NET_INDEX_AND_IFACE_MAP_PATH, BPF_ANY);
    if (!netIdAndIfaceMap.IsValid()) {
        GTEST_LOG_(INFO) << "net index and iface map is invalid, skip bear type branch check";
    }
    net_index key = static_cast<net_index>(netId);
    net_interface_name_id bearType = NETWORK_BEARER_TYPE_WIFI;
    int32_t writeRet = netIdAndIfaceMap.Write(key, bearType, BPF_ANY);
    bool ret = dnsParCache.IsInSkipIpv6UidPerHost(netId, uid, "www.example.com");
    if (writeRet == 0) {
        EXPECT_TRUE(ret);
        netIdAndIfaceMap.Delete(key);
    } else {
        // Map write failed, bear type of netId is unknown, host matches preset domain
        EXPECT_TRUE(ret);
    }
}

namespace {
// The dns ip to domain map is not pinned by any component on device, create and pin it for the test.
// mapFd is set to the created map fd, or -1 when the map was already pinned by a previous test.
bool CreateOrPinDnsIpToDomainMap(const std::string &mapPath, int &mapFd)
{
    mapFd = -1;
    OHOS::NetManagerStandard::BpfMapper<dns_ip_to_domain_key, dns_ip_exists_value> existedMap(
        mapPath.c_str(), BPF_ANY);
    if (existedMap.IsValid()) {
        return true;
    }
    union bpf_attr createAttr = {};
    createAttr.map_type = BPF_MAP_TYPE_HASH;
    createAttr.key_size = sizeof(dns_ip_to_domain_key);
    createAttr.value_size = sizeof(dns_ip_exists_value);
    createAttr.max_entries = 16;
    mapFd = static_cast<int>(syscall(__NR_bpf, BPF_MAP_CREATE, &createAttr, sizeof(createAttr)));
    if (mapFd < 0) {
        return false;
    }
    union bpf_attr pinAttr = {};
    pinAttr.pathname = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(mapPath.c_str()));
    pinAttr.bpf_fd = static_cast<uint32_t>(mapFd);
    if (syscall(__NR_bpf, BPF_OBJ_PIN, &pinAttr, sizeof(pinAttr)) != 0) {
        close(mapFd);
        mapFd = -1;
        return false;
    }
    return true;
}

// Cleanup the map pinned by CreateOrPinDnsIpToDomainMap, a no-op when the map already existed
void UnlinkTestBpfMap(int mapFd, const std::string &mapPath)
{
    if (mapFd >= 0) {
        close(mapFd);
        unlink(mapPath.c_str());
    }
}
} // namespace

HWTEST_F(DNSParamCacheTest, TrySyncPresetDomainIpWithCacheKeyTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("TrySyncPresetDomainIpWithCacheKeyTest001 enter");
    DnsParamCache dnsParCache;
    uint32_t uid = 20000001;
    uint16_t netId = 1;
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    // The dns ip to domain map is not pinned by any component on host, create and pin it here
    std::string mapPath = OHOS::NetManagerStandard::DNS_IP_TO_DOMAIN_MAP_PATH;
    int mapFd = -1;
    ASSERT_TRUE(CreateOrPinDnsIpToDomainMap(mapPath, mapFd)) << "create or find dns ip to domain map failed";

    // The resolv client (MakeKey in netsys_client.c) sends the decorated cache key
    // "host serv ai_family ai_flags ai_protocol ai_socktype" as the name for SET_CACHE,
    // so the preset domain match must strip the decoration to hit "example.com"
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    addrInfo.aiAddr.sin.sin_addr.s_addr = inet_addr("10.10.10.10");
    AddrInfoWithTtl addrInfoWithTtl = {};
    addrInfoWithTtl.addrInfo = addrInfo;
    addrInfoWithTtl.ttl = 300;
    dnsParCache.SetDnsCache(netId, uid, "example.com 443 0 0 6 1", addrInfoWithTtl);

    OHOS::NetManagerStandard::BpfMapper<dns_ip_to_domain_key, dns_ip_exists_value> verifyMap(
        mapPath.c_str(), BPF_ANY);
    dns_ip_to_domain_key verifyKey = {};
    verifyKey.ipAddr = inet_addr("10.10.10.10");
    dns_ip_exists_value verifyValue = {};
    ASSERT_TRUE(verifyMap.IsValid());
    EXPECT_EQ(verifyMap.Read(verifyKey, verifyValue), 0);
    EXPECT_EQ(verifyValue.flag, 1u);

    UnlinkTestBpfMap(mapFd, mapPath);
}

HWTEST_F(DNSParamCacheTest, SetPresetDomainsClearIpMapTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetPresetDomainsClearIpMapTest001 enter");
    DnsParamCache dnsParCache;
    std::string mapPath = OHOS::NetManagerStandard::DNS_IP_TO_DOMAIN_MAP_PATH;
    int mapFd = -1;
    ASSERT_TRUE(CreateOrPinDnsIpToDomainMap(mapPath, mapFd)) << "create or find dns ip to domain map failed";

    dns_ip_to_domain_key key = {};
    key.ipAddr = inet_addr("10.10.10.10");
    dns_ip_exists_value value = {};
    value.flag = 1;
    OHOS::NetManagerStandard::BpfMapper<dns_ip_to_domain_key, dns_ip_exists_value> writeMap(
        mapPath.c_str(), BPF_ANY);
    ASSERT_TRUE(writeMap.IsValid());
    EXPECT_EQ(writeMap.Write(key, value, BPF_ANY), 0);

    // SetPresetDomains is a full replacement, stale entries must be cleared
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    OHOS::NetManagerStandard::BpfMapper<dns_ip_to_domain_key, dns_ip_exists_value> verifyMap(
        mapPath.c_str(), BPF_ANY);
    ASSERT_TRUE(verifyMap.IsValid());
    dns_ip_exists_value verifyValue = {};
    EXPECT_NE(verifyMap.Read(key, verifyValue), 0);

    // Empty preset domains (cloud config removed) also clears the map
    EXPECT_EQ(writeMap.Write(key, value, BPF_ANY), 0);
    dnsParCache.SetPresetDomains({});
    EXPECT_NE(verifyMap.Read(key, verifyValue), 0);

    UnlinkTestBpfMap(mapFd, mapPath);
}

HWTEST_F(DNSParamCacheTest, GetDnsCacheSyncPresetDomainIpTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetDnsCacheSyncPresetDomainIpTest001 enter");
    DnsParamCache dnsParCache;
    std::string mapPath = OHOS::NetManagerStandard::DNS_IP_TO_DOMAIN_MAP_PATH;
    int mapFd = -1;
    ASSERT_TRUE(CreateOrPinDnsIpToDomainMap(mapPath, mapFd)) << "create or find dns ip to domain map failed";

    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.preset.example.com";
    dnsParCache.CreateCacheForNet(netId);
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);

    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfo);

    dns_ip_to_domain_key key = {};
    key.ipAddr = inet_addr("10.10.10.10");
    OHOS::NetManagerStandard::BpfMapper<dns_ip_to_domain_key, dns_ip_exists_value> opMap(
        mapPath.c_str(), BPF_ANY);
    ASSERT_TRUE(opMap.IsValid());
    dns_ip_exists_value verifyValue = {};
    EXPECT_EQ(opMap.Read(key, verifyValue), 0);
    EXPECT_EQ(verifyValue.flag, 1u);

    // Simulate entry loss (lru eviction or map clear), cache hit re-syncs it into the map
    EXPECT_EQ(opMap.Delete(key), 0);
    EXPECT_NE(opMap.Read(key, verifyValue), 0);
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(netId, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    EXPECT_EQ(opMap.Read(key, verifyValue), 0);
    EXPECT_EQ(verifyValue.flag, 1u);

    dnsParCache.DestroyNetworkCache(netId);
    UnlinkTestBpfMap(mapFd, mapPath);
}

HWTEST_F(DNSParamCacheTest, SetPresetDomainsClearDnsCacheTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetPresetDomainsClearDnsCacheTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    // The resolv client (MakeKey in netsys_client.c) sends the decorated cache key
    // "host serv ai_family ai_flags ai_protocol ai_socktype" as the name for both SET_CACHE
    // and GET_CACHE, so the cache entries and lookups here must use the decorated form to
    // exercise the real protocol format
    std::string hostName = "www.preset.example.com 443 0 0 6 1";
    std::string normalHost = "www.normal.example.com";
    dnsParCache.CreateCacheForNet(netId);

    // Cache results (including AAAA records) while the domain is not preset
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET6;
    addrInfo.aiAddr.sin6.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "fe80::42", &addrInfo.aiAddr.sin6.sin6_addr);
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfo);
    dnsParCache.SetDnsCache(netId, uid, normalHost, addrInfo);
    EXPECT_EQ(dnsParCache.GetDnsCache(netId, uid, hostName).size(), 1u);
    EXPECT_EQ(dnsParCache.GetDnsCache(netId, uid, normalHost).size(), 1u);

    // Non-empty preset domains delivery purges the cached entries of the preset uid+domain
    // so that subsequent dns queries only do single A queries
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[uid] = {"preset.example.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    EXPECT_EQ(dnsParCache.GetDnsCache(netId, uid, hostName).size(), 0u);
    // Entries of non-preset hosts are kept
    EXPECT_EQ(dnsParCache.GetDnsCache(netId, uid, normalHost).size(), 1u);

    dnsParCache.DestroyNetworkCache(netId);
}

/**
 * @tc.number: IsPresetCacheKeyTest001
 * @tc.name: Test IsPresetCacheKey branch parsing
 * @tc.desc: Verify cache key parsing: no delimiter, delimiter at head, non-digit uid, preset hit and miss
 */
HWTEST_F(DNSParamCacheTest, IsPresetCacheKeyTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("IsPresetCacheKeyTest001 enter");
    DnsParamCache dnsParCache;
    std::map<uint32_t, std::vector<std::string>> presetDomains;
    presetDomains[20000001] = {"example.com"};
    dnsParCache.SetPresetDomains(presetDomains);
    // No delimiter - covers IsPresetCacheKey branch: find('|') == npos
    EXPECT_FALSE(dnsParCache.IsPresetCacheKey("www.example.com"));
    // Delimiter at position 0 - covers IsPresetCacheKey branch: empty uid part
    EXPECT_FALSE(dnsParCache.IsPresetCacheKey("|www.example.com"));
    // Non-digit uid part - covers IsPresetCacheKey branch: uid parsing fails
    EXPECT_FALSE(dnsParCache.IsPresetCacheKey("12a|www.example.com"));
    // Decorated resolv cache key of a preset host - covers the hit branch
    EXPECT_TRUE(dnsParCache.IsPresetCacheKey("20000001|www.example.com 443 0 0 6 1"));
    // Trailing NUL of the resolv cache key is trimmed before matching
    std::string keyWithNul = "20000001|example.com";
    keyWithNul += '\0';
    EXPECT_TRUE(dnsParCache.IsPresetCacheKey(keyWithNul));
    // Decorated resolv cache key of a non-preset host - covers the miss branch
    EXPECT_FALSE(dnsParCache.IsPresetCacheKey("20000001|www.notpreset.com 443 0 0 6 1"));
}

/**
 * @tc.number: GetDnsCacheDefaultNetIdTest001
 * @tc.name: Test GetDnsCache with netId 0
 * @tc.desc: Verify GetDnsCache falls back to defaultNetId_ when netId is 0
 */
HWTEST_F(DNSParamCacheTest, GetDnsCacheDefaultNetIdTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("GetDnsCacheDefaultNetIdTest001 enter");
    DnsParamCache dnsParCache;
    uint16_t netId = 1;
    uint32_t uid = 20000001;
    std::string hostName = "www.example.com";
    dnsParCache.SetDefaultNetwork(netId);
    dnsParCache.CreateCacheForNet(netId);
    AddrInfo addrInfo = {};
    addrInfo.aiFamily = AF_INET;
    addrInfo.aiAddr.sin.sin_family = AF_INET;
    inet_pton(AF_INET, "10.10.10.10", &addrInfo.aiAddr.sin.sin_addr);
    dnsParCache.SetDnsCache(netId, uid, hostName, addrInfo);
    // netId 0 falls back to the default network - covers GetDnsCache branch: netId = defaultNetId_
    std::vector<AddrInfo> retAddrInfo = dnsParCache.GetDnsCache(0, uid, hostName);
    EXPECT_EQ(retAddrInfo.size(), 1u);
    dnsParCache.DestroyNetworkCache(netId);
}

} // namespace NetsysNative
} // namespace OHOS
