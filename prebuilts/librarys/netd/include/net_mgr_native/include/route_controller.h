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

#ifndef INCLUDE_ROUTE_CONTROLLER_H__
#define INCLUDE_ROUTE_CONTROLLER_H__

#include <map>
#include <netinet/in.h>
#include "nmd_network.h"

namespace OHOS {
namespace nmd {
typedef struct _inet_addr {
    int family;
    int bitlen;
    int prefixlen;
    uint8_t data[sizeof(struct in6_addr)];
} _inet_addr;

class route_controller {
public:
    route_controller();
    ~route_controller();

    static int createChildChains(const char *table, const char *parentChain, const char *childChain);
    static int addInterfaceToDefaultNetwork(const char *interface, NetworkPermission permission);
    static int removeInterfaceFromDefaultNetwork(const char *interface, NetworkPermission permission);
    static int addInterfaceToPhysicalNetwork(uint16_t netId, const char *interface, NetworkPermission permission);

    static int removeInterfaceFromPhysicalNetwork(
        uint16_t netId, const char *interfaceName, NetworkPermission permission);

    static int addRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);

    static int removeRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);

    static int read_addr(const char *addr, _inet_addr *res);
    static int read_addr_gw(const char *addr, _inet_addr *res);

private:
    static int executeIptablesRestore(std::string command);
    static void updateTableNamesFile();
    static std::map<std::string, uint32_t> interfaceToTable_;
    static uint32_t getRouteTableForInterface(const char *interfaceName);

    void modifyIpRule(std::string interface, NetworkPermission permission);
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_ROUTE_CONTROLLER_H__