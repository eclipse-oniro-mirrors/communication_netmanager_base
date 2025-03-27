/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

    const std::string TEST_DOMAINS(R"([{"include-subdomains": false, "name": "baidu.com"},
                                       {"include-subdomains": true, "name": "taobao.com"}])");

    const std::string TEST_PINSET(R"({
                    "expiration": "2024-8-6",
                    "pin": [
                    {"digest-algorithm": "sha256", "digest": "Q9TCQAWqP4t+eq41xnKaUgJdrPWqyG5L+Ni2YzMhqdY="},
                    {"digest-algorithm": "sha256", "digest": "Q6TCQAWqP4t+eq41xnKaUgJdrPWqyG5L+Ni2YzMhqdY="}
                    ]})");
    
    const std::string TEST_CLEARTEXT_TRAFFIC_PERMITTED(R"([{"cleartextTrafficPermitted": false}])");
} // namespace

std::shared_ptr<NetworkSecurityConfig> g_networkSecurityConfig;

class NetworkSecurityConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetworkSecurityConfigTest::SetUpTestCase() {}

void NetworkSecurityConfigTest::TearDownTestCase() {}

void NetworkSecurityConfigTest::SetUp() {}

void NetworkSecurityConfigTest::TearDown() {}

void BuildTestJsonObject(std::string &content, cJSON* &json)
{
    json = cJSON_Parse(content.c_str());
}

/**
 * @tc.name: IsCACertFileNameTest001
 * @tc.desc: Test NetworkSecurityConfig::IsCACertFileName
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, IsCACertFileNameTest001, TestSize.Level1)
{
    std::string fileName("cafile.Pem");
    std::cout << "IsCACertFileNameTest001 In" << std::endl;
    auto ret = NetworkSecurityConfig::GetInstance().IsCACertFileName(fileName.c_str());
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetCAFilesFromPathTest001
 * @tc.desc: Test NetworkSecurityConfig::GetCAFilesFromPath
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, GetCAFilesFromPathTest001, TestSize.Level1)
{
    std::string caPath("/etc/security/certificates/test");
    std::vector<std::string> caFiles;
    std::cout << "GetCAFilesFromPathTest001 In" << std::endl;
    NetworkSecurityConfig::GetInstance().GetCAFilesFromPath(caPath, caFiles);
    EXPECT_EQ(caFiles.size(), 0);
}

/**
 * @tc.name: AddSurfixToCACertFileNameTest001
 * @tc.desc: Test NetworkSecurityConfig::AddSurfixToCACertFileName
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, AddSurfixToCACertFileNameTest001, TestSize.Level1)
{
    std::string caPath("/etc/security/certificates/test");
    std::set<std::string> allFileNames;
    std::string caFile("cacert.pem");
    std::cout << "AddSurfixToCACertFileNameTest001 In" << std::endl;
    NetworkSecurityConfig::GetInstance().AddSurfixToCACertFileName(caPath, allFileNames, caFile);
    EXPECT_EQ(allFileNames.size(), 1);
}

/**
 * @tc.name: ReadCertFileTest001
 * @tc.desc: Test NetworkSecurityConfig::ReadCertFile
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ReadCertFileTest001, TestSize.Level1)
{
    std::string caFile("cacert.pem");
    std::cout << "ReadCertFileTest001 In" << std::endl;
    auto ret = NetworkSecurityConfig::GetInstance().ReadCertFile(caFile);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetRehashedCADirName001
 * @tc.desc: Test NetworkSecurityConfig::GetRehashedCADirName
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, GetRehashedCADirName001, TestSize.Level1)
{
    std::string caPath("/etc/security/certificates/test");
    std::string caPathHashValue = "eb6bf998a72433bdeb4eeb32cfa4bc15";
    std::cout << "GetRehashedCADirName001 In" << std::endl;
    auto ret = NetworkSecurityConfig::GetInstance().GetRehashedCADirName(caPath);
    EXPECT_EQ(ret, caPathHashValue);
}

/**
 * @tc.name: BuildRehasedCAPath001
 * @tc.desc: Test NetworkSecurityConfig::BuildRehasedCAPath
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, BuildRehasedCAPath001, TestSize.Level1)
{
    std::string caPath("/etc/security/certificates/test");
    std::cout << "BuildRehasedCAPath001 In" << std::endl;
    auto ret = NetworkSecurityConfig::GetInstance().BuildRehasedCAPath(caPath);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: GetRehasedCAPath001
 * @tc.desc: Test NetworkSecurityConfig::GetRehasedCAPath
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, GetRehasedCAPath001, TestSize.Level1)
{
    std::string caPath("/etc/security/certificates/test");
    std::cout << "GetRehasedCAPath001 In" << std::endl;
    auto ret = NetworkSecurityConfig::GetInstance().GetRehasedCAPath(caPath);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: ReHashCAPathForX509001
 * @tc.desc: Test NetworkSecurityConfig::ReHashCAPathForX509
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ReHashCAPathForX509001, TestSize.Level1)
{
    std::string caPath("/etc/security/certificates/test");
    std::cout << "ReHashCAPathForX509001 In" << std::endl;
    auto ret = NetworkSecurityConfig::GetInstance().ReHashCAPathForX509(caPath);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: ParseJsonTrustAnchorsTest001
 * @tc.desc: Test NetworkSecurityConfig::ParseJsonTrustAnchors, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ParseJsonTrustAnchorsTest001, TestSize.Level1)
{
    cJSON *root = nullptr;
    TrustAnchors trustAnchors;

    std::string jsonTxt(TEST_TRUST_ANCHORS);
    BuildTestJsonObject(jsonTxt, root);

    std::cout << "ParseJsonTrustAnchorsTest001 In" << std::endl;
    NetworkSecurityConfig::GetInstance().ParseJsonTrustAnchors(root, trustAnchors);
    EXPECT_EQ(trustAnchors.certs_[0], "@resource/raw/ca");
}

/**
 * @tc.name: ParseJsonDomainsTest001
 * @tc.desc: Test NetworkSecurityConfig::ParseJsonDomains, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ParseJsonDomainsTest001, TestSize.Level1)
{
    cJSON *root = nullptr;
    std::vector<Domain> domains;

    std::string jsonTxt(TEST_DOMAINS);
    BuildTestJsonObject(jsonTxt, root);

    std::cout << "ParseJsonDomainsTest001 In" << std::endl;
    NetworkSecurityConfig::GetInstance().ParseJsonDomains(root, domains);
    ASSERT_EQ(domains[0].domainName_, "baidu.com");
    ASSERT_EQ(domains[0].includeSubDomains_, false);
    ASSERT_EQ(domains[1].domainName_, "taobao.com");
    EXPECT_EQ(domains[1].includeSubDomains_, true);
}

/**
 * @tc.name: ParseJsonPinSet001
 * @tc.desc: Test NetworkSecurityConfig::ParseJsonPinSet, not applying for
 * permission,return NETMANAGER_ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ParseJsonPinSet001, TestSize.Level1)
{
    cJSON *root = nullptr;
    PinSet pinSet;

    std::string jsonTxt(TEST_PINSET);
    BuildTestJsonObject(jsonTxt, root);

    std::cout << "ParseJsonPinSet001 In" << std::endl;
    NetworkSecurityConfig::GetInstance().ParseJsonPinSet(root, pinSet);
    ASSERT_EQ(pinSet.expiration_, "2024-8-6");
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
    PinSet pinSet;

    std::string hostname("www.example.com");
    std::string pins;

    std::cout << "GetPinSetForHostName001 In" << std::endl;
    auto ret = NetworkSecurityConfig::GetInstance().GetPinSetForHostName(hostname, pins);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.name: HWTEST_F(NetworkSecurityConfigTest, ParseJsonCleartextPermitted001, TestSize.Level1)
 * @tc.desc: Test NetworkSecurityConfig::ParseJsonCleartextPermitted, not applying for
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, ParseJsonCleartextPermitted001, TestSize.Level1)
{
    cJSON *root = nullptr;
    bool cleartextPermitted;

    std::string jsonTxt(TEST_CLEARTEXT_TRAFFIC_PERMITTED);
    BuildTestJsonObject(jsonTxt, root);

    std::cout << "ParseJsonCleartextPermitted001 In" << std::endl;
    NetworkSecurityConfig::GetInstance().ParseJsonCleartextPermitted(root, cleartextPermitted);
    EXPECT_TRUE(cleartextPermitted);
}

/**
 * @tc.name: HWTEST_F(NetworkSecurityConfigTest, IsCleartextPermitted001, TestSize.Level1)
 * @tc.desc: Test NetworkSecurityConfig::IsCleartextPermitted, not applying for
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, IsCleartextPermitted001, TestSize.Level1)
{
    NetworkSecurityConfig::GetInstance().baseConfig_.cleartextTrafficPermitted_ = true;
    std::cout << "IsCleartextPermitted001 In" << std::endl;
    bool isclearpermitted;
    auto ret = NetworkSecurityConfig::GetInstance().IsCleartextPermitted(isclearpermitted);
    EXPECT_NE(ret, NETMANAGER_ERR_PERMISSION_DENIED);
    auto ret2 = NetworkSecurityConfig::GetInstance().IsCleartextPermitted("", isclearpermitted);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: HWTEST_F(NetworkSecurityConfigTest, IsCleartextPermitted002, TestSize.Level1)
 * @tc.desc: Test NetworkSecurityConfig::IsCleartextPermitted, not applying for
 * @tc.type: FUNC
 */
HWTEST_F(NetworkSecurityConfigTest, IsCleartextPermitted002, TestSize.Level1)
{
    NetworkSecurityConfig::GetInstance().baseConfig_.cleartextTrafficPermitted_ = false;
    std::vector<DomainConfig> domainConfigs;
    DomainConfig domainConfig;
    domainConfig.cleartextTrafficPermitted_ = true;
    std::vector<Domain> domains;
    Domain domain;
    domain.domainName_ = "www.text.com";
    domains.push_back(domain);
    domainConfig.domains_ = domains;
    domainConfigs.push_back(domainConfig);
    std::cout << "IsCleartextPermitted001 In" << std::endl;
    bool isclearpermitted;
    auto ret = NetworkSecurityConfig::GetInstance().IsCleartextPermitted("www.text.com", isclearpermitted);
    EXPECT_NE(ret, NETMANAGER_ERR_PERMISSION_DENIED);
    auto ret2 = NetworkSecurityConfig::GetInstance().IsCleartextPermitted("www.text2.com", isclearpermitted);
    EXPECT_EQ(ret, NETMANAGER_ERR_PERMISSION_DENIED);
}

}
}
