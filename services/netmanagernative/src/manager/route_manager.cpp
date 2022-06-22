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
#include "bitcast.h"
#include "fwmark.h"
#include "netlink_manager.h"
#include "netlink_msg.h"
#include "netnative_log_wrapper.h"
#include "securec.h"
#include "route_manager.h"

namespace OHOS {
namespace nmd {
namespace {
constexpr uint32_t ROUTE_VPN_NETWORK_TABLE = 98;
constexpr uint32_t ROUTE_LOCAL_NETWORK_TABLE = 99;
constexpr uint32_t OUTPUT_MAX = 128;
constexpr uint32_t BIT_32_LEN = 32;
constexpr uint32_t BIT_MAX_LEN = 255;
constexpr uint32_t DECIMAL_DIGITAL = 10;
constexpr uint32_t BYTE_ALIGNMENT = 8;
constexpr uint32_t THOUSAND_LEN = 100;
constexpr uint16_t LOCAL_NET_ID = 99;
constexpr uint16_t NETID_UNSET = 0;
constexpr uint32_t MARK_UNSET = 0;
constexpr bool ACTION_ADD = true;
constexpr bool ACTION_DEL = false;
const std::string IIF_LOOPBACK = "lo";
const std::string IIF_NONE = "";
const std::string OIF_NONE = "";
const std::string LOCAL_MANGLE_INPUT = "routectrl_mangle_INPUT";
} // namespace
std::mutex RouteManager::m_interfaceToTableLock_;
std::map<std::string, uint32_t> RouteManager::m_interfaceToTable_;

RouteManager::RouteManager()
{
    Init();
}

RouteManager::~RouteManager() {}

int32_t RouteManager::AddRoute(TableType tableType, const std::string &interfaceName,
    const std::string &destinationName, const std::string &nextHop)
{
    NETNATIVE_LOGI("Entry RouteManager::AddRoute,interfaceName:%{public}s,destination:%{public}s, nextHop:%{public}s",
        interfaceName.c_str(), destinationName.c_str(), nextHop.c_str());
    uint32_t table = GetRouteTableFromType(tableType, interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }

    // This is a user-defined structure used to integrate the information required for setting up routes.
    RouteInfo routeInfo;
    routeInfo.routeTable = table;
    routeInfo.routeInterfaceName = interfaceName;
    routeInfo.routeDestinationName = destinationName;
    routeInfo.routeNextHop = nextHop;
    return ModifyRoute(RTM_NEWROUTE, NETLINK_ROUTE_CREATE_FLAGS, routeInfo);
}

int32_t RouteManager::RemoveRoute(TableType tableType, const std::string &interfaceName,
    const std::string &destinationName, const std::string &nextHop)
{
    NETNATIVE_LOGI("Entry RouteManager::RemoveRoute,interfaceName:%{public}s,destination:%{public}s,nextHop:%{public}s",
        interfaceName.c_str(), destinationName.c_str(), nextHop.c_str());
    uint32_t table = GetRouteTableFromType(tableType, interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }

    RouteInfo routeInfo;
    routeInfo.routeTable = table;
    routeInfo.routeInterfaceName = interfaceName;
    routeInfo.routeDestinationName = destinationName;
    routeInfo.routeNextHop = nextHop;
    return ModifyRoute(RTM_DELROUTE, NETLINK_REQUEST_FLAGS, routeInfo);
}

int32_t RouteManager::UpdateRoute(TableType tableType, const std::string &interfaceName,
    const std::string &destinationName, const std::string &nextHop)
{
    NETNATIVE_LOGI("Entry RouteManager::UpdateRoute,interfaceName:%{public}s,destination:%{public}s,nextHop:%{public}s",
        interfaceName.c_str(), destinationName.c_str(), nextHop.c_str());
    uint32_t table = GetRouteTableFromType(tableType, interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }

    RouteInfo routeInfo;
    routeInfo.routeTable = table;
    routeInfo.routeInterfaceName = interfaceName;
    routeInfo.routeDestinationName = destinationName;
    routeInfo.routeNextHop = nextHop;
    return ModifyRoute(RTM_NEWROUTE, NETLINK_ROUTE_REPLACE_FLAGS, routeInfo);
}

int32_t RouteManager::AddInterfaceToDefaultNetwork(const std::string &interfaceName, NetworkPermission permission)
{
    NETNATIVE_LOGI("AddInterfaceToDefaultNetwork, %{public}s;permission:%{public}d;", interfaceName.c_str(),
        permission);
    uint32_t table = GetRouteTableForInterface(interfaceName);
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
    ruleInfo.ruleIif = IIF_LOOPBACK;
    ruleInfo.ruleOif = OIF_NONE;
    return ModifyRule(RTM_NEWRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::RemoveInterfaceFromDefaultNetwork(const std::string &interfaceName, NetworkPermission permission)
{
    NETNATIVE_LOGI("RemoveInterfaceFromDefaultNetwork, %{public}s;permission:%{public}d;", interfaceName.c_str(),
        permission);
    uint32_t table = GetRouteTableForInterface(interfaceName);
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
    ruleInfo.ruleIif = IIF_LOOPBACK;
    ruleInfo.ruleOif = OIF_NONE;
    return ModifyRule(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::AddInterfaceToPhysicalNetwork(uint16_t netId, const std::string &interfaceName,
    NetworkPermission permission)
{
    NETNATIVE_LOGI("AddInterfaceToPhysicalNetwork, netId:%{public}d;interfaceName:%{public}s;permission:%{public}d;",
        netId, interfaceName.c_str(), permission);
    return ModifyPhysicalNetwork(netId, interfaceName, permission, ACTION_ADD);
}

int32_t RouteManager::RemoveInterfaceFromPhysicalNetwork(uint16_t netId, const std::string &interfaceName,
    NetworkPermission permission)
{
    NETNATIVE_LOGI("RemoveInterfacePhysicalNetwork, netId:%{public}d;interfaceName:%{public}s;permission:%{public}d;",
        netId, interfaceName.c_str(), permission);
    if (int32_t ret = ModifyPhysicalNetwork(netId, interfaceName, permission, ACTION_DEL)) {
        NETNATIVE_LOGE("ModifyPhysicalNetwork err, error is %{public}d", ret);
        return ret;
    }
    if (int32_t ret = FlushRoutes(interfaceName)) {
        NETNATIVE_LOGE("FlushRoutes err, error is %{public}d", ret);
        return ret;
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
    NETNATIVE_LOGI("Entry RouteManager::ModifyPhysicalNetworkPermission, %{public}s", interfaceName.c_str());
    if (int32_t ret = ModifyPhysicalNetwork(netId, interfaceName, newPermission, ACTION_ADD)) {
        NETNATIVE_LOGE("ModifyPhysicalNetwork err, error is %{public}d", ret);
        return ret;
    }

    return ModifyPhysicalNetwork(netId, interfaceName, newPermission, ACTION_DEL);
}

int32_t RouteManager::AddInterfaceToLocalNetwork(uint16_t netId, const std::string &interfaceName)
{
    NETNATIVE_LOGI("Entry RouteManager::AddInterfaceToLocalNetwork, %{public}s", interfaceName.c_str());
    if (int32_t ret = ModifyLocalNetwork(netId, interfaceName, ACTION_ADD)) {
        NETNATIVE_LOGE("ModifyLocalNetwork err, error is %{public}d", ret);
        return ret;
    }
    std::lock_guard lock(m_interfaceToTableLock_);
    m_interfaceToTable_[interfaceName] = ROUTE_LOCAL_NETWORK_TABLE;

    return 0;
}

int32_t RouteManager::RemoveInterfaceFromLocalNetwork(uint16_t netId, const std::string &interfaceName)
{
    NETNATIVE_LOGI("Entry RouteManager::RemoveInterfaceFromLocalNetwork");
    if (int32_t ret = ModifyLocalNetwork(netId, interfaceName, ACTION_DEL)) {
        NETNATIVE_LOGE("ModifyLocalNetwork err, error is %{public}d", ret);
        return ret;
    }
    std::lock_guard lock(m_interfaceToTableLock_);
    m_interfaceToTable_.erase(interfaceName);

    return 0;
}

int32_t RouteManager::EnableSharing(const std::string &inputInterface, const std::string &outputInterface)
{
    return ModifySharingNetwork(RTM_NEWRULE, inputInterface, outputInterface);
}

int32_t RouteManager::DisableSharing(const std::string &inputInterface, const std::string &outputInterface)
{
    return ModifySharingNetwork(RTM_DELRULE, inputInterface, outputInterface);
}

int32_t RouteManager::ReadAddrGw(const std::string &addr, InetAddr *res)
{
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

int32_t RouteManager::Init()
{
    NETNATIVE_LOGI("Entry RouteManager::Init");
    // need to call IptablesWrapper's RunCommand function.
    std::string commandNew;
    commandNew.append(" -t mangle -N ");
    commandNew.append(LOCAL_MANGLE_INPUT);

    std::string commandJump;
    commandJump.append(" -A INPUT -j ");
    commandJump.append(LOCAL_MANGLE_INPUT);

    if (int32_t ret = FlushRules()) {
        NETNATIVE_LOGE("FlushRules failed, err is %{public}d", ret);
        return ret;
    }

    if (int32_t ret = AddLocalNetworkRules()) {
        NETNATIVE_LOGE("AddLocalNetworkRules failed, err is %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::FlushRules()
{
    return RtNetlinkFlush(RTM_GETRULE, RTM_DELRULE, "rules", 0);
}

int32_t RouteManager::FlushRoutes(const std::string &interfaceName)
{
    std::lock_guard lock(RouteManager::m_interfaceToTableLock_);
    uint32_t table = GetRouteTableForInterface(interfaceName);
    NETNATIVE_LOGI("RouteManager::FlushRoutes--table==:%{public}d", table);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }
    int32_t ret = RtNetlinkFlush(RTM_GETROUTE, RTM_DELROUTE, "routes", table);
    if (ret == 0) {
        m_interfaceToTable_.erase(interfaceName);
    }

    return 0;
}

int32_t RouteManager::AddLocalNetworkRules()
{
    NETNATIVE_LOGI("Entry RouteManager::AddLocalNetworkRules");
    if (int32_t ret = ModifyExplicitNetworkRule(LOCAL_NET_ID, ROUTE_LOCAL_NETWORK_TABLE, PERMISSION_NONE, ACTION_ADD)) {
        NETNATIVE_LOGE("ModifyExplicitNetworkRule failed, err is %{public}d", ret);
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
    ruleInfo.ruleIif = IIF_NONE;
    ruleInfo.ruleOif = OIF_NONE;

    return ModifyRule(RTM_NEWRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::ModifyPhysicalNetwork(uint16_t netId, const std::string &interfaceName,
    NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifyPhysicalNetwork,add===%{public}d", add);
    uint32_t table = GetRouteTableForInterface(interfaceName);
    if (table == RT_TABLE_UNSPEC) {
        NETNATIVE_LOGE("table == RT_TABLE_UNSPEC, this is error");
        return -1;
    }

    if (int32_t ret = ModifyIncomingPacketMark(netId, interfaceName, permission, add)) {
        NETNATIVE_LOGE("ModifyIncomingPacketMark failed, err is %{public}d", ret);
        return ret;
    }

    if (int32_t ret = ModifyExplicitNetworkRule(netId, table, permission, add)) {
        NETNATIVE_LOGE("ModifyExplicitNetworkRule failed, err is %{public}d", ret);
        return ret;
    }

    if (int32_t ret = ModifyOutputInterfaceRules(interfaceName, table, permission, add)) {
        NETNATIVE_LOGE("ModifyOutputInterfaceRules failed, err is %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::ModifyLocalNetwork(uint16_t netId, const std::string &interfaceName, bool add)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifyLocalNetwork");
    if (int32_t ret = ModifyIncomingPacketMark(netId, interfaceName, PERMISSION_NONE, add)) {
        NETNATIVE_LOGE("ModifyIncomingPacketMark err");
        return ret;
    }

    return ModifyOutputInterfaceRules(interfaceName, ROUTE_LOCAL_NETWORK_TABLE, PERMISSION_NONE, add);
}

int32_t RouteManager::ModifyIncomingPacketMark(uint16_t netId, const std::string &interfaceName,
    NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifyIncomingPacketMark");
    Fwmark fwmark;
    fwmark.netId = netId;
    fwmark.explicitlySelected = true;
    fwmark.protectedFromVpn = true;
    fwmark.permission = permission;
    const uint32_t mask = ~Fwmark::GetUidBillingMask();
    std::string command = "";
    std::string action = "";
    if (add) {
        action = " -A ";
    } else {
        action = " -D ";
    }
    std::stringstream ss;
    ss << action << LOCAL_MANGLE_INPUT << " -i " << interfaceName << " -j MARK --set-mark 0x" << std::nouppercase
       << std::hex << fwmark.intValue << "/0x" << std::nouppercase << std::hex << mask;
    command = ss.str();
    // need to call IptablesWrapper's RunCommand function.

    return 0;
}

int32_t RouteManager::ModifyExplicitNetworkRule(uint16_t netId, uint32_t table, NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifyExplicitNetworkRule");
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
    ruleInfo.ruleIif = IIF_LOOPBACK;
    ruleInfo.ruleOif = OIF_NONE;

    return ModifyRule(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::ModifyOutputInterfaceRules(const std::string &interfaceName, uint32_t table,
    NetworkPermission permission, bool add)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifyOutputInterfaceRules");
    Fwmark fwmark;
    fwmark.permission = permission;

    Fwmark mask;
    mask.permission = permission;

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_OUTPUT_INTERFACE;
    ruleInfo.ruleFwmark = fwmark.intValue;
    ruleInfo.ruleMask = mask.intValue;
    ruleInfo.ruleIif = IIF_LOOPBACK;
    ruleInfo.ruleOif = interfaceName;

    return ModifyRule(add ? RTM_NEWRULE : RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::ModifySharingNetwork(uint16_t action, const std::string &inputInterface,
    const std::string &outputInterface)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifySharingNetwork");
    uint32_t table = GetRouteTableForInterface(outputInterface);
    if (table == RT_TABLE_UNSPEC) {
        return -1;
    }

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = table;
    ruleInfo.rulePriority = RULE_LEVEL_SHARING;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    ruleInfo.ruleIif = inputInterface;
    ruleInfo.ruleOif = OIF_NONE;

    return ModifyRule(action, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::ClearSharingRules(const std::string &inputInterface)
{
    NETNATIVE_LOGI("Entry RouteManager::ClearSharingRules");

    RuleInfo ruleInfo;
    ruleInfo.ruleTable = 0;
    ruleInfo.rulePriority = RULE_LEVEL_SHARING;
    ruleInfo.ruleFwmark = MARK_UNSET;
    ruleInfo.ruleMask = MARK_UNSET;
    ruleInfo.ruleIif = inputInterface;
    ruleInfo.ruleOif = OIF_NONE;

    return ModifyRule(RTM_DELRULE, FR_ACT_TO_TBL, ruleInfo);
}

int32_t RouteManager::ModifyRule(uint32_t action, uint8_t ruleType, RuleInfo ruleInfo)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifyRule");
    if (ruleInfo.rulePriority < 0) {
        NETNATIVE_LOGI("invalid IP-rule priority %{public}d", ruleInfo.rulePriority);
        return -1;
    }

    if (ruleInfo.ruleFwmark & ~ruleInfo.ruleMask) {
        NETNATIVE_LOGI("mask 0x%{public}x does not select all the bits set in fwmark 0x%{public}x",
            ruleInfo.ruleMask, ruleInfo.ruleFwmark);
        return -1;
    }

    if (ruleInfo.ruleTable == RT_TABLE_UNSPEC && ruleType == FR_ACT_TO_TBL && action != RTM_DELRULE) {
        NETNATIVE_LOGE("RT_TABLE_UNSPEC only allowed when deleting rules");
        return -ENOTUNIQ;
    }

    // The main work is to assemble the structure required for rule.
    uint16_t ruleFlag = NETLINK_REQUEST_FLAGS;
    if (action == RTM_NEWRULE) {
        ruleFlag = NETLINK_RULE_CREATE_FLAGS;
    }

    int32_t ret = SendRuleToKernel(action, ruleFlag, ruleType, ruleInfo);
    if (ret < 0) {
        NETNATIVE_LOGE("SendNetlinkMsgToKernel Error, ret = %{public}d", ret);
        return ret;
    }

    return 0;
}

int32_t RouteManager::SendRuleToKernel(uint32_t action, uint16_t ruleFlag, uint8_t ruleType, RuleInfo ruleInfo)
{
    // Parse the prefix.
    char iifName[IFNAMSIZ], oifName[IFNAMSIZ];
    size_t iifLength, oifLength;
    uint16_t iifPadding, oifPadding;
    if (int32_t ret = PadInterfaceName(ruleInfo.ruleIif, iifName, &iifLength, &iifPadding)) {
        NETNATIVE_LOGE("PadInterfaceName Iif Error, err is %{public}d", ret);
        return ret;
    }
    if (int32_t ret = PadInterfaceName(ruleInfo.ruleOif, oifName, &oifLength, &oifPadding)) {
        NETNATIVE_LOGE("PadInterfaceName Oif Error, err is %{public}d", ret);
        return ret;
    }

    struct fib_rule_hdr msg = {0};
    msg.action = ruleType;
    msg.family = AF_INET;
    NetlinkMsg nlmsg(ruleFlag, NETLINK_MAX_LEN, NetlinkManager::GetPid());
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
    if (ruleInfo.ruleIif != IIF_NONE) {
        if (int32_t ret = nlmsg.AddAttr(FRA_IIFNAME, (void *)iifName, iifLength)) {
            return ret;
        }
    }
    if (ruleInfo.ruleOif != OIF_NONE) {
        if (int32_t ret = nlmsg.AddAttr(FRA_OIFNAME, (void *)oifName, oifLength)) {
            return ret;
        }
    }
    NETNATIVE_LOGI("Rule AddAttr all is success");

    return SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
}

int32_t RouteManager::ModifyRoute(uint16_t action, uint16_t flags, RouteInfo routeInfo)
{
    NETNATIVE_LOGI("Entry RouteManager::ModifyRoute");
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
        routeInfoModify.routeInterfaceName = nullptr;
        routeInfoModify.routeNextHop = nullptr;
    } else if (!routeInfo.routeNextHop.empty() && !strcmp(routeInfo.routeNextHop.c_str(), "throw")) {
        msg.rtm_type = RTN_THROW;
        routeInfoModify.routeInterfaceName = nullptr;
        routeInfoModify.routeNextHop = nullptr;
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
    msg.rtm_family = dst.family;
    msg.rtm_dst_len = dst.prefixlen;
    if (dst.family == AF_INET) {
        msg.rtm_scope = RT_SCOPE_LINK;
    } else if (dst.family == AF_INET6) {
        msg.rtm_scope = RT_SCOPE_UNIVERSE;
    }

    InetAddr gw;
    readAddrResult = ReadAddrGw(routeInfo.routeNextHop, &gw);
    if (readAddrResult != 1) {
        NETNATIVE_LOGE("gw parse failed:%{public}d", readAddrResult);
        return -1;
    }
    if (gw.bitlen != 0) {
        msg.rtm_scope = RT_SCOPE_UNIVERSE;
        msg.rtm_family = gw.family;
    }

    NetlinkMsg nlmsg(routeFlag, NETLINK_MAX_LEN, NetlinkManager::GetPid());
    nlmsg.AddRoute(action, msg);
    if (int32_t ret = nlmsg.AddAttr32(RTA_TABLE, routeInfo.routeTable)) {
        return ret;
    }
    if (int32_t ret = nlmsg.AddAttr(RTA_DST, (void *)dst.data, dst.bitlen / BYTE_ALIGNMENT)) {
        return ret;
    }
    if (!routeInfo.routeNextHop.empty()) {
        if (int32_t ret = nlmsg.AddAttr(RTA_GATEWAY, (void *)gw.data, gw.bitlen / BYTE_ALIGNMENT)) {
            return ret;
        }
    }
    if (!routeInfo.routeInterfaceName.empty()) {
        NETNATIVE_LOGI("index is :%{public}d", index);
        if (int32_t ret = nlmsg.AddAttr32(RTA_OIF, index)) {
            return ret;
        }
    }
    NETNATIVE_LOGI("Route AddAttr all is success");

    return SendNetlinkMsgToKernel(nlmsg.GetNetLinkMessage());
}

int32_t RouteManager::PadInterfaceName(const std::string &input, char *name, size_t *length, uint16_t *padding)
{
    if (input.empty()) {
        *length = 0;
        *padding = 0;
        return 0;
    }
    *length = strlcpy(name, input.c_str(), IFNAMSIZ) + 1;
    if (*length > IFNAMSIZ) {
        NETNATIVE_LOGE("interface name too long,currentSizeIs==%{public}zu", *length);
        return -ENAMETOOLONG;
    }
    *padding = RTA_SPACE(*length) - RTA_LENGTH(*length);
    return 0;
}

uint32_t RouteManager::GetRouteTableForInterface(const std::string &interfaceName)
{
    auto iter = m_interfaceToTable_.find(interfaceName);
    if (iter != m_interfaceToTable_.end()) {
        return iter->second;
    }

    uint32_t table = if_nametoindex(interfaceName.c_str());
    if (table == 0) {
        NETNATIVE_LOGE("RouteManager cannot find interface %{public}s", interfaceName.c_str());
        return RT_TABLE_UNSPEC;
    }
    table += THOUSAND_LEN;
    m_interfaceToTable_[interfaceName] = table;
    return table;
}

uint32_t RouteManager::GetRouteTableFromType(TableType tableType, const std::string &interfaceName)
{
    uint32_t table;
    switch (tableType) {
        case RouteManager::INTERFACE:
            table = GetRouteTableForInterface(interfaceName);
            break;
        case RouteManager::VPN_NETWORK:
            table = ROUTE_VPN_NETWORK_TABLE;
            break;
        case RouteManager::LOCAL_NETWORK:
            table = ROUTE_LOCAL_NETWORK_TABLE;
            break;
    }
    return table;
}
} // namespace nmd
} // namespace OHOS