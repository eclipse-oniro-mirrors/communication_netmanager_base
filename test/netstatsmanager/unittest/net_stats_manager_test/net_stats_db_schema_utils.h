/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_BASE_TEST_NET_STATS_DB_SCHEMA_UTILS_H
#define NETMANAGER_BASE_TEST_NET_STATS_DB_SCHEMA_UTILS_H

#include <memory>
#include <sys/stat.h>

#include "net_stats_database_defines.h"
#include "net_stats_database_helper.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;
/* The testonly net_stats_manager_static library redirects the statistics
 * databases into /data/local/tmp/netmanager_base_test_db
 * (NETMANAGER_TEST_DB_REDIRECT, see net_stats_database_defines.h), so a
 * redirected database starts empty. Tests that exercise NetStatsDataHandler
 * or NetStatsHistory must create the schema themselves first: before the
 * redirect they silently relied on the tables already present in the real
 * /data/service/el1/public/netmanager database maintained by the netmanager
 * service on the device. CreateTable() uses CREATE TABLE IF NOT EXISTS, so
 * this bootstrap is idempotent.
 * Upgrade() must run as well: the *_CREATE_PARAM base schemas do not contain
 * every column that the *_PARAM_LIST insert/select statements reference
 * (e.g. UserId on T_uid/T_uid_sim is added by table version 6, Ident on
 * T_iface by version 1). This mirrors what NetStatsCached::CreatNetStatsTables
 * does for the real service database: CreateTable() x6 followed by Upgrade().
 * Re-running Upgrade() on an already upgraded schema is a no-op (guarded by
 * the T_version rows); on a freshly bootstrapped schema the redundant
 * ALTER TABLE steps fail with "duplicate column name" and are ignored,
 * exactly like on the device database. */
inline void EnsureNetStatsDbSchema()
{
    mkdir("/data/local/tmp/netmanager_base_test_db", 0755); // EEXIST is fine
    for (const char *path : {NET_STATS_DATABASE_PATH, NET_STATS_DATABASE_BACK_PATH}) {
        auto helper = std::make_unique<NetStatsDatabaseHelper>(path);
        if (helper == nullptr) {
            continue;
        }
        helper->CreateTable(VERSION_TABLE, VERSION_TABLE_CREATE_PARAM);
        helper->CreateTable(UID_TABLE, UID_TABLE_CREATE_PARAM);
        helper->CreateTable(IFACE_TABLE, IFACE_TABLE_CREATE_PARAM);
        helper->CreateTable(UID_SIM_TABLE, UID_SIM_TABLE_CREATE_PARAM);
        helper->CreateTable(CALIBRATION_TABLE, CALIBRATION_TABLE_CREATE_PARAM);
        helper->CreateTable(CHANGE_TABLE, CHANGE_TABLE_CREATE_PARAM);
        helper->Upgrade();
    }
}
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_TEST_NET_STATS_DB_SCHEMA_UTILS_H
