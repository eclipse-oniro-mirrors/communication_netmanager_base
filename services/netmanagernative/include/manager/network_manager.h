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

#ifndef INCLUDE_NETWORK_MANAGER_H__
#define INCLUDE_NETWORK_MANAGER_H__

#include <map>
#include <vector>
#include <set>
#include "nmd_network.h"

namespace OHOS {
namespace nmd {
class NetworkManager {
public:
    NetworkManager() = default;
    ~NetworkManager();

    int CreatePhysicalNetwork(uint16_t netId, NetworkPermission permission);

    int DestroyNetwork(int netId);

    int SetDefaultNetwork(int netId);

    int ClearDefaultNetwork();

    int GetDefaultNetwork();

    int AddInterfaceToNetwork(int netId, std::string &interafceName);

    int RemoveInterfaceFromNetwork(int netId, std::string &interafceName);

    int AddRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);

    int RemoveRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);

    int GetFwmarkForNetwork(int netId);

    int SetPermissionForNetwork(int netId, NetworkPermission permission);

    std::vector<nmd::NmdNetwork *> GetNetworks();

    nmd::NmdNetwork *GetNetwork(int netId);

private:
    int defaultNetId;
    std::map<int, NmdNetwork *> networks;
    std::tuple<bool, nmd::NmdNetwork *> FindNetworkById(int netId);
    int GetNetworkForInterface(std::string &interfaceName);
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETWORK_MANAGER_H__
