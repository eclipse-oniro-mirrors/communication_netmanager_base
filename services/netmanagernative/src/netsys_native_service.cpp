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

#include "netsys_native_service.h"

#include <csignal>
#include <thread>
#include <unistd.h>
#include <sys/types.h>

#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "system_ability_definition.h"

using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace NetsysNative {
REGISTER_SYSTEM_ABILITY_BY_ID(NetsysNativeService, COMM_NETSYS_NATIVE_SYS_ABILITY_ID, true)

NetsysNativeService::NetsysNativeService()
    : SystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, true), netsysService_(nullptr),
    manager_(nullptr), notifyCallback_(nullptr)
{
}

void NetsysNativeService::OnStart()
{
    NETNATIVE_LOGI("NetsysNativeService::OnStart Begin");
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
    struct tm *timeNow;
    time_t second = time(0);
    if (second < 0) {
        return;
    }
    timeNow = localtime(&second);
    if (timeNow != nullptr) {
        NETNATIVE_LOGI(
            "NetsysNativeService start time:%{public}d-%{public}d-%{public}d %{public}d:%{public}d:%{public}d",
            timeNow->tm_year + startTime_, timeNow->tm_mon + extraMonth_, timeNow->tm_mday, timeNow->tm_hour,
            timeNow->tm_min, timeNow->tm_sec);
    }
    manager_->StartListener();
}

void NetsysNativeService::OnStop()
{
    std::lock_guard<std::mutex> guard(instanceLock_);
    struct tm *timeNow;
    time_t second = time(0);
    if (second < 0) {
        return;
    }
    timeNow = localtime(&second);
    if (timeNow != nullptr) {
        NETNATIVE_LOGI(
            "NetsysNativeService dump time:%{public}d-%{public}d-%{public}d %{public}d:%{public}d:%{public}d",
            timeNow->tm_year + startTime_, timeNow->tm_mon + extraMonth_, timeNow->tm_mday, timeNow->tm_hour,
            timeNow->tm_min, timeNow->tm_sec);
    }
    state_ = ServiceRunningState::STATE_STOPPED;
    manager_->StopListener();
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
    exit(1);
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
    dhcpController_ = std::make_unique<OHOS::nmd::DhcpController>();
    fwmarkNetwork_ = std::make_unique<OHOS::nmd::FwmarkNetwork>();

    return true;
}

int32_t NetsysNativeService::SetResolverConfigParcel(const DnsResolverParamsParcel & resolvParams)
{
    NETNATIVE_LOG_D("SetResolverConfig retryCount = %{public}d", resolvParams.retryCount_);

    return 0;
}

int32_t NetsysNativeService::SetResolverConfig(uint16_t netId,
                                               uint16_t baseTimeoutMsec,
                                               uint8_t retryCount,
                                               const std::vector<std::string> &servers,
                                               const std::vector<std::string> &domains)
{
    netsysService_->DnsSetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    return 0;
}

int32_t NetsysNativeService::GetResolverConfig(const uint16_t netid,
                                               std::vector<std::string> &servers,
                                               std::vector<std::string> &domains,
                                               uint16_t &baseTimeoutMsec,
                                               uint8_t &retryCount)
{
    NETNATIVE_LOG_D("GetResolverConfig netid = %{public}d", netid);
    netsysService_->DnsGetResolverConfig(netid, servers, domains, baseTimeoutMsec, retryCount);
    return 0;
}

int32_t NetsysNativeService::CreateNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOG_D("CreateNetworkCache Begin");
    netsysService_->DnsCreateNetworkCache(netid);

    return 0;
}

int32_t NetsysNativeService::FlushNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOG_D("FlushNetworkCache Begin");
    return 0;
}

int32_t NetsysNativeService::DestroyNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOG_D("DestroyNetworkCache");
    return 0;
}

int32_t NetsysNativeService::Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
    struct addrinfo** result, const uint16_t netid)
{
    NETNATIVE_LOG_D("Getaddrinfo");
    return 0;
}

int32_t NetsysNativeService::InterfaceSetMtu(const std::string &interfaceName, int32_t mtu)
{
    NETNATIVE_LOG_D("InterfaceSetMtu  Begin");
    return  netsysService_->InterfaceSetMtu(interfaceName, mtu);
}

int32_t NetsysNativeService::InterfaceGetMtu(const std::string &interfaceName)
{
    NETNATIVE_LOG_D("InterfaceSetMtu  Begin");
    return  netsysService_->InterfaceGetMtu(interfaceName);
}

int32_t NetsysNativeService::RegisterNotifyCallback(sptr<INotifyCallback> &callback)
{
    NETNATIVE_LOG_D("RegisterNotifyCallback");
    notifyCallback_ = callback;
    dhcpController_->RegisterNotifyCallback(callback);
    manager_-> RegisterNetlinkCallback(callback);
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
    NETNATIVE_LOG_D("NetsysNativeService::NetworkAddRoute unpacket %{public}d %{public}s %{public}s %{public}s",
        netId, interfaceName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());

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

int32_t NetsysNativeService::GetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
    const std::string  &parameter, std::string  &value)
{
    int32_t result = netsysService_->GetProcSysNet(ipversion,  which,  ifname,  parameter, &value);
    NETNATIVE_LOG_D("GetProcSysNet");
    return result;
}

int32_t NetsysNativeService::SetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
    const std::string  &parameter, std::string  &value)
{
    int32_t result = netsysService_->SetProcSysNet(ipversion,  which,  ifname,  parameter, value);
    NETNATIVE_LOG_D("SetProcSysNet");
    return result;
}

int32_t NetsysNativeService::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    int32_t result = netsysService_->NetworkCreatePhysical(netId, permission);
    NETNATIVE_LOG_D("NetworkCreatePhysical out.");
    return result;
}

int32_t NetsysNativeService::InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    int32_t result = netsysService_->InterfaceAddAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOG_D("InterfaceAddAddress");
    return result;
}

int32_t NetsysNativeService::InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    int32_t result = netsysService_->InterfaceDelAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOG_D("InterfaceDelAddress");
    return result;
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

int32_t NetsysNativeService::InterfaceSetConfig(const InterfaceConfigurationParcel &cfg)
{
    NETNATIVE_LOG_D("InterfaceSetConfig");
    netsysService_->InterfaceSetConfig(cfg);
    return ERR_NONE;
}

int32_t NetsysNativeService::InterfaceGetConfig(InterfaceConfigurationParcel &cfg)
{
    NETNATIVE_LOG_D("InterfaceGetConfig");
    std::string ifName = cfg.ifName;
    cfg = netsysService_->InterfaceGetConfig(ifName);
    NETNATIVE_LOG_D("InterfaceGetConfig end");
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
    dhcpController_->StartDhcpClient(iface, bIpv6);
    return ERR_NONE;
}

int32_t NetsysNativeService::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOG_D("StopDhcpClient");
    dhcpController_->StopDhcpClient(iface, bIpv6);
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

int32_t NetsysNativeService::FirewallSetUidRule(uint32_t chain, uint32_t uid, uint32_t firewallRule)
{
    NETNATIVE_LOG_D("firewallSetUidRule");
    return netsysService_->FirewallSetUidRule(chain, uid, firewallRule);
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
} // namespace NetsysNative
} // namespace OHOS
