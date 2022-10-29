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

#include "statistics_async_work.h"

#include "base_async_work.h"
#include "get_cellular_rxbeytes_contex.h"
#include "get_iface_rxbeytes_context.h"
#include "get_iface_stats_context.h"
#include "get_iface_uid_stats_context.h"
#include "get_uid_rxbeytes_context.h"
#include "statistics_exec.h"
#include "update_iface_stats_context.h"

namespace OHOS {
namespace NetManagerStandard {
void StatisticsAsyncWork::ExecGetCellularRxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetCellularRxBytesContext, StatisticsExec::ExecGetCellularRxBytes>(env, data);
}

void StatisticsAsyncWork::ExecGetCellularTxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetCellularTxBytesContext, StatisticsExec::ExecGetCellularTxBytes>(env, data);
}

void StatisticsAsyncWork::ExecGetAllRxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetAllRxBytesContext, StatisticsExec::ExecGetAllRxBytes>(env, data);
}

void StatisticsAsyncWork::ExecGetAllTxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetAllTxBytesContext, StatisticsExec::ExecGetAllTxBytes>(env, data);
}

void StatisticsAsyncWork::ExecGetUidRxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetUidRxBytesContext, StatisticsExec::ExecGetUidRxBytes>(env, data);
}

void StatisticsAsyncWork::ExecGetUidTxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetUidTxBytesContext, StatisticsExec::ExecGetUidTxBytes>(env, data);
}

void StatisticsAsyncWork::ExecGetIfaceRxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetIfaceRxBytesContext, StatisticsExec::ExecGetIfaceRxBytes>(env, data);
}

void StatisticsAsyncWork::ExecGetIfaceTxBytes(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetIfaceTxBytesContext, StatisticsExec::ExecGetIfaceTxBytes>(env, data);
}

void StatisticsAsyncWork::GetCellularRxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetCellularRxBytesContext, StatisticsExec::GetCellularRxBytesCallback>(env, status,
                                                                                                            data);
}

void StatisticsAsyncWork::GetCellularTxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetCellularTxBytesContext, StatisticsExec::GetCellularTxBytesCallback>(env, status,
                                                                                                            data);
}

void StatisticsAsyncWork::GetAllRxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetAllRxBytesContext, StatisticsExec::GetAllRxBytesCallback>(env, status, data);
}

void StatisticsAsyncWork::GetAllTxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetAllTxBytesContext, StatisticsExec::GetAllTxBytesCallback>(env, status, data);
}

void StatisticsAsyncWork::GetUidRxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetUidRxBytesContext, StatisticsExec::GetUidRxBytesCallback>(env, status, data);
}

void StatisticsAsyncWork::GetUidTxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetUidTxBytesContext, StatisticsExec::GetUidTxBytesCallback>(env, status, data);
}
void StatisticsAsyncWork::GetIfaceRxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetIfaceRxBytesContext, StatisticsExec::GetIfaceRxBytesCallback>(env, status,
                                                                                                      data);
}

void StatisticsAsyncWork::GetIfaceTxBytesCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetIfaceTxBytesContext, StatisticsExec::GetIfaceTxBytesCallback>(env, status,
                                                                                                      data);
}
} // namespace NetManagerStandard
} // namespace OHOS
