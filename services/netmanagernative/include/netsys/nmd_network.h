/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_NMD_NETWORK_H__
#define INCLUDE_NMD_NETWORK_H__

#include <set>
#include <string>

namespace OHOS {
namespace nmd {
enum NetworkPermission : int {
    PERMISSION_NONE = 0x0,
    PERMISSION_NETWORK = 0x1,
    PERMISSION_SYSTEM = 0x3,
};

class NmdNetwork {
public:
    NmdNetwork(uint16_t netId, NetworkPermission permission);
    virtual ~NmdNetwork();

    void AddDefault();
    void RemoveDefault();

    int AddInterface(std::string &interfaceName);
    int RemoveInterface(std::string &interfaceName);
    int ClearInterfaces();
    bool ExistInterface(std::string &interfaceName);

    std::set<std::string> GetAllInterface()
    {
        return this->interfaces;
    }

    uint16_t GetNetId()
    {
        return this->netId;
    }

    NetworkPermission GetPermission()
    {
        return this->permission;
    }

private:
    uint16_t netId;
    bool isDefault = false;
    NetworkPermission permission;
    std::set<std::string> interfaces;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NMD_NETWORK_H__
