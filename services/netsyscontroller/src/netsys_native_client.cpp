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

#include "netsys_native_client.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "iservice_registry.h"
#include "net_conn_types.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "netsys_native_service_proxy.h"
#include "securec.h"
#include "system_ability_definition.h"

using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace NetManagerStandard {
static constexpr const int64_t DELAY_TIME_US = 1000 * 100;
static constexpr const int32_t RETRY_TIMES = 10;
static constexpr const char *DEV_NET_TUN_PATH = "/dev/net/tun";
static constexpr const char *IF_CFG_UP = "up";
static constexpr const char *IF_CFG_DOWN = "down";

NetsysNativeClient::NativeNotifyCallback::NativeNotifyCallback(NetsysNativeClient &netsysNativeClient)
    : netsysNativeClient_(netsysNativeClient)
{
}

NetsysNativeClient::NativeNotifyCallback::~NativeNotifyCallback() {}

int32_t NetsysNativeClient::NativeNotifyCallback::OnInterfaceAddressUpdated(const std::string &addr,
                                                                            const std::string &ifName, int flags,
                                                                            int scope)
{
    for (auto &cb : netsysNativeClient_.cbObjects_) {
        cb->OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnInterfaceAddressRemoved(const std::string &addr,
                                                                            const std::string &ifName, int flags,
                                                                            int scope)
{
    for (auto &cb : netsysNativeClient_.cbObjects_) {
        cb->OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnInterfaceAdded(const std::string &ifName)
{
    for (auto &cb : netsysNativeClient_.cbObjects_) {
        cb->OnInterfaceAdded(ifName);
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnInterfaceRemoved(const std::string &ifName)
{
    for (auto &cb : netsysNativeClient_.cbObjects_) {
        cb->OnInterfaceRemoved(ifName);
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnInterfaceChanged(const std::string &ifName, bool up)
{
    for (auto &cb : netsysNativeClient_.cbObjects_) {
        cb->OnInterfaceChanged(ifName, up);
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
{
    for (auto &cb : netsysNativeClient_.cbObjects_) {
        cb->OnInterfaceLinkStateChanged(ifName, up);
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnRouteChanged(bool updated, const std::string &route,
                                                                 const std::string &gateway,
                                                                 const std::string &ifName)
{
    for (auto &cb : netsysNativeClient_.cbObjects_) {
        cb->OnRouteChanged(updated, route, gateway, ifName);
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnDhcpSuccess(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult)
{
    NETMGR_LOG_I("NetsysNativeClient::NativeNotifyCallback::OnDhcpSuccess");
    netsysNativeClient_.ProcessDhcpResult(dhcpResult);
    return ERR_NONE;
}

int32_t NetsysNativeClient::NativeNotifyCallback::OnBandwidthReachedLimit(const std::string &limitName,
                                                                          const std::string &iface)
{
    NETMGR_LOG_I("NetsysNativeClient::NativeNotifyCallback::OnBandwidthReachedLimit");
    netsysNativeClient_.ProcessBandwidthReachedLimit(limitName, iface);
    return ERR_NONE;
}

NetsysNativeClient::NetsysNativeClient()
{
    Init();
}

void NetsysNativeClient::Init()
{
    NETMGR_LOG_I("netsys Init");
    if (initFlag_) {
        NETMGR_LOG_I("netsys initialization is complete");
        return;
    }
    initFlag_ = true;
    nativeNotifyCallback_ = std::make_unique<NativeNotifyCallback>(*this).release();
    netsysNativeService_ = GetProxy();
    std::thread thread([this]() {
        int i = 0;
        while (netsysNativeService_ == nullptr) {
            netsysNativeService_ = GetProxy();
            NETMGR_LOG_I("netsysNativeService_ is null waiting for netsys service");
            usleep(DELAY_TIME_US);
            i++;
            if (i > RETRY_TIMES) {
                NETMGR_LOG_E("netsysNativeService_ is null for 10 times");
                break;
            }
        }
        if (netsysNativeService_ != nullptr) {
            netsysNativeService_->RegisterNotifyCallback(nativeNotifyCallback_);
        }
    });
    thread.detach();
}

int32_t NetsysNativeClient::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOG_I("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->NetworkCreatePhysical(netId, permission);
}

int32_t NetsysNativeClient::NetworkDestroy(int32_t netId)
{
    NETMGR_LOG_I("Destroy network: netId[%{public}d]", netId);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->NetworkDestroy(netId);
}

int32_t NetsysNativeClient::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->NetworkAddInterface(netId, iface);
}

int32_t NetsysNativeClient::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOG_I("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->NetworkRemoveInterface(netId, iface);
}

int32_t NetsysNativeClient::NetworkAddRoute(int32_t netId, const std::string &ifName, const std::string &destination,
                                            const std::string &nextHop)
{
    NETMGR_LOG_I("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
                 netId, ifName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetsysNativeClient::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
                                               const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOG_D("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
                 netId, ifName.c_str(), ToAnonymousIp(destination).c_str(), ToAnonymousIp(nextHop).c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

int32_t NetsysNativeClient::InterfaceGetConfig(OHOS::nmd::InterfaceConfigurationParcel &cfg)
{
    NETMGR_LOG_D("Get interface config: ifName[%{public}s]", cfg.ifName.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->InterfaceGetConfig(cfg);
}

int32_t NetsysNativeClient::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOG_D("Set interface down: iface[%{public}s]", iface.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    OHOS::nmd::InterfaceConfigurationParcel ifCfg;
    ifCfg.ifName = iface;
    netsysNativeService_->InterfaceGetConfig(ifCfg);
    auto fit = std::find(ifCfg.flags.begin(), ifCfg.flags.end(), IF_CFG_UP);
    if (fit != ifCfg.flags.end()) {
        ifCfg.flags.erase(fit);
    }
    ifCfg.flags.push_back(IF_CFG_DOWN);
    return netsysNativeService_->InterfaceSetConfig(ifCfg);
}

int32_t NetsysNativeClient::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOG_D("Set interface up: iface[%{public}s]", iface.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    OHOS::nmd::InterfaceConfigurationParcel ifCfg;
    ifCfg.ifName = iface;
    netsysNativeService_->InterfaceGetConfig(ifCfg);
    auto fit = std::find(ifCfg.flags.begin(), ifCfg.flags.end(), IF_CFG_DOWN);
    if (fit != ifCfg.flags.end()) {
        ifCfg.flags.erase(fit);
    }
    ifCfg.flags.push_back(IF_CFG_UP);
    return netsysNativeService_->InterfaceSetConfig(ifCfg);
}

void NetsysNativeClient::InterfaceClearAddrs(const std::string &ifName)
{
    NETMGR_LOG_D("Clear addrs: ifName[%{public}s]", ifName.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return;
    }
    return;
}

int32_t NetsysNativeClient::InterfaceGetMtu(const std::string &ifName)
{
    NETMGR_LOG_D("Get mtu: ifName[%{public}s]", ifName.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->InterfaceGetMtu(ifName);
}

int32_t NetsysNativeClient::InterfaceSetMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOG_D("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->InterfaceSetMtu(ifName, mtu);
}

int32_t NetsysNativeClient::InterfaceAddAddress(const std::string &ifName, const std::string &ipAddr,
                                                int32_t prefixLength)
{
    NETMGR_LOG_D("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]", ifName.c_str(),
                 ToAnonymousIp(ipAddr).c_str(), prefixLength);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->InterfaceAddAddress(ifName, ipAddr, prefixLength);
}

int32_t NetsysNativeClient::InterfaceDelAddress(const std::string &ifName, const std::string &ipAddr,
                                                int32_t prefixLength)
{
    NETMGR_LOG_D("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]", ifName.c_str(),
                 ToAnonymousIp(ipAddr).c_str(), prefixLength);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->InterfaceDelAddress(ifName, ipAddr, prefixLength);
}

int32_t NetsysNativeClient::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
                                              const std::vector<std::string> &servers,
                                              const std::vector<std::string> &domains)
{
    NETMGR_LOG_D("Set resolver config: netId[%{public}d]", netId);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
}

int32_t NetsysNativeClient::GetResolverConfig(uint16_t netId, std::vector<std::string> &servers,
                                              std::vector<std::string> &domains, uint16_t &baseTimeoutMsec,
                                              uint8_t &retryCount)
{
    NETMGR_LOG_D("Get resolver config: netId[%{public}d]", netId);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->GetResolverConfig(netId, servers, domains, baseTimeoutMsec, retryCount);
}

int32_t NetsysNativeClient::CreateNetworkCache(uint16_t netId)
{
    NETMGR_LOG_D("create dns cache: netId[%{public}d]", netId);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->CreateNetworkCache(netId);
}

int32_t NetsysNativeClient::DestroyNetworkCache(uint16_t netId)
{
    NETMGR_LOG_D("Destroy dns cache: netId[%{public}d]", netId);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->DestroyNetworkCache(netId);
}

int32_t NetsysNativeClient::GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                                     nmd::NetworkSharingTraffic &traffic)
{
    NETMGR_LOG_D("NetsysNativeClient GetNetworkSharingTraffic");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->GetNetworkSharingTraffic(downIface, upIface, traffic);
}

int64_t NetsysNativeClient::GetCellularRxBytes()
{
    NETMGR_LOG_D("NetsysNativeClient GetCellularRxBytes");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetCellularTxBytes()
{
    NETMGR_LOG_D("NetsysNativeClient GetCellularTxBytes");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetAllRxBytes()
{
    NETMGR_LOG_D("NetsysNativeClient GetAllRxBytes");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetAllTxBytes()
{
    NETMGR_LOG_D("NetsysNativeClient GetAllTxBytes");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetUidRxBytes(uint32_t uid)
{
    NETMGR_LOG_D("NetsysNativeClient GetUidRxBytes uid is [%{public}u]", uid);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetUidTxBytes(uint32_t uid)
{
    NETMGR_LOG_D("NetsysNativeClient GetUidTxBytes uid is [%{public}u]", uid);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetUidOnIfaceRxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysNativeClient GetUidOnIfaceRxBytes uid is [%{public}u] iface name is [%{public}s]", uid,
                 interfaceName.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetUidOnIfaceTxBytes(uint32_t uid, const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysNativeClient GetUidOnIfaceTxBytes uid is [%{public}u] iface name is [%{public}s]", uid,
                 interfaceName.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetIfaceRxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysNativeClient GetIfaceRxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetIfaceTxBytes(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysNativeClient GetIfaceTxBytes iface name is [%{public}s]", interfaceName.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysNativeService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

std::vector<std::string> NetsysNativeClient::InterfaceGetList()
{
    NETMGR_LOG_D("NetsysNativeClient InterfaceGetList");
    std::vector<std::string> ret;
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ret;
    }
    netsysNativeService_->InterfaceGetList(ret);
    return ret;
}

std::vector<std::string> NetsysNativeClient::UidGetList()
{
    NETMGR_LOG_D("NetsysNativeClient UidGetList");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return {};
    }
    return {};
}

int64_t NetsysNativeClient::GetIfaceRxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysNativeClient GetIfaceRxPackets iface name is [%{public}s]", interfaceName.c_str());
    return ERR_NONE;
}

int64_t NetsysNativeClient::GetIfaceTxPackets(const std::string &interfaceName)
{
    NETMGR_LOG_D("NetsysNativeClient GetIfaceTxPackets iface name is [%{public}s]", interfaceName.c_str());
    return ERR_NONE;
}

int32_t NetsysNativeClient::SetDefaultNetWork(int32_t netId)
{
    NETMGR_LOG_D("NetsysNativeClient SetDefaultNetWork");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->NetworkSetDefault(netId);
}

int32_t NetsysNativeClient::ClearDefaultNetWorkNetId()
{
    NETMGR_LOG_D("NetsysNativeClient ClearDefaultNetWorkNetId");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::BindSocket(int32_t socket_fd, uint32_t netId)
{
    NETMGR_LOG_D("NetsysNativeClient::BindSocket: netId = [%{public}u]", netId);
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::IpEnableForwarding(const std::string &requestor)
{
    NETMGR_LOG_D("NetsysNativeClient IpEnableForwarding: requestor[%{public}s]", requestor.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->IpEnableForwarding(requestor);
}

int32_t NetsysNativeClient::IpDisableForwarding(const std::string &requestor)
{
    NETMGR_LOG_D("NetsysNativeClient IpDisableForwarding: requestor[%{public}s]", requestor.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->IpDisableForwarding(requestor);
}

int32_t NetsysNativeClient::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETMGR_LOG_D("NetsysNativeClient EnableNat: intIface[%{public}s] intIface[%{public}s]", downstreamIface.c_str(),
                 upstreamIface.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->EnableNat(downstreamIface, upstreamIface);
}

int32_t NetsysNativeClient::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    NETMGR_LOG_D("NetsysNativeClient DisableNat: intIface[%{public}s] intIface[%{public}s]", downstreamIface.c_str(),
                 upstreamIface.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->DisableNat(downstreamIface, upstreamIface);
}

int32_t NetsysNativeClient::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->IpfwdAddInterfaceForward(fromIface, toIface);
}

int32_t NetsysNativeClient::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    NETMGR_LOG_D("NetsysNativeClient IpfwdRemoveInterfaceForward: fromIface[%{public}s], toIface[%{public}s]",
                 fromIface.c_str(), toIface.c_str());
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->IpfwdRemoveInterfaceForward(fromIface, toIface);
}

int32_t NetsysNativeClient::ShareDnsSet(uint16_t netId)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->ShareDnsSet(netId);
}

int32_t NetsysNativeClient::StartDnsProxyListen()
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->StartDnsProxyListen();
}

int32_t NetsysNativeClient::StopDnsProxyListen()
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->StopDnsProxyListen();
}

int32_t NetsysNativeClient::RegisterNetsysNotifyCallback(const NetsysNotifyCallback &callback)
{
    NETMGR_LOG_D("NetsysNativeClient RegisterNetsysNotifyCallback");
    return ERR_NONE;
}

sptr<OHOS::NetsysNative::INetsysService> NetsysNativeClient::GetProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        NETMGR_LOG_E("NetsysNativeClient samgr null");
        return nullptr;
    }
    auto remote = samgr->GetSystemAbility(OHOS::COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOG_E("NetsysNativeClient remote null");
        return nullptr;
    }
    auto proxy = iface_cast<NetsysNative::INetsysService>(remote);
    return proxy;
}

int32_t NetsysNativeClient::BindNetworkServiceVpn(int32_t socketFd)
{
    NETMGR_LOG_D("NetsysNativeClient::BindNetworkServiceVpn: socketFd[%{public}d]", socketFd);
    /* netsys provide default interface name */
    const char *defaultNetName = "wlan0";
    int defaultNetNameLen = strlen(defaultNetName);
    /* set socket by option. */
    int32_t ret = setsockopt(socketFd, SOL_SOCKET, SO_MARK, defaultNetName, defaultNetNameLen);
    if (ret < 0) {
        NETMGR_LOG_E("The SO_BINDTODEVICE of setsockopt failed.");
        return ERR_VPN;
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::EnableVirtualNetIfaceCard(int32_t socketFd, struct ifreq &ifRequest, int32_t &ifaceFd)
{
    NETMGR_LOG_D("NetsysNativeClient::EnableVirtualNetIfaceCard: socketFd[%{public}d]", socketFd);
    int32_t ifaceFdTemp = 0;
    if ((ifaceFdTemp = open(DEV_NET_TUN_PATH, O_RDWR)) < 0) {
        NETMGR_LOG_E("VPN tunnel device open was failed.");
        return ERR_VPN;
    }

    /*
     * Flags:
     * IFF_TUN   - TUN device (no Ethernet headers)
     * IFF_TAP   - TAP device
     * IFF_NO_PI - Do not provide packet information
     **/
    ifRequest.ifr_flags = IFF_TUN | IFF_NO_PI;
    /**
     * Try to create the device. if it cannot assign the device interface name, kernel can
     * allocate the next device interface name. for example, there is tun0, kernel can
     * allocate tun1.
     **/
    if (ioctl(ifaceFdTemp, TUNSETIFF, &ifRequest) < 0) {
        NETMGR_LOG_E("The TUNSETIFF of ioctl failed, ifRequest.ifr_name[%{public}s]", ifRequest.ifr_name);
        return ERR_VPN;
    }

    /* Activate the device */
    ifRequest.ifr_flags = IFF_UP;
    if (ioctl(socketFd, SIOCSIFFLAGS, &ifRequest) < 0) {
        NETMGR_LOG_E("The SIOCSIFFLAGS of ioctl failed.");
        return ERR_VPN;
    }

    ifaceFd = ifaceFdTemp;
    return ERR_NONE;
}

static inline in_addr_t *AsInAddr(sockaddr *sa)
{
    return &(reinterpret_cast<sockaddr_in *>(sa))->sin_addr.s_addr;
}

int32_t NetsysNativeClient::SetIpAddress(int32_t socketFd, const std::string &ipAddress, int32_t prefixLen,
                                         struct ifreq &ifRequest)
{
    NETMGR_LOG_D("NetsysNativeClient::SetIpAddress: socketFd[%{public}d]", socketFd);

    ifRequest.ifr_addr.sa_family = AF_INET;
    ifRequest.ifr_netmask.sa_family = AF_INET;

    /* inet_pton is IP ipAddress translation to binary network byte order. */
    if (inet_pton(AF_INET, ipAddress.c_str(), AsInAddr(&ifRequest.ifr_addr)) != 1) {
        NETMGR_LOG_E("inet_pton failed.");
        return ERR_VPN;
    }
    if (ioctl(socketFd, SIOCSIFADDR, &ifRequest) < 0) {
        NETMGR_LOG_E("The SIOCSIFADDR of ioctl failed.");
        return ERR_VPN;
    }
    in_addr_t addressPrefixLength = prefixLen ? (~0 << (MAX_IPV4_ADDRESS_LEN - prefixLen)) : 0;
    *AsInAddr(&ifRequest.ifr_netmask) = htonl(addressPrefixLength);
    if (ioctl(socketFd, SIOCSIFNETMASK, &ifRequest)) {
        NETMGR_LOG_E("The SIOCSIFNETMASK of ioctl failed.");
        return ERR_VPN;
    }

    return ERR_NONE;
}

int32_t NetsysNativeClient::SetBlocking(int32_t ifaceFd, bool isBlock)
{
    NETMGR_LOG_D("NetsysNativeClient::SetBlocking");
    int32_t blockingFlag = 0;
    blockingFlag = fcntl(ifaceFd, F_GETFL);
    if (blockingFlag < 0) {
        NETMGR_LOG_E("The blockingFlag of fcntl failed.");
        return ERR_VPN;
    }

    if (!isBlock) {
        blockingFlag = static_cast<int>(static_cast<uint32_t>(blockingFlag) | static_cast<uint32_t>(O_NONBLOCK));
    } else {
        blockingFlag = static_cast<int>(static_cast<uint32_t>(blockingFlag) | static_cast<uint32_t>(~O_NONBLOCK));
    }

    if (fcntl(ifaceFd, F_SETFL, blockingFlag) < 0) {
        NETMGR_LOG_E("The F_SETFL of fcntl failed.");
        return ERR_VPN;
    }
    return ERR_NONE;
}

int32_t NetsysNativeClient::StartDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetsysNativeClient::StartDhcpClient");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->StartDhcpClient(iface, bIpv6);
}

int32_t NetsysNativeClient::StopDhcpClient(const std::string &iface, bool bIpv6)
{
    NETMGR_LOG_D("NetsysNativeClient::StopDhcpClient");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("netsysService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netsysNativeService_->StopDhcpClient(iface, bIpv6);
}

int32_t NetsysNativeClient::RegisterCallback(sptr<NetsysControllerCallback> callback)
{
    NETMGR_LOG_D("NetsysNativeClient::RegisterCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("Callback is nullptr");
        return ERR_INVALID_PARAMS;
    }
    cbObjects_.push_back(callback);
    return ERR_NONE;
}

void NetsysNativeClient::ProcessDhcpResult(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult)
{
    NETMGR_LOG_I("NetsysNativeClient::ProcessDhcpResult");
    NetsysControllerCallback::DhcpResult result;
    for (std::vector<sptr<NetsysControllerCallback>>::iterator it = cbObjects_.begin(); it != cbObjects_.end();
         ++it) {
        result.iface_ = dhcpResult->iface_;
        result.ipAddr_ = dhcpResult->ipAddr_;
        result.gateWay_ = dhcpResult->gateWay_;
        result.subNet_ = dhcpResult->subNet_;
        result.route1_ = dhcpResult->route1_;
        result.route2_ = dhcpResult->route2_;
        result.dns1_ = dhcpResult->dns1_;
        result.dns2_ = dhcpResult->dns2_;
        (*it)->OnDhcpSuccess(result);
    }
}

int32_t NetsysNativeClient::StartDhcpService(const std::string &iface, const std::string &ipv4addr)
{
    NETMGR_LOG_D("NetsysNativeClient StartDhcpService");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("StartDhcpService netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->StartDhcpService(iface, ipv4addr);
}

int32_t NetsysNativeClient::StopDhcpService(const std::string &iface)
{
    NETMGR_LOG_D("NetsysNativeClient StopDhcpService");
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("StopDhcpService netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->StopDhcpService(iface);
}

void NetsysNativeClient::ProcessBandwidthReachedLimit(const std::string &limitName, const std::string &iface)
{
    NETMGR_LOG_D("NetsysNativeClient ProcessBandwidthReachedLimit, limitName=%{public}s, iface=%{public}s",
                 limitName.c_str(), iface.c_str());
    std::for_each(cbObjects_.begin(), cbObjects_.end(),
                  [limitName, iface](const sptr<NetsysControllerCallback> &callback) {
                      callback->OnBandwidthReachedLimit(limitName, iface);
                  });
}

int32_t NetsysNativeClient::BandwidthEnableDataSaver(bool enable)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("BandwidthEnableDataSaver netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->BandwidthEnableDataSaver(enable);
}

int32_t NetsysNativeClient::BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("BandwidthSetIfaceQuota netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->BandwidthSetIfaceQuota(ifName, bytes);
}

int32_t NetsysNativeClient::BandwidthRemoveIfaceQuota(const std::string &ifName)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("BandwidthRemoveIfaceQuota netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->BandwidthRemoveIfaceQuota(ifName);
}

int32_t NetsysNativeClient::BandwidthAddDeniedList(uint32_t uid)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("BandwidthAddDeniedList netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->BandwidthAddDeniedList(uid);
}

int32_t NetsysNativeClient::BandwidthRemoveDeniedList(uint32_t uid)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("BandwidthRemoveDeniedList netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->BandwidthRemoveDeniedList(uid);
}

int32_t NetsysNativeClient::BandwidthAddAllowedList(uint32_t uid)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("BandwidthAddAllowedList netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->BandwidthAddAllowedList(uid);
}

int32_t NetsysNativeClient::BandwidthRemoveAllowedList(uint32_t uid)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("BandwidthRemoveAllowedList netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->BandwidthRemoveAllowedList(uid);
}

int32_t NetsysNativeClient::FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("FirewallSetUidsAllowedListChain netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->FirewallSetUidsAllowedListChain(chain, uids);
}

int32_t NetsysNativeClient::FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("FirewallSetUidsDeniedListChain netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->FirewallSetUidsDeniedListChain(chain, uids);
}

int32_t NetsysNativeClient::FirewallEnableChain(uint32_t chain, bool enable)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("FirewallEnableChain netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->FirewallEnableChain(chain, enable);
}

int32_t NetsysNativeClient::FirewallSetUidRule(uint32_t chain, uint32_t uid, uint32_t firewallRule)
{
    if (netsysNativeService_ == nullptr) {
        NETMGR_LOG_E("FirewallSetUidRule netsysNativeService_ is null");
        return ERR_NATIVESERVICE_NOTFIND;
    }
    return netsysNativeService_->FirewallSetUidRule(chain, uid, firewallRule);
}
} // namespace NetManagerStandard
} // namespace OHOS
