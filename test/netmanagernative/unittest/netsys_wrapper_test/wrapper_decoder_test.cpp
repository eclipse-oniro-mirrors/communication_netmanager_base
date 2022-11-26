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

#include "netlink_define.h"
#include "wrapper_decoder.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
} // namespace

class WrapperDecoderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void WrapperDecoderTest::SetUpTestCase() {}

void WrapperDecoderTest::TearDownTestCase() {}

void WrapperDecoderTest::SetUp() {}

void WrapperDecoderTest::TearDown() {}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest001, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    std::string buffer = "testMsg@@";
    auto ret = decoder->DecodeAscii(buffer.data(), buffer.length());
    EXPECT_FALSE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest002, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    std::string buffer = "@testMsg";
    auto ret = decoder->DecodeAscii(buffer.data(), 0);
    EXPECT_FALSE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest003, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    std::string buffer = "@testMsg";
    auto ret = decoder->DecodeAscii(buffer.data(), buffer.length());
    EXPECT_FALSE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeAsciiTest004, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    const char *buffer =
        "action@msg\0ACTION=add\0ACTION=remove\0ACTION=change\0SEQNUM=111\0SEQNUM=\0SUBSYSTEM=net\0SUBSYSTEM="
        "\0SUBSYSTEM=test\0dfdfcc=ttt\0";
    auto ret = decoder->DecodeAscii(buffer, sizeof(buffer));
    EXPECT_TRUE(ret);
}

HWTEST_F(WrapperDecoderTest, DecodeBinaryTest001, TestSize.Level1)
{
    auto msg = std::make_shared<NetsysEventMessage>();
    std::unique_ptr<WrapperDecoder> decoder = std::make_unique<WrapperDecoder>(msg);
    auto ret = decoder->DecodeBinary(nullptr, 0);
    EXPECT_FALSE(ret);
}
} // namespace nmd
} // namespace OHOS