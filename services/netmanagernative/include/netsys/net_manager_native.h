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

#ifndef INCLUDE_NET_MANAGER_NATIVE_H__
#define INCLUDE_NET_MANAGER_NATIVE_H__

#include <interface_controller.h>
#include <memory>
#include <network_controller.h>
#include <route_controller.h>
#include <string>
#include <vector>

namespace OHOS {
namespace nmd {
typedef struct RouteInfoParcel {
    std::string destination;
    std::string ifName;
    std::string nextHop;
    int mtu;
} RouteInfoParcel;

typedef struct MarkMaskParcel {
    int mark;
    int mask;
} MarkMaskParcel;

class NetManagerNative {
public:
    NetManagerNative();
    ~NetManagerNative();

    static void GetOriginInterfaceIndex();
    static std::vector<unsigned int> GetCurrentInterfaceIndex();
    static void UpdateInterfaceIndex(unsigned int infIndex);

    void Init();

    int NetworkCreatePhysical(int netId, int permission);
    int NetworkDestroy(int netId);
    int NetworkAddInterface(int netId, std::string iface);
    int NetworkRemoveInterface(int netId, std::string iface);

    MarkMaskParcel GetFwmarkForNetwork(int netId);
    int NetworkAddRoute(int netId, std::string ifName, std::string destination, std::string nextHop);
    int NetworkRemoveRoute(int netId, std::string ifName, std::string destination, std::string nextHop);
    int NetworkGetDefault();
    int NetworkSetDefault(int netId);
    int NetworkClearDefault();
    int NetworkSetPermissionForNetwork(int netId, NetworkPermission permission);
    std::vector<std::string> InterfaceGetList();

    int SetProcSysNet(int32_t ipversion, int32_t which, const std::string ifname, const std::string parameter,
        const std::string value);
    int GetProcSysNet(int32_t ipversion, int32_t which, const std::string ifname, const std::string parameter,
        std::string *value);

    nmd::InterfaceConfigurationParcel InterfaceGetConfig(std::string ifName);
    void InterfaceSetConfig(InterfaceConfigurationParcel cfg);
    void InterfaceClearAddrs(const std::string ifName);
    int InterfaceGetMtu(std::string ifName);
    int InterfaceSetMtu(std::string ifName, int mtuValue);
    int InterfaceAddAddress(std::string ifName, std::string addrString, int prefixLength);
    int InterfaceDelAddress(std::string ifName, std::string addrString, int prefixLength);

    int NetworkAddRouteParcel(int netId, RouteInfoParcel routeInfo);
    int NetworkRemoveRouteParcel(int netId, RouteInfoParcel routeInfo);

    long GetCellularRxBytes();
    long GetCellularTxBytes();
    long GetAllRxBytes();
    long GetAllTxBytes();
    long GetUidTxBytes(int uid);
    long GetUidRxBytes(int uid);
    long GetIfaceRxBytes(std::string interfaceName);
    long GetIfaceTxBytes(std::string interfaceName);
    long GetTetherRxBytes();
    long GetTetherTxBytes();

private:
    std::shared_ptr<NetworkController> networkController;
    std::shared_ptr<RouteController> routeController;
    std::shared_ptr<InterfaceController> interfaceController;
    static std::vector<unsigned int> interfaceIdex;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NET_MANAGER_NATIVE_H__
