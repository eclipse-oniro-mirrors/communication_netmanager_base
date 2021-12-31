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

#ifndef INCLUDE_NETWORK_CONTROLLER_H__
#define INCLUDE_NETWORK_CONTROLLER_H__
#include <map>
#include <tuple>
#include <vector>
#include <set>
#include "nmd_network.h"
namespace OHOS {
namespace nmd {
class network_controller {
public:
    network_controller() = default;
    ~network_controller();

    int createPhysicalNetwork(uint16_t netId, Permission permission);

    int destroyNetwork(int netId);

    int setDefaultNetwork(int netId);

    int clearDefaultNetwork();

    int getDefaultNetwork();

    int addInterfaceToNetwork(int netId, std::string &interafceName);

    int removeInterfaceFromNetwork(int netId, std::string &interafceName);

    int addRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);

    int removeRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);

    int getFwmarkForNetwork(int netId);

    int setPermissionForNetwork(int netId, Permission permission);

    std::vector<nmd::NmdNetwork *> getNetworks();

    nmd::NmdNetwork *getNetwork(int netId);

private:
    int defaultNetId_;
    std::map<int, NmdNetwork *> networks_;
    std::tuple<bool, nmd::NmdNetwork *> findNetworkById(int netId);
    int getNetworkForInterface(std::string &interfaceName);
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETWORK_CONTROLLER_H__