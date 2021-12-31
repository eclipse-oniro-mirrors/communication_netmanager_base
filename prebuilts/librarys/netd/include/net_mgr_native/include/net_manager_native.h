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
#include <traffic_controller.h>
#include <vector>
#include "event_reporter.h"

namespace OHOS {
namespace nmd {
enum set_proc_sys_net {
    IPV4 = 4,
    IPV6 = 6,
    CONF = 1,
    NEIGH = 2,
};
typedef struct route_info_parcel {
    std::string destination;
    std::string ifName;
    std::string nextHop;
    int mtu;
} route_info_parcel;

typedef struct tether_offload_rule_parcel {
    int inputInterfaceIndex;
    int outputInterfaceIndex;
    std::vector<char> destination;
    int prefixLength;
    std::vector<char> srcL2Address;
    std::vector<char> dstL2Address;
    int pmtu = 1500;
} tether_offload_rule_parcel;

typedef struct tether_config_parcel {
    bool usingLegacyDnsProxy;
    std::vector<std::string> dhcpRanges;
} tether_config_parcel;

typedef struct mark_mask_parcel {
    int mark;
    int mask;
} mark_mask_parcel;
class NetManagerNative {
public:
    NetManagerNative();
    ~NetManagerNative();

    static void getOriginInterfaceIdex();
    static std::vector<unsigned int> getCurrentInterfaceIdex();
    static void updateInterfaceIdex(unsigned int infIdex);

    void initChildChains();
    void initUnixSocket();
    void initDnsConfFile();
    void init();

    int networkCreatePhysical(int netId, int permission);
    int networkDestroy(int netId);
    int networkAddInterface(int netId, std::string iface);
    int networkRemoveInterface(int netId, std::string iface);
    void socketDestroy(std::string iface);
    void socketDestroy(int netId);
    mark_mask_parcel getFwmarkForNetwork(int netId);
    int networkAddRoute(int netId, std::string ifName, std::string destination, std::string nextHop);
    int networkRemoveRoute(int netId, std::string ifName, std::string destination, std::string nextHop);
    int networkGetDefault();
    int networkSetDefault(int netId);
    int networkClearDefault();
    int networkSetPermissionForNetwork(int netId, NetworkPermission permission);
    std::vector<std::string> interfaceGetList();

    int setProcSysNet(int32_t ipversion, int32_t which, const std::string ifname, const std::string parameter,
        const std::string value);
    int getProcSysNet(int32_t ipversion, int32_t which, const std::string ifname, const std::string parameter,
        std::string *value);

    nmd::interface_configuration_parcel interfaceGetConfig(std::string ifName);
    void interfaceSetConfig(interface_configuration_parcel cfg);
    void interfaceClearAddrs(const std::string ifName);
    int interfaceGetMtu(std::string ifName);
    int interfaceSetMtu(std::string ifName, int mtuValue);
    int interfaceAddAddress(std::string ifName, std::string addrString, int prefixLength);
    int interfaceDelAddress(std::string ifName, std::string addrString, int prefixLength);

    void registerUnsolicitedEventListener(inetd_unsolicited_event_listener listener);
    int  networkAddRouteParcel(int netId, route_info_parcel routeInfo);
    int  networkRemoveRouteParcel(int netId, route_info_parcel routeInfo);

    long getCellularRxBytes();
    long getCellularTxBytes();
    long getAllRxBytes();
    long getAllTxBytes();
    long getUidTxBytes(int uid);
    long getUidRxBytes(int uid);
    traffic_stats_parcel interfaceGetStats(std::string interfaceName);
    long getIfaceRxBytes(std::string interfaceName);
    long getIfaceTxBytes(std::string interfaceName);
    long getTetherRxBytes();
    long getTetherTxBytes();

private:
    std::shared_ptr<network_controller> networkController_;
    std::shared_ptr<route_controller> routeController_;
    std::shared_ptr<interface_controller> interfaceController_;
    static std::vector<unsigned int> interfaceIdex_;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NET_MANAGER_NATIVE_H__
