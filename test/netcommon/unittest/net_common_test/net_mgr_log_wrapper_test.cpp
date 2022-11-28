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

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetMgrLogWrapperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetMgrLogWrapperTest::SetUpTestCase() {}

void NetMgrLogWrapperTest::TearDownTestCase() {}

void NetMgrLogWrapperTest::SetUp() {}

void NetMgrLogWrapperTest::TearDown() {}

HWTEST_F(NetMgrLogWrapperTest, JudgeLevelTest001, TestSize.Level1)
{
    auto ret = NetMgrLogWrapper::JudgeLevel(NetMgrLogLevel::INFO);
    ASSERT_TRUE(ret);
    NetMgrLogWrapper::SetLogLevel(NetMgrLogLevel::FATAL);
    auto level = NetMgrLogWrapper::GetLogLevel();
    ASSERT_EQ(level, NetMgrLogLevel::FATAL);
    ret = NetMgrLogWrapper::JudgeLevel(NetMgrLogLevel::INFO);
    ASSERT_FALSE(ret);
}

HWTEST_F(NetMgrLogWrapperTest, GetBriefFileNameTest001, TestSize.Level1)
{
    const std::string testFile = "testFile";
    auto ret = NetMgrLogWrapper::GetBriefFileName(testFile);
    EXPECT_EQ(ret, testFile);
}

HWTEST_F(NetMgrLogWrapperTest, GetBriefFileNameTest002, TestSize.Level1)
{
    const std::string testPath = "test";
    const std::string testFileName = "testFile";
    auto ret = NetMgrLogWrapper::GetBriefFileName(testPath + "/" + testFileName);
    EXPECT_EQ(ret, testFileName);
}

HWTEST_F(NetMgrLogWrapperTest, GetBriefFileNameTest003, TestSize.Level1)
{
    const std::string testPath = "test";
    const std::string testFileName = "testFile";
    auto ret = NetMgrLogWrapper::GetBriefFileName(testPath + "\\" + testFileName);
    EXPECT_EQ(ret, testFileName);
}
} // namespace NetManagerStandard
} // namespace OHOS