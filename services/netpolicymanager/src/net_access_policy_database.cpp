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

#include "net_access_policy_rdb.h"
#include <unistd.h>

#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
static const int32_t RDB_VERSION = 0;
const std::string DATABASE_NAME = "/data/service/el1/public/netmanager/net_uid_access_policy.db";
const std::string NETMANAGER_DB_UID_ACCESS_POLICY_TABLE = "uid_access_policy_infos";
const std::string SQL_TABLE_COLUMS = std::string(
    "uid INTEGER NOT NULL PRIMARY KEY, "
    "wifiPolicy INTEGER NOT NULL, cellularPolicy INTEGER NOT NULL, setFromConfigFlag INTEGER NOT NULL");
} // namespace

int NetAccessPolicyRDB::RdbDataOpenCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    return NETMANAGER_SUCCESS;
}

int NetAccessPolicyRDB::RdbDataOpenCallback::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetAccessPolicyRDB::InitRdbStore()
{
    int errCode = NETMANAGER_SUCCESS;
    NativeRdb::RdbStoreConfig config(DATABASE_NAME);
    NetAccessPolicyRDB::RdbDataOpenCallback helper;
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, RDB_VERSION, helper, errCode);
    if (rdbStore_ == nullptr) {
        NETMGR_LOG_E("RDB create failed");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    std::string createTable =
        "CREATE TABLE IF NOT EXISTS " + NETMANAGER_DB_UID_ACCESS_POLICY_TABLE + " (" + SQL_TABLE_COLUMS + ")";
    rdbStore_->ExecuteSql(createTable);

    NETMGR_LOG_D("InitRdbStore");
    return NETMANAGER_SUCCESS;
}

int32_t NetAccessPolicyRDB::InsertData(NetAccessPolicyData policy)
{
    NETMGR_LOG_D("InsertData");
    if (rdbStore_ == nullptr) {
        NETMGR_LOG_E("error: rdbStore_ is nullptr");
        return NETMANAGER_ERROR;
    }

    int ret = NETMANAGER_SUCCESS;
    NativeRdb::ValuesBucket policyValues;
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_UID, policy.uid);
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_WIFI_POLICY, policy.wifiPolicy);
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_CELLULAR_POLICY, policy.cellularPolicy);
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_SET_FROM_CONFIG_FLAG, policy.setFromConfigFlag);

    int64_t id = 0;
    ret = rdbStore_->Insert(id, NETMANAGER_DB_UID_ACCESS_POLICY_TABLE, policyValues);
    if (ret != NativeRdb::E_OK) {
        ret = UpdateByUid(policy.uid, policy);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Update operation failed, result is %{public}d", ret);
            return NETMANAGER_ERROR;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetAccessPolicyRDB::DeleteByUid(const int32_t uid)
{
    NETMGR_LOG_D("DeleteByUid");
    if (rdbStore_ == nullptr) {
        NETMGR_LOG_E("%{public}s: rdb store is not initialized", __func__);
        return NETMANAGER_ERROR;
    }
    int32_t deletedRows = -1;
    std::vector<std::string> whereArgs;
    OHOS::NativeRdb::RdbPredicates rdbPredicate{NETMANAGER_DB_UID_ACCESS_POLICY_TABLE};
    rdbPredicate.EqualTo(NetAccessPolicyRdbFiledConst::FILED_UID, std::to_string(uid));
    int32_t result = rdbStore_->Delete(deletedRows, rdbPredicate);
    if (result != NativeRdb::E_OK) {
        NETMGR_LOG_E("delete operation failed, result is %{public}d", result);
        return result;
    }

    return deletedRows;
}

int32_t NetAccessPolicyRDB::UpdateByUid(int32_t uid, NetAccessPolicyData policy)
{
    NETMGR_LOG_D("UpdateByUid");
    if (rdbStore_ == nullptr) {
        NETMGR_LOG_E("%{public}s: rdb store is not initialized", __func__);
        return NETMANAGER_ERROR;
    }

    OHOS::NativeRdb::RdbPredicates rdbPredicate{NETMANAGER_DB_UID_ACCESS_POLICY_TABLE};
    rdbPredicate.EqualTo(NetAccessPolicyRdbFiledConst::FILED_UID, std::to_string(uid));

    NativeRdb::ValuesBucket policyValues;
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_UID, uid);
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_WIFI_POLICY, policy.wifiPolicy);
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_CELLULAR_POLICY, policy.cellularPolicy);
    policyValues.PutInt(NetAccessPolicyRdbFiledConst::FILED_SET_FROM_CONFIG_FLAG, policy.setFromConfigFlag);

    int32_t rowId = -1;
    int32_t result = rdbStore_->Update(rowId, policyValues, rdbPredicate);
    if (result != NativeRdb::E_OK) {
        NETMGR_LOG_E("Update operation failed. Result %{public}d", result);
        return result;
    }

    return NETMANAGER_SUCCESS;
}

std::vector<NetAccessPolicyData> NetAccessPolicyRDB::QueryAll()
{
    NETMGR_LOG_D("QueryAll");
    std::vector<NetAccessPolicyData> result;
    if (rdbStore_ == nullptr) {
        NETMGR_LOG_E("%{public}s: rdb store is not initialized", __func__);
        return result;
    }

    OHOS::NativeRdb::RdbPredicates rdbPredicate{NETMANAGER_DB_UID_ACCESS_POLICY_TABLE};
    std::vector<std::string> whereArgs;
    auto queryResultSet = rdbStore_->Query(rdbPredicate, whereArgs);
    if (queryResultSet == nullptr) {
        return result;
    }

    bool isAtLastRow = false;
    queryResultSet->IsAtLastRow(isAtLastRow);
    while (!queryResultSet->GoToNextRow()) {
        NetAccessPolicyData policy;
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_ZERO, policy.uid);
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_ONE, policy.wifiPolicy);
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_TWO, policy.cellularPolicy);
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_THREE, policy.setFromConfigFlag);
        result.emplace_back(policy);
    }
    return result;
}

int32_t NetAccessPolicyRDB::QueryByUid(int uid, NetAccessPolicyData& uidPolicy)
{
    NETMGR_LOG_D("QueryByUid uid:%{public}d", uid);
    if (rdbStore_ == nullptr) {
        NETMGR_LOG_E("%{public}s: rdb store is not initialized", __func__);
        return NETMANAGER_ERROR;
    }

    OHOS::NativeRdb::RdbPredicates rdbPredicate{NETMANAGER_DB_UID_ACCESS_POLICY_TABLE};
    rdbPredicate.EqualTo(NetAccessPolicyRdbFiledConst::FILED_UID, uid);

    std::vector<std::string> whereArgs;
    auto queryResultSet = rdbStore_->Query(rdbPredicate, whereArgs);
    if (queryResultSet == nullptr) {
        NETMGR_LOG_E("QueryByUid error");
        return NETMANAGER_ERROR;
    }

    int32_t rowCount = 0;
    int ret = queryResultSet->GetRowCount(rowCount);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_LOG_E("query setting failed, get row count failed, name:%{public}d, ret:%{public}d", uid, ret);
        return ret;
    }
    if (rowCount == 0) {
        NETMGR_LOG_E("query setting name:%{public}d, num is 0", uid);
        return NETMANAGER_ERROR;
    }

    while (!queryResultSet->GoToNextRow()) {
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_ZERO, uidPolicy.uid);
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_ONE, uidPolicy.wifiPolicy);
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_TWO, uidPolicy.cellularPolicy);
        queryResultSet->GetInt(NetAccessPolicyRdbFiledConst::FILED_COLUMN_INDEX_THREE, uidPolicy.setFromConfigFlag);
        if (uidPolicy.uid == uid) {
            return NETMANAGER_SUCCESS;
        }
    }

    return NETMANAGER_ERROR;
}

} // namespace NetManagerStandard
} // namespace OHOS