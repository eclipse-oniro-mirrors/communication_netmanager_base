/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "net_bundle_impl.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
sptr<AppExecFwk::BundleMgrProxy> GetBundleMgrProxy()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        NETMGR_LOG_E("fail to get system ability mgr.");
        return nullptr;
    }

    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        NETMGR_LOG_E("fail to get bundle manager proxy.");
        return nullptr;
    }
    return iface_cast<AppExecFwk::BundleMgrProxy>(remoteObject);
}

int32_t NetBundleImpl::GetJsonFromBundle(std::string &jsonProfile)
{
    sptr<AppExecFwk::BundleMgrProxy> bundleMgrProxy = GetBundleMgrProxy();
    if (bundleMgrProxy == nullptr) {
        NETMGR_LOG_E("Failed to get bundle manager proxy.");
        return NETMANAGER_ERR_INTERNAL;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(0, bundleInfo);
    if (ret != ERR_OK) {
        NETMGR_LOG_E("GetSelfBundleName: bundleName get fail.");
        return NETMANAGER_ERR_INTERNAL;
    }
    ret = bundleMgrProxy->GetJsonProfile(AppExecFwk::ProfileType::NETWORK_PROFILE,
        bundleInfo.name, bundleInfo.entryModuleName, jsonProfile);
    if (ret != ERR_OK) {
        NETMGR_LOG_D("No network_config profile configured in bundle manager.[%{public}d]", ret);
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_SUCCESS;
}

bool NetBundleImpl::IsAtomicService(std::string &bundleName)
{
    sptr<AppExecFwk::BundleMgrProxy> bundleMgrProxy = GetBundleMgrProxy();
    if (bundleMgrProxy == nullptr) {
        NETMGR_LOG_E("Failed to get bundle manager proxy.");
        return false;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto flags = AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(flags), bundleInfo);
    if (ret != ERR_OK) {
        NETMGR_LOG_E("GetSelfBundleName: bundleName get fail.");
        return false;
    }
    bundleName = bundleInfo.applicationInfo.bundleName;
    return bundleInfo.applicationInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE;
}

INetBundle *GetNetBundle()
{
    static NetBundleImpl impl;
    return &impl;
}

bool IsAtomicService(std::string &bundleName)
{
    NetBundleImpl impl;
    return impl.IsAtomicService(bundleName);
}
} // namespace NetManagerStandard
} // namespace OHOS
