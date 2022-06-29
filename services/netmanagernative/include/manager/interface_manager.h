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

#ifndef __INCLUDE_INTERFACE_MANAGER_H__
#define __INCLUDE_INTERFACE_MANAGER_H__

#include <string>
#include <vector>
#include <ostream>
#include "interface_type.h"

namespace OHOS {
namespace nmd {
static const uint32_t INTERFACE_ERR_MAX_LEN = 256;

class InterfaceManager {
public:
    InterfaceManager();
    ~InterfaceManager();
    static int SetMtu(const char *interfaceName, const char *mtuValue);
    static int GetMtu(const char *interfaceName);
    static int AddAddress(const char *interfaceName, const char *addr, int prefixLen);
    static int DelAddress(const char *interfaceName, const char *addr, int prefixLen);
    static std::vector<std::string> GetInterfaceNames();
    static InterfaceConfigurationParcel GetIfaceConfig(const std::string &ifName);
    static int SetIfaceConfig(const nmd::InterfaceConfigurationParcel &ifaceConfig);
private:
    static int ModifyAddress(uint32_t action, const char *interfaceName, const char *addr, int prefixLen);
};
} // namespace nmd
} // namespace OHOS
#endif //  !__INCLUDE_INTERFACE_MANAGER_H__
