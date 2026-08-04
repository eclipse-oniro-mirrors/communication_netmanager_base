/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "message_parcel.h"
#include "netfirewall_parcel.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;
using namespace OHOS::NetManagerStandard;

namespace {
static constexpr uint32_t TEST_SEQ = 7;
static constexpr uint16_t TEST_QUEUE_NUM = 123;
static constexpr uint32_t TEST_INVALID_INDEX = NFQ_MAX_QUEUES;
}

class NfqueueParcelTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NfqueueParcelTest::SetUpTestCase() {}

void NfqueueParcelTest::TearDownTestCase() {}

void NfqueueParcelTest::SetUp() {}

void NfqueueParcelTest::TearDown() {}

HWTEST_F(NfqueueParcelTest, NfqQueueMarshalling001, TestSize.Level1)
{
    sptr<NfqQueue> queue = new (std::nothrow) NfqQueue();
    ASSERT_NE(queue, nullptr);
    queue->queueNum = TEST_QUEUE_NUM;

    MessageParcel parcel;
    EXPECT_TRUE(queue->Marshalling(parcel));

    sptr<NfqQueue> result = NfqQueue::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->queueNum, TEST_QUEUE_NUM);
}

HWTEST_F(NfqueueParcelTest, NfqQueueMarshallingInvalid001, TestSize.Level1)
{
    MessageParcel parcel;
    sptr<NfqQueue> result = NfqQueue::Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(NfqueueParcelTest, NfqCtxMarshalling001, TestSize.Level1)
{
    sptr<NfqCtx> ctx = new (std::nothrow) NfqCtx();
    ASSERT_NE(ctx, nullptr);
    ctx->seq = TEST_SEQ;

    MessageParcel parcel;
    EXPECT_TRUE(ctx->Marshalling(parcel));

    sptr<NfqCtx> result = NfqCtx::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->seq, TEST_SEQ);
    for (uint32_t i = 0; i < NFQ_MAX_QUEUES; i++) {
        EXPECT_EQ(result->queues[i], nullptr);
    }
}

HWTEST_F(NfqueueParcelTest, NfqCtxMarshalling002, TestSize.Level1)
{
    sptr<NfqCtx> ctx = new (std::nothrow) NfqCtx();
    ASSERT_NE(ctx, nullptr);
    ctx->seq = TEST_SEQ;

    sptr<NfqQueue> queue0 = new (std::nothrow) NfqQueue();
    ASSERT_NE(queue0, nullptr);
    queue0->queueNum = 0;
    ctx->queues[0] = queue0;

    sptr<NfqQueue> queue5 = new (std::nothrow) NfqQueue();
    ASSERT_NE(queue5, nullptr);
    queue5->queueNum = 5;
    ctx->queues[5] = queue5;

    sptr<NfqQueue> queue63 = new (std::nothrow) NfqQueue();
    ASSERT_NE(queue63, nullptr);
    queue63->queueNum = 63;
    ctx->queues[NFQ_MAX_QUEUES - 1] = queue63;

    MessageParcel parcel;
    EXPECT_TRUE(ctx->Marshalling(parcel));

    sptr<NfqCtx> result = NfqCtx::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->seq, TEST_SEQ);
    ASSERT_NE(result->queues[0], nullptr);
    EXPECT_EQ(result->queues[0]->queueNum, 0);
    ASSERT_NE(result->queues[5], nullptr);
    EXPECT_EQ(result->queues[5]->queueNum, 5);
    EXPECT_EQ(result->queues[NFQ_MAX_QUEUES - 1]->queueNum, 63);
}

HWTEST_F(NfqueueParcelTest, NfqCtxMarshallingInvalidIndex001, TestSize.Level1)
{
    MessageParcel parcel;
    parcel.WriteUint32(TEST_SEQ);
    parcel.WriteUint32(1);
    parcel.WriteUint32(TEST_INVALID_INDEX);
    parcel.WriteUint16(TEST_QUEUE_NUM);

    sptr<NfqCtx> result = NfqCtx::Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(NfqueueParcelTest, NfqCtxMarshallingTruncated001, TestSize.Level1)
{
    MessageParcel parcel;
    parcel.WriteUint32(TEST_SEQ);
    parcel.WriteUint32(2);
    parcel.WriteUint32(0);
    parcel.WriteUint16(TEST_QUEUE_NUM);

    sptr<NfqCtx> result = NfqCtx::Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
}
