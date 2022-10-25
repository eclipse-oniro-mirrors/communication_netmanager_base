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

#include "message_parcel.h"
#include "net_all_capabilities.h"
#include "net_mgr_log_wrapper.h"
#include "net_specifier.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetSpecifierTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetSpecifierTest::SetUpTestCase() {}

void NetSpecifierTest::TearDownTestCase() {}

void NetSpecifierTest::SetUp() {}

void NetSpecifierTest::TearDown() {}

/**
 * @tc.name: SpecifierIsValidTest
 * @tc.desc: Test NetSpecifier::SpecifierIsValid
 * @tc.type: FUNC
 */
HWTEST_F(NetSpecifierTest, SpecifierIsValidTest, TestSize.Level1)
{
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier;
    ASSERT_TRUE(specifier != nullptr);

    specifier->netCapabilities_.netCaps_.insert(NET_CAPABILITY_INTERNET);
    bool bValid = specifier->SpecifierIsValid();
    ASSERT_TRUE(bValid == true);
}

/**
 * @tc.name: SetTypesTest
 * @tc.desc: Test NetSpecifier::SetTypes
 * @tc.type: FUNC
 */
HWTEST_F(NetSpecifierTest, SetTypesTest, TestSize.Level1)
{
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier;
    ASSERT_TRUE(specifier != nullptr);

    std::set<NetBearType> bearerTypes;
    NetBearType bearerType = NetBearType::BEARER_WIFI;
    bearerTypes.insert(bearerType);
    specifier->SetTypes(bearerTypes);
}

/**
 * @tc.name: SetTypeTest
 * @tc.desc: Test NetSpecifier::SetType
 * @tc.type: FUNC
 */
HWTEST_F(NetSpecifierTest, SetTypeTest, TestSize.Level1)
{
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier;
    ASSERT_TRUE(specifier != nullptr);

    NetBearType bearerType = NetBearType::BEARER_WIFI;
    specifier->SetType(bearerType);
}

/**
 * @tc.name: MarshallingTest01
 * @tc.desc: Test NetSpecifier::Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetSpecifierTest, MarshallingTest01, TestSize.Level1)
{
    MessageParcel dataParcel;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier;
    ASSERT_TRUE(specifier != nullptr);

    bool bRet = specifier->Marshalling(dataParcel);
    ASSERT_TRUE(bRet == true);
}

/**
 * @tc.name: MarshallingTest02
 * @tc.desc: Test static NetSpecifier::Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetSpecifierTest, MarshallingTest02, TestSize.Level1)
{
    MessageParcel dataParcel;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier;
    ASSERT_TRUE(specifier != nullptr);

    bool bRet = NetSpecifier::Marshalling(dataParcel, specifier);
    ASSERT_TRUE(bRet == true);
}

/**
 * @tc.name: UnmarshallingTest
 * @tc.desc: Test NetSpecifier::Unmarshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetSpecifierTest, UnmarshallingTest, TestSize.Level1)
{
    MessageParcel dataParcel;
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier;
    bool bRet = NetSpecifier::Marshalling(dataParcel, specifier);
    ASSERT_TRUE(bRet == true);

    sptr<NetSpecifier> specifierUnMarshalling = NetSpecifier::Unmarshalling(dataParcel);
    ASSERT_TRUE(specifierUnMarshalling != nullptr);
}

/**
 * @tc.name: ToStringTest
 * @tc.desc: Test NetSpecifier::ToString
 * @tc.type: FUNC
 */
HWTEST_F(NetSpecifierTest, ToStringTest, TestSize.Level1)
{
    sptr<NetSpecifier> specifier = new (std::nothrow) NetSpecifier;
    ASSERT_TRUE(specifier != nullptr);

    std::string str = specifier->ToString("testTab");
    NETMGR_LOG_D("netLinkInfo.ToString string is : [%{public}s]", str.c_str());
}
} // namespace NetManagerStandard
} // namespace OHOS
