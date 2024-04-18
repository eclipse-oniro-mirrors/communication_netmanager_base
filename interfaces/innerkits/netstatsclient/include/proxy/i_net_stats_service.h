/*
 * Copyright (c) 2021-20223 Huawei Device Co., Ltd.
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

#ifndef I_NET_STATS_SERVICE_H
#define I_NET_STATS_SERVICE_H

#include <string>

#include "i_net_stats_callback.h"
#include "iremote_broker.h"
#include "net_push_stats_info.h"
#include "net_stats_constants.h"
#include "net_stats_info.h"
#include "net_stats_info_sequence.h"
#include "net_stats_network.h"
#include "stats_ipc_interface_code.h"

namespace OHOS {
namespace NetManagerStandard {
class INetStatsService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetStatsService");

public:
    virtual int32_t GetIfaceRxBytes(uint64_t &stats, const std::string &interfaceName) = 0;
    virtual int32_t GetIfaceTxBytes(uint64_t &stats, const std::string &interfaceName) = 0;
    virtual int32_t GetCellularRxBytes(uint64_t &stats) = 0;
    virtual int32_t GetCellularTxBytes(uint64_t &stats) = 0;
    virtual int32_t GetAllRxBytes(uint64_t &stats) = 0;
    virtual int32_t GetAllTxBytes(uint64_t &stats) = 0;
    virtual int32_t GetUidRxBytes(uint64_t &stats, uint32_t uid) = 0;
    virtual int32_t GetUidTxBytes(uint64_t &stats, uint32_t uid) = 0;
    virtual int32_t GetAllStatsInfo(std::vector<NetStatsInfo> &info) = 0;
    virtual int32_t GetAllContainerStatsInfo(std::vector<NetStatsInfo> &infos) = 0;
    virtual int32_t GetTrafficStatsByNetwork(std::unordered_map<uint32_t, NetStatsInfo> &infos,
                                             const sptr<NetStatsNetwork> &network) = 0;
    virtual int32_t GetTrafficStatsByUidNetwork(std::vector<NetStatsInfoSequence> &infos, uint32_t uid,
                                                const sptr<NetStatsNetwork> &network) = 0;
    virtual int32_t SetAppStats(const PushStatsInfo &info) = 0;
    virtual int32_t RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback) = 0;
    virtual int32_t UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback) = 0;
    virtual int32_t GetIfaceStatsDetail(const std::string &iface, uint64_t start, uint64_t end,
                                        NetStatsInfo &statsInfo) = 0;
    virtual int32_t GetUidStatsDetail(const std::string &iface, uint32_t uid, uint64_t start, uint64_t end,
                                      NetStatsInfo &statsInfo) = 0;
    virtual int32_t UpdateIfacesStats(const std::string &iface, uint64_t start, uint64_t end,
                                      const NetStatsInfo &stats) = 0;
    virtual int32_t UpdateStatsData() = 0;
    virtual int32_t ResetFactory() = 0;
    virtual int32_t GetCookieRxBytes(uint64_t &stats, uint64_t cookie) = 0;
    virtual int32_t GetCookieTxBytes(uint64_t &stats, uint64_t cookie) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_STATS_SERVICE_H
