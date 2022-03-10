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

#ifndef DNS_RESOLVER_SERVICE_H
#define DNS_RESOLVER_SERVICE_H

#include "singleton.h"
#include "system_ability.h"

#include "dns_resolver_service_stub.h"
#include "dns_service_iface.h"

namespace OHOS {
namespace NetManagerStandard {
class DnsResolverService : public SystemAbility,
    public DnsResolverServiceStub, public std::enable_shared_from_this<DnsResolverService> {
    DECLARE_DELAYED_SINGLETON(DnsResolverService)
    DECLARE_SYSTEM_ABILITY(DnsResolverService)

    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

public:
    void OnStart() override;
    void OnStop() override;

    int32_t GetAddressesByName(const std::string &hostName, std::vector<INetAddr> &addrInfo) override;
    int32_t GetAddressesByName(const std::string &hostName, int32_t netId, std::vector<INetAddr> &addrInfo);
    int32_t GetAddrInfo(const std::string &hostName, const std::string &server,
        const sptr<DnsAddrInfo> &hints, std::vector<sptr<DnsAddrInfo>> &dnsAddrInfo) override;
    int32_t CreateNetworkCache(int32_t netId) override;
    int32_t DestroyNetworkCache(int32_t netId) override;
    int32_t FlushNetworkCache(int32_t netId) override;
    int32_t SetResolverConfig(int32_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
        const std::vector<std::string> &servers, const std::vector<std::string> &domains) override;
    int32_t GetResolverInfo(int32_t netId, std::vector<std::string> &servers,
        std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount) override;

private:
    bool Init();

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    bool registerToService_ = false;
    sptr<DnsServiceIface> serviceIface_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // DNS_RESOLVER_SERVICE_H