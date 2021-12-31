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

#include "dns_resolver_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
DnsResolverClient::DnsResolverClient() : dnsResolverService_(nullptr), deathRecipient_(nullptr) {}

DnsResolverClient::~DnsResolverClient() {}

int32_t DnsResolverClient::GetAddressesByName(const std::string &hostName, std::vector<INetAddr> &addrInfo)
{
    sptr<IDnsResolverService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->GetAddressesByName(hostName, addrInfo);
}

int32_t DnsResolverClient::GetAddrInfo(const std::string &hostName, const std::string &server,
    const sptr<DnsAddrInfo> &hints, std::vector<sptr<DnsAddrInfo>> &dnsAddrInfo)
{
    sptr<IDnsResolverService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->GetAddrInfo(hostName, server, hints, dnsAddrInfo);
}

int32_t DnsResolverClient::CreateNetworkCache(uint16_t netId)
{
    sptr<IDnsResolverService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->CreateNetworkCache(netId);
}

int32_t DnsResolverClient::DestoryNetworkCache(uint16_t netId)
{
    sptr<IDnsResolverService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->DestoryNetworkCache(netId);
}

int32_t DnsResolverClient::FlushNetworkCache(uint16_t netId)
{
    sptr<IDnsResolverService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->FlushNetworkCache(netId);
}

int32_t DnsResolverClient::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    sptr<IDnsResolverService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
}

int32_t DnsResolverClient::GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    sptr<IDnsResolverService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOG_E("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->GetResolverInfo(netId, servers, domains, baseTimeoutMsec, retryCount);
}

sptr<IDnsResolverService> DnsResolverClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (dnsResolverService_) {
        NETMGR_LOG_D("get proxy is ok");
        return dnsResolverService_;
    }
    NETMGR_LOG_D("execute GetSystemAbilityManager");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOG_E("GetProxy, get SystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_DNS_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOG_E("get Remote service failed");
        return nullptr;
    }
    deathRecipient_ = (std::make_unique<DnsResolverDeathRecipient>(*this)).release();
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_LOG_E("add death recipient failed");
        return nullptr;
    }
    dnsResolverService_ = iface_cast<IDnsResolverService>(remote);
    if (dnsResolverService_ == nullptr) {
        NETMGR_LOG_E("get Remote service proxy failed");
        return nullptr;
    }
    return dnsResolverService_;
}

void DnsResolverClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_LOG_D("on remote died");
    if (remote == nullptr) {
        NETMGR_LOG_E("remote object is nullptr");
        return;
    }
    std::lock_guard lock(mutex_);
    if (dnsResolverService_ == nullptr) {
        NETMGR_LOG_E("dnsResolverService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = dnsResolverService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_LOG_E("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    dnsResolverService_ = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS