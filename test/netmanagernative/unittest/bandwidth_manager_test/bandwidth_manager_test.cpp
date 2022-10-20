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

#include "conn_manager_test.h"
#include "iptables_type.h"
#include "iservice_registry.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_proxy.h"

#include "system_ability_definition.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace ConnGetProxy;
using namespace NetManagerStandard;
class BandwidthManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void BandwidthManagerTest::SetUpTestCase() {}

void BandwidthManagerTest::TearDownTestCase() {}

void BandwidthManagerTest::SetUp() {}

void BandwidthManagerTest::TearDown() {}

/**
 * @tc.name: BandwidthEnableDataSaverTest001
 * @tc.desc: Test BandwidthManager BandwidthEnableDataSaver.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthEnableDataSaverTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // enable
    ret = netsysNativeService->BandwidthEnableDataSaver(true);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthEnableDataSaverTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthEnableDataSaverTest002
 * @tc.desc: Test BandwidthManager BandwidthEnableDataSaver.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthEnableDataSaverTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // disable
    ret = netsysNativeService->BandwidthEnableDataSaver(false);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthEnableDataSaverTest002");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthEnableDataSaverTest003
 * @tc.desc: Test BandwidthManager BandwidthEnableDataSaver.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthEnableDataSaverTest003, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // disable
    ret = netsysNativeService->BandwidthEnableDataSaver(true);
    ret = netsysNativeService->BandwidthEnableDataSaver(true);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthEnableDataSaverTest003");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: BandwidthEnableDataSaverTest004
 * @tc.desc: Test BandwidthManager BandwidthEnableDataSaver.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthEnableDataSaverTest004, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // disable
    ret = netsysNativeService->BandwidthEnableDataSaver(false);
    ret = netsysNativeService->BandwidthEnableDataSaver(false);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthEnableDataSaverTest004");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: BandwidthSetIfaceQuotaTest001
 * @tc.desc: Test BandwidthManager BandwidthSetIfaceQuota.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthSetIfaceQuotaTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // iface0
    ret = netsysNativeService->BandwidthSetIfaceQuota("iface0", 2097152);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthSetIfaceQuotaTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthSetIfaceQuotaTest002
 * @tc.desc: Test BandwidthManager BandwidthSetIfaceQuota.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthSetIfaceQuotaTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // wlan0
    ret = netsysNativeService->BandwidthSetIfaceQuota("wlan0", 2097152);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthSetIfaceQuotaTest002");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthSetIfaceQuotaTest003
 * @tc.desc: Test BandwidthManager BandwidthSetIfaceQuota.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthSetIfaceQuotaTest003, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // wlan0
    ret = netsysNativeService->BandwidthSetIfaceQuota("wlan0", 5000000000000000);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthSetIfaceQuotaTest003");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthSetIfaceQuotaTest004
 * @tc.desc: Test BandwidthManager BandwidthSetIfaceQuota.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthSetIfaceQuotaTest004, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // wlan0
    ret = netsysNativeService->BandwidthSetIfaceQuota("wlan0", 5000000000000);
    ret = netsysNativeService->BandwidthSetIfaceQuota("wlan0", 2097152);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthSetIfaceQuotaTest004");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthRemoveIfaceQuotaTest001
 * @tc.desc: Test BandwidthManager BandwidthRemoveIfaceQuota.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthRemoveIfaceQuotaTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // iface0
    ret = netsysNativeService->BandwidthRemoveIfaceQuota("iface0");
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthRemoveIfaceQuotaTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthRemoveIfaceQuotaTest002
 * @tc.desc: Test BandwidthManager BandwidthRemoveIfaceQuota.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthRemoveIfaceQuotaTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // wlan0
    ret = netsysNativeService->BandwidthRemoveIfaceQuota("wlan0");
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthRemoveIfaceQuotaTest002");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthAddDeniedListTest001
 * @tc.desc: Test BandwidthManager BandwidthAddDeniedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthAddDeniedListTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthAddDeniedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthAddDeniedListTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthAddDeniedListTest002
 * @tc.desc: Test BandwidthManager BandwidthAddDeniedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthAddDeniedListTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthAddDeniedList(0);
    ret = netsysNativeService->BandwidthAddDeniedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthAddDeniedListTest002");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: BandwidthRemoveDeniedListTest001
 * @tc.desc: Test BandwidthManager BandwidthRemoveDeniedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthRemoveDeniedListTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthRemoveDeniedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthRemoveDeniedListTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthRemoveDeniedListTest002
 * @tc.desc: Test BandwidthManager BandwidthRemoveDeniedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthRemoveDeniedListTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthRemoveDeniedList(0);
    ret = netsysNativeService->BandwidthRemoveDeniedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthRemoveDeniedListTest002");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: BandwidthAddAllowedListTest001
 * @tc.desc: Test BandwidthManager BandwidthAddAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthAddAllowedListTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthAddAllowedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthAddAllowedListTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthAddAllowedListTest002
 * @tc.desc: Test BandwidthManager BandwidthAddAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthAddAllowedListTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthAddAllowedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthAddAllowedListTest002");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: BandwidthRemoveAllowedListTest001
 * @tc.desc: Test BandwidthManager BandwidthRemoveAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthRemoveAllowedListTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthRemoveAllowedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthRemoveAllowedListTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: BandwidthRemoveAllowedListTest002
 * @tc.desc: Test BandwidthManager BandwidthRemoveAllowedList.
 * @tc.type: FUNC
 */
HWTEST_F(BandwidthManagerTest, BandwidthRemoveAllowedListTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = ConnManagerGetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // root
    ret = netsysNativeService->BandwidthRemoveAllowedList(0);
    ret = netsysNativeService->BandwidthRemoveAllowedList(0);
    NETNATIVE_LOG_D("BandwidthManagerTest BandwidthRemoveAllowedListTest002");
    EXPECT_TRUE(ret == -1);
}
} // namespace NetsysNative
} // namespace OHOS
