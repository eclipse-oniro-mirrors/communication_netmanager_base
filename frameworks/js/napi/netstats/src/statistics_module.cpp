/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <string>

#include <napi/native_api.h>

#include "constant.h"
#include "get_cellular_rxbytes_context.h"
#include "get_iface_rxbytes_context.h"
#include "get_iface_stats_context.h"
#include "get_iface_uid_stats_context.h"
#include "get_traffic_stats_by_network_context.h"
#include "get_traffic_stats_by_uid_network_context.h"
#include "get_uid_rxbytes_context.h"
#include "module_template.h"
#include "napi_utils.h"
#include "statistics_async_work.h"
#include "statistics_callback_observer.h"
#include "statistics_exec.h"
#include "statistics_observer_wrapper.h"
#include "update_iface_stats_context.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *STATISTICS_MODULE_NAME = "net.statistics";

constexpr const char *FUNCTION_GET_CELLULAR_RXBYTES = "getCellularRxBytes";
constexpr const char *FUNCTION_GET_CELLULAR_TXBYTES = "getCellularTxBytes";
constexpr const char *FUNCTION_GET_ALL_RXBYTES = "getAllRxBytes";
constexpr const char *FUNCTION_GET_ALL_TXBYTES = "getAllTxBytes";
constexpr const char *FUNCTION_GET_UID_RXBYTES = "getUidRxBytes";
constexpr const char *FUNCTION_GET_UID_TXBYTES = "getUidTxBytes";
constexpr const char *FUNCTION_GET_IFACE_RXBYTES = "getIfaceRxBytes";
constexpr const char *FUNCTION_GET_IFACE_TXBYTES = "getIfaceTxBytes";
constexpr const char *FUNCTION_GET_IFACE_STATS = "getTrafficStatsByIface";
constexpr const char *FUNCTION_GET_IFACE_UID_STATS = "getTrafficStatsByUid";
constexpr const char *FUNCTION_UPDATE_IFACE_STATS = "updateIfacesStats";
constexpr const char *FUNCTION_UPDATE_STATS_DATA = "updateStatsData";
constexpr const char *FUNCTION_ON = "on";
constexpr const char *FUNCTION_OFF = "off";
constexpr const char *FUNCTION_GET_SOCKFD_RXBYTES = "getSockfdRxBytes";
constexpr const char *FUNCTION_GET_SOCKFD_TXBYTES = "getSockfdTxBytes";
constexpr const char *FUNCTION_GET_TRAFFIC_STATS_BY_NETWORK = "getTrafficStatsByNetwork";
constexpr const char *FUNCTION_GET_TRAFFIC_STATS_BY_UID_NETWORK = "getTrafficStatsByUidNetwork";
} // namespace

napi_value GetCellularRxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetCellularRxBytesContext>(env, info, FUNCTION_GET_CELLULAR_RXBYTES, nullptr,
                                                                StatisticsAsyncWork::ExecGetCellularRxBytes,
                                                                StatisticsAsyncWork::GetCellularRxBytesCallback);
}

napi_value GetCellularTxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetCellularTxBytesContext>(env, info, FUNCTION_GET_CELLULAR_TXBYTES, nullptr,
                                                                StatisticsAsyncWork::ExecGetCellularTxBytes,
                                                                StatisticsAsyncWork::GetCellularTxBytesCallback);
}

napi_value GetAllRxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAllRxBytesContext>(env, info, FUNCTION_GET_ALL_RXBYTES, nullptr,
                                                           StatisticsAsyncWork::ExecGetAllRxBytes,
                                                           StatisticsAsyncWork::GetAllRxBytesCallback);
}

napi_value GetAllTxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAllTxBytesContext>(env, info, FUNCTION_GET_ALL_TXBYTES, nullptr,
                                                           StatisticsAsyncWork::ExecGetAllTxBytes,
                                                           StatisticsAsyncWork::GetAllTxBytesCallback);
}

napi_value GetUidRxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetUidRxBytesContext>(env, info, FUNCTION_GET_UID_RXBYTES, nullptr,
                                                           StatisticsAsyncWork::ExecGetUidRxBytes,
                                                           StatisticsAsyncWork::GetUidRxBytesCallback);
}

napi_value GetUidTxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetUidTxBytesContext>(env, info, FUNCTION_GET_UID_TXBYTES, nullptr,
                                                           StatisticsAsyncWork::ExecGetUidTxBytes,
                                                           StatisticsAsyncWork::GetUidTxBytesCallback);
}

napi_value GetSockfdRxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetSockfdRxBytesContext>(env, info, FUNCTION_GET_SOCKFD_RXBYTES, nullptr,
                                                              StatisticsAsyncWork::ExecGetSockfdRxBytes,
                                                              StatisticsAsyncWork::GetSockfdRxBytesCallback);
}

napi_value GetSockfdTxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetSockfdTxBytesContext>(env, info, FUNCTION_GET_SOCKFD_TXBYTES, nullptr,
                                                              StatisticsAsyncWork::ExecGetSockfdTxBytes,
                                                              StatisticsAsyncWork::GetSockfdTxBytesCallback);
}

napi_value GetIfaceRxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetIfaceRxBytesContext>(env, info, FUNCTION_GET_IFACE_RXBYTES, nullptr,
                                                             StatisticsAsyncWork::ExecGetIfaceRxBytes,
                                                             StatisticsAsyncWork::GetIfaceRxBytesCallback);
}

napi_value GetIfaceTxBytes(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetIfaceTxBytesContext>(env, info, FUNCTION_GET_IFACE_TXBYTES, nullptr,
                                                             StatisticsAsyncWork::ExecGetIfaceTxBytes,
                                                             StatisticsAsyncWork::GetIfaceTxBytesCallback);
}

napi_value GetIfaceStats(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetIfaceStatsContext>(env, info, FUNCTION_GET_IFACE_STATS, nullptr,
                                                           StatisticsAsyncWork::ExecGetIfaceStats,
                                                           StatisticsAsyncWork::GetIfaceStatsCallback);
}

napi_value GetIfaceUidStats(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetIfaceUidStatsContext>(env, info, FUNCTION_GET_IFACE_UID_STATS, nullptr,
                                                              StatisticsAsyncWork::ExecGetIfaceUidStats,
                                                              StatisticsAsyncWork::GetIfaceUidStatsCallback);
}

napi_value GetTrafficStatsByNetwork(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetTrafficStatsByNetworkContext>(
        env, info, FUNCTION_GET_TRAFFIC_STATS_BY_NETWORK, nullptr, StatisticsAsyncWork::ExecGetTrafficStatsByNetwork,
        StatisticsAsyncWork::GetTrafficStatsByNetworkCallback);
}

napi_value GetTrafficStatsByUidNetwork(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetTrafficStatsByUidNetworkContext>(
        env, info, FUNCTION_GET_TRAFFIC_STATS_BY_UID_NETWORK, nullptr,
        StatisticsAsyncWork::ExecGetTrafficStatsByUidNetwork, StatisticsAsyncWork::GetTrafficStatsByUidNetworkCallback);
}

napi_value UpdateIfacesStats(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UpdateIfacesStatsContext>(env, info, FUNCTION_UPDATE_IFACE_STATS, nullptr,
                                                               StatisticsAsyncWork::ExecUpdateIfacesStats,
                                                               StatisticsAsyncWork::UpdateIfacesStatsCallback);
}

napi_value UpdateStatsData(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UpdateStatsDataContext>(env, info, FUNCTION_UPDATE_STATS_DATA, nullptr,
                                                             StatisticsAsyncWork::ExecUpdateStatsData,
                                                             StatisticsAsyncWork::UpdateStatsDataCallback);
}

napi_value On(napi_env env, napi_callback_info info)
{
    return StatisticsObserverWrapper::GetInstance().On(env, info, {EVENT_STATS_CHANGE}, false);
}

napi_value Off(napi_env env, napi_callback_info info)
{
    return StatisticsObserverWrapper::GetInstance().Off(env, info, {EVENT_STATS_CHANGE}, false);
}

napi_value InitStatisticsModule(napi_env env, napi_value exports)
{
    NapiUtils::DefineProperties(
        env, exports,
        {
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_CELLULAR_RXBYTES, GetCellularRxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_CELLULAR_TXBYTES, GetCellularTxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_ALL_RXBYTES, GetAllRxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_ALL_TXBYTES, GetAllTxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_UID_RXBYTES, GetUidRxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_UID_TXBYTES, GetUidTxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_SOCKFD_RXBYTES, GetSockfdRxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_SOCKFD_TXBYTES, GetSockfdTxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_IFACE_RXBYTES, GetIfaceRxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_IFACE_TXBYTES, GetIfaceTxBytes),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_IFACE_STATS, GetIfaceStats),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_IFACE_UID_STATS, GetIfaceUidStats),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_TRAFFIC_STATS_BY_NETWORK, GetTrafficStatsByNetwork),
            DECLARE_NAPI_FUNCTION(FUNCTION_GET_TRAFFIC_STATS_BY_UID_NETWORK, GetTrafficStatsByUidNetwork),
            DECLARE_NAPI_FUNCTION(FUNCTION_UPDATE_IFACE_STATS, UpdateIfacesStats),
            DECLARE_NAPI_FUNCTION(FUNCTION_UPDATE_STATS_DATA, UpdateStatsData),
            DECLARE_NAPI_FUNCTION(FUNCTION_ON, On),
            DECLARE_NAPI_FUNCTION(FUNCTION_OFF, Off),
        });
    return exports;
}

static napi_module g_statisticsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitStatisticsModule,
    .nm_modname = STATISTICS_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void RegisterNetStatsModule(void)
{
    napi_module_register(&g_statisticsModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
