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

#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr const char *TEST_TEXT = "adfsjfjkfk^#$ajf!@!#$#kjd nck?fgnf<kdjnf>kjask?.fcnvdkjfn kjdkj.,.vd";
constexpr const char *SPLIT = "?";
constexpr const char *TEST_IP = "155.153.144.154";
constexpr const char *TEST_IPV4 = "534/6::45/144.15:4::44";
constexpr uint32_t ADDREDD_LEN = 16;
} // namespace
class UtNetmanagerBaseCommon : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void UtNetmanagerBaseCommon::SetUpTestCase() {}

void UtNetmanagerBaseCommon::TearDownTestCase() {}

void UtNetmanagerBaseCommon::SetUp() {}

void UtNetmanagerBaseCommon::TearDown() {}

/**
 * @tc.name: UtNetmanagerBaseCommon001
 * @tc.desc: Test UtNetmanagerBaseCommon ForkExec.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, UtNetmanagerBaseCommon001, TestSize.Level1)
{
    std::string out;
    CommonUtils::ForkExec("/system/bin/ls -a", &out);
    ASSERT_FALSE(out.empty());
    std::cout << "out: " << out << std::endl;
}

/**
 * @tc.name: UtNetmanagerBaseCommon002
 * @tc.desc: Test UtNetmanagerBaseCommon ForkExec.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, UtNetmanagerBaseCommon002, TestSize.Level1)
{
    std::string out;
    CommonUtils::ForkExec("/system/bin/ls -l", &out);
    ASSERT_FALSE(out.empty());
    std::cout << "out: " << out << std::endl;
}

/**
 * @tc.name: UtNetmanagerBaseCommon003
 * @tc.desc: Test UtNetmanagerBaseCommon ForkExec.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, UtNetmanagerBaseCommon003, TestSize.Level1)
{
    CommonUtils::ForkExec("/system/bin/mount -o rw,remount /");
    CommonUtils::ForkExec("/system/bin/mkdir uttest");
    std::string out;
    CommonUtils::ForkExec("/system/bin/ls -a", &out);
    ASSERT_TRUE(out.find("uttest") != std::string::npos);
    CommonUtils::ForkExec("/system/bin/rm -rf uttest");
}

/**
 * @tc.name: SplitTest001
 * @tc.desc: Test UtNetmanagerBaseCommon Split.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, SplitTest001, TestSize.Level1)
{
    std::vector<std::string> result = CommonUtils::Split(TEST_TEXT, SPLIT);
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: SplitTest002
 * @tc.desc: Test UtNetmanagerBaseCommon Split.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, SplitTest002, TestSize.Level1)
{
    std::vector<std::string> result = CommonUtils::Split({}, SPLIT);
    ASSERT_TRUE(result.empty());
}

/**
 * @tc.name: StripTest001
 * @tc.desc: Test UtNetmanagerBaseCommon Strip.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StripTest001, TestSize.Level1)
{
    auto result = CommonUtils::Strip(TEST_TEXT, '?');
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: IsValidIPV4Test001
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV4.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV4Test001, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV4(TEST_TEXT);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidIPV4Test002
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV4.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV4Test002, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV4({});
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidIPV6Test001
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV6.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV6Test001, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV6(TEST_TEXT);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: IsValidIPV6Test002
 * @tc.desc: Test UtNetmanagerBaseCommon IsValidIPV6.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, IsValidIPV6Test002, TestSize.Level1)
{
    auto result = CommonUtils::IsValidIPV6({});
    ASSERT_FALSE(result);
}

/**
 * @tc.name: GetAddrFamilyTest001
 * @tc.desc: Test UtNetmanagerBaseCommon GetAddrFamily.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, GetAddrFamilyTest001, TestSize.Level1)
{
    auto result = CommonUtils::GetAddrFamily(TEST_IP);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: GetMaskLengthTest001
 * @tc.desc: Test UtNetmanagerBaseCommon GetMaskLength.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, GetMaskLengthTest001, TestSize.Level1)
{
    auto result = CommonUtils::GetMaskLength(TEST_TEXT);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: ConvertIpv4AddressTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest001, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(0);
    ASSERT_TRUE(result.empty());
}

/**
 * @tc.name: ConvertIpv4AddressTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest002, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(ADDREDD_LEN);
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: ConvertIpv4AddressTest003
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest003, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(TEST_IP);
    ASSERT_NE(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: ConvertIpv4AddressTest004
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertIpv4AddressTest004, TestSize.Level1)
{
    std::string addr;
    auto result = CommonUtils::ConvertIpv4Address(addr);
    ASSERT_EQ(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: Ipv4PrefixLenTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, Ipv4PrefixLenTest001, TestSize.Level1)
{
    std::string addr;
    auto result = CommonUtils::ConvertIpv4Address(addr);
    ASSERT_EQ(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: Ipv4PrefixLenTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertIpv4Address.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, Ipv4PrefixLenTest002, TestSize.Level1)
{
    auto result = CommonUtils::ConvertIpv4Address(TEST_IP);
    ASSERT_NE(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: ParseIntTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest001, TestSize.Level1)
{
    std::string testStr = "123";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_NE(value, 0);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: ParseIntTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest002, TestSize.Level1)
{
    std::string testStr = "abcdfagdshrfsth";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_EQ(value, 0);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ParseIntTest003
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest003, TestSize.Level1)
{
    std::string testStr = "44514564121561456745456891564564894";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_EQ(value, 0);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ParseIntTest004
 * @tc.desc: Test UtNetmanagerBaseCommon ParseInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ParseIntTest004, TestSize.Level1)
{
    std::string testStr = "-156423456123512423456146";
    int32_t value = 0;
    auto result = CommonUtils::ParseInt(testStr, &value);
    ASSERT_EQ(value, 0);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ConvertToInt64Test001
 * @tc.desc: Test UtNetmanagerBaseCommon ConvertToInt64.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ConvertToInt64Test001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::ConvertToInt64(testStr);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: ToAnonymousIpTest001
 * @tc.desc: Test UtNetmanagerBaseCommon ToAnonymousIp.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ToAnonymousIpTest001, TestSize.Level1)
{
    auto result = CommonUtils::ToAnonymousIp(TEST_IPV4);
    ASSERT_FALSE(result.empty());
}

/**
 * @tc.name: ToAnonymousIpTest002
 * @tc.desc: Test UtNetmanagerBaseCommon ToAnonymousIp.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ToAnonymousIpTest002, TestSize.Level1)
{
    auto result = CommonUtils::ToAnonymousIp(TEST_IP);
    ASSERT_FALSE(result.empty());
}

/*
 * @tc.name: ToAnonymousIpTest003
 * @tc.desc: Test UtNetmanagerBaseCommon ToAnonymousIp.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, ToAnonymousIpTest003, TestSize.Level1)
{
    auto result = CommonUtils::ToAnonymousIp({});
    ASSERT_TRUE(result.empty());
}

/**
 * @tc.name: StrToIntTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToInt.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToIntTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToInt(testStr);
    ASSERT_NE(result, 0);
}

/**
 * @tc.name: StrToUintTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToUint.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToUintTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToUint(testStr);
    ASSERT_NE(result, static_cast<uint32_t>(0));
}

/**
 * @tc.name: StrToBoolTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToBool.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToBoolTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToBool(testStr);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: StrToLongTest001
 * @tc.desc: Test UtNetmanagerBaseCommon StrToUint.
 * @tc.type: FUNC
 */
HWTEST_F(UtNetmanagerBaseCommon, StrToLongTest001, TestSize.Level1)
{
    std::string testStr = "145689";
    auto result = CommonUtils::StrToLong(testStr);
    ASSERT_NE(result, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
