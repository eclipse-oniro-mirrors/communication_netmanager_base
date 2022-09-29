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

#include "iservice_registry.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service_proxy.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace ConnGetProxy {
sptr<INetsysService> ConnManagerGetProxy()
{
    NETNATIVE_LOGI("Get samgr >>>>>>>>>>>>>>>>>>>>>>>>>>");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    NETNATIVE_LOGI("Get samgr %{public}p", samgr.GetRefPtr());
    std::cout << "Get samgr  " << samgr.GetRefPtr() << std::endl;

    auto remote = samgr->GetSystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    NETNATIVE_LOGI("Get remote %{public}p", remote.GetRefPtr());
    std::cout << "Get remote " << remote.GetRefPtr() << std::endl;

    auto proxy = iface_cast<NetsysNative::INetsysService>(remote);
    NETNATIVE_LOGI("Get proxy %{public}p", proxy.GetRefPtr());
    std::cout << "Get proxy " << proxy.GetRefPtr() << std::endl;
    return proxy;
}
} // namespace ConnGetProxy
} // namespace NetManagerStandard
} // namespace OHOS