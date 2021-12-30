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

#ifndef I_DNS_RESOLVER_SERVICE_H
#define I_DNS_RESOLVER_SERVICE_H

#include <stdint.h>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "inet_addr.h"

#include "dns_addr_info.h"

namespace OHOS {
namespace NetManagerStandard {
class IDnsResolverService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.IDnsResolverService");
    enum {
        CMD_GET_ADDR_BY_NAME,
        CMD_GET_ADDR_INFO,
        CMD_CRT_NETWORK_CACHE,
        CMD_DEL_NETWORK_CACHE,
        CMD_FLS_NETWORK_CACHE,
        CMD_SET_RESOLVER_CONFIG,
        CMD_GET_RESOLVER_INFO,
    };

public:
    virtual int32_t GetAddressesByName(const std::string &hostName, std::vector<INetAddr> &addrInfo) = 0;
    virtual int32_t GetAddrInfo(const std::string &hostName, const std::string &server,
        const sptr<DnsAddrInfo> &hints, std::vector<sptr<DnsAddrInfo>> &dnsAddrInfo) = 0;
    virtual int32_t CreateNetworkCache(uint16_t netId) = 0;
    virtual int32_t DestoryNetworkCache(uint16_t netId) = 0;
    virtual int32_t FlushNetworkCache(uint16_t netId) = 0;
    virtual int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
        const std::vector<std::string> &servers, const std::vector<std::string> &domains) = 0;
    virtual int32_t GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
        std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_DNS_RESOLVER_SERVICE_H