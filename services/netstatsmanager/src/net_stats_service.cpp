/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <dlfcn.h>
#include <net/if.h>
#include <sys/time.h>
#include <unistd.h>
#include <chrono>
#include <format>
#include <regex>

#include <cinttypes>

#include <initializer_list>

#include "bpf_stats.h"
#include "bpf_path.h"
#include "bpf_def.h"
#include "broadcast_manager.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "ffrt_inner.h"
#include "net_bundle.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"
#include "net_stats_database_defines.h"
#include "net_stats_service_common.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_permission.h"
#include "netmanager_hitrace.h"
#include "netsys_controller.h"
#include "system_ability_definition.h"
#ifdef SUPPORT_TRAFFIC_STATISTIC
#include "cellular_data_client.h"
#include "core_service_client.h"
#include "net_conn_client.h"
#include "cellular_data_types.h"
#include "net_info_observer.h"
#include "net_stats_utils.h"
#include "net_stats_notification.h"
#include "net_stats_rdb.h"
#endif // SUPPORT_TRAFFIC_STATISTIC
#include "iptables_wrapper.h"
#ifdef SUPPORT_NETWORK_SHARE
#include "networkshare_client.h"
#include "networkshare_constants.h"
#endif // SUPPORT_NETWORK_SHARE

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;
namespace {
constexpr std::initializer_list<NetBearType> BEAR_TYPE_LIST = {
    NetBearType::BEARER_CELLULAR, NetBearType::BEARER_WIFI, NetBearType::BEARER_BLUETOOTH,
    NetBearType::BEARER_ETHERNET, NetBearType::BEARER_VPN,  NetBearType::BEARER_WIFI_AWARE,
};
constexpr uint32_t DEFAULT_UPDATE_TRAFFIC_INFO_CYCLE_MS = 30 * 60 * 1000;
constexpr uint32_t DAY_SECONDS = 2 * 24 * 60 * 60;
constexpr const char* UID = "uid";
const std::string LIB_NET_BUNDLE_UTILS_PATH = "libnet_bundle_utils.z.so";
constexpr uint64_t DELAY_US = 35 * 1000 * 1000;
constexpr const char* COMMON_EVENT_STATUS = "usual.event.RGM_STATUS_CHANGED";
constexpr const char* STATUS_FIELD = "rgmStatus";
const std::string STATUS_UNLOCKED = "rgm_user_unlocked";
} // namespace
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetStatsService>::GetInstance().get());

NetStatsService::NetStatsService()
    : SystemAbility(COMM_NET_STATS_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    netStatsCallback_ = std::make_shared<NetStatsCallback>();
    netStatsCached_ = std::make_unique<NetStatsCached>();
#ifdef SUPPORT_TRAFFIC_STATISTIC
    netconnCallback_ = std::make_unique<NetInfoObserver>().release();
    trafficObserver_ = std::make_unique<TrafficObserver>().release();
#endif // SUPPORT_TRAFFIC_STATISTIC
}

NetStatsService::~NetStatsService()
{
#ifdef SUPPORT_TRAFFIC_STATISTIC
    StopTrafficOvserver();
#endif // SUPPORT_TRAFFIC_STATISTIC
}

void NetStatsService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("init failed");
        return;
    }
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
#ifdef SUPPORT_TRAFFIC_STATISTIC
    AddSystemAbilityListener(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
#endif // SUPPORT_TRAFFIC_STATISTIC
    state_ = STATE_RUNNING;
    sptr<NetStatsBaseService> baseService = new (std::nothrow) NetStatsServiceCommon();
    if (baseService == nullptr) {
        NETMGR_LOG_E("Net stats base service instance create failed");
        return;
    }
    NetManagerCenter::GetInstance().RegisterStatsService(baseService);
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

void NetStatsService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_LOG_I("OnAddSystemAbility: systemAbilityId:%{public}d", systemAbilityId);
#ifdef SUPPORT_TRAFFIC_STATISTIC
    if (systemAbilityId == COMM_NET_CONN_MANAGER_SYS_ABILITY_ID) {
        StartNetObserver();
        return;
    } else if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        StartTrafficOvserver();
        return;
    }
#endif // SUPPORT_TRAFFIC_STATISTIC
    if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
        RefreshUidStatsFlag(DELAY_US);
        return;
    }
    RegisterCommonEvent();
}

void NetStatsService::RegisterCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SHUTDOWN);
#ifdef SUPPORT_TRAFFIC_STATISTIC
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED);  // 监听卡状态
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CELLULAR_DATA_STATE_CHANGED);
#endif // SUPPORT_TRAFFIC_STATISTIC
    matchingSkills.AddEvent(COMMON_EVENT_STATUS);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<NetStatsListener>(subscribeInfo);
    subscriber_->RegisterStatsCallback(EventFwk::CommonEventSupport::COMMON_EVENT_SHUTDOWN,
        [this](const EventFwk::Want &want) { return UpdateStatsData(); });
    subscriber_->RegisterStatsCallback(
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED, [this](const EventFwk::Want &want) {
            uint32_t uid = want.GetIntParam(UID, 0);
            NETMGR_LOG_D("Net Manager delete uid, uid:[%{public}d]", uid);
            return CommonEventPackageRemoved(uid);
        });
    subscriber_->RegisterStatsCallback(
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED, [this](const EventFwk::Want &want) {
            uint32_t uid = want.GetIntParam(UID, 0);
            NETMGR_LOG_D("Net Manager add uid, uid:[%{public}d]", uid);
            return CommonEventPackageAdded(uid);
        });
    subscriber_->RegisterStatsCallback(COMMON_EVENT_STATUS, [this](const EventFwk::Want &want) -> bool {
        std::string status = want.GetStringParam(STATUS_FIELD);
        NETMGR_LOG_I("Net Manager status changed, status:[%{public}s]", status.c_str());
        if (status == STATUS_UNLOCKED) {
            RefreshUidStatsFlag(0);
        }
        return true;
    });
#ifdef SUPPORT_TRAFFIC_STATISTIC
    subscriber_->RegisterStatsCallback(
        EventFwk::CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED, [this](const EventFwk::Want &want) {
            int32_t slotId = want.GetIntParam("slotId", -1);
            int32_t simStatus = want.GetIntParam("state", -1);
            return CommonEventSimStateChanged(slotId, simStatus);
        });
    subscriber_->RegisterStatsCallback(
        EventFwk::CommonEventSupport::COMMON_EVENT_CELLULAR_DATA_STATE_CHANGED, [this](const EventFwk::Want &want) {
            int32_t slotId = want.GetIntParam("slotId", -1);
            int32_t dataState = want.GetIntParam("dataState", -1);
            return CommonEventCellularDataStateChanged(slotId, dataState);
        });
#endif // SUPPORT_TRAFFIC_STATISTIC
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void NetStatsService::GetDumpMessage(std::string &message)
{
    message.append("Net Stats Info:\n");
    uint64_t rxBytes = 0;
    uint64_t txBytes = 0;
    uint64_t rxPackets = 0;
    uint64_t txPackets = 0;
    NetsysController::GetInstance().GetTotalStats(rxBytes, static_cast<uint32_t>(StatsType::STATS_TYPE_RX_BYTES));
    NetsysController::GetInstance().GetTotalStats(txBytes, static_cast<uint32_t>(StatsType::STATS_TYPE_TX_BYTES));
    NetsysController::GetInstance().GetTotalStats(rxPackets, static_cast<uint32_t>(StatsType::STATS_TYPE_RX_PACKETS));
    NetsysController::GetInstance().GetTotalStats(txPackets, static_cast<uint32_t>(StatsType::STATS_TYPE_TX_PACKETS));

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
    if (nullptr == netStatsCached_) {
        return false;
    }
    netStatsCached_->SetCallbackManager(netStatsCallback_);
    auto ret = netStatsCached_->StartCached();
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Start cached failed");
        return false;
    }
    AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    uint64_t delay = DELAY_US / 2;
    RefreshUidStatsFlag(delay);

#ifdef SUPPORT_TRAFFIC_STATISTIC
#ifndef UNITTEST_FORBID_FFRT
    trafficTimer_ = std::make_unique<FfrtTimer>();
    trafficTimer_->Start(DEFAULT_UPDATE_TRAFFIC_INFO_CYCLE_MS, [this]() { UpdateBpfMap(); });
#endif
    NetStatsRDB netStats;
    netStats.InitRdbStore();
#endif // SUPPORT_TRAFFIC_STATISTIC
    return true;
}

int32_t NetStatsService::RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    NETMGR_LOG_I("Enter RegisterNetStatsCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("RegisterNetStatsCallback parameter callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    netStatsCallback_->RegisterNetStatsCallback(callback);
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    NETMGR_LOG_I("Enter UnregisterNetStatsCallback");
    if (callback == nullptr) {
        NETMGR_LOG_E("UnregisterNetStatsCallback parameter callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    netStatsCallback_->UnregisterNetStatsCallback(callback);
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::GetIfaceRxBytes(uint64_t &stats, const std::string &interfaceName)
{
    return NetsysController::GetInstance().GetIfaceStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_RX_BYTES),
                                                         interfaceName);
}

int32_t NetStatsService::GetIfaceTxBytes(uint64_t &stats, const std::string &interfaceName)
{
    return NetsysController::GetInstance().GetIfaceStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_TX_BYTES),
                                                         interfaceName);
}

int32_t NetStatsService::GetCellularRxBytes(uint64_t &stats)
{
    std::list<std::string> ifaceNames;
    if (!GetIfaceNamesFromManager(ifaceNames)) {
        return STATS_ERR_GET_IFACE_NAME_FAILED;
    }

    for (const auto &name : ifaceNames) {
        uint64_t totalCellular = 0;
        auto ret = NetsysController::GetInstance().GetIfaceStats(
            totalCellular, static_cast<uint32_t>(StatsType::STATS_TYPE_RX_BYTES), name);
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
        auto ret = NetsysController::GetInstance().GetIfaceStats(
            totalCellular, static_cast<uint32_t>(StatsType::STATS_TYPE_TX_BYTES), name);
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
    NETMGR_LOG_D("Enter GetAllRxBytes");
    return NetsysController::GetInstance().GetTotalStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_RX_BYTES));
}

int32_t NetStatsService::GetAllTxBytes(uint64_t &stats)
{
    NETMGR_LOG_D("Enter GetAllTxBytes");
    return NetsysController::GetInstance().GetTotalStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_TX_BYTES));
}

int32_t NetStatsService::GetUidRxBytes(uint64_t &stats, uint32_t uid)
{
    NETMGR_LOG_D("Enter GetUidRxBytes, uid is %{public}d", uid);
    return NetsysController::GetInstance().GetUidStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_RX_BYTES),
                                                       uid);
}

int32_t NetStatsService::GetUidTxBytes(uint64_t &stats, uint32_t uid)
{
    NETMGR_LOG_D("Enter GetUidTxBytes,uid is %{public}d", uid);
    return NetsysController::GetInstance().GetUidStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_TX_BYTES),
                                                       uid);
}

int32_t NetStatsService::GetIfaceStatsDetail(const std::string &iface, uint64_t start, uint64_t end,
                                             NetStatsInfo &statsInfo)
{
    // Start of get traffic data by interface name.
    NETMGR_LOG_D("Enter GetIfaceStatsDetail, iface= %{public}s", iface.c_str());
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetIfaceStatsDetail start");
    if (start > end) {
        NETMGR_LOG_E("start is after end.");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::vector<NetStatsInfo> allInfo;
    auto history = std::make_unique<NetStatsHistory>();
    int32_t ret = history->GetHistory(allInfo, iface, start, end);

    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("netStatsCached_ is fail");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netStatsCached_->GetIfaceStatsCached(allInfo);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Get traffic stats data failed");
        return ret;
    }
    std::for_each(allInfo.begin(), allInfo.end(), [&statsInfo, &iface, &start, &end](const auto &info) {
        if (info.iface_ == iface && info.date_ >= start && info.date_ <= end) {
            statsInfo += info;
        }
    });
    statsInfo.iface_ = iface;
    statsInfo.date_ = end;
    // End of get traffic data by interface name.
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetIfaceStatsDetail end");
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::GetUidStatsDetail(const std::string &iface, uint32_t uid, uint64_t start, uint64_t end,
                                           NetStatsInfo &statsInfo)
{
    // Start of get traffic data by usr id.
    NETMGR_LOG_D("Enter GetIfaceStatsDetail, iface= %{public}s uid= %{public}d", iface.c_str(), uid);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetUidStatsDetail start");
    if (start > end) {
        NETMGR_LOG_E("start is after end.");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::vector<NetStatsInfo> allInfo;
    auto history = std::make_unique<NetStatsHistory>();
    int32_t ret = history->GetHistory(allInfo, iface, uid, start, end);
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("netStatsCached_ is fail");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netStatsCached_->GetUidStatsCached(allInfo);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Get traffic stats data failed");
        return ret;
    }
    std::for_each(allInfo.begin(), allInfo.end(), [&statsInfo, &iface, &uid, &start, &end](const auto &info) {
        if (info.iface_ == iface && info.uid_ == uid && info.date_ >= start && info.date_ <= end) {
            statsInfo += info;
        }
    });
    statsInfo.uid_ = uid;
    statsInfo.iface_ = iface;
    statsInfo.date_ = end;
    // End of get traffic data by usr id.
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("NetStatsService GetUidStatsDetail end");
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::UpdateIfacesStats(const std::string &iface, uint64_t start, uint64_t end,
                                           const NetStatsInfo &stats)
{
    // Start of update traffic data by date.
    NETMGR_LOG_I("UpdateIfacesStats ifaces is %{public}s", iface.c_str());
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService UpdateIfacesStats start");
    if (start > end) {
        NETMGR_LOG_E("start is after end.");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::vector<NetStatsInfo> infos;
    infos.push_back(stats);
    auto handler = std::make_unique<NetStatsDataHandler>();
    auto ret = handler->DeleteByDate(IFACE_TABLE, start, end);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Update ifaces stats failed");
    }
    ret = handler->WriteStatsData(infos, IFACE_TABLE);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Update ifaces stats failed");
        return STATS_ERR_WRITE_DATA_FAIL;
    }
    // End of update traffic data by date.
    NetmanagerHiTrace::NetmanagerFinishSyncTrace("NetStatsService UpdateIfacesStats end");
    return ret;
}

int32_t NetStatsService::UpdateStatsData()
{
    NETMGR_LOG_I("Enter UpdateStatsData.");
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("Cached is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netStatsCached_->ForceUpdateStats();
    NETMGR_LOG_D("End UpdateStatsData.");
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::ResetFactory()
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ClearData();
}

int32_t NetStatsService::GetAllStatsInfo(std::vector<NetStatsInfo> &infos)
{
    NETMGR_LOG_D("Enter GetAllStatsInfo.");
    if (netStatsCached_ != nullptr) {
        netStatsCached_->GetUidPushStatsCached(infos);
        netStatsCached_->GetAllPushStatsCached(infos);
    } else {
        NETMGR_LOG_E("Cached is nullptr");
    }
    return NetsysController::GetInstance().GetAllStatsInfo(infos);
}

int32_t NetStatsService::GetAllSimStatsInfo(std::vector<NetStatsInfo> &infos)
{
    NETMGR_LOG_D("Enter GetAllSimStatsInfo.");
    return NetsysController::GetInstance().GetAllSimStatsInfo(infos);
}

#ifdef SUPPORT_NETWORK_SHARE
bool NetStatsService::IsSharingOn()
{
    int32_t share = 0;
    int ret = DelayedSingleton<NetManagerStandard::NetworkShareClient>::GetInstance()->IsSharing(share);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        NETMGR_LOG_E("get sharing state res: %{public}d, isSharing: %{public}d", ret, share);
        return false;
    }
    return share == NetManagerStandard::NETWORKSHARE_IS_SHARING;
}

void NetStatsService::GetSharingStats(std::vector<NetStatsInfo> &sharingStats, uint32_t endtime)
{
    if (endtime > netStatsCached_->GetWriteDateTime())
    {
        // 跑在非ipc线程防止鉴权失败
        bool isSharingOn = false;
        auto task = ffrt::submit_h([&isSharingOn, this]() { isSharingOn = NetStatsService::IsSharingOn(); }, {}, {},
            ffrt::task_attr().name("isSharingOn"));
        ffrt::wait({task});
        if (isSharingOn)
        {
            NETMGR_LOG_D("GetSharingStats enter");
            netStatsCached_->GetIptablesStatsIncrease(sharingStats);
        }
    }
}
#endif

int32_t NetStatsService::GetTrafficStatsByNetwork(std::unordered_map<uint32_t, NetStatsInfo> &infos,
                                                  const sptr<NetStatsNetwork> &network)
{
    NETMGR_LOG_D("Enter GetTrafficStatsByNetwork.");
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetTrafficStatsByNetwork start");
    if (netStatsCached_ == nullptr) {
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (network == nullptr || network->startTime_ > network->endTime_) {
        NETMGR_LOG_E("param network is invalid");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::string ident;
    if (network->type_ == 0) {
        ident = std::to_string(network->simId_);
    }
    uint32_t start = network->startTime_;
    uint32_t end = network->endTime_;
    NETMGR_LOG_D("param: ident=%{public}s, start=%{public}u, end=%{public}u", ident.c_str(), start, end);
    auto history = std::make_unique<NetStatsHistory>();
    if (history == nullptr) {
        NETMGR_LOG_E("history is null");
        return NETMANAGER_ERR_INTERNAL;
    }
    std::vector<NetStatsInfo> allInfo;
    int32_t ret = history->GetHistoryByIdent(allInfo, ident, start, end);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("get history by ident failed, err code=%{public}d", ret);
        return ret;
    }
    netStatsCached_->GetKernelStats(allInfo);
    netStatsCached_->GetUidPushStatsCached(allInfo);
    netStatsCached_->GetUidStatsCached(allInfo);
    netStatsCached_->GetUidSimStatsCached(allInfo);
#ifdef SUPPORT_NETWORK_SHARE
    GetSharingStats(allInfo, end);
#endif
    FilterTrafficStatsByNetwork(allInfo, infos, ident, start, end);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetTrafficStatsByNetwork end");
    return NETMANAGER_SUCCESS;
}

void NetStatsService::FilterTrafficStatsByNetwork(std::vector<NetStatsInfo> &allInfo,
    std::unordered_map<uint32_t, NetStatsInfo> &infos,
    const std::string ident, uint32_t startTime, uint32_t endTime)
{
    std::for_each(allInfo.begin(), allInfo.end(), [&infos, &ident, &startTime, &endTime](NetStatsInfo &info) {
        if (ident != info.ident_ || startTime > info.date_ || endTime < info.date_) {
            return;
        }
        if (info.flag_ == STATS_DATA_FLAG_UNINSTALLED) {
            info.uid_ = UNINSTALLED_UID;
        }
        auto item = infos.find(info.uid_);
        if (item == infos.end()) {
            infos.emplace(info.uid_, info);
        } else {
            item->second += info;
        }
    });
}

int32_t NetStatsService::GetTrafficStatsByUidNetwork(std::vector<NetStatsInfoSequence> &infos, uint32_t uid,
                                                     const sptr<NetStatsNetwork> &network)
{
    NETMGR_LOG_D("Enter GetTrafficStatsByUidNetwork.");
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetTrafficStatsByUidNetwork start");
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("Cached is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (network == nullptr || network->startTime_ > network->endTime_) {
        NETMGR_LOG_E("param network is invalid");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::string ident;
    if (network->type_ == 0) {
        ident = std::to_string(network->simId_);
    }
    uint32_t start = network->startTime_;
    uint32_t end = network->endTime_;
    NETMGR_LOG_D("GetTrafficStatsByUidNetwork param: "
        "uid=%{public}u, ident=%{public}s, start=%{public}u, end=%{public}u", uid, ident.c_str(), start, end);
    auto history = std::make_unique<NetStatsHistory>();
    if (history == nullptr) {
        NETMGR_LOG_E("history is null");
        return NETMANAGER_ERR_INTERNAL;
    }
    std::vector<NetStatsInfo> allInfo;
    int32_t ret = history->GetHistory(allInfo, uid, ident, start, end);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("get history by uid and ident failed, err code=%{public}d", ret);
        return ret;
    }
    netStatsCached_->GetKernelStats(allInfo);
    netStatsCached_->GetUidPushStatsCached(allInfo);
    netStatsCached_->GetUidStatsCached(allInfo);
    netStatsCached_->GetUidSimStatsCached(allInfo);
#ifdef SUPPORT_NETWORK_SHARE
    if (uid == IPTABLES_UID) {
        GetSharingStats(allInfo, end);///增加一个只有是uid==热点的uid的时候才去查iptables
    }
#endif
    FilterTrafficStatsByUidNetwork(allInfo, infos, uid, ident, start, end);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetTrafficStatsByUidNetwork end");
    return NETMANAGER_SUCCESS;
}

void NetStatsService::FilterTrafficStatsByUidNetwork(std::vector<NetStatsInfo> &allInfo,
    std::vector<NetStatsInfoSequence> &infos, const uint32_t uid,
    const std::string ident, uint32_t startTime, uint32_t endTime)
{
    std::for_each(allInfo.begin(), allInfo.end(),
        [this, &infos, &uid, &ident, &startTime, &endTime](const NetStatsInfo &info) {
        if (uid != info.uid_ || ident != info.ident_ || startTime > info.date_ || endTime < info.date_) {
            return;
        }
        if (info.flag_ == STATS_DATA_FLAG_UNINSTALLED) {
            return;
        }
        MergeTrafficStats(infos, info, endTime);
    });
}

int32_t NetStatsService::SetAppStats(const PushStatsInfo &info)
{
    NETMGR_LOG_D("Enter GetTrafficStatsByUidNetwork.");
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService SetAppStats start");
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("Cached is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netStatsCached_->SetAppStats(info);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService SetAppStats end");
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::SaveSharingTraffic(const NetStatsInfo &infos)
{
    NETMGR_LOG_D("Enter SaveSharingTraffic");
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService SaveSharingTraffic start");
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("Cached is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netStatsCached_->SaveSharingTraffic(infos);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService SaveSharingTraffic end");
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::GetCookieRxBytes(uint64_t &stats, uint64_t cookie)
{
    return NetsysController::GetInstance().GetCookieStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_RX_BYTES),
                                                          cookie);
}

int32_t NetStatsService::GetCookieTxBytes(uint64_t &stats, uint64_t cookie)
{
    return NetsysController::GetInstance().GetCookieStats(stats, static_cast<uint32_t>(StatsType::STATS_TYPE_TX_BYTES),
                                                          cookie);
}

void NetStatsService::MergeTrafficStats(std::vector<NetStatsInfoSequence> &statsInfoSequences, const NetStatsInfo &info,
                                        uint32_t currentTimestamp)
{
    NetStatsInfoSequence tmp;
    tmp.startTime_ = info.date_;
    tmp.endTime_ = info.date_;
    tmp.info_ = info;
    uint32_t previousTimestamp = currentTimestamp > DAY_SECONDS ? currentTimestamp - DAY_SECONDS : 0;
    if (info.date_ > previousTimestamp) {
        statsInfoSequences.push_back(std::move(tmp));
        return;
    }
    auto findRet = std::find_if(
        statsInfoSequences.begin(), statsInfoSequences.end(), [&info, previousTimestamp](const auto &item) {
            return item.endTime_ < previousTimestamp && CommonUtils::IsSameNaturalDay(info.date_, item.endTime_);
        });
    if (findRet == statsInfoSequences.end()) {
        statsInfoSequences.push_back(std::move(tmp));
        return;
    }
    (*findRet).info_ += info;
}

bool NetStatsService::GetIfaceNamesFromManager(std::list<std::string> &ifaceNames)
{
    int32_t ret = NetManagerCenter::GetInstance().GetIfaceNames(BEARER_CELLULAR, ifaceNames);
    if (ret != NETMANAGER_SUCCESS || ifaceNames.empty()) {
        NETMGR_LOG_D("Iface list is empty, ret = %{public}d", ret);
        return false;
    }
    ifaceNames.sort();
    ifaceNames.erase(std::unique(ifaceNames.begin(), ifaceNames.end()), ifaceNames.end());
    return true;
}

std::unordered_map<uint32_t, SampleBundleInfo> NetStatsService::GetSampleBundleInfosForActiveUser()
{
    void *handler = dlopen(LIB_NET_BUNDLE_UTILS_PATH.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (handler == nullptr) {
        NETMGR_LOG_E("load lib failed, reason : %{public}s", dlerror());
        return std::unordered_map<uint32_t, SampleBundleInfo>{};
    }
    using GetNetBundleClass = INetBundle *(*)();
    auto getNetBundle = (GetNetBundleClass)dlsym(handler, "GetNetBundle");
    if (getNetBundle == nullptr) {
        NETMGR_LOG_E("GetNetBundle failed, reason : %{public}s", dlerror());
        dlclose(handler);
        return std::unordered_map<uint32_t, SampleBundleInfo>{};
    }
    auto netBundle = getNetBundle();
    if (netBundle == nullptr) {
        NETMGR_LOG_E("netBundle is nullptr");
        dlclose(handler);
        return std::unordered_map<uint32_t, SampleBundleInfo>{};
    }
    std::optional<std::unordered_map<uint32_t, SampleBundleInfo>> result = netBundle->ObtainBundleInfoForActive();
    dlclose(handler);
    if (!result.has_value()) {
        NETMGR_LOG_W("ObtainBundleInfoForActive is nullopt");
        return std::unordered_map<uint32_t, SampleBundleInfo>{};
    }
    return result.value();
}

SampleBundleInfo NetStatsService::GetSampleBundleInfoForUid(uint32_t uid)
{
    void *handler = dlopen(LIB_NET_BUNDLE_UTILS_PATH.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (handler == nullptr) {
        NETMGR_LOG_E("load lib failed, reason : %{public}s", dlerror());
        return SampleBundleInfo{};
    }
    using GetNetBundleClass = INetBundle *(*)();
    auto getNetBundle = (GetNetBundleClass)dlsym(handler, "GetNetBundle");
    if (getNetBundle == nullptr) {
        NETMGR_LOG_E("GetNetBundle failed, reason : %{public}s", dlerror());
        dlclose(handler);
        return SampleBundleInfo{};
    }
    auto netBundle = getNetBundle();
    if (netBundle == nullptr) {
        NETMGR_LOG_E("netBundle is nullptr");
        dlclose(handler);
        return SampleBundleInfo{};
    }
    std::optional<SampleBundleInfo> result = netBundle->ObtainBundleInfoForUid(uid);
    dlclose(handler);
    if (!result.has_value()) {
        NETMGR_LOG_W("ObtainBundleInfoForUid is nullopt");
        return SampleBundleInfo{};
    }
    return result.value();
}

void NetStatsService::RefreshUidStatsFlag(uint64_t delay)
{
    std::function<void()> uidInstallSourceFunc = [this]() {
        auto tmp = GetSampleBundleInfosForActiveUser();
        for (auto iter = tmp.begin(); iter != tmp.end(); ++iter) {
            if (CommonUtils::IsSim(iter->second.bundleName_) ||
                CommonUtils::IsSim2(iter->second.bundleName_)) {
                netStatsCached_->SetUidSimSampleBundle(iter->first, iter->second);
            }
        }
        netStatsCached_->ClearUidStatsFlag();
        netStatsCached_->SetUidStatsFlag(tmp);
    };
    ffrt::submit(std::move(uidInstallSourceFunc), {}, {}, ffrt::task_attr().name("RefreshUidStatsFlag").delay(delay));
}

bool NetStatsService::CommonEventPackageAdded(uint32_t uid)
{
    SampleBundleInfo sampleBundleInfo = GetSampleBundleInfoForUid(uid);
    if (CommonUtils::IsSim(sampleBundleInfo.bundleName_) ||
        CommonUtils::IsSim2(sampleBundleInfo.bundleName_)) {
        uint64_t delay = 0;
        if (netStatsCached_->GetUidSimSampleBundlesSize() == 0) {
            delay = DELAY_US;
            netStatsCached_->ForceCachedStats();
        }
        RefreshUidStatsFlag(delay);
    } else {
        std::unordered_map<uint32_t, SampleBundleInfo> tmp{{uid, sampleBundleInfo}};
        netStatsCached_->SetUidStatsFlag(tmp);
    }
    return true;
}

bool NetStatsService::CommonEventPackageRemoved(uint32_t uid)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    if (handler == nullptr) {
        NETMGR_LOG_E("Net Manager package removed, get db handler failed. uid:[%{public}d]", uid);
        return static_cast<int32_t>(NETMANAGER_ERR_INTERNAL);
    }
    auto ret1 = handler->UpdateStatsFlag(uid, STATS_DATA_FLAG_UNINSTALLED);
    if (ret1 != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Net Manager update stats flag failed, uid:[%{public}d]", uid);
    }
    auto ret2 = handler->UpdateSimStatsFlag(uid, STATS_DATA_FLAG_UNINSTALLED);
    if (ret2 != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Net Manager update sim stats flag failed, uid:[%{public}d]", uid);
    }
    auto ffrtHandle = netStatsCached_->ForceArchiveStats(uid);
    if (netStatsCached_->GetUidSimSampleBundle(uid).has_value()) {
        ffrt::wait({ffrtHandle});
        RefreshUidStatsFlag(0);
    }
    return ret1 != NETMANAGER_SUCCESS ? ret1 : ret2;
}

#ifdef SUPPORT_TRAFFIC_STATISTIC
bool NetStatsService::CommonEventSimStateChanged(int32_t slotId, int32_t simState)
{
    int32_t simId = Telephony::CoreServiceClient::GetInstance().GetSimId(slotId);
    NETMGR_LOG_I("CommonEventSimStateChanged simId: %{public}d, slotId:%{public}d, simState:%{public}d",
        simId, slotId, simState);
    if (simId < 0) {
        NETMGR_LOG_E("get simId error");
        return false;
    }

    if (simState == static_cast<int32_t>(Telephony::SimState::SIM_STATE_LOADED)) {
        if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
            ObserverPtr trafficDataObserver = std::make_shared<TrafficDataObserver>(simId);
            SettingsInfoPtr trafficSettingsInfo = std::make_shared<TrafficSettingsInfo>();
            trafficDataObserver->ReadTrafficDataSettings(trafficSettingsInfo);
            settingsTrafficMap_.insert(
                std::make_pair(simId, std::make_pair(trafficDataObserver, trafficSettingsInfo)));
            UpdateNetStatsToMapFromDB(simId);
            NETMGR_LOG_I("settingsTrafficMap_.insert(simId). simId:%{public}d", simId);
            trafficDataObserver->RegisterTrafficDataSettingObserver();
        }
    } else if (simState != static_cast<int32_t>(Telephony::SimState::SIM_STATE_READY)) {
        // 卡异常，取消监听
        if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
            // 去注册
            settingsTrafficMap_[simId].first->UnRegisterTrafficDataSettingObserver();
            NETMGR_LOG_I("settingsTrafficMap_.erase(simId). simId:%{public}d", simId);
            settingsTrafficMap_.erase(simId);
        }
    }
    return true;
}

bool NetStatsService::CommonEventCellularDataStateChanged(int32_t slotId, int32_t dataState)
{
    NETMGR_LOG_I("CommonEventCellularDataStateChanged slotId:%{public}d, dateState:%{public}d", slotId, dataState);
    if (!isWifiConnected_) {
        NETMGR_LOG_I("CommonEventCellularDataStateChanged. but wifi is not connected");
        return false;
    }
    int32_t defaultSlotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    if (defaultSlotId < 0 || (defaultSlotId >= 0 && defaultSlotId != slotId)) { // 后续支持拉副卡的话，这里需要修改
        NETMGR_LOG_E(" defaultSlotId err or not default");
        return false;
    }

    int32_t simId = Telephony::CoreServiceClient::GetInstance().GetSimId(slotId);
    NETMGR_LOG_I("CommonEventCellularDataStateChanged simId: %{public}d, curActiviteSimId_: %{public}d",
        simId, curActiviteSimId_);
    if (simId < 0 || simId == curActiviteSimId_ ||
        dataState != static_cast<int32_t>(Telephony::DataConnectState::DATA_STATE_CONNECTED)) {
        return false;
    }

    curActiviteSimId_ = simId;
    int32_t ret = NetConnClient::GetInstance().GetIfaceNameIdentMaps(NetBearType::BEARER_CELLULAR, ifaceNameIdentMap_);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("CommonEventCellularDataStateChanged error. ret=%{public}d", ret);
    }
    NETMGR_LOG_I("CommonEventCellularDataStateChanged ifaceNameIdentMap size: %{public}d", ifaceNameIdentMap_.Size());
    ifaceNameIdentMap_.Iterate([this](const std::string &k, const std::string &v) {
        NETMGR_LOG_E("CommonEventCellularDataStateChanged K:%{public}s, V:%{public}s", k.c_str(), v.c_str());
        if (v == std::to_string(curActiviteSimId_)) {
            curIfIndex_ = if_nametoindex(k.c_str());
            NETMGR_LOG_E("CommonEventCellularDataStateChanged curIfIndex_:%{public}" PRIu64, curIfIndex_);
        }
    });

    UpdateCurActiviteSimChanged();
    return true;
}

void NetStatsService::StartNetObserver()
{
    NETMGR_LOG_I("StartNetObserver start");
    if (netconnCallback_ == nullptr) {
        netconnCallback_ = std::make_unique<NetInfoObserver>().release();
    }
    if (netconnCallback_ == nullptr) {
        return;
    }
    NetManagerStandard::NetSpecifier netSpecifier;
    NetManagerStandard::NetAllCapabilities netAllCapabilities;
    netAllCapabilities.netCaps_.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
    netSpecifier.ident_ = "";
    netSpecifier.netCapabilities_ = netAllCapabilities;
    sptr<NetManagerStandard::NetSpecifier> specifier =
        new (std::nothrow) NetManagerStandard::NetSpecifier(netSpecifier);
    int32_t ret = NetConnClient::GetInstance().RegisterNetConnCallback(specifier, netconnCallback_, 0);
    if (ret != 0) {
        NETMGR_LOG_E("StartNetObserver fail, ret = %{public}d", ret);
        return;
    }
    NETMGR_LOG_I("StartNetObserver end");
}

void NetStatsService::StartTrafficOvserver()
{
    NETMGR_LOG_I("StartTrafficOvserver start");
    if (trafficObserver_ == nullptr) {
        trafficObserver_ = std::make_unique<TrafficObserver>().release();
    }
    if (trafficObserver_ == nullptr) {
        return;
    }
    int32_t ret = NetsysController::GetInstance().RegisterNetsysTrafficCallback(trafficObserver_);
    if (ret != 0) {
        NETMGR_LOG_E("StartTrafficOvserver fail, ret = %{public}d", ret);
        return;
    }
    NETMGR_LOG_I("StartTrafficOvserver end");
}

void NetStatsService::StopTrafficOvserver()
{
    NETMGR_LOG_I("StopTrafficOvserver start");
    if (trafficObserver_ == nullptr) {
        trafficObserver_ = std::make_unique<TrafficObserver>().release();
    }
    if (trafficObserver_ == nullptr) {
        return;
    }
    int32_t ret = NetsysController::GetInstance().UnRegisterNetsysTrafficCallback(trafficObserver_);
    if (ret != 0) {
        NETMGR_LOG_E("StopTrafficOvserver fail, ret = %{public}d", ret);
        return;
    }
    NETMGR_LOG_I("StopTrafficOvserver end");
}

// 网络信息变化
bool NetStatsService::ProcessNetConnectionPropertiesChange(int32_t simId, uint64_t ifIndex)
{
    if (simId == INT32_MAX) {
        NETMGR_LOG_I("ProcessNetConnectionPropertiesChange. current default net is wifi");
        isWifiConnected_ = true;
        return true;
    }

    isWifiConnected_ = false;
    if (simId < 0 || simId == curActiviteSimId_) {
        NETMGR_LOG_I("ProcessNetConnectionPropertiesChange. simId == curActiviteSimId_, no process");
        return false;
    }

    NETMGR_LOG_I("ProcessNetConnectionPropertiesChange. update curActiviteSimId_:%{public}d, curIfIndex_:%{public}lu",
        simId, ifIndex);
    curActiviteSimId_ = simId;
    curIfIndex_ = ifIndex;

    UpdateCurActiviteSimChanged();
    return true;
}

void NetStatsService::UpdateCurActiviteSimChanged()
{
    if (settingsTrafficMap_.find(curActiviteSimId_) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("UpdateCurActiviteSimChanged settingsTrafficMap_ not find simId: %{public}d", curActiviteSimId_);
        std::shared_ptr<TrafficDataObserver> observer = std::make_shared<TrafficDataObserver>(curActiviteSimId_);
        // 1. 读simId_数据库
        std::shared_ptr<TrafficSettingsInfo> settingsInfo = std::make_shared<TrafficSettingsInfo>();
        observer->ReadTrafficDataSettings(settingsInfo);
        // 2. 注册监听
        observer->RegisterTrafficDataSettingObserver();
        settingsTrafficMap_.insert(std::make_pair(curActiviteSimId_, std::make_pair(observer, settingsInfo)));
        UpdateNetStatsToMapFromDB(curActiviteSimId_);
        NETMGR_LOG_I("ProcessNetConnectionPropertiesChange insert settingsInfo beginDate:%{public}d,\
unLimitedDataEnable:%{public}d, monthlyLimitdNotifyType:%{public}d,\
monthlyLimit:%{public}" PRIu64 ", monthlyMark:%{public}u, dailyMark:%{public}u",
            settingsInfo->beginDate, settingsInfo->unLimitedDataEnable, settingsInfo->monthlyLimitdNotifyType,
            settingsInfo->monthlyLimit, settingsInfo->monthlyMark, settingsInfo->dailyMark);
    }
    // 3. 判断是否设置余额，如果设置了 就计算已用流量，更新bpfMap
    // （1）表示没有设置余额或打开了无限流量开关，这种情况将余额map都改为最大值，因为不需要弹窗
    if (settingsTrafficMap_[curActiviteSimId_].second->monthlyLimit == UINT64_MAX ||
        settingsTrafficMap_[curActiviteSimId_].second->unLimitedDataEnable == 1) {
        SetTrafficMapMaxValue();
    } else {  // (2)有设置余额，这种情况需要计算可用余额map、增量map
        UpdateBpfMap();
    }
}

// 查询某段时间用到的总流量
int32_t NetStatsService::GetAllUsedTrafficStatsByNetwork(const sptr<NetStatsNetwork> &network, uint64_t &allUsedTraffic)
{
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    int32_t ret = GetTrafficStatsByNetwork(infos, network);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }

    allUsedTraffic = 0;
    NETMGR_LOG_I("NetStatsInfo size: %{public}zu", infos.size());
    for (auto it = infos.begin(); it != infos.end(); ++it) {
        allUsedTraffic += it->second.rxBytes_;
        allUsedTraffic += it->second.txBytes_;
    }
    return NETMANAGER_SUCCESS;
}

void NetStatsService::UpdateBpfMap()
{
    NETMGR_LOG_I("UpdateBpfMap start");

    if (settingsTrafficMap_.find(curActiviteSimId_) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("simId: %{public}d error", curActiviteSimId_);
        return;
    }

    NetsysController::GetInstance().ClearIncreaseTrafficMap();
    NetsysController::GetInstance().UpdateIfIndexMap(0, curIfIndex_);

    SettingsInfoPtr info = settingsTrafficMap_[curActiviteSimId_].second;
    if (info != nullptr) {
        NETMGR_LOG_I("settingsInfo-> simId:%{public}d, beginDate:%{public}d, unLimitedDataEnable:%{public}d,\
monthlyLimitdNotifyType:%{public}d, monthlyLimit:%{public}" PRIu64 ", monthlyMark:%{public}u,\
dailyMark:%{public}u",
            curActiviteSimId_, info->beginDate, info->unLimitedDataEnable, info->monthlyLimitdNotifyType,
            info->monthlyLimit, info->monthlyMark, info->dailyMark);
    }
    
    uint64_t monthlyAvailable = UINT64_MAX;
    uint64_t monthlyMarkAvailable = UINT64_MAX;
    uint64_t dailyMarkAvailable = UINT64_MAX;
    bool ret = CalculateTrafficAvailable(curActiviteSimId_, monthlyAvailable, monthlyMarkAvailable, dailyMarkAvailable);
    if (!ret) {
        NETMGR_LOG_E("CalculateTrafficAvailable error or not set limit or open unlimit");
        return;
    }

    NETMGR_LOG_E("GetTrafficMap before write. monthlyAvailable:%{public}" PRIu64", \
monthlyMarkAvailable:%{public}" PRIu64", dailyMarkAvailable:%{public}" PRIu64,
        monthlyAvailable, monthlyMarkAvailable, dailyMarkAvailable);
    NetsysController::GetInstance().SetNetStateTrafficMap(NET_STATS_MONTHLY_LIMIT, monthlyAvailable);
    NetsysController::GetInstance().SetNetStateTrafficMap(NET_STATS_MONTHLY_MARK, monthlyMarkAvailable);
    NetsysController::GetInstance().SetNetStateTrafficMap(NET_STATS_DAILY_MARK, dailyMarkAvailable);

    uint64_t monthlyAvailableMap = UINT64_MAX;
    uint64_t monthlyMarkAvailableMap = UINT64_MAX;
    uint64_t dailyMarkAvailableMap = UINT64_MAX;
    NetsysController::GetInstance().GetNetStateTrafficMap(NET_STATS_MONTHLY_LIMIT, monthlyAvailableMap);
    NetsysController::GetInstance().GetNetStateTrafficMap(NET_STATS_MONTHLY_MARK, monthlyMarkAvailableMap);
    NetsysController::GetInstance().GetNetStateTrafficMap(NET_STATS_DAILY_MARK, dailyMarkAvailableMap);
    NETMGR_LOG_E("GetTrafficMap after write. monthlyAvailable:%{public}" PRIu64", \
monthlyMarkAvailable:%{public}" PRIu64", dailyMarkAvailable:%{public}" PRIu64,
        monthlyAvailableMap, monthlyMarkAvailableMap, dailyMarkAvailableMap);

    if (monthlyAvailable == UINT64_MAX) {
        NotifyTrafficAlert(NET_STATS_MONTHLY_LIMIT);
    } else if (monthlyMarkAvailable == UINT64_MAX) {
        NotifyTrafficAlert(NET_STATS_MONTHLY_MARK);
    } else if (dailyMarkAvailable == UINT64_MAX) {
        NotifyTrafficAlert(NET_STATS_DAILY_MARK);
    }
}

bool NetStatsService::CalculateTrafficAvailable(int32_t simId, uint64_t &monthlyAvailable,
    uint64_t &monthlyMarkAvailable, uint64_t &dailyMarkAvailable)
{
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("settingsTrafficMap not find simId, simId is %{public}d", simId);
        return false;
    }
    sptr<NetStatsNetwork> network = (std::make_unique<NetStatsNetwork>()).release();
    network->startTime_ =
        NetStatsUtils::GetStartTimestamp(settingsTrafficMap_[simId].second->beginDate);
    network->endTime_ = NetStatsUtils::GetNowTimestamp();
    NETMGR_LOG_I("endTime: %{public}lu. simId: %{public}d", network->endTime_, simId);
    network->type_ = 0;
    network->simId_ = simId;
    uint64_t allUsedTraffic = 0;
    int ret = GetAllUsedTrafficStatsByNetwork(network, allUsedTraffic);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("GetAllUsedTrafficStatsByNetwork err. ret: %{public}d", ret);
        return false;
    }

    NETMGR_LOG_I("GetAllUsedTrafficStatsByNetwork allUsedTraffic: %{public}" PRIu64, allUsedTraffic);
    // 限额不是u64且没有打开无限开关
    if (settingsTrafficMap_[simId].second->monthlyLimit != UINT64_MAX &&
        settingsTrafficMap_[simId].second->unLimitedDataEnable != 1) {
        // 设置值大于已用值时，才计算余额
        if (settingsTrafficMap_[simId].second->monthlyLimit > allUsedTraffic) {
            monthlyAvailable = settingsTrafficMap_[simId].second->monthlyLimit - allUsedTraffic;
        }
        // (设置值*月限制比例)大于已用值
        uint64_t monthTmp = (settingsTrafficMap_[simId].second->monthlyLimit / 100.0) *
            settingsTrafficMap_[simId].second->monthlyMark;
        if (monthTmp > allUsedTraffic) {
            monthlyMarkAvailable = monthTmp - allUsedTraffic;
        }
        uint64_t todayStartTime = NetStatsUtils::GetTodayStartTimestamp();
        network->startTime_ = todayStartTime;
        uint64_t allTodayUsedTraffix = 0;
        ret = GetAllUsedTrafficStatsByNetwork(network, allTodayUsedTraffix);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("GetAllUsedTrafficStatsByNetwork err. ret: %{public}d", ret);
            return false;
        }
        uint64_t dayTmp = (settingsTrafficMap_[simId].second->monthlyLimit / 100.0) *
            settingsTrafficMap_[simId].second->dailyMark;
        NETMGR_LOG_E("dayTmp:%{public}" PRIu64 ", allTodayUsedTraffix:%{public}" PRIu64, dayTmp, allTodayUsedTraffix);
        if (dayTmp > allTodayUsedTraffix) {
            dailyMarkAvailable = dayTmp - allTodayUsedTraffix;
        }
        return true;
    }
    return false;
}

void NetStatsService::SetTrafficMapMaxValue()
{
    NETMGR_LOG_I("SetTrafficMapMaxValue");
    NetsysController::GetInstance().SetNetStateTrafficMap(NET_STATS_MONTHLY_LIMIT, UINT64_MAX);
    NetsysController::GetInstance().SetNetStateTrafficMap(NET_STATS_MONTHLY_MARK, UINT64_MAX);
    NetsysController::GetInstance().SetNetStateTrafficMap(NET_STATS_DAILY_MARK, UINT64_MAX);
}

void NetStatsService::UpdataSettingsdata(int32_t simId, uint8_t flag, uint64_t value)
{
    NETMGR_LOG_I("UpdataSettingsdata. simId: %{public}d, flag: %{public}d, value: %{public}lu", simId, flag, value);
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("not found simId: %{public}d.", simId);
        return;
    }
    switch (flag) {
        case NET_STATS_NO_LIMIT_ENABLE:
            if (value == 0 || value == 1) {
                settingsTrafficMap_[simId].second->unLimitedDataEnable = static_cast<int8_t>(value);
            }
            break;
        case NET_STATS_MONTHLY_LIMIT:
                settingsTrafficMap_[simId].second->monthlyLimit = value;
                settingsTrafficMap_[simId].second->isCanNotifyMonthlyLimit = true;
                settingsTrafficMap_[simId].second->isCanNotifyMonthlyMark = true;
                settingsTrafficMap_[simId].second->isCanNotifyDailyMark = true;
                UpdateTrafficLimitDate(simId);
            break;
        case NET_STATS_BEGIN_DATE:
            if (value >= 1 && value <= 31) { // 31: 每月日期数最大值
                settingsTrafficMap_[simId].second->beginDate = static_cast<int32_t>(value);
            }
            break;
        case NET_STATS_NOTIFY_TYPE:
            if (value == 0 || value == 1) {
                settingsTrafficMap_[simId].second->monthlyLimitdNotifyType = static_cast<int8_t>(value);
            }
            break;
        case NET_STATS_MONTHLY_MARK:
            if (value >= 0 || value <= 100) { // 100: 百分比最大值
                settingsTrafficMap_[simId].second->monthlyMark = value;
                settingsTrafficMap_[simId].second->isCanNotifyMonthlyMark = true;
                UpdateTrafficLimitDate(simId);
            }
            break;
        case NET_STATS_DAILY_MARK:
            if (value >= 0 || value <= 100) { // 100: 百分比最大值
                settingsTrafficMap_[simId].second->dailyMark = value;
            }
            break;
        default:
            break;
    }

    if (simId == curActiviteSimId_) {
        UpdateBpfMap();
    }
}

TrafficObserver::TrafficObserver() {}
TrafficObserver::~TrafficObserver() {}

int32_t TrafficObserver::OnExceedTrafficLimits(int8_t &flag)
{
    NETMGR_LOG_I("OnExceedTrafficLimits flag: %{public}d", flag);
    if (flag < NET_STATS_MONTHLY_LIMIT || flag > NET_STATS_DAILY_MARK) {
        NETMGR_LOG_E("OnExceedTrafficLimits flag error. value: %{public}d", flag);
        return -1;
    }

    // 触发弹窗检测
    DelayedSingleton<NetStatsService>::GetInstance()->NotifyTrafficAlert(flag);
    return 0;
}

void NetStatsService::UpdateBeginDate(int32_t simId, int32_t beginDate)
{
    if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
        settingsTrafficMap_[simId].second->beginDate = beginDate;
    }
}

void NetStatsService::UpdateUnLimitedDataEnable(int32_t simId, int8_t unLimitedDataEnable)
{
    if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
        settingsTrafficMap_[simId].second->unLimitedDataEnable = unLimitedDataEnable;
    }
}

void NetStatsService::UpdateMonthlyLimitdNotifyType(int32_t simId, int8_t monthlyLimitdNotifyType)
{
    if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
        settingsTrafficMap_[simId].second->monthlyLimitdNotifyType = monthlyLimitdNotifyType;
    }
}

void NetStatsService::UpdateMonthlyLimit(int32_t simId, uint64_t monthlyLimit)
{
    if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
        settingsTrafficMap_[simId].second->monthlyLimit = monthlyLimit;
    }
}

void NetStatsService::UpdateMonthlyMark(int32_t simId, uint16_t monthlyMark)
{
    if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
        settingsTrafficMap_[simId].second->monthlyMark = monthlyMark;
    }
}

void NetStatsService::UpdateDailyMark(int32_t simId, uint16_t dailyMark)
{
    if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
        settingsTrafficMap_[simId].second->dailyMark = dailyMark;
    }
}

// 从DB中读simId上次弹窗对应的时间戳
void NetStatsService::UpdateNetStatsToMapFromDB(int32_t simId)
{
    /* 获取用Query */
    NETMGR_LOG_I("UpdateNetStatsToMapFromDB enter.");
    NetStatsRDB netStats;
    
    std::vector<NetStatsData> result = netStats.QueryAll();
    for (size_t i = 0; i < result.size(); i++) {
        int32_t curSumId = result[i].simId;
        if (simId == curSumId && settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
            settingsTrafficMap_[curSumId].second->lastMonAlertTime = result[i].monWarningDate;
            settingsTrafficMap_[curSumId].second->lastMonNotifyTime = result[i].dayNoticeDate;
            settingsTrafficMap_[curSumId].second->lastDayNotifyTime = result[i].monNoticeDate;
            settingsTrafficMap_[curSumId].second->isCanNotifyMonthlyLimit =
                static_cast<bool>(result[i].monWarningState);
            settingsTrafficMap_[curSumId].second->isCanNotifyMonthlyMark = static_cast<bool>(result[i].monNoticeState);
            settingsTrafficMap_[curSumId].second->isCanNotifyDailyMark = static_cast<bool>(result[i].dayNoticeState);
        }
    }
}

int32_t NetStatsService::NotifyTrafficAlert(uint8_t flag)
{
    if (NetStatsUtils::IsMobileDataEnabled() && GetNotifyStats(flag)) {
        DealNotificaiton(flag);
    } else {
        NETMGR_LOG_I("There is no need to pop up trafficLimit notification.");
    }
    return NETMANAGER_SUCCESS;
}

// 判断是否满足弹窗条件 flag：表示弹窗类型
bool NetStatsService::GetNotifyStats(uint8_t flag)
{
    NETMGR_LOG_I("Enter GetNotifyStats.");
    if (settingsTrafficMap_.find(curActiviteSimId_) == settingsTrafficMap_.end()) {
        NETMGR_LOG_I("GetCurActiviteSimId Key failed : curActiviteSimId_ = %{public}d.", curActiviteSimId_);
        return false;
    }
    if (settingsTrafficMap_[curActiviteSimId_].second->unLimitedDataEnable == 1) {
        NETMGR_LOG_I("setting unLimitedData, unLimitedDataEnable:%{public}d",
            settingsTrafficMap_[curActiviteSimId_].second->unLimitedDataEnable);
        return false;
    }
 
    switch (flag) {
        case NET_STATS_MONTHLY_LIMIT:
            return GetMonAlertStatus();
            break;
        case NET_STATS_MONTHLY_MARK:
            return GetMonNotifyStatus();
            break;
        case NET_STATS_DAILY_MARK:
            return GetDayNotifyStatus();
            break;
        default:
            NETMGR_LOG_I("unknown notification type");
            return false;
    }
    return false;
}

bool NetStatsService::GetMonNotifyStatus()
{
    NETMGR_LOG_I("Enter GetMonNotifyStatus.");
 
    if (settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyMonthlyMark) {
        settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyMonthlyMark = false;
        return true;
    }

    int32_t currentTime = NetStatsUtils::GetNowTimestamp();
    int32_t currentStartTime =
        NetStatsUtils::GetStartTimestamp(settingsTrafficMap_[curActiviteSimId_].second->beginDate);
    NETMGR_LOG_I("Enter currentTime:%{public}d, currentDayStartTime:%{public}d, lastMonNotifyTime: %{public}d",
        currentTime, currentStartTime, settingsTrafficMap_[curActiviteSimId_].second->lastMonNotifyTime);
    if (settingsTrafficMap_[curActiviteSimId_].second->lastMonNotifyTime < currentStartTime) {
        return true;
    }
    return false;
}
 
bool NetStatsService::GetDayNotifyStatus()
{
    NETMGR_LOG_I("Enter GetDayNotifyStatus.");
    if (settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyDailyMark) {
        settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyDailyMark = false;
        return true;
    }
    int32_t currentDayStartTime = NetStatsUtils::GetTodayStartTimestamp();
    NETMGR_LOG_I("Enter currentDayStartTime:%{public}d, lastDayNotifyTime: %{public}d",
        currentDayStartTime, settingsTrafficMap_[curActiviteSimId_].second->lastDayNotifyTime);
    if (settingsTrafficMap_[curActiviteSimId_].second->lastDayNotifyTime < currentDayStartTime) {
        return true;
    }
    return false;
}
 
bool NetStatsService::GetMonAlertStatus()
{
    NETMGR_LOG_I("Enter GetMonAlertStatus.");
    if (settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyMonthlyLimit) {
        NETMGR_LOG_I("isCanNotify true : states changed caused.");
        settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyMonthlyLimit = false;
        settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyMonthlyMark = false;
        settingsTrafficMap_[curActiviteSimId_].second->isCanNotifyDailyMark = false;
        return true;
    }
 
    int currentTime = NetStatsUtils::GetNowTimestamp();
    int currentStartTime = NetStatsUtils::GetStartTimestamp(settingsTrafficMap_[curActiviteSimId_].second->beginDate);
    NETMGR_LOG_I("Enter currentTime:%{public}d, currentDayStartTime:%{public}d, lastMonAlertTime: %{public}d",
        currentTime, currentStartTime, settingsTrafficMap_[curActiviteSimId_].second->lastMonAlertTime);
    if (settingsTrafficMap_[curActiviteSimId_].second->lastMonAlertTime < currentStartTime) {
        return true;
    }
    return false;
}

// 拉起弹窗
void NetStatsService::DealNotificaiton(uint8_t flag)
{
    NETMGR_LOG_I("Enter DealDayNotification.");
    int simNum = NetStatsUtils::IsDaulCardEnabled();
    bool isDaulCard = false;
    if (simNum == 0) {
        return;
    } else if (simNum == DUAL_CARD) {
        isDaulCard = true;
    }
 
    switch (flag) {
        case NET_STATS_MONTHLY_LIMIT:
            return DealMonAlert(isDaulCard);
            break;
        case NET_STATS_MONTHLY_MARK:
            return DealMonNotification(isDaulCard);
            break;
        case NET_STATS_DAILY_MARK:
            return DealDayNotification(isDaulCard);
            break;
        default:
            NETMGR_LOG_I("unknown notificationdeal type");
    }
}

void NetStatsService::DealDayNotification(bool isDaulCard)
{
    NETMGR_LOG_I("Enter DealDayNotification.");
    NetMgrNetStatsLimitNotification::GetInstance().PublishNetStatsLimitNotification(NETMGR_STATS_LIMIT_DAY, isDaulCard);
    settingsTrafficMap_[curActiviteSimId_].second->lastDayNotifyTime = NetStatsUtils::GetNowTimestamp();
    UpdateTrafficLimitDate(curActiviteSimId_);
    NETMGR_LOG_I("update DayNotification time:%{public}d",
        settingsTrafficMap_[curActiviteSimId_].second->lastDayNotifyTime);
}
 
void NetStatsService::DealMonNotification(bool isDaulCard)
{
    NETMGR_LOG_I("Enter DealMonNotification.");
    NetMgrNetStatsLimitNotification::GetInstance().PublishNetStatsLimitNotification(
        NETMGR_STATS_LIMIT_MONTH, isDaulCard);
    settingsTrafficMap_[curActiviteSimId_].second->lastMonNotifyTime = NetStatsUtils::GetNowTimestamp();
    UpdateTrafficLimitDate(curActiviteSimId_);
    NETMGR_LOG_I("update MonNotification time:%{public}d",
        settingsTrafficMap_[curActiviteSimId_].second->lastMonNotifyTime);
}

void NetStatsService::DealMonAlert(bool isDaulCard)
{
    NETMGR_LOG_I("Enter DealMonAlert.");
    if (dialog_ == nullptr) {
        dialog_ = std::make_shared<TrafficLimitDialog>();
    }
    
    if (dialog_ == nullptr) {
        NETMGR_LOG_E("Get TrafficLimitDialog faied.");
        return;
    }

    if (settingsTrafficMap_.find(curActiviteSimId_) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("map find error");
    }
    NetMgrNetStatsLimitNotification::GetInstance().PublishNetStatsLimitNotification(
        NETMGR_STATS_ALERT_MONTH, isDaulCard);
 
    if (settingsTrafficMap_[curActiviteSimId_].second->monthlyLimitdNotifyType) {
        dialog_->PopUpTrafficLimitDialog();
    }
    settingsTrafficMap_[curActiviteSimId_].second->lastMonAlertTime = NetStatsUtils::GetNowTimestamp();
    UpdateTrafficLimitDate(curActiviteSimId_);
    NETMGR_LOG_I("update MonAlert time:%{public}d", settingsTrafficMap_[curActiviteSimId_].second->lastMonAlertTime);
}

int32_t NetStatsService::GetCurActiviteSimId()
{
    return curActiviteSimId_ ;
}
 
std::map<int32_t, std::pair<ObserverPtr, SettingsInfoPtr>> NetStatsService::GetSettingsObserverMap()
{
    return settingsTrafficMap_;
}

void NetStatsService::UpdateTrafficLimitDate(int32_t simId)
{
    NETMGR_LOG_I("UpdateTrafficLimitDate start");
    NetStatsRDB netStats;
    NetStatsData statsData;
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("UpdateTrafficLimitDate err. Not find simId:%{public}d", simId);
        return;
    }
    auto info = settingsTrafficMap_[simId];
    statsData.simId = simId;
    statsData.monWarningDate = info.second->lastMonAlertTime;
    statsData.dayNoticeDate = info.second->lastMonNotifyTime;
    statsData.monNoticeDate = info.second->lastDayNotifyTime;
    statsData.monWarningState = info.second->isCanNotifyMonthlyLimit;
    statsData.dayNoticeState = info.second->isCanNotifyDailyMark;
    statsData.monNoticeState = info.second->isCanNotifyMonthlyMark;

    netStats.InsertData(statsData);
}
#endif //SUPPORT_TRAFFIC_STATISTIC
} // namespace NetManagerStandard
} // namespace OHOS
