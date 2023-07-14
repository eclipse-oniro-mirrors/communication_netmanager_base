/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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
static constexpr const char *GET_NETWORK_INFO = "ohos.permission.GET_NETWORK_INFO";
static constexpr const char *INTERNET = "ohos.permission.INTERNET";
static constexpr const char *CONNECTIVITY_INTERNAL = "ohos.permission.CONNECTIVITY_INTERNAL";
static constexpr const char *GET_NETSTATS_SUMMARY = "ohos.permission.GET_NETSTATS_SUMMARY";
static constexpr const char *MANAGE_NET_STRATEGY = "ohos.permission.MANAGE_NET_STRATEGY";
static constexpr const char *MANAGE_VPN = "ohos.permission.MANAGE_VPN";
static constexpr const char *GET_NETWORK_STATS = "ohos.permission.GET_NETWORK_STATS";
static constexpr const char *NETSYS_INTERNAL = "ohos.permission.NETSYS_INTERNAL";
} // namespace Permission

class NetManagerPermission {
public:
    static bool CheckPermission(const std::string &permissionName);
    static bool CheckPermissionWithCache(const std::string &permissionName);
    static bool IsSystemCaller();
    static bool CheckNetSysInternalPermission(const std::string &permissionName);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_PERMISSION_H
