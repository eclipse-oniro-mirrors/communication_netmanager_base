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

#ifndef NETSYS_NATIVE_CLIENT_H
#define NETSYS_NATIVE_CLIENT_H

#include <linux/if.h>
#include <memory>
#include <netdb.h>
#include <string>
#include <vector>

#include "i_netsys_service.h"
#include "i_net_diag_callback.h"
#include "i_net_dns_health_callback.h"
#include "net_dns_result_callback_stub.h"
#include "netsys_controller_callback.h"
#include "netsys_controller_define.h"
#include "network_sharing.h"
#include "notify_callback_stub.h"
#include "netsys_dns_report_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetsysNativeClient : public std::enable_shared_from_this<NetsysNativeClient> {
private:
    class NativeNotifyCallback : public OHOS::NetsysNative::NotifyCallbackStub {
    public:
        explicit NativeNotifyCallback(std::weak_ptr<NetsysNativeClient> netsysNativeClient);
        ~NativeNotifyCallback() override = default;
        int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAdded(const std::string &ifName) override;
        int32_t OnInterfaceRemoved(const std::string &ifName) override;
        int32_t OnInterfaceChanged(const std::string &ifName, bool up) override;
        int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up) override;
        int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                               const std::string &ifName) override;
        int32_t OnDhcpSuccess(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult) override;
        int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override;

    private:
        std::weak_ptr<NetsysNativeClient> netsysNativeClient_;
    };

    class NativeNetDnsResultCallback : public OHOS::NetsysNative::NetDnsResultCallbackStub {
    public:
        explicit NativeNetDnsResultCallback(std::weak_ptr<NetsysNativeClient> netsysNativeClient);
        ~NativeNetDnsResultCallback() override = default;
        int32_t OnDnsResultReport(uint32_t size, std::list<OHOS::NetsysNative::NetDnsResultReport> res) override;
        int32_t OnDnsQueryResultReport(uint32_t size,
            std::list<OHOS::NetsysNative::NetDnsQueryResultReport> res) override;
        int32_t OnDnsQueryAbnormalReport(uint32_t eventfailcause,
            OHOS::NetsysNative::NetDnsQueryResultReport res) override;

    private:
        std::weak_ptr<NetsysNativeClient> netsysNativeClient_;
    };

public:
    NetsysNativeClient();
    ~NetsysNativeClient();
    void Init();

    /**
     * Disallow or allow a app to create AF_INET or AF_INET6 socket
     *
     * @param uid App's uid which need to be disallowed ot allowed to create AF_INET or AF_INET6 socket
     * @param allow 0 means disallow, 1 means allow
     * @return return 0 if OK, return error number if not OK
     */
    int32_t SetInternetPermission(uint32_t uid, uint8_t allow);

    /**
     * Create a physical network
     *
     * @param netId
     * @param permission Permission to create a physical network
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission);

    int32_t NetworkCreateVirtual(int32_t netId, bool hasDns);
    int32_t NetworkAddUids(int32_t netId, const std::vector<UidRange> &uidRanges);
    int32_t NetworkDelUids(int32_t netId, const std::vector<UidRange> &uidRanges);

    /**
     * Destroy the network
     *
     * @param netId
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkDestroy(int32_t netId, bool isVpnNet = false);

    int32_t CreateVnic(uint16_t mtu, const std::string &tunAddr, int32_t prefix, const std::set<int32_t> &uids);
    int32_t DestroyVnic();
    int32_t EnableDistributedClientNet(const std::string &virnicAddr, const std::string &iif);
    int32_t EnableDistributedServerNet(const std::string &iif, const std::string &devIface, const std::string &dstAddr);
    int32_t DisableDistributedNet(bool isServer);

    /**
     * Add network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface, NetBearType netBearerType);

    /**
     * Delete network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface);

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
                            const std::string &nextHop);

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
                               const std::string &nextHop);

    /**
     * @brief Get interface config
     *
     * @param iface Network port device name
     * @return Return the result of this action， ERR_NONE is success.
     */
    int32_t GetInterfaceConfig(OHOS::nmd::InterfaceConfigurationParcel &cfg);

    /**
     * @brief Set interface config
     *
     * @param cfg Network port info
     * @return Return the result of this action， ERR_NONE is success.
     */
    int32_t SetInterfaceConfig(const OHOS::nmd::InterfaceConfigurationParcel &cfg);

    /**
     * Turn off the device
     *
     * @param iface Network port device name
     * @return Return the result of this action
     */
    int32_t SetInterfaceDown(const std::string &iface);

    /**
     * Turn on the device
     *
     * @param iface Network port device name
     * @return Return the result of this action
     */
    int32_t SetInterfaceUp(const std::string &iface);

    /**
     * Clear the network interface ip address
     *
     * @param ifName Network port device name
     */
    void ClearInterfaceAddrs(const std::string &ifName);

    /**
     * Obtain mtu from the network interface device
     *
     * @param ifName Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t GetInterfaceMtu(const std::string &ifName);

    /**
     * Set mtu to network interface device
     *
     * @param ifName Network port device name
     * @param mtu
     * @return Return the return value of the netsys interface call
     */
    int32_t SetInterfaceMtu(const std::string &ifName, int32_t mtu);

    /**
     * Set tcp buffer sizes
     *
     * @param tcpBufferSizes tcpBufferSizes
     * @return Return the return value of the netsys interface call
     */
    int32_t SetTcpBufferSizes(const std::string &tcpBufferSizes);

    /**
     * Add ip address
     *
     * @param ifName Network port device name
     * @param ipAddr    ip address
     * @param prefixLength  subnet mask
     * @return Return the return value of the netsys interface call
     */
    int32_t AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength);

    /**
     * Delete ip address
     *
     * @param ifName Network port device name
     * @param ipAddr ip address
     * @param prefixLength subnet mask
     * @return Return the return value of the netsys interface call
     */
    int32_t DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength);

    /**
     * Delete ip address
     *
     * @param ifName Network port device name
     * @param ipAddr ip address
     * @param prefixLength subnet mask
     * @param netCapabilities Net capabilities in string format
     * @return Return the return value of the netsys interface call
     */
    int32_t DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength,
                                const std::string &netCapabilities);

    /**
     * Set iface ip address
     *
     * @param ifaceName Network port device name
     * @param ipAddress Ip address
     * @return Return the return value of the netsys interface call
     */
    int32_t InterfaceSetIpAddress(const std::string &ifaceName, const std::string &ipAddress);

    /**
     * Set iface up
     *
     * @param ifaceName Network port device name
     * @return Return the return value of the netsys interface call
     */
    int32_t InterfaceSetIffUp(const std::string &ifaceName);

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
                              const std::vector<std::string> &servers, const std::vector<std::string> &domains);

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
                              uint16_t &baseTimeoutMsec, uint8_t &retryCount);

    /**
     * Create dns cache before set dns
     *
     * @param netId
     * @return Return the return value for status of call
     */
    int32_t CreateNetworkCache(uint16_t netId, bool isVpnNet = false);

    /**
     * Destroy dns cache
     *
     * @param netId
     * @return Return the return value of the netsys interface call
     */
    int32_t DestroyNetworkCache(uint16_t netId, bool isVpnNet = false);

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
                        uint16_t netId, std::vector<AddrInfo> &res);

    /**
     * Obtains the bytes of the sharing network.
     *
     * @return Success return 0.
     */
    int32_t GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                     nmd::NetworkSharingTraffic &traffic);
    
    /**
     * Obtains the bytes of the cellular sharing network.
     *
     * @return Success return 0.
     */
    int32_t GetNetworkCellularSharingTraffic(nmd::NetworkSharingTraffic &traffic, std::string &ifaceName);

    /**
     * Obtains the bytes received over the cellular network.
     *
     * @return The number of received bytes.
     */
    int64_t GetCellularRxBytes();

    /**
     * Obtains the bytes sent over the cellular network.
     *
     * @return The number of sent bytes.
     */
    int64_t GetCellularTxBytes();

    /**
     * Obtains the bytes received through all NICs.
     *
     * @return The number of received bytes.
     */
    int64_t GetAllRxBytes();

    /**
     * Obtains the bytes sent through all NICs.
     *
     * @return The number of sent bytes.
     */
    int64_t GetAllTxBytes();

    /**
     * Obtains the bytes received through a specified UID.
     *
     * @param uid app id.
     * @return The number of received bytes.
     */
    int64_t GetUidRxBytes(uint32_t uid);

    /**
     * Obtains the bytes sent through a specified UID.
     *
     * @param uid app id.
     * @return The number of sent bytes.
     */
    int64_t GetUidTxBytes(uint32_t uid);

    /**
     * Obtains the bytes received through a specified UID on Iface.
     *
     * @param uid app id.
     * @param iface The name of the interface.
     * @return The number of received bytes.
     */
    int64_t GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName);

    /**
     * Obtains the bytes sent through a specified UID on Iface.
     *
     * @param uid app id.
     * @param iface The name of the interface.
     * @return The number of sent bytes.
     */
    int64_t GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName);

    /**
     * Obtains the bytes received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received bytes.
     */
    int64_t GetIfaceRxBytes(const std::string &interfaceName);

    /**
     * Obtains the bytes sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent bytes.
     */
    int64_t GetIfaceTxBytes(const std::string &interfaceName);

    /**
     * Obtains the NIC list.
     *
     * @return The list of interface.
     */
    std::vector<std::string> InterfaceGetList();

    /**
     * Obtains the uid list.
     *
     * @return The list of uid.
     */
    std::vector<std::string> UidGetList();

    /**
     * Obtains the packets received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received packets.
     */
    int64_t GetIfaceRxPackets(const std::string &interfaceName);

    /**
     * Obtains the packets sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent packets.
     */
    int64_t GetIfaceTxPackets(const std::string &interfaceName);

    /**
     *  set default network.
     *
     * @return Return the return value of the netsys interface call
     */
    int32_t SetDefaultNetWork(int32_t netId);

    /**
     * clear default network netId.
     *
     * @return Return the return value of the netsys interface call
     */
    int32_t ClearDefaultNetWorkNetId();

    /**
     * Obtains the NIC list.
     *
     * @param socketFd
     * @param netId
     * @return Return the return value of the netsys interface call
     */
    int32_t BindSocket(int32_t socketFd, uint32_t netId);

    /**
     * Enable ip forwarding.
     *
     * @param requestor the requestor of forwarding
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpEnableForwarding(const std::string &requestor);

    /**
     * Disable ip forwarding.
     *
     * @param requestor the requestor of forwarding
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpDisableForwarding(const std::string &requestor);

    /**
     * Enable Nat.
     *
     * @param downstreamIface the name of downstream interface
     * @param upstreamIface the name of upstream interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t EnableNat(const std::string &downstreamIface, const std::string &upstreamIface);
    /**
     * Disable Nat.
     *
     * @param downstreamIface the name of downstream interface
     * @param upstreamIface the name of upstream interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t DisableNat(const std::string &downstreamIface, const std::string &upstreamIface);

    /**
     * Add interface forward.
     *
     * @param fromIface the name of incoming interface
     * @param toIface the name of outcoming interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface);

    /**
     * Remove interface forward.
     *
     * @param fromIface the name of incoming interface
     * @param toIface the name of outcoming interface
     * @return Return the return value of the netsys interface call.
     */
    int32_t IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface);

    /**
     * Set tether dns.
     *
     * @param netId network id
     * @param dnsAddr the list of dns address
     * @return Return the return value of the netsys interface call.
     */
    int32_t ShareDnsSet(uint16_t netId);

    /**
     * tart dns proxy listen
     *
     * @return Return the return value of the netsys interface call.
     */
    int32_t StartDnsProxyListen();

    /**
     * stop dns proxy listen
     *
     * @return Return the return value of the netsys interface call.
     */
    int32_t StopDnsProxyListen();

    /**
     * Set net callback function.
     *
     * @param callback callback function class
     * @return Return the return value of the netsys interface call.
     */
    int32_t RegisterNetsysNotifyCallback(const NetsysNotifyCallback &callback);

    /**
     * protect tradition network to connect VPN.
     *
     * @param socketFd socket file description
     * @return Return the return value of the netsys interface call.
     */
    int32_t BindNetworkServiceVpn(int32_t socketFd);

    /**
     * enable virtual network interface card.
     *
     * @param socketFd socket file description
     * @param ifRequest interface request
     * @return Return the return value of the netsys interface call.
     */
    int32_t EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd);

    /**
     * Set ip address.
     *
     * @param socketFd socket file description
     * @param ipAddress ip address
     * @param prefixLen the mask of ip address
     * @param ifRequest interface request
     * @return Return the return value of the netsys interface call.
     */
    int32_t SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen, struct ifreq &ifRequest);

    /**
     * Set network blocking.
     *
     * @param ifaceFd interface file description
     * @param isBlock network blocking
     * @return Return the return value of the netsys interface call.
     */
    int32_t SetBlocking(int32_t ifaceFd, bool isBlock);

    /**
     * Start Dhcp Client.
     *
     * @param iface interface file description
     * @param bIpv6 network blocking
     * @return Return the return value of the netsys interface call.
     */
    int32_t StartDhcpClient(const std::string &iface, bool bIpv6);

    /**
     * Stop Dhcp Client.
     *
     * @param iface interface file description
     * @param bIpv6 network blocking
     * @return Return the return value of the netsys interface call.
     */
    int32_t StopDhcpClient(const std::string &iface, bool bIpv6);

    /**
     * Register Notify Callback
     *
     * @param callback
     * @return Return the return value of the netsys interface call.
     */
    int32_t RegisterCallback(const sptr<NetsysControllerCallback> &callback);

    /**
     * start dhcpservice.
     *
     * @param iface interface name
     * @param ipv4addr ipv4 addr
     * @return Return the return value of the netsys interface call.
     */
    int32_t StartDhcpService(const std::string &iface, const std::string &ipv4addr);

    /**
     * stop dhcpservice.
     *
     * @param iface interface name
     * @return Return the return value of the netsys interface call.
     */
    int32_t StopDhcpService(const std::string &iface);

    /**
     * Turn on data saving mode.
     *
     * @param enable enable or disable
     * @return value the return value of the netsys interface call.
     */
    int32_t BandwidthEnableDataSaver(bool enable);

    /**
     * Set quota.
     *
     * @param iface interface name
     * @param bytes
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes);

    /**
     * delete quota.
     *
     * @param iface interface name
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthRemoveIfaceQuota(const std::string &ifName);

    /**
     * Add DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthAddDeniedList(uint32_t uid);

    /**
     * Remove DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthRemoveDeniedList(uint32_t uid);

    /**
     * Add DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthAddAllowedList(uint32_t uid);

    /**
     * Remove DeniedList.
     *
     * @param uid
     * @return Return the return value of the netsys interface call.
     */
    int32_t BandwidthRemoveAllowedList(uint32_t uid);

    /**
     * Set firewall rules.
     *
     * @param chain chain type
     * @param isAllowedList is or not AllowedList
     * @param uids
     * @return value the return value of the netsys interface call.
     */
    int32_t FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids);

    /**
     * Set firewall rules.
     *
     * @param chain chain type
     * @param isAllowedList is or not AllowedList
     * @param uids
     * @return value the return value of the netsys interface call.
     */
    int32_t FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids);

    /**
     * Enable or disable the specified firewall chain.
     *
     * @param chain chain type
     * @param enable enable or disable
     * @return Return the return value of the netsys interface call.
     */
    int32_t FirewallEnableChain(uint32_t chain, bool enable);

    /**
     * Firewall set uid rule.
     *
     * @param chain chain type
     * @param uid uid
     * @param firewallRule firewall rule
     * @return Return the return value of the netsys interface call.
     */
    int32_t FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids, uint32_t firewallRule);

    /**
     * Get total traffic
     *
     * @param stats stats
     * @param type type
     * @return returns the total traffic of the specified type
     */
    int32_t GetTotalStats(uint64_t &stats, uint32_t type);

    /**
     * Get uid traffic
     *
     * @param stats stats
     * @param type type
     * @param uid uid
     * @return returns the traffic of the uid
     */
    int32_t GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid);

    /**
     * Get Iface traffic
     *
     * @param stats stats
     * @param type type
     * @param interfaceName interfaceName
     * @return returns the traffic of the Iface
     */
    int32_t GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName);

    /**
     * Get all Sim stats info
     * @param stats stats
     * @return returns the all info of the stats
     */
    int32_t GetAllSimStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats);

    /**
     * Delete the Sim Iface Stats with uid
     *
     * @param uid the uid of application
     * @return returns 0 for success other as failed.
     */
    int32_t DeleteSimStatsInfo(uint32_t uid);

    /**
     * Get all stats info
     *
     * @param stats stats
     * @return returns the all info of the stats
     */
    int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats);

    /**
     * Delete the Iface Stats with uid
     *
     * @param uid the uid of application
     * @return returns 0 for success other as failed.
     */
    int32_t DeleteStatsInfo(uint32_t uid);

    int32_t SetNetStateTrafficMap(uint8_t flag, uint64_t availableTraffic);
    int32_t GetNetStateTrafficMap(uint8_t flag, uint64_t &availableTraffic);
    int32_t ClearIncreaseTrafficMap();
    int32_t DeleteIncreaseTrafficMap(uint64_t ifIndex);
    int32_t UpdateIfIndexMap(int8_t key, uint64_t index);
    int32_t SetNetStatusMap(uint8_t type, uint8_t value);

    /**
     * Set iptables for result
     *
     * @param cmd Iptables command
     * @param respond The respond of execute iptables command
     * @param ipType The type of iptables command.
     * @return Value the return value of the netsys interface call
     */
    int32_t SetIptablesCommandForRes(const std::string &cmd, std::string &respond, NetsysNative::IptablesType ipType);

    /**
     * Set ip command for result
     *
     * @param cmd ip command
     * @param respond The respond of execute ip command
     * @return Value the return value of the netsys interface call
     */
    int32_t SetIpCommandForRes(const std::string &cmd, std::string &respond);

    /**
     * Check network connectivity by sending packets to a host and reporting its response.
     *
     * @param pingOption Ping option
     * @param callback The respond of execute ping cmd.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagPingHost(const OHOS::NetsysNative::NetDiagPingOption &pingOption,
                            const sptr<OHOS::NetsysNative::INetDiagCallback> &callback);

    /**
     * Get networking route table
     *
     * @param routeTables Network route table list.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagGetRouteTable(std::list<OHOS::NetsysNative::NetDiagRouteTable> &routeTables);

    /**
     * Get networking sockets info.
     *
     * @param socketType Network protocol.
     * @param socketsInfo The result of network sockets info.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagGetSocketsInfo(OHOS::NetsysNative::NetDiagProtocolType socketType,
                                  OHOS::NetsysNative::NetDiagSocketsInfo &socketsInfo);

    /**
     * Get network interface configuration.
     *
     * @param configs The result of network interface configuration.
     * @param ifaceName Get interface configuration information for the specified interface name.
     *                  If the interface name is empty, default to getting all interface configuration information.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagGetInterfaceConfig(std::list<OHOS::NetsysNative::NetDiagIfaceConfig> &configs,
                                      const std::string &ifaceName);

    /**
     * Update network interface configuration.
     *
     * @param configs Network interface configuration.
     * @param ifaceName Interface name.
     * @param add Add or delete.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagUpdateInterfaceConfig(const OHOS::NetsysNative::NetDiagIfaceConfig &config,
                                         const std::string &ifaceName, bool add);

    /**
     * Set network interface up/down state.
     *
     * @param ifaceName Interface name.
     * @param up Up or down.
     * @return Value the return value of the netsys interface call
     */
    int32_t NetDiagSetInterfaceActiveState(const std::string &ifaceName, bool up);
    int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName);
    int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName);

        /**
     * Register Dns Result Callback Listener.
     *
     * @param callback Callback function
     * @param timestep Time gap between two callbacks
     * @return Value the return value of the netsys interface call
     */
    int32_t RegisterDnsResultCallback(const sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> &callback,
        uint32_t timeStep);

    /**
     * Unregister Dns Result Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t UnregisterDnsResultCallback(const sptr<OHOS::NetManagerStandard::NetsysDnsReportCallback> &callback);

    /**
     * Register Dns Result Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t RegisterDnsQueryResultCallback(
        const sptr<OHOS::NetManagerStandard::NetsysDnsQueryReportCallback> &callback);

    /**
     * Unregister Dns Result Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t UnregisterDnsQueryResultCallback(
        const sptr<OHOS::NetManagerStandard::NetsysDnsQueryReportCallback> &callback);

    /**
     * Register Dns Health Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t RegisterDnsHealthCallback(const sptr<OHOS::NetsysNative::INetDnsHealthCallback> &callback);

    /**
     * Unregister Dns Health Callback Listener.
     *
     * @param callback Callback function
     * @return Value the return value of the netsys interface call
     */
    int32_t UnregisterDnsHealthCallback(const sptr<OHOS::NetsysNative::INetDnsHealthCallback> &callback);

    /**
     * Get Cookie Stats.
     *
     * @param stats stats
     * @param type type
     * @param cookie cookie
     * @return Value the return value of the netsys interface call
     */
    int32_t GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie);

    int32_t GetNetworkSharingType(std::set<uint32_t>& sharingTypeIsOn);

    int32_t UpdateNetworkSharingType(uint32_t type, bool isOpen);

#ifdef FEATURE_NET_FIREWALL_ENABLE
    /**
     * Set firewall rules to native
     *
     * @param type ip, dns, domain
     * @param ruleList list of NetFirewallIpRule
     * @param isFinish transmit finish or not
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallRules(NetFirewallRuleType type, const std::vector<sptr<NetFirewallBaseRule>> &ruleList,
                             bool isFinish);

    /**
     * Set firewall default action
     *
     * @param userId user id
     * @param inDefault  Default action of NetFirewallRuleDirection:RULE_IN
     * @param outDefault Default action of NetFirewallRuleDirection:RULE_OUT
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallDefaultAction(int32_t userId, FirewallRuleAction inDefault, FirewallRuleAction outDefault);

    /**
     * Set firewall current user id
     *
     * @param userId current user id
     * @return 0 if success or -1 if an error occurred
     */
    int32_t SetFirewallCurrentUserId(int32_t userId);

    /**
     * Clear firewall rules by type
     *
     * @param type type
     * @return 0 if success or -1 if an error occurred
     */
    int32_t ClearFirewallRules(NetFirewallRuleType type);

    /**
     * Register callback for recevie intercept event
     *
     * @param callback implement of INetFirewallCallback
     * @return 0 if success or -1 if an error occurred
     */
    int32_t RegisterNetFirewallCallback(const sptr<NetsysNative::INetFirewallCallback> &callback);

    /**
     * Unregister callback for recevie intercept event
     *
     * @param callback register callback for recevie intercept event
     * @return 0 if success or -1 if an error occurred
     */
    int32_t UnRegisterNetFirewallCallback(const sptr<NetsysNative::INetFirewallCallback> &callback);
#endif

#ifdef FEATURE_WEARABLE_DISTRIBUTED_NET_ENABLE
    int32_t EnableWearableDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId);
    int32_t DisableWearableDistributedNetForward();
#endif

    int32_t RegisterNetsysTrafficCallback(const sptr<NetsysNative::INetsysTrafficCallback> &callback);
    int32_t UnRegisterNetsysTrafficCallback(const sptr<NetsysNative::INetsysTrafficCallback> &callback);

    int32_t SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on);

    int32_t SetEnableIpv6(const std::string &interfaceName, const uint32_t on);

    /**
     * Set the policy to access the network of the specified application.
     *
     * @param uid - The specified UID of application.
     * @param policy - the network access policy of application. For details, see {@link NetworkAccessPolicy}.
     * @param reconfirmFlag true means a reconfirm diaglog trigger while policy deny network access.
     * @return return 0 if OK, return error number if not OK
     */
    int32_t SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag);

    int32_t NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes);
    int32_t DeleteNetworkAccessPolicy(uint32_t uid);

    int32_t StartClat(const std::string &interfaceName, int32_t netId, const std::string &nat64PrefixStr);
    int32_t StopClat(const std::string &interfaceName);
    int32_t ClearFirewallAllRules();
    int32_t CloseSocketsUid(const std::string &ipAddr, uint32_t uid);

    /**
     * Set NIC Traffic allowed or disallowed
     *
     * @param ifaceNames ifaceNames
     * @param status true for allowed, false for disallowed
     * @return Returns 0 success. Otherwise fail, {@link NetPolicyResultCode}.
     */
    int32_t SetNicTrafficAllowed(const std::vector<std::string> &ifaceNames, bool status);

    int32_t SetUserDefinedServerFlag(uint16_t netId, bool isUserDefinedServer);

#ifdef SUPPORT_SYSVPN
    /**
     * process the next vpn stage by SysVpnStageCode
     *
     * @param stage the next vpn stage code
     * @return Returns 0 success. Otherwise fail
     */
    int32_t ProcessVpnStage(NetsysNative::SysVpnStageCode stage, const std::string &message = "");

    /**
     * update vpn interface rules
     *
     * @param netId Network number
     * @param extMessages ext message
     * @param add true add, false remove
     * @return Returns 0, add network ip mark successfully, otherwise it will fail
     */
    int32_t UpdateVpnRules(uint16_t netId, const std::vector<std::string> &extMessages, bool add);
#endif // SUPPORT_SYSVPN
    int32_t SetBrokerUidAccessPolicyMap(const std::unordered_map<uint32_t, uint32_t> &uidMaps);
    int32_t DelBrokerUidAccessPolicyMap(uint32_t uid);
    int32_t FlushDnsCache(uint16_t netId);

private:
    void ProcessDhcpResult(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult);
    void ProcessBandwidthReachedLimit(const std::string &limitName, const std::string &iface);
    sptr<OHOS::NetsysNative::INetsysService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

    void RegisterNotifyCallback();
    void UnRegisterNotifyCallback();

private:
    sptr<OHOS::NetsysNative::INotifyCallback> nativeNotifyCallback_ = nullptr;
    sptr<OHOS::NetsysNative::INetDnsResultCallback> nativeNetDnsResultCallback_ = nullptr;
    uint32_t dnsReportTimeStep = 500;
    sptr<OHOS::NetsysNative::INetsysService> netsysNativeService_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::list<sptr<NetsysControllerCallback>> cbObjects_;
    std::list<sptr<NetsysDnsReportCallback>> cbDnsReportObjects_;
    std::list<sptr<NetsysDnsQueryReportCallback>> cbDnsQueryReportObjects_;
    std::mutex mutex_;
    std::mutex cbObjMutex_;
    std::mutex cbDnsReportObjMutex_;
    std::mutex cbDnsQueryReportObjMutex_;

private:
    class NetNativeConnDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetNativeConnDeathRecipient(std::weak_ptr<NetsysNativeClient> netsysNativeClient)
            : netsysNativeClient_(netsysNativeClient) {}
        ~NetNativeConnDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            auto netsysNativeClient = netsysNativeClient_.lock();
            if (netsysNativeClient == nullptr) {
                return;
            }
            netsysNativeClient->OnRemoteDied(remote);
        }

    private:
        std::weak_ptr<NetsysNativeClient> netsysNativeClient_;
    };
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETSYS_NATIVE_CLIENT_H
