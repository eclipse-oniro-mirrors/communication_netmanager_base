/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "net_handler.h"
#include "net_conn_types.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetHandlerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetHandlerTest::SetUpTestCase() {}

void NetHandlerTest::TearDownTestCase() {}

void NetHandlerTest::SetUp() {}

void NetHandlerTest::TearDown() {}

HWTEST_F(NetHandlerTest, BindSocket, TestSize.Level1)
{
    int32_t socket_fd = 1;
    uint32_t netId = 5;
    auto handler = DelayedSingleton<NetHandler>::GetInstance();
    handler->SetNetId(netId);
    int32_t result = handler->BindSocket(socket_fd);
    ASSERT_TRUE(result == 0);
}

HWTEST_F(NetHandlerTest, GetAddressesByName, TestSize.Level1)
{
    std::string host = "netAddr";
    std::list<INetAddr> addrList;
    uint32_t netId = 5;
    auto handler = DelayedSingleton<NetHandler>::GetInstance();
    handler->SetNetId(netId);
    int32_t result = handler->GetAddressesByName(host, addrList);
    ASSERT_TRUE(result == 0);
}

HWTEST_F(NetHandlerTest, GetAddressByName, TestSize.Level1)
{
    std::string host = "netAddr";
    INetAddr addr;
    uint32_t netId = 5;
    auto handler = DelayedSingleton<NetHandler>::GetInstance();
    handler->SetNetId(netId);
    int32_t result = handler->GetAddressByName(host, addr);
    ASSERT_TRUE(result == ERR_NO_ADDRESS);
}
} // namespace NetManagerStandard
} // namespace OHOS