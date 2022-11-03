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

#include "napi_utils.h"
#include "netmanager_base_log.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"
#include "statistics_observer_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const std::string RX_BYTES = "rxBytes";
const std::string TX_BYTES = "txBytes";
const std::string RX_PACKETS = "rxPackets";
const std::string TX_PACKETS = "txPackets";
} // namespace

bool StatisticsExec::ExecGetCellularRxBytes(GetCellularRxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularRxBytes());
    return true;
}

bool StatisticsExec::ExecGetCellularTxBytes(GetCellularTxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularTxBytes());
    return true;
}

bool StatisticsExec::ExecGetAllRxBytes(GetAllRxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetAllRxBytes());
    return true;
}

bool StatisticsExec::ExecGetAllTxBytes(GetAllTxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetAllTxBytes());
    return true;
}

bool StatisticsExec::ExecGetUidRxBytes(GetUidRxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetUidRxBytes(context->GetUid()));
    return true;
}

bool StatisticsExec::ExecGetUidTxBytes(GetUidTxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetUidTxBytes(context->GetUid()));
    return true;
}

bool StatisticsExec::ExecGetIfaceRxBytes(GetIfaceRxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceRxBytes(context->GetNic()));
    return true;
}

bool StatisticsExec::ExecGetIfaceTxBytes(GetIfaceTxBytesContext *context)
{
    context->SetBytes64(DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceTxBytes(context->GetNic()));
    return true;
}

napi_value StatisticsExec::GetCellularRxBytesCallback(GetCellularRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}

napi_value StatisticsExec::GetCellularTxBytesCallback(GetCellularTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}

napi_value StatisticsExec::GetAllRxBytesCallback(GetAllRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}

napi_value StatisticsExec::GetAllTxBytesCallback(GetAllTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}

napi_value StatisticsExec::GetUidRxBytesCallback(GetUidRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}

napi_value StatisticsExec::GetUidTxBytesCallback(GetUidTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}

napi_value StatisticsExec::GetIfaceRxBytesCallback(GetIfaceRxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}

napi_value StatisticsExec::GetIfaceTxBytesCallback(GetIfaceTxBytesContext *context)
{
    return NapiUtils::CreateInt64(context->GetEnv(), context->GetBytes64());
}
} // namespace NetManagerStandard
} // namespace OHOS
