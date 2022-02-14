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
#include <cstdio>
#include <thread>
#include "netnative_log_wrapper.h"
#include "system_ability_definition.h"
#include "netd_native_service.h"

namespace OHOS {
namespace NetdNative {
using namespace std;

REGISTER_SYSTEM_ABILITY_BY_ID(NetdNativeService, COMM_NETD_NATIVE_SYS_ABILITY_ID, true);

NetdNativeService::NetdNativeService()
    : SystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, true), netdService_(nullptr),
    manager_(nullptr), notifyCallback_(nullptr)
{
}

void NetdNativeService::OnStart()
{
    NETNATIVE_LOGI("NetdNativeService::OnStart Begin");
    std::lock_guard<std::mutex> guard(instanceLock_);
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        return;
    }

    if (!Init()) {
        NETNATIVE_LOGE("NetdNativeService init failed!");
        return;
    }
    bool res = SystemAbility::Publish(this);
    if (!res) {
        NETNATIVE_LOGE("publishing NetdNativeService to sa manager failed!");
        return;
    }
    NETNATIVE_LOGI("Publish NetdNativeService SUCCESS");
    state_ = ServiceRunningState::STATE_RUNNING;
    struct tm *timeNow;
    time_t second = time(0);
    if (second < 0) {
        return;
    }
    timeNow = localtime(&second);
    if (timeNow != nullptr) {
        NETNATIVE_LOGI(
            "NetdNativeService start time:%{public}d-%{public}d-%{public}d %{public}d:%{public}d:%{public}d",
            timeNow->tm_year + startTime_, timeNow->tm_mon + extraMonth_, timeNow->tm_mday, timeNow->tm_hour,
            timeNow->tm_min, timeNow->tm_sec);
    }
}

void NetdNativeService::OnStop()
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
            "NetdNativeService dump time:%{public}d-%{public}d-%{public}d %{public}d:%{public}d:%{public}d",
            timeNow->tm_year + startTime_, timeNow->tm_mon + extraMonth_, timeNow->tm_mday, timeNow->tm_hour,
            timeNow->tm_min, timeNow->tm_sec);
    }
    state_ = ServiceRunningState::STATE_STOPPED;
}

void ExitHandler(int32_t signum)
{
    exit(1);
}

bool NetdNativeService::Init()
{
    (void)signal(SIGTERM, ExitHandler);
    (void)signal(SIGABRT, ExitHandler);

    netdService_ = std::make_unique<nmd::NetManagerNative>();
    if (netdService_ == nullptr) {
        NETNATIVE_LOGE("netdService_ is nullptr!");
        return false;
    }
    netdService_->init();

    int32_t pid = getpid();
    manager_ = std::make_unique<OHOS::nmd::netlink_manager>(pid);
    if (manager_ == nullptr) {
        NETNATIVE_LOGE("manager_ is nullptr!");
        return false;
    }
    std::thread nlManager([&] { manager_->start(); });
    fwmarkServer_ = std::make_unique<OHOS::nmd::fwmark_server>();
    std::thread fwserve([&] { fwmarkServer_->start(); });

    dnsResolvService_ = std::make_unique<OHOS::nmd::dnsresolv_service>();

    dhcpController_ = std::make_unique<OHOS::nmd::DhcpController>();

    nlManager.detach();
    fwserve.detach();
    return true;
}

int32_t NetdNativeService::SetResolverConfigParcel(const DnsresolverParamsParcel& resolvParams)
{
    NETNATIVE_LOGI("SetResolverConfig retryCount = %{public}d", resolvParams.retryCount_);
    
    return 0;
}

int32_t NetdNativeService::SetResolverConfig(const dnsresolver_params &resolvParams)
{
    NETNATIVE_LOGI("SetResolverConfig retryCount = %{public}d", resolvParams.retryCount);
    return dnsResolvService_->setResolverConfig(resolvParams);
}

int32_t NetdNativeService::GetResolverConfig(const  uint16_t  netid, std::vector<std::string> &servers,
    std::vector<std::string> &domains, nmd::dns_res_params &param)
{
    NETNATIVE_LOGI("GetResolverConfig netid = %{public}d", netid);
    int32_t  ret = dnsResolvService_->getResolverInfo(netid, servers, domains, param);
    NETNATIVE_LOGE("NETDSERVICE: %{public}d,  %{public}d", param.baseTimeoutMsec,  param.retryCount);
    for (auto item:servers) {
        NETNATIVE_LOGE("server: %{public}s", item.c_str());
    }
    return ret;
}

int32_t NetdNativeService::CreateNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("CreateNetworkCache Begin");
    return dnsResolvService_->createNetworkCache(netid);
}

int32_t NetdNativeService::FlushNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("FlushNetworkCache Begin");
    return dnsResolvService_->flushNetworkCache(netid);
}

int32_t NetdNativeService::DestoryNetworkCache(const uint16_t netid)
{
    NETNATIVE_LOGI("DestoryNetworkCache");
    return dnsResolvService_->destoryNetworkCache(netid);
}

int32_t NetdNativeService::Getaddrinfo(const char* node, const char* service, const struct addrinfo* hints,
    struct addrinfo** result, const uint16_t netid)
{
    NETNATIVE_LOGI("Getaddrinfo");
    return dnsResolvService_->getaddrinfo(node, service, hints, result, netid);
}

int32_t NetdNativeService::InterfaceSetMtu(const std::string &interfaceName, int32_t mtu)
{
    NETNATIVE_LOGI("InterfaceSetMtu  Begin");
    return  netdService_->interfaceSetMtu(interfaceName, mtu);
}

int32_t NetdNativeService::InterfaceGetMtu(const std::string &interfaceName)
{
    NETNATIVE_LOGI("InterfaceSetMtu  Begin");
    return  netdService_->interfaceGetMtu(interfaceName);
}

int32_t NetdNativeService::RegisterNotifyCallback(sptr<INotifyCallback> &callback)
{
    NETNATIVE_LOGI("RegisterNotifyCallback");
    notifyCallback_ = callback;
    dhcpController_->RegisterNotifyCallback(callback);
    return 0;
}

int32_t NetdNativeService::NetworkAddRoute(int32_t netId, const std::string &interfaceName,
    const std::string &destination, const std::string &nextHop)
{
    NETNATIVE_LOGI("NetdNativeService::NetworkAddRoute unpacket %{public}d %{public}s %{public}s %{public}s",
        netId, interfaceName.c_str(), destination.c_str(), nextHop.c_str());

    int32_t result = this->netdService_->networkAddRoute(netId, interfaceName, destination, nextHop);
    NETNATIVE_LOGI("NetworkAddRoute %{public}d", result);
    return result;
}

int32_t NetdNativeService::NetworkRemoveRoute(int32_t netId, const std::string &interfaceName,
    const std::string &destination, const std::string &nextHop)
{
    int32_t result = this->netdService_->networkRemoveRoute(netId, interfaceName, destination, nextHop);
    NETNATIVE_LOGI("NetworkRemoveRoute %{public}d", result);
    return result;
}

int32_t NetdNativeService::NetworkAddRouteParcel(int32_t netId, const route_info_parcel &routeInfo)
{
    int32_t result = this->netdService_->networkAddRouteParcel(netId, routeInfo);
    NETNATIVE_LOGI("NetworkAddRouteParcel %{public}d", result);
    return result;
}

int32_t NetdNativeService::NetworkRemoveRouteParcel(int32_t netId, const route_info_parcel &routeInfo)
{
    int32_t result = this->netdService_->networkRemoveRouteParcel(netId, routeInfo);
    NETNATIVE_LOGI("NetworkRemoveRouteParcel %{public}d", result);
    return result;
}

int32_t NetdNativeService::NetworkSetDefault(int32_t netId)
{
    int32_t result = this->netdService_->networkSetDefault(netId);
    NETNATIVE_LOGI("NetworkSetDefault");
    return result;
}

int32_t NetdNativeService::NetworkGetDefault()
{
    int32_t result = this->netdService_->networkGetDefault();
    NETNATIVE_LOGI("NetworkGetDefault");
    return result;
}

int32_t NetdNativeService::NetworkClearDefault()
{
    int32_t result = this->netdService_->networkClearDefault();
    NETNATIVE_LOGI("NetworkClearDefault");
    return result;
}

int32_t NetdNativeService::GetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
    const std::string  &parameter, std::string  &value)
{
    int32_t result = this->netdService_->getProcSysNet(ipversion,  which,  ifname,  parameter, &value);
    NETNATIVE_LOGI("GetProcSysNet");
    return result;
}

int32_t NetdNativeService::SetProcSysNet(int32_t ipversion, int32_t which, const std::string &ifname,
    const std::string  &parameter, std::string  &value)
{
    int32_t result = this->netdService_->setProcSysNet(ipversion,  which,  ifname,  parameter, value);
    NETNATIVE_LOGI("SetProcSysNet");
    return result;
}

int32_t NetdNativeService::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    int32_t result = this->netdService_->networkCreatePhysical(netId, permission);
    NETNATIVE_LOGI("NetworkCreatePhysical");
    return result;
}

int32_t NetdNativeService::InterfaceAddAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    int32_t result = this->netdService_->interfaceAddAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOGI("InterfaceAddAddress");
    return result;
}

int32_t NetdNativeService::InterfaceDelAddress(const std::string &interfaceName, const std::string &addrString,
    int32_t prefixLength)
{
    int32_t result = this->netdService_->interfaceDelAddress(interfaceName, addrString, prefixLength);
    NETNATIVE_LOGI("InterfaceDelAddress");
    return result;
}

int32_t NetdNativeService::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETNATIVE_LOGI("NetworkAddInterface");
    int32_t result = this->netdService_->networkAddInterface(netId, iface);
    return result;
}

int32_t NetdNativeService::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    int32_t result = this->netdService_->networkRemoveInterface(netId, iface);
    NETNATIVE_LOGI("NetworkRemoveInterface");
    return result;
}

int32_t NetdNativeService::NetworkDestroy(int32_t netId)
{
    int32_t result = this->netdService_->networkDestroy(netId);
    NETNATIVE_LOGI("NetworkDestroy");
    return result;
}

int32_t NetdNativeService::GetFwmarkForNetwork(int32_t netId, mark_mask_parcel &markMaskParcel)
{
    markMaskParcel = this->netdService_->getFwmarkForNetwork(netId);
    NETNATIVE_LOGI("GetFwmarkForNetwork");
    return ERR_NONE;
}

int32_t NetdNativeService::InterfaceSetConfig(const interface_configuration_parcel &cfg)
{
    NETNATIVE_LOGI("InterfaceSetConfig");
    this->netdService_->interfaceSetConfig(cfg);
    return ERR_NONE;
}

int32_t NetdNativeService::InterfaceGetConfig(interface_configuration_parcel &cfg)
{
    NETNATIVE_LOGI("InterfaceGetConfig");
    std::string  ifName = cfg.ifName ;
    cfg = this->netdService_->interfaceGetConfig(ifName);
    return ERR_NONE;
}

int32_t NetdNativeService::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOGI("StartDhcpClient");
    this->dhcpController_->StartDhcpClient(iface, bIpv6);
    return ERR_NONE;
}

int32_t NetdNativeService::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETNATIVE_LOGI("StopDhcpClient");
    this->dhcpController_->StopDhcpClient(iface, bIpv6);
    return ERR_NONE;
}

int32_t NetdNativeService::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETNATIVE_LOGI("StartDhcpService");
    this->dhcpController_->StartDhcpService(iface, ipv4addr);
    return ERR_NONE;
}

int32_t NetdNativeService::StopDhcpService(const std::string &iface)
{
    NETNATIVE_LOGI("StopDhcpService");
    this->dhcpController_->StopDhcpService(iface);
    return ERR_NONE;
}
} // namespace NetdNative
} // namespace OHOS
