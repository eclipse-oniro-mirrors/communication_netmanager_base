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

#ifndef DNS_RESOLVER_MANAGER_H
#define DNS_RESOLVER_MANAGER_H

#include <string>

#include "parcel.h"
#include "singleton.h"

#include "i_dns_resolver_service.h"

namespace OHOS {
namespace NetManagerStandard {
class DnsResolverClient {
    DECLARE_DELAYED_SINGLETON(DnsResolverClient)

public:
    int32_t GetAddressesByName(const std::string &hostName, std::vector<INetAddr> &addrInfo);
    int32_t GetAddrInfo(const std::string &hostName, const std::string &server,
        const sptr<DnsAddrInfo> &hints, std::vector<sptr<DnsAddrInfo>> &dnsAddrInfo);
    int32_t CreateNetworkCache(uint16_t netId);
    int32_t DestoryNetworkCache(uint16_t netId);
    int32_t FlushNetworkCache(uint16_t netId);
    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
        const std::vector<std::string> &servers, const std::vector<std::string> &domains);
    int32_t GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
        std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount);

private:
    class DnsResolverDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit DnsResolverDeathRecipient(DnsResolverClient &client) : client_(client) {}
        ~DnsResolverDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        DnsResolverClient &client_;
    };

private:
    sptr<IDnsResolverService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<IDnsResolverService> dnsResolverService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // DNS_RESOLVER_MANAGER_H