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

#ifndef NET_TETHER_REQUEST_NETWORK_H
#define NET_TETHER_REQUEST_NETWORK_H

#include <map>

#include "net_tether_define.h"
#include "i_net_conn_service.h"
#include "net_conn_callback_stub.h"
#include "i_net_conn_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTetherRequestNetwork {
public:
    class TetherRequestNetworkCallback : public NetConnCallbackStub {
    public:
        TetherRequestNetworkCallback(NetTetherRequestNetwork &netTetherRequestNetwork);
        virtual ~TetherRequestNetworkCallback() override;
        int32_t NetConnStateChanged(const sptr<NetConnCallbackInfo> &info) override;
        int32_t NetAvailable(int32_t netId) override;
        int32_t NetCapabilitiesChange(int32_t netId, const uint64_t &netCap) override;
        int32_t NetConnectionPropertiesChange(int32_t netId, const sptr<NetLinkInfo> &info) override;
        int32_t NetLost(int32_t netId) override;
    private:
        NetTetherRequestNetwork &netTetherRequestNetwork_;
    };

public:
    NetTetherRequestNetwork();
    ~NetTetherRequestNetwork();
    void RerequestNetwork();
    void RegisterNetRequestCallback(const RequestNetworkCallback &callback);
    int32_t GetUpstreamNetId() const;
    const NetLinkInfo &GetUpstreamLinkInfo() const;

private:
    int32_t NetAvailable(int32_t netId);
    int32_t NetLost(int32_t netId);
    int32_t NetConnectionPropertiesChange(int32_t netId, const NetLinkInfo &info);
    sptr<INetConnService> GetProxy();

private:
    uint32_t reqId_ = 0;
    RequestNetworkCallback callback_;
    NetLinkInfo info_;
    int32_t netId_ = -1;
    sptr<INetConnService> netConnService_ = nullptr;
    sptr<INetConnCallback> netConncallback_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_REQUEST_NETWORK_H