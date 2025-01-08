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
#include "netsys_net_dns_result_data.h"

namespace OHOS {
namespace NetsysNative {
namespace {
using namespace testing::ext;
using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::InSequence;
}  // namespace

class ParcelMock : public Parcel {
public:
    ParcelMock() {}
    virtual ~ParcelMock() {}
    MOCK_METHOD1(WriteUint32, bool(uint32_t));
    MOCK_METHOD1(WriteString, bool(const std::string &));
    MOCK_METHOD1(WriteUint16, bool(uint16_t));
    MOCK_METHOD1(WriteBool, bool(bool));
    MOCK_METHOD1(ReadUint32, bool(uint32_t &));
    MOCK_METHOD1(ReadString, bool(const std::string &));
    MOCK_METHOD1(ReadUint16, bool(uint16_t));
    MOCK_METHOD1(ReadBool, bool(bool));
};

class NetDnsResultReportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    NetDnsResultReport report;
    ParcelMock parcel;
};

void NetDnsResultReportTest::SetUpTestCase() {}

void NetDnsResultReportTest::TearDownTestCase() {}

void NetDnsResultReportTest::SetUp()
{
    report.netid_ = 1;
    report.uid_ = 1;
    report.pid_ = 1;
    report.timeused_ = 1;
    report.queryresult_ = 1;
    report.host_ = "test.host";

    NetDnsResultAddrInfo addrInfo1;
    addrInfo1.type_ = NetDnsResultAddrType::ADDR_TYPE_IPV4;
    addrInfo1.addr_ = "192.168.1.1";
    report.addrlist_.push_back(addrInfo1);

    NetDnsResultAddrInfo addrInfo2;
    addrInfo2.type_ = NetDnsResultAddrType::ADDR_TYPE_IPV6;
    addrInfo2.addr_ = "2001:db8::1";
    report.addrlist_.push_back(addrInfo2);
}

void NetDnsResultReportTest::TearDown() {}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_001, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_002, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).Times(2).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_003, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).Times(3).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_004, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, WriteUint32(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_005, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, WriteUint32(_)).Times(4).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_006, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, WriteUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_007, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, WriteUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_008, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, WriteUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_009, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, WriteUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnFalse_010, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, WriteUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, WriteUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Marshalling_ShouldReturnTrue_001, TestSize.Level0)
{
    EXPECT_CALL(parcel, WriteUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, WriteString(_)).WillRepeatedly(Return(true));
    EXPECT_TRUE(report.Marshalling(parcel));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_001, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_002, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_003, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_004, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_005, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(4).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_006, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_007, TestSize.Level0)
{
    InSequence s;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_008, TestSize.Level0)
{
    InSequence s;
    uint32_t size = 2;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(size), Return(true)));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_009, TestSize.Level0)
{
    InSequence s;
    uint32_t size = 2;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(size), Return(true)));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnFalse_010, TestSize.Level0)
{
    InSequence s;
    uint32_t size = 2;
    EXPECT_CALL(parcel, ReadUint32(_)).Times(5).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(size), Return(true)));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(parcel, ReadUint32(_)).WillOnce(Return(false));
    EXPECT_FALSE(NetDnsResultReport::Unmarshalling(parcel, report));
}

HWTEST_F(NetDnsResultReportTest, Unmarshalling_ShouldReturnTrue_001, TestSize.Level0)
{
    EXPECT_CALL(parcel, ReadUint32(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(parcel, ReadString(_)).WillRepeatedly(Return(true));
    EXPECT_TRUE(NetDnsResultReport::Unmarshalling(parcel, report));
}

}  // namespace NetsysNative
}  // namespace OHOS