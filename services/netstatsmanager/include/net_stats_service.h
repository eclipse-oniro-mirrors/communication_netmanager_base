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

#ifndef NET_STATS_SERVICE_H
#define NET_STATS_SERVICE_H

#include "singleton.h"
#include "system_ability.h"

#include "net_stats_callback.h"
#include "net_stats_csv.h"
#include "net_stats_listener.h"
#include "net_stats_service_iface.h"
#include "net_stats_service_stub.h"
#include "net_stats_wrapper.h"
#include "timer.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t INTERVAL_UPDATE_STATS_TIME_MS = 1800000; // half an hour
class NetStatsService : public SystemAbility,
    public NetStatsServiceStub,
    public std::enable_shared_from_this<NetStatsService> {
    DECLARE_DELAYED_SINGLETON(NetStatsService)
    DECLARE_SYSTEM_ABILITY(NetStatsService)

public:
    void OnStart() override;
    void OnStop() override;
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
    int64_t GetIfaceRxBytes(const std::string &interfaceName) override;
    int64_t GetIfaceTxBytes(const std::string &interfaceName) override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    int64_t GetCellularRxBytes() override;
    int64_t GetCellularTxBytes() override;
    int64_t GetAllRxBytes() override;
    int64_t GetAllTxBytes() override;
    int64_t GetUidRxBytes(uint32_t uid) override;
    int64_t GetUidTxBytes(uint32_t uid) override;
private:
    bool Init();

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    bool registerToService_;
    ServiceRunningState state_;
    Timer updateStatsTimer_;
    sptr<NetStatsCallback> netStatsCallback_;
    std::shared_ptr<NetStatsListener> subscriber_ = nullptr;
    std::unique_ptr<NetStatsCsv> netStatsCsv_ = nullptr;
    sptr<NetStatsServiceIface> serviceIface_ = nullptr;
    std::unique_ptr<NetStatsWrapper> netStatsWrapper_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_SERVICE_H
