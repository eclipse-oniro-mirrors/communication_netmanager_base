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

#ifndef NETMANAGER_NET_CONN_SECURITY_H
#define NETMANAGER_NET_CONN_SECURITY_H

#include "accesstoken_kit.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "net_all_capabilities.h"
#include "net_conn_service.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
using Security::AccessToken::AccessTokenKit;
using Security::AccessToken::AccessTokenID;
using Security::AccessToken::AccessTokenIDEx;
class AccessToken {
public:
    AccessToken(Security::AccessToken::HapInfoParams &testInfoParms,
                Security::AccessToken::HapPolicyParams &testPolicyPrams)
    {
        currentID_ = GetSelfTokenID();
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_NET_CONN_SECURITY_H