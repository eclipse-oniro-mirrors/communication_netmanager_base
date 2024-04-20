/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef COMMUNICATION_NETMANAGER_BASE_GET_TRAFFIC_STATS_BY_NETWORK_CONTEXT_H
#define COMMUNICATION_NETMANAGER_BASE_GET_TRAFFIC_STATS_BY_NETWORK_CONTEXT_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include <napi/native_api.h>

#include "base_context.h"
#include "net_stats_info.h"

namespace OHOS {
namespace NetManagerStandard {
class GetTrafficStatsByNetworkContext final : public BaseContext {
public:
    GetTrafficStatsByNetworkContext() = delete;
    explicit GetTrafficStatsByNetworkContext(napi_env env, EventManager *manager);

    void SetNetBearType(uint32_t bearerType);
    void SetStartTime(uint32_t startTime);
    void SetEndTime(uint32_t endTime);
    void SetSimId(uint32_t simId);
    uint32_t GetNetBearType() const;
    uint32_t GetStartTime() const;
    uint32_t GetEndTime() const;
    uint32_t GetSimId() const;

    std::unordered_map<uint32_t, NetStatsInfo> &GetNetStatsInfo();

    void ParseParams(napi_value *params, size_t paramsCount);

private:
    bool CheckParamsType(napi_value *params, size_t paramsCount);

    bool CheckNetworkParams(napi_value *params, size_t paramsCount);

private:
    uint32_t netBearType_ = 0;
    uint32_t startTime_ = 0;
    uint32_t endTime_ = 0;
    uint32_t simId_ = UINT32_MAX;

    std::unordered_map<uint32_t, NetStatsInfo> stats_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // COMMUNICATION_NETMANAGER_BASE_GET_TRAFFIC_STATS_BY_NETWORK_CONTEXT_H