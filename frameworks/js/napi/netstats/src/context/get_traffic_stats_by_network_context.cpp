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
    netBearType_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_0], NET_BEAR_TYPE);
    startTime_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_0], START_TIME);
    endTime_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_0], END_TIME);
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
    bool hasStartTime = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], START_TIME);
    bool hasEndTime = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], END_TIME);
    bool hasSimId = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_0], SIM_ID);
    if (!(hasNetBearType && hasStartTime && hasEndTime)) {
        NETMANAGER_BASE_LOGE("param error hasNetBearType=%{public}d, hasStartTime=%{public}d, hasEndTime=%{public}d",
                             hasNetBearType, hasStartTime, hasEndTime);
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        SetNeedThrowException(true);
        return false;
    }
    bool checkNetBearType = NapiUtils::GetValueType(
        GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), params[ARG_INDEX_0], NET_BEAR_TYPE)) == napi_number;
    bool checkStartTime = NapiUtils::GetValueType(
        GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), params[ARG_INDEX_0], START_TIME)) == napi_number;
    bool checkEndTime = NapiUtils::GetValueType(
        GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), params[ARG_INDEX_0], END_TIME)) == napi_number;
    bool checkSimId = true;
    if (hasSimId) {
        checkSimId = NapiUtils::GetValueType(
            GetEnv(), NapiUtils::GetNamedProperty(GetEnv(), params[ARG_INDEX_0], SIM_ID)) == napi_number;
    }
    if (!(checkNetBearType && checkStartTime && checkEndTime && checkSimId)) {
        NETMANAGER_BASE_LOGE(
            "param check checkType=%{public}d, checkStartTime=%{public}d, checkEndTime=%{public}d, "
            "checkSimId=%{public}d",
            checkNetBearType, checkStartTime, checkEndTime, checkSimId);
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

void GetTrafficStatsByNetworkContext::SetNetBearType(uint32_t bearerType)
{
    netBearType_ = bearerType;
}

void GetTrafficStatsByNetworkContext::SetStartTime(uint32_t startTime)
{
    startTime_ = startTime;
}

void GetTrafficStatsByNetworkContext::SetEndTime(uint32_t endTime)
{
    endTime_ = endTime;
}

void GetTrafficStatsByNetworkContext::SetSimId(uint32_t simId)
{
    simId_ = simId;
}

uint32_t GetTrafficStatsByNetworkContext::GetNetBearType() const
{
    return netBearType_;
}

uint32_t GetTrafficStatsByNetworkContext::GetStartTime() const
{
    return startTime_;
}

uint32_t GetTrafficStatsByNetworkContext::GetEndTime() const
{
    return endTime_;
}

uint32_t GetTrafficStatsByNetworkContext::GetSimId() const
{
    return simId_;
}

std::unordered_map<uint32_t, NetStatsInfo> &GetTrafficStatsByNetworkContext::GetNetStatsInfo()
{
    return stats_;
}

} // namespace NetManagerStandard
} // namespace OHOS