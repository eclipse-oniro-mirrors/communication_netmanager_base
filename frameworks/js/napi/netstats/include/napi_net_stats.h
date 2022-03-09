/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NAPI_NET_STATS_H
#define NAPI_NET_STATS_H

#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "net_stats_info.h"
#include "base_context.h"

namespace OHOS {
namespace NetManagerStandard {
// netstats async context
struct NetStatsAsyncContext : BaseContext {
    int32_t eventStatsId = 0;
    // result of bytes
    int64_t bytes64 = 0;
    // js parameter
    uint32_t uid = 0;
    // js parameter
    std::string interfaceName;
    uint32_t start;
    uint32_t end;
    NetStatsInfo statsInfo;
    int32_t result;
};

class NapiNetStats {
public:
    NapiNetStats();
    ~NapiNetStats() = default;
    static napi_value RegisterNetStatsInterface(napi_env env, napi_value exports);
    static napi_value DeclareNapiNetStatsInterface(napi_env env, napi_value exports);

    static void ExecGetCellularRxBytes(napi_env env, void *data);
    static void ExecGetCellularTxBytes(napi_env env, void *data);
    static void ExecGetAllRxBytes(napi_env env, void *data);
    static void ExecGetAllTxBytes(napi_env env, void *data);
    static void ExecGetUidRxBytes(napi_env env, void *data);
    static void ExecGetUidTxBytes(napi_env env, void *data);
    static void ExecGetIfaceRxBytes(napi_env env, void *data);
    static void ExecGetIfaceTxBytes(napi_env env, void *data);
    static void ExecGetIfaceStatsDetail(napi_env env, void *data);
    static void ExecGetUidStatsDetail(napi_env env, void *data);
    static void ExecUpdateIfacesStats(napi_env env, void *data);
    static void ExecUpdateStatsData(napi_env env, void *data);
    static void ExecOn(napi_env env, void *data);
    static void ExecOff(napi_env env, void *data);
    static void CompleteGetCellularRxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetCellularTxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetAllRxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetAllTxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetUidRxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetUidTxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetIfaceRxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetIfaceTxBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetBytes(napi_env env, napi_status status, void *data);
    static void CompleteGetIfaceStatsDetail(napi_env env, napi_status status, void *data);
    static void CompleteGetUidStatsDetail(napi_env env, napi_status status, void *data);
    static void CompleteUpdateIfacesStats(napi_env env, napi_status status, void *data);
    static void CompleteUpdateStatsData(napi_env env, napi_status status, void *data);
    static void CompleteOn(napi_env env, napi_status status, void *data);
    static void CompleteOff(napi_env env, napi_status status, void *data);
    // Declare napi interfaces for JS
    static napi_value GetCellularRxBytes(napi_env env, napi_callback_info info);
    static napi_value GetCellularTxBytes(napi_env env, napi_callback_info info);
    static napi_value GetAllRxBytes(napi_env env, napi_callback_info info);
    static napi_value GetAllTxBytes(napi_env env, napi_callback_info info);
    static napi_value GetUidRxBytes(napi_env env, napi_callback_info info);
    static napi_value GetUidTxBytes(napi_env env, napi_callback_info info);
    static napi_value GetIfaceRxBytes(napi_env env, napi_callback_info info);
    static napi_value GetIfaceTxBytes(napi_env env, napi_callback_info info);
    static napi_value GetIfaceStatsDetail(napi_env env, napi_callback_info info);
    static napi_value GetUidStatsDetail(napi_env env, napi_callback_info info);
    static napi_value UpdateIfacesStats(napi_env env, napi_callback_info info);
    static napi_value UpdateStatsData(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NET_STATS_H