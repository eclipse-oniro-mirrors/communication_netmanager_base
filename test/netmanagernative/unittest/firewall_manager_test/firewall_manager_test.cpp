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

#include "iptables_type.h"
#include "iservice_registry.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_proxy.h"

#include "system_ability_definition.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace NetManagerStandard;
class FirewallManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void FirewallManagerTest::SetUpTestCase() {}

void FirewallManagerTest::TearDownTestCase() {}

void FirewallManagerTest::SetUp() {}

void FirewallManagerTest::TearDown() {}

sptr<INetsysService> GetProxy()
{
    NETNATIVE_LOGI("Get samgr >>>>>>>>>>>>>>>>>>>>>>>>>>");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    NETNATIVE_LOGI("Get samgr %{public}p", samgr.GetRefPtr());
    std::cout << "Get samgr  "<< samgr.GetRefPtr() << std::endl;

    auto remote = samgr->GetSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    NETNATIVE_LOGI("Get remote %{public}p", remote.GetRefPtr());
    std::cout << "Get remote "<< remote.GetRefPtr() << std::endl;

    auto proxy = iface_cast<NetsysNative::INetsysService>(remote);
    NETNATIVE_LOGI("Get proxy %{public}p", proxy.GetRefPtr());
    std::cout << "Get proxy "<<proxy.GetRefPtr()<<std::endl;

    return proxy;
}

/**
 * @tc.name: FirewallEnableChainTest001
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_DOZABLE, enable
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, true);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallEnableChainTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallEnableChainTest002
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_DOZABLE, disable
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, false);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallEnableChainTest002");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallEnableChainTest003
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest003, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, enable
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, true);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallEnableChainTest003");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallEnableChainTest004
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest004, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, disable
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, false);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallEnableChainTest004");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallEnableChainTest005
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest005, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, disable
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, true);
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, true);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallEnableChainTest005");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: FirewallEnableChainTest006
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest006, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, disable
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, false);
    ret = netsysNativeService->FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, false);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallEnableChainTest006");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: FirewallSetUidRuleTest001
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_DOZABLE, root, RULE_ALLOW
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, 0, FirewallRule::RULE_ALLOW);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidRuleTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidRuleTest002
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_DOZABLE, root, RULE_DENY
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, 0, FirewallRule::RULE_DENY);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidRuleTest002");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidRuleTest003
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest003, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, root, RULE_ALLOW
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, 0, FirewallRule::RULE_DENY);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidRuleTest003");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidRuleTest004
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest004, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, root, RULE_DENY
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, 0, FirewallRule::RULE_ALLOW);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidRuleTest004");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidRuleTest005
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest005, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, root, RULE_DENY
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, 0, FirewallRule::RULE_ALLOW);
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, 0, FirewallRule::RULE_ALLOW);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidRuleTest005");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: FirewallSetUidRuleTest006
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest006, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, root, RULE_DENY
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, 0, FirewallRule::RULE_DENY);
    ret = netsysNativeService->FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, 0, FirewallRule::RULE_DENY);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidRuleTest006");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: FirewallSetUidsAllowedListChainTest001
 * @tc.desc: Test FirewallManager FirewallSetUidsAllowedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsAllowedListChainTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_DOZABLE, <root>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    ret = netsysNativeService->FirewallSetUidsAllowedListChain(ChainType::CHAIN_OHFW_DOZABLE, uids);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidsAllowedListChainTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidsAllowedListChainTest002
 * @tc.desc: Test FirewallManager FirewallSetUidsAllowedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsAllowedListChainTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    ret = netsysNativeService->FirewallSetUidsAllowedListChain(ChainType::CHAIN_OHFW_DOZABLE, uids);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidsAllowedListChainTest002");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidsAllowedListChainTest003
 * @tc.desc: Test FirewallManager FirewallSetUidsAllowedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsAllowedListChainTest003, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    ret = netsysNativeService->FirewallSetUidsAllowedListChain(ChainType::CHAIN_OHFW_UNDOZABLE, uids);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidsAllowedListChainTest003");
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: FirewallSetUidsDeniedListChainTest001
 * @tc.desc: Test FirewallManager FirewallSetUidsDeniedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsDeniedListChainTest001, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_DOZABLE, <root>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    ret = netsysNativeService->FirewallSetUidsDeniedListChain(ChainType::CHAIN_OHFW_UNDOZABLE, uids);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidsDeniedListChainTest001");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidsDeniedListChainTest002
 * @tc.desc: Test FirewallManager FirewallSetUidsDeniedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsDeniedListChainTest002, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    ret = netsysNativeService->FirewallSetUidsDeniedListChain(ChainType::CHAIN_OHFW_UNDOZABLE, uids);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidsDeniedListChainTest002");
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: FirewallSetUidsDeniedListChainTest003
 * @tc.desc: Test FirewallManager FirewallSetUidsDeniedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsDeniedListChainTest003, TestSize.Level1)
{
    OHOS::sptr<OHOS::NetsysNative::INetsysService> netsysNativeService = GetProxy();
    if (netsysNativeService == nullptr) {
        std::cout << "netsysNativeService is nullptr" << std::endl;
        EXPECT_FALSE(0);
    }

    int32_t ret = 0;
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    ret = netsysNativeService->FirewallSetUidsDeniedListChain(ChainType::CHAIN_OHFW_DOZABLE, uids);
    NETNATIVE_LOG_D("FirewallManagerTest FirewallSetUidsDeniedListChainTest003");
    EXPECT_TRUE(ret == -1);
}
} // namespace NetsysNative
} // namespace OHOS
