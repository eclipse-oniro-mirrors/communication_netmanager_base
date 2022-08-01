/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "netmanager_base_permission.h"

#include "ipc_skeleton.h"
#include "accesstoken_kit.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
/**
 * @brief Permission check by callingTokenID.
 * @param permissionName permission name.
 * @return Returns true on success, false on failure.
 */
bool NetManagerPermission::CheckPermission(const std::string &permissionName)
{
    if (permissionName.empty()) {
        NETMGR_LOG_E("permission check failed，permission name is empty.");
        return false;
    }

    auto callerToken = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    int result = Security::AccessToken::PERMISSION_DENIED;

    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
#ifdef NETMANAGER_SUPPORT_NATIVE_PERMISSION
        result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
#endif
    } else if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    } else {
        NETMGR_LOG_E("permission check failed, callerToken:%{public}u, tokenType:%{public}d",
            callerToken, tokenType);
    }

    if (result != Security::AccessToken::PERMISSION_GRANTED) {
        NETMGR_LOG_E("permission check failed, permission:%{public}s, callerToken:%{public}u, tokenType:%{public}d",
            permissionName.c_str(), callerToken, tokenType);
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS