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

#include <climits>

#include "netd_controller.h"
#include "net_mgr_log_wrapper.h"
#include "netd_controller_define.h"
#include "net_tether_netd_utils.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherNetdUtils* NetTetherNetdUtils::instance_ = nullptr;

NetTetherNetdUtils* NetTetherNetdUtils::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = new NetTetherNetdUtils();
    }
    return instance_;
}

NetTetherNetdUtils::NetTetherNetdUtils()
{
    NetdNotifyCallback netdNotifyCallback = {
        std::bind(&NetTetherNetdUtils::InterfaceAdd, this, std::placeholders::_1),
        std::bind(&NetTetherNetdUtils::InterfaceRemove, this, std::placeholders::_1),
    };
    NetdController::GetInstance().RegisterNetdNotifyCallback(netdNotifyCallback);
}

NetTetherNetdUtils::~NetTetherNetdUtils()
{
    NetdNotifyCallback netdNotifyCallback;
    NetdController::GetInstance().RegisterNetdNotifyCallback(netdNotifyCallback);
}

void NetTetherNetdUtils::GetAllInterfaceList(std::vector<std::string> &allInterfaceList)
{
    if (!allInterfaceList.empty()) {
        allInterfaceList.clear();
    }
    allInterfaceList = NetdController::GetInstance().InterfaceGetList();
}

bool NetTetherNetdUtils::AddIfaceConfigAndUp(const std::string &ifaceName, const NetTetherIpAddress &ipAddr)
{
    std::string ip = ipAddr.GetAddress();
    int32_t prefixLength = ipAddr.GetPrefixLength();
    if (NetdController::GetInstance().InterfaceAddAddress(ifaceName, ip, prefixLength) != 0) {
        NETMGR_LOG_E("interface add ip address failed. ifaceName: [%{public}s], ip address: [%{public}s]",
            ifaceName.c_str(), ip.c_str());
        return false;
    }
    NetdController::GetInstance().SetInterfaceUp(ifaceName);
    return true;
}

bool NetTetherNetdUtils::DelIfaceConfigAndDown(const std::string &ifaceName, const NetTetherIpAddress &ipAddr)
{
    bool ret = true;
    std::string ip = ipAddr.GetAddress();
    int32_t prefixLength = ipAddr.GetPrefixLength();
    if (NetdController::GetInstance().InterfaceDelAddress(ifaceName, ip, prefixLength) != 0) {
        NETMGR_LOG_E("interface delete ip address failed. ifaceName: [%{public}s], ip address: [%{public}s]",
            ifaceName.c_str(), ip.c_str());
        ret = false;
    }
    NetdController::GetInstance().SetInterfaceDown(ifaceName);
    return ret;
}

bool NetTetherNetdUtils::NetworkAddInterface(int32_t netId, const std::string &ifaceName)
{
    return NetdController::GetInstance().NetworkAddInterface(netId, ifaceName) == 0;
}

bool NetTetherNetdUtils::NetworkRemoveInterface(int32_t netId, const std::string &ifaceName)
{
    return NetdController::GetInstance().NetworkRemoveInterface(netId, ifaceName) == 0;
}

bool NetTetherNetdUtils::IpEnableForwarding(const std::string &requester)
{
    return NetdController::GetInstance().IpEnableForwarding(requester) == 0;
}

bool NetTetherNetdUtils::IpDisableForwarding(const std::string &requester)
{
    return NetdController::GetInstance().IpDisableForwarding(requester) == 0;
}

bool NetTetherNetdUtils::TetherAddForward(const std::string &downstreamIface, const std::string &upstreamIface)
{
    return NetdController::GetInstance().TetherAddForward(downstreamIface, upstreamIface) == 0;
}

bool NetTetherNetdUtils::TetherRemoveForward(const std::string &downstreamIface, const std::string &upstreamIface)
{
    return NetdController::GetInstance().TetherRemoveForward(downstreamIface, upstreamIface) == 0;
}

bool NetTetherNetdUtils::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    return NetdController::GetInstance().IpfwdAddInterfaceForward(fromIface, toIface) == 0;
}

bool NetTetherNetdUtils::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    return NetdController::GetInstance().IpfwdRemoveInterfaceForward(fromIface, toIface) == 0;
}

bool NetTetherNetdUtils::TetherDnsSet(uint32_t netId, const std::list<INetAddr> &dnsList)
{
    std::vector<std::string> dnsAddrs;
    for (auto iter = dnsList.begin(); iter != dnsList.end(); ++iter) {
        dnsAddrs.push_back(iter->address_);
    }
    return NetdController::GetInstance().TetherDnsSet(netId, dnsAddrs) == 0;
}

void NetTetherNetdUtils::RegisterNetdResponseCallback(const NetdResponseCallback &callback)
{
    callback_ = callback;
}

void NetTetherNetdUtils::InterfaceAdd(const std::string &iface)
{
    if (callback_.NetdResponseInterfaceAdd != nullptr) {
        callback_.NetdResponseInterfaceAdd(iface);
    }
}

void NetTetherNetdUtils::InterfaceRemove(const std::string &iface)
{
    if (callback_.NetdResponseInterfaceRemoved != nullptr) {
        callback_.NetdResponseInterfaceRemoved(iface);
    }
}

bool  NetTetherNetdUtils::SetTetherRoute(int32_t netid, const std::string &ifaceName, const std::string &desaddr,
    int32_t desprefixlen)
{
    std::string dest;
    Ipv4ToRoute(desaddr, desprefixlen, dest);
    if (dest.empty()) {
        NETMGR_LOG_E("route dest is empty.");
        return false;
    }
    if (NetdController::GetInstance().NetworkAddRoute(netid, ifaceName, dest, DEFAULT_NEXT_HOP) == 0) {
        return false;
    }
    return true;
}

bool  NetTetherNetdUtils::DelTetherRoute(int32_t netid, const std::string &ifaceName, const std::string &desaddr,
    int32_t desprefixlen)
{
    std::string dest;
    Ipv4ToRoute(desaddr, desprefixlen, dest);
    if (dest.empty()) {
        NETMGR_LOG_E("route dest is empty.");
        return false;
    }
    if (NetdController::GetInstance().NetworkRemoveRoute(netid, ifaceName, dest, DEFAULT_NEXT_HOP) == 0) {
        return false;
    }
    return true;
}

void NetTetherNetdUtils::Ipv4ToRoute(const std::string &src, int32_t prefixLen, std::string &dest)
{
    constexpr int32_t MAXIPPREFIXLEN = 32;
    constexpr int32_t MAXLOOPCOUNT = 4;
    if (src.empty() || prefixLen > MAXIPPREFIXLEN) {
        return;
    }
    std::string ipTemp = src;
    uint64_t ipTonumber = 0;
    for (int32_t i = 0; i < MAXLOOPCOUNT; i++) {
        int32_t index = ipTemp.find('.');
        uint32_t nIpsub = atoi(ipTemp.substr(0, index).c_str());
        ipTonumber += nIpsub << ((MAXLOOPCOUNT - 1 - i) * CHAR_BIT);
        ipTemp = ipTemp.substr(index + 1, ipTemp.length() - index);
    }
    uint64_t prefixlentomask = 1;
    for (int32_t i = 0; i < (prefixLen - 1); i++) {
        prefixlentomask = prefixlentomask << 1;
        prefixlentomask += 1;
    }
    prefixlentomask = prefixlentomask << (MAXIPPREFIXLEN - 1 - prefixLen);
    uint64_t ndestIp = ipTonumber&prefixlentomask;
    if (ndestIp != 0) {
        dest += std::to_string((ndestIp & 0xff000000) >> (CHAR_BIT + CHAR_BIT + CHAR_BIT));
        dest += ".";
        dest += std::to_string((ndestIp & 0xff0000) >> (CHAR_BIT + CHAR_BIT));
        dest += ".";
        dest += std::to_string((ndestIp & 0xff00) >> CHAR_BIT);
        dest += ".";
        dest += std::to_string(ndestIp & 0xff);
    }
    NETMGR_LOG_D("Ipv4ToRoute: src addr[%{public}s], src prefixlen[%{public}d], mask dest addr[%{public}s]",
        src.c_str(), prefixLen, dest.c_str());
    if (!dest.empty()) {
        dest += "/";
        dest += std::to_string(prefixLen);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS