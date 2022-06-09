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

#ifndef INCLUDE_ROUTE_MANAGER_H__
#define INCLUDE_ROUTE_MANAGER_H__

#include <map>
#include <netinet/in.h>
#include "nmd_network.h"
#include "route_type.h"

namespace OHOS {
namespace nmd {
typedef struct InetAddr {
    int family;
    int bitlen;
    int prefixlen;
    uint8_t data[sizeof(struct in6_addr)];
} InetAddr;

class RouteManager {
public:
    RouteManager();
    ~RouteManager();

    static int AddInterfaceToDefaultNetwork(const char *interface, NetworkPermission permission);
    static int RemoveInterfaceFromDefaultNetwork(const char *interface, NetworkPermission permission);

    static int AddRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);
    static int RemoveRoute(int netId, std::string interfaceName, std::string destination, std::string nextHop);

    static int ReadAddr(const char *addr, InetAddr *res);
    static int ReadAddrGw(const char *addr, InetAddr *res);

private:
    static std::map<std::string, uint32_t> interfaceToTable;
    static uint32_t GetRouteTableForInterface(const char *interfaceName);
    static int ModifyRule(uint32_t type, uint32_t table, uint8_t action, uint32_t priority);
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_ROUTE_MANAGER_H__
