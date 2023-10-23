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

#ifndef NET_DNS_HEALTH_CALLBACK_STUB_H
#define NET_DNS_HEALTH_CALLBACK_STUB_H

#include <map>

#include "i_net_dns_health_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace NetsysNative {
class NetDnsHealthCallbackStub : public IRemoteStub<INetDnsHealthCallback> {
public:
    NetDnsHealthCallbackStub();
    virtual ~NetDnsHealthCallbackStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t OnDnsHealthReport(const NetDnsHealthReport &dnsHealthReport) override;

private:
    using NetDnsHealthCallbackFunc = int32_t (NetDnsHealthCallbackStub::*)(MessageParcel &, MessageParcel &);
    int32_t CmdDnsHealthReport(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, NetDnsHealthCallbackFunc> memberFuncMap_;
};
} // namespace NetsysNative
} // namespace OHOS
#endif // NET_DNS_HEALTH_CALLBACK_STUB_H
