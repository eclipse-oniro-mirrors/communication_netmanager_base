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

/* Fuzz harness interposition for NetsysController (real-IPC isolation).
 *
 * NetStatsClientFuzzTest links NetStatsService/NetStatsCached statically, but
 * NetsysController lives in the prebuilt libnetsys_controller.z.so, whose
 * methods delegate to NetsysNativeClient and issue REAL binder IPC to the
 * netsysnative service (SA 1158) on the device. On a real phone this blocks
 * forever (see dump: WaitForCompletion <- GetNetworkCellularSharingTraffic
 * <- NetStatsCached::CacheStats <- NetStatsService::UpdateStatsData), which
 * violates the fuzz rule "no real IPC from fuzz targets".
 *
 * Defining these non-virtual member functions in the fuzz executable shadows
 * the shared-library exports at static-link time (regular object definitions
 * take precedence over shared libraries), so every in-process call from
 * NetStatsService/NetStatsCached is answered locally with canned data. This
 * follows the same pattern as common_mock_netmanager_permission.cpp in this
 * target. Product binaries never see this file.
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <vector>

#include "net_manager_constants.h"
#include "net_stats_info.h"
#include "netsys_controller.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint64_t MOCK_STATS_VALUE = 12345;
constexpr uint32_t MOCK_UID = 1000;
const std::string MOCK_IFACE_NAME = "wlan0";
const std::string MOCK_CELLULAR_IFACE_NAME = "rmnet_data0";

/* The redirected test database directory (see net_stats_database_defines.h)
 * must exist before sqlite3_open_v2 is called: sqlite creates the file but
 * never the parent directory.
 */
__attribute__((constructor)) void EnsureTestDbDir()
{
    mkdir("/data/local/tmp/netmanager_base_test_db", 0755);
}

void FillMockStatsInfo(std::vector<NetStatsInfo> &stats)
{
    NetStatsInfo info;
    info.uid_ = MOCK_UID;
    info.iface_ = MOCK_IFACE_NAME;
    info.rxBytes_ = 1024;
    info.txBytes_ = 512;
    stats.push_back(info);
}
} // namespace

std::vector<std::string> NetsysController::InterfaceGetList()
{
    return {MOCK_IFACE_NAME};
}

int32_t NetsysController::GetIfaceStats(uint64_t &stats, uint32_t type, const std::string &interfaceName)
{
    (void)type;
    (void)interfaceName;
    stats = MOCK_STATS_VALUE;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::GetTotalStats(uint64_t &stats, uint32_t type)
{
    (void)type;
    stats = MOCK_STATS_VALUE;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid)
{
    (void)type;
    (void)uid;
    stats = MOCK_STATS_VALUE;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::GetCookieStats(uint64_t &stats, uint32_t type, uint64_t cookie)
{
    (void)type;
    (void)cookie;
    stats = MOCK_STATS_VALUE;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    stats.clear();
    FillMockStatsInfo(stats);
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::GetAllSimStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats)
{
    stats.clear();
    FillMockStatsInfo(stats);
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::GetNetworkCellularSharingTraffic(nmd::NetworkSharingTraffic &traffic, std::string &ifaceName)
{
    (void)traffic;
    ifaceName = MOCK_CELLULAR_IFACE_NAME;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::SetNetStateTrafficMap(uint8_t flag, uint64_t availableTraffic)
{
    (void)flag;
    (void)availableTraffic;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::GetNetStateTrafficMap(uint8_t flag, uint64_t &availableTraffic)
{
    (void)flag;
    availableTraffic = MOCK_STATS_VALUE;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::ClearSimStatsBpfMap()
{
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::DeleteIncreaseTrafficMap(uint64_t ifIndex)
{
    (void)ifIndex;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::UpdateIfIndexMap(int8_t key, uint64_t index)
{
    (void)key;
    (void)index;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::DeleteSimStatsInfo(uint32_t uid)
{
    (void)uid;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::DeleteStatsInfo(uint32_t uid)
{
    (void)uid;
    return NETMANAGER_SUCCESS;
}

int32_t NetsysController::RegisterNetsysTrafficCallback(const sptr<NetsysNative::INetsysTrafficCallback> &callback)
{
    /* Never register with the real netsysnative: a registered callback would
     * leave a dangling binder object in the service after the fuzz process
     * is killed. Report success so service-side bookkeeping stays consistent.
     */
    (void)callback;
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
