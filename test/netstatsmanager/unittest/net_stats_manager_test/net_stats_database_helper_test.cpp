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

#include <memory>

#include <gtest/gtest.h>

#include "net_stats_database_helper.h"
#include "net_stats_database_defines.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;
using namespace testing::ext;
namespace {
constexpr const char *NET_STATS_DATABASE_TEST_PATH = "/data/service/el1/public/netmanager/net_stats_test.db";
NetStatsDatabaseHelper::SqlCallback sqlCallback = [](void *notUsed, int argc, char **argv, char **colName) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s", colName[i], argv[i] ? argv[i] : "nullptr");
    }
    return 0;
};
} // namespace
class NetStatsDatabaseHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetStatsDatabaseHelperTest::SetUpTestCase() {}

void NetStatsDatabaseHelperTest::TearDownTestCase() {}

void NetStatsDatabaseHelperTest::SetUp() {}

void NetStatsDatabaseHelperTest::TearDown() {}

HWTEST_F(NetStatsDatabaseHelperTest, CreateTableTest001, TestSize.Level1)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_TEST_PATH);
    std::string tableInfo =
        "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, AGE INT NOT NULL, ADDRESS CHAR(50)";
    int32_t ret = helper->CreateTable("testTable", tableInfo);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsDatabaseHelperTest, CreateTableTest002, TestSize.Level1)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_TEST_PATH);
    const std::string tableInfo =
        "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
        "UID INTEGER NOT NULL,"
        "IFace CHAR(50) NOT NULL,"
        "Date INTEGER NOT NULL,"
        "RxBytes INTEGER NOT NULL,"
        "RxPackets INTEGER NOT NULL,"
        "TxBytes INTEGER NOT NULL,"
        "TxPackets INTEGER NOT NULL";
    int32_t ret = helper->CreateTable(UID_TABLE, tableInfo);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsDatabaseHelperTest, CreateTableTest003, TestSize.Level1)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_TEST_PATH);
    const std::string tableInfo =
        "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
        "UID INTEGER NOT NULL,"
        "Date INTEGER NOT NULL,"
        "RxBytes INTEGER NOT NULL,"
        "RxPackets INTEGER NOT NULL,"
        "TxBytes INTEGER NOT NULL,"
        "TxPackets INTEGER NOT NULL";
    int32_t ret = helper->CreateTable(IFACE_TABLE, tableInfo);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsDatabaseHelperTest, InsertDataHelperTest001, TestSize.Level1)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_TEST_PATH);
    const std::string datalist = "10222, 'eth0', 15254500, 4455, 8536, 45122, 144215";
    int32_t ret = helper->InsertData(UID_TABLE, UID_TABLE_PARAM_LIST, datalist);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsDatabaseHelperTest, SelectDataHelperTest001, TestSize.Level1)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_TEST_PATH);
    int32_t ret = helper->SelectData(UID_TABLE, nullptr, sqlCallback, 0, LONG_MAX);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetStatsDatabaseHelperTest, DeleteDataHelperTest001, TestSize.Level1)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_TEST_PATH);
    int32_t ret = helper->DeleteData(UID_TABLE, 15254540, 15254560);
    helper->SelectData(UID_TABLE, nullptr, sqlCallback, 0, LONG_MAX);
    EXPECT_EQ(ret, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
