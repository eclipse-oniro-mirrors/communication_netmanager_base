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

#include "net_stats_security.h"

#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

namespace {
HapInfoParams netStatsManagerInfo = {
    .userID = 1,
    .bundleName = "net_stats_manager_test",
    .instIndex = 0,
    .appIDDesc = "test",
    .isSystemApp = true,
};

PermissionDef testNetStatsPermDef = {
    .permissionName = "ohos.permission.GET_NETWORK_STATS",
    .bundleName = "net_stats_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net stats manager",
    .descriptionId = 1,
};

PermissionStateFull testNetStatsState = {
    .permissionName = "ohos.permission.GET_NETWORK_STATS",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

HapPolicyParams netStatsManagerPolicy = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testNetStatsPermDef},
    .permStateList = {testNetStatsState},
};
} // namespace

NetStatsSecurityAccessToken::NetStatsSecurityAccessToken()
{
    currentID_ = GetSelfTokenID();
    AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(netStatsManagerInfo, netStatsManagerPolicy);
    accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
    SetSelfTokenID(tokenIdEx.tokenIDEx);
}

NetStatsSecurityAccessToken::~NetStatsSecurityAccessToken()
{
    AccessTokenKit::DeleteToken(accessID_);
    SetSelfTokenID(currentID_);
}
} // namespace NetManagerStandard
} // namespace OHOS
