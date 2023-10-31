/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "iservice_registry.h"
#include "system_ability_definition.h"

#define private public
#include "firewall_manager.h"
#undef private
#include "iptables_type.h"
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace nmd;
using namespace NetManagerStandard;
std::shared_ptr<FirewallManager> g_firewallManager = nullptr;
class FirewallManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void FirewallManagerTest::SetUpTestCase()
{
    NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, false);
    g_firewallManager = std::make_shared<FirewallManager>();
}

void FirewallManagerTest::TearDownTestCase() {}

void FirewallManagerTest::SetUp() {}

void FirewallManagerTest::TearDown()
{
    NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, {0}, FirewallRule::RULE_DENY);
}

/**
 * @tc.name: FirewallEnableChainTest001
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest001, TestSize.Level1)
{
    // CHAIN_OHFW_DOZABLE, enable
    int32_t ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, true);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallEnableChainTest002
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest002, TestSize.Level1)
{
    // CHAIN_OHFW_DOZABLE, disable
    int32_t ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallEnableChainTest003
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest003, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, enable
    int32_t ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, true);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallEnableChainTest004
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest004, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, disable
    int32_t ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, false);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallEnableChainTest005
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest005, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, disable
    int32_t ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, true);
    ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, true);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: FirewallEnableChainTest006
 * @tc.desc: Test FirewallManager FirewallEnableChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallEnableChainTest006, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, disable
    int32_t ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, false);
    ret = NetsysController::GetInstance().FirewallEnableChain(ChainType::CHAIN_OHFW_DOZABLE, false);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: FirewallSetUidRuleTest001
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest001, TestSize.Level1)
{
    NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, {0}, FirewallRule::RULE_DENY);
    // CHAIN_OHFW_DOZABLE, root, RULE_ALLOW
    int32_t ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, {0},
                                                                     FirewallRule::RULE_ALLOW);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidRuleTest002
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest002, TestSize.Level1)
{
    // CHAIN_OHFW_DOZABLE, root, RULE_DENY
    int32_t ret =
        NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, {0}, FirewallRule::RULE_DENY);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidRuleTest003
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest003, TestSize.Level1)
{
    NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, {0}, FirewallRule::RULE_ALLOW);
    // CHAIN_OHFW_UNDOZABLE, root, RULE_ALLOW
    int32_t ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, {0},
                                                                     FirewallRule::RULE_DENY);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidRuleTest004
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest004, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, root, RULE_DENY
    int32_t ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, {0},
                                                                     FirewallRule::RULE_ALLOW);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidRuleTest005
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest005, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, root, RULE_DENY
    int32_t ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, {0},
                                                                     FirewallRule::RULE_ALLOW);
    ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_DOZABLE, {0},
                                                             FirewallRule::RULE_ALLOW);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: FirewallSetUidRuleTest006
 * @tc.desc: Test FirewallManager FirewallSetUidRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidRuleTest006, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, root, RULE_DENY
    int32_t ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, {0},
                                                                     FirewallRule::RULE_DENY);
    ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, {0},
                                                             FirewallRule::RULE_DENY);
    EXPECT_EQ(ret, -1);
    ret = NetsysController::GetInstance().FirewallSetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, {0},
                                                             FirewallRule::RULE_ALLOW);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidsAllowedListChainTest001
 * @tc.desc: Test FirewallManager FirewallSetUidsAllowedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsAllowedListChainTest001, TestSize.Level1)
{
    // CHAIN_OHFW_DOZABLE, <root>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    int32_t ret =
        NetsysController::GetInstance().FirewallSetUidsAllowedListChain(ChainType::CHAIN_OHFW_DOZABLE, uids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidsAllowedListChainTest002
 * @tc.desc: Test FirewallManager FirewallSetUidsAllowedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsAllowedListChainTest002, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    int32_t ret =
        NetsysController::GetInstance().FirewallSetUidsAllowedListChain(ChainType::CHAIN_OHFW_DOZABLE, uids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidsAllowedListChainTest003
 * @tc.desc: Test FirewallManager FirewallSetUidsAllowedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsAllowedListChainTest003, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    int32_t ret =
        NetsysController::GetInstance().FirewallSetUidsAllowedListChain(ChainType::CHAIN_OHFW_UNDOZABLE, uids);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: FirewallSetUidsDeniedListChainTest001
 * @tc.desc: Test FirewallManager FirewallSetUidsDeniedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsDeniedListChainTest001, TestSize.Level1)
{
    // CHAIN_OHFW_DOZABLE, <root>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    int32_t ret =
        NetsysController::GetInstance().FirewallSetUidsDeniedListChain(ChainType::CHAIN_OHFW_UNDOZABLE, uids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidsDeniedListChainTest002
 * @tc.desc: Test FirewallManager FirewallSetUidsDeniedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsDeniedListChainTest002, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    int32_t ret =
        NetsysController::GetInstance().FirewallSetUidsDeniedListChain(ChainType::CHAIN_OHFW_UNDOZABLE, uids);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FirewallSetUidsDeniedListChainTest003
 * @tc.desc: Test FirewallManager FirewallSetUidsDeniedListChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallSetUidsDeniedListChainTest003, TestSize.Level1)
{
    // CHAIN_OHFW_UNDOZABLE, <root, system>
    std::vector<uint32_t> uids;
    uids.push_back(0);
    uids.push_back(20010034);
    int32_t ret = NetsysController::GetInstance().FirewallSetUidsDeniedListChain(ChainType::CHAIN_OHFW_DOZABLE, uids);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: FirewallManagerInnerFunctionTest
 * @tc.desc: Test FirewallManager FirewallManagerInnerFunctionTest.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FirewallManagerInnerFunctionTest, TestSize.Level1)
{
    std::shared_ptr<OHOS::nmd::FirewallManager> firewallManager = std::make_shared<OHOS::nmd::FirewallManager>();
    if (firewallManager == nullptr) {
        return;
    }
    if (firewallManager->chainInitFlag_ == false) {
        int32_t ret = firewallManager->InitChain();
        EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
        ret = firewallManager->InitDefaultRules();
        EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
    }
    ChainType chain = ChainType::CHAIN_OHFW_DOZABLE;
    FirewallType fwType = firewallManager->FetchChainType(chain);
    EXPECT_EQ((fwType == FirewallType::TYPE_ALLOWED_LIST), true);

    chain = ChainType::CHAIN_OHFW_UNDOZABLE;
    fwType = firewallManager->FetchChainType(chain);
    EXPECT_EQ((fwType == FirewallType::TYPE_DENIDE_LIST), true);

    chain = ChainType::CHAIN_NONE;
    fwType = firewallManager->FetchChainType(chain);
    EXPECT_EQ((fwType == FirewallType::TYPE_ALLOWED_LIST), true);

    int32_t ret = firewallManager->DeInitChain();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsFirewallChian1
 * @tc.desc: Test FirewallManager IsFirewallChian1.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, IsFirewallChian1, TestSize.Level1)
{
    int32_t ret = g_firewallManager->IsFirewallChian(ChainType::CHAIN_OHFW_DOZABLE);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsFirewallChian2
 * @tc.desc: Test FirewallManager IsFirewallChian2.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, IsFirewallChian2, TestSize.Level1)
{
    int32_t ret = g_firewallManager->IsFirewallChian(ChainType::CHAIN_OHFW_FORWARD);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: IsFirewallChian3
 * @tc.desc: Test FirewallManager IsFirewallChian3.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, IsFirewallChian3, TestSize.Level1)
{
    int32_t ret = g_firewallManager->IsFirewallChian(ChainType::CHAIN_OHFW_POWERSAVING);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IsFirewallChian4
 * @tc.desc: Test FirewallManager IsFirewallChian4.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, IsFirewallChian4, TestSize.Level1)
{
    int32_t ret = g_firewallManager->IsFirewallChian(ChainType::CHAIN_OHFW_UNDOZABLE);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: FetchChainName
 * @tc.desc: Test FirewallManager FetchChainName.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FetchChainName, TestSize.Level1)
{
    std::string ret = g_firewallManager->FetchChainName(ChainType::CHAIN_OHFW_INPUT);
    EXPECT_EQ(ret, "ohfw_INPUT");
    ret = g_firewallManager->FetchChainName(ChainType::CHAIN_OHFW_OUTPUT);
    EXPECT_EQ(ret, "ohfw_OUTPUT");
    ret = g_firewallManager->FetchChainName(ChainType::CHAIN_OHFW_FORWARD);
    EXPECT_EQ(ret, "ohfw_FORWARD");
    ret = g_firewallManager->FetchChainName(ChainType::CHAIN_OHFW_DOZABLE);
    EXPECT_EQ(ret, "ohfw_dozable");
    ret = g_firewallManager->FetchChainName(ChainType::CHAIN_OHFW_POWERSAVING);
    EXPECT_EQ(ret, "ohfw_powersaving");
    ret = g_firewallManager->FetchChainName(ChainType::CHAIN_OHFW_UNDOZABLE);
    EXPECT_EQ(ret, "ohfw_undozable");
    ret = g_firewallManager->FetchChainName(ChainType::CHAIN_OHBW_DATA_SAVER);
    EXPECT_EQ(ret, "oh_unusable");
}

/**
 * @tc.name: FetchChainType
 * @tc.desc: Test FirewallManager FetchChainType.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FetchChainType1, TestSize.Level1)
{
    FirewallType type = g_firewallManager->FetchChainType(ChainType::CHAIN_OHFW_DOZABLE);
    EXPECT_EQ(type, FirewallType::TYPE_ALLOWED_LIST);
}

/**
 * @tc.name: FetchChainType
 * @tc.desc: Test FirewallManager FetchChainType.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FetchChainType2, TestSize.Level1)
{
    FirewallType type = g_firewallManager->FetchChainType(ChainType::CHAIN_OHFW_POWERSAVING);
    EXPECT_EQ(type, FirewallType::TYPE_ALLOWED_LIST);
}

/**
 * @tc.name: FetchChainType
 * @tc.desc: Test FirewallManager FetchChainType.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FetchChainType3, TestSize.Level1)
{
    FirewallType type = g_firewallManager->FetchChainType(ChainType::CHAIN_OHFW_UNDOZABLE);
    EXPECT_EQ(type, FirewallType::TYPE_DENIDE_LIST);
}

/**
 * @tc.name: FetchChainType
 * @tc.desc: Test FirewallManager FetchChainType.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, FetchChainType4, TestSize.Level1)
{
    FirewallType type = g_firewallManager->FetchChainType(ChainType::CHAIN_OHBW_DATA_SAVER);
    EXPECT_EQ(type, FirewallType::TYPE_ALLOWED_LIST);
}

/**
 * @tc.name: InitChain
 * @tc.desc: Test FirewallManager InitChain.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, InitChain, TestSize.Level1)
{
    int32_t ret = g_firewallManager->InitChain();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    int32_t ret2 = g_firewallManager->DeInitChain();
    EXPECT_EQ(ret2, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: InitDefaultRules
 * @tc.desc: Test FirewallManager InitDefaultRules.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, InitDefaultRules, TestSize.Level1)
{
    int32_t ret = g_firewallManager->InitDefaultRules();
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);

    int32_t ret2 = g_firewallManager->ClearAllRules();
    EXPECT_EQ(ret2, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: InitDefaultRules
 * @tc.desc: Test FirewallManager InitDefaultRules.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, IptablesNewChain, TestSize.Level1)
{
    ChainType type = ChainType::CHAIN_OHFW_UNDOZABLE;
    int32_t ret = g_firewallManager->IptablesNewChain(type);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_SUCCESS);
}

/**
 * @tc.name: IptablesSetRule
 * @tc.desc: Test FirewallManager IptablesSetRule.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, IptablesSetRule, TestSize.Level1)
{
    std::string chainName = g_firewallManager->FetchChainName(ChainType::CHAIN_OHFW_INPUT);
    std::string option = "-A";
    std::string target = "DROP";
    uint32_t uid = 150000;
    int32_t ret = g_firewallManager->IptablesSetRule(chainName, option, target, uid);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetUidsAllowedListChain1
 * @tc.desc: Test FirewallManager SetUidsAllowedListChain1.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidsAllowedListChain1, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    uids.push_back(150000);
    int32_t ret = g_firewallManager->SetUidsAllowedListChain(ChainType::CHAIN_OHBW_DATA_SAVER, uids);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: SetUidsAllowedListChain2
 * @tc.desc: Test FirewallManager SetUidsAllowedListChain2.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidsAllowedListChain2, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    uids.push_back(150000);
    int32_t ret = g_firewallManager->SetUidsAllowedListChain(ChainType::CHAIN_OHFW_DOZABLE, uids);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetUidsDeniedListChain1
 * @tc.desc: Test FirewallManager SetUidsDeniedListChain1.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidsDeniedListChain1, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    uids.push_back(150000);
    int32_t ret = g_firewallManager->SetUidsDeniedListChain(ChainType::CHAIN_OHBW_DATA_SAVER, uids);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: SetUidsDeniedListChain2
 * @tc.desc: Test FirewallManager SetUidsDeniedListChain2.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidsDeniedListChain2, TestSize.Level1)
{
    std::vector<uint32_t> uids;
    uids.push_back(150000);
    int32_t ret = g_firewallManager->SetUidsDeniedListChain(ChainType::CHAIN_OHFW_UNDOZABLE, uids);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: EnableChain1
 * @tc.desc: Test FirewallManager EnableChain1.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, EnableChain1, TestSize.Level1)
{
    int32_t ret = g_firewallManager->EnableChain(ChainType::CHAIN_OHFW_FORWARD, false);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: EnableChain2
 * @tc.desc: Test FirewallManager EnableChain2.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, EnableChain2, TestSize.Level1)
{
    g_firewallManager->EnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, false);
    int32_t ret = g_firewallManager->EnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: EnableChain3
 * @tc.desc: Test FirewallManager EnableChain3.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, EnableChain3, TestSize.Level1)
{
    int32_t ret = g_firewallManager->EnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: EnableChain4
 * @tc.desc: Test FirewallManager EnableChain4.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, EnableChain4, TestSize.Level1)
{
    int32_t ret = g_firewallManager->EnableChain(ChainType::CHAIN_OHFW_UNDOZABLE, false);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: SetUidRule1
 * @tc.desc: Test FirewallManager SetUidRule1.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidRule1, TestSize.Level1)
{
    uint32_t uid = 150000;
    int32_t ret = g_firewallManager->SetUidRule(ChainType::CHAIN_FORWARD, uid, FirewallRule::RULE_DENY);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.name: SetUidRule2
 * @tc.desc: Test FirewallManager SetUidRule2.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidRule2, TestSize.Level1)
{
    uint32_t uid = 150000;
    int32_t ret = g_firewallManager->SetUidRule(ChainType::CHAIN_OHFW_DOZABLE, uid, FirewallRule::RULE_DENY);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetUidRule3
 * @tc.desc: Test FirewallManager SetUidRule3.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidRule3, TestSize.Level1)
{
    uint32_t uid = 150000;
    int32_t ret = g_firewallManager->SetUidRule(ChainType::CHAIN_OHFW_DOZABLE, uid, FirewallRule::RULE_ALLOW);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetUidRule4
 * @tc.desc: Test FirewallManager SetUidRule4.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidRule4, TestSize.Level1)
{
    uint32_t uid = 160000;
    int32_t ret = g_firewallManager->SetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, uid, FirewallRule::RULE_DENY);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: SetUidRule5
 * @tc.desc: Test FirewallManager SetUidRule5.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, SetUidRule5, TestSize.Level1)
{
    uint32_t uid = 160000;
    int32_t ret = g_firewallManager->SetUidRule(ChainType::CHAIN_OHFW_UNDOZABLE, uid, FirewallRule::RULE_ALLOW);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: ClearAllRules
 * @tc.desc: Test FirewallManager ClearAllRules.
 * @tc.type: FUNC
 */
HWTEST_F(FirewallManagerTest, ClearAllRules, TestSize.Level1)
{
    int32_t ret = g_firewallManager->ClearAllRules();
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
} // namespace NetsysNative
} // namespace OHOS
