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

#include "network_manager.h"
#include "route_manager.h"
#include "nmd_network.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
namespace {
    constexpr int32_t INTERFACE_UNSET = -1;
}

NetworkManager::~NetworkManager()
{
    for (std::map<int, NmdNetwork *>::iterator it = networks.begin(); it != networks.end(); ++it) {
        delete it->second;
    }
}

int NetworkManager::CreatePhysicalNetwork(uint16_t netId, NetworkPermission permission)
{
    NmdNetwork *nw = new NmdNetwork(netId, permission);
    this->networks[netId] = nw;
    return netId;
}

int NetworkManager::DestroyNetwork(int netId)
{
    std::tuple<bool, NmdNetwork *> net = this->FindNetworkById(netId);
    NmdNetwork *nw = std::get<1>(net);

    if (this->defaultNetId == netId) {
        nw->RemoveDefault();
        this->defaultNetId = 0;
    }

    if (std::get<0>(net)) {
        nw->ClearInterfaces();
    }

    this->networks.erase(netId);
    delete nw;

    return 1;
}

int NetworkManager::SetDefaultNetwork(int netId)
{
    if (this->defaultNetId == netId) {
        return netId;
    }

    // check if this network exists
    std::tuple<bool, NmdNetwork *> net = this->FindNetworkById(netId);
    if (std::get<0>(net)) {
        NmdNetwork *nw = std::get<1>(net);
        nw->AddDefault();
    }

    if (this->defaultNetId != 0) {
        net = this->FindNetworkById(this->defaultNetId);
        if (std::get<0>(net)) {
            NmdNetwork *nw = std::get<1>(net);
            nw->RemoveDefault();
        }
    }
    this->defaultNetId = netId;
    return this->defaultNetId;
}

int NetworkManager::ClearDefaultNetwork()
{
    if (this->defaultNetId != 0) {
        std::tuple<bool, NmdNetwork *> net = this->FindNetworkById(this->defaultNetId);
        if (std::get<0>(net)) {
            NmdNetwork *nw = std::get<1>(net);
            nw->RemoveDefault();
        }
    }
    this->defaultNetId = 0;
    return 1;
}

std::tuple<bool, NmdNetwork *> NetworkManager::FindNetworkById(int netId)
{
    NETNATIVE_LOGI("Entry NetworkManager::FindNetworkById");
    std::map<int, NmdNetwork *>::iterator it;
    for (it = this->networks.begin(); it != this->networks.end(); ++it) {
        if (netId == it->first) {
            return std::make_tuple(true, it->second);
        }
    }
    return std::make_tuple<bool, NmdNetwork *>(false, nullptr);
}

int NetworkManager::GetDefaultNetwork()
{
    return this->defaultNetId;
}

int NetworkManager::GetNetworkForInterface(std::string &interfaceName)
{
    NETNATIVE_LOGI("Entry NetworkManager::GetNetworkForInterface");
    std::map<int, NmdNetwork *>::iterator it;
    for (it = this->networks.begin(); it != this->networks.end(); ++it) {
        if (it->second->ExistInterface(interfaceName)) {
            return it->first;
        }
    }
    return INTERFACE_UNSET;
}

int NetworkManager::AddInterfaceToNetwork(int netId, std::string &interafceName)
{
    NETNATIVE_LOGI("Entry NetworkManager::AddInterfaceToNetwork");
    int alreadySetNetId = GetNetworkForInterface(interafceName);
    if ((alreadySetNetId != netId) && (alreadySetNetId != INTERFACE_UNSET)) {
        return -1;
    }

    std::tuple<bool, NmdNetwork *> net = this->FindNetworkById(netId);
    if (std::get<0>(net)) {
        NmdNetwork *nw = std::get<1>(net);
        return nw->AddInterface(interafceName);
    }
    return -1;
}

int NetworkManager::RemoveInterfaceFromNetwork(int netId, std::string &interafceName)
{
    int alreadySetNetId = GetNetworkForInterface(interafceName);
    if ((alreadySetNetId != netId) || (alreadySetNetId == INTERFACE_UNSET)) {
        return 1;
    } else if (alreadySetNetId == netId) {
        std::tuple<bool, NmdNetwork *> net = this->FindNetworkById(netId);
        if (std::get<0>(net)) {
            NmdNetwork *nw = std::get<1>(net);
            return nw->RemoveInterface(interafceName);
        }
    }
    return 1;
}

int NetworkManager::AddRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return RouteManager::AddRoute(netId, interfaceName, destination, nextHop);
}

int NetworkManager::RemoveRoute(
    int netId, std::string interfaceName, std::string destination, std::string nextHop)
{
    return RouteManager::RemoveRoute(netId, interfaceName, destination, nextHop);
}

int NetworkManager::GetFwmarkForNetwork(int netId)
{
    return 0;
}

int NetworkManager::SetPermissionForNetwork(int netId, NetworkPermission permission)
{
    return 0;
}

NmdNetwork *NetworkManager::GetNetwork(int netId)
{
    return networks.find(netId)->second;
}

std::vector<NmdNetwork *> NetworkManager::GetNetworks()
{
    std::vector<nmd::NmdNetwork *> nws;
    std::map<int, NmdNetwork *>::iterator it;
    for (it = this->networks.begin(); it != this->networks.end(); ++it) {
        nws.push_back(it->second);
    }
    return nws;
}
} // namespace nmd
} // namespace OHOS
