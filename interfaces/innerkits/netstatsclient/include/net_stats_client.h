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

#ifndef NET_STATS_MANAGER_H
#define NET_STATS_MANAGER_H

#include <string>

#include "parcel.h"
#include "singleton.h"

#include "i_net_stats_service.h"
#include "net_stats_constants.h"
#include "net_stats_info.h"

namespace OHOS {
namespace NetManagerStandard {
class NetStatsClient {
    DECLARE_DELAYED_SINGLETON(NetStatsClient)

public:
    int32_t RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback);
    int32_t UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback);
    int64_t GetIfaceRxBytes(const std::string &interfaceName);
    int64_t GetIfaceTxBytes(const std::string &interfaceName);
    int64_t GetCellularRxBytes();
    int64_t GetCellularTxBytes();
    int64_t GetAllRxBytes();
    int64_t GetAllTxBytes();
    int64_t GetUidRxBytes(uint32_t uid);
    int64_t GetUidTxBytes(uint32_t uid);

private:
    class NetStatsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetStatsDeathRecipient(NetStatsClient &client) : client_(client) {}
        ~NetStatsDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetStatsClient &client_;
    };

private:
    sptr<INetStatsService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<INetStatsService> netStatsService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_MANAGER_H