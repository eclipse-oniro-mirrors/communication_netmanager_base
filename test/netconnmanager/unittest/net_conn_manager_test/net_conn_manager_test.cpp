/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "net_conn_callback_test.h"
#include "net_conn_client.h"
#include "net_conn_types.h"
#include "net_detection_callback_test.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int WAIT_TIME_SECOND_LONG = 60;
constexpr int WAIT_TIME_SECOND_NET_DETECTION = 2;
using namespace testing::ext;
class NetConnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetLinkInfo> GetUpdateLinkInfoSample() const;

    sptr<NetConnCallbackTest> GetINetConnCallbackSample() const;

    sptr<NetDetectionCallbackTest> GetINetDetectionCallbackSample() const;
};

void NetConnManagerTest::SetUpTestCase() {}

void NetConnManagerTest::TearDownTestCase() {}

void NetConnManagerTest::SetUp() {}

void NetConnManagerTest::TearDown() {}

sptr<NetLinkInfo> NetConnManagerTest::GetUpdateLinkInfoSample() const
{
    sptr<NetLinkInfo> netLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    netLinkInfo->ifaceName_ = "test";
    netLinkInfo->domain_ = "test";

    sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
    netAddr->type_ = INetAddr::IPV4;
    netAddr->family_ = 0x10;
    netAddr->prefixlen_ = 0x17;
    netAddr->address_ = "192.168.2.0";
    netAddr->netMask_ = "192.255.255.255";
    netAddr->hostName_ = "netAddr";
    netLinkInfo->netAddrList_.push_back(*netAddr);

    sptr<INetAddr> dns = (std::make_unique<INetAddr>()).release();
    dns->type_ = INetAddr::IPV4;
    dns->family_ = 0x10;
    dns->prefixlen_ = 0x17;
    dns->address_ = "192.168.2.0";
    dns->netMask_ = "192.255.255.255";
    dns->hostName_ = "netAddr";
    netLinkInfo->dnsList_.push_back(*dns);

    sptr<Route> route = (std::make_unique<Route>()).release();
    route->iface_ = "iface0";
    route->destination_.type_ = INetAddr::IPV4;
    route->destination_.family_ = 0x10;
    route->destination_.prefixlen_ = 0x17;
    route->destination_.address_ = "192.168.2.0";
    route->destination_.netMask_ = "192.255.255.255";
    route->destination_.hostName_ = "netAddr";
    route->gateway_.type_ = INetAddr::IPV4;
    route->gateway_.family_ = 0x10;
    route->gateway_.prefixlen_ = 0x17;
    route->gateway_.address_ = "192.168.2.0";
    route->gateway_.netMask_ = "192.255.255.255";
    route->gateway_.hostName_ = "netAddr";
    netLinkInfo->routeList_.push_back(*route);

    netLinkInfo->mtu_ = 0x5DC;

    return netLinkInfo;
}

sptr<NetConnCallbackTest> NetConnManagerTest::GetINetConnCallbackSample() const
{
    sptr<NetConnCallbackTest> callback = (std::make_unique<NetConnCallbackTest>()).release();
    return callback;
}

sptr<NetDetectionCallbackTest> NetConnManagerTest::GetINetDetectionCallbackSample() const
{
    sptr<NetDetectionCallbackTest> detectionCallback = (std::make_unique<NetDetectionCallbackTest>()).release();
    return detectionCallback;
}

/**
 * @tc.name: NetConnManager001
 * @tc.desc: Test NetConnManager SystemReady.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager001, TestSize.Level1)
{
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->SystemReady();
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: NetConnManager002
 * @tc.desc: Test NetConnManager RegisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager002, TestSize.Level1)
{
    uint64_t netCapabilities = 0x00;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident01";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR, ident,
        netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager003
 * @tc.desc: Test NetConnManager UnregisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager003, TestSize.Level1)
{
    uint64_t netCapabilities = 0x01;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident02";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);

    result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetSupplier(supplierId);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager004
 * @tc.desc: Test NetConnManager UpdateNetSupplierInfo.
 * @tc.type: FUNC
 */

HWTEST_F(NetConnManagerTest, NetConnManager004, TestSize.Level1)
{
    uint64_t netCapabilities = 0x02;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident03";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);

    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo;
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    result = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetSupplierInfo(supplierId,
        netSupplierInfo);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager005
 * @tc.desc: Test NetConnManager UpdateNetLinkInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager005, TestSize.Level1)
{
    uint64_t netCapabilities = 0x03;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident04";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);

    sptr<NetLinkInfo> netLinkInfo = GetUpdateLinkInfoSample();
    result = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager006
 * @tc.desc: Test NetConnManager RegisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager006, TestSize.Level1)
{
    uint64_t netCapabilities = 0x02;
    netCapabilities |= NET_CAPABILITIES_INTERNET;

    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);

    sptr<NetConnCallbackTest> callback = GetINetConnCallbackSample();
    result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    if (result == ERR_NONE) {
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        int32_t netState = callback->GetNetState();
        std::cout << "NetConnManager006 RegisterNetConnCallback netState:" << netState << std::endl;
        ASSERT_GT(netState, 0);
    } else {
        std::cout << "NetConnManager006 RegisterNetConnCallback return fail" << std::endl;
    }

    result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager007
 * @tc.desc: Test NetConnManager RegisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager007, TestSize.Level1)
{
    uint64_t netCapabilities = 0x02;
    netCapabilities |= NET_CAPABILITIES_INTERNET;

    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);

    sptr<NetSpecifier> netSpecifier = (std::make_unique<NetSpecifier>()).release();
    netSpecifier->ident_ = "ident";
    netSpecifier->netType_ = NET_TYPE_CELLULAR;
    netSpecifier->netCapabilities_ = NET_CAPABILITIES_INTERNET;

    sptr<NetConnCallbackTest> callback = GetINetConnCallbackSample();
    result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback);
    if (result == ERR_NONE) {
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        int32_t netState = callback->GetNetState();
        std::cout << "NetConnManager007 RegisterNetConnCallback netState:" << netState << std::endl;
        ASSERT_GT(netState, 0);
    } else {
        std::cout << "NetConnManager007 RegisterNetConnCallback return fail" << std::endl;
    }

    result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(netSpecifier, callback);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager008
 * @tc.desc: Test NetConnManager RegisterNetDetectionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager008, TestSize.Level1)
{
    int32_t netType = static_cast<int32_t>(NetworkType::NET_TYPE_WIFI);
    std::list<int32_t> netIdList;
    DelayedSingleton<NetConnClient>::GetInstance()->GetSpecificNet(netType, netIdList);

    int32_t result = 0;
    sptr<NetDetectionCallbackTest> callback = GetINetDetectionCallbackSample();
    if (callback == nullptr) {
        return;
    }
    for (int32_t netId : netIdList) {
        result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetDetectionCallback(netId, callback);
        ASSERT_TRUE(result == ERR_NONE);
        std::cout << "TestRegisterNetDetectionCallback netId:" << netId << "result:" << result << std::endl;
        result = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(netId);
        ASSERT_TRUE(result == ERR_NONE);
        std::cout << "TestNetDetection result:" << result << std::endl;
        callback->WaitFor(WAIT_TIME_SECOND_NET_DETECTION);
        int32_t netDetectionRet = callback->GetNetDetectionResult();
        std::cout << "RegisterNetDetectionCallback netDetectionRet:" << netDetectionRet << std::endl;
        std::string urlRedirect = callback->GetUrlRedirect();
        std::cout << "RegisterNetDetectionCallback urlRedirect:" << urlRedirect << std::endl;

        result = DelayedSingleton<NetConnClient>::GetInstance()->UnRegisterNetDetectionCallback(netId, callback);
        ASSERT_TRUE(result == ERR_NONE);
        std::cout << "TestUnRegisterNetDetectionCallback result:" << result << std::endl;
        result = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(netId);
        ASSERT_TRUE(result == ERR_NONE);
        std::cout << "TestNetDetection result:" << result << std::endl;
        callback->WaitFor(WAIT_TIME_SECOND_NET_DETECTION);
        netDetectionRet = callback->GetNetDetectionResult();
        ASSERT_TRUE(netDetectionRet == static_cast<int32_t>(NetDetectionResultCode::NET_DETECTION_FAIL));
        std::cout << "RegisterNetDetectionCallback netDetectionRet:" << netDetectionRet << std::endl;
        urlRedirect = callback->GetUrlRedirect();
        ASSERT_TRUE(urlRedirect.empty());
        std::cout << "RegisterNetDetectionCallback urlRedirect:" << urlRedirect << std::endl;
    }
}

/**
 * @tc.name: NetConnManager010
 * @tc.desc: Test NetConnManager GetIfaceNameByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager010, TestSize.Level1)
{
    uint64_t netCapabilities = 0x03;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);

    sptr<NetLinkInfo> netLinkInfo = GetUpdateLinkInfoSample();
    result = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    ASSERT_TRUE(result == ERR_NONE);
    std::string ifaceName;
    result = DelayedSingleton<NetConnClient>::GetInstance()->GetIfaceNameByType(NET_TYPE_CELLULAR, ident, ifaceName);
    ASSERT_TRUE(result == ERR_NONE);
    ASSERT_TRUE(ifaceName == "test");
}

/**
 * @tc.name: NetConnManager011
 * @tc.desc: Test NetConnManager GetSpecificNet.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager011, TestSize.Level1)
{
    uint64_t netCapabilities = 0x03;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident";
    uint32_t supplierId1 = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId1);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId1 : " << supplierId1 << std::endl;

    netCapabilities = 0x02;
    ident = "ident2";
    uint32_t supplierId2 = 0;
    result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId2);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId2 : " << supplierId2 << std::endl;

    netCapabilities = 0x01;
    ident = "ident3";
    uint32_t supplierId3 = 0;
    result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId3);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId3 : " << supplierId3 << std::endl;

    std::list<int32_t> netIdList;
    result = DelayedSingleton<NetConnClient>::GetInstance()->GetSpecificNet(NET_TYPE_CELLULAR, netIdList);
    ASSERT_TRUE(result == ERR_NONE);
    for (auto it : netIdList) {
        std::cout << "netid = " << it << std::endl;
    }
}

/**
 * @tc.name: NetConnManager012
 * @tc.desc: Test NetConnManager GetAllNets.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager012, TestSize.Level1)
{
    uint64_t netCapabilities = 0x03;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident";
    uint32_t supplierId1 = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId1);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId1 : " << supplierId1 << std::endl;

    netCapabilities = 0x02;
    ident = "ident2";
    uint32_t supplierId2 = 0;
    result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_ETHERNET,
        ident, netCapabilities, supplierId2);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId2 : " << supplierId2 << std::endl;

    netCapabilities = 0x01;
    ident = "ident3";
    uint32_t supplierId3 = 0;
    result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId3);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId3 : " << supplierId3 << std::endl;

    std::list<int32_t> netIdList;
    result = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(netIdList);
    ASSERT_TRUE(result == ERR_NONE);
    for (auto it : netIdList) {
        std::cout << "netid = " << it << std::endl;
    }
}

/**
 * @tc.name: NetConnManager013
 * @tc.desc: Test NetConnManager GetNetCapabilities.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager013, TestSize.Level1)
{
    uint64_t netCapabilities = 0x03;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId : " << supplierId << std::endl;

    std::list<int32_t> netIdList;
    result = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(netIdList);
    ASSERT_TRUE(result == ERR_NONE);
    for (auto it : netIdList) {
        std::cout << "netid = " << it << std::endl;
        uint64_t cap;
        result = DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(it, cap);
        switch (cap) {
            case NET_CAPABILITIES_NONE:
                std::cout << "NET_CAPABILITIES_NONE" << std::endl;
                break;
            case NET_CAPABILITIES_INTERNET:
                std::cout << "NET_CAPABILITIES_INTERNET" << std::endl;
                break;
            case NET_CAPABILITIES_MMS:
                std::cout << "NET_CAPABILITIES_MMS" << std::endl;
                break;
            default:
                std::cout << "unknown NET_CAPABILITIES" << std::endl;
        }
    }
}

/**
 * @tc.name: NetConnManager014
 * @tc.desc: Test NetConnManager GetConnectionProperties.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager014, TestSize.Level1)
{
    uint64_t netCapabilities = 0x02;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident";
    uint32_t supplierId = 0;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities, supplierId);
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << "supplierId : " << supplierId << std::endl;

    sptr<NetLinkInfo> netLinkInfo = GetUpdateLinkInfoSample();
    result = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
    ASSERT_TRUE(result == ERR_NONE);

    // 现在问题是我们只知道supplierId，但是不知道netId，GetSpecificNet和GetAllNet接口都返回netId的list
    // GetNetworkFromListBysupplierId 根据supplierId获取network可以取得对应的netId，但这个接口不对外暴露
    // 而且只有针对每一个supplier的注册和注销接口，没有整体全部注销的接口(由于netId是从-1开始递增的，所以知晓了有多少supplier，
    // 似乎可以知道下一个netId是多少)
    // 测试阶段重启netmanager 那么目前只注册了一个supplier，netId应该为1
    NetLinkInfo info;
    result = DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(1, info);
    std::cout << "result = " << result << std::endl;
    ASSERT_TRUE(result == ERR_NONE);
    std::cout << info.ToString("\n") << std::endl;
}
} // namespace NetManagerStandard
} // namespace OHOS
