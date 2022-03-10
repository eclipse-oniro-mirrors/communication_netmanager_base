/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "nmd_network.h"
#include "route_controller.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
NmdNetwork::NmdNetwork(uint16_t netId, NetworkPermission permission) : netId(netId), permission(permission) {}

NmdNetwork::~NmdNetwork() {}

void NmdNetwork::AddDefault()
{
    std::set<std::string>::iterator it;
    for (it = this->interfaces.begin(); it != this->interfaces.end(); ++it) {
        RouteController::AddInterfaceToDefaultNetwork(it->c_str(), this->permission);
    }
    this->isDefault = true;
}

void NmdNetwork::RemoveDefault()
{
    std::set<std::string>::iterator it;
    for (it = this->interfaces.begin(); it != this->interfaces.end(); ++it) {
        RouteController::RemoveInterfaceFromDefaultNetwork(it->c_str(), this->permission);
    }
    this->isDefault = false;
}

int NmdNetwork::AddInterface(std::string &interfaceName)
{
    NETNATIVE_LOGI("Entry NmdNetwork::AddInterface");
    if (ExistInterface(interfaceName)) {
        return 1;
    }

    if (this->isDefault) {
        RouteController::AddInterfaceToDefaultNetwork(interfaceName.c_str(), this->permission);
    }

    this->interfaces.insert(interfaceName);
    return 1;
}

int NmdNetwork::RemoveInterface(std::string &interfaceName)
{
    if (!ExistInterface(interfaceName)) {
        return 1;
    }

    if (this->isDefault) {
        RouteController::RemoveInterfaceFromDefaultNetwork(interfaceName.c_str(), this->permission);
    }

    this->interfaces.erase(interfaceName);
    return 1;
}

int NmdNetwork::ClearInterfaces()
{
    this->interfaces.clear();
    return 1;
}

bool NmdNetwork::ExistInterface(std::string &interfaceName)
{
    return this->interfaces.find(interfaceName) != this->interfaces.end();
}
} // namespace nmd
} // namespace OHOS