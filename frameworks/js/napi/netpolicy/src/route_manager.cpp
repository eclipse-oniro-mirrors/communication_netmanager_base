/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <net/if.h>
#include <netlink_socket.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <linux/fib_rules.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "fwmark.h"
#include "net_manager_constants.h"
#include "netlink_manager.h"
#include "netlink_msg.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "securec.h"
#include "distributed_manager.h"
#ifdef SUPPORT_SYSVPN
#include "iptables_wrapper.h"
#endif // SUPPORT_SYSVPN
#include "route_manager.h"

using namespace OHOS::NetManagerStandard;
using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace nmd {
namespace {
constexpr int32_t RULE_LEVEL_CLAT_TUN = 8000;
constexpr int32_t RULE_LEVEL_VPN_OUTPUT_TO_LOCAL = 9000;
constexpr int32_t RULE_LEVEL_SECURE_VPN = 10000;
constexpr int32_t RULE_LEVEL_VNIC_NETWORK = 10500;
constexpr int32_t RULE_LEVEL_EXPLICIT_NETWORK = 11000;
constexpr int32_t RULE_LEVEL_OUTPUT_IFACE_VPN = 11500;
constexpr int32_t RULE_LEVEL_OUTPUT_INTERFACE = 12000;
constexpr int32_t RULE_LEVEL_LOCAL_NETWORK = 13000;
constexpr int32_t RULE_LEVEL_SHARING = 14000;
#ifdef FEATURE_ENTERPRISE_ROUTE_CUSTOM
constexpr int32_t RULE_LEVEL_ENTERPRISE = 15000;
#endif
constexpr int32_t RULE_LEVEL_DEFAULT = 16000;
constexpr int32_t RULE_LEVEL_DISTRIBUTE_COMMUNICATION = 16500;
constexpr uint32_t ROUTE_DISTRIBUTE_TO_CLIENT_TABLE = 90;
constexpr uint32_t ROUTE_DISTRIBUTE_FROM_CLIENT_TABLE = 91;
constexpr uint32_t ROUTE_VNIC_TABLE = 97;
constexpr uint32_t ROUTE_VPN_NETWORK_TABLE = 98;
constexpr uint32_t ROUTE_LOCAL_NETWORK_TABLE = 99;
constexpr uint32_t OUTPUT_MAX = 128;
constexpr uint32_t BIT_32_LEN = 32;
constexpr uint32_t BIT_MAX_LEN = 255;
constexpr uint32_t DECIMAL_DIGITAL = 10;
constexpr uint32_t BYTE_ALIGNMENT = 8;
constexpr uint32_t ROUTE_TABLE_OFFSET_FROM_INDEX = 2000;
constexpr uint16_t LOCAL_NET_ID = 99;
constexpr uint16_t NETID_UNSET = 0;
constexpr uint32_t MARK_UNSET = 0;
#ifdef SUPPORT_SYSVPN
constexpr uint32_t DEFAULT_ROUTE_VPN_NETWORK_BASE_TABLE = 1000;
constexpr const char *XFRM_CARD_NAME = "xfrm-vpn";
constexpr const char *PPP_CARD_NAME = "ppp-vpn";
constexpr const char *TUN_CARD_NAME = "vpn-tun";
constexpr const char *MULTI_TUN_CARD_NAME = "multitun-vpn";
const std::string LOCAL_MANGLE_OUTPUT = "routectrl_mangle_OUTPUT";
#endif // SUPPORT_SYSVPN
constexpr uid_t UID_ROOT = 0;
constexpr std::pair<uid_t, uid_t> UID_ALLOW_INTERNAL = {7023, 7023};
constexpr int32_t ROUTEMANAGER_SUCCESS = 0;
constexpr int32_t ROUTEMANAGER_ERROR = -1;
constexpr bool ADD_CONTROL = true;
constexpr bool DEL_CONTROL = false;
const std::string RULEIIF_LOOPBACK = "lo";
const std::string RULEIIF_NULL = "";
const std::string RULEOIF_NULL = "";
const std::string RULEIP_NULL = "";
const std::string LOCAL_MANGLE_INPUT = "routectrl_mangle_INPUT";
constexpr const char *DISTRIBUTED_TUN_CARD_NAME = "virnic";
constexpr const char *NETSYS_ROUTE_INIT_DIR_PATH = "/data/service/el1/public/netmanager/route";
constexpr const char *DISTRIBUTED_TUN_CARD_NAME_VETH = "virnic-veth";
constexpr const char *IP_CMD_PATH = "/system/bin/ip";

struct FibRuleUidRange {
    __u32 start;
    __u32 end;
};
} // namespace

std::mutex RouteManager::interfaceToTableLock_;
std::map<std::string, uint32_t> RouteManager::interfaceToTable_;

#ifdef SUPPORT_SYSVPN
bool RouteManager::vpnSysCall_ = true;
std::string RouteManager::defauleNetWorkName_ = "";
#endif // SUPPORT_SYSVPN

RouteManager::RouteManager()
{
    Init();
#ifdef SUPPORT_SYSVPN
    InitOutcomingPacketMark();
#endif // SUPPORT_SYSVPN
}

int32_t RouteManager::UpdateVnicRoute(const std::string &interfaceName, const std::string &destinationName,
                                      const std::string &nextHop, bool add)
{
    NETNATIVE_LOGI(
        "VnicChangeRoute,interfaceName:%{public}s,destination:%{public}s, nextHop:%{public}s, add:%{public}d ",
        interfaceName.c_str(), ToAnonymousIp(destinationName).c_str(), ToAnonymousIp(nextHop).c_str(), add);

    RouteInfo routeInfo;
    routeInfo.routeTable = ROUTE_VNIC_TABLE;
    routeInfo.routeInterfaceName = interfaceName;
    routeInfo.routeDestinationName = destinationName;
    routeInfo.routeNextHop = nextHop;
    uint16_t flags = add ? (NLM_F_CREATE | NLM_F_EXCL) : NLM_F_EXCL;
    uint16_t action = add ? RTM_NEWROUTE : RTM_DELROUTE;

    return UpdateRouteRule(action, flags, routeInfo);
}

int32_t RouteManager::AddRoute(TableType tableType, NetworkRouteInfo networkRouteInfo, bool& routeRepeat)
{
    std::string interfaceName = networkRouteInfo.ifName;
    std::string destinationName = networkRouteInfo.destination;
    std::string nextHop = networkRouteInfo.nextHop;
    bool isExcludedRoute = networkRouteInfo.isExcludedRoute;
    NETNATIVE_LOGI("AddRoute,interfaceName:%{public}s,destination:%{public}s, nextHop:%{public}s, \
        isExcludedRoute:%{public}d", interfaceName.c_str(), ToAnonymousIp(destinationName).c_str(),
        ToAnonymousIp(nextHop).c_str(), isExcludedRoute);

    // This is a user-defined structure used to integrate the information required for setting up routes.
    RouteInfo routeInfo;
    if (SetRouteInfo(tableType, networkRouteInfo, routeInfo) != 0) {
        return -1;
    }

    int32_t ret = UpdateRouteRule(RTM_NEWROUTE, NLM_F_CREATE | NLM_F_EXCL, routeInfo);
    if (ret == EEXIST) {
        routeRepeat = true;
    } else {
        routeRepeat = false;
    }
    return ret;
}

int32_t RouteManager::RemoveRoute(TableType tableType, const std::string &interfaceName,
                                  const std::string &destinationName, const std::string &nextHop)
{
    NETNATIVE_LOGI("RemoveRoute,interfaceName:%{public}s,destination:%{public}s,nextHop:%{public}s",
                   interfaceName.c_str(), ToAnonymousIp(destinationName).c_str(), ToAnonymousIp(nextHop).c_str());

    NetworkRouteInfo networkRouteInfo;
    networkRouteInfo.ifName = interfaceName;
    networkRouteInfo.destination = destinationName;
    networkRouteInfo.nextHop = nextHop;
    RouteInfo routeInfo;
    if (SetRouteInfo(tableType, networkRouteInfo, routeInfo) != 0) {
        return -1;
    }
    return UpdateRouteRule(RTM_DELROUTE, NLM_F_EXCL, routeInfo);
}

int32_t RouteManager::UpdateRoute(TableType tableType, const std::string &interfaceName,
                                  const std::string &destinationName, const std::string &nextHop)
{
    NETNATIVE_LOGI("UpdateRoute,interfaceName:%{public}s,destination:%{public}s,nextHop:%{public}s",
                   interfaceName.c_str(), ToAnonymousIp(destinationName).c_str(), ToAnonymousIp(nextHop).c_str());
    NetworkRouteInfo networkRouteInfo;
    networkRouteInfo.ifName = interfaceName;
    networkRouteInfo.destination = destinationName;
    networkRouteInfo.nextHop = nextHop;
    RouteInfo routeInfo;
    if (SetRouteInfo(tableType, networkRouteInfo, routeInfo) != 0) {
        return -1;
    }
    return UpdateRouteRule(RTM_NEWROUTE, NLM_F_REPLACE, routeInfo);
}

int32_t RouteManager::AddInterfaceToDefaultNetwork(const std::string &interfaceName, NetworkPermission permission)
{
    NETNATIVE_LOGI("AddInterfaceToDefaultNetwork, %{public}s;permission:%{public}d;", interfaceName.c_str(),
                   permission);
#ifdef SUPPORT_SYSVPN
    defauleNetWorkName_ = interfaceName;
#endif // SUPPORT_SYSVPN
    uint32_t table = FindTableByInterfacename(interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }
    Fwmark fwmark;
    fwmark.netId = NETID_UNSET;
    fwmark.permission = permission;

    Fwmark mask;
    mask.netId = FWMARK_NET_ID_MASK;
    mask.permission = permission;

    // This is a user-defined structure used to integrate the information required for setting up rules.
    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_DEFAULT;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = RULEOIF_NULL;
    return UpdateRuleInfo(RTM_NEWRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::RemoveInterfaceFromDefaultNetwork(const std::string &interfaceName, NetworkPermission permission)
{
    NETNATIVE_LOGI("RemoveInterfaceFromDefaultNetwork, %{public}s;permission:%{public}d;", interfaceName.c_str(),
                   permission);
    uint32_t table = FindTableByInterfacename(interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }

    Fwmark fwmark;
    fwmark.netId = NETID_UNSET;
    fwmark.permission = permission;

    Fwmark mask;
    mask.netId = FWMARK_NET_ID_MASK;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_DEFAULT;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = RULEOIF_NULL;
    return UpdateRuleInfo(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::AddInterfaceToPhysicalNetwork(uint16_t netId, const std::string &interfaceName,
                                                    NetworkPermission permission)
{
    NETNATIVE_LOGI("AddInterfaceToPhysicalNetwork, netId:%{public}d;interfaceName:%{public}s;permission:%{public}d;",
                   netId, interfaceName.c_str(), permission);
    return UpdatePhysicalNetwork(netId, interfaceName, permission, ADD_CONTROL);
}

int32_t RouteManager::RemoveInterfaceFromPhysicalNetwork(uint16_t netId, const std::string &interfaceName,
                                                         NetworkPermission permission)
{
    NETNATIVE_LOGI("RemoveInterfacePhysicalNetwork, netId:%{public}d;interfaceName:%{public}s;permission:%{public}d;",
                   netId, interfaceName.c_str(), permission);
    if (int32_t ret = UpdatePhysicalNetwork(netId, interfaceName, permission, DEL_CONTROL)) {
        NETNATIVE_LOGE("UpdatePhysicalNetwork err, error is %{public}d", ret);
        return ret;
    }
    if (int32_t ret = ClearRoutes(interfaceName, netId)) {
        NETNATIVE_LOGE("ClearRoutes err, error is %{public}d", ret);
        return ret;
    }
    if (NetManagerStandard::IsInternalNetId(netId)) {
        NETNATIVE_LOGI("InternalNetId skip");
        return 0;
    }
    if (int32_t ret = ClearSharingRules(interfaceName)) {
        NETNATIVE_LOGE("ClearSharingRules err, error is %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::ModifyPhysicalNetworkPermission(uint16_t netId, const std::string &interfaceName,
                                                      NetworkPermission oldPermission, NetworkPermission newPermission)
{
    NETNATIVE_LOGI("ModifyPhysicalNetworkPermission, %{public}s", interfaceName.c_str());
    if (int32_t ret = UpdatePhysicalNetwork(netId, interfaceName, newPermission, ADD_CONTROL)) {
        NETNATIVE_LOGE("UpdatePhysicalNetwork err, error is %{public}d", ret);
        return ret;
    }

    return UpdatePhysicalNetwork(netId, interfaceName, newPermission, DEL_CONTROL);
}

#ifdef SUPPORT_SYSVPN
int32_t RouteManager::InitOutcomingPacketMark()
{
    NETNATIVE_LOGI("InitOutcomingPacketMark");
    // need to call IptablesWrapper's RunCommand function.
    std::string commandNew;
    commandNew.append("-t mangle -N ");
    commandNew.append(LOCAL_MANGLE_OUTPUT);
    if (IptablesWrapper::GetInstance()->RunCommand(IPTYPE_IPV4, commandNew) == ROUTEMANAGER_ERROR) {
        NETNATIVE_LOGI("InitOutcomingPacketMark error");
        return ROUTEMANAGER_ERROR;
    }

    std::string commandJump;
    commandJump.append("-t mangle");
    commandJump.append(" -A OUTPUT -j ");
    commandJump.append(LOCAL_MANGLE_OUTPUT);
    if (IptablesWrapper::GetInstance()->RunCommand(IPTYPE_IPV4, commandJump) == ROUTEMANAGER_ERROR) {
        NETNATIVE_LOGE("InitOutcomingPacketMark error");
        return ROUTEMANAGER_ERROR;
    }
    return ROUTEMANAGER_SUCCESS;
}

int32_t RouteManager::UpdateVpnRules(uint16_t netId, const std::string &interface,
    const std::vector<std::string> &extMessages, bool add)
{
    int32_t ret = ROUTEMANAGER_SUCCESS;
    if (interface.empty()) {
        NETNATIVE_LOGE("UpdateVpnRules err, vpn name is empty");
        return ROUTEMANAGER_ERROR;
    }
    NETNATIVE_LOG_D("update vpn rules on interface, %{public}s.", interface.c_str());
    bool isSysVpn = CheckSysVpnCall();
    bool isTunVpn = CheckTunVpnCall(interface);

    for (const auto& msg : extMessages) {
        if (!CommonUtils::IsValidIPV4(msg)) {
            NETNATIVE_LOGE("failed to add update vpn rules on interface of netId, %{public}u.", netId);
            return ROUTEMANAGER_ERROR;
        }
        if (isTunVpn) {
            NETNATIVE_LOGI("TUN mode, skipping update for interface: %{public}s", interface.c_str());
            continue;
        }
        if (isSysVpn) {
            ret = UpdateVpnOutPutPenetrationRule(netId, defauleNetWorkName_, msg, add);
        } else {
            ret = UpdateOutcomingIpMark(netId, msg, add);
        }
    }
    return ret;
}

int32_t RouteManager::UpdateOutcomingIpMark(uint16_t netId, const std::string &addr, bool add)
{
    NETNATIVE_LOGI("UpdateOutcomingIpMark,add===%{public}d", add);
    Fwmark fwmark;
    fwmark.netId = netId;
    NetworkPermission permission = NetworkPermission::PERMISSION_SYSTEM;
    fwmark.permission = permission;
    std::string action = "";
    if (add) {
        action = "-A ";
    } else {
        action = "-D ";
    }
    std::stringstream ss;
    ss << "-t mangle " << action << LOCAL_MANGLE_OUTPUT << " -s " << addr
    << " -j MARK --set-mark 0x" << std::nouppercase
    << std::hex << fwmark.intValue;
    // need to call IptablesWrapper's RunCommand function.
    if (IptablesWrapper::GetInstance()->RunCommand(IPTYPE_IPV4, ss.str()) == ROUTEMANAGER_ERROR) {
        NETNATIVE_LOGE("UpdateOutcomingIpMark error");
        return ROUTEMANAGER_ERROR;
    }
    return ROUTEMANAGER_SUCCESS;
}

int32_t RouteManager::UpdateOutcomingUidMark(uint16_t netId, uid_t startUid, uid_t endUid, bool add)
{
    NETNATIVE_LOGI("UpdateOutcomingUidMark,add===%{public}d", add);
    Fwmark fwmark;
    fwmark.netId = netId;
    NetworkPermission permission = NetworkPermission::PERMISSION_SYSTEM;
    fwmark.permission = permission;
    std::string action = "";
    if (add) {
        action = "-A ";
    } else {
        action = "-D ";
    }
    std::stringstream ss;
    ss << "-t mangle " << action << LOCAL_MANGLE_OUTPUT << " -m owner --uid-owner " << startUid << "-" << endUid
       << " -j MARK --set-mark 0x" << std::nouppercase
       << std::hex << fwmark.intValue;
    // need to call IptablesWrapper's RunCommand function.
    if (IptablesWrapper::GetInstance()->RunCommand(IPTYPE_IPV4, ss.str()) == ROUTEMANAGER_ERROR) {
        NETNATIVE_LOGE("UpdateOutcomingUidMark error");
        return ROUTEMANAGER_ERROR;
    }
    return ROUTEMANAGER_SUCCESS;
}

int32_t RouteManager::SetVpnCallMode(const std::string &message)
{
    std::lock_guard lock(interfaceToTableLock_);
    if (message.find("0") == std::string::npos) {
        vpnSysCall_ = true;
    } else {
        vpnSysCall_ = false;
    }
    NETNATIVE_LOG_D("vpnSysCall_ %{public}d", vpnSysCall_);
    return ROUTEMANAGER_SUCCESS;
}

bool RouteManager::CheckTunVpnCall(const std::string &vpnName)
{
    if (vpnName.empty()) {
        NETNATIVE_LOGE("CheckTunVpnCall err, vpn name is empty");
        return false;
    }
    NETNATIVE_LOG_D("vpnName %{public}s, vpnSysCall_ %{public}d", vpnName.c_str(), vpnSysCall_);
    if (vpnName.find(TUN_CARD_NAME) != std::string::npos) {
        return true;
    }
    return false;
}

bool RouteManager::CheckSysVpnCall()
{
    return vpnSysCall_;
}

bool RouteManager::CheckMultiVpnCall(const std::string &vpnName)
{
    if (vpnName.empty()) {
        NETNATIVE_LOGE("CheckTunVpnCall err, vpn name is empty");
        return false;
    }
    if (CheckTunVpnCall(vpnName)) {
        return false;
    } else {
        return !CheckSysVpnCall();
    }
    return false;
}

uint32_t RouteManager::GetVpnInterffaceToId(const std::string &ifName)
{
    if (ifName.find(XFRM_CARD_NAME) != std::string::npos) {
        return CommonUtils::StrToUint(ifName.substr(strlen(XFRM_CARD_NAME)));
    } else if (ifName.find(PPP_CARD_NAME) != std::string::npos) {
        return CommonUtils::StrToUint(ifName.substr(strlen(PPP_CARD_NAME)));
    } else if (ifName.find(MULTI_TUN_CARD_NAME) != std::string::npos) {
        return CommonUtils::StrToUint(ifName.substr(strlen(MULTI_TUN_CARD_NAME)));
    }
    return 0;
}

uint32_t RouteManager::FindVpnIdByInterfacename(VpnRuleIdType type, const std::string &interfaceName)
{
    std::lock_guard lock(interfaceToTableLock_);
    NETNATIVE_LOG_D("type %{public}d, interface %{public}s", type, interfaceName.c_str());
    uint32_t id = GetVpnInterffaceToId(interfaceName.c_str());
    switch (type) {
        case VpnRuleIdType::VPN_OUTPUT_TO_LOCAL:
            id = RULE_LEVEL_VPN_OUTPUT_TO_LOCAL - id;
            break;
        case VpnRuleIdType::VPN_SECURE:
            id = RULE_LEVEL_SECURE_VPN - id;
            break;
        case VpnRuleIdType::VPN_EXPLICIT_NETWORK:
            id = RULE_LEVEL_EXPLICIT_NETWORK - id;
            break;
        case VpnRuleIdType::VPN_OUTPUT_IFACE:
            id = RULE_LEVEL_OUTPUT_IFACE_VPN - id;
            break;
        case VpnRuleIdType::VPN_NETWORK_TABLE:
            id = DEFAULT_ROUTE_VPN_NETWORK_BASE_TABLE + id;
            break;
        default :
            NETNATIVE_LOGI("unkonw type %{public}d, interface %{public}s", type, interfaceName.c_str());
            break;
    }
    return id;
}
#endif // SUPPORT_SYSVPN

int32_t RouteManager::AddInterfaceToVirtualNetwork(int32_t netId, const std::string &interfaceName)
{
    return ModifyVirtualNetBasedRules(netId, interfaceName, true);
}

int32_t RouteManager::RemoveInterfaceFromVirtualNetwork(int32_t netId, const std::string &interfaceName)
{
    if (ModifyVirtualNetBasedRules(netId, interfaceName, false) != ROUTEMANAGER_SUCCESS) {
        return ROUTEMANAGER_ERROR;
    }
#ifdef SUPPORT_SYSVPN
    if (CheckMultiVpnCall(interfaceName)) {
        uint32_t tableId = FindVpnIdByInterfacename(VpnRuleIdType::VPN_NETWORK_TABLE, interfaceName);
        NETNATIVE_LOG_D("RemoveInterfaceFromVirtualNetwork, clear table %{public}d", tableId);
        return ClearRouteInfo(RTM_GETROUTE, tableId);
    } else {
        return ClearRouteInfo(RTM_GETROUTE, ROUTE_VPN_NETWORK_TABLE);
    }
#else
    return ClearRouteInfo(RTM_GETROUTE, ROUTE_VPN_NETWORK_TABLE);
#endif // SUPPORT_SYSVPN
}

int32_t RouteManager::ModifyVirtualNetBasedRules(int32_t netId, const std::string &ifaceName, bool add)
{
    NETNATIVE_LOGI("ModifyVirtualNetBasedRules,add===%{public}d", add);
    uint32_t table = GetRouteTableFromType(RouteManager::VPN_NETWORK, ifaceName);
    if (table == RT_TABLE_UNSPEC) {
        NETNATIVE_LOGE("table == RT_TABLE_UNSPEC, this is error");
        return ROUTEMANAGER_ERROR;
    }

    // If the rule fails to be added, continue to execute the next rule
    int32_t ret = UpdateVpnOutputToLocalRule(ifaceName, add);
#ifdef SUPPORT_SYSVPN
    ret += UpdateVpnSystemPermissionRule(netId, table, add, ifaceName);
    ret += UpdateExplicitNetworkRuleWithUid(netId, table, PERMISSION_NONE, UID_ROOT, UID_ROOT, add, ifaceName);
#else
    ret += UpdateVpnSystemPermissionRule(netId, table, add);
    ret += UpdateExplicitNetworkRuleWithUid(netId, table, PERMISSION_NONE, UID_ROOT, UID_ROOT, add);
#endif // SUPPORT_SYSVPN
    return ret;
}

#ifdef SUPPORT_SYSVPN
int32_t RouteManager::UpdateVpnOutPutPenetrationRule(int32_t netId, const std::string &interfaceName,
                                                     const std::string &ruleDstIp, bool add)
{
    RuleInfo ruleInfo;
    ruleInfo.ruleTable = FindTableByInterfacename(interfaceName);
    ruleInfo.rulePriority = RULE_LEVEL_VPN_OUTPUT_TO_LOCAL;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    ruleInfo.ruleOif = RULEOIF_NULL;
    ruleInfo.ruleDstIp = ruleDstIp;

    NETNATIVE_LOG_D("rule ruleDstIp %{public}s", ToAnonymousIp(ruleDstIp).c_str());
    return UpdateDistributedRule(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
}
#endif // SUPPORT_SYSVPN

int32_t RouteManager::UpdateVpnOutputToLocalRule(const std::string &interfaceName, bool add)
{
    RuleInfo ruleInfo;
    ruleInfo.ruleTable = ROUTE_LOCAL_NETWORK_TABLE;
#ifdef SUPPORT_SYSVPN
    if (CheckMultiVpnCall(interfaceName)) {
        ruleInfo.rulePriority = FindVpnIdByInterfacename(VpnRuleIdType::VPN_OUTPUT_TO_LOCAL, interfaceName);
    } else {
        ruleInfo.rulePriority = RULE_LEVEL_VPN_OUTPUT_TO_LOCAL;
    }
    NETNATIVE_LOG_D("rule priority %{public}d", ruleInfo.rulePriority);
#else
    ruleInfo.rulePriority = RULE_LEVEL_VPN_OUTPUT_TO_LOCAL;
#endif // SUPPORT_SYSVPN

    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    if (interfaceName.find("vpn") == std::string::npos) {
        ruleInfo.ruleIif = interfaceName;
    }
    ruleInfo.ruleOif = RULEOIF_NULL;

    return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
}

int32_t RouteManager::UpdateVpnSystemPermissionRule(int32_t netId, uint32_t table, bool add,
    const std::string &interfaceName)
{
    Fwmark fwmark;
    fwmark.netId = netId;
    NetworkPermission permission = NetworkPermission::PERMISSION_SYSTEM;
    fwmark.permission = permission;

    Fwmark mask;
    mask.netId = FWMARK_NET_ID_MASK;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
#ifdef SUPPORT_SYSVPN
    if (CheckMultiVpnCall(interfaceName)) {
        ruleInfo.rulePriority = FindVpnIdByInterfacename(VpnRuleIdType::VPN_SECURE, interfaceName);
    } else {
        ruleInfo.rulePriority = RULE_LEVEL_SECURE_VPN;
    }
    NETNATIVE_LOG_D("rule priority %{public}d", ruleInfo.rulePriority);
    if (CheckMultiVpnCall(interfaceName) || CheckTunVpnCall(interfaceName)) {
        NETNATIVE_LOGI("is ext vpn, add fwmark");
        ruleInfo.ruleFwmark = fwmark.intValue;
        ruleInfo.ruleMask = mask.intValue;
    }
#else
    ruleInfo.rulePriority = RULE_LEVEL_SECURE_VPN;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
#endif // SUPPORT_SYSVPN
    ruleInfo.ruleIif = RULEIIF_NULL;
    ruleInfo.ruleOif = RULEOIF_NULL;

    return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
}

int32_t RouteManager::AddUsersToVirtualNetwork(int32_t netId, const std::string &interfaceName,
                                               const std::vector<NetManagerStandard::UidRange> &uidRanges)
{
    return UpdateVirtualNetwork(netId, interfaceName, uidRanges, true);
}

int32_t RouteManager::RemoveUsersFromVirtualNetwork(int32_t netId, const std::string &interfaceName,
                                                    const std::vector<NetManagerStandard::UidRange> &uidRanges)
{
    return UpdateVirtualNetwork(netId, interfaceName, uidRanges, false);
}

int32_t RouteManager::UpdateVirtualNetwork(int32_t netId, const std::string &interfaceName,
                                           const std::vector<NetManagerStandard::UidRange> &uidRanges, bool add)
{
    NETNATIVE_LOGI("UpdateVirtualNetwork, add == %{public}d", add);
    uint32_t table = GetRouteTableFromType(RouteManager::VPN_NETWORK, interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        NETNATIVE_LOGE("table == RT_TABLE_UNSPEC, this is error");
        return ROUTEMANAGER_ERROR;
    }
    int32_t ret = ROUTEMANAGER_SUCCESS;
    for (auto range : uidRanges) {
        // If the rule fails to be added, continue to execute the next rule
#ifdef SUPPORT_SYSVPN
        ret += UpdateVpnUidRangeRule(table, range.begin_, range.end_, add, interfaceName);
        ret += UpdateExplicitNetworkRuleWithUid(netId, table, PERMISSION_NONE, range.begin_, range.end_, add,
                                                interfaceName);
        ret += UpdateOutputInterfaceRulesWithUid(interfaceName, table, PERMISSION_NONE, range.begin_, range.end_, add);

        if (CheckMultiVpnCall(interfaceName)) {
            NETNATIVE_LOGI("is ext vpn, add uid mark");
            ret += UpdateOutcomingUidMark(netId, range.begin_, range.end_, add);
            if (ret != ROUTEMANAGER_SUCCESS) {
                NETNATIVE_LOGE("add uid mark error.");
            }
        }
#else
        ret += UpdateVpnUidRangeRule(table, range.begin_, range.end_, add);
        ret += UpdateExplicitNetworkRuleWithUid(netId, table, PERMISSION_NONE, range.begin_, range.end_, add);
        ret += UpdateOutputInterfaceRulesWithUid(interfaceName, table, PERMISSION_NONE, range.begin_, range.end_, add);
#endif // SUPPORT_SYSVPN
    }
    return ret;
}

int32_t RouteManager::UpdateVnicUidRangesRule(const std::vector<NetManagerStandard::UidRange> &uidRanges, bool add)
{
    int32_t ret = ROUTEMANAGER_SUCCESS;
    for (const auto &range : uidRanges) {
        Fwmark fwmark;
        Fwmark mask;
        fwmark.protectedFromVpn = false;
        mask.protectedFromVpn = false;

        RuleInfo ruleInfo;
        ruleInfo.ruleTable = ROUTE_VNIC_TABLE;
        ruleInfo.rulePriority = RULE_LEVEL_VNIC_NETWORK;
        ruleInfo.ruleFwmark = fwmark.intValue;
        ruleInfo.ruleMask = mask.intValue;
        ruleInfo.ruleIif = RULEIIF_LOOPBACK;
        ruleInfo.ruleOif = RULEOIF_NULL;
        ret += UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, range.begin_, range.end_);
    }
    return ret;
}

int32_t RouteManager::EnableDistributedClientNet(const std::string &virNicAddr, const std::string &iif)
{
    NETNATIVE_LOGI("EnableDistributedClientNet virNicAddr:%{public}s,iif:%{public}s",
                   ToAnonymousIp(virNicAddr).c_str(), iif.c_str());
    int32_t ret = DistributedManager::GetInstance().ConfigVirnicAndVeth(virNicAddr, DISTRIBUTED_TUN_CARD_NAME,
        DISTRIBUTED_TUN_CARD_NAME_VETH);
    if (ret != ROUTEMANAGER_SUCCESS) {
        NETNATIVE_LOGE("ConfigVirnicAndVeth err, error is %{public}d", ret);
        return ret;
    }
    NETNATIVE_LOGI("EnableDistributedClientNet ConfigVirnicAndVeth success.");
    RuleInfo ruleInfo;
    ruleInfo.ruleTable = ROUTE_DISTRIBUTE_TO_CLIENT_TABLE;
    ruleInfo.rulePriority = RULE_LEVEL_DISTRIBUTE_COMMUNICATION;
    ruleInfo.ruleIif = iif;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    ret = UpdateDistributedRule(RTM_NEWRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
    if (ret != ROUTEMANAGER_SUCCESS) {
        NETNATIVE_LOGE("EnableDistributedClientNet UpdateDistributedRule err, error is %{public}d", ret);
        return ret;
    }
    std::string maskAddr = CommonUtils::GetMaskByLength(DEFAULT_GATEWAY_MASK_MAX_LENGTH);
    std::string virNicVethAddr = CommonUtils::GetGatewayAddr(virNicAddr, maskAddr);
    if (virNicVethAddr.empty()) {
        NETNATIVE_LOGE("get gateway addr is empty");
        return ROUTEMANAGER_ERROR;
    }
    uint32_t table = if_nametoindex(DISTRIBUTED_TUN_CARD_NAME);
    if (table == 0) {
        NETNATIVE_LOGE("create Virnic Route, if_nametoindex error %{public}d", errno);
        return -errno;
    }
    table += ROUTE_TABLE_OFFSET_FROM_INDEX;

    std::string out;
    std::string createVirnicRoute = std::string(IP_CMD_PATH) + " route add default via " + virNicVethAddr +
        " dev " + DISTRIBUTED_TUN_CARD_NAME + " table " + std::to_string(table) + " proto static";
    NETNATIVE_LOGI("create Virnic Route: %{public}s", CommonUtils::AnonymousIpInStr(createVirnicRoute).c_str());
    if (CommonUtils::ForkExec(createVirnicRoute.c_str(), &out) != ROUTEMANAGER_SUCCESS) {
        NETNATIVE_LOGE("create Virnic Route failed, output %{public}s", out.c_str());
        return ROUTEMANAGER_ERROR;
    }
    NETNATIVE_LOGI("EnableDistributedClientNet add route success.");
    return ROUTEMANAGER_SUCCESS;
}

int32_t RouteManager::AddServerUplinkRoute(const std::string &UplinkIif, const std::string &devIface)
{
    RuleInfo ruleInfo;
    ruleInfo.ruleTable = ROUTE_DISTRIBUTE_FROM_CLIENT_TABLE;
    ruleInfo.rulePriority = RULE_LEVEL_DISTRIBUTE_COMMUNICATION;
    ruleInfo.ruleIif = UplinkIif;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    int32_t ret = UpdateDistributedRule(RTM_NEWRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
    if (ret != ROUTEMANAGER_SUCCESS) {
        NETNATIVE_LOGE("EnableDistributedServerNet Update Uplink RuleInfo err, error is %{public}d", ret);
        return ret;
    }

    RouteInfo routeInfo;
    routeInfo.routeTable = ROUTE_DISTRIBUTE_FROM_CLIENT_TABLE;
    routeInfo.routeInterfaceName = devIface;
    routeInfo.routeDestinationName = "0.0.0.0/0";
    routeInfo.routeNextHop = "0.0.0.0";
    uint16_t flags = (NLM_F_CREATE | NLM_F_EXCL);
    uint16_t action = RTM_NEWROUTE;
    ret = UpdateRouteRule(action, flags, routeInfo);
    if (ret != ROUTEMANAGER_SUCCESS) {
        NETNATIVE_LOGE("EnableDistributedServerNet Update Uplink RouteRule err, NLM_F_REPLACE");
        if (UpdateRouteRule(RTM_NEWROUTE, NLM_F_REPLACE, routeInfo)) {
            UpdateDistributedRule(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
            return ROUTEMANAGER_ERROR;
        }
    }
    NETNATIVE_LOGE("EnableDistributedServerNet AddServerUplinkRoute success");

    return ROUTEMANAGER_SUCCESS;
}

int32_t RouteManager::AddServerDownlinkRoute(const std::string &UplinkIif, const std::string &dstAddr)
{
    RuleInfo ruleInfo;
    ruleInfo.ruleTable = ROUTE_DISTRIBUTE_TO_CLIENT_TABLE;
    ruleInfo.rulePriority = RULE_LEVEL_DISTRIBUTE_COMMUNICATION;
    ruleInfo.ruleDstIp = dstAddr;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    int32_t ret = UpdateDistributedRule(RTM_NEWRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
    if (ret != ROUTEMANAGER_SUCCESS) {
        NETNATIVE_LOGE("EnableDistributedServerNet Update Downlink RuleInfo err, error is %{public}d", ret);
        return ret;
    }

    RouteInfo routeInfo;
    routeInfo.routeTable = ROUTE_DISTRIBUTE_TO_CLIENT_TABLE;
    routeInfo.routeInterfaceName = UplinkIif;
    routeInfo.routeDestinationName = "0.0.0.0/0";
    routeInfo.routeNextHop = "0.0.0.0";
    uint16_t flags = (NLM_F_CREATE | NLM_F_EXCL);
    uint16_t action = RTM_NEWROUTE;
    ret = UpdateRouteRule(action, flags, routeInfo);
    if (ret != ROUTEMANAGER_SUCCESS) {
        NETNATIVE_LOGE("EnableDistributedServerNet Update Downlink RouteRule err, NLM_F_REPLACE");
        if (UpdateRouteRule(RTM_NEWROUTE, NLM_F_REPLACE, routeInfo)) {
            UpdateDistributedRule(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
            return ROUTEMANAGER_ERROR;
        }
    }
    NETNATIVE_LOGE("EnableDistributedServerNet AddServerDownlinkRoute success");

    return ROUTEMANAGER_SUCCESS;
}

int32_t RouteManager::EnableDistributedServerNet(const std::string &iif, const std::string &devIface,
                                                 const std::string &dstAddr)
{
    NETNATIVE_LOGI("EnableDistributedServerNet iif:%{public}s,devIface:%{public}s,dstAddr:%{public}s",
                   iif.c_str(), devIface.c_str(), ToAnonymousIp(dstAddr).c_str());

    int32_t ret = ROUTEMANAGER_SUCCESS;
    DistributedManager::GetInstance().SetServerNicInfo(iif, devIface);
    ret += AddServerUplinkRoute(iif, devIface);
    ret += AddServerDownlinkRoute(iif, dstAddr);

    return ret;
}

int32_t RouteManager::DisableDistributedNet(bool isServer)
{
    NETNATIVE_LOGI("DisableDistributedNet Enter, isServer:%{public}d", isServer);
    RuleInfo ruleInfo;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    ruleInfo.ruleIif = RULEIIF_NULL;
    ruleInfo.ruleOif = RULEOIF_NULL;
    ruleInfo.ruleTable = RT_TABLE_UNSPEC;
    ruleInfo.rulePriority = RULE_LEVEL_DISTRIBUTE_COMMUNICATION;
    RouteInfo routeInfo;
    routeInfo.routeTable = ROUTE_DISTRIBUTE_TO_CLIENT_TABLE;
    routeInfo.routeInterfaceName = DISTRIBUTED_TUN_CARD_NAME;
    routeInfo.routeDestinationName = "0.0.0.0/0";
    routeInfo.routeNextHop = "0.0.0.0";
    int32_t ret = ROUTEMANAGER_SUCCESS;
    if (isServer) {
        ret += UpdateDistributedRule(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
        if (ret != ROUTEMANAGER_SUCCESS) {
            NETNATIVE_LOGE("del server uplink rule err, rule prio is %{public}d", ruleInfo.rulePriority);
        }
        ret += UpdateDistributedRule(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
        if (ret != ROUTEMANAGER_SUCCESS) {
            NETNATIVE_LOGE("del server downlink rule err, rule prio is %{public}d", ruleInfo.rulePriority);
        }
        routeInfo.routeTable = ROUTE_DISTRIBUTE_FROM_CLIENT_TABLE;
        routeInfo.routeInterfaceName = DistributedManager::GetInstance().GetServerDevIfaceNic();
        ret += UpdateRouteRule(RTM_DELROUTE, NLM_F_EXCL, routeInfo);
        if (ret != ROUTEMANAGER_SUCCESS) {
            NETNATIVE_LOGE("del server uplink route err, route table is %{public}d", routeInfo.routeTable);
        }
        routeInfo.routeTable = ROUTE_DISTRIBUTE_TO_CLIENT_TABLE;
        routeInfo.routeInterfaceName = DistributedManager::GetInstance().GetServerIifNic();
        ret += UpdateRouteRule(RTM_DELROUTE, NLM_F_EXCL, routeInfo);
        if (ret != ROUTEMANAGER_SUCCESS) {
            NETNATIVE_LOGE("del server downlink route err, route table is %{public}d", routeInfo.routeTable);
        }
    } else {
        ret += UpdateDistributedRule(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, INVALID_UID, INVALID_UID);
        if (ret != ROUTEMANAGER_SUCCESS) {
            NETNATIVE_LOGE("del client rule err, rule prio is %{public}d", ruleInfo.rulePriority);
        }
        DistributedManager::GetInstance().DisableVirnic(DISTRIBUTED_TUN_CARD_NAME);
    }
    return ret;
}

int32_t RouteManager::UpdateVpnUidRangeRule(uint32_t table, uid_t uidStart, uid_t uidEnd, bool add,
    const std::string &interfaceName)
{
    Fwmark fwmark;
    Fwmark mask;
    fwmark.protectedFromVpn = false;
    mask.protectedFromVpn = true;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
#ifdef SUPPORT_SYSVPN
    if (CheckMultiVpnCall(interfaceName)) {
        ruleInfo.rulePriority = FindVpnIdByInterfacename(VpnRuleIdType::VPN_SECURE, interfaceName);
    } else {
        ruleInfo.rulePriority = RULE_LEVEL_SECURE_VPN;
    }
    NETNATIVE_LOG_D("rule priority %{public}d", ruleInfo.rulePriority);
#else
    ruleInfo.rulePriority = RULE_LEVEL_SECURE_VPN;
#endif // SUPPORT_SYSVPN

    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = RULEOIF_NULL;
    return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, uidStart, uidEnd);
}

int32_t RouteManager::UpdateExplicitNetworkRuleWithUid(int32_t netId, uint32_t table, NetworkPermission permission,
                                                       uid_t uidStart, uid_t uidEnd, bool add,
                                                       const std::string &interfaceName)
{
    NETNATIVE_LOGI("UpdateExplicitNetworkRuleWithUid");
    Fwmark fwmark;
    fwmark.netId = netId;
    fwmark.explicitlySelected = true;
    fwmark.permission = permission;

    Fwmark mask;
    mask.netId = FWMARK_NET_ID_MASK;
    mask.explicitlySelected = true;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
#ifdef SUPPORT_SYSVPN
    if (CheckMultiVpnCall(interfaceName)) {
        ruleInfo.rulePriority = FindVpnIdByInterfacename(VpnRuleIdType::VPN_EXPLICIT_NETWORK, interfaceName);
    } else {
        ruleInfo.rulePriority = RULE_LEVEL_EXPLICIT_NETWORK;
    }
    NETNATIVE_LOG_D("rule priority %{public}d", ruleInfo.rulePriority);
#else
    ruleInfo.rulePriority = RULE_LEVEL_EXPLICIT_NETWORK;
#endif // SUPPORT_SYSVPN
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = RULEOIF_NULL;

    return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, uidStart, uidEnd);
}

int32_t RouteManager::UpdateOutputInterfaceRulesWithUid(const std::string &interface, uint32_t table,
                                                        NetworkPermission permission, uid_t uidStart, uid_t uidEnd,
                                                        bool add)
{
    NETNATIVE_LOGI("UpdateOutputInterfaceRulesWithUid interface:%{public}s", interface.c_str());
    Fwmark fwmark;
    fwmark.permission = permission;

    Fwmark mask;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
#ifdef SUPPORT_SYSVPN
    if (CheckMultiVpnCall(interface)) {
        ruleInfo.rulePriority = FindVpnIdByInterfacename(VpnRuleIdType::VPN_OUTPUT_IFACE, interface);
    } else {
        ruleInfo.rulePriority = RULE_LEVEL_OUTPUT_IFACE_VPN;
    }
    NETNATIVE_LOG_D("UpdateOutputInterfaceRulesWithUid rule priority %{public}d", ruleInfo.rulePriority);
#else
    ruleInfo.rulePriority = RULE_LEVEL_OUTPUT_IFACE_VPN;
#endif // SUPPORT_SYSVPN
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = interface;

    return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, uidStart, uidEnd);
}

int32_t RouteManager::AddInterfaceToLocalNetwork(uint16_t netId, const std::string &interfaceName)
{
    NETNATIVE_LOGI("AddInterfaceToLocalNetwork, %{public}s", interfaceName.c_str());
    if (int32_t ret = UpdateLocalNetwork(netId, interfaceName, ADD_CONTROL)) {
        NETNATIVE_LOGE("UpdateLocalNetwork err, error is %{public}d", ret);
        return ret;
    }
    std::lock_guard lock(interfaceToTableLock_);
    interfaceToTable_[interfaceName] = ROUTE_LOCAL_NETWORK_TABLE;

    return 0;
}

int32_t RouteManager::RemoveInterfaceFromLocalNetwork(uint16_t netId, const std::string &interfaceName)
{
    NETNATIVE_LOGI("RemoveInterfaceFromLocalNetwork");
    if (int32_t ret = UpdateLocalNetwork(netId, interfaceName, DEL_CONTROL)) {
        NETNATIVE_LOGE("UpdateLocalNetwork err, error is %{public}d", ret);
        return ret;
    }
    std::lock_guard lock(interfaceToTableLock_);
    interfaceToTable_.erase(interfaceName);

    return 0;
}

int32_t RouteManager::EnableSharing(const std::string &inputInterface, const std::string &outputInterface)
{
    return UpdateSharingNetwork(RTM_NEWRULE, inputInterface, outputInterface);
}

int32_t RouteManager::DisableSharing(const std::string &inputInterface, const std::string &outputInterface)
{
    return UpdateSharingNetwork(RTM_DELRULE, inputInterface, outputInterface);
}

int32_t RouteManager::ReadAddrGw(const std::string &addr, InetAddr *res)
{
    if (res == nullptr) {
        return -1;
    }

    std::string addressString(addr.c_str());
    if (strchr(addr.c_str(), ':')) {
        res->family = AF_INET6;
        res->bitlen = OUTPUT_MAX;
    } else {
        res->family = AF_INET;
        res->bitlen = BIT_32_LEN;
    }

    return inet_pton(res->family, addressString.c_str(), res->data);
}

int32_t RouteManager::ReadAddr(const std::string &addr, InetAddr *res)
{
    if (res == nullptr) {
        return -EINVAL;
    }

    const char *slashStr = strchr(addr.c_str(), '/');
    if (slashStr == nullptr) {
        return -EINVAL;
    }

    const char *maskLenStr = slashStr + 1;
    if (*maskLenStr == 0) {
        return -EINVAL;
    }

    char *endptr = nullptr;
    unsigned templen = strtoul(maskLenStr, &endptr, DECIMAL_DIGITAL);
    if ((endptr == nullptr) || (templen > BIT_MAX_LEN)) {
        return -EINVAL;
    }
    res->prefixlen = templen;

    std::string addressString(addr.c_str(), slashStr - addr.c_str());
    if (strchr(addr.c_str(), ':')) {
        res->family = AF_INET6;
        res->bitlen = OUTPUT_MAX;
    } else {
        res->family = AF_INET;
        res->bitlen = BIT_32_LEN;
    }

    return inet_pton(res->family, addressString.c_str(), res->data);
}

int32_t RouteManager::AddClatTunInterface(const std::string &interfaceName, const std::string &dstAddr,
                                          const std::string &nxtHop)
{
    NETNATIVE_LOGI("AddClatTunInterface, interfaceName:%{public}s; dstAddr:%{public}s; nxtHop:%{public}s;",
                   interfaceName.c_str(), dstAddr.c_str(), nxtHop.c_str());
    bool routeRepeat = false;
    NetworkRouteInfo networkRouteInfo;
    networkRouteInfo.ifName = interfaceName;
    networkRouteInfo.destination = dstAddr;
    networkRouteInfo.nextHop = nxtHop;
    networkRouteInfo.isExcludedRoute = false;
    if (int32_t ret = AddRoute(RouteManager::INTERFACE, networkRouteInfo, routeRepeat)) {
        NETNATIVE_LOGE("AddRoute err, error is %{public}d", ret);
        return ret;
    }
    return UpdateClatTunInterface(interfaceName, PERMISSION_NONE, ADD_CONTROL);
}

int32_t RouteManager::RemoveClatTunInterface(const std::string &interfaceName)
{
    NETNATIVE_LOGI("RemoveClatTunInterface, interfaceName:%{public}s", interfaceName.c_str());
    if (int32_t ret = UpdateClatTunInterface(interfaceName, PERMISSION_NONE, DEL_CONTROL)) {
        NETNATIVE_LOGE("UpdatePhysicalNetwork err, error is %{public}d", ret);
        return ret;
    }
    if (int32_t ret = ClearRoutes(interfaceName)) {
        NETNATIVE_LOGE("ClearRoutes err, error is %{public}d", ret);
        return ret;
    }
    if (int32_t ret = ClearSharingRules(interfaceName)) {
        NETNATIVE_LOGE("ClearSharingRules err, error is %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::UpdateClatTunInterface(const std::string &interfaceName, NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("UpdateClatTunInterface, interfaceName: %{public}s, permission: %{public}d, add: %{public}d",
                   interfaceName.c_str(), static_cast<int32_t>(permission), add);
    uint32_t table = FindTableByInterfacename(interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        NETNATIVE_LOGE("table == RT_TABLE_UNSPEC, this is error");
        return -1;
    }

    Fwmark fwmark;
    fwmark.permission = permission;

    Fwmark mask;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_CLAT_TUN;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = RULEOIF_NULL;

    if (int32_t ret = UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo)) {
        NETNATIVE_LOG_D("UpdateRuleInfo failed, err is %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::Init()
{
    NETNATIVE_LOGI("Init");
    // need to call IptablesWrapper's RunCommand function.
    std::string commandNew;
    commandNew.append(" -t mangle -N ");
    commandNew.append(LOCAL_MANGLE_INPUT);

    std::string commandJump;
    commandJump.append(" -A INPUT -j ");
    commandJump.append(LOCAL_MANGLE_INPUT);

    if (int32_t ret = ClearRules()) {
        NETNATIVE_LOGE("ClearRules failed, err is %{public}d", ret);
        return ret;
    }

    if (access(NETSYS_ROUTE_INIT_DIR_PATH, F_OK) == 0) {
        if (int32_t ret = AddLocalNetworkRules()) {
            NETNATIVE_LOGE("AddLocalNetworkRules failed, err is %{public}d", ret);
            return ret;
        }
    } else {
        NETNATIVE_LOGI("AddLocalNetworkRules init ok, do not need repeat");
    }

    return 0;
}

int32_t RouteManager::ClearRules()
{
    return ClearRouteInfo(RTM_GETRULE, 0) >= 0 ? 0 : -1;
}

int32_t RouteManager::ClearRoutes(const std::string &interfaceName, int32_t netId)
{
    uint32_t table = FindTableByInterfacename(interfaceName, netId);
    NETNATIVE_LOGI("ClearRoutes--table==:%{public}d", table);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }
    int32_t ret = ClearRouteInfo(RTM_GETROUTE, table);
    if (ret == 0 && table > ROUTE_INTERNAL_DEFAULT_TABLE) {
        interfaceToTable_.erase(interfaceName);
    }

    return 0;
}

int32_t RouteManager::AddLocalNetworkRules()
{
    NETNATIVE_LOGI("AddLocalNetworkRules");
    if (int32_t ret =
            UpdateExplicitNetworkRule(LOCAL_NET_ID, ROUTE_LOCAL_NETWORK_TABLE, PERMISSION_NONE, ADD_CONTROL)) {
        NETNATIVE_LOGE("UpdateExplicitNetworkRule failed, err is %{public}d", ret);
        return ret;
    }
    Fwmark fwmark;
    fwmark.explicitlySelected = false;

    Fwmark mask;
    mask.explicitlySelected = true;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = ROUTE_LOCAL_NETWORK_TABLE;
    ruleInfo.rulePriority = RULE_LEVEL_LOCAL_NETWORK;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_NULL;
    ruleInfo.ruleOif = RULEOIF_NULL;

    return UpdateRuleInfo(RTM_NEWRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::UpdatePhysicalNetwork(uint16_t netId, const std::string &interfaceName,
                                            NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("UpdatePhysicalNetwork,add===%{public}d", add);
    uint32_t table = FindTableByInterfacename(interfaceName, netId);
    if (table == RT_TABLE_UNSPEC) {
        NETNATIVE_LOGE("table == RT_TABLE_UNSPEC, this is error");
        return -1;
    }

    if (int32_t ret = UpdateExplicitNetworkRule(netId, table, permission, add)) {
        NETNATIVE_LOGE("UpdateExplicitNetworkRule failed, err is %{public}d", ret);
        return ret;
    }

    if (int32_t ret = UpdateOutputInterfaceRules(interfaceName, table, permission, add)) {
        NETNATIVE_LOGE("UpdateOutputInterfaceRules failed, err is %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::UpdateLocalNetwork(uint16_t netId, const std::string &interfaceName, bool add)
{
    NETNATIVE_LOGI("UpdateLocalNetwork");
    return UpdateOutputInterfaceRules(interfaceName, ROUTE_LOCAL_NETWORK_TABLE, PERMISSION_NONE, add);
}

int32_t RouteManager::UpdateIncomingPacketMark(uint16_t netId, const std::string &interfaceName,
                                               NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("UpdateIncomingPacketMark");
    Fwmark fwmark;
    fwmark.netId = netId;
    fwmark.explicitlySelected = true;
    fwmark.protectedFromVpn = true;
    fwmark.permission = permission;
    const uint32_t mask = ~Fwmark::GetUidBillingMask();
    std::string action = "";
    if (add) {
        action = " -A ";
    } else {
        action = " -D ";
    }
    std::stringstream ss;
    ss << action << LOCAL_MANGLE_INPUT << " -i " << interfaceName << " -j MARK --set-mark 0x" << std::nouppercase
       << std::hex << fwmark.intValue << "/0x" << std::nouppercase << std::hex << mask;
    // need to call IptablesWrapper's RunCommand function.

    return 0;
}

int32_t RouteManager::UpdateExplicitNetworkRule(uint16_t netId, uint32_t table, NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("UpdateExplicitNetworkRule");
    Fwmark fwmark;
    fwmark.netId = netId;
    fwmark.explicitlySelected = true;
    fwmark.permission = permission;

    Fwmark mask;
    mask.netId = FWMARK_NET_ID_MASK;
    mask.explicitlySelected = true;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_EXPLICIT_NETWORK;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = RULEOIF_NULL;

    if (NetManagerStandard::IsInternalNetId(netId)) {
        return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo, UID_ALLOW_INTERNAL.first,
                              UID_ALLOW_INTERNAL.second);
    }
    return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::UpdateOutputInterfaceRules(const std::string &interfaceName, uint32_t table,
                                                 NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("UpdateOutputInterfaceRules");
    Fwmark fwmark;
    fwmark.permission = permission;

    Fwmark mask;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_OUTPUT_INTERFACE;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = RULEIIF_LOOPBACK;
    ruleInfo.ruleOif = interfaceName;

    return UpdateRuleInfo(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::UpdateSharingNetwork(uint16_t action, const std::string &inputInterface,
                                           const std::string &outputInterface)
{
    NETNATIVE_LOGI("UpdateSharingNetwork");
    uint32_t table = FindTableByInterfacename(outputInterface);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_SHARING;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    ruleInfo.ruleIif = inputInterface;
    ruleInfo.ruleOif = RULEOIF_NULL;

    return UpdateRuleInfo(action, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::ClearSharingRules(const std::string &inputInterface)
{
    NETNATIVE_LOGI("ClearSharingRules");

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = 0;
    ruleInfo.rulePriority = RULE_LEVEL_SHARING;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    ruleInfo.ruleIif = inputInterface;
    ruleInfo.ruleOif = RULEOIF_NULL;

    return UpdateRuleInfo(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::UpdateRuleInfo(uint32_t action, uint8_t ruleType, RuleInfo ruleInfo, uid_t uidStart, uid_t uidEnd)
{
    NETNATIVE_LOG_D("UpdateRuleInfo");
    if (ruleInfo.rulePriority < 0) {
        NETNATIVE_LOGE("invalid IP-rule priority %{public}d", ruleInfo.rulePriority);
        return ROUTEMANAGER_ERROR;
    }

    if (ruleInfo.ruleFwmark & ~ruleInfo.ruleMask) {
        NETNATIVE_LOGE("mask 0x%{public}x does not select all the bits set in fwmark 0x%{public}x", ruleInfo.ruleMask,
                       ruleInfo.ruleFwmark);
        return ROUTEMANAGER_ERROR;
    }

    if (ruleInfo.ruleTable == RT_TABLE_UNSPEC && ruleType == FR_ACT_TO_TBL && action != RTM_DELRULE) {
        NETNATIVE_LOGE("RT_TABLE_UNSPEC only allowed when deleting rules");
        return -ENOTUNIQ;
    }

    // The main work is to assemble the structure required for rule.
    for (const uint8_t family : {AF_INET, AF_INET6}) {
        if (SendRuleToKernel(action, family, ruleType, ruleInfo, uidStart, uidEnd) < 0) {
            NETNATIVE_LOGE("Update %{public}s rule info failed, action = %{public}d",
                           (family == AF_INET) ? "IPv4" : "IPv6", action);
            return NETMANAGER_ERR_INTERNAL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t RouteManager::UpdateDistributedRule(uint32_t action, uint8_t ruleType, RuleInfo ruleInfo, uid_t uidStart,
                                            uid_t uidEnd)
{
    NETNATIVE_LOGI("UpdateDistributedRule");
    if (ruleInfo.rulePriority < 0) {
        NETNATIVE_LOGE("invalid IP-rule priority %{public}d", ruleInfo.rulePriority);
        return ROUTEMANAGER_ERROR;
    }

    if (ruleInfo.ruleTable == RT_TABLE_UNSPEC && ruleType == FR_ACT_TO_TBL && action != RTM_DELRULE) {
        NETNATIVE_LOGE("RT_TABLE_UNSPEC only allowed when deleting rules");
        return -ENOTUNIQ;
    }

    int32_t family;
    if (!ruleInfo.ruleDstIp.empty() && strchr(ruleInfo.ruleDstIp.c_str(), ':')) {
        family = AF_INET6;
    } else {
        family = AF_INET;
    }

    if (SendRuleToKernelEx(action, family, ruleType, ruleInfo, uidStart, uidEnd) < 0) {
        NETNATIVE_LOGE("Update %{public}s rule info failed, action = %{public}d",
                       (family == AF_INET) ? "IPv4" : "IPv6", action);
        return NETMANAGER_ERR_INTERNAL;
    }

    return NETMANAGER_SUCCESS;
}

uint16_t RouteManager::GetRuleFlag(uint32_t action)
{
#ifdef SUPPORT_SYSVPN
    return (action == RTM_NEWRULE) ? NLM_F_CREATE : 0;
#else
    return (action == RTM_NEWRULE) ? NLM_F_CREATE : NLM_F_EXCL;
#endif // SUPPORT_SYSVPN
}

int32_t RouteManager::SendRuleToKernel(uint32_t action, uint8_t family, uint8_t ruleType, RuleInfo ruleInfo,
                                       uid_t uidStart, uid_t uidEnd)
{
    struct fib_rule_hdr msg = {0};
    msg.action = ruleType;
    msg.family = family;
    uint16_t ruleFlag = GetRuleFlag(action);
    NetlinkMsg nlmsg(ruleFlag, NETLINK_MAX_LEN, getpid());
    nlmsg.AddRule(action, msg);
    if (int32_t ret = nlmsg.AddAttr32(FRA_PRIORITY, ruleInfo.rulePriority)) {
        return ret;
    }
    if (ruleInfo.ruleTable != RT_TABLE_UNSPEC) {
        if (int32_t ret = nlmsg.AddAttr32(FRA_TABLE, ruleInfo.ruleTable)) {
            return ret;
        }
    }
    if (ruleInfo.ruleMask != 0) {
        if (int32_t ret = nlmsg.AddAttr32(FRA_FWMARK, ruleInfo.ruleFwmark)) {
            return ret;
        }
        if (int32_t ret = nlmsg.AddAttr32(FRA_FWMASK, ruleInfo.ruleMask)) {
            return ret;
        }
    }
    if ((uidStart != INVALID_UID) && (uidEnd != INVALID_UID)) {
        FibRuleUidRange uidRange = {uidStart, uidEnd};
        if (int32_t ret = nlmsg.AddAttr(FRA_UID_RANGE, &uidRange, sizeof(uidRange))) {
            NETNATIVE_LOGE("SendRuleToKernel FRA_UID_RANGE is error.");
            return ret;
        }
    }
    if (ruleInfo.ruleIif != RULEIIF_NULL) {
        char ruleIifName[IFNAMSIZ] = {0};
        size_t ruleIifLength = strlcpy(ruleIifName, ruleInfo.ruleIif.c_str(), IFNAMSIZ) + 1;
        if (int32_t ret = nlmsg.AddAttr(FRA_IIFNAME, ruleIifName, ruleIifLength)) {
            return ret;
        }
    }
    if (ruleInfo.ruleOif != RULEOIF_NULL) {
        char ruleOifName[IFNAMSIZ] = {0};
        size_t ruleOifLength = strlcpy(ruleOifName, ruleInfo.ruleOif.c_str(), IFNAMSIZ) + 1;
        if (int32_t ret = nlmsg.AddAttr(FRA_OIFNAME, ruleOifName, ruleOifLength)) {
            return ret;
        }
    }

    return SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
}

int32_t RouteManager::SendRuleToKernelEx(uint32_t action, uint8_t family, uint8_t ruleType, RuleInfo ruleInfo,
                                         uid_t uidStart, uid_t uidEnd)
{
    struct fib_rule_hdr msg = {0};
    msg.action = ruleType;
    msg.family = family;
    if (ruleInfo.ruleDstIp != RULEIP_NULL && family == AF_INET) {
        msg.dst_len = BIT_32_LEN;
    }
    uint16_t ruleFlag = GetRuleFlag(action);
    NetlinkMsg nlmsg(ruleFlag, NETLINK_MAX_LEN, getpid());
    nlmsg.AddRule(action, msg);
    if (int32_t ret = nlmsg.AddAttr32(FRA_PRIORITY, ruleInfo.rulePriority)) {
        return ret;
    }
    if (ruleInfo.ruleTable != RT_TABLE_UNSPEC) {
        if (int32_t ret = nlmsg.AddAttr32(FRA_TABLE, ruleInfo.ruleTable)) {
            return ret;
        }
    }
    if (ruleInfo.ruleMask != 0) {
        if (int32_t ret = nlmsg.AddAttr32(FRA_FWMARK, ruleInfo.ruleFwmark)) {
            return ret;
        }
        if (int32_t ret = nlmsg.AddAttr32(FRA_FWMASK, ruleInfo.ruleMask)) {
            return ret;
        }
    }
    if (ruleInfo.ruleIif != RULEIIF_NULL) {
        char ruleIifName[IFNAMSIZ] = {0};
        size_t ruleIifLength = strlcpy(ruleIifName, ruleInfo.ruleIif.c_str(), IFNAMSIZ) + 1;
        if (int32_t ret = nlmsg.AddAttr(FRA_IIFNAME, ruleIifName, ruleIifLength)) {
            return ret;
        }
    }
    if (ruleInfo.ruleDstIp != RULEIP_NULL) {
        InetAddr dst = {0};
        if (ReadAddrGw(ruleInfo.ruleDstIp, &dst) <= 0) {
            NETNATIVE_LOGE("dest addr parse failed.");
            return NETMANAGER_ERR_OPERATION_FAILED;
        }
        if (int32_t ret = nlmsg.AddAttr(FRA_DST, dst.data, dst.bitlen / BYTE_ALIGNMENT)) {
            return ret;
        }
    }
    return SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
}

int32_t RouteManager::UpdateRouteRule(uint16_t action, uint16_t flags, RouteInfo routeInfo)
{
    NETNATIVE_LOG_D("UpdateRouteRule");
    RouteInfo routeInfoModify = routeInfo;
    // The main work is to assemble the structure required for route.
    struct rtmsg msg;
    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    msg.rtm_family = AF_INET;
    msg.rtm_dst_len = BIT_32_LEN;
    msg.rtm_protocol = RTPROT_STATIC;
    msg.rtm_scope = RT_SCOPE_UNIVERSE;
    msg.rtm_type = RTN_UNICAST;
    msg.rtm_table = RT_TABLE_UNSPEC;

    uint32_t index = 0;
    if (!routeInfo.routeNextHop.empty() && !strcmp(routeInfo.routeNextHop.c_str(), "unreachable")) {
        msg.rtm_type = RTN_UNREACHABLE;
        routeInfoModify.routeInterfaceName = "";
        routeInfoModify.routeNextHop = "";
    } else if ((!routeInfo.routeNextHop.empty() && !strcmp(routeInfo.routeNextHop.c_str(), "throw")) ||
        routeInfo.isExcludedRoute == true) {
        msg.rtm_type = RTN_THROW;
        routeInfoModify.routeInterfaceName = "";
        routeInfoModify.routeNextHop = "";
    } else {
        index = if_nametoindex(routeInfo.routeInterfaceName.c_str());
    }

    int32_t ret = SendRouteToKernel(action, flags, msg, routeInfoModify, index);
    if (ret < 0) {
        NETNATIVE_LOGE("SendNetlinkMsgToKernel Error ret = %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::SendRouteToKernel(uint16_t action, uint16_t routeFlag, rtmsg msg, RouteInfo routeInfo,
                                        uint32_t index)
{
    InetAddr dst;
    int32_t readAddrResult = ReadAddr(routeInfo.routeDestinationName, &dst);
    if (readAddrResult != 1) {
        NETNATIVE_LOGE("dest parse failed:%{public}d", readAddrResult);
        return -1;
    }
    msg.rtm_family = static_cast<uint8_t>(dst.family);
    msg.rtm_dst_len = static_cast<uint8_t>(dst.prefixlen);
    if (dst.family == AF_INET) {
        msg.rtm_scope = RT_SCOPE_LINK;
    } else if (dst.family == AF_INET6) {
        msg.rtm_scope = RT_SCOPE_UNIVERSE;
    }

    InetAddr gw = {0};
    if (!routeInfo.routeNextHop.empty() && ReadAddrGw(routeInfo.routeNextHop, &gw) <= 0) {
        NETNATIVE_LOGE("gw parse failed:%{public}d", readAddrResult);
        return -1;
    }
    if (gw.bitlen != 0) {
        msg.rtm_scope = RT_SCOPE_UNIVERSE;
        msg.rtm_family = static_cast<uint8_t>(gw.family);
    }
    NetlinkMsg nlmsg(routeFlag, NETLINK_MAX_LEN, getpid());
    nlmsg.AddRoute(action, msg);
    if (int32_t ret = nlmsg.AddAttr32(RTA_TABLE, routeInfo.routeTable)) {
        return ret;
    }
    if (int32_t ret = nlmsg.AddAttr(RTA_DST, dst.data, dst.bitlen / BYTE_ALIGNMENT)) {
        return ret;
    }
    if (!routeInfo.routeNextHop.empty()) {
        if (int32_t ret = nlmsg.AddAttr(RTA_GATEWAY, gw.data, gw.bitlen / BYTE_ALIGNMENT)) {
            return ret;
        }
    }
    if (!routeInfo.routeInterfaceName.empty()) {
        NETNATIVE_LOGI("index is :%{public}d", index);
        if (int32_t ret = nlmsg.AddAttr32(RTA_OIF, index)) {
            return ret;
        }
    }

    return SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
}

uint32_t RouteManager::FindTableByInterfacename(const std::string &interfaceName, int32_t netId)
{
    NETNATIVE_LOG_D("FindTableByInterfacename netId %{public}d", netId);
    auto iter = interfaceToTable_.find(interfaceName);
    if (iter != interfaceToTable_.end()) {
        return ConvertTableByNetId(netId, iter->second);
    }

    uint32_t table = if_nametoindex(interfaceName.c_str());
    if (table == 0) {
        NETNATIVE_LOGE("RouteManager cannot find interface %{public}s", interfaceName.c_str());
        return RT_TABLE_UNSPEC;
    }
    table += ROUTE_TABLE_OFFSET_FROM_INDEX;
    std::lock_guard lock(RouteManager::interfaceToTableLock_);
    interfaceToTable_[interfaceName] = table;
    return ConvertTableByNetId(netId, table);
}

uint32_t RouteManager::GetRouteTableFromType(TableType tableType, const std::string &interfaceName)
{
    switch (tableType) {
        case RouteManager::INTERFACE:
            return FindTableByInterfacename(interfaceName);
        case RouteManager::LOCAL_NETWORK:
            return ROUTE_LOCAL_NETWORK_TABLE;
        case RouteManager::VPN_NETWORK:
#ifdef SUPPORT_SYSVPN
            if (CheckMultiVpnCall(interfaceName)) {
                return FindVpnIdByInterfacename(VpnRuleIdType::VPN_NETWORK_TABLE, interfaceName);
            } else {
                return ROUTE_VPN_NETWORK_TABLE;
            }
#else
            return ROUTE_VPN_NETWORK_TABLE;
#endif // SUPPORT_SYSVPN
        case RouteManager::INTERNAL_DEFAULT:
            return FindTableByInterfacename(interfaceName) % ROUTE_INTERNAL_DEFAULT_TABLE + 1;
        default:
            NETNATIVE_LOGE("tableType [%{tableType}d] is error", tableType);
            return RT_TABLE_UNSPEC;
    }
}

int32_t RouteManager::SetRouteInfo(TableType tableType, NetworkRouteInfo networkRouteInfo, RouteInfo &routeInfo)
{
    uint32_t table = GetRouteTableFromType(tableType, networkRouteInfo.ifName);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }

    routeInfo.routeTable = table;
    routeInfo.routeInterfaceName = networkRouteInfo.ifName;
    routeInfo.routeDestinationName = networkRouteInfo.destination;
    routeInfo.routeNextHop = networkRouteInfo.nextHop;
    routeInfo.isExcludedRoute = networkRouteInfo.isExcludedRoute;
    return 0;
}

#ifdef FEATURE_ENTERPRISE_ROUTE_CUSTOM
/* route target uid to enterprise inner net */
int32_t RouteManager::UpdateEnterpriseRoute(const std::string &interfaceName, uint32_t uid, bool add)
{
    NETNATIVE_LOGI("UpdateEnterpriseRoute,interfaceName:%{public}s, uid:%{public}u, add:%{public}d ",
        interfaceName.c_str(), uid, add);
    uint32_t table = FindTableByInterfacename(interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        NETNATIVE_LOGE("find table by name fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
 
    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_ENTERPRISE;
    uint16_t action = add ? RTM_NEWRULE : RTM_DELRULE;
 
    if (uid != 0) {
        return UpdateRuleInfo(action, FR_ACT_TO_TBL, ruleInfo, uid, uid);
    }
 
    return UpdateRuleInfo(action, FR_ACT_TO_TBL, ruleInfo);
}
#endif
} // namespace nmd
} // namespace OHOS
