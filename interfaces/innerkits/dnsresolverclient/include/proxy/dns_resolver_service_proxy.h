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

#ifndef DNS_RESOLVER_SERVICE_PROXY_H
#define DNS_RESOLVER_SERVICE_PROXY_H

#include <string>

#include "iremote_proxy.h"
#include "i_dns_resolver_service.h"

namespace OHOS {
namespace NetManagerStandard {
class DnsResolverServiceProxy : public IRemoteProxy<IDnsResolverService> {
public:
    explicit DnsResolverServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~DnsResolverServiceProxy();
    bool WriteInterfaceToken(MessageParcel &data);
    int32_t GetAddressesByName(const std::string &hostName, std::vector<INetAddr> &addrInfo) override;
    int32_t GetAddrInfo(const std::string &hostName, const std::string &server,
        const sptr<DnsAddrInfo> &hints, std::vector<sptr<DnsAddrInfo>> &dnsAddrInfo) override;
    int32_t CreateNetworkCache(uint16_t netId) override;
    int32_t DestoryNetworkCache(uint16_t netId) override;
    int32_t FlushNetworkCache(uint16_t netId) override;
    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
        const std::vector<std::string> &servers, const std::vector<std::string> &domains) override;
    int32_t GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
        std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount) override;

private:
    static inline BrokerDelegator<DnsResolverServiceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // DNS_RESOLVER_SERVICE_PROXY_H