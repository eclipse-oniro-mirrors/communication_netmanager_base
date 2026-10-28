/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_FAKE_NET_STATS_SERVICE_H
#define COMMON_FAKE_NET_STATS_SERVICE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "inet_stats_service.h"

namespace OHOS {
namespace NetManagerStandard {

/**
 * @brief Hand written fake of the IDL generated INetStatsService. Every method
 *        succeeds, so NetStatsClient tests verify the client path without real
 *        IPC to the stats service (SA 1153) and without its permission checks.
 */
class FakeNetStatsService : public INetStatsService {
public:
    FakeNetStatsService() = default;
    ~FakeNetStatsService() override = default;

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    ErrCode GetIfaceRxBytes(uint64_t &stats, const std::string &interfaceName) override
    {
        return 0;
    }

    ErrCode GetIfaceTxBytes(uint64_t &stats, const std::string &interfaceName) override
    {
        return 0;
    }

    ErrCode GetCellularRxBytes(uint64_t &stats) override
    {
        return 0;
    }

    ErrCode GetCellularTxBytes(uint64_t &stats) override
    {
        return 0;
    }

    ErrCode GetAllRxBytes(uint64_t &stats) override
    {
        return 0;
    }

    ErrCode GetAllTxBytes(uint64_t &stats) override
    {
        return 0;
    }

    ErrCode GetUidRxBytes(uint64_t &stats, uint32_t uid) override
    {
        return 0;
    }

    ErrCode GetUidTxBytes(uint64_t &stats, uint32_t uid) override
    {
        return 0;
    }

    ErrCode GetAllStatsInfo(std::vector<NetStatsInfo> &info) override
    {
        return 0;
    }

    ErrCode GetAllSimStatsInfo(std::vector<NetStatsInfo> &infos) override
    {
        return 0;
    }

    ErrCode GetTrafficStatsByNetwork(std::unordered_map<uint32_t, NetStatsInfo> &infos,
                                     const NetStatsNetwork &network) override
    {
        return 0;
    }

    ErrCode GetTrafficStatsByUidNetwork(std::vector<NetStatsInfoSequence> &infos, uint32_t uid,
                                        const NetStatsNetwork &network) override
    {
        return 0;
    }

#ifdef FEATURE_ENABLE_AUTOMOTIVE_TRAFFIC_STAT
    ErrCode GetIfaceTrafficStats(std::unordered_map<uint32_t, NetStatsInfo> &infos, const std::string &iface,
                                 uint64_t start, uint64_t end) override
    {
        return 0;
    }
#endif

    ErrCode GetMonthTrafficStatsByNetwork(uint32_t simId, uint64_t &monthTrafficData) override
    {
        return 0;
    }

    ErrCode SetAppStats(const PushStatsInfo &info) override
    {
        return 0;
    }

    ErrCode SetDpaAppStats(const NetStatsInfo &info) override
    {
        return 0;
    }

    ErrCode RegisterNetStatsCallback(const sptr<INetStatsCallback> &cb) override
    {
        return 0;
    }

    ErrCode UnregisterNetStatsCallback(const sptr<INetStatsCallback> &cb) override
    {
        return 0;
    }

    ErrCode GetIfaceStatsDetail(const std::string &iface, uint64_t start, uint64_t end,
                                NetStatsInfo &statsInfo) override
    {
        return 0;
    }

    ErrCode GetUidStatsDetail(const std::string &iface, uint32_t uid, uint64_t start, uint64_t end,
                              NetStatsInfo &statsInfo) override
    {
        return 0;
    }

    ErrCode UpdateIfacesStats(const std::string &iface, uint64_t start, uint64_t end,
                              const NetStatsInfo &stats) override
    {
        return 0;
    }

    ErrCode UpdateStatsData() override
    {
        return 0;
    }

    ErrCode ResetFactory() override
    {
        return 0;
    }

    ErrCode GetCookieRxBytes(uint64_t &stats, uint64_t cookie) override
    {
        return 0;
    }

    ErrCode GetCookieTxBytes(uint64_t &stats, uint64_t cookie) override
    {
        return 0;
    }

    ErrCode SaveSharingTraffic(const NetStatsInfo &infos) override
    {
        return 0;
    }

    ErrCode SetCalibrationTraffic(uint32_t simId, int64_t remainingData, uint64_t totalMonthlyData) override
    {
        return 0;
    }

    ErrCode SetTrafficPlanInfo(int32_t simId, int32_t param, int64_t value) override
    {
        return 0;
    }

    ErrCode GetTrafficPlanInfo(int32_t simId, int32_t param, int64_t &value) override
    {
        return 0;
    }
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // COMMON_FAKE_NET_STATS_SERVICE_H
