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

#ifndef NET_STATS_SERVICE_PROXY_H
#define NET_STATS_SERVICE_PROXY_H

#include "iremote_proxy.h"
#include "i_net_stats_service.h"

namespace OHOS {
namespace NetManagerStandard {
class NetStatsServiceProxy : public IRemoteProxy<INetStatsService> {
public:
    explicit NetStatsServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetStatsServiceProxy();
    int32_t SystemReady() override;
    int32_t RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override;
    int32_t UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override;
    NetStatsResultCode GetIfaceStatsDetail(const std::string &iface, uint32_t start, uint32_t end,
        NetStatsInfo &statsInfo) override;
    NetStatsResultCode GetUidStatsDetail(const std::string &iface, uint32_t uid,
        uint32_t start, uint32_t end, NetStatsInfo &statsInfo) override;
    NetStatsResultCode UpdateIfacesStats(const std::string &iface,
        uint32_t start, uint32_t end, const NetStatsInfo &stats) override;
    NetStatsResultCode UpdateStatsData() override;
    NetStatsResultCode ResetFactory() override;
private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<NetStatsServiceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_SERVICE_PROXY_H
