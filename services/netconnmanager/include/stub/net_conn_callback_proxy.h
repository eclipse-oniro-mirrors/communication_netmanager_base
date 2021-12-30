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

#ifndef NET_CONN_CALLBACK_PROXY_H
#define NET_CONN_CALLBACK_PROXY_H

#include "iremote_proxy.h"

#include "i_net_conn_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnCallbackProxy : public IRemoteProxy<INetConnCallback> {
public:
    explicit NetConnCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetConnCallbackProxy();

public:
    int32_t NetConnStateChanged(const sptr<NetConnCallbackInfo> &info) override;
    int32_t NetAvailable(int32_t netId) override;
    int32_t NetCapabilitiesChange(int32_t netId, const uint64_t &netCap) override;
    int32_t NetConnectionPropertiesChange(int32_t netId, const sptr<NetLinkInfo> &info) override;
    int32_t NetLost(int32_t netId) override;
private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<NetConnCallbackProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_CALLBACK_PROXY_H
