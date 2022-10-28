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
#include "net_stats_service_stub.h"
#include "net_stats_wrapper.h"
#include "timer.h"

namespace OHOS {
namespace NetManagerStandard {
class NetStatsService : public SystemAbility,
    public NetStatsServiceStub,
    public std::enable_shared_from_this<NetStatsService> {
    DECLARE_DELAYED_SINGLETON(NetStatsService)
    DECLARE_SYSTEM_ABILITY(NetStatsService)

public:
    void OnStart() override;
    void OnStop() override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;
    int32_t RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override;
    int32_t UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override;
    int64_t GetIfaceRxBytes(const std::string &interfaceName) override;
    int64_t GetIfaceTxBytes(const std::string &interfaceName) override;
    int64_t GetCellularRxBytes() override;
    int64_t GetCellularTxBytes() override;
    int64_t GetAllRxBytes() override;
    int64_t GetAllTxBytes() override;
    int64_t GetUidRxBytes(uint32_t uid) override;
    int64_t GetUidTxBytes(uint32_t uid) override;
private:
    bool Init();
    void GetDumpMessage(std::string &message);

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    bool registerToService_;
    ServiceRunningState state_;
    sptr<NetStatsCallback> netStatsCallback_;
    std::unique_ptr<NetStatsWrapper> netStatsWrapper_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_SERVICE_H
