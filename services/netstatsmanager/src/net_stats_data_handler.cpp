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

#include "net_stats_data_handler.h"

#include "net_mgr_log_wrapper.h"
#include "net_stats_database_defines.h"
#include "net_stats_database_helper.h"
#include "net_stats_constants.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;
namespace {
NetStatsDatabaseHelper::SqlCallback dataHandler = [](void *recv, int argc, char **argv, char **colName) {
    if (recv == nullptr) {
        NETMGR_LOG_E("Unable to save data");
        return static_cast<int32_t>(STATS_ERR_DATABASE_RECV_NO_DATA);
    }
    StatsDataBuilder builder;
    for (int i = 0; i < argc; i++) {
        builder[i] = argv[i];
    }
    auto data = static_cast<std::vector<NetStatsInfo> *>(recv);
    data->emplace_back(builder.Build(argc == UID_PARAM_NUM ? DataType::UID : DataType::IFACE));
    NETMGR_LOG_D("INFO: %{public}s", data->back().IfaceData().c_str());
    return static_cast<int32_t>(NETMANAGER_SUCCESS);
};
} // namespace

NetStatsDataHandler::NetStatsDataHandler() = default;

NetStatsDataHandler::~NetStatsDataHandler() = default;

int32_t NetStatsDataHandler::ReadStatsData(std::vector<NetStatsInfo> &infos, uint64_t start, uint64_t end)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    return helper->SelectData(UID_TABLE, &infos, dataHandler, start, end);
}

int32_t NetStatsDataHandler::ReadStatsData(std::vector<NetStatsInfo> &infos, uint64_t uid, uint64_t start, uint64_t end)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    return helper->SelectData(&infos, dataHandler, uid, start, end);
}

int32_t NetStatsDataHandler::ReadStatsData(std::vector<NetStatsInfo> &infos, const std::string &iface, uint64_t start,
                                           uint64_t end)
{
    if (iface.empty()) {
        NETMGR_LOG_E("Param is invalid");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    return helper->SelectData(&infos, dataHandler, iface, start, end);
}

int32_t NetStatsDataHandler::ReadStatsData(std::vector<NetStatsInfo> &infos, const std::string &iface,
                                           const uint32_t uid, uint64_t start, uint64_t end)
{
    if (iface.empty()) {
        NETMGR_LOG_E("Param is invalid");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    return helper->SelectData(&infos, dataHandler, iface, uid, start, end);
}

int32_t NetStatsDataHandler::WriteStatsData(const std::vector<NetStatsInfo> &infos, const std::string &tableName)
{
    if (infos.empty() || tableName.empty()) {
        NETMGR_LOG_E("Param wrong, info: %{public}zu, tableName: %{public}zu", infos.size(), tableName.size());
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    if (tableName == UID_TABLE) {
        std::for_each(infos.begin(), infos.end(), [&helper](const auto &info) {
            helper->InsertData(UID_TABLE, UID_TABLE_PARAM_LIST, info.UidData());
        });
        return NETMANAGER_SUCCESS;
    }
    if (tableName == IFACE_TABLE) {
        std::for_each(infos.begin(), infos.end(), [&helper](const auto &info) {
            helper->InsertData(IFACE_TABLE, IFACE_TABLE_PARAM_LIST, info.IfaceData());
        });
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_ERR_PARAMETER_ERROR;
}

int32_t NetStatsDataHandler::DeleteByUid(uint64_t uid)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    return helper->DeleteData(UID_TABLE, uid);
}

int32_t NetStatsDataHandler::DeleteByDate(const std::string &tableName, uint64_t start, uint64_t end)
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    return helper->DeleteData(tableName, start, end);
}

int32_t NetStatsDataHandler::ClearData()
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    int32_t ifaceDataRet = helper->ClearData(IFACE_TABLE);
    int32_t uidDataRet = helper->ClearData(UID_TABLE);
    return ifaceDataRet + uidDataRet;
}
} // namespace NetManagerStandard
} // namespace OHOS
