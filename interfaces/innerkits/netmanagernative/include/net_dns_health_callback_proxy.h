/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef  NET_DNS_HEALTH_CALLBACK_PROXY_H
#define  NET_DNS_HEALTH_CALLBACK_PROXY_H
#include "iremote_proxy.h"

#include "i_net_dns_health_callback.h"

namespace OHOS {
namespace NetsysNative {
class NetDnsHealthCallbackProxy : public IRemoteProxy<INetDnsHealthCallback> {
public:
    explicit NetDnsHealthCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetDnsHealthCallbackProxy() = default;

    int32_t OnDnsHealthReport(const NetDnsHealthReport &dnsHealthReport) override;

private:
    static inline BrokerDelegator<NetDnsHealthCallbackProxy> delegator_;
};
} // namespace NetsysNative
} // namespace OHOS
#endif // NET_DNS_HEALTH_CALLBACK_PROXY_H
