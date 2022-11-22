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
using namespace testing::ext;
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
} // namespace NetManagerStandard
} // namespace OHOS
