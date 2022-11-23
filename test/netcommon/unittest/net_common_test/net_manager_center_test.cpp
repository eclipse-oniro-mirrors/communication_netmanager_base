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

#include <gtest/gtest.h>

#include "dns_base_service.h"
#include "net_conn_base_service.h"
#include "net_conn_service_iface.h"
#include "net_conn_types.h"
#include "net_ethernet_base_service.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "net_policy_base_service.h"
#include "net_stats_base_service.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *TEST_IDENT = "testIdent";
constexpr std::initializer_list<NetBearType> BEAR_TYPE_LIST = {
    NetBearType::BEARER_CELLULAR, NetBearType::BEARER_WIFI, NetBearType::BEARER_BLUETOOTH,
    NetBearType::BEARER_ETHERNET, NetBearType::BEARER_VPN,  NetBearType::BEARER_WIFI_AWARE,
};

class TestDnsService : public DnsBaseService {
    inline int32_t GetAddressesByName(const std::string &hostName, int32_t netId,
                                      std::vector<INetAddr> &addrInfo) override
    {
        return NETMANAGER_SUCCESS;
    }
};

class TestConnService : public NetConnBaseService {
public:
    inline int32_t GetIfaceNames(NetBearType bearerType, std::list<std::string> &ifaceNames) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t GetIfaceNameByType(NetBearType bearerType, const std::string &ident, std::string &ifaceName) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t RegisterNetSupplier(NetBearType bearerType, const std::string &ident,
                                       const std::set<NetCap> &netCaps, uint32_t &supplierId) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t UnregisterNetSupplier(uint32_t supplierId) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t RestrictBackgroundChanged(bool isRestrictBackground) override
    {
        return NETMANAGER_SUCCESS;
    }
};

class TestNetEthernetService : public NetEthernetBaseService {
public:
    inline int32_t ResetEthernetFactory() override
    {
        return NETMANAGER_SUCCESS;
    }
};

class TestNetPolicyService : public NetPolicyBaseService {
public:
    inline int32_t ResetPolicies() override
    {
        return NETMANAGER_SUCCESS;
    }
    inline bool IsUidNetAllowed(uint32_t uid, bool metered) override
    {
        return NETMANAGER_SUCCESS;
    }
};

class TestNetStatsService : public NetStatsBaseService {
public:
    inline int32_t GetIfaceStatsDetail(const std::string &iface, uint32_t start, uint32_t end,
                                       NetStatsInfo &info) override
    {
        return NETMANAGER_SUCCESS;
    }
    inline int32_t ResetStatsFactory() override
    {
        return NETMANAGER_SUCCESS;
    }
};
} // namespace

class NetManagerCenterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline NetManagerCenter &instance_ = NetManagerCenter::GetInstance();
    static inline uint32_t supplierId_ = 0;
};

void NetManagerCenterTest::SetUpTestCase() {}

void NetManagerCenterTest::TearDownTestCase() {}

void NetManagerCenterTest::SetUp()
{
    instance_.RegisterConnService(nullptr);
    instance_.RegisterStatsService(nullptr);
    instance_.RegisterPolicyService(nullptr);
    instance_.RegisterEthernetService(nullptr);
    instance_.RegisterDnsService(nullptr);
}

void NetManagerCenterTest::TearDown() {}

HWTEST_F(NetManagerCenterTest, GetIfaceNamesTest001, TestSize.Level1)
{
    std::list<std::string> list;
    std::for_each(BEAR_TYPE_LIST.begin(), BEAR_TYPE_LIST.end(), [this, &list](const auto &type) {
        int32_t ret = instance_.GetIfaceNames(type, list);
        std::cout << "TYPE:" << type << "LIST_SIZE:" << list.size() << std::endl;
        EXPECT_EQ(ret, NETMANAGER_ERROR);
        EXPECT_TRUE(list.empty());
        list.clear();
    });
}

HWTEST_F(NetManagerCenterTest, GetIfaceNamesTest002, TestSize.Level1)
{
    std::list<std::string> list;
    int32_t ret = instance_.GetIfaceNames(NetBearType::BEARER_DEFAULT, list);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    EXPECT_TRUE(list.empty());
}

HWTEST_F(NetManagerCenterTest, GetIfaceNamesTest003, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    std::list<std::string> list;
    std::for_each(BEAR_TYPE_LIST.begin(), BEAR_TYPE_LIST.end(), [this, &list](const auto &type) {
        int32_t ret = instance_.GetIfaceNames(type, list);
        std::cout << "TYPE:" << type << "LIST_SIZE:" << list.size() << std::endl;
        EXPECT_EQ(ret, NETMANAGER_SUCCESS);
        EXPECT_TRUE(list.empty());
        list.clear();
    });
}

HWTEST_F(NetManagerCenterTest, GetIfaceNamesTest004, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    std::list<std::string> list;
    int32_t ret = instance_.GetIfaceNames(NetBearType::BEARER_DEFAULT, list);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_TRUE(list.empty());
}

HWTEST_F(NetManagerCenterTest, GetIfaceNameByTypeTest001, TestSize.Level1)
{
    std::string ifaceName;
    std::for_each(BEAR_TYPE_LIST.begin(), BEAR_TYPE_LIST.end(), [this, &ifaceName](const auto &type) {
        int32_t ret = instance_.GetIfaceNameByType(type, TEST_IDENT, ifaceName);
        std::cout << "TYPE:" << type << "LIST_SIZE:" << ifaceName.size() << std::endl;
        EXPECT_EQ(ret, NETMANAGER_ERROR);
        EXPECT_TRUE(ifaceName.empty());
        ifaceName.clear();
    });
}

HWTEST_F(NetManagerCenterTest, GetIfaceNameByTypeTest002, TestSize.Level1)
{
    std::string ifaceName;
    int32_t ret = instance_.GetIfaceNameByType(NetBearType::BEARER_DEFAULT, TEST_IDENT, ifaceName);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
    EXPECT_TRUE(ifaceName.empty());
}

HWTEST_F(NetManagerCenterTest, GetIfaceNameByTypeTest003, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    std::string ifaceName;
    std::for_each(BEAR_TYPE_LIST.begin(), BEAR_TYPE_LIST.end(), [this, &ifaceName](const auto &type) {
        int32_t ret = instance_.GetIfaceNameByType(type, TEST_IDENT, ifaceName);
        std::cout << "TYPE:" << type << "LIST_SIZE:" << ifaceName.size() << std::endl;
        EXPECT_EQ(ret, NETMANAGER_SUCCESS);
        EXPECT_TRUE(ifaceName.empty());
        ifaceName.clear();
    });
}

HWTEST_F(NetManagerCenterTest, GetIfaceNameByTypeTest004, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    std::string ifaceName;
    int32_t ret = instance_.GetIfaceNameByType(NetBearType::BEARER_DEFAULT, TEST_IDENT, ifaceName);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_TRUE(ifaceName.empty());
}

HWTEST_F(NetManagerCenterTest, RegisterNetSupplierTest001, TestSize.Level1)
{
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET};
    std::string ident = "ident";
    int32_t result = instance_.RegisterNetSupplier(bearerType, ident, netCaps, supplierId_);
    ASSERT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, RegisterNetSupplierTest002, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET};
    std::string ident = "ident";
    int32_t result = instance_.RegisterNetSupplier(bearerType, ident, netCaps, supplierId_);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, UnegisterNetSupplierTest001, TestSize.Level1)
{
    int32_t result = instance_.UnregisterNetSupplier(supplierId_);
    ASSERT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, UnegisterNetSupplierTest002, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    int32_t result = instance_.UnregisterNetSupplier(supplierId_);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, UpdateNetLinkInfoTest001, TestSize.Level1)
{
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};

    std::string ident = "ident04";
    uint32_t supplierId = 0;
    int32_t result = instance_.RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    ASSERT_EQ(result, NETMANAGER_ERROR);

    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    result = instance_.UpdateNetLinkInfo(supplierId, netLinkInfo);
    ASSERT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, UpdateNetLinkInfoTest002, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps = {NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};

    std::string ident = "ident04";
    uint32_t supplierId = 0;
    int32_t result = instance_.RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);

    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    result = instance_.UpdateNetLinkInfo(supplierId, netLinkInfo);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, UpdateNetSupplierInfoTest001, TestSize.Level1)
{
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};
    std::string ident = "ident03";
    uint32_t supplierId = 0;
    int32_t result = instance_.RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    ASSERT_EQ(result, NETMANAGER_ERROR);

    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo();
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    result = instance_.UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    ASSERT_EQ(result, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, UpdateNetSupplierInfoTest002, TestSize.Level1)
{
    sptr<NetConnBaseService> service = new (std::nothrow) TestConnService();
    instance_.RegisterConnService(service);
    NetBearType bearerType = BEARER_CELLULAR;
    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};
    std::string ident = "ident03";
    uint32_t supplierId = 0;
    int32_t result = instance_.RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);

    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo();
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    result = instance_.UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    ASSERT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, GetIfaceStatsDetailTest001, TestSize.Level1)
{
    std::string iface = "test_iface";
    uint32_t startTime = 0;
    uint32_t endTime = 9999999;
    NetStatsInfo info;
    int32_t ret = instance_.GetIfaceStatsDetail(iface, startTime, endTime, info);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, GetIfaceStatsDetailTest002, TestSize.Level1)
{
    sptr<NetStatsBaseService> service = new (std::nothrow) TestNetStatsService();
    instance_.RegisterStatsService(service);
    std::string iface = "test_iface";
    uint32_t startTime = 0;
    uint32_t endTime = 9999999;
    NetStatsInfo info;
    int32_t ret = instance_.GetIfaceStatsDetail(iface, startTime, endTime, info);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, ResetStatsFactoryTest001, TestSize.Level1)
{
    int32_t ret = instance_.ResetStatsFactory();
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, ResetStatsFactoryTest002, TestSize.Level1)
{
    sptr<NetStatsBaseService> service = new (std::nothrow) TestNetStatsService();
    instance_.RegisterStatsService(service);
    int32_t ret = instance_.ResetStatsFactory();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, ResetPolicyFactoryTest001, TestSize.Level1)
{
    int32_t ret = instance_.ResetPolicyFactory();
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, ResetPolicyFactoryTest002, TestSize.Level1)
{
    sptr<NetPolicyBaseService> service = new (std::nothrow) TestNetPolicyService();
    instance_.RegisterPolicyService(service);
    int32_t ret = instance_.ResetPolicyFactory();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, ResetPoliciesTest001, TestSize.Level1)
{
    int32_t ret = instance_.ResetPolicies();
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, ResetPoliciesTest002, TestSize.Level1)
{
    sptr<NetPolicyBaseService> service = new (std::nothrow) TestNetPolicyService();
    instance_.RegisterPolicyService(service);
    int32_t ret = instance_.ResetPolicies();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, ResetEthernetFactoryTest001, TestSize.Level1)
{
    int32_t ret = instance_.ResetEthernetFactory();
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, ResetEthernetFactoryTest002, TestSize.Level1)
{
    sptr<NetEthernetBaseService> service = new (std::nothrow) TestNetEthernetService();
    instance_.RegisterEthernetService(service);
    int32_t ret = instance_.ResetEthernetFactory();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, GetAddressesByNameTest001, TestSize.Level1)
{
    const std::string testHostName = "test_hostname";
    int32_t testNetId = 111;
    std::vector<INetAddr> addrInfo;
    int32_t ret = instance_.GetAddressesByName(testHostName, testNetId, addrInfo);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, GetAddressesByNameTest002, TestSize.Level1)
{
    sptr<DnsBaseService> service = new (std::nothrow) TestDnsService();
    instance_.RegisterDnsService(service);
    const std::string testHostName = "test_hostname";
    int32_t testNetId = 111;
    std::vector<INetAddr> addrInfo;
    int32_t ret = instance_.GetAddressesByName(testHostName, testNetId, addrInfo);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetManagerCenterTest, RestrictBackgroundChangedTest001, TestSize.Level1)
{
    int32_t ret = instance_.RestrictBackgroundChanged(true);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, RestrictBackgroundChangedTest002, TestSize.Level1)
{
    int32_t ret = instance_.RestrictBackgroundChanged(false);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetManagerCenterTest, IsUidNetAccessTest001, TestSize.Level1)
{
    bool ret = instance_.IsUidNetAccess(0, false);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetManagerCenterTest, IsUidNetAccessTest002, TestSize.Level1)
{
    bool ret = instance_.IsUidNetAccess(0, true);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetManagerCenterTest, IsUidNetAllowedTest001, TestSize.Level1)
{
    bool ret = instance_.IsUidNetAllowed(0, true);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetManagerCenterTest, IsUidNetAllowedTest002, TestSize.Level1)
{
    bool ret = instance_.IsUidNetAllowed(0, false);
    EXPECT_TRUE(ret);
}
} // namespace NetManagerStandard
} // namespace OHOS