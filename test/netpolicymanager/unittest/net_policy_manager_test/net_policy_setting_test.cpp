/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "net_mgr_log_wrapper.h"
#include "net_policy_callback_test.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_policy_inner_define.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms1 = {.userID = 1,
                                .bundleName = "net_policy_manager_test",
                                .instIndex = 0,
                                .appIDDesc = "test",
                                .isSystemApp = true};

PermissionDef testPermDef1 = {.permissionName = "ohos.permission.MANAGE_NET_STRATEGY",
                              .bundleName = "net_policy_manager_test",
                              .grantMode = 1,
                              .availableLevel = APL_SYSTEM_BASIC,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net policy connectivity internal",
                              .descriptionId = 1};

PermissionStateFull testState1 = {.permissionName = "ohos.permission.MANAGE_NET_STRATEGY",
                                  .isGeneral = true,
                                  .resDeviceID = {"local"},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .grantFlags = {2}};

HapPolicyParams testPolicyPrams1 = {.apl = APL_SYSTEM_BASIC,
                                    .domain = "test.domain",
                                    .permList = {testPermDef1},
                                    .permStateList = {testState1}};
} // namespace


class AccessToken {
public:
    AccessToken(HapInfoParams &testInfoParms, HapPolicyParams &testPolicyPrams) : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(tokenIdEx.tokenIDEx);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};

using namespace testing::ext;
class NetPolicySettingTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<NetPolicyCallbackTest> GetINetPolicyCallbackSample() const;
};

void NetPolicySettingTest::SetUpTestCase()
{
}

void NetPolicySettingTest::TearDownTestCase()
{
}
void NetPolicySettingTest::SetUp() {}

void NetPolicySettingTest::TearDown() {}

HWTEST_F(NetPolicySettingTest, OpenPowerSave, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPowerSavePolicy(true);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetPolicySettingTest, ClosePowerSave, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetPowerSavePolicy(false);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetPolicySettingTest, OpenDeviceIdle, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdlePolicy(true);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetPolicySettingTest,CloseDeviceIdle, TestSize.Level1)
{
    AccessToken token(testInfoParms1, testPolicyPrams1);
    int32_t result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetDeviceIdlePolicy(false);
    EXPECT_EQ(result, NETMANAGER_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS
