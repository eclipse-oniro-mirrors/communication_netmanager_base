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

#include "net_mgr_log_wrapper.h"
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
    std::filesystem::path file = fileName;
    auto dir = file.parent_path();
    if (!realpath(dir.string().c_str(), tmpPath)) {
        NETMGR_LOG_E("Get realPath failed error: %{public}d, %{public}s", errno, strerror(errno));
        return false;
    }
    if (strcmp(tmpPath, dir.string().c_str()) != 0) {
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
    return ret;
}

int32_t NetStatsDatabaseHelper::CreateTable(const std::string &tableName, const std::string &tableInfo)
{
    std::string sql = "CREATE TABLE IF NOT EXISTS " + tableName + "(" + tableInfo + ");";
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::Open(const std::string &path)
{
    return sqlite3_open_v2(path.c_str(), &sqlite_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                           nullptr);
}

int32_t NetStatsDatabaseHelper::InsertData(const std::string &tableName, const std::string &paramList,
                                           const std::string &params)
{
    std::string sql = "INSERT INTO " + tableName + " (" + paramList + ") " + "VALUES" + " (" + params + ") ";
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::SelectData(const std::string &tableName, void *recv, SqlCallback callback,
                                           uint64_t start, uint64_t end)
{
    std::string sql = "SELECT * FROM " + tableName + " t WHERE 1=1 AND t.Date >= " + std::to_string(start) +
                      " AND t.Date <= " + std::to_string(end);
    return ExecSql(sql, recv, callback);
}

int32_t NetStatsDatabaseHelper::SelectData(void *recv, SqlCallback callback, const uint32_t uid, uint64_t start,
                                           uint64_t end)
{
    std::string sql = "SELECT * FROM " + std::string(UID_TABLE) + " t WHERE 1=1 AND t.UID == " + std::to_string(uid) +
                      " AND t.Date >= " + std::to_string(start) + " AND t.Date <= " + std::to_string(end);
    return ExecSql(sql, recv, callback);
}

int32_t NetStatsDatabaseHelper::SelectData(void *recv, SqlCallback callback, const std::string &iface, uint64_t start,
                                           uint64_t end)
{
    std::string sql = "SELECT * FROM " + std::string(IFACE_TABLE) + " t WHERE 1=1 AND t.IFace = \"" + iface +
                      "\" AND t.Date >= " + std::to_string(start) + " AND t.Date <= " + std::to_string(end);
    return ExecSql(sql, recv, callback);
}

int32_t NetStatsDatabaseHelper::SelectData(void *recv, SqlCallback callback, const std::string &iface,
                                           const uint32_t uid, uint64_t start, uint64_t end)
{
    std::string sql = "SELECT * FROM " + std::string(UID_TABLE) + " t WHERE 1=1 AND t.UID = " + std::to_string(uid) +
                      " AND t.IFace = \"" + iface + "\" AND t.Date >= " + std::to_string(start) +
                      " AND t.Date <= " + std::to_string(end);
    return ExecSql(sql, recv, callback);
}

int32_t NetStatsDatabaseHelper::DeleteData(const std::string &tableName, uint64_t start, uint64_t end)
{
    std::string sql =
        "DELETE FROM " + tableName + " WHERE Date >= " + std::to_string(start) + " AND Date <= " + std::to_string(end);
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::DeleteData(const std::string &tableName, uint64_t uid)
{
    std::string sql = "DELETE FROM " + tableName + " WHERE UID = " + std::to_string(uid);
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t NetStatsDatabaseHelper::Close()
{
    return sqlite3_close_v2(sqlite_);
}

int32_t NetStatsDatabaseHelper::ClearData(const std::string &tableName)
{
    std::string sql = "DELETE FROM " + tableName;
    std::string shrinkMemSql = "PRAGMA shrink_memory";
    int32_t execSqlRet = ExecSql(sql, nullptr, sqlCallback);
    if (execSqlRet != 0) {
        NETMGR_LOG_E("Delete data failed");
        return execSqlRet;
    }
    execSqlRet = ExecSql(shrinkMemSql, nullptr, sqlCallback);
    if (execSqlRet != 0) {
        NETMGR_LOG_E("Delete data failed");
        return execSqlRet;
    }
    sql = "VACUUM";
    execSqlRet = ExecSql(sql, nullptr, sqlCallback);
    if (execSqlRet != 0) {
        NETMGR_LOG_E("Delete data failed");
        return execSqlRet;
    }
    execSqlRet = ExecSql(shrinkMemSql, nullptr, sqlCallback);
    return execSqlRet;
}
} // namespace NetManagerStandard
} // namespace OHOS
