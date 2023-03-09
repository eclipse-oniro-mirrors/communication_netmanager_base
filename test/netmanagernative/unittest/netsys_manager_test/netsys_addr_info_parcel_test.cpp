/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <sys/un.h>

#include "netsys_addr_info_parcel.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace testing::ext;
} // namespace

class NetsysAddrInfoParcelTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetsysAddrInfoParcelTest::SetUpTestCase() {}

void NetsysAddrInfoParcelTest::TearDownTestCase() {}

void NetsysAddrInfoParcelTest::SetUp() {}

void NetsysAddrInfoParcelTest::TearDown() {}

HWTEST_F(NetsysAddrInfoParcelTest, MarshallingTest001, TestSize.Level1)
{
    MessageParcel data;
    NetsysAddrInfoParcel testData;
    testData.isHintsNull = false;
    testData.aiFamily = 1;
    testData.aiSocktype = 1;
    testData.aiFlags = 2;
    testData.aiProtocol = 1;
    testData.netId = 130;
    testData.hostName = "testhost";
    testData.serverName = "testserver";
    bool ret = testData.Marshalling(data);
    EXPECT_EQ(ret, true);

    NetsysAddrInfoParcel noHintData;
    noHintData.isHintsNull = false;
    noHintData.netId = 130;
    noHintData.hostName = "testhost";
    noHintData.serverName = "testserver";
    ret = testData.Marshalling(data);
    EXPECT_EQ(ret, true);
}

HWTEST_F(NetsysAddrInfoParcelTest, UnmarshallingTest001, TestSize.Level1)
{
    int32_t ret = 0;
    int32_t size = 1;
    int16_t flag = 1;
    int16_t family = 1;
    int16_t sockType = 1;
    int16_t protocol = 2;
    int16_t canSize = 0;
    MessageParcel reply;
    if (!reply.WriteInt32(ret)) {
        return;
    }
    if (!reply.WriteInt32(size)) {
        return;
    }
    if (!reply.WriteInt16(flag)) {
        return;
    }
    if (!reply.WriteInt16(family)) {
        return;
    }
    if (!reply.WriteInt16(sockType)) {
        return;
    }
    if (!reply.WriteInt16(protocol)) {
        return;
    }
    struct sockaddr_un tmpAddr = {AF_UNIX, "/dev/testfwmarkd"};
    uint32_t addrlen = sizeof(tmpAddr);
    if (!reply.WriteUint32(addrlen)) {
        return;
    }
    if (!reply.WriteInt16(canSize)) {
        return;
    }
    if (!reply.WriteRawData(reinterpret_cast<void *>(&tmpAddr), addrlen)) {
        return;
    }
    NetsysAddrInfoParcel testData;
    sptr<NetsysAddrInfoParcel> ptr = testData.Unmarshalling(reply);
    EXPECT_NE(ptr, nullptr);
}

HWTEST_F(NetsysAddrInfoParcelTest, UnmarshallingTest002, TestSize.Level1)
{
    int32_t ret = 0;
    int32_t size = 1;
    int16_t flag = 1;
    int16_t family = 1;
    int16_t sockType = 1;
    int16_t protocol = 2;
    MessageParcel reply;
    if (!reply.WriteInt32(ret)) {
        return;
    }
    if (!reply.WriteInt32(size)) {
        return;
    }
    if (!reply.WriteInt16(flag)) {
        return;
    }
    if (!reply.WriteInt16(family)) {
        return;
    }
    if (!reply.WriteInt16(sockType)) {
        return;
    }
    if (!reply.WriteInt16(protocol)) {
        return;
    }
    struct sockaddr_un tmpAddr = {AF_UNIX, "/dev/testfwmarkd"};
    uint32_t addrlen = sizeof(tmpAddr);
    if (!reply.WriteUint32(addrlen)) {
        return;
    }
    char canonname[] = {"testname"};
    int16_t canSize = strlen(canonname);
    if (!reply.WriteInt16(canSize)) {
        return;
    }
    if (!reply.WriteRawData(canonname, canSize)) {
        return;
    }
    if (!reply.WriteRawData(reinterpret_cast<void *>(&tmpAddr), addrlen)) {
        return;
    }
    NetsysAddrInfoParcel testData;
    sptr<NetsysAddrInfoParcel> ptr = testData.Unmarshalling(reply);
    EXPECT_NE(ptr, nullptr);
}
} // namespace nmd
} // namespace OHOS