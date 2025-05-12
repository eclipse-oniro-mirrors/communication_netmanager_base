/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#include "netnative_log_wrapper.h"
#include "netsys_net_dns_health_data.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace testing::ext;
using ::testing::_;
using ::testing::Return;
}  // namespace

class ParcelMock : public Parcel {
public:
    ParcelMock() {}
    virtual ~ParcelMock() {}
    MOCK_METHOD1(WriteUint32, bool(uint32_t));
    MOCK_METHOD1(WriteString, bool(const std::string &));
    MOCK_METHOD1(WriteUint16, bool(uint16_t));
    MOCK_METHOD1(WriteUint8, bool(uint8_t));
    MOCK_METHOD1(WriteUint64, bool(uint64_t));
    MOCK_METHOD1(WriteInt32, bool(int32_t));
    MOCK_METHOD1(WriteBool, bool(bool));
    MOCK_METHOD1(ReadUint32, bool(uint32_t &));
    MOCK_METHOD1(ReadString, bool(const std::string &));
    MOCK_METHOD1(ReadUint16, bool(uint16_t));
    MOCK_METHOD1(ReadBool, bool(bool));
    MOCK_METHOD1(ReadUint8, bool(uint8_t));
    MOCK_METHOD1(ReadUint64, bool(uint64_t));
    MOCK_METHOD1(ReadInt32, bool(int32_t));
};

class NetDnsHealthReportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    NetDnsHealthReport report;
    ParcelMock parcel;
};

void NetDnsHealthReportTest::SetUpTestCase() {}

void NetDnsHealthReportTest::TearDownTestCase() {}

void NetDnsHealthReportTest::SetUp()
{
    report.netid_ = 1;
    report.uid_ = 1;
    report.appid_ = 1;
    report.host_ = "test.host";
    report.type_ = 1;
    report.result_ = true;
}

void NetDnsHealthReportTest::TearDown() {}

HWTEST_F(NetDnsHealthReportTest, Marshalling_WhenWriteUint32Fails_netid, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(false));
    EXPECT_EQ(report.Marshalling(parcel), false);
}

HWTEST_F(NetDnsHealthReportTest, Marshalling_WhenWriteUint32Fails_uid, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).Times(2).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_EQ(report.Marshalling(parcel), false);
}

HWTEST_F(NetDnsHealthReportTest, Marshalling_WhenWriteUint32Fails_appid, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).Times(3).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_EQ(report.Marshalling(parcel), false);
}

HWTEST_F(NetDnsHealthReportTest, Marshalling_ShouldReturnFalse_WhenWriteStringFails, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(false));
    EXPECT_EQ(report.Marshalling(parcel), false);
}

HWTEST_F(NetDnsHealthReportTest, Marshalling_ShouldReturnFalse_WhenWriteUint16Fails, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint16(_)).WillOnce(Return(false));
    EXPECT_EQ(report.Marshalling(parcel), false);
}

HWTEST_F(NetDnsHealthReportTest, Marshalling_ShouldReturnFalse_WhenWriteBoolFails, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint16(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteBool(_)).WillOnce(Return(false));
    EXPECT_EQ(report.Marshalling(parcel), false);
}

HWTEST_F(NetDnsHealthReportTest, Marshalling_ShouldReturnTrue_WhenAllWriteFunctionsSucceed, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint16(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteBool(_)).WillOnce(Return(true));
    EXPECT_EQ(report.Marshalling(parcel), true);
}

HWTEST_F(NetDnsHealthReportTest, Unmarshalling_ShouldReturnTrue_WhenAllReadOperationsAreSuccessful, TestSize.Level0)
{
    EXPECT_CALL(parcel, ReadUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadUint16(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadBool(_)).WillOnce(Return(true));
    EXPECT_EQ(NetDnsHealthReport::Unmarshalling(parcel, report), true);
}

HWTEST_F(NetDnsHealthReportTest, Unmarshalling_ShouldReturnFalse_WhenAllReadUint32Fails, TestSize.Level0)
{
    EXPECT_CALL(parcel, ReadUint32(_)).WillRepeatedly(Return(false));
    EXPECT_EQ(NetDnsHealthReport::Unmarshalling(parcel, report), false);
}

HWTEST_F(NetDnsHealthReportTest, Unmarshalling_ShouldReturnFalse_WhenAllReadStringFails, TestSize.Level0)
{
    EXPECT_CALL(parcel, ReadUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(false));
    EXPECT_EQ(NetDnsHealthReport::Unmarshalling(parcel, report), false);
}

HWTEST_F(NetDnsHealthReportTest, Unmarshalling_ShouldReturnFalse_WhenAllReadUint16Fails, TestSize.Level0)
{
    EXPECT_CALL(parcel, ReadUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint16(_)).WillOnce(Return(false));
    EXPECT_EQ(NetDnsHealthReport::Unmarshalling(parcel, report), false);
}

HWTEST_F(NetDnsHealthReportTest, Unmarshalling_ShouldReturnFalse_WhenAllReadBoolFails, TestSize.Level0)
{
    EXPECT_CALL(parcel, ReadUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint16(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadBool(_)).WillOnce(Return(false));
    EXPECT_EQ(NetDnsHealthReport::Unmarshalling(parcel, report), false);
}
}  // namespace NetsysNative
}  // namespace OHOS