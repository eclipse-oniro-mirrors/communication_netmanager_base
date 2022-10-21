/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "update_iface_stats_context.h"

#include "constant.h"
#include "napi_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string RX_BYTES = "rxBytes";
const std::string TX_BYTES = "txBytes";
const std::string RX_PACKETS = "rxPackets";
const std::string TX_PACKETS = "txPackets";
} // namespace

UpdateIfacesStatsContext::UpdateIfacesStatsContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void UpdateIfacesStatsContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        return;
    }

    interfaceName_ = NapiUtils::GetStringFromValueUtf8(GetEnv(), params[ARG_INDEX_0]);
    start_ = static_cast<uint32_t>(NapiUtils::GetInt32FromValue(GetEnv(), params[ARG_INDEX_1]));
    end_ = static_cast<uint32_t>(NapiUtils::GetInt32FromValue(GetEnv(), params[ARG_INDEX_2]));
    statsInfo_.rxBytes_ = NapiUtils::GetInt64Property(GetEnv(), params[ARG_INDEX_3], RX_BYTES);
    statsInfo_.txBytes_ = NapiUtils::GetInt64Property(GetEnv(), params[ARG_INDEX_3], TX_BYTES);
    statsInfo_.rxPackets_ = NapiUtils::GetInt64Property(GetEnv(), params[ARG_INDEX_3], RX_PACKETS);
    statsInfo_.txPackets_ = NapiUtils::GetInt64Property(GetEnv(), params[ARG_INDEX_3], TX_PACKETS);

    if (paramsCount == PARAM_FOUR_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_4]) == napi_ok);
        return;
    }
    SetParseOK(true);
}

bool UpdateIfacesStatsContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_FOUR_OPTIONS) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_0]) == napi_string &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_1]) == napi_number &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_2]) == napi_number &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_3]) == napi_object;
    }
    if (paramsCount == PARAM_FOUR_OPTIONS_AND_CALLBACK) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_4]) == napi_function &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_0]) == napi_string &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_1]) == napi_number &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_2]) == napi_number &&
               NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_3]) == napi_object;
    }
    return false;
}

void UpdateIfacesStatsContext::SetUid(int32_t uid)
{
    uid_ = uid;
}

void UpdateIfacesStatsContext::SetInterfaceName(std::string interfaceName)
{
    interfaceName_ = interfaceName;
}

void UpdateIfacesStatsContext::SetStatsInfo(NetStatsInfo statsInfo)
{
    statsInfo_ = statsInfo;
}

void UpdateIfacesStatsContext::SetStart(uint32_t start)
{
    start_ = start;
}

void UpdateIfacesStatsContext::SetEnd(uint32_t end)
{
    end_ = end;
}

int32_t UpdateIfacesStatsContext::GetUid()
{
    return uid_;
}
std::string UpdateIfacesStatsContext::GetInterfaceName()
{
    return interfaceName_;
}

NetStatsInfo UpdateIfacesStatsContext::GetStatsInfo()
{
    return statsInfo_;
}

uint32_t UpdateIfacesStatsContext::GetStart()
{
    return start_;
}

uint32_t UpdateIfacesStatsContext::GetEnd()
{
    return end_;
}
} // namespace NetManagerStandard
} // namespace OHOS
