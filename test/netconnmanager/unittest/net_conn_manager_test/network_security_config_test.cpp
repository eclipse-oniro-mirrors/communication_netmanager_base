/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#endif
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"
#include "net_mgr_log_wrapper.h"
#include "net_manager_constants.h"
#include "network_security_config.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
    using namespace testing::ext;

    const std::string TEST_TRUST_ANCHORS(R"([{"certificates": "@resource/raw/ca"}])");

    const std::string TEST_DOMAINS(R"({[{"include-subdomains": false, "name": "baidu.com"}]})");

    const std::string TEST_PINSET(R"({
                    "expiration": "2024-8-6",
                    "pin": [
                    {"digest-algorithm": "sha256", "digest": "Q9TCQAWqP4t+eq41xnKaUgJdrPWqyG5L+Ni2YzMhqdY="},
                    {"digest-algorithm": "sha256", "digest": "Q6TCQAWqP4t+eq41xnKaUgJdrPWqyG5L+Ni2YzMhqdY="}
                    ]})");
} // namespace

std::shared_ptr<NetworkSecurityConfig> g_networkSecurityConfig;

class NetworkSecurityConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkSecurityConfigTest::SetUpTestCase()
{
    g_networkSecurityConfig = std::make_shared<NetworkSecurityConfig>();
}

void NetworkSecurityConfigTest::TearDownTestCase() {}

void NetworkSecurityConfigTest::SetUp() {}

void NetworkSecurityConfigTest::TearDown() {}

void BuildTestJsonObject(std::string &content, Json::Value &root)
{
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    JSONCPP_STRING errs;

    reader->parse(content.c_str(), content.c_str() + content.length(), &root, &errs);
}

/**
 * @tc.name: ParseJsonTrustAnchorsTest001
 * @tc.desc: Test NetworkSecurityConfig::ParseJsonTrustAnchors, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ParseJsonTrustAnchorsTest001, TestSize.Level1)
{
    Json::Value root;
    TrustAnchors trustAnchors;

    std::string jsonTxt(TEST_TRUST_ANCHORS);
    BuildTestJsonObject(jsonTxt, root);

    std::cout << "ParseJsonTrustAnchorsTest001 In" << std::endl;
    g_networkSecurityConfig->ParseJsonTrustAnchors(root, trustAnchors);
    EXPECT_EQ(trustAnchors.certs_[0], "@resource/raw/ca");
}

/**
 * @tc.name: ParseJsonPinSet001
 * @tc.desc: Test NetworkSecurityConfig::ParseJsonPinSet, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ParseJsonPinSet001, TestSize.Level1)
{
    Json::Value root;
    PinSet pinSet;
    
    std::string jsonTxt(TEST_PINSET);
    BuildTestJsonObject(jsonTxt, root);

    std::cout << "ParseJsonPinSet001 In" << std::endl;
    g_networkSecurityConfig->ParseJsonPinSet(root, pinSet);
    ASSERT_EQ(pinSet.pins_[0].digestAlgorithm_, "sha256");
    ASSERT_EQ(pinSet.pins_[0].digest_, "Q9TCQAWqP4t+eq41xnKaUgJdrPWqyG5L+Ni2YzMhqdY=");
    ASSERT_EQ(pinSet.pins_[1].digestAlgorithm_, "sha256");
    EXPECT_EQ(pinSet.pins_[1].digest_, "Q6TCQAWqP4t+eq41xnKaUgJdrPWqyG5L+Ni2YzMhqdY=");
}

/**
 * @tc.name: HWTEST_F(NetworkSecurityConfigTest, GetPinSetForHostName001, TestSize.Level1)
 * @tc.desc: Test NetworkSecurityConfig::GetPinSetForHostName, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, GetPinSetForHostName001, TestSize.Level1)
{
    Json::Value root;
    PinSet pinSet;
    
    std::string hostname("www.example.com");
    std::string pins;

    std::cout << "GetPinSetForHostName001 In" << std::endl;
    auto ret = g_networkSecurityConfig->GetPinSetForHostName(hostname, pins);
    EXPECT_EQ(ret, NETMANAGER_ERR_INTERNAL);
}

}
}
