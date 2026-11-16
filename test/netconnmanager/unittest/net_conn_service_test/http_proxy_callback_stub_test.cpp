/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "http_proxy_callback_stub.h"
#include "conn_ipc_interface_code.h"
#include "net_conn_constants.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class HttpProxyCallbackStubTest : public testing::Test {};

/**
 * @tc.name: RequestBranches
 * @tc.desc: Test descriptor, request code, callback execution, and cleared callback branches.
 * @tc.type: FUNC
 */
HWTEST_F(HttpProxyCallbackStubTest, RequestBranches, TestSize.Level1)
{
    sptr<HttpProxyCallbackStub> stub = new HttpProxyCallbackStub();
    MessageParcel reply;
    MessageOption option;
    MessageParcel wrong;
    wrong.WriteInterfaceToken(u"invalid");
    EXPECT_EQ(stub->OnRemoteRequest(0, wrong, reply, option), NETMANAGER_ERR_DESCRIPTOR_MISMATCH);

    MessageParcel unknown;
    unknown.WriteInterfaceToken(HttpProxyCallbackStub::GetDescriptor());
    EXPECT_NE(stub->OnRemoteRequest(99, unknown, reply, option), NETMANAGER_ERR_DESCRIPTOR_MISMATCH);

    int count = 0;
    stub->SetCallback([&count]() { ++count; });
    MessageParcel valid;
    valid.WriteInterfaceToken(HttpProxyCallbackStub::GetDescriptor());
    EXPECT_EQ(stub->OnRemoteRequest(
        static_cast<uint32_t>(HttpProxyCallbackInterfaceCode::ON_HTTP_PROXY_CHANGE), valid, reply, option),
        NETMANAGER_SUCCESS);
    EXPECT_EQ(count, 1);
    stub->SetCallback(nullptr);
    EXPECT_EQ(stub->OnHttpProxyChange(), NETMANAGER_SUCCESS);
    EXPECT_EQ(count, 1);
    stub->ClearCallbackAndWait();
}
} // namespace NetManagerStandard
} // namespace OHOS
