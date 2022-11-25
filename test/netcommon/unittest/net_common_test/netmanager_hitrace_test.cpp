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

#include <thread>

#include <gtest/gtest.h>

#include "netmanager_hitrace.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr uint32_t TEST_DELAY_TIME = 1;
constexpr const char *TEST_VALUE = "TEST_VALUE";
constexpr int32_t TEST_TASK_ID = 10054;
} // namespace

class NetManagerHiTraceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetManagerHiTraceTest::SetUpTestCase() {}

void NetManagerHiTraceTest::TearDownTestCase() {}

void NetManagerHiTraceTest::SetUp() {}

void NetManagerHiTraceTest::TearDown() {}

HWTEST_F(NetManagerHiTraceTest, AllTest001, TestSize.Level1)
{
    NetmanagerHiTrace::NetmanagerStartSyncTrace(TEST_VALUE);
    std::this_thread::sleep_for(std::chrono::seconds(TEST_DELAY_TIME));
    NetmanagerHiTrace::NetmanagerFinishSyncTrace(TEST_VALUE);
    std::thread thread([this]() {
        NetmanagerHiTrace::NetmanagerStartAsyncTrace(TEST_VALUE, TEST_TASK_ID);
        std::this_thread::sleep_for(std::chrono::seconds(TEST_DELAY_TIME));
        NetmanagerHiTrace::NetmanagerFinishAsyncTrace(TEST_VALUE, TEST_TASK_ID);
    });
    thread.join();
}
} // namespace NetManagerStandard
} // namespace OHOS