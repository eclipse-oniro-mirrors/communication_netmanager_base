/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "net_stats_service.h"

#include <cinttypes>
#include <initializer_list>
#include <sys/time.h>
#include <unistd.h>

#include "broadcast_manager.h"
#include "common_event_support.h"
#include "system_ability_definition.h"

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"
#include "netmanager_base_permission.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr std::initializer_list<NetBearType> BEAR_TYPE_LIST = {
    NetBearType::BEARER_CELLULAR, NetBearType::BEARER_WIFI, NetBearType::BEARER_BLUETOOTH,
    NetBearType::BEARER_ETHERNET, NetBearType::BEARER_VPN,  NetBearType::BEARER_WIFI_AWARE,
};

bool GetIfaceNamesFromManager(std::list<std::string> &ifaceNames)
{
    int32_t ret = NetManagerCenter::GetInstance().GetIfaceNames(BEARER_CELLULAR, ifaceNames);
    if (ret != 0 || ifaceNames.empty()) {
        NETMGR_LOG_E("Iface list is empty, ret = %{public}d", ret);
        return false;
    }
    return true;
}
} // namespace
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetStatsService>::GetInstance().get());

NetStatsService::NetStatsService()
    : SystemAbility(COMM_NET_STATS_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    netStatsCallback_ = new (std::nothrow) NetStatsCallback();
}

NetStatsService::~NetStatsService() = default;

void NetStatsService::OnStart()
{
    NETMGR_LOG_D("NetStatsService::OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("init failed");
        return;
    }
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    state_ = STATE_RUNNING;
    NETMGR_LOG_D("NetStatsService::OnStart end");
}

void NetStatsService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = true;
}

int32_t NetStatsService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_LOG_D("Start Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return ret < 0 ? STATS_DUMP_MESSAGE_FAIL : NETMANAGER_SUCCESS;
}

void NetStatsService::GetDumpMessage(std::string &message)
{
    message.append("Net Stats Info:\n");
    uint64_t rxBytes = 0;
    uint64_t txBytes = 0;
    uint64_t rxPackets = 0;
    uint64_t txPackets = 0;
    NetStatsWrapper::GetInstance().GetTotalStats(rxBytes, StatsType::STATS_TYPE_RX_BYTES);
    NetStatsWrapper::GetInstance().GetTotalStats(txBytes, StatsType::STATS_TYPE_TX_BYTES);
    NetStatsWrapper::GetInstance().GetTotalStats(rxPackets, StatsType::STATS_TYPE_RX_PACKETS);
    NetStatsWrapper::GetInstance().GetTotalStats(txPackets, StatsType::STATS_TYPE_TX_PACKETS);

    message.append("\tRxBytes: " + std::to_string(rxBytes) + "\n");
    message.append("\tTxBytes: " + std::to_string(txBytes) + "\n");
    message.append("\tRxPackets: " + std::to_string(rxPackets) + "\n");
    message.append("\tTxPackets: " + std::to_string(txPackets) + "\n");
    std::for_each(BEAR_TYPE_LIST.begin(), BEAR_TYPE_LIST.end(), [&message, this](const auto &bearType) {
        std::list<std::string> ifaceNames;
        if (NetManagerCenter::GetInstance().GetIfaceNames(bearType, ifaceNames)) {
            return;
        }
        uint64_t rx = 0;
        uint64_t tx = 0;
        for (const auto &name : ifaceNames) {
            GetIfaceRxBytes(rx, name);
            GetIfaceTxBytes(tx, name);
            message.append("\t" + name + "-TxBytes: " + std::to_string(tx));
            message.append("\t" + name + "-RxBytes: " + std::to_string(rx));
        }
    });
}

bool NetStatsService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOG_E("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetStatsService>::GetInstance().get())) {
            NETMGR_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    return true;
}

int32_t NetStatsService::RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("RegisterNetStatsCallback parameter callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    netStatsCallback_->RegisterNetStatsCallback(callback);
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("UnregisterNetStatsCallback parameter callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    netStatsCallback_->UnregisterNetStatsCallback(callback);
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::GetIfaceRxBytes(uint64_t &stats, const std::string &interfaceName)
{
    return NetStatsWrapper::GetInstance().GetIfaceStats(stats, StatsType::STATS_TYPE_RX_BYTES, interfaceName);
}

int32_t NetStatsService::GetIfaceTxBytes(uint64_t &stats, const std::string &interfaceName)
{
    return NetStatsWrapper::GetInstance().GetIfaceStats(stats, StatsType::STATS_TYPE_TX_BYTES, interfaceName);
}

int32_t NetStatsService::GetCellularRxBytes(uint64_t &stats)
{
    std::list<std::string> ifaceNames;
    if (!GetIfaceNamesFromManager(ifaceNames)) {
        return STATS_ERR_GET_IFACE_NAME_FAILED;
    }

    for (const auto &name : ifaceNames) {
        uint64_t totalCellular = 0;
        auto ret = NetStatsWrapper::GetInstance().GetIfaceStats(totalCellular, StatsType::STATS_TYPE_RX_BYTES, name);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Get iface stats failed result: %{public}d", ret);
            return ret;
        }
        stats += totalCellular;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::GetCellularTxBytes(uint64_t &stats)
{
    std::list<std::string> ifaceNames;
    if (!GetIfaceNamesFromManager(ifaceNames)) {
        return STATS_ERR_GET_IFACE_NAME_FAILED;
    }

    uint64_t totalCellular = 0;
    for (const auto &name : ifaceNames) {
        auto ret = NetStatsWrapper::GetInstance().GetIfaceStats(totalCellular, StatsType::STATS_TYPE_TX_BYTES, name);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Get iface stats failed result: %{public}d", ret);
            return ret;
        }
        stats += totalCellular;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::GetAllRxBytes(uint64_t &stats)
{
    return NetStatsWrapper::GetInstance().GetTotalStats(stats, StatsType::STATS_TYPE_RX_BYTES);
}

int32_t NetStatsService::GetAllTxBytes(uint64_t &stats)
{
    return NetStatsWrapper::GetInstance().GetTotalStats(stats, StatsType::STATS_TYPE_TX_BYTES);
}

int32_t NetStatsService::GetUidRxBytes(uint64_t &stats, uint32_t uid)
{
    return NetStatsWrapper::GetInstance().GetUidStats(stats, StatsType::STATS_TYPE_RX_BYTES, uid);
}

int32_t NetStatsService::GetUidTxBytes(uint64_t &stats, uint32_t uid)
{
    return NetStatsWrapper::GetInstance().GetUidStats(stats, StatsType::STATS_TYPE_TX_BYTES, uid);
}
} // namespace NetManagerStandard
} // namespace OHOS
