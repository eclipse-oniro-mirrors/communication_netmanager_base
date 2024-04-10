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

#include "get_traffic_stats_by_network_context.h"

#include "napi_constant.h"
#include "napi_utils.h"
#include "netmanager_base_log.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *NET_BEAR_TYPE = "type";
constexpr const char *START_TIME = "startTime";
constexpr const char *END_TIME = "endTime";
constexpr const char *SIM_ID = "simId";
} // namespace
GetTrafficStatsByNetworkContext::GetTrafficStatsByNetworkContext(napi_env env, EventManager *manager)
    : BaseContext(env, manager) {}

void GetTrafficStatsByNetworkContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        NETMANAGER_BASE_LOGE("checkParamsType false");
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        SetNeedThrowException(true);
        return;
    }
    if (!CheckNetworkParams(params, paramsCount)) {
        return;
    }
    type_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_0], NET_BEAR_TYPE);
    start_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_0], START_TIME);
    end_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_0], END_TIME);
    bool hasSimId = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], SIM_ID);
    if (hasSimId) {
        simId_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_0], SIM_ID);
    } else {
        simId_ = UINT32_MAX;
    }
    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_1]) == napi_ok);
        return;
    }
    SetParseOK(true);
}

bool GetTrafficStatsByNetworkContext::CheckNetworkParams(napi_value *params, size_t paramsCount)
{
    bool hasNetBearType = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], NET_BEAR_TYPE);
    bool hasStart = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], START_TIME);
    bool hasEnd = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], END_TIME);
    bool hasSimId = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], SIM_ID);
    if (!(hasNetBearType && hasStart && hasEnd)) {
        NETMANAGER_BASE_LOGE("param error hasNetBearType=%{public}d, hasStart=%{public}d, hasEnd=%{public}d",
                             hasNetBearType, hasStart, hasEnd);
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        SetNeedThrowException(true);
        return false;
    }
    bool checkNetBearType = NapiUtils::GetValueType(GetEnv(),
                                                    NapiUtils::GetNamedProperty(GetEnv(),
                                                                                params[ARG_INDEX_0],
                                                                                NET_BEAR_TYPE)) == napi_number;
    bool checkStart = NapiUtils::GetValueType(GetEnv(),
                                              NapiUtils::GetNamedProperty(GetEnv(),
                                                                          params[ARG_INDEX_0],
                                                                          START_TIME)) == napi_number;
    bool checkEnd = NapiUtils::GetValueType(GetEnv(),
                                            NapiUtils::GetNamedProperty(GetEnv(),
                                                                        params[ARG_INDEX_0],
                                                                        END_TIME)) == napi_number;
    bool checkSimId = true;
    if (hasSimId) {
        checkSimId = NapiUtils::GetValueType(GetEnv(),
                                             NapiUtils::GetNamedProperty(GetEnv(),
                                                                         params[ARG_INDEX_0],
                                                                         SIM_ID)) == napi_number;
    }
    if (!(checkNetBearType && checkStart && checkEnd && checkSimId)) {
        NETMANAGER_BASE_LOGE(
            "param check checkType=%{public}d, checkStart=%{public}d, checkEnd=%{public}d, checkSimId=%{public}d",
            checkNetBearType, checkStart, checkEnd, checkSimId);
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        SetNeedThrowException(true);
        return false;
    }
    return true;
}

bool GetTrafficStatsByNetworkContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_JUST_OPTIONS) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_0]) == napi_object;
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_0]) == napi_object &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_1]) == napi_function;
    }
    return false;
}

void GetTrafficStatsByNetworkContext::SetType(uint32_t typ)
{
    type_ = typ;
}

void GetTrafficStatsByNetworkContext::SetStart(uint32_t startTime)
{
    start_ = startTime;
}

void GetTrafficStatsByNetworkContext::SetEnd(uint32_t endTime)
{
    end_ = endTime;
}

void GetTrafficStatsByNetworkContext::SetSimId(uint32_t simId)
{
    simId_ = simId;
}

uint32_t GetTrafficStatsByNetworkContext::GetType() const
{
    return type_;
}

uint32_t GetTrafficStatsByNetworkContext::GetStart() const
{
    return start_;
}

uint32_t GetTrafficStatsByNetworkContext::GetEnd() const
{
    return end_;
}

uint32_t GetTrafficStatsByNetworkContext::GetSimId() const
{
    return simId_;
}

std::vector<NetStatsInfo> &GetTrafficStatsByNetworkContext::GetNetStatsInfo()
{
    return stats_;
}

} // namespace NetManagerStandard
} // namespace OHOS