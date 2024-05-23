/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef NETSYS_CONTROLLER_SERVICE_IMPL_H
#define NETSYS_CONTROLLER_SERVICE_IMPL_H

#include "i_netsys_controller_service.h"
#include "mock_netsys_native_client.h"
#include "netsys_native_client.h"

namespace OHOS {
namespace NetManagerStandard {
class NetsysControllerServiceImpl : public INetsysControllerService {
public:
    NetsysControllerServiceImpl() = default;
    ~NetsysControllerServiceImpl() = default;
    void Init() override;

    /**
     * Disallow or allow a app to create AF_INET or AF_INET6 socket
     *
     * @param uid App's uid which need to be disallowed ot allowed to create AF_INET or AF_INET6 socket
     * @param allow 0 means disallow, 1 means allow
     * @return return 0 if OK, return error number if not OK
     */
    int32_t SetInternetPermission(uint32_t uid, uint8_t allow) override;

    /**
     * Create a physical network
     *
     * @param netId
     * @param permission Permission to create a physical network
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission) override;

    int32_t NetworkCreateVirtual(int32_t netId, bool hasDns) override;

    /**
     * Destroy the network
     *
     * @param netId
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkDestroy(int32_t netId) override;

    int32_t NetworkAddUids(int32_t netId, const std::vector<UidRange> &uidRanges) override;
    int32_t NetworkDelUids(int32_t netId, const std::vector<UidRange> &uidRanges) override;

    /**
     * Add network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface) override;

    /**
     * Delete network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface) override;

    /**
     * Add route
     *
     * @param netId
     * @param ifName Network port device name
     * @param destination Target host ip
     * @param nextHop Next hop address
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkAddRoute(int32_t netId, const std::string &ifName, const std::string &destination,
                            const std::string &nextHop) override;

    /**
     * Remove route
     *
     * @param netId
     * @param ifName Network port device name
     * @param destination Target host ip
     * @param nextHop Next hop address
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkRemoveRoute(int32_t netId, const std::string &ifName, const std::string &destination,
                               const std::string &nextHop) override;

    /**
     * @brief Get interface config
     *
     * @param iface Network port device name
     * @return Return the result of this action， ERR_NONE is success.
     */
    int32_t GetInterfaceConfig(OHOS::nmd::InterfaceConfigurationParcel &cfg) override;

    /**
     * @brief Set interface config
     *
     * @param cfg Network port info
     * @return Return the result of this action， ERR_NONE is success.
     */
    int32_t SetInterfaceConfig(const OHOS::nmd::InterfaceConfigurationParcel &cfg) override;

    /**
     * Turn off the device
     *
     * @param iface Network port device name
     * @return Return the result of this action
     */
    int32_t SetInterfaceDown(const std::string &iface) override;

    /**
     * Turn on the device
     *
     * @param iface Network port device name
     * @return Return the result of this action
     */
    int32_t SetInterfaceUp(const std::string &iface) override;

    /**
     * Clear the network interface ip address
     *
     * @param ifName Network port device name
     */
    void ClearInterfaceAddrs(const std::string &ifName) override;

    /**
     * Obtain mtu from the network interface device
     *
     * @param ifName Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t GetInterfaceMtu(const std::string &ifName) override;

    /**
     * Set mtu to network interface device
     *
     * @param ifName Network port device name
     * @param mtu
     * @return Return the return value of the netsys interface call
     */
    int32_t SetInterfaceMtu(const std::string &ifName, int32_t mtu) override;

    /**
     * Set tcp buffer sizes
     *
     * @param tcpBufferSizes tcpBufferSizes
     * @return Return the return value of the netsys interface call
     */
    int32_t SetTcpBufferSizes(const std::string &tcpBufferSizes) override;

    /**
     * Add ip address
     *
     * @param ifName Network port device name
     * @param ipAddr    ip address
     * @param prefixLength  subnet mask
     * @return Return the return value of the netsys interface call
     */
    int32_t AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength) override;

    /**
     * Delete ip address
     *
     * @param ifName Network port device name
     * @param ipAddr ip address
     * @param prefixLength subnet mask
     * @return Return the return value of the netsys interface call
     */
    int32_t DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength) override;

    /**
     * Set iface ip address
     *
     * @param ifaceName Network port device name
     * @param ipAddress Ip address
     * @return Return the return value of the netsys interface call
     */
    int32_t InterfaceSetIpAddress(const std::string &ifaceName, const std::string &ipAddress) override;

    /**
     * Set iface up
     *
     * @param ifaceName Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t InterfaceSetIffUp(const std::string &ifaceName) override;

    /**
     * Set dns
     *
     * @param netId
     * @param baseTimeoutMsec
     * @param retryCount
     * @param servers
     * @param domains
     * @return Return the return value of the netsys interface call
     */
    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                              const std::vector<std::string> &servers,
                              const std::vector<std::string> &domains) override;

    /**
     * Get dns server param info
     *
     * @param netId
     * @param servers
     * @param domains
     * @param baseTimeoutMsec
     * @param retryCount
     * @return Return the return value of the netsys interface call
     */
    int32_t GetResolverConfig(uint16_t netId, std::vector<std::string> &servers, std::vector<std::string> &domains,
                              uint16_t &baseTimeoutMsec, uint8_t &retryCount) override;

    /**
     * Create dns cache before set dns
     *
     * @param netId
     * @return Return the return value for status of call
     */
    int32_t CreateNetworkCache(uint16_t netId) override;

    /**
     * Destroy dns cache
     *
     * @param netId
     * @return Return the return value of the netsys interface call
     */
    int32_t DestroyNetworkCache(uint16_t netId) override;

    /**
     * Domain name resolution Obtains the domain name address
     *
     * @param hostName Domain name to be resolved
     * @param serverName Server name used for query
     * @param hints Limit parameters when querying
     * @param netId Network id
     * @param res return addrinfo
     * @return Return the return value of the netsys interface call
     */
    int32_t GetAddrInfo(const std::string &hostName, const std::string &serverName, const AddrInfo &hints,
                        uint16_t netId, std::vector<AddrInfo> &res) override;

    /**
     * Obtains the bytes of the sharing network.
     *
     * @return Success return 0.
     */
    int32_t GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                     nmd::NetworkSharingTraffic &traffic) override;

    /**
     * Obtains the bytes received over the cellular network.
     *
     * @return The number of received bytes.
     */
    int64_t GetCellularRxBytes() override;

    /**
     * Obtains the bytes sent over the cellular network.
     *
     * @return The number of sent bytes.
     */
    int64_t GetCellularTxBytes() override;

    /**
     * Obtains the bytes received through all NICs.
     *
     * @return The number of received bytes.
     */
    int64_t GetAllRxBytes() override;

    /**
     * Obtains the bytes sent through all NICs.
     *
     * @return The number of sent bytes.
     */
    int64_t GetAllTxBytes() override;

    /**
     * Obtains the bytes received through a specified UID.
     *
     * @param uid app id.
     * @return The number of received bytes.
     */
    int64_t GetUidRxBytes(uint32_t uid) override;

    /**
     * Obtains the bytes sent through a specified UID.
     *
     * @param uid app id.
     * @return The number of sent bytes.
     */
    int64_t GetUidTxBytes(uint32_t uid) override;

    /**
     * Obtains the bytes received through a specified UID on Iface.
     *
     * @param uid app id.
     * @param iface The name of the interface.
     * @return The number of received bytes.
     */
    int64_t GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName) override;

    /**
     * Obtains the bytes sent through a specified UID on Iface.
     *
     * @param uid app id.
     * @param iface The name of the interface.
     * @return The number of sent bytes.
     */
    int64_t GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName) override;

    /**
     * Obtains the bytes received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received bytes.
     */
    int64_t GetIfaceRxBytes(const std::string &interfaceName) override;

    /**
     * Obtains the bytes sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent bytes.
     */
    int64_t GetIfaceTxBytes(const std::string &interfaceName) override;

    /**
     * Obtains the NIC list.
     *
     * @return The list of interface.
     */
    std::vector<std::string> InterfaceGetList() override;

    /**
     * Obtains the uid list.
     *
     * @return The list of uid.
     */
    std::vector<std::string> UidGetList() override;

    /**
     * Obtains the packets received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received packets.
     */
    int64_t GetIfaceRxPackets(const std::string &interfaceName) override;

    /**
     * Obtains the packets sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent packets.
     */
    int64_t GetIfaceTxPackets(const std::string &interfaceName) override;

    /**
     *  set default network.
     *
     * @return Return the return value of the netsys interface call
     */
    int32_t SetDefaultNetWork(int32_t netId) override;

    /**
     * clear default network netId.
     *
     * @return Return the return value of the netsys interface call
     */
    int32_t ClearDefaultNetWorkNetId() override;

    /**
     * Obtains the NIC list.
     *
     * @param socketFd
     * @param netId
     * @return Return the return value of the netsys interface call
     */
    int32_t BindSocket(int32_t socketFd, uint32_t netId) override;

    /**
     * Enable ip forwarding.
     *
     * @param requestor the requestor of forwarding
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpEnableForwarding(const std::string &requestor) override;

    /**
     * Disable ip forwarding.
     *
     * @param requestor the requestor of forwarding
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpDisableForwarding(const std::string &requestor) override;

    /**
     * Enable Nat.
     *
     * @param downstreamIface the name of downstream interface
     * @param upstreamIface the name of upstream interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t EnableNat(const std::string &downstramIface, const std::string &upstreamIface) override;
    /**
     * Disable Nat.
     *
     * @param downstreamIface the name of downstream interface
     * @param upstreamIface the name of upstream interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t DisableNat(const std::string &downstramIface, const std::string &upstreamIface) override;

    /**
     * Add interface forward.
     *
     * @param fromIface the name of incoming interface
     * @param toIface the name of outcoming interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface) override;

    /**
     * Remove interface forward.
     *
     * @param fromIface the name of incoming interface
     * @param toIface the name of outcoming interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface) override;

    /**
     * Set tether dns.
     *
     * @param netId network id
     * @param dnsAddr the list of dns address
     * @return Return the return value of the netsys interface call.
     */
    int32_t ShareDnsSet(uint16_t netId) override;

    /**
     * start dns proxy listen
     *
     * @return int32_t
     */
    int32_t StartDnsProxyListen() override;

    /**
     * stop dns proxy listen
     *
     * @return int32_t
     */
    int32_t StopDnsProxyListen() override;

    /**
     * Set net callbackfuction.
     *
     * @param callback callbackfuction class
     * @return Return the return value of the netsys interface call.
     */
    int32_t RegisterNetsysNotifyCallback(const NetsysNotifyCallback &callback) override;

    /**
     * protect tradition network to connect VPN.
     *
     * @param socketFd socket file description
     * @return Return the return value of the netsys interface call.
     */
    int32_t BindNetworkServiceVpn(int32_t socketFd) override;

    /**
     * enable virtual network iterface card.
     *
     * @param socketFd socket file description
     * @param ifRequest interface request
     * @return Return the return value of the netsys interface call.
     */
    int32_t EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd) override;

    /**
     * Set ip address.
     *
     * @param socketFd socket file description
     * @param ipAddress ip address
     * @param prefixLen the mask of ip address
     * @param ifRequest interface request
     * @return Return the return value of the netsys interface call.
     */
    int32_t SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
                         struct ifreq &ifRequest) override;

    /**
     * Set network blocking.
     *
     * @param ifaceFd interface file description
     * @param isBlock network blocking
     * @return Return the return value of the netsys interface call.
     */
    int32_t SetBlocking(int32_t ifaceFd, bool isBlock) override;
    /**
     * Start Dhcp Client.
     *
     * @param iface interface file description
     * @param bIpv6 network blocking
     * @return Return the return value of the netsys interface call.
     */
    int32_t StartDhcpClient(const std::string &iface, bool bIpv6) override;
    /**
     * Stop Dhcp Client.
     *
     * @param iface interface file description
     * @param bIpv6 network blocking
     * @return Return the return value of the netsys interface call.
     */
    int32_t StopDhcpClient(const std::string &iface, bool bIpv6) override;
    /**
     * Register Notify Callback
     *
     * @param callback
     * @return Return the return value of the netsys interface call.
     */
    int32_t RegisterCallback(sptr<NetsysControllerCallback> callback) override;

    /**
     * start dhcpservice.
     *
     * @param iface interface name
     * @param ipv4addr ipv4 addr
     * @return Return the return value of the netsys interface call.
     */
    int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr) override;

    /**
     * stop dhcpservice.
     *
     * @param iface interface name
     * @return Return the return value of the netsys interface call.
     */
    int32_t StopDhcpService(const std::string &iface) override;

    /**
     * Turn on data saving mode.
     *
     * @param enable enable or disable
     * @return value the return value of the netsys interface call.
     */
    int32_t BandwidthEnableDataSaver(bool enable) override;

    /**
     * Set quota.
     *
     * @param iface interface name
     * @param bytes
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes) override;

    /**
     * Delete quota.
     *
     * @param iface interface name
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthRemoveIfaceQuota(const std::string &ifName) override;

    /**
     * Add DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthAddDeniedList(uint32_t uid) override;

    /**
     * Remove DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthRemoveDeniedList(uint32_t uid) override;

    /**
     * Add DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthAddAllowedList(uint32_t uid) override;

    /**
     * Remove DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthRemoveAllowedList(uint32_t uid) override;

    /**
     * Set firewall rules.
     *
     * @param chain chain type
     * @param isAllowedList is or not AllowedList
     * @param uids
     * @return value the return value of the netsys interface call.
     */
    int32_t FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override;

    /**
     * Set firewall rules.
     *
     * @param chain chain type
     * @param isAllowedList is or not AllowedList
     * @param uids
     * @return value the return value of the netsys interface call.
     */
    int32_t FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override;

    /**
     * Enable or disable the specified firewall chain.
     *
     * @param chain chain type
     * @param enable enable or disable
     * @return Return the return value of the netsys interface call.
     */
    int32_t FirewallEnableChain(uint32_t chain, bool enable) override;

    /**
     * Firewall set uid rule.
     *
     * @param chain chain type
     * @param uid uid
     * @param firewallRule firewall rule
     * @return Return the return value of the netsys interface call.
     */
    int32_t FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids, uint32_t firewallRule) override;

    /**
     * Get total traffic
     *
     * @param stats stats
     * @param type type
     * @return returns the total traffic of the specified type
     */
    int32_t GetTotalStats(uint64_t &stats, uint32_t type) override;

    /**
     * Get uid traffic
     *
     * @param stats stats
     * @param type type
     * @param uid uid
     * @return returns the traffic of the uid
     */
    int32_t GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid) override;

    /**
     * Get Iface traffic
     *
     * @param stats stats
     * @param type type
     * @param interfaceName interfaceName
     * @return returns the traffic of the Iface
     */
    int32_t GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName) override;

    /**
     * Get all container stats info
     * @param stats stats
     * @return returns the all info of the stats
     */
    int32_t GetAllContainerStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override;

    /**
     * Get all stats info
     *
     * @param stats stats
     * @return returns the all info of the stats
     */
    int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override;

    /**
     * Set iptables for result
     *
     * @param cmd Iptables command
     * @param respond The respond of execute iptables command
     * @return Value the return value of the netsys interface call
     */
    int32_t SetIptablesCommandForRes(const std::string &cmd, std::string &respond) override;

    /**
     * Check network connectivity by sending packets to a host and reporting its response.
     *
     * @param pingOption Ping option
     * @param callback The respond of execute ping cmd.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagPingHost(const OHOS::NetsysNative::NetDiagPingOption &pingOption,
                            const sptr<OHOS::NetsysNative::INetDiagCallback> &callback) override;

    /**
     * Get networking route table
     *
     * @param routeTables Network route table list.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagGetRouteTable(std::list<OHOS::NetsysNative::NetDiagRouteTable> &routeTables) override;

    /**
     * Get networking sockets info.
     *
     * @param socketType Network protocol.
     * @param socketsInfo The result of network sockets info.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagGetSocketsInfo(OHOS::NetsysNative::NetDiagProtocolType socketType,
                                  OHOS::NetsysNative::NetDiagSocketsInfo &socketsInfo) override;

    /**
     * Get network interface configuration.
     *
     * @param configs The result of network interface configuration.
     * @param ifaceName Get interface configuration information for the specified interface name.
     *                  If the interface name is empty, default to getting all interface configuration information.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagGetInterfaceConfig(std::list<OHOS::NetsysNative::NetDiagIfaceConfig> &configs,
                                      const std::string &ifaceName) override;

    /**
     * Update network interface configuration.
     *
     * @param configs Network interface configuration.
     * @param ifaceName Interface name.
     * @param add Add or delete.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagUpdateInterfaceConfig(const OHOS::NetsysNative::NetDiagIfaceConfig &config,
                                         const std::string &ifaceName, bool add) override;

    /**
     * Set network interface up/down state.
     *
     * @param ifaceName Interface name.
     * @param up Up or down.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagSetInterfaceActiveState(const std::string &ifaceName, bool up) override;
    int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName) override;

    int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName) override;

    /**
     * Register Dns Result Callback Listener.
     *
     * @param callback Callback function
     * @param timestep Time gap between two callbacks
     * @return Value the return value of the netsys interface call
     */
    int32_t RegisterDnsResultCallback(const sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> &callback,
        uint32_t timeStep) override;

    /**
     * Unregister Dns Result Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t UnregisterDnsResultCallback(
        const sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> &callback) override;

    /**
     * Register Dns Health Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t RegisterDnsHealthCallback(const sptr<OHOS::NetsysNative::INetDnsHealthCallback> &callback) override;

    /**
     * Unregister Dns Health Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t UnregisterDnsHealthCallback(const sptr<OHOS::NetsysNative::INetDnsHealthCallback> &callback) override;

    /**
     * Get Cookie Stats.
     *
     * @param stats stats
     * @param type type
     * @param cookie cookie
     * @return Value the return value of the netsys interface call
     */
    int32_t GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie) override;

    int32_t GetNetworkSharingType(std::set<uint32_t>& sharingTypeIsOn) override;
    
    int32_t UpdateNetworkSharingType(uint32_t type, bool isOpen) override;

    int32_t SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on) override;

    int32_t SetEnableIpv6(const std::string &interfaceName, const uint32_t on) override;

    /**
     * Set the policy to access the network of the specified application.
     *
     * @param uid - The specified UID of application.
     * @param policy - the network access policy of application. For details, see {@link NetworkAccessPolicy}.
     * @param reconfirmFlag true means a reconfirm diaglog trigger while policy deny network access.
     * @return return 0 if OK, return error number if not OK
     */
    int32_t SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag) override;
    int32_t DeleteNetworkAccessPolicy(uint32_t uid) override;
    int32_t NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes) override;
private:
    MockNetsysNativeClient mockNetsysClient_;
    NetsysNativeClient netsysClient_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETSYS_CONTROLLER_SERVICE_IMPL_H
