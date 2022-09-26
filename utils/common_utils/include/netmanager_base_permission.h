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

#ifndef NETMANAGER_PERMISSION_H
#define NETMANAGER_PERMISSION_H

#include <string>

namespace OHOS {
namespace NetManagerStandard {
namespace Permission {
static const std::string GET_NETWORK_INFO = "ohos.permission.GET_NETWORK_INFO";
static const std::string INTERNET = "ohos.permission.INTERNET";
static const std::string CONNECTIVITY_INTERNAL = "ohos.permission.CONNECTIVITY_INTERNAL";
} // namespace Permission

class NetManagerPermission {
public:
    static bool CheckPermission(const std::string &permissionName);
    static bool CheckPermissionWithCache(const std::string &permissionName);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_PERMISSION_H
