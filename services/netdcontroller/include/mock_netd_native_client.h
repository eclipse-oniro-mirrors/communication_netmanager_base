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

#ifndef MOCK_NETD_NATIVE_CLIENT_H
#define MOCK_NETD_NATIVE_CLIENT_H

#include <string>
#include <vector>
#include <unordered_set>
#include <cstring>
#include <netdb.h>
#include <linux/if.h>

#include "netd_controller_callback.h"
#include "netd_controller_define.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string MOCK_NETWORKCREATEPHYSICAL_API = "NetworkCreatePhysical";
const std::string MOCK_NETWORKDESTROY_API = "NetworkDestroy";
const std::string MOCK_NETWORKADDINTERFACE_API = "NetworkAddInterface";
const std::string MOCK_NETWORKREMOVEINTERFACE_API = "NetworkRemoveInterface";
const std::string MOCK_NETWORKADDROUTE_API = "NetworkAddRoute";
const std::string MOCK_NETWORKREMOVEROUTE_API = "NetworkRemoveRoute";
const std::string MOCK_SETINTERFACEDOWN_API = "SetInterfaceDown";
const std::string MOCK_SETINTERFACEUP_API = "SetInterfaceUp";
const std::string MOCK_INTERFACECLEARADDRS_API = "InterfaceClearAddrs";
const std::string MOCK_INTERFACEGETMTU_API = "InterfaceGetMtu";
const std::string MOCK_INTERFACESETMTU_API = "InterfaceSetMtu";
const std::string MOCK_INTERFACEADDADDRESS_API = "InterfaceAddAddress";
const std::string MOCK_INTERFACEDELADDRESS_API = "InterfaceDelAddress";
const std::string MOCK_SETRESOLVERCONFIG_API = "SetResolverConfig";
const std::string MOCK_GETRESOLVERINFO_API = "GetResolverInfo";
const std::string MOCK_CREATENETWORKCACHE_API = "CreateNetworkCache";
const std::string MOCK_DESTORYNETWORKCACHE_API = "DestoryNetworkCache";
const std::string MOCK_FLUSHNETWORKCACHE_API = "FlushNetworkCache";
const std::string MOCK_GETADDRINFO_API = "GetAddrInfo";
const std::string MOCK_GETCELLULARRXBYTES_API = "GetCellularRxBytes";
const std::string MOCK_GETCELLULARTXBYTES_API = "GetCellularTxBytes";
const std::string MOCK_GETALLRXBYTES_API = "GetAllRxBytes";
const std::string MOCK_GETALLTXBYTES_API = "GetAllTxBytes";
const std::string MOCK_GETUIDRXBYTES_API = "GetUidRxBytes";
const std::string MOCK_GETUIDTXBYTES_API = "GetUidTxBytes";
const std::string MOCK_GETUIDONIFACERXBYTES_API = "GetUidOnIfaceRxBytes";
const std::string MOCK_GETUIDONIFACETXBYTES_API = "GetUidOnIfaceTxBytes";
const std::string MOCK_GETIFACERXBYTES_API = "GetIfaceRxBytes";
const std::string MOCK_GETIFACETXBYTES_API = "GetIfaceTxBytes";
const std::string MOCK_INTERFACEGETLIST_API = "InterfaceGetList";
const std::string MOCK_UIDGETLIST_API = "UidGetList";
const std::string MOCK_GETIFACERXPACKETS_API = "GetIfaceRxPackets";
const std::string MOCK_GETIFACETXPACKETS_API = "GetIfaceTxPackets";
const std::string MOCK_SETDEFAULTNETWORK_API = "SetDefaultNetWork";
const std::string MOCK_CLEARDEFAULTNETWORK_API = "ClearDefaultNetWorkNetId";
const std::string MOCK_BINDSOCKET_API = "BindSocket";
const std::string MOCK_IPENABLEFORWARDING_API = "IpEnableForwarding";
const std::string MOCK_IPDISABLEFORWARDING_API = "IpDisableForwarding";
const std::string MOCK_TETHERADDFORWARD_API = "TetherAddForward";
const std::string MOCK_TETHERREMOVEFORWARD_API = "TetherRemoveForward";
const std::string MOCK_IPFWDADDIFACEFORWARD_API = "IpfwdAddInterfaceForward";
const std::string MOCK_IPFWDREMOVEIFACEFORWARD_API = "IpfwdRemoveInterfaceForward";
const std::string MOCK_TETHERDNSSET_API = "TetherDnsSet";
const std::string MOCK_REGISTERNETDNOTIFYCALLBACK_API = "RegisterNetdNotifyCallback";
const std::string MOCK_BINDNETWORKSERVICEVPN_API = "BindNetworkServiceVpn";
const std::string MOCK_ENABLEVIRTUALNETIFACECARD_API = "EnableVirtualNetIfaceCard";
const std::string MOCK_SETIPADDRESS_API = "SetIpAddress";
const std::string MOCK_SETBLOCKING_API = "SetBlocking";
const std::string MOCK_STARTDHCPCLIENT_API = "StartDhcpClient";
const std::string MOCK_STOPDHCPCLIENT_API = "StopDhcpClient";
const std::string MOCK_REGISTERNOTIFYCALLBACK_API = "RegisterNotifyCallback";
const std::string MOCK_STARTDHCPSERVICE_API = "StartDhcpService";
const std::string MOCK_STOPDHCPSERVICE_API = "StopDhcpService";

class MockNetdNativeClient {
public:
    MockNetdNativeClient();
    ~MockNetdNativeClient();
    void Init();

    void RegisterMockApi();
    bool CheckMockApi(const std::string &api);

    /**
     * @brief Create a physical network
     *
     * @param netId
     * @param permission Permission to create a physical network
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission);

    /**
     * @brief Destroy the network
     *
     * @param netId
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkDestroy(int32_t netId);

    /**
     * @brief Add network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface);

    /**
     * @brief Delete network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface);

    /**
     * @brief Add route
     *
     * @param netId
     * @param ifName Network port device name
     * @param destination Target host ip
     * @param nextHop Next hop address
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkAddRoute(int32_t netId, const std::string &ifName, const std::string &destination,
        const std::string &nextHop);

    /**
     * @brief Remove route
     *
     * @param netId
     * @param ifName Network port device name
     * @param destination Target host ip
     * @param nextHop Next hop address
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkRemoveRoute(int32_t netId, const std::string &ifName, const std::string &destination,
        const std::string &nextHop);

    /**
     * @brief Turn off the device
     *
     * @param iface Network port device name
     */
    void SetInterfaceDown(const std::string &iface);

    /**
     * @brief Turn on the device
     *
     * @param iface Network port device name
     */
    void SetInterfaceUp(const std::string &iface);

    /**
     * @brief Clear the network interface ip address
     *
     * @param ifName Network port device name
     */
    void InterfaceClearAddrs(const std::string &ifName);

    /**
     * @brief Obtain mtu from the network interface device
     *
     * @param ifName Network port device name
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceGetMtu(const std::string &ifName);

    /**
     * @brief Set mtu to network interface device
     *
     * @param ifName Network port device name
     * @param mtu
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceSetMtu(const std::string &ifName, int32_t mtu);

    /**
     * @brief Add ip address
     *
     * @param ifName Network port device name
     * @param ipAddr    ip address
     * @param prefixLength  subnet mask
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceAddAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength);

    /**
     * @brief Delete ip address
     *
     * @param ifName Network port device name
     * @param ipAddr ip address
     * @param prefixLength subnet mask
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceDelAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength);

    /**
     * @brief Set dns
     *
     * @param netId
     * @param baseTimeoutMsec
     * @param retryCount
     * @param servers
     * @param domains
     * @return Return the return value of the netd interface call
     */
    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
        const std::vector<std::string> &servers, const std::vector<std::string> &domains);
    /**
     * @brief Get dns server param info
     *
     * @param netId
     * @param servers
     * @param domains
     * @param baseTimeoutMsec
     * @param retryCount
     * @return Return the return value of the netd interface call
     */
    int32_t GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
        std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount);

    /**
     * @brief Create dns cache before set dns
     *
     * @param netId
     * @return Return the return value for status of call
     */
    int32_t CreateNetworkCache(uint16_t netId);

    /**
     * @brief Destory dns cache
     *
     * @param netId
     * @return Return the return value of the netd interface call
     */
    int32_t DestoryNetworkCache(uint16_t netId);

    /**
     * @brief Flush dns cache
     *
     * @param netId
     * @return Return the return value of the netd interface call
     */
    int32_t FlushNetworkCache(uint16_t netId);

    /**
     * @brief Domain name resolution Obtains the domain name address
     *
     * @param hostName
     * @param serverName
     * @param hints
     * @param res
     * @return Return the return value of the netd interface call
     */
    int32_t GetAddrInfo(const std::string &hostName, const std::string &serverName,
        const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId);

    /**
     * @brief Obtains the bytes received over the cellular network.
     *
     * @return The number of received bytes.
     */
    int64_t GetCellularRxBytes();

    /**
     * @brief Obtains the bytes sent over the cellular network.
     *
     * @return The number of sent bytes.
     */
    int64_t GetCellularTxBytes();

    /**
     * @brief Obtains the bytes received through all NICs.
     *
     * @return The number of received bytes.
     */
    int64_t GetAllRxBytes();

    /**
     * @brief Obtains the bytes sent through all NICs.
     *
     * @return The number of sent bytes.
     */
    int64_t GetAllTxBytes();

    /**
     * @brief Obtains the bytes received through a specified UID.
     *
     * @param uid app id.
     * @return The number of received bytes.
     */
    int64_t GetUidRxBytes(uint32_t uid);

    /**
     * @brief Obtains the bytes sent through a specified UID.
     *
     * @param uid app id.
     * @return The number of sent bytes.
     */
    int64_t GetUidTxBytes(uint32_t uid);

    /**
     * @brief Obtains the bytes received through a specified UID on Iface.
     *
     * @param uid app id.
     * @param iface The name of the interface.
     * @return The number of received bytes.
     */
    int64_t GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName);

    /**
     * @brief Obtains the bytes sent through a specified UID on Iface.
     *
     * @param uid app id.
     * @param iface The name of the interface.
     * @return The number of sent bytes.
     */
    int64_t GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName);

    /**
     * @brief Obtains the bytes received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received bytes.
     */
    int64_t GetIfaceRxBytes(const std::string &interfaceName);

    /**
     * @brief Obtains the bytes sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent bytes.
     */
    int64_t GetIfaceTxBytes(const std::string &interfaceName);

    /**
     * @brief Obtains the NIC list.
     *
     * @return The list of interface.
     */
    std::vector<std::string> InterfaceGetList();

    /**
     * @brief Obtains the uid list.
     *
     * @return The list of uid.
     */
    std::vector<std::string> UidGetList();

    /**
     * @brief Obtains the packets received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received packets.
     */
    int64_t GetIfaceRxPackets(const std::string &interfaceName);

    /**
     * @brief Obtains the packets sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent packets.
     */
    int64_t GetIfaceTxPackets(const std::string &interfaceName);

    /**
     * @brief  set default network.
     *
     * @return Return the return value of the netd interface call
     */
    int32_t  SetDefaultNetWork(int32_t   netId);

    /**
     * @brief clear default network netId.
     *
     * @return Return the return value of the netd interface call
     */
    int32_t  ClearDefaultNetWorkNetId();

    /**
     * @brief Obtains the NIC list.
     *
     * @param socket_fd
     * @param netId
     * @return Return the return value of the netd interface call
     */
    int32_t BindSocket(int32_t socket_fd, uint32_t netId);

    /**
     * @brief Enable ip forwarding.
     *
     * @param requester the requester of forwarding
     * @return Return the return value of the netd interface call.
     */
    int32_t IpEnableForwarding(const std::string& requester);

    /**
     * @brief Disable ip forwarding.
     *
     * @param requester the requester of forwarding
     * @return Return the return value of the netd interface call.
     */
    int32_t IpDisableForwarding(const std::string& requester);

    /**
     * @brief Add tether forward.
     *
     * @param downstreamIface the name of downstream interface
     * @param upstreamIface the name of upstream interface
     * @return Return the return value of the netd interface call.
     */
    int32_t TetherAddForward(const std::string& downstreamIface, const std::string& upstreamIface);

    /**
     * @brief Remove tether forward.
     *
     * @param downstreamIface the name of downstream interface
     * @param upstreamIface the name of upstream interface
     * @return Return the return value of the netd interface call.
     */
    int32_t TetherRemoveForward(const std::string& downstreamIface, const std::string& upstreamIface);

    /**
     * @brief Add interface forward.
     *
     * @param fromIface the name of incoming interface
     * @param toIface the name of outcoming interface
     * @return Return the return value of the netd interface call.
     */
    int32_t IpfwdAddInterfaceForward(const std::string& fromIface, const std::string& toIface);

    /**
     * @brief Remove interface forward.
     *
     * @param fromIface the name of incoming interface
     * @param toIface the name of outcoming interface
     * @return Return the return value of the netd interface call.
     */
    int32_t IpfwdRemoveInterfaceForward(const std::string& fromIface, const std::string& toIface);

    /**
     * @brief Set tether dns.
     *
     * @param netId network id
     * @param dnsAddr the list of dns address
     * @return Return the return value of the netd interface call.
     */
    int32_t TetherDnsSet(uint32_t netId, const std::vector<std::string>& dnsAddrs);

    /**
     * @brief Set net callbackfuction.
     *
     * @param callback callbackfuction class
     * @return Return the return value of the netd interface call.
     */
    int32_t RegisterNetdNotifyCallback(const NetdNotifyCallback &callback);

    /**
     * @brief protect tradition network to connect VPN.
     *
     * @param socketFd socket file description
     * @return Return the return value of the netd interface call.
     */
    int32_t BindNetworkServiceVpn(int32_t socketFd);

    /**
     * @brief enable virtual network iterface card.
     *
     * @param socketFd socket file description
     * @param ifRequest interface request
     * @return Return the return value of the netd interface call.
     */
    int32_t EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd);

    /**
     * @brief Set ip address.
     *
     * @param socketFd socket file description
     * @param ipAddress ip address
     * @param prefixLen the mask of ip address
     * @param ifRequest interface request
     * @return Return the return value of the netd interface call.
     */
    int32_t SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen, struct ifreq &ifRequest);

    /**
     * @brief Set network blocking.
     *
     * @param ifaceFd interface file description
     * @param isBlock network blocking
     * @return Return the return value of the netd interface call.
     */
    int32_t SetBlocking(int32_t ifaceFd, bool isBlock);
    /**
    * @brief Start Dhcp Client.
    *
    * @param iface interface file description
    * @param bIpv6 network blocking
    * @return.
    */
    int32_t StartDhcpClient(const std::string &iface, bool bIpv6);
    /**
     * @brief Stop Dhcp Client.
     *
     * @param iface interface file description
     * @param bIpv6 network blocking
     * @return .
     */
    int32_t StopDhcpClient(const std::string &iface, bool bIpv6);
    /**
    * @brief Register Notify Callback
    *
    * @param callback
    * @return .
    */
    int32_t RegisterCallback(sptr<NetdControllerCallback> callback);
    /**
     * @brief start dhcpservice.
     *
     * @param iface interface name
     * @param ipv4addr ipv4 addr
     * @return Return the return value of the netd interface call.
     */
    int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr);

    /**
     * @brief stop dhcpservice.
     *
     * @param iface interface name
     * @return Return the return value of the netd interface call.
     */
    int32_t StopDhcpService(const std::string &iface);
private:
    int64_t GetIfaceBytes(const std::string &interfaceName, const std::string &filename);
    int64_t GetAllBytes(const std::string &filename);
    int32_t AddRoute(const std::string &ip, const std::string &mask, const std::string &gateWay,
        const std::string &devName);

private:
    std::unordered_set<std::string> mockApi_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_NETD_NATIVE_CLIENT_H
