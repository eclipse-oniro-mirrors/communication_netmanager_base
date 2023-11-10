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

#include "net_policy_security.h"

#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

namespace {
HapInfoParams netPolicyManagerInfo = {
    .userID = 1,
    .bundleName = "net_policy_manager_test",
    .instIndex = 0,
    .appIDDesc = "test",
    .isSystemApp = true,
};

PermissionDef testNetPolicyStrategyPermDef = {
    .permissionName = "ohos.permission.MANAGE_NET_STRATEGY",
    .bundleName = "net_policy_manager_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net policy manager",
    .descriptionId = 1,
};

PermissionStateFull testNetPolicyStrategyState = {
    .permissionName = "ohos.permission.MANAGE_NET_STRATEGY",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2},
};

HapPolicyParams netPolicyManagerPolicy = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = { testNetPolicyStrategyPermDef },
    .permStateList = { testNetPolicyStrategyState },
};
} // namespace

NetPolicyAccessToken::NetPolicyAccessToken()
{
    currentID_ = GetSelfTokenID();
    AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(netPolicyManagerInfo, netPolicyManagerPolicy);
    accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
    SetSelfTokenID(tokenIdEx.tokenIDEx);
}

NetPolicyAccessToken::~NetPolicyAccessToken()
{
    AccessTokenKit::DeleteToken(accessID_);
    SetSelfTokenID(currentID_);
}
} // namespace NetManagerStandard
} // namespace OHOS
