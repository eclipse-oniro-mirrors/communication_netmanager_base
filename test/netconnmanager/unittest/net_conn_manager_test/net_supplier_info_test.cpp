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
#include "net_mgr_log_wrapper.h"
#include "net_supplier_info.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetSupplierInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetSupplierInfoTest::SetUpTestCase() {}

void NetSupplierInfoTest::TearDownTestCase() {}

void NetSupplierInfoTest::SetUp() {}

void NetSupplierInfoTest::TearDown() {}

/**
 * @tc.name: UnmarshallingTest
 * @tc.desc: Test NetSupplierInfo::Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(NetSupplierInfoTest, UnmarshallingTest, TestSize.Level1)
{
    MessageParcel data;
    sptr<NetSupplierInfo> info = new (std::nothrow) NetSupplierInfo();
    bool bRet = NetSupplierInfo::Marshalling(data, info);
    ASSERT_TRUE(bRet == true);

    sptr<NetSupplierInfo> retInf = NetSupplierInfo::Unmarshalling(data);
    ASSERT_TRUE(retInf != nullptr);
}

/**
 * @tc.name: ToStringTest
 * @tc.desc: Test NetSupplierInfo::ToString
 * @tc.type: FUNC
 */
HWTEST_F(NetSupplierInfoTest, ToStringTest, TestSize.Level1)
{
    sptr<NetSupplierInfo> info = new (std::nothrow) NetSupplierInfo();
    std::string str = info->ToString("testTab");
    NETMGR_LOG_D("NetSupplierInfoTest.ToString string is : [%{public}s]", str.c_str());
}
} // namespace NetManagerStandard
} // namespace OHOS
