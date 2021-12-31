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

#ifndef NET_TETHER_NETD_UTILS_H
#define NET_TETHER_NETD_UTILS_H

#include <string>
#include <vector>

#include "net_tether_ip_address.h"
#include "net_tether_define.h"
#include "inet_addr.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTetherNetdUtils {
public:
    static NetTetherNetdUtils* GetInstance();
    ~NetTetherNetdUtils();
    void GetAllInterfaceList(std::vector<std::string> &allInterfaceList);
    bool AddIfaceConfigAndUp(const std::string &ifaceName, const NetTetherIpAddress &ipAddr);
    bool DelIfaceConfigAndDown(const std::string &ifaceName, const NetTetherIpAddress &ipAddr);
    bool NetworkAddInterface(int32_t netId, const std::string &ifaceName);
    bool NetworkRemoveInterface(int32_t netId, const std::string &ifaceName);
    bool IpEnableForwarding(const std::string &requester);
    bool IpDisableForwarding(const std::string &requester);
    bool TetherAddForward(const std::string &downstreamIface, const std::string &upstreamIface);
    bool TetherRemoveForward(const std::string &downstreamIface, const std::string &upstreamIface);
    bool IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface);
    bool IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface);
    bool SetTetherRoute(int32_t netid, const std::string &ifaceName, const std::string &desaddr,
        int32_t desprefixlen);
    bool DelTetherRoute(int32_t netid, const std::string &ifaceName, const std::string &desaddr,
        int32_t desprefixlen);
    bool TetherDnsSet(uint32_t netId, const std::list<INetAddr> &dnsList);
    void RegisterNetdResponseCallback(const NetdResponseCallback &callback);

private:
    NetTetherNetdUtils();
    void InterfaceAdd(const std::string &iface);
    void InterfaceRemove(const std::string &iface);
    void Ipv4ToRoute(const std::string &src, int32_t prefixLen, std::string &dest);

private:
    static NetTetherNetdUtils* instance_;
    NetdResponseCallback callback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_NETD_UTILS_H