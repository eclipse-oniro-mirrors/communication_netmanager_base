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

#include "statistics_exec.h"

#include "errorcode_convertor.h"
#include "napi_utils.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"
#include "netmanager_base_log.h"
#include "statistics_observer_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string RX_BYTES = "rxBytes";
const std::string TX_BYTES = "txBytes";
const std::string RX_PACKETS = "rxPackets";
const std::string TX_PACKETS = "txPackets";

std::unique_ptr<ErrorCodeConvertor> g_covert = std::make_unique<NetBaseErrorCodeConvertor>();

std::string ConvertErrorCode(int32_t code)
{
    return g_covert->ConvertErrorCode(code);
}
} // namespace

bool StatisticsExec::ExecGetCellularRxBytes(GetCellularRxBytesContext *context)
{
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularRxBytes(context->bytes64_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

bool StatisticsExec::ExecGetCellularTxBytes(GetCellularTxBytesContext *context)
{
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularTxBytes(context->bytes64_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

bool StatisticsExec::ExecGetAllRxBytes(GetAllRxBytesContext *context)
{
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllRxBytes(context->bytes64_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

bool StatisticsExec::ExecGetAllTxBytes(GetAllTxBytesContext *context)
{
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllTxBytes(context->bytes64_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

bool StatisticsExec::ExecGetUidRxBytes(GetUidRxBytesContext *context)
{
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidRxBytes(context->bytes64_, context->uid_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

bool StatisticsExec::ExecGetUidTxBytes(GetUidTxBytesContext *context)
{
    int32_t result = DelayedSingleton<NetStatsClient>::GetInstance()->GetUidTxBytes(context->bytes64_, context->uid_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

bool StatisticsExec::ExecGetIfaceRxBytes(GetIfaceRxBytesContext *context)
{
    int32_t result =
        DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceRxBytes(context->bytes64_, context->interfaceName_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

bool StatisticsExec::ExecGetIfaceTxBytes(GetIfaceTxBytesContext *context)
{
    auto instance = DelayedSingleton<NetStatsClient>::GetInstance();
    int32_t result = instance->GetIfaceTxBytes(context->bytes64_, context->interfaceName_);
    context->SetError(result, ConvertErrorCode(result));
    return result == NETMANAGER_SUCCESS;
}

napi_value StatisticsExec::GetCellularRxBytesCallback(GetCellularRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}

napi_value StatisticsExec::GetCellularTxBytesCallback(GetCellularTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}

napi_value StatisticsExec::GetAllRxBytesCallback(GetAllRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}

napi_value StatisticsExec::GetAllTxBytesCallback(GetAllTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}

napi_value StatisticsExec::GetUidRxBytesCallback(GetUidRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}

napi_value StatisticsExec::GetUidTxBytesCallback(GetUidTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}

napi_value StatisticsExec::GetIfaceRxBytesCallback(GetIfaceRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}

napi_value StatisticsExec::GetIfaceTxBytesCallback(GetIfaceTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->bytes64_);
}
} // namespace NetManagerStandard
} // namespace OHOS
