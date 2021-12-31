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
typedef enum Permission {
    PERMISSION_NONE = 0x0,
    PERMISSION_NETWORK = 0x1,
    PERMISSION_SYSTEM = 0x3,
} NetworkPermission;

class NmdNetwork {
public:
    NmdNetwork(uint16_t netId, NetworkPermission permission);

    void asDefault();
    void removeAsDefault();

    int addInterface(std::string &interfaceName);
    int removeInterface(std::string &interfaceName);
    int clearInterfaces();

    bool hasInterface(std::string &interfaceName);
    std::set<std::string> getAllInterface()
    {
        return this->interfaces_;
    }

    uint16_t getNetId()
    {
        return this->netId_;
    }
    NetworkPermission getPermission()
    {
        return this->permission_;
    }

    bool isDefault()
    {
        return this->isDefault_;
    }

    virtual ~NmdNetwork();

private:
    uint16_t netId_;

    bool isDefault_ = false;

    NetworkPermission permission_;

    std::set<std::string> interfaces_;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NMD_NETWORK_H__
