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

#ifndef DNS_RESOLVER_SERVICE_STUB_H
#define DNS_RESOLVER_SERVICE_STUB_H

#include <map>
#include "iremote_stub.h"
#include "i_dns_resolver_service.h"

namespace OHOS {
namespace NetManagerStandard {
class DnsResolverServiceStub : public IRemoteStub<IDnsResolverService> {
    using DnsResolverServiceFunc = int32_t (DnsResolverServiceStub::*)(MessageParcel &, MessageParcel &);

public:
    DnsResolverServiceStub();
    ~DnsResolverServiceStub();
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnGetAddressesByName(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetAddrInfo(MessageParcel &data, MessageParcel &reply);
    int32_t OnCreateNetworkCache(MessageParcel &data, MessageParcel &reply);
    int32_t OnDestoryNetworkCache(MessageParcel &data, MessageParcel &reply);
    int32_t OnFlushNetworkCache(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetResolverConfig(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetResolverInfo(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, DnsResolverServiceFunc> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // DNS_RESOLVER_SERVICE_STUB_H

