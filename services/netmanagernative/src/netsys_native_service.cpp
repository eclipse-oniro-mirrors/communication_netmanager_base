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

#include <csignal>
#include <thread>
#include <sys/types.h>
#include <unistd.h>
#include "netnative_log_wrapper.h"
#include "system_ability_definition.h"
#include "netsys_native_service.h"

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

    int32_t pid = getpid();
    manager_ = std::make_unique<OHOS::nmd::NetlinkManager>(pid);
    if (manager_ == nullptr) {
        NETNATIVE_LOGE("manager_ is nullptr!");
        return false;
    }
    dhcpController_ = std::make_unique<OHOS::nmd::DhcpController>();

    return true;
}

int32_t NetsysNativeService::SetResolverConfigParcel(const DnsresolverParamsParcel& resolvParams)
{
    NETNATIVE_LOGI("SetResolverConfig retryCount = %{public}d", resolvParams.retryCount_);

    return 0;
}

int32_t NetsysNativeService::SetResolverConfig(const DnsresolverParams &resolvParams)
{
    NETNATIVE_LOGI("SetResolverConfig retryCount = %{public}d", resolvParams.retryCount);
    return 0;
}

int32_t NetsysNativeService::GetResolverConfig(const  uint16_t  netid, std::vector<std::string> &servers,
    std::vector<std::string> &domains, nmd::DnsResParams &param)
{
    NETNATIVE_LOGI("GetResolverConfig netid = %{public}d", netid);
    NETNATIVE_LOGE("NETSYSSERVICE: %{public}d,  %{public}d", param.baseTimeoutMsec,  param.retryCount);
    return 0;
}

int32_t NetsysNativeService::CreateNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("CreateNetworkCache Begin");
    return 0;
}

int32_t NetsysNativeService::FlushNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("FlushNetworkCache Begin");
    return 0;
}

int32_t NetsysNativeService::DestroyNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("DestroyNetworkCache");
    return 0;
}

int32_t NetsysNativeService::Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
    struct addrinfo** result, const uint16_t netid)
{
    NETNATIVE_LOGI("Getaddrinfo");
    return 0;
}

int32_t NetsysNativeService::InterfaceSetMtu(const std::string &interfaceName, int32_t mtu)
{
    NETNATIVE_LOGI("InterfaceSetMtu  Begin");
    return  netsysService_->InterfaceSetMtu(interfaceName, mtu);
}

int32_t NetsysNativeService::InterfaceGetMtu(const std::string &interfaceName)
{
    NETNATIVE_LOGI("InterfaceSetMtu  Begin");
    return  netsysService_->InterfaceGetMtu(interfaceName);
}

int32_t NetsysNativeService::RegisterNotifyCallback(sptr<INotifyCallback> &callback)
{
    NETNATIVE_LOGI("RegisterNotifyCallback");
    notifyCallback_ = callback;
    dhcpController_->RegisterNotifyCallback(callback);
    return 0;
}

int32_t NetsysNativeService::NetworkAddRoute(int32_t netId, const std::string &interfaceName,
    const std::string &destination, const std::string &nextHop)
{
    NETNATIVE_LOGI("NetsysNativeService::NetworkAddRoute unpacket %{public}d %{public}s %{public}s %{public}s",
        netId, interfaceName.c_str(), destination.c_str(), nextHop.c_str());

    int32_t result = this->netsysService_->NetworkAddRoute(netId, interfaceName, destination, nextHop);
    NETNATIVE_LOGI("NetworkAddRoute %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkRemoveRoute(int32_t netId, const std::string &interfaceName,
    const std::string &destination, const std::string &nextHop)
{
    int32_t result = this->netsysService_->NetworkRemoveRoute(netId, interfaceName, destination, nextHop);
    NETNATIVE_LOGI("NetworkRemoveRoute %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkAddRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo)
{
    int32_t result = this->netsysService_->NetworkAddRouteParcel(netId, routeInfo);
    NETNATIVE_LOGI("NetworkAddRouteParcel %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkRemoveRouteParcel(int32_t netId, const RouteInfoParcel &routeInfo)
{
    int32_t result = this->netsysService_->NetworkRemoveRouteParcel(netId, routeInfo);
    NETNATIVE_LOGI("NetworkRemoveRouteParcel %{public}d", result);
    return result;
}

int32_t NetsysNativeService::NetworkSetDefault(int32_t netId)
{
    NETNATIVE_LOG_D("NetworkSetDefault in.");
    int32_t result = this->netsysService_->NetworkSetDefault(netId);
    NETNATIVE_LOG_D("NetworkSetDefault out.");
    return result;
}

int32_t NetsysNativeService::NetworkGetDefault()
{
    int32_t result = this->netsysService_->NetworkGetDefault();
    NETNATIVE_LOGI("NetworkGetDefault");
    return result;
}

int32_t NetsysNativeService::NetworkClearDefault()
{
    int32_t result = this->netsysService_->NetworkClearDefault();
    NETNATIVE_LOGI("NetworkClearDefault");
    return result;
}

int32_t NetsysNativeService::GetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
    const std::string  &parameter, std::string  &value)
{
    int32_t result = this->netsysService_->GetProcSysNet(ipversion,  which,  ifname,  parameter, &value);
    NETNATIVE_LOGI("GetProcSysNet");
    return result;
}

int32_t NetsysNativeService::SetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
    const std::string  &parameter, std::string  &value)
{
    int32_t result = this->netsysService_->SetProcSysNet(ipversion,  which,  ifname,  parameter, value);
    NETNATIVE_LOGI("SetProcSysNet");
    return result;
}

int32_t NetsysNativeService::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    int32_t result = this->netsysService_->NetworkCreatePhysical(netId, permission);
    NETNATIVE_LOGI("NetworkCreatePhysical out.");
    return result;
}

int32_t NetsysNativeService::InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    int32_t result = this->netsysService_->InterfaceAddAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOGI("InterfaceAddAddress");
    return result;
}

int32_t NetsysNativeService::InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    int32_t result = this->netsysService_->InterfaceDelAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOGI("InterfaceDelAddress");
    return result;
}

int32_t NetsysNativeService::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETNATIVE_LOGI("NetworkAddInterface");
    int32_t result = this->netsysService_->NetworkAddInterface(netId, iface);
    return result;
}

int32_t NetsysNativeService::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    int32_t result = this->netsysService_->NetworkRemoveInterface(netId, iface);
    NETNATIVE_LOGI("NetworkRemoveInterface");
    return result;
}

int32_t NetsysNativeService::NetworkDestroy(int32_t netId)
{
    int32_t result = this->netsysService_->NetworkDestroy(netId);
    NETNATIVE_LOGI("NetworkDestroy");
    return result;
}

int32_t NetsysNativeService::GetFwmarkForNetwork(int32_t netId, MarkMaskParcel &markMaskParcel)
{
    markMaskParcel = this->netsysService_->GetFwmarkForNetwork(netId);
    NETNATIVE_LOGI("GetFwmarkForNetwork");
    return ERR_NONE;
}

int32_t NetsysNativeService::InterfaceSetConfig(const InterfaceConfigurationParcel &cfg)
{
    NETNATIVE_LOGI("InterfaceSetConfig");
    this->netsysService_->InterfaceSetConfig(cfg);
    return ERR_NONE;
}

int32_t NetsysNativeService::InterfaceGetConfig(InterfaceConfigurationParcel &cfg)
{
    NETNATIVE_LOGI("InterfaceGetConfig");
    std::string ifName = cfg.ifName;
    cfg = this->netsysService_->InterfaceGetConfig(ifName);
    NETNATIVE_LOGI("InterfaceGetConfig end");
    return ERR_NONE;
}

int32_t NetsysNativeService::InterfaceGetList(std::vector<std::string> &ifaces)
{
    NETNATIVE_LOGI("InterfaceGetList");
    ifaces = this->netsysService_->InterfaceGetList();
    return ERR_NONE;
}

int32_t NetsysNativeService::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOGI("StartDhcpClient");
    this->dhcpController_->StartDhcpClient(iface, bIpv6);
    return ERR_NONE;
}

int32_t NetsysNativeService::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOGI("StopDhcpClient");
    this->dhcpController_->StopDhcpClient(iface, bIpv6);
    return ERR_NONE;
}

int32_t NetsysNativeService::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETNATIVE_LOGI("StartDhcpService");
    this->dhcpController_->StartDhcpService(iface, ipv4addr);
    return ERR_NONE;
}

int32_t NetsysNativeService::StopDhcpService(const std::string &iface)
{
    NETNATIVE_LOGI("StopDhcpService");
    this->dhcpController_->StopDhcpService(iface);
    return ERR_NONE;
}

int32_t NetsysNativeService::IpEnableForwarding(const std::string &requester)
{
    NETNATIVE_LOGI("ipEnableForwarding");
    return this->netsysService_->IpEnableForwarding(requester);
}

int32_t NetsysNativeService::IpDisableForwarding(const std::string &requester)
{
    NETNATIVE_LOGI("ipDisableForwarding");
    return this->netsysService_->IpDisableForwarding(requester);
}

int32_t NetsysNativeService::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETNATIVE_LOGI("enableNat");
    return this->netsysService_->EnableNat(downstreamIface, upstreamIface);
}

int32_t NetsysNativeService::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETNATIVE_LOGI("disableNat");
    return this->netsysService_->DisableNat(downstreamIface, upstreamIface);
}

int32_t NetsysNativeService::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    NETNATIVE_LOGI("ipfwdAddInterfaceForward");
    return this->netsysService_->IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetsysNativeService::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    NETNATIVE_LOGI("ipfwdRemoveInterfaceForward");
    return this->netsysService_->IpfwdRemoveInterfaceForward(fromIface, toIface);
}
} // namespace NetsysNative
} // namespace OHOS
