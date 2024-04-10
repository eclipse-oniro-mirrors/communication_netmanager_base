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

#include "get_traffic_stats_by_uid_network_context.h"

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

GetTrafficStatsByUidNetworkContext::GetTrafficStatsByUidNetworkContext(napi_env env, EventManager *manager)
    : BaseContext(env, manager) {}

void GetTrafficStatsByUidNetworkContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        NETMANAGER_BASE_LOGE("checkParamsType false");
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        SetNeedThrowException(true);
        return;
    }
    uid_ = NapiUtils::GetUint32FromValue(GetEnv(), params[ARG_INDEX_0]);
    if (uid_ <= 0) {
        NETMANAGER_BASE_LOGE("uid invalid");
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        SetNeedThrowException(true);
        return;
    }
    if (!CheckNetworkParams(params, paramsCount)) {
        return;
    }
    type_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_1], NET_BEAR_TYPE);
    start_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_1], START_TIME);
    end_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_1], END_TIME);
    bool hasSimId = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_1], SIM_ID);
    if (hasSimId) {
        simId_ = NapiUtils::GetUint32Property(GetEnv(), params[ARG_INDEX_1], SIM_ID);
    } else {
        simId_ = UINT32_MAX;
    }
    if (paramsCount == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_2]) == napi_ok);
        return;
    }
    SetParseOK(true);
}

bool GetTrafficStatsByUidNetworkContext::CheckNetworkParams(napi_value *params, size_t paramsCount)
{
    bool hasNetBearType = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_1], NET_BEAR_TYPE);
    bool hasStart = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_1], START_TIME);
    bool hasEnd = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_1], END_TIME);
    bool hasSimId = NapiUtils::HasNamedProperty(GetEnv(), params[ARG_INDEX_1], SIM_ID);
    if (!(hasNetBearType && hasStart && hasEnd)) {
        NETMANAGER_BASE_LOGE("param error hasNetBearType=%{public}d, hasStart=%{public}d, hasEnd=%{public}d",
                             hasNetBearType, hasStart, hasEnd);
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        SetNeedThrowException(true);
        return false;
    }
    bool checkNetBearType = NapiUtils::GetValueType(GetEnv(),
                                                    NapiUtils::GetNamedProperty(GetEnv(),
                                                                                params[ARG_INDEX_1],
                                                                                NET_BEAR_TYPE)) == napi_number;
    bool checkStart = NapiUtils::GetValueType(GetEnv(),
                                              NapiUtils::GetNamedProperty(GetEnv(),
                                                                          params[ARG_INDEX_1],
                                                                          START_TIME)) == napi_number;
    bool checkEnd = NapiUtils::GetValueType(GetEnv(),
                                            NapiUtils::GetNamedProperty(GetEnv(),
                                                                        params[ARG_INDEX_1],
                                                                        END_TIME)) == napi_number;
    bool checkSimId = true;
    if (hasSimId) {
        checkSimId = NapiUtils::GetValueType(GetEnv(),
                                             NapiUtils::GetNamedProperty(GetEnv(),
                                                                         params[ARG_INDEX_1],
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

bool GetTrafficStatsByUidNetworkContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_DOUBLE_OPTIONS) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_0]) == napi_number &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_1]) == napi_object;
    }
    if (paramsCount == PARAM_DOUBLE_OPTIONS_AND_CALLBACK) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_0]) == napi_number &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_1]) == napi_object &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_2]) == napi_function;
    }
    return false;
}

void GetTrafficStatsByUidNetworkContext::SetUid(uint32_t uid)
{
    uid_ = uid;
}

void GetTrafficStatsByUidNetworkContext::SetType(uint32_t typ)
{
    type_ = typ;
}

void GetTrafficStatsByUidNetworkContext::SetStart(uint32_t startTime)
{
    start_ = startTime;
}

void GetTrafficStatsByUidNetworkContext::SetEnd(uint32_t endTime)
{
    end_ = endTime;
}

void GetTrafficStatsByUidNetworkContext::SetSimId(uint32_t simId)
{
    simId_ = simId;
}

uint32_t GetTrafficStatsByUidNetworkContext::GetUid() const
{
    return uid_;
}

uint32_t GetTrafficStatsByUidNetworkContext::GetType() const
{
    return type_;
}

uint32_t GetTrafficStatsByUidNetworkContext::GetStart() const
{
    return start_;
}

uint32_t GetTrafficStatsByUidNetworkContext::GetEnd() const
{
    return end_;
}

uint32_t GetTrafficStatsByUidNetworkContext::GetSimId() const
{
    return simId_;
}

std::vector<NetStatsInfoSequence> &GetTrafficStatsByUidNetworkContext::GetNetStatsInfoSequence()
{
    return stats_;
}

} // namespace NetManagerStandard
} // namespace OHOS