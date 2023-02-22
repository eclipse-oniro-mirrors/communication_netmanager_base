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
#include <map>

#include "errorcode_convertor.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class ErrorCodeConvertorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ErrorCodeConvertorTest::SetUpTestCase() {}

void ErrorCodeConvertorTest::TearDownTestCase() {}

void ErrorCodeConvertorTest::SetUp() {}

void ErrorCodeConvertorTest::TearDown() {}

HWTEST_F(ErrorCodeConvertorTest, ConvertErrorCodeTest001, TestSize.Level1)
{
    int32_t testErrorCode = 5445645;
    auto instance = std::make_unique<NetBaseErrorCodeConvertor>();
    int32_t errorCode = NETMANAGER_ERR_INTERNAL;
    auto ret = instance->ConvertErrorCode(errorCode);
    ASSERT_FALSE(ret.empty());
    ret = instance->ConvertErrorCode(testErrorCode);
    ASSERT_TRUE(ret.empty());
}
} // namespace NetManagerStandard
} // namespace OHOS