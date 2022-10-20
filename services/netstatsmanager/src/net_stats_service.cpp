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
#include "system_ability_definition.h"
#include "common_event_support.h"
#include "net_stats_constants.h"
#include "net_stats_csv.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr std::initializer_list<NetBearType> BEAR_TYPE_LIST = {
    NetBearType::BEARER_CELLULAR,
    NetBearType::BEARER_WIFI,
    NetBearType::BEARER_BLUETOOTH,
    NetBearType::BEARER_ETHERNET,
    NetBearType::BEARER_VPN,
    NetBearType::BEARER_WIFI_AWARE,
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
    netStatsCsv_ = std::make_unique<NetStatsCsv>();
    netStatsCallback_ = new (std::nothrow) NetStatsCallback();
}

NetStatsService::~NetStatsService() {}

static void UpdateStatsTimer()
{
    sptr<NetStatsCsv> statsCsv = (std::make_unique<NetStatsCsv>()).release();
    if (statsCsv == nullptr) {
        NETMGR_LOG_E("statsCsv is nullptr");
        return;
    }
    if (!statsCsv->UpdateIfaceStats()) {
        NETMGR_LOG_E("UpdateIfaceStats failed");
    }
    if (!statsCsv->UpdateUidStats()) {
        NETMGR_LOG_E("UpdateUidStats failed");
    }
    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_NETMANAGER_NETSTATES_UPDATED;
    info.data = "Net Manager Iface and Uid States Updated";
    info.ordered = true;
    std::map<std::string, bool> param = {{"NetStatsUpdated", true}};
    DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);
    NETMGR_LOG_D("NetStatsService Update Iface and Uid Stats.");
}

void NetStatsService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
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
    updateStatsTimer_.Start(INTERVAL_UPDATE_STATS_TIME_MS, UpdateStatsTimer);

    state_ = STATE_RUNNING;
    gettimeofday(&tv, NULL);
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
    return ret < 0 ? static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR)
                   : static_cast<int32_t>(NetStatsResultCode::ERR_NONE);
}

void NetStatsService::GetDumpMessage(std::string &message)
{
    message.append("Net Stats Info:\n");
    message.append("\tRxBytes: " +
                   std::to_string(NetStatsWrapper::GetInstance().GetTotalStats(StatsType::STATS_TYPE_RX_BYTES)) +
                   "\n");
    message.append("\tTxBytes: " +
                   std::to_string(NetStatsWrapper::GetInstance().GetTotalStats(StatsType::STATS_TYPE_TX_BYTES)) +
                   "\n");
    message.append("\tRxPackets: " +
                   std::to_string(NetStatsWrapper::GetInstance().GetTotalStats(StatsType::STATS_TYPE_RX_PACKETS)) +
                   "\n");
    message.append("\tTxPackets: " +
                   std::to_string(NetStatsWrapper::GetInstance().GetTotalStats(StatsType::STATS_TYPE_TX_PACKETS)) +
                   "\n");
    std::for_each(BEAR_TYPE_LIST.begin(), BEAR_TYPE_LIST.end(), [&message, this](const auto &bearType) {
        std::list<std::string> ifaceNames;
        if (NetManagerCenter::GetInstance().GetIfaceNames(bearType, ifaceNames)) {
            return;
        }
        for (const auto &name : ifaceNames) {
            message.append("\t" + name + "-TxBytes: " + std::to_string(GetIfaceTxBytes(name)));
            message.append("\t" + name + "-RxBytes: " + std::to_string(GetIfaceRxBytes(name)));
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
    if (!netStatsCsv_->UpdateIfaceCsvInfo()) {
        NETMGR_LOG_E("update iface.csv failed");
        return false;
    }
    if (!netStatsCsv_->UpdateUidCsvInfo()) {
        NETMGR_LOG_E("update uid.csv failed");
        return false;
    }
    if (!netStatsCsv_->UpdateIfaceStats()) {
        NETMGR_LOG_E("update iface_stats.csv failed");
        return false;
    }
    if (!netStatsCsv_->UpdateUidStats()) {
        NETMGR_LOG_E("update uid_stats.csv failed");
        return false;
    }
    serviceIface_ = new (std::nothrow) NetStatsServiceIface();
    NetManagerCenter::GetInstance().RegisterStatsService(serviceIface_);
    return true;
}

void NetStatsService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_NETMANAGER_NETSTATES_LIMITED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_UID_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<NetStatsListener>(subscribeInfo);
    subscriber_->SetStatsCallback(netStatsCallback_);
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    NETMGR_LOG_D("NetStatsService SubscribeCommonEvent"
        " COMMON_EVENT_NETMANAGER_NETSTATES_LIMITED and COMMON_EVENT_UID_REMOVED");
}

int32_t NetStatsService::RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("RegisterNetStatsCallback parameter callback is null");
        return static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR);
    }

    netStatsCallback_->RegisterNetStatsCallback(callback);

    return static_cast<int32_t>(NetStatsResultCode::ERR_NONE);
}

int32_t NetStatsService::UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("UnregisterNetStatsCallback parameter callback is null");
        return static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR);
    }

    netStatsCallback_->UnregisterNetStatsCallback(callback);

    return static_cast<int32_t>(NetStatsResultCode::ERR_NONE);
}

NetStatsResultCode NetStatsService::GetIfaceStatsDetail(const std::string &iface, uint32_t start, uint32_t end,
    NetStatsInfo &statsInfo)
{
    if (!netStatsCsv_->ExistsIface(iface)) {
        NETMGR_LOG_E("iface not exist");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }
    if (start >= end) {
        NETMGR_LOG_E("the start time should be less than the end time.");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }
    NetStatsResultCode result = netStatsCsv_->GetIfaceBytes(iface, start, end, statsInfo);
    NETMGR_LOG_I("GetIfaceStatsDetail iface[%{public}s], statsInfo.rxBytes[%{public}" PRId64 "]"
        "statsInfo.txBytes[%{public}" PRId64 "]", iface.c_str(), statsInfo.rxBytes_, statsInfo.txBytes_);
    if (result == NetStatsResultCode::ERR_INVALID_TIME_PERIOD) {
        NETMGR_LOG_E("GetIfaceBytes is error.");
    }
    return result;
}

NetStatsResultCode NetStatsService::GetUidStatsDetail(const std::string &iface, uint32_t uid,
    uint32_t start, uint32_t end, NetStatsInfo &statsInfo)
{
    if (!netStatsCsv_->ExistsIface(iface)) {
        NETMGR_LOG_E("iface not exist");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }
    if (!netStatsCsv_->ExistsUid(uid)) {
        NETMGR_LOG_E("uid not exist");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }
    if (start >= end) {
        NETMGR_LOG_E("the start time should be less than the end time.");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }
    NetStatsResultCode result = netStatsCsv_->GetUidBytes(iface, uid, start, end, statsInfo);
    NETMGR_LOG_I("GetUidStatsDetail iface[%{public}s], uid[%{public}d] statsInfo.rxBytes[%{public}" PRId64 "] "
        "statsInfo.txBytes[%{public}" PRId64 "]", iface.c_str(), uid, statsInfo.rxBytes_, statsInfo.txBytes_);
    if (result == NetStatsResultCode::ERR_INVALID_TIME_PERIOD) {
        NETMGR_LOG_E("GetUidBytes is error.");
    }
    return result;
}

NetStatsResultCode NetStatsService::UpdateIfacesStats(const std::string &iface,
    uint32_t start, uint32_t end, const NetStatsInfo &stats)
{
    NETMGR_LOG_I("UpdateIfacesStats info: iface[%{public}s],rxBytes_[%{public}" PRId64 "]"
        "txBytes_[%{public}" PRId64 "]", iface.c_str(), stats.rxBytes_, stats.txBytes_);

    if (!netStatsCsv_->ExistsIface(iface)) {
        NETMGR_LOG_E("iface not exist");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }

    if (start >= end) {
        NETMGR_LOG_E("the start time should be less than the end time.");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }

    if (stats.rxBytes_ < 0 || stats.txBytes_ < 0) {
        NETMGR_LOG_E("the bytes cannot be negative.");
        return NetStatsResultCode::ERR_INVALID_PARAMETER;
    }

    if (!netStatsCsv_->CorrectedIfacesStats(iface, start, end, stats)) {
        NETMGR_LOG_E("UpdateIfacesStats failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return NetStatsResultCode::ERR_NONE;
}

NetStatsResultCode NetStatsService::UpdateStatsData()
{
    if (!netStatsCsv_->UpdateIfaceStats()) {
        NETMGR_LOG_E("UpdateIfaceStats failed");
    }
    if (!netStatsCsv_->UpdateUidStats()) {
        NETMGR_LOG_E("UpdateUidStats failed");
    }
    return NetStatsResultCode::ERR_NONE;
}

NetStatsResultCode NetStatsService::ResetFactory()
{
    NETMGR_LOG_I("ResetFactory begin");
    return netStatsCsv_->ResetFactory();
}

int64_t NetStatsService::GetIfaceRxBytes(const std::string &interfaceName)
{
    return NetStatsWrapper::GetInstance().GetIfaceStats(StatsType::STATS_TYPE_RX_BYTES, interfaceName);
}

int64_t NetStatsService::GetIfaceTxBytes(const std::string &interfaceName)
{
    return NetStatsWrapper::GetInstance().GetIfaceStats(StatsType::STATS_TYPE_TX_BYTES, interfaceName);
}

int64_t NetStatsService::GetCellularRxBytes()
{
    std::list<std::string> ifaceNames;
    int64_t err = -1;
    if (!GetIfaceNamesFromManager(ifaceNames)) {
        return err;
    }
    int64_t totalCellular = 0;
    for (const auto &name : ifaceNames) {
        totalCellular = totalCellular + NetStatsWrapper::GetInstance().GetIfaceStats(
            StatsType::STATS_TYPE_RX_BYTES, name);
    }
    return totalCellular;
}

int64_t NetStatsService::GetCellularTxBytes()
{
    std::list<std::string> ifaceNames;
    int64_t err = -1;
    if (!GetIfaceNamesFromManager(ifaceNames)) {
        return err;
    }
    int64_t totalCellular = 0;
    for (const auto &name : ifaceNames) {
        totalCellular = totalCellular + NetStatsWrapper::GetInstance().GetIfaceStats(
            StatsType::STATS_TYPE_TX_BYTES, name);
    }
    return totalCellular;
}

int64_t NetStatsService::GetAllRxBytes()
{
    return NetStatsWrapper::GetInstance().GetTotalStats(StatsType::STATS_TYPE_RX_BYTES);
}

int64_t NetStatsService::GetAllTxBytes()
{
    return NetStatsWrapper::GetInstance().GetTotalStats(StatsType::STATS_TYPE_TX_BYTES);
}

int64_t NetStatsService::GetUidRxBytes(uint32_t uid)
{
    return NetStatsWrapper::GetInstance().GetUidStats(StatsType::STATS_TYPE_RX_BYTES, uid);
}

int64_t NetStatsService::GetUidTxBytes(uint32_t uid)
{
    return NetStatsWrapper::GetInstance().GetUidStats(StatsType::STATS_TYPE_TX_BYTES, uid);
}
} // namespace NetManagerStandard
} // namespace OHOS
