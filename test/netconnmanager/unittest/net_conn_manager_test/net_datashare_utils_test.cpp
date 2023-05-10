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
#include <memory>

#include "message_parcel.h"
#include "net_conn_constants.h"
#include "net_conn_security.h"
#include "net_datashare_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

HapInfoParams testInfoParms = {.bundleName = "net_datashare_utils_test",
                               .userID = 100,
                               .instIndex = 0,
                               .appIDDesc = "test",
                               .isSystemApp = true};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.MANAGE_SECURE_SETTINGS",
    .bundleName = "net_datashare_utils_test",
    .grantMode = 1,
    .label = "label",
    .labelId = 1,
    .description = "Test net data share",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testState = {
    .grantFlags = {2},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .isGeneral = true,
    .permissionName = "ohos.permission.MANAGE_SECURE_SETTINGS",
    .resDeviceID = {"local"},
};

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef},
    .permStateList = {testState},
};
} // namespace

std::unique_ptr<NetDataShareHelperUtils> netDataShareHelperUtils_ = nullptr;
class NetDataShareHelperUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetDataShareHelperUtilsTest::SetUpTestCase()
{
    netDataShareHelperUtils_ = std::make_unique<NetDataShareHelperUtils>();
}

void NetDataShareHelperUtilsTest::TearDownTestCase()
{
    netDataShareHelperUtils_.reset();
}

void NetDataShareHelperUtilsTest::SetUp() {}

void NetDataShareHelperUtilsTest::TearDown() {}

/**
 * @tc.name: InsertTest001
 * @tc.desc: Test NetDataShareHelperUtils::Insert
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, InsertTest001, TestSize.Level1)
{
    std::string airplaneMode = "1";
    Uri uri(SETTINGS_DATASHARE_URL_AIRPLANE_MODE);
    int32_t ret = netDataShareHelperUtils_->Insert(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_ERROR);

    airplaneMode = "0";
    ret = netDataShareHelperUtils_->Insert(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_ERROR);
}

/**
 * @tc.name: InsertTest002
 * @tc.desc: Test NetDataShareHelperUtils::Insert
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, InsertTest002, TestSize.Level1)
{
    OHOS::NetManagerStandard::AccessToken token(testInfoParms, testPolicyPrams);
    std::string airplaneMode = "1";
    Uri uri(SETTINGS_DATASHARE_URL_AIRPLANE_MODE);
    int32_t ret = netDataShareHelperUtils_->Insert(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

    airplaneMode = "0";
    ret = netDataShareHelperUtils_->Insert(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: UpdateTest001
 * @tc.desc: Test NetDataShareHelperUtils::Update
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, UpdateTest001, TestSize.Level1)
{
    std::string airplaneMode = "1";
    Uri uri(SETTINGS_DATASHARE_URL_AIRPLANE_MODE);
    int32_t ret = netDataShareHelperUtils_->Update(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_ERROR);

    airplaneMode = "0";
    ret = netDataShareHelperUtils_->Update(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_ERROR);
}

/**
 * @tc.name: UpdateTest002
 * @tc.desc: Test NetDataShareHelperUtils::Update
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, UpdateTest002, TestSize.Level1)
{
    OHOS::NetManagerStandard::AccessToken token(testInfoParms, testPolicyPrams);
    std::string airplaneMode = "1";
    Uri uri(SETTINGS_DATASHARE_URL_AIRPLANE_MODE);
    int32_t ret = netDataShareHelperUtils_->Update(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);

    airplaneMode = "0";
    ret = netDataShareHelperUtils_->Update(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}

/**
 * @tc.name: QueryTest001
 * @tc.desc: Test NetDataShareHelperUtils::Query
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, QueryTest001, TestSize.Level1)
{
    std::string airplaneMode;
    Uri uri(SETTINGS_DATASHARE_URL_AIRPLANE_MODE);
    int32_t ret = netDataShareHelperUtils_->Query(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    std::cout << "QueryTest result:" << airplaneMode << std::endl;
    ASSERT_TRUE(ret == NETMANAGER_ERROR);
}

/**
 * @tc.name: QueryTest002
 * @tc.desc: Test NetDataShareHelperUtils::Query
 * @tc.type: FUNC
 */
HWTEST_F(NetDataShareHelperUtilsTest, QueryTest002, TestSize.Level1)
{
    OHOS::NetManagerStandard::AccessToken token(testInfoParms, testPolicyPrams);
    std::string airplaneMode;
    Uri uri(SETTINGS_DATASHARE_URL_AIRPLANE_MODE);
    int32_t ret = netDataShareHelperUtils_->Query(uri, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, airplaneMode);
    std::cout << "QueryTest result:" << airplaneMode << std::endl;
    ASSERT_TRUE(ret == NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
