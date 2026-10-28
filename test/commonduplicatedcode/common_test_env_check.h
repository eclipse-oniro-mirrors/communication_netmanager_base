/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_TEST_ENV_CHECK_H
#define COMMON_TEST_ENV_CHECK_H

#include <string>
#include <unistd.h>

namespace OHOS {
namespace NetManagerStandard {
namespace TestEnvCheck {
/**
 * @brief Check whether the current process runs as root. Most netsysnative kernel
 *        operations (netlink/ioctl/procfs writes) require root privileges.
 */
inline bool IsRoot()
{
    return geteuid() == 0;
}

/**
 * @brief Check whether a file or directory exists (e.g. a bpf pinned map path).
 */
inline bool FileExists(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

/**
 * @brief Check whether a network interface exists (e.g. "wlan0"). Reads the kernel's
 *        /sys/class/net listing instead of calling if_nametoindex so this header does
 *        not pull in <net/if.h>, which conflicts with <linux/if.h> in some tests.
 */
inline bool HasInterface(const std::string &ifName)
{
    return FileExists("/sys/class/net/" + ifName);
}
} // namespace TestEnvCheck
} // namespace NetManagerStandard
} // namespace OHOS
#endif // COMMON_TEST_ENV_CHECK_H
