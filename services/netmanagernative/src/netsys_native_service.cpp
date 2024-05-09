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

#include <csignal>
#include <sys/types.h>
#include <regex>
#include <thread>
#include <unistd.h>

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bpf_loader.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "netsys_native_service.h"
#include "bpf_ring_buffer.h"
#include "parameters.h"

using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace NetsysNative {
static constexpr const char *BFP_NAME_NETSYS_PATH = "/system/etc/bpf/netsys.o";
const std::regex REGEX_CMD_IPTABLES(std::string(R"(^-[\S]*[\s\S]*)"));
const std::string DEVICETYPE_KEY = "const.product.devicetype";
const std::string PHONE_TYPE = "phone";

REGISTER_SYSTEM_ABILITY_BY_ID(NetsysNativeService, COMM_NETSYS_NATIVE_SYS_ABILITY_ID, true)

NetsysNativeService::NetsysNativeService()
    : SystemAbility(COMM_NETSYS_NATIVE_SYS_ABILITY_ID, true),
      netsysService_(nullptr),
      manager_(nullptr),
      notifyCallback_(nullptr)
{
}

void NetsysNativeService::OnStart()
{
    NETNATIVE_LOGI("OnStart Begin");
    std::lock_guard<std::mutex> guard(instanceLock_);
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        return;
    }

    if (!Init()) {
        NETNATIVE_LOGE("NetsysNativeService init failed!");
        return;
    }
    bool res = SystemAbility::Publish(this);
    if (!res) {
        NETNATIVE_LOGE("publishing NetsysNativeService to sa manager failed!");
        return;
    }
    NETNATIVE_LOGI("Publish NetsysNativeService SUCCESS");
    state_ = ServiceRunningState::STATE_RUNNING;
    NETNATIVE_LOGI("start listener");
    manager_->StartListener();
    NETNATIVE_LOGI("start listener end on start end");
}

void NetsysNativeService::OnStop()
{
    std::lock_guard<std::mutex> guard(instanceLock_);
    state_ = ServiceRunningState::STATE_STOPPED;
    NETNATIVE_LOGI("stop listener");
    manager_->StopListener();
    NETNATIVE_LOGI("stop listener end on stop end");
    NetsysBpfRingBuffer::ExistRingBufferPoll();
}

int32_t NetsysNativeService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETNATIVE_LOG_D("Start Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? SESSION_UNOPEN_ERR : ERR_NONE;
}

void NetsysNativeService::GetDumpMessage(std::string &message)
{
    netsysService_->GetDumpInfo(message);
}

void ExitHandler(int32_t signum)
{
    (void)signum;
    _Exit(1);
}

bool NetsysNativeService::Init()
{
    (void)signal(SIGTERM, ExitHandler);
    (void)signal(SIGABRT, ExitHandler);

    netsysService_ = std::make_unique<nmd::NetManagerNative>();
    if (netsysService_ == nullptr) {
        NETNATIVE_LOGE("netsysService_ is nullptr!");
        return false;
    }
    netsysService_->Init();

    manager_ = std::make_unique<OHOS::nmd::NetlinkManager>();
    if (manager_ == nullptr) {
        NETNATIVE_LOGE("manager_ is nullptr!");
        return false;
    }
    bpfStats_ = std::make_unique<OHOS::NetManagerStandard::NetsysBpfStats>();
    dhcpController_ = std::make_unique<OHOS::nmd::DhcpController>();
    fwmarkNetwork_ = std::make_unique<OHOS::nmd::FwmarkNetwork>();
    sharingManager_ = std::make_unique<SharingManager>();
    iptablesWrapper_ = IptablesWrapper::GetInstance();
    netDiagWrapper = NetDiagWrapper::GetInstance();

    auto ret = OHOS::NetManagerStandard::LoadElf(BFP_NAME_NETSYS_PATH);
    NETNATIVE_LOGI("LoadElf is %{public}d", ret);

    if (OHOS::system::GetParameter(DEVICETYPE_KEY, "") == PHONE_TYPE) {
        NetsysBpfRingBuffer::ListenNetworkAccessPolicyEvent();
    }
    AddSystemAbilityListener(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    return true;
}

void NetsysNativeService::OnNetManagerRestart()
{
    NETNATIVE_LOGI("OnNetManagerRestart");
    if (netsysService_ != nullptr) {
        netsysService_->NetworkReinitRoute();
    }
    if (manager_ != nullptr && notifyCallback_ != nullptr) {
        manager_->UnregisterNetlinkCallback(notifyCallback_);
    }
}

int32_t NetsysNativeService::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                               const std::vector<std::string> &servers,
                                               const std::vector<std::string> &domains)
{
    netsysService_->DnsSetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    return 0;
}

int32_t NetsysNativeService::GetResolverConfig(uint16_t netid, std::vector<std::string> &servers,
                                               std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                               uint8_t &retryCount)
{
    NETNATIVE_LOG_D("GetResolverConfig netid = %{public}d", netid);
    netsysService_->DnsGetResolverConfig(netid, servers, domains, baseTimeoutMsec, retryCount);
    return 0;
}

int32_t NetsysNativeService::CreateNetworkCache(uint16_t netid)
{
    NETNATIVE_LOG_D("CreateNetworkCache Begin");
    netsysService_->DnsCreateNetworkCache(netid);

    return 0;
}

int32_t NetsysNativeService::DestroyNetworkCache(uint16_t netId)
{
    NETNATIVE_LOG_D("DestroyNetworkCache");
    return netsysService_->DnsDestroyNetworkCache(netId);
}

int32_t NetsysNativeService::GetAddrInfo(const std::string &hostName, const std::string &serverName,
                                         const AddrInfo &hints, uint16_t netId, std::vector<AddrInfo> &res)
{
    return netsysService_->DnsGetAddrInfo(hostName, serverName, hints, netId, res);
}

int32_t NetsysNativeService::SetInterfaceMtu(const std::string &interfaceName, int32_t mtu)
{
    NETNATIVE_LOG_D("SetInterfaceMtu  Begin");
    return netsysService_->SetInterfaceMtu(interfaceName, mtu);
}

int32_t NetsysNativeService::GetInterfaceMtu(const std::string &interfaceName)
{
    NETNATIVE_LOG_D("SetInterfaceMtu  Begin");
    return netsysService_->GetInterfaceMtu(interfaceName);
}

int32_t NetsysNativeService::SetTcpBufferSizes(const std::string &tcpBufferSizes)
{
    NETNATIVE_LOG_D("SetTcpBufferSizes  Begin");
    return netsysService_->SetTcpBufferSizes(tcpBufferSizes);
}

int32_t NetsysNativeService::RegisterNotifyCallback(sptr<INotifyCallback> &callback)
{
    NETNATIVE_LOG_D("RegisterNotifyCallback");
    notifyCallback_ = callback;
    dhcpController_->RegisterNotifyCallback(callback);
    manager_->RegisterNetlinkCallback(callback);
    return 0;
}

int32_t NetsysNativeService::UnRegisterNotifyCallback(sptr<INotifyCallback> &callback)
{
    NETNATIVE_LOGI("UnRegisterNotifyCallback");
    manager_->UnregisterNetlinkCallback(notifyCallback_);
    return 0;
}

int32_t NetsysNativeService::NetworkAddRoute(int32_t netId, const std::string &interfaceName,
                                             const std::string &destination, const std::string &nextHop)
{
    NETNATIVE_LOG_D("NetworkAddRoute unpacket %{public}d %{public}s %{public}s %{public}s", netId,
                    interfaceName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());

    int32_t result = netsysService_->NetworkAddRoute(netId, interfaceName, destination, nextHop);
    NETNATIVE_LOG_D("NetworkAddRoute %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkRemoveRoute(int32_t netId, const std::string &interfaceName,
                                                const std::string &destination, const std::string &nextHop)
{
    int32_t result = netsysService_->NetworkRemoveRoute(netId, interfaceName, destination, nextHop);
    NETNATIVE_LOG_D("NetworkRemoveRoute %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkAddRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo)
{
    int32_t result = netsysService_->NetworkAddRouteParcel(netId, routeInfo);
    NETNATIVE_LOG_D("NetworkAddRouteParcel %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkRemoveRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo)
{
    int32_t result = netsysService_->NetworkRemoveRouteParcel(netId, routeInfo);
    NETNATIVE_LOG_D("NetworkRemoveRouteParcel %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkSetDefault(int32_t netId)
{
    NETNATIVE_LOG_D("NetworkSetDefault in.");
    int32_t result = netsysService_->NetworkSetDefault(netId);
    NETNATIVE_LOG_D("NetworkSetDefault out.");
    return result;
}

int32_t NetsysNativeService::NetworkGetDefault()
{
    int32_t result = netsysService_->NetworkGetDefault();
    NETNATIVE_LOG_D("NetworkGetDefault");
    return result;
}

int32_t NetsysNativeService::NetworkClearDefault()
{
    int32_t result = netsysService_->NetworkClearDefault();
    NETNATIVE_LOG_D("NetworkClearDefault");
    return result;
}

int32_t NetsysNativeService::GetProcSysNet(int32_t family, int32_t which, const std::string &ifname,
                                           const std::string &parameter, std::string &value)
{
    int32_t result = netsysService_->GetProcSysNet(family, which, ifname, parameter, &value);
    NETNATIVE_LOG_D("GetProcSysNet");
    return result;
}

int32_t NetsysNativeService::SetProcSysNet(int32_t family, int32_t which, const std::string &ifname,
                                           const std::string &parameter, std::string &value)
{
    int32_t result = netsysService_->SetProcSysNet(family, which, ifname, parameter, value);
    NETNATIVE_LOG_D("SetProcSysNet");
    return result;
}

int32_t NetsysNativeService::SetInternetPermission(uint32_t uid, uint8_t allow, uint8_t isBroker)
{
    int32_t result = netsysService_->SetInternetPermission(uid, allow, isBroker);
    NETNATIVE_LOG_D("SetInternetPermission out.");
    return result;
}

int32_t NetsysNativeService::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    int32_t result = netsysService_->NetworkCreatePhysical(netId, permission);
    NETNATIVE_LOG_D("NetworkCreatePhysical out.");
    return result;
}

int32_t NetsysNativeService::NetworkCreateVirtual(int32_t netId, bool hasDns)
{
    int32_t result = netsysService_->NetworkCreateVirtual(netId, hasDns);
    NETNATIVE_LOG_D("NetworkCreateVirtual out.");
    return result;
}

int32_t NetsysNativeService::NetworkAddUids(int32_t netId, const std::vector<UidRange> &uidRanges)
{
    int32_t result = netsysService_->NetworkAddUids(netId, uidRanges);
    NETNATIVE_LOG_D("NetworkAddUids out.");
    return result;
}

int32_t NetsysNativeService::NetworkDelUids(int32_t netId, const std::vector<UidRange> &uidRanges)
{
    int32_t result = netsysService_->NetworkDelUids(netId, uidRanges);
    NETNATIVE_LOG_D("NetworkDelUids out.");
    return result;
}

int32_t NetsysNativeService::AddInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                                 int32_t prefixLength)
{
    int32_t result = netsysService_->AddInterfaceAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOG_D("AddInterfaceAddress");
    return result;
}

int32_t NetsysNativeService::DelInterfaceAddress(const std::string &interfaceName, const std::string &addrString,
                                                 int32_t prefixLength)
{
    int32_t result = netsysService_->DelInterfaceAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOG_D("DelInterfaceAddress");
    return result;
}

int32_t NetsysNativeService::InterfaceSetIpAddress(const std::string &ifaceName, const std::string &ipAddress)
{
    NETNATIVE_LOG_D("InterfaceSetIpAddress");
    return netsysService_->InterfaceSetIpAddress(ifaceName, ipAddress);
}

int32_t NetsysNativeService::InterfaceSetIffUp(const std::string &ifaceName)
{
    NETNATIVE_LOG_D("InterfaceSetIffUp");
    return netsysService_->InterfaceSetIffUp(ifaceName);
}

int32_t NetsysNativeService::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETNATIVE_LOG_D("NetworkAddInterface");
    int32_t result = netsysService_->NetworkAddInterface(netId, iface);
    return result;
}

int32_t NetsysNativeService::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    int32_t result = netsysService_->NetworkRemoveInterface(netId, iface);
    NETNATIVE_LOG_D("NetworkRemoveInterface");
    return result;
}

int32_t NetsysNativeService::NetworkDestroy(int32_t netId)
{
    int32_t result = netsysService_->NetworkDestroy(netId);
    NETNATIVE_LOG_D("NetworkDestroy");
    return result;
}

int32_t NetsysNativeService::GetFwmarkForNetwork(int32_t netId, MarkMaskParcel &markMaskParcel)
{
    markMaskParcel = netsysService_->GetFwmarkForNetwork(netId);
    NETNATIVE_LOG_D("GetFwmarkForNetwork");
    return ERR_NONE;
}

int32_t NetsysNativeService::SetInterfaceConfig(const InterfaceConfigurationParcel &cfg)
{
    NETNATIVE_LOG_D("SetInterfaceConfig");
    netsysService_->SetInterfaceConfig(cfg);
    return ERR_NONE;
}

int32_t NetsysNativeService::GetInterfaceConfig(InterfaceConfigurationParcel &cfg)
{
    NETNATIVE_LOG_D("GetInterfaceConfig");
    std::string ifName = cfg.ifName;
    cfg = netsysService_->GetInterfaceConfig(ifName);
    NETNATIVE_LOG_D("GetInterfaceConfig end");
    return ERR_NONE;
}

int32_t NetsysNativeService::InterfaceGetList(std::vector<std::string> &ifaces)
{
    NETNATIVE_LOG_D("InterfaceGetList");
    ifaces = netsysService_->InterfaceGetList();
    return ERR_NONE;
}

int32_t NetsysNativeService::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOG_D("StartDhcpClient");
    dhcpController_->StartClient(iface, bIpv6);
    return ERR_NONE;
}

int32_t NetsysNativeService::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOG_D("StopDhcpClient");
    dhcpController_->StopClient(iface, bIpv6);
    return ERR_NONE;
}

int32_t NetsysNativeService::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETNATIVE_LOG_D("StartDhcpService");
    dhcpController_->StartDhcpService(iface, ipv4addr);
    return ERR_NONE;
}

int32_t NetsysNativeService::StopDhcpService(const std::string &iface)
{
    NETNATIVE_LOG_D("StopDhcpService");
    dhcpController_->StopDhcpService(iface);
    return ERR_NONE;
}

int32_t NetsysNativeService::IpEnableForwarding(const std::string &requester)
{
    NETNATIVE_LOG_D("ipEnableForwarding");
    return netsysService_->IpEnableForwarding(requester);
}

int32_t NetsysNativeService::IpDisableForwarding(const std::string &requester)
{
    NETNATIVE_LOG_D("ipDisableForwarding");
    return netsysService_->IpDisableForwarding(requester);
}

int32_t NetsysNativeService::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETNATIVE_LOG_D("enableNat");
    return netsysService_->EnableNat(downstreamIface, upstreamIface);
}

int32_t NetsysNativeService::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETNATIVE_LOG_D("disableNat");
    return netsysService_->DisableNat(downstreamIface, upstreamIface);
}

int32_t NetsysNativeService::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    NETNATIVE_LOG_D("ipfwdAddInterfaceForward");
    return netsysService_->IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetsysNativeService::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    NETNATIVE_LOG_D("ipfwdRemoveInterfaceForward");
    return netsysService_->IpfwdRemoveInterfaceForward(fromIface, toIface);
}

int32_t NetsysNativeService::BandwidthEnableDataSaver(bool enable)
{
    NETNATIVE_LOG_D("bandwidthEnableDataSaver");
    return netsysService_->BandwidthEnableDataSaver(enable);
}

int32_t NetsysNativeService::BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes)
{
    NETNATIVE_LOG_D("BandwidthSetIfaceQuota");
    return netsysService_->BandwidthSetIfaceQuota(ifName, bytes);
}

int32_t NetsysNativeService::BandwidthRemoveIfaceQuota(const std::string &ifName)
{
    NETNATIVE_LOG_D("BandwidthRemoveIfaceQuota");
    return netsysService_->BandwidthRemoveIfaceQuota(ifName);
}

int32_t NetsysNativeService::BandwidthAddDeniedList(uint32_t uid)
{
    NETNATIVE_LOG_D("BandwidthAddDeniedList");
    return netsysService_->BandwidthAddDeniedList(uid);
}

int32_t NetsysNativeService::BandwidthRemoveDeniedList(uint32_t uid)
{
    NETNATIVE_LOG_D("BandwidthRemoveDeniedList");
    return netsysService_->BandwidthRemoveDeniedList(uid);
}

int32_t NetsysNativeService::BandwidthAddAllowedList(uint32_t uid)
{
    NETNATIVE_LOG_D("BandwidthAddAllowedList");
    return netsysService_->BandwidthAddAllowedList(uid);
}

int32_t NetsysNativeService::BandwidthRemoveAllowedList(uint32_t uid)
{
    NETNATIVE_LOG_D("BandwidthRemoveAllowedList");
    return netsysService_->BandwidthRemoveAllowedList(uid);
}

int32_t NetsysNativeService::FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    NETNATIVE_LOG_D("FirewallSetUidsAllowedListChain");
    return netsysService_->FirewallSetUidsAllowedListChain(chain, uids);
}

int32_t NetsysNativeService::FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    NETNATIVE_LOG_D("FirewallSetUidsDeniedListChain");
    return netsysService_->FirewallSetUidsDeniedListChain(chain, uids);
}

int32_t NetsysNativeService::FirewallEnableChain(uint32_t chain, bool enable)
{
    NETNATIVE_LOG_D("FirewallEnableChain");
    return netsysService_->FirewallEnableChain(chain, enable);
}

int32_t NetsysNativeService::FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids,
                                                uint32_t firewallRule)
{
    NETNATIVE_LOG_D("firewallSetUidRule");
    return netsysService_->FirewallSetUidRule(chain, uids, firewallRule);
}

int32_t NetsysNativeService::ShareDnsSet(uint16_t netid)
{
    NETNATIVE_LOG_D("NetsysNativeService ShareDnsSet");
    if (netsysService_ == nullptr) {
        NETNATIVE_LOGE("netsysService_ is null");
        return -1;
    }
    netsysService_->ShareDnsSet(netid);
    return ERR_NONE;
}

int32_t NetsysNativeService::StartDnsProxyListen()
{
    NETNATIVE_LOG_D("NetsysNativeService StartDnsProxyListen");
    if (netsysService_ == nullptr) {
        NETNATIVE_LOGE("netsysService_ is null");
        return -1;
    }
    netsysService_->StartDnsProxyListen();
    return ERR_NONE;
}

int32_t NetsysNativeService::StopDnsProxyListen()
{
    NETNATIVE_LOG_D("NetsysNativeService StopDnsProxyListen");
    if (netsysService_ == nullptr) {
        NETNATIVE_LOGE("netsysService_ is null");
        return -1;
    }
    netsysService_->StopDnsProxyListen();
    return ERR_NONE;
}

int32_t NetsysNativeService::GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                                      NetworkSharingTraffic &traffic)
{
    if (sharingManager_ == nullptr) {
        NETNATIVE_LOGE("manager is null.");
        return NetManagerStandard::NETMANAGER_ERROR;
    }
    return sharingManager_->GetNetworkSharingTraffic(downIface, upIface, traffic);
}

void NetsysNativeService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETNATIVE_LOGI("OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NET_CONN_MANAGER_SYS_ABILITY_ID) {
        if (!hasSARemoved_) {
            hasSARemoved_ = true;
            return;
        }
        OnNetManagerRestart();
    }
}

void NetsysNativeService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETNATIVE_LOGI("OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NET_CONN_MANAGER_SYS_ABILITY_ID) {
        OnNetManagerRestart();
        hasSARemoved_ = true;
    }
}

int32_t NetsysNativeService::GetTotalStats(uint64_t &stats, uint32_t type)
{
    if (bpfStats_ == nullptr) {
        NETNATIVE_LOGE("bpfStats is null.");
        return NetManagerStandard::NETMANAGER_ERROR;
    }

    return bpfStats_->GetTotalStats(stats, static_cast<OHOS::NetManagerStandard::StatsType>(type));
}

int32_t NetsysNativeService::GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid)
{
    if (bpfStats_ == nullptr) {
        NETNATIVE_LOGE("bpfStats is null.");
        return NetManagerStandard::NETMANAGER_ERROR;
    }

    return bpfStats_->GetUidStats(stats, static_cast<OHOS::NetManagerStandard::StatsType>(type), uid);
}

int32_t NetsysNativeService::GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName)
{
    if (bpfStats_ == nullptr) {
        NETNATIVE_LOGE("bpfStats is null.");
        return NetManagerStandard::NETMANAGER_ERROR;
    }

    return bpfStats_->GetIfaceStats(stats, static_cast<OHOS::NetManagerStandard::StatsType>(type), interfaceName);
}

int32_t NetsysNativeService::GetAllContainerStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    if (bpfStats_ == nullptr) {
        NETNATIVE_LOGE("bpfStats is null.");
        return NetManagerStandard::NETMANAGER_ERROR;
    }
    return bpfStats_->GetAllContainerStatsInfo(stats);
}

int32_t NetsysNativeService::GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    if (bpfStats_ == nullptr) {
        NETNATIVE_LOGE("bpfStats is null.");
        return NetManagerStandard::NETMANAGER_ERROR;
    }

    return bpfStats_->GetAllStatsInfo(stats);
}

int32_t NetsysNativeService::SetIptablesCommandForRes(const std::string &cmd, std::string &respond)
{
    if (!regex_match(cmd, REGEX_CMD_IPTABLES)) {
        NETNATIVE_LOGE("IptablesWrapper command format is invalid");
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }
    if (iptablesWrapper_ == nullptr) {
        NETNATIVE_LOGE("SetIptablesCommandForRes iptablesWrapper_ is null");
        return NetManagerStandard::NETMANAGER_ERROR;
    }
    respond = iptablesWrapper_->RunCommandForRes(IPTYPE_IPV4V6, cmd);
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetsysNativeService::NetDiagPingHost(const NetDiagPingOption &pingOption,
                                             const sptr<INetDiagCallback> &callback)
{
    if (netDiagWrapper == nullptr) {
        NETNATIVE_LOGE("netDiagWrapper is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netDiagWrapper->PingHost(pingOption, callback);
}

int32_t NetsysNativeService::NetDiagGetRouteTable(std::list<NetDiagRouteTable> &routeTables)
{
    if (netDiagWrapper == nullptr) {
        NETNATIVE_LOGE("netDiagWrapper is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netDiagWrapper->GetRouteTable(routeTables);
}

int32_t NetsysNativeService::NetDiagGetSocketsInfo(NetDiagProtocolType socketType, NetDiagSocketsInfo &socketsInfo)
{
    if (netDiagWrapper == nullptr) {
        NETNATIVE_LOGE("netDiagWrapper is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netDiagWrapper->GetSocketsInfo(socketType, socketsInfo);
}

int32_t NetsysNativeService::NetDiagGetInterfaceConfig(std::list<NetDiagIfaceConfig> &configs,
                                                       const std::string &ifaceName)
{
    if (netDiagWrapper == nullptr) {
        NETNATIVE_LOGE("netDiagWrapper is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netDiagWrapper->GetInterfaceConfig(configs, ifaceName);
}

int32_t NetsysNativeService::NetDiagUpdateInterfaceConfig(const NetDiagIfaceConfig &config,
                                                          const std::string &ifaceName, bool add)
{
    if (netDiagWrapper == nullptr) {
        NETNATIVE_LOGE("netDiagWrapper is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netDiagWrapper->UpdateInterfaceConfig(config, ifaceName, add);
}

int32_t NetsysNativeService::NetDiagSetInterfaceActiveState(const std::string &ifaceName, bool up)
{
    if (netDiagWrapper == nullptr) {
        NETNATIVE_LOGE("netDiagWrapper is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netDiagWrapper->SetInterfaceActiveState(ifaceName, up);
}

int32_t NetsysNativeService::AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                          const std::string &ifName)
{
    NETNATIVE_LOG_D("AddStaticArp");
    if (netsysService_ == nullptr) {
        NETNATIVE_LOGE("netsysService_ is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netsysService_->AddStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetsysNativeService::DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                          const std::string &ifName)
{
    NETNATIVE_LOG_D("DelStaticArp");
    if (netsysService_ == nullptr) {
        NETNATIVE_LOGE("netsysService_ is null");
        return NetManagerStandard::NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netsysService_->DelStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetsysNativeService::RegisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback, uint32_t timeStep)
{
    return netsysService_->RegisterDnsResultCallback(callback, timeStep);
}

int32_t NetsysNativeService::UnregisterDnsResultCallback(const sptr<INetDnsResultCallback> &callback)
{
    return netsysService_->UnregisterDnsResultCallback(callback);
}

int32_t NetsysNativeService::RegisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback)
{
    return netsysService_->RegisterDnsHealthCallback(callback);
}

int32_t NetsysNativeService::UnregisterDnsHealthCallback(const sptr<INetDnsHealthCallback> &callback)
{
    return netsysService_->UnregisterDnsHealthCallback(callback);
}

int32_t NetsysNativeService::SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on)
{
    int32_t result = netsysService_->SetIpv6PrivacyExtensions(interfaceName, on);
    NETNATIVE_LOG_D("SetIpv6PrivacyExtensions");
    return result;
}
int32_t NetsysNativeService::SetEnableIpv6(const std::string &interfaceName, const uint32_t on)
{
    int32_t result = netsysService_->SetEnableIpv6(interfaceName, on);
    NETNATIVE_LOG_D("SetEnableIpv6");
    return result;
}

int32_t NetsysNativeService::GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie)
{
    if (bpfStats_ == nullptr) {
        NETNATIVE_LOGE("bpfStats is null.");
        return NetManagerStandard::NETMANAGER_ERROR;
    }

    return bpfStats_->GetCookieStats(stats, static_cast<OHOS::NetManagerStandard::StatsType>(type), cookie);
}

int32_t NetsysNativeService::GetNetworkSharingType(std::set<uint32_t>& sharingTypeIsOn)
{
    NETNATIVE_LOGI("GetNetworkSharingType");
    std::lock_guard<std::mutex> guard(instanceLock_);
    sharingTypeIsOn = sharingTypeIsOn_;
    return NETSYS_SUCCESS;
}

int32_t NetsysNativeService::UpdateNetworkSharingType(uint32_t type, bool isOpen)
{
    NETNATIVE_LOGI("UpdateNetworkSharingType");
    std::lock_guard<std::mutex> guard(instanceLock_);
    if (isOpen) {
        sharingTypeIsOn_.insert(type);
    } else {
        sharingTypeIsOn_.erase(type);
    }
    return NETSYS_SUCCESS;
}

int32_t NetsysNativeService::SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag)
{
    NETNATIVE_LOGI("SetNetworkAccessPolicy");

    return netsysService_->SetNetworkAccessPolicy(uid, policy, reconfirmFlag);
}

int32_t NetsysNativeService::DeleteNetworkAccessPolicy(uint32_t uid)
{
    NETNATIVE_LOGI("DeleteNetworkAccessPolicy");
    return netsysService_->DeleteNetworkAccessPolicy(uid);
}

int32_t NetsysNativeService::NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes)
{
    NETNATIVE_LOGI("NotifyNetBearerTypeChange");
    return netsysService_->NotifyNetBearerTypeChange(bearerTypes);
}
} // namespace NetsysNative
} // namespace OHOS
