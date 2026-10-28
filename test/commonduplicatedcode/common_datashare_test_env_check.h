/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_DATASHARE_TEST_ENV_CHECK_H
#define COMMON_DATASHARE_TEST_ENV_CHECK_H

#include <string>

#include "net_datashare_utils.h"
#include "net_manager_constants.h"
#include "uri.h"

namespace OHOS {
namespace NetManagerStandard {
namespace DataShareEnvCheck {
// The netmanager DataShare tests read and write the com.ohos.settingsdata DataShare service,
// which only exists on a device with the settings data service running. Probe the service by
// inserting (and removing) a dedicated probe key: when the service is absent every operation
// fails; when it is reachable the insert succeeds.
inline bool HasSettingsDataShareService(const std::string &probeUri)
{
    auto utils = std::make_unique<NetDataShareHelperUtils>();
    constexpr const char *PROBE_KEY = "settings.netmanager.dt_env_probe";
    Uri uri(probeUri);
    if (utils->Insert(uri, PROBE_KEY, "1") != NETMANAGER_SUCCESS) {
        return false;
    }
    utils->Delete(uri, PROBE_KEY);
    return true;
}
} // namespace DataShareEnvCheck
} // namespace NetManagerStandard
} // namespace OHOS

// Skips the enclosing test when the com.ohos.settingsdata DataShare service is not
// reachable on the environment (e.g. host runs without the settings data service); the
// uri argument is the DataShare URI the test operates on.
#define SKIP_IF_NO_SETTING_DATASHARE(uri)                                           \
    do {                                                                            \
        if (!DataShareEnvCheck::HasSettingsDataShareService(uri)) {                 \
            GTEST_SKIP() << "requires the com.ohos.settingsdata DataShare service"; \
        }                                                                           \
    } while (0)
#endif // COMMON_DATASHARE_TEST_ENV_CHECK_H
