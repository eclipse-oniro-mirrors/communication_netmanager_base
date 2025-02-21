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

#include "net_stats_database_helper.h"

#include <cstdlib>
#include <filesystem>

#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"
#include "net_stats_database_defines.h"
#include "net_stats_info.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;
namespace {
NetStatsDatabaseHelper::SqlCallback sqlCallback = [](void *notUsed, int argc, char **argv, char **colName) {
    std::string data;
    for (int i = 0; i < argc; i++) {
        data.append(colName[i]).append(" = ").append(argv[i] ? argv[i] : "nullptr\n");
    }
    NETMGR_LOG_D("Recv data: %{public}s", data.c_str());
    return 0;
};

bool CheckFilePath(const std::string &fileName)
{
    char tmpPath[PATH_MAX] = {0};
    const auto pos = fileName.find_last_of('/');
    const auto dir = fileName.substr(0, pos);
    if (!realpath(dir.c_str(), tmpPath)) {
        NETMGR_LOG_E("Get realPath failed error: %{public}d, %{public}s", errno, strerror(errno));
        return false;
    }
    if (strcmp(tmpPath, dir.c_str()) != 0) {
        NETMGR_LOG_E("file name is illegal fileName: %{public}s, tmpPath: %{public}s", fileName.c_str(), tmpPath);
        return false;
    }
    return true;
}
} // namespace

NetStatsDatabaseHelper::NetStatsDatabaseHelper(const std::string &path)
{
    if (!CheckFilePath(path)) {
        return;
    }
    Open(path);
}

NetStatsDatabaseHelper::~NetStatsDatabaseHelper()
{
    Close();
    sqlite_ = nullptr;
}

int32_t NetStatsDatabaseHelper::ExecSql(const std::string &sql, void *recv, SqlCallback callback)
{
    char *errMsg = nullptr;
    int32_t ret = sqlite3_exec(sqlite_, sql.c_str(), callback, recv, &errMsg);
    NETMGR_LOG_D("EXEC SQL : %{public}s", sql.c_str());
    if (errMsg != nullptr) {
        NETMGR_LOG_E("Exec sql failed err:%{public}s", errMsg);
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK ? NETMANAGER_SUCCESS : NETMANAGER_ERROR;
}

int32_t NetStatsDatabaseHelper::CreateTable(const std::string &tableName, const std::string &tableInfo)
{
    std::string sql = "CREATE TABLE IF NOT EXISTS " + tableName + "(" + tableInfo + ");";
    int32_t ret = ExecSql(sql, nullptr, sqlCallback);
    if (ret != NETMANAGER_SUCCESS) {
        return STATS_ERR_CREATE_TABLE_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsDatabaseHelper::Open(const std::string &path)
{
    int32_t ret = sqlite3_open_v2(path.c_str(), &sqlite_,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    return ret == SQLITE_OK ? NETMANAGER_SUCCESS : NETMANAGER_ERROR;
}

int32_t NetStatsDatabaseHelper::InsertData(const std::string &tableName, const std::string &paramList,
                                           const NetStatsInfo &info)
{
    std::string params;
    int32_t paramCount = count(paramList.begin(), paramList.end(), ',') + 1;
    for (int32_t i = 0; i < paramCount; ++i) {
        params += "?";
        if (i != paramCount - 1) {
            params += ",";
        }
    }
    std::string sql = "INSERT INTO " + tableName + " (" + paramList + ") " + "VALUES" + " (" + params + ") ";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_WRITE_DATA_FAIL;
    }
    int32_t idx = 1;
    if (paramCount == UID_PARAM_NUM) {
        statement_.BindInt64(idx, info.uid_);
        ++idx;
    }
    statement_.BindText(idx, info.iface_);
    statement_.BindInt64(++idx, info.date_);
    statement_.BindInt64(++idx, info.rxBytes_);
    statement_.BindInt64(++idx, info.rxPackets_);
    statement_.BindInt64(++idx, info.txBytes_);
    statement_.BindInt64(++idx, info.txPackets_);
    if (paramCount == UID_PARAM_NUM) {
        statement_.BindText(++idx, info.ident_);
    }
    statement_.BindInt64(++idx, info.flag_);
    ret = statement_.Step();
    statement_.ResetStatementAndClearBindings();
    if (ret != SQLITE_DONE) {
        NETMGR_LOG_E("Step failed ret:%{public}d", ret);
        return STATS_ERR_WRITE_DATA_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsDatabaseHelper::SelectData(std::vector<NetStatsInfo> &infos, const std::string &tableName,
                                           uint64_t start, uint64_t end)
{
    infos.clear();
    std::string sql = "SELECT * FROM " + tableName + " t WHERE 1=1 AND t.Date >= ?" + " AND t.Date <= ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t idx = 1;
    ret = statement_.BindInt64(idx, start);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Bind int64 failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = statement_.BindInt64(++idx, end);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Bind int64 failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    return Step(infos);
}

int32_t NetStatsDatabaseHelper::SelectData(const uint32_t uid, uint64_t start, uint64_t end,
                                           std::vector<NetStatsInfo> &infos)
{
    infos.clear();
    std::string sql = "SELECT * FROM " + std::string(UID_TABLE) + " t WHERE 1=1 AND t.UID == ?" + " AND t.Date >= ?" +
                      " AND t.Date <= ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t idx = 1;
    ret = statement_.BindInt64(idx, uid);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Bind int32 failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = BindInt64(idx, start, end);
    if (ret != SQLITE_OK) {
        return ret;
    }
    return Step(infos);
}

int32_t NetStatsDatabaseHelper::SelectData(const std::string &iface, uint64_t start, uint64_t end,
                                           std::vector<NetStatsInfo> &infos)
{
    infos.clear();
    std::string sql = "SELECT * FROM " + std::string(IFACE_TABLE) + " t WHERE 1=1 AND t.IFace = ?" +
                      " AND t.Date >= ?" + " AND t.Date <= ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t idx = 1;
    ret = statement_.BindText(idx, iface);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Bind text failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = BindInt64(idx, start, end);
    if (ret != SQLITE_OK) {
        return ret;
    }
    return Step(infos);
}

int32_t NetStatsDatabaseHelper::SelectData(const std::string &iface, const uint32_t uid, uint64_t start, uint64_t end,
                                           std::vector<NetStatsInfo> &infos)
{
    infos.clear();
    std::string sql = "SELECT * FROM " + std::string(UID_TABLE) + " t WHERE 1=1 AND t.UID = ?" + " AND t.IFace = ?" +
                      " AND t.Date >= ?" + " AND t.Date <= ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t idx = 1;
    ret = statement_.BindInt64(idx, uid);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("bind int32 ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = statement_.BindText(++idx, iface);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Bind text failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = BindInt64(idx, start, end);
    if (ret != SQLITE_OK) {
        return ret;
    }
    return Step(infos);
}

int32_t NetStatsDatabaseHelper::QueryData(const std::string &tableName, const std::string &ident, uint64_t start,
                                          uint64_t end, std::vector<NetStatsInfo> &infos)
{
    infos.clear();
    std::string sql =
        "SELECT * FROM " + tableName + " t WHERE 1=1 AND t.Ident = ?" + " AND t.Date >= ?" + " AND t.Date <= ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t idx = 1;
    ret = statement_.BindText(idx, ident);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("bind text ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = BindInt64(idx, start, end);
    if (ret != SQLITE_OK) {
        return ret;
    }
    return Step(infos);
}

int32_t NetStatsDatabaseHelper::QueryData(const std::string &tableName, const uint32_t uid, const std::string &ident,
                                          uint64_t start, uint64_t end, std::vector<NetStatsInfo> &infos)
{
    infos.clear();
    std::string sql = "SELECT * FROM " + tableName + " t WHERE 1=1 AND T.UID = ? AND t.Ident = ?" + " AND t.Date >= ?" +
                      " AND t.Date <= ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t idx = 1;
    ret = statement_.BindInt64(idx, uid);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("bind int32 ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = statement_.BindText(++idx, ident);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("bind text ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = BindInt64(idx, start, end);
    if (ret != SQLITE_OK) {
        return ret;
    }
    return Step(infos);
}

int32_t NetStatsDatabaseHelper::DeleteData(const std::string &tableName, uint64_t start, uint64_t end)
{
    std::string sql =
        "DELETE FROM " + tableName + " WHERE Date >= " + std::to_string(start) + " AND Date <= " + std::to_string(end);
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::DeleteData(const std::string &tableName, uint64_t uid)
{
    std::string sql = "DELETE FROM " + tableName + " WHERE UID = ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_WRITE_DATA_FAIL;
    }
    int32_t idx = 1;
    statement_.BindInt64(idx, uid);
    ret = statement_.Step();
    statement_.ResetStatementAndClearBindings();
    if (ret != SQLITE_DONE) {
        NETMGR_LOG_E("Step failed ret:%{public}d", ret);
        return STATS_ERR_WRITE_DATA_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsDatabaseHelper::Close()
{
    int32_t ret = sqlite3_close_v2(sqlite_);
    return ret == SQLITE_OK ? NETMANAGER_SUCCESS : NETMANAGER_ERROR;
}

int32_t NetStatsDatabaseHelper::ClearData(const std::string &tableName)
{
    std::string sql = "DELETE FROM " + tableName;
    std::string shrinkMemSql = "PRAGMA shrink_memory";
    int32_t execSqlRet = ExecSql(sql, nullptr, sqlCallback);
    if (execSqlRet != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Delete data failed");
        return execSqlRet;
    }
    execSqlRet = ExecSql(shrinkMemSql, nullptr, sqlCallback);
    if (execSqlRet != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Delete data failed");
        return execSqlRet;
    }
    sql = "VACUUM";
    execSqlRet = ExecSql(sql, nullptr, sqlCallback);
    if (execSqlRet != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Delete data failed");
        return execSqlRet;
    }
    return ExecSql(shrinkMemSql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::Step(std::vector<NetStatsInfo> &infos)
{
    int32_t rc = statement_.Step();
    NETMGR_LOG_I("Step result:%{public}d", rc);
    while (rc != SQLITE_DONE) {
        if (rc != SQLITE_ROW) {
            NETMGR_LOG_E("sqlite step error: %{public}d", rc);
            statement_.ResetStatementAndClearBindings();
            return STATS_ERR_READ_DATA_FAIL;
        }
        int32_t i = 0;
        NetStatsInfo info;
        if (statement_.GetColumnCount() == UID_PARAM_NUM) {
            statement_.GetColumnInt(i, info.uid_);
            ++i;
        }
        statement_.GetColumnString(i, info.iface_);
        statement_.GetColumnLong(++i, info.date_);
        statement_.GetColumnLong(++i, info.rxBytes_);
        statement_.GetColumnLong(++i, info.rxPackets_);
        statement_.GetColumnLong(++i, info.txBytes_);
        statement_.GetColumnLong(++i, info.txPackets_);
        if (statement_.GetColumnCount() == UID_PARAM_NUM) {
            statement_.GetColumnString(++i, info.ident_);
        }
        statement_.GetColumnInt(++i, info.flag_);
        infos.emplace_back(info);
        rc = statement_.Step();
        NETMGR_LOG_D("Step result:%{public}d", rc);
    }
    statement_.ResetStatementAndClearBindings();
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsDatabaseHelper::BindInt64(int32_t idx, uint64_t start, uint64_t end)
{
    int32_t ret = statement_.BindInt64(++idx, start);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Bind int64 failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    ret = statement_.BindInt64(++idx, end);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Bind int64 failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }

    return ret;
}

int32_t NetStatsDatabaseHelper::Upgrade()
{
    auto ret = ExecTableUpgrade(UID_TABLE, Version_1);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Upgrade db failed. table is %{public}s, version is %{public}d", UID_TABLE, Version_1);
    }
    ret = ExecTableUpgrade(UID_SIM_TABLE, Version_2);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Upgrade db failed. table is %{public}s, version is %{public}d", UID_SIM_TABLE, Version_2);
    }
    ret = ExecTableUpgrade(UID_TABLE, Version_3);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Upgrade db failed. table is %{public}s, version is %{public}d", UID_TABLE, Version_3);
    }
    ret = ExecTableUpgrade(UID_SIM_TABLE, Version_3);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Upgrade db failed. table is %{public}s, version is %{public}d", UID_SIM_TABLE, Version_3);
    }
    ret = ExecTableUpgrade(UID_SIM_TABLE, Version_4);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Upgrade db failed. table is %{public}s, version is %{public}d", UID_SIM_TABLE, Version_4);
    }
    return ret;
}

int32_t NetStatsDatabaseHelper::ExecTableUpgrade(const std::string &tableName, TableVersion newVersion)
{
    TableVersion oldVersion;
    auto ret = GetTableVersion(oldVersion, tableName);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("ExecTableUpgrade getTableVersion failed. ret = %{public}d", ret);
        return NETMANAGER_ERROR;
    }
    if (oldVersion == newVersion) {
        return NETMANAGER_SUCCESS;
    }
    NETMGR_LOG_I("ExecTableUpgrade tableName = %{public}s, oldVersion = %{public}d, newVersion = %{public}d",
                 tableName.c_str(), oldVersion, newVersion);
    ExecUpgradeSql(tableName, oldVersion, newVersion);
    if (oldVersion != newVersion) {
        NETMGR_LOG_E("ExecTableUpgrade error. oldVersion = %{public}d, newVersion = %{public}d",
                     oldVersion, newVersion);
        return NETMANAGER_ERROR;
    }
    ret = UpdateTableVersion(oldVersion, tableName);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("ExecTableUpgrade updateVersion failed. ret = %{public}d", ret);
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}

void NetStatsDatabaseHelper::ExecUpgradeSql(const std::string &tableName, TableVersion &oldVersion,
                                            TableVersion newVersion)
{
    int32_t ret = NETMANAGER_SUCCESS;
    if (oldVersion < Version_1 && newVersion >= Version_1) {
        std::string sql = "ALTER TABLE " + tableName + " ADD COLUMN Ident CHAR(100) NOT NULL DEFAULT '';";
        ret = ExecSql(sql, nullptr, sqlCallback);
        if (ret != SQLITE_OK) {
            NETMGR_LOG_E("ExecTableUpgrade version_1 failed. ret = %{public}d", ret);
        }
        oldVersion = Version_1;
    }
    if (oldVersion < Version_2 && newVersion >= Version_2) {
        ret = DeleteData(tableName, Sim_UID);
        if (ret != SQLITE_OK) {
            NETMGR_LOG_E("ExecTableUpgrade Version_2 failed. ret = %{public}d", ret);
        }
        oldVersion = Version_2;
    }
    if (oldVersion < Version_3 && newVersion >= Version_3) {
        std::string sql = "ALTER TABLE " + tableName + " ADD COLUMN Flag INTEGER NOT NULL DEFAULT 0;";
        ret = ExecSql(sql, nullptr, sqlCallback);
        if (ret != SQLITE_OK) {
            NETMGR_LOG_E("ExecTableUpgrade version_3 failed. ret = %{public}d", ret);
        }
        oldVersion = Version_3;
    }
    if (oldVersion < Version_4 && newVersion >= Version_4) {
        std::string sql = "UPDATE " + tableName + " SET Flag = " + std::to_string(STATS_DATA_FLAG_SIM) +
                          " WHERE Flag = " + std::to_string(STATS_DATA_FLAG_DEFAULT) + ";";
        ret = ExecSql(sql, nullptr, sqlCallback);
        if (ret != SQLITE_OK) {
            NETMGR_LOG_E("ExecTableUpgrade Version_4 failed. ret = %{public}d", ret);
        }
        oldVersion = Version_4;
    }
}

int32_t NetStatsDatabaseHelper::GetTableVersion(TableVersion &version, const std::string &tableName)
{
    std::string sql = "SELECT * FROM " + std::string(VERSION_TABLE) + " WHERE Name = ?;";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("Prepare failed ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t idx = 1;
    ret = statement_.BindText(idx, tableName);
    if (ret != SQLITE_OK) {
        NETMGR_LOG_E("bind text ret:%{public}d", ret);
        return STATS_ERR_READ_DATA_FAIL;
    }
    int32_t rc = statement_.Step();
    auto v = static_cast<uint32_t>(Version_0);
    while (rc != SQLITE_DONE) {
        int32_t i = 1;
        statement_.GetColumnInt(i, v);
        rc = statement_.Step();
    }
    statement_.ResetStatementAndClearBindings();
    version = static_cast<TableVersion>(v);
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsDatabaseHelper::UpdateTableVersion(TableVersion version, const std::string &tableName)
{
    std::string sql = "INSERT OR REPLACE INTO "+ std::string(VERSION_TABLE) + "(Name, Version) VALUES('" +
                      tableName + "', " + std::to_string(version) + ");";
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::UpdateStatsFlag(const std::string &tableName, uint32_t uid, uint32_t flag)
{
    std::string sql = "UPDATE " + tableName + " SET Flag = " + std::to_string(flag) +
                      " WHERE UID = " + std::to_string(uid);
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::UpdateDataFlag(const std::string &tableName, uint32_t oldFlag, uint32_t newFlag)
{
    std::string sql =
        "UPDATE " + tableName + " SET Flag = " + std::to_string(newFlag) + " WHERE Flag = " + std::to_string(oldFlag);
    return ExecSql(sql, nullptr, sqlCallback);
}
} // namespace NetManagerStandard
} // namespace OHOS
