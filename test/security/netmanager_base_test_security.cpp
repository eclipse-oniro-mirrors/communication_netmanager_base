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

#include "netmanager_base_test_security.h"

#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
namespace {
HapInfoParams netManagerBaseParms = {
    .userID = 1,
    .bundleName = "netmanager_base_test",
    .instIndex = 0,
    .appIDDesc = "test",
    .isSystemApp = true,
};

PermissionDef testNetConnInternetPermDef = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .bundleName = "netmanager_base_test",
    .grantMode = 1,
    .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect manager internet",
    .descriptionId = 1,
};

PermissionStateFull testNetConnInternetState = {
    .permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
    .isGeneral = true,
    .resDeviceID = { "local" },
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .grantFlags = { 2 },
};

PermissionDef getNetworkInfoPermDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "netmanager_base_test",
    .grantMode = 1,
    .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect manager network info",
    .descriptionId = 1,
};

PermissionStateFull getNetworkInfoState = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .isGeneral = true,
    .resDeviceID = { "local" },
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .grantFlags = { 2 },
};


HapPolicyParams netManagerBasePolicy = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = { getNetworkInfoPermDef, testNetConnInternetPermDef },
    .permStateList = { getNetworkInfoState, testNetConnInternetState },
};

PermissionDef testNoPermissionDef = {
    .permissionName = "",
    .bundleName = "netmanager_base_test",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test no permission",
    .descriptionId = 1,
};

PermissionStateFull testNoPermissionState = {
    .permissionName = "",
    .isGeneral = true,
    .resDeviceID = { "local" },
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .grantFlags = { 2 },
};

HapPolicyParams testNoPermission = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = { testNoPermissionDef },
    .permStateList = { testNoPermissionState },
};
} // namespace

NetManagerBaseAccessToken::NetManagerBaseAccessToken() : currentID_(GetSelfTokenID())
{
    AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(netManagerBaseParms, netManagerBasePolicy);
    accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
    SetSelfTokenID(tokenIdEx.tokenIDEx);
}

NetManagerBaseAccessToken::~NetManagerBaseAccessToken()
{
    AccessTokenKit::DeleteToken(accessID_);
    SetSelfTokenID(currentID_);
}

NetManagerBaseNotSystemAccessToken::NetManagerBaseNotSystemAccessToken() : currentID_(GetSelfTokenID())
{
    AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(netManagerBaseParms, netManagerBasePolicy);
    accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
    SetSelfTokenID(accessID_);
}

NetManagerBaseNotSystemAccessToken::~NetManagerBaseNotSystemAccessToken()
{
    AccessTokenKit::DeleteToken(accessID_);
    SetSelfTokenID(currentID_);
}

NoPermissionAccessToken::NoPermissionAccessToken() : currentID_(GetSelfTokenID())
{
    AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(netManagerBaseParms, testNoPermission);
    accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
    SetSelfTokenID(tokenIdEx.tokenIDEx);
}

NoPermissionAccessToken::~NoPermissionAccessToken()
{
    AccessTokenKit::DeleteToken(accessID_);
    SetSelfTokenID(currentID_);
}
} // namespace NetManagerStandard
} // namespace OHOS
