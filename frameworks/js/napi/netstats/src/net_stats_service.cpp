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
#include "net_stats_utils.h"
#include "net_stats_notification.h"
#include "net_stats_rdb.h"
#endif // SUPPORT_TRAFFIC_STATISTIC
#include "iptables_wrapper.h"
#ifdef SUPPORT_NETWORK_SHARE
#include "networkshare_client.h"
#include "networkshare_constants.h"
#endif // SUPPORT_NETWORK_SHARE
#include "system_timer.h"
#include "net_stats_subscriber.h"

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
constexpr uint32_t DAY_MILLISECONDS = 24 * 60 * 60 * 1000;
constexpr int32_t TRAFFIC_NOTIFY_TYPE = 3;
constexpr const char* UID = "uid";
const std::string LIB_NET_BUNDLE_UTILS_PATH = "libnet_bundle_utils.z.so";
constexpr uint64_t DELAY_US = 35 * 1000 * 1000;
constexpr const char* COMMON_EVENT_STATUS = "usual.event.RGM_STATUS_CHANGED";
constexpr const char* STATUS_FIELD = "rgmStatus";
const std::string STATUS_UNLOCKED = "rgm_user_unlocked";

enum NetStatusType : uint8_t {
    WIFI_TYPE = 0,
    CELLULAR_TYPE = 1,
};

enum NetStatusConn : uint8_t {
    NON_CONNECTED = 0,
    CONNECTED = 1,
};
} // namespace
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetStatsService>::GetInstance().get());

NetStatsService::NetStatsService()
    : SystemAbility(COMM_NET_STATS_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    netStatsCallback_ = std::make_shared<NetStatsCallback>();
    netStatsCached_ = std::make_unique<NetStatsCached>();
#ifdef SUPPORT_TRAFFIC_STATISTIC
    trafficObserver_ = std::make_unique<TrafficObserver>().release();
    trafficPlanFfrtQueue_ = std::make_shared<ffrt::queue>("TrafficPlanStatistic");
#endif // SUPPORT_TRAFFIC_STATISTIC
}

NetStatsService::~NetStatsService() = default;

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
    AddSystemAbilityListener(TIME_SERVICE_ID);
#ifdef SUPPORT_TRAFFIC_STATISTIC
    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
#endif // SUPPORT_TRAFFIC_STATISTIC
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
    state_ = STATE_RUNNING;
    sptr<NetStatsBaseService> baseService = new (std::nothrow) NetStatsServiceCommon();
    if (baseService == nullptr) {
        NETMGR_LOG_E("Net stats base service instance create failed");
        return;
    }
    NetManagerCenter::GetInstance().RegisterStatsService(baseService);
}

void NetStatsService::StartSysTimer()
{
    NETMGR_LOG_I("NetStatsService StartSysTimer");
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (netStatsSysTimerId_ != 0) {
        NETMGR_LOG_E("netStatsSysTimerId_ is not zero, value is %{public}" PRIu64, netStatsSysTimerId_);
        return;
    }
    std::shared_ptr<NetmanagerSysTimer> netStatsSysTimer =
        std::make_unique<NetmanagerSysTimer>(true, DAY_MILLISECONDS, true);
    std::function<void()> callback = [this]() {
#ifdef SUPPORT_TRAFFIC_STATISTIC
        NetStatsRDB netStats;
        netStats.BackUpNetStatsFreqDB(NOTICE_DATABASE_NAME, NOTICE_DATABASE_BACK_NAME);
#endif // SUPPORT_TRAFFIC_STATISTIC
        UpdateStatsDataInner();
    };
    netStatsSysTimer->SetCallbackInfo(callback);
    netStatsSysTimer->SetName("netstats_data_persistence_timer");
    netStatsSysTimerId_ = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(netStatsSysTimer);
    uint64_t todayStartTime = static_cast<uint64_t>(CommonUtils::GetTodayMidnightTimestamp(23, 59, 55)) * 1000;
    MiscServices::TimeServiceClient::GetInstance()->StartTimer(netStatsSysTimerId_, todayStartTime);
    NETMGR_LOG_I("netStatsSysTimerId_ success. value is %{public}" PRIu64, netStatsSysTimerId_);
}

void NetStatsService::StopSysTimer()
{
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (netStatsSysTimerId_ == 0) {
        NETMGR_LOG_W("netStatsSysTimerId_ is zero");
        return;
    }
    MiscServices::TimeServiceClient::GetInstance()->StopTimer(netStatsSysTimerId_);
    MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(netStatsSysTimerId_);
    netStatsSysTimerId_ = 0;
    NETMGR_LOG_I("stop netStatsSysTimerId_ success");
}

int32_t NetStatsService::ModifySysTimer()
{
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (netStatsSysTimerId_ == 0) {
        NETMGR_LOG_E("netStatsSysTimerId_ is zero");
        return NETMANAGER_ERROR;
    }
    MiscServices::TimeServiceClient::GetInstance()->StopTimer(netStatsSysTimerId_);
    uint64_t todayStartTime = static_cast<uint64_t>(CommonUtils::GetTodayMidnightTimestamp(23, 59, 55)) * 1000;
    MiscServices::TimeServiceClient::GetInstance()->StartTimer(netStatsSysTimerId_, todayStartTime);
    NETMGR_LOG_I("ModifySysTimer netStatsSysTimerId_ success. timer: %{public}" PRIu64, todayStartTime);
    return NETMANAGER_SUCCESS;
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
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        StartTrafficOvserver();
        return;
    }
#endif // SUPPORT_TRAFFIC_STATISTIC
    if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
        RefreshUidStatsFlag(DELAY_US);
        return;
    }
    if (systemAbilityId == TIME_SERVICE_ID) {
        StartSysTimer();
        return;
    }

    if (systemAbilityId == SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN) {
        InitPrivateUserId();
        StartAccountObserver();
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
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
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
    RegisterCommonTelephonyEvent();
    RegisterCommonTimeEvent();
    RegisterCommonNetStatusEvent();
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void NetStatsService::RegisterCommonNetStatusEvent()
{
    subscriber_->RegisterStatsCallbackData(
        EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE, [this](const EventFwk::CommonEventData& eventData) {
            int32_t state = eventData.GetCode();
            NETMGR_LOG_I("COMMON_EVENT_WIFI_CONN_STATE: %{public}d", state);
            if (state == 4) { // 4:OHOS::Wifi::ConnState::CONNECTED
                return UpdateNetStatusMap(0, 1);
            } else {
                return UpdateNetStatusMap(0, 0);
            }
            return false;
        });
}

void NetStatsService::RegisterCommonTelephonyEvent()
{
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
}

void NetStatsService::RegisterCommonTimeEvent()
{
    subscriber_->RegisterStatsCallback(
        EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED, [this](const EventFwk::Want &want) {
            NETMGR_LOG_I("COMMON_EVENT_TIME_CHANGED");
            return ModifySysTimer();
        });
    subscriber_->RegisterStatsCallback(
        EventFwk::CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED, [this](const EventFwk::Want &want) -> bool {
            NETMGR_LOG_I("COMMON_EVENT_TIMEZONE_CHANGED");
            return ModifySysTimer();
        });
}

bool NetStatsService::UpdateNetStatusMap(uint8_t type, uint8_t value)
{
    int32_t ret = NetsysController::GetInstance().SetNetStatusMap(type, value);
    if (ret != NETMANAGER_SUCCESS) {
        return false;
    }
    return true;
}

void NetStatsService::InitPrivateUserId()
{
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    for (auto info : osAccountInfos) {
        AccountSA::OsAccountType accountType;
        AccountSA::OsAccountManager::GetOsAccountType(info.GetLocalId(), accountType);
        NETMGR_LOG_I("InitPrivateUserId, info: %{public}d", info.GetLocalId());
        if (accountType == AccountSA::OsAccountType::PRIVATE) {
            netStatsCached_->SetCurPrivateUserId(info.GetLocalId());
        }
    }
    int32_t defaultUserId = -1;
    int32_t ret = AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(defaultUserId);
    NETMGR_LOG_I("default userId: %{public}d", defaultUserId);
    netStatsCached_->SetCurDefaultUserId(defaultUserId);
}

int32_t NetStatsService::ProcessOsAccountChanged(int32_t userId, AccountSA::OsAccountState state)
{
    NETMGR_LOG_I("OsAccountChanged toId: %{public}d, state:%{public}d", userId, state);
    if (state == AccountSA::OsAccountState::CREATED) {
        AccountSA::OsAccountType accountType;
        AccountSA::OsAccountManager::GetOsAccountType(userId, accountType);
        if (accountType == AccountSA::OsAccountType::PRIVATE) {
            netStatsCached_->SetCurPrivateUserId(userId);
        }
        return 0;
    }
    if (state == AccountSA::OsAccountState::STOPPING || state ==  AccountSA::OsAccountState::STOPPED ||
        state ==  AccountSA::OsAccountState::REMOVED) {
        if (netStatsCached_->GetCurPrivateUserId() != userId) {
            return 0;
        }
        netStatsCached_->SetCurPrivateUserId(-1);  // -1:invalid userID
        auto handler = std::make_unique<NetStatsDataHandler>();
        if (handler == nullptr) {
            NETMGR_LOG_E("handler is nullptr");
            return static_cast<int32_t>(NETMANAGER_ERR_INTERNAL);
        }
        auto ret1 = handler->UpdateStatsFlagByUserId(userId, STATS_DATA_FLAG_UNINSTALLED);
        if (ret1 != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("update stats flag failed, uid:[%{public}d]", userId);
        }
        auto ret2 = handler->UpdateSimStatsFlagByUserId(userId, STATS_DATA_FLAG_UNINSTALLED);
        if (ret2 != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("update sim stats flag failed, uid:[%{public}d]", userId);
        }
        UpdateStatsData();
    }
    return 0;
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
#ifndef NETMANAGER_TEST
        if (!Publish(DelayedSingleton<NetStatsService>::GetInstance().get())) {
            NETMGR_LOG_E("Register to sa manager failed");
            return false;
        }
#endif
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
    trafficTimer_->Start(DEFAULT_UPDATE_TRAFFIC_INFO_CYCLE_MS, [this]() { UpdateBpfMapTimer(); });
#endif
    NetStatsRDB netStats;
    netStats.InitRdbStore();
#endif // SUPPORT_TRAFFIC_STATISTIC
    return true;
}

int32_t NetStatsService::RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    NETMGR_LOG_I("Enter RegisterNetStatsCallback");
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    if (!NetManagerPermission::IsSystemCaller()) {
        return NETMANAGER_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
        return NETMANAGER_ERR_PERMISSION_DENIED;
    }
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("Cached is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netStatsCached_->ForceUpdateStats();
    NETMGR_LOG_D("End UpdateStatsData.");
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::UpdateStatsDataInner()
{
    NETMGR_LOG_I("Enter UpdateStatsDataInner.");
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("Cached is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netStatsCached_->ForceUpdateStats();
    NETMGR_LOG_I("End UpdateStatsDataInner.");
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::ResetFactory()
{
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ClearData();
}

int32_t NetStatsService::GetAllStatsInfo(std::vector<NetStatsInfo> &infos)
{
    NETMGR_LOG_D("Enter GetAllStatsInfo.");
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    if (endtime > netStatsCached_->GetWriteDateTime()) {
        // 跑在非ipc线程防止鉴权失败
        bool isSharingOn = false;
        auto task = ffrt::submit_h([&isSharingOn, this]() { isSharingOn = NetStatsService::IsSharingOn(); }, {}, {},
            ffrt::task_attr().name("isSharingOn"));
        ffrt::wait({task});
        if (isSharingOn) {
            NETMGR_LOG_D("GetSharingStats enter");
            netStatsCached_->GetIptablesStatsCached(sharingStats);
        }
    }
}
#endif

int32_t NetStatsService::GetTrafficStatsByNetwork(std::unordered_map<uint32_t, NetStatsInfo> &infos,
                                                  const NetStatsNetwork &networkIpc)
{
    NETMGR_LOG_D("Enter GetTrafficStatsByNetwork.");
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetTrafficStatsByNetwork start");
    if (netStatsCached_ == nullptr) {
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork(networkIpc);
    if (network == nullptr) {
        NETMGR_LOG_E("param network is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
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
    MergeTrafficStatsByAccount(allInfo);
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

void NetStatsService::MergeTrafficStatsByAccount(std::vector<NetStatsInfo> &infos)
{
    int32_t curUserId = -1;
    int32_t ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(curUserId);
    int32_t defaultUserId = netStatsCached_->GetCurDefaultUserId();
    if (ret != 0) {
        NETMGR_LOG_E("get userId error. ret1: %{public}d", ret);
    }

    if (curUserId == defaultUserId) {
        for (auto &info : infos) {
            if (info.userId_ == netStatsCached_->GetCurPrivateUserId()) {
                info.uid_ = OTHER_ACCOUNT_UID;
            }
        }
    } else if (curUserId == netStatsCached_->GetCurPrivateUserId()) {
        for (auto &info : infos) {
            if (info.userId_ != curUserId) {
                info.uid_ = DEFAULT_ACCOUNT_UID;
            }
        }
    } else {
        NETMGR_LOG_W("curUserId:%{public}d, defaultUserId:%{public}d", curUserId, defaultUserId);
    }
}

int32_t NetStatsService::GetHistoryData(std::vector<NetStatsInfo> &infos, std::string ident,
    uint32_t uid, uint32_t start, uint32_t end)
{
    auto history = std::make_unique<NetStatsHistory>();
    if (history == nullptr) {
        NETMGR_LOG_E("history is null");
        return NETMANAGER_ERR_INTERNAL;
    }
    if (uid != DEFAULT_ACCOUNT_UID && uid != OTHER_ACCOUNT_UID) {
        int32_t ret = history->GetHistory(infos, uid, ident, start, end);
        return ret;
    }
    int32_t userId = -1;
    if (uid == DEFAULT_ACCOUNT_UID) {
        userId = netStatsCached_->GetCurDefaultUserId();
        std::vector<NetStatsInfo> infos1;
        std::vector<NetStatsInfo> infos2;
        history->GetHistoryByIdentAndUserId(infos1, ident, userId, start, end);
        history->GetHistoryByIdentAndUserId(infos2, ident, SYSTEM_DEFAULT_USERID, start, end);
        infos.insert(infos.end(), infos1.begin(), infos1.end());
        infos.insert(infos.end(), infos2.begin(), infos2.end());
    } else if (netStatsCached_->GetCurPrivateUserId() != -1) {
        userId = netStatsCached_->GetCurPrivateUserId();
        history->GetHistoryByIdentAndUserId(infos, ident, userId, start, end);
    }
    if (userId == -1) {
        NETMGR_LOG_E("GetHistoryData error. uid:%{public}u, curPrivateUserId: %{public}d",
            uid, netStatsCached_->GetCurPrivateUserId());
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsService::GetTrafficStatsByUidNetwork(std::vector<NetStatsInfoSequence> &infos, uint32_t uid,
                                                     const NetStatsNetwork &networkIpc)
{
    NETMGR_LOG_D("Enter GetTrafficStatsByUidNetwork.");
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetTrafficStatsByUidNetwork start");
    if (netStatsCached_ == nullptr) {
        NETMGR_LOG_E("Cached is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    sptr<NetStatsNetwork> network = new (std::nothrow) NetStatsNetwork(networkIpc);
    if (network == nullptr) {
        NETMGR_LOG_E("param network is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    std::string ident;
    if (network->type_ == 0) {
        ident = std::to_string(network->simId_);
    }
    uint32_t start = network->startTime_;
    uint32_t end = network->endTime_;
    NETMGR_LOG_D("GetTrafficStatsByUidNetwork param: "
        "uid=%{public}u, ident=%{public}s, start=%{public}u, end=%{public}u", uid, ident.c_str(), start, end);

    std::vector<NetStatsInfo> allInfo;
    int32_t ret = GetHistoryData(allInfo, ident, uid, start, end);
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
        GetSharingStats(allInfo, end);
    }
#endif
    FilterTrafficStatsByUidNetwork(allInfo, infos, uid, ident, start, end);
    DeleteTrafficStatsByAccount(infos, uid);
    NetmanagerHiTrace::NetmanagerStartSyncTrace("NetStatsService GetTrafficStatsByUidNetwork end");
    return NETMANAGER_SUCCESS;
}

void NetStatsService::DeleteTrafficStatsByAccount(std::vector<NetStatsInfoSequence> &infos, uint32_t uid)
{
    int32_t defaultUserId = netStatsCached_->GetCurDefaultUserId();
    if (uid == DEFAULT_ACCOUNT_UID) {
        for (auto it = infos.begin(); it != infos.end();) {
            if (it->info_.userId_ != defaultUserId && it->info_.userId_ != SYSTEM_DEFAULT_USERID) {
                it = infos.erase(it);
            } else {
                ++it;
            }
        }
    } else if (uid == OTHER_ACCOUNT_UID) {
        for (auto it = infos.begin(); it != infos.end();) {
            if (it->info_.userId_ == defaultUserId || it->info_.userId_ == SYSTEM_DEFAULT_USERID) {
                it = infos.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void NetStatsService::FilterTrafficStatsByUidNetwork(std::vector<NetStatsInfo> &allInfo,
    std::vector<NetStatsInfoSequence> &infos, const uint32_t uid,
    const std::string ident, uint32_t startTime, uint32_t endTime)
{
    std::for_each(allInfo.begin(), allInfo.end(),
        [this, &infos, &uid, &ident, &startTime, &endTime](const NetStatsInfo &info) {
        if (uid != DEFAULT_ACCOUNT_UID && uid != OTHER_ACCOUNT_UID && uid != info.uid_) {
            return;
        }

        if (ident != info.ident_ || startTime > info.date_ || endTime < info.date_) {
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
    NETMGR_LOG_D("Enter SetAppStats.");
    int32_t checkPermission = CheckNetManagerAvailable();
    if (checkPermission != NETMANAGER_SUCCESS) {
        return checkPermission;
    }
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
    if (uid / USER_ID_DIVIDOR != netStatsCached_->GetCurDefaultUserId() &&
        uid / USER_ID_DIVIDOR != SYSTEM_DEFAULT_USERID &&
        uid / USER_ID_DIVIDOR != netStatsCached_->GetCurPrivateUserId()) {
        NETMGR_LOG_E("CommonEventPackageRemoved uid:%{public}d", uid);
        return true;
    }
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

int32_t NetStatsService::CheckNetManagerAvailable()
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_LOG_E("Permission check failed.");
        return NETMANAGER_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_STATS)) {
        return NETMANAGER_ERR_PERMISSION_DENIED;
    }
    return NETMANAGER_SUCCESS;
}

void NetStatsService::StartAccountObserver()
{
    NETMGR_LOG_I("StartAccountObserver start");
    std::set<AccountSA::OsAccountState> states = {
        AccountSA::OsAccountState::STOPPING, AccountSA::OsAccountState::CREATED,
        AccountSA::OsAccountState::SWITCHING, AccountSA::OsAccountState::SWITCHED, AccountSA::OsAccountState::UNLOCKED,
        AccountSA::OsAccountState::STOPPED, AccountSA::OsAccountState::REMOVED };
    bool withHandShake = false;
    AccountSA::OsAccountSubscribeInfo subscribeInfo(states, withHandShake);
    accountSubscriber_ = std::make_shared<NetStatsAccountSubscriber>(subscribeInfo);
    ErrCode errCode = AccountSA::OsAccountManager::SubscribeOsAccount(accountSubscriber_);
    if (errCode != 0) {
        NETMGR_LOG_E("SubscribeOsAccount error. errCode:%{public}d", errCode);
    }
    NETMGR_LOG_I("StartAccountObserver end");
}

#ifdef SUPPORT_TRAFFIC_STATISTIC
void NetStatsService::UpdateBpfMapTimer()
{
    NETMGR_LOG_I("UpdateBpfMapTimer start");
    if (!trafficPlanFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return;
    }
#ifndef UNITTEST_FORBID_FFRT
    trafficPlanFfrtQueue_->submit([this]() {
#endif
        int32_t primarySlotId = NetStatsUtils::GetPrimarySlotId();
        int32_t primarySimId = Telephony::CoreServiceClient::GetInstance().GetSimId(primarySlotId);
        int slaveSlotId = primarySlotId == 0 ? 1 : 0;
        int32_t slaveSimId = Telephony::CoreServiceClient::GetInstance().GetSimId(slaveSlotId);
        UpdateBpfMap(primarySimId);
        UpdateBpfMap(slaveSimId);
#ifndef UNITTEST_FORBID_FFRT
    });
#endif
}

bool NetStatsService::CommonEventSimStateChanged(int32_t slotId, int32_t simState)
{
    if (!trafficPlanFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return false;
    }
#ifndef UNITTEST_FORBID_FFRT
    trafficPlanFfrtQueue_->submit([this, slotId, simState]() {
#endif
        CommonEventSimStateChangedFfrt(slotId, simState);
#ifndef UNITTEST_FORBID_FFRT
    });
#endif
    return true;
}

bool NetStatsService::CommonEventSimStateChangedFfrt(int32_t slotId, int32_t simState)
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
        if (settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
            settingsTrafficMap_[simId].first->UnRegisterTrafficDataSettingObserver();
            NETMGR_LOG_I("settingsTrafficMap_.erase(simId). simId:%{public}d", simId);
            settingsTrafficMap_.erase(simId);
        }
    }
    return true;
}

bool NetStatsService::CommonEventCellularDataStateChanged(int32_t slotId, int32_t dataState)
{
    UpdateNetStatusMapCellular(dataState);
    if (!trafficPlanFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return false;
    }
#ifndef UNITTEST_FORBID_FFRT
    trafficPlanFfrtQueue_->submit([this, slotId, dataState]() {
#endif
        CellularDataStateChangedFfrt(slotId, dataState);
#ifndef UNITTEST_FORBID_FFRT
    });
#endif
    return true;
}

void NetStatsService::UpdateNetStatusMapCellular(int32_t dataState)
{
    if (dataState == static_cast<int32_t>(Telephony::DataConnectState::DATA_STATE_CONNECTED)) {
        UpdateNetStatusMap(NetStatusType::CELLULAR_TYPE, NetStatusConn::CONNECTED);
    } else {
        UpdateNetStatusMap(NetStatusType::CELLULAR_TYPE, NetStatusConn::NON_CONNECTED);
    }
}

bool NetStatsService::CellularDataStateChangedFfrt(int32_t slotId, int32_t dataState)
{
    NETMGR_LOG_I("slotId:%{public}d, dateState:%{public}d", slotId, dataState);
    int32_t simId = Telephony::CoreServiceClient::GetInstance().GetSimId(slotId);

    if (dataState != static_cast<int32_t>(Telephony::DataConnectState::DATA_STATE_CONNECTED)) {
        if (simIdToIfIndexMap_.find(simId) != simIdToIfIndexMap_.end()) {
            NETMGR_LOG_E("simIdToIfIndexMap erase, simId: %{public}d", simId);
            ClearTrafficMapBySlotId(slotId, simIdToIfIndexMap_[simId]);
            simIdToIfIndexMap_.erase(simId);
        }
        return true;
    }
    int32_t ret = NetConnClient::GetInstance().GetIfaceNameIdentMaps(
        NetBearType::BEARER_CELLULAR, ifaceNameIdentMap_);
    if (ret != NETMANAGER_SUCCESS || ifaceNameIdentMap_.IsEmpty()) {
        NETMGR_LOG_E("error or empty.ret: %{public}d, ifaceNameIdentMap size: %{public}u",
            ret, ifaceNameIdentMap_.Size());
        return false;
    }
    NETMGR_LOG_I("ifaceNameIdentMap size: %{public}d", ifaceNameIdentMap_.Size());
    uint64_t ifIndex = UINT64_MAX;
    ifaceNameIdentMap_.Iterate([this, simId, &ifIndex](const std::string &k, const std::string &v) {
        if (v == std::to_string(simId)) {
            ifIndex = if_nametoindex(k.c_str());
            NETMGR_LOG_E("curIfIndex_:%{public}" PRIu64, ifIndex);
        }
    });
    if (simIdToIfIndexMap_.find(simId) != simIdToIfIndexMap_.end() && simIdToIfIndexMap_[simId] == ifIndex) {
        NETMGR_LOG_E("not need process");
        return true;
    }
    UpdateCurActiviteSimChanged(simId, ifIndex);
    return true;
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
}


void NetStatsService::UpdateCurActiviteSimChanged(int32_t simId, uint64_t ifIndex)
{
    AddSimIdInTwoMap(simId, ifIndex);
    int32_t slotId = Telephony::CoreServiceClient::GetInstance().GetSlotId(simId);
    if (settingsTrafficMap_[simId].second->monthlyLimit == UINT64_MAX ||
        settingsTrafficMap_[simId].second->unLimitedDataEnable == 1) {
        SetTrafficMapMaxValue(slotId);
    } else {
        UpdateBpfMap(simId);
    }
}

bool NetStatsService::IsSameStateInTwoMap(int32_t simId)
{
    auto ifIndexItem = simIdToIfIndexMap_.find(simId);
    auto settingsItem = settingsTrafficMap_.find(simId);
    if (ifIndexItem == simIdToIfIndexMap_.end() &&
        settingsItem == settingsTrafficMap_.end()) {
        return true;
    }
    if (ifIndexItem != simIdToIfIndexMap_.end() &&
        settingsItem != settingsTrafficMap_.end()) {
        return true;
    }
    return false;
}

void NetStatsService::DeleteSimIdInTwoMap(int32_t simId)
{
    if (simIdToIfIndexMap_.find(simId) != simIdToIfIndexMap_.end() &&
        settingsTrafficMap_.find(simId) != settingsTrafficMap_.end()) {
        simIdToIfIndexMap_.erase(simId);
        settingsTrafficMap_.erase(simId);
    }
}

void NetStatsService::AddSimIdInTwoMap(int32_t simId, uint64_t ifIndex)
{
    NETMGR_LOG_I("AddSimIdInTwoMap. simId:%{public}d, ifIndex:%{public}" PRIu64, simId, ifIndex);
    if (simIdToIfIndexMap_.find(simId) != simIdToIfIndexMap_.end()) {
        int32_t slotId = Telephony::CoreServiceClient::GetInstance().GetSlotId(simId);
        if (slotId != 0 && slotId != 1) {
            NETMGR_LOG_I("SetTrafficMapMaxValue error. slotId: %{public}d", slotId);
            return;
        }
        ClearTrafficMapBySlotId(slotId, simIdToIfIndexMap_[simId]);
    }
    simIdToIfIndexMap_[simId] = ifIndex;

    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("settingsTrafficMap_ not find simId: %{public}d", simId);
        std::shared_ptr<TrafficDataObserver> observer = std::make_shared<TrafficDataObserver>(simId);

        std::shared_ptr<TrafficSettingsInfo> settingsInfo = std::make_shared<TrafficSettingsInfo>();
        observer->ReadTrafficDataSettings(settingsInfo);

        observer->RegisterTrafficDataSettingObserver();
        settingsTrafficMap_.insert(std::make_pair(simId, std::make_pair(observer, settingsInfo)));
        UpdateNetStatsToMapFromDB(simId);
        NETMGR_LOG_I("AddSimIdInTwoMap insert settingsInfo beginDate:%{public}d,\
unLimitedDataEnable:%{public}d, monthlyLimitdNotifyType:%{public}d,\
monthlyLimit:%{public}" PRIu64 ", monthlyMark:%{public}u, dailyMark:%{public}u",
            settingsInfo->beginDate, settingsInfo->unLimitedDataEnable, settingsInfo->monthlyLimitdNotifyType,
            settingsInfo->monthlyLimit, settingsInfo->monthlyMark, settingsInfo->dailyMark);
    }
}

void NetStatsService::ClearTrafficMapBySlotId(int32_t slotId, uint64_t ifIndex)
{
    NETMGR_LOG_I("ClearTrafficMapBySlotId slotId:%{public}d, ifIndex: %{public}lu", slotId, ifIndex);
    NetsysController::GetInstance().DeleteIncreaseTrafficMap(ifIndex);
    NetsysController::GetInstance().UpdateIfIndexMap(slotId, UINT64_MAX);
    SetTrafficMapMaxValue(slotId);
}


int32_t NetStatsService::GetAllUsedTrafficStatsByNetwork(const sptr<NetStatsNetwork> &network, uint64_t &allUsedTraffic)
{
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    int32_t ret = GetTrafficStatsByNetwork(infos, *network);
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

void NetStatsService::UpdateBpfMap(int32_t simId)
{
    NETMGR_LOG_I("UpdateBpfMap start. simId:%{public}d", simId);
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end() ||
        simIdToIfIndexMap_.find(simId) == simIdToIfIndexMap_.end()) {
        NETMGR_LOG_E("simId: %{public}d error", simId);
        return;
    }

    int32_t slotId = Telephony::CoreServiceClient::GetInstance().GetSlotId(simId);
    if (slotId != 0 && slotId != 1) {
        NETMGR_LOG_I("SetTrafficMapMaxValue error. slotId: %{public}d", slotId);
        return;
    }

    NetsysController::GetInstance().DeleteIncreaseTrafficMap(simIdToIfIndexMap_[simId]);
    NetsysController::GetInstance().UpdateIfIndexMap(slotId, simIdToIfIndexMap_[simId]);

    PrintTrafficSettingsMapInfo(simId);

    uint64_t monthlyAvailable = UINT64_MAX;
    uint64_t monthlyMarkAvailable = UINT64_MAX;
    uint64_t dailyMarkAvailable = UINT64_MAX;
    bool ret = CalculateTrafficAvailable(simId, monthlyAvailable, monthlyMarkAvailable, dailyMarkAvailable);
    if (!ret) {
        NETMGR_LOG_E("CalculateTrafficAvailable error or open unlimit");
        return;
    }

    NETMGR_LOG_I("GetTrafficMap before write. monthlyAvailable:%{public}" PRIu64", \
monthlyMarkAvailable:%{public}" PRIu64", dailyMarkAvailable:%{public}" PRIu64,
        monthlyAvailable, monthlyMarkAvailable, dailyMarkAvailable);
    NetsysController::GetInstance().SetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_MONTHLY_LIMIT, monthlyAvailable);
    NetsysController::GetInstance().SetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_MONTHLY_MARK, monthlyMarkAvailable);
    NetsysController::GetInstance().SetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_DAILY_MARK, dailyMarkAvailable);

    PrintTrafficBpfMapInfo(slotId);

    if (settingsTrafficMap_[simId].second->monthlyLimit == UINT64_MAX) {
        return;
    }

    if (monthlyAvailable == UINT64_MAX) {
        NotifyTrafficAlert(simId, NET_STATS_MONTHLY_LIMIT);
    } else if (monthlyMarkAvailable == UINT64_MAX) {
        NotifyTrafficAlert(simId, NET_STATS_MONTHLY_MARK);
    } else if (dailyMarkAvailable == UINT64_MAX) {
        NotifyTrafficAlert(simId, NET_STATS_DAILY_MARK);
    }
}

void NetStatsService::PrintTrafficSettingsMapInfo(int32_t simId)
{
    SettingsInfoPtr info = settingsTrafficMap_[simId].second;
    if (info != nullptr) {
        NETMGR_LOG_I("settingsInfo-> simId:%{public}d, beginDate:%{public}d, unLimitedDataEnable:%{public}d,\
monthlyLimitdNotifyType:%{public}d, monthlyLimit:%{public}" PRIu64 ", monthlyMark:%{public}u,\
dailyMark:%{public}u",
            simId, info->beginDate, info->unLimitedDataEnable, info->monthlyLimitdNotifyType,
            info->monthlyLimit, info->monthlyMark, info->dailyMark);
    }
}

void NetStatsService::PrintTrafficBpfMapInfo(int32_t slotId)
{
    uint64_t monthlyAvailableMap = UINT64_MAX;
    uint64_t monthlyMarkAvailableMap = UINT64_MAX;
    uint64_t dailyMarkAvailableMap = UINT64_MAX;
    NetsysController::GetInstance().GetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_MONTHLY_LIMIT, monthlyAvailableMap);
    NetsysController::GetInstance().GetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_MONTHLY_MARK, monthlyMarkAvailableMap);
    NetsysController::GetInstance().GetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_DAILY_MARK, dailyMarkAvailableMap);
    NETMGR_LOG_I("GetTrafficMap after write. monthlyAvailable:%{public}" PRIu64", \
monthlyMarkAvailable:%{public}" PRIu64", dailyMarkAvailable:%{public}" PRIu64,
        monthlyAvailableMap, monthlyMarkAvailableMap, dailyMarkAvailableMap);
}

bool NetStatsService::CalculateTrafficAvailable(int32_t simId, uint64_t &monthlyAvailable,
    uint64_t &monthlyMarkAvailable, uint64_t &dailyMarkAvailable)
{
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        NETMGR_LOG_E("settingsTrafficMap not find simId, simId is %{public}d", simId);
        return false;
    }

    if (settingsTrafficMap_[simId].second->monthlyLimit == UINT64_MAX) {
        return true;
    }
    sptr<NetStatsNetwork> network = (std::make_unique<NetStatsNetwork>()).release();
    network->startTime_ =
        static_cast<uint64_t>(NetStatsUtils::GetStartTimestamp(settingsTrafficMap_[simId].second->beginDate));
    network->endTime_ = NetStatsUtils::GetNowTimestamp();
    NETMGR_LOG_I("endTime: %{public}lu. simId: %{public}d", network->endTime_, simId);
    network->type_ = 0;
    network->simId_ = static_cast<uint32_t>(simId);
    uint64_t allUsedTraffic = 0;
    int ret = GetAllUsedTrafficStatsByNetwork(network, allUsedTraffic);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("GetAllUsedTrafficStatsByNetwork err. ret: %{public}d", ret);
        return false;
    }

    NETMGR_LOG_I("GetAllUsedTrafficStatsByNetwork allUsedTraffic: %{public}" PRIu64, allUsedTraffic);
    if (settingsTrafficMap_[simId].second->unLimitedDataEnable != 1) {
        if (settingsTrafficMap_[simId].second->monthlyLimit > allUsedTraffic) {
            monthlyAvailable = settingsTrafficMap_[simId].second->monthlyLimit - allUsedTraffic;
        }

        uint64_t monthTmp = (settingsTrafficMap_[simId].second->monthlyLimit / 100.0) *
            settingsTrafficMap_[simId].second->monthlyMark;
        if (monthTmp > allUsedTraffic) {
            monthlyMarkAvailable = monthTmp - allUsedTraffic;
        }
        uint64_t todayStartTime = static_cast<uint64_t>(NetStatsUtils::GetTodayStartTimestamp());
        network->startTime_ = todayStartTime;
        uint64_t allTodayUsedTraffix = 0;
        ret = GetAllUsedTrafficStatsByNetwork(network, allTodayUsedTraffix);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("GetAllUsedTrafficStatsByNetwork err. ret: %{public}d", ret);
            return false;
        }

        uint64_t dayTmp = (settingsTrafficMap_[simId].second->monthlyLimit / 100.0) *
            settingsTrafficMap_[simId].second->dailyMark;
        NETMGR_LOG_I("dayTmp:%{public}" PRIu64 ", allTodayUsedTraffix:%{public}" PRIu64, dayTmp, allTodayUsedTraffix);
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

void NetStatsService::SetTrafficMapMaxValue(int32_t slotId)
{
    NETMGR_LOG_I("SetTrafficMapMaxValue");
    if (slotId != 0 && slotId != 1) {
        NETMGR_LOG_E("SetTrafficMapMaxValue error. slotId: %{public}d", slotId);
        return;
    }
    NetsysController::GetInstance().SetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_MONTHLY_LIMIT, UINT64_MAX);
    NetsysController::GetInstance().SetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_MONTHLY_MARK, UINT64_MAX);
    NetsysController::GetInstance().SetNetStateTrafficMap(
        slotId * TRAFFIC_NOTIFY_TYPE + NET_STATS_DAILY_MARK, UINT64_MAX);
}

void NetStatsService::UpdataSettingsdata(int32_t simId, uint8_t flag, uint64_t value)
{
    if (!trafficPlanFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return;
    }
#ifndef UNITTEST_FORBID_FFRT
    trafficPlanFfrtQueue_->submit([this, simId, flag, value]() {
#endif
        UpdataSettingsdataFfrt(simId, flag, value);
#ifndef UNITTEST_FORBID_FFRT
    });
#endif
}

int32_t NetStatsService::UpdataSettingsdataFfrt(int32_t simId, uint8_t flag, uint64_t value)
{
    NETMGR_LOG_I("UpdataSettingsdata. simId: %{public}d, flag: %{public}d, value: %{public}lu", simId, flag, value);
    auto iter = settingsTrafficMap_.find(simId);
    if (iter == settingsTrafficMap_.end() || iter->second.second == nullptr) {
        NETMGR_LOG_I("iter is nullptr.");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    switch (flag) {
        case NET_STATS_NO_LIMIT_ENABLE:
            if (value == 0 || value == 1) {
                iter->second.second->unLimitedDataEnable = static_cast<int8_t>(value);
            }
            break;
        case NET_STATS_MONTHLY_LIMIT:
                iter->second.second->monthlyLimit = value;
                iter->second.second->isCanNotifyMonthlyLimit = true;
                iter->second.second->isCanNotifyMonthlyMark = true;
                iter->second.second->isCanNotifyDailyMark = true;
                UpdateTrafficLimitDate(simId);
            break;
        case NET_STATS_BEGIN_DATE:
            if (value >= 1 && value <= 31) { // 31: 每月日期数最大值
                iter->second.second->beginDate = static_cast<int32_t>(value);
            }
            break;
        case NET_STATS_NOTIFY_TYPE:
            if (value == 0 || value == 1) {
                iter->second.second->monthlyLimitdNotifyType = static_cast<int8_t>(value);
            }
            break;
        case NET_STATS_MONTHLY_MARK:
            if (value >= 0 && value <= 100) { // 100: 百分比最大值
                iter->second.second->monthlyMark = value;
                iter->second.second->isCanNotifyMonthlyMark = true;
                UpdateTrafficLimitDate(simId);
            }
            break;
        case NET_STATS_DAILY_MARK:
            if (value >= 0 && value <= 100) { // 100: 百分比最大值
                iter->second.second->dailyMark = value;
            }
            break;
        default:
            break;
    }

    if (simIdToIfIndexMap_.find(simId) != simIdToIfIndexMap_.end()) {
        UpdateBpfMap(simId);
    }
    return NETMANAGER_SUCCESS;
}

TrafficObserver::TrafficObserver() {}
TrafficObserver::~TrafficObserver() {}

int32_t TrafficObserver::OnExceedTrafficLimits(int8_t &flag)
{
    NETMGR_LOG_I("OnExceedTrafficLimits flag: %{public}d", flag);
    if (flag < NET_STATS_MONTHLY_LIMIT || flag > NET_STATS_DAILY_MARK + TRAFFIC_NOTIFY_TYPE * 1) {
        NETMGR_LOG_E("OnExceedTrafficLimits flag error. value: %{public}d", flag);
        return -1;
    }

    int8_t slotId = -1;
    if (flag == 0) {
        slotId = 0;
    } else {
        slotId = flag / TRAFFIC_NOTIFY_TYPE;
    }
    int8_t trafficFlag = flag - TRAFFIC_NOTIFY_TYPE * slotId;
    int32_t simId = Telephony::CoreServiceClient::GetInstance().GetSimId(slotId);
    if (simId < 0) {
        NETMGR_LOG_E("get simId error");
        return -1;
    }

    DelayedSingleton<NetStatsService>::GetInstance()->NotifyTrafficAlert(simId, trafficFlag);
    return 0;
}

void NetStatsService::UpdateNetStatsToMapFromDB(int32_t simId)
{
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

int32_t NetStatsService::NotifyTrafficAlert(int32_t simId, uint8_t flag)
{
    if (simIdToIfIndexMap_.find(simId) == simIdToIfIndexMap_.end()) {
        NETMGR_LOG_E("simIdToIfIndexMap not find simId: %{public}d", simId);
        return -1;
    }

    if (NetStatsUtils::IsMobileDataEnabled() && GetNotifyStats(simId, flag)) {
        DealNotificaiton(simId, flag);
    } else {
        NETMGR_LOG_I("There is no need to pop up trafficLimit notification.");
    }
    return NETMANAGER_SUCCESS;
}

bool NetStatsService::GetNotifyStats(int32_t simId, uint8_t flag)
{
    NETMGR_LOG_I("Enter GetNotifyStats.");
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        return false;
    }
    if (settingsTrafficMap_[simId].second->unLimitedDataEnable == 1) {
        NETMGR_LOG_I("simId: %{public}d, setting unLimitedDataEnable: true.", simId);
        return false;
    }
 
    switch (flag) {
        case NET_STATS_MONTHLY_LIMIT:
            return GetMonAlertStatus(simId);
        case NET_STATS_MONTHLY_MARK:
            return GetMonNotifyStatus(simId);
        case NET_STATS_DAILY_MARK:
            return GetDayNotifyStatus(simId);
        default:
            NETMGR_LOG_E("unknown notification type");
            return false;
    }
    return false;
}

bool NetStatsService::GetMonNotifyStatus(int32_t simId)
{
    NETMGR_LOG_I("Enter GetMonNotifyStatus.");
    auto iter = settingsTrafficMap_.find(simId);
    if (iter == settingsTrafficMap_.end() || iter->second.second == nullptr) {
        NETMGR_LOG_E("iter is nullptr.");
        return false;
    }
    if (iter->second.second->isCanNotifyMonthlyMark) {
        iter->second.second->isCanNotifyMonthlyMark = false;
        return true;
    }

    int32_t currentTime = NetStatsUtils::GetNowTimestamp();
    int32_t currentStartTime =
        NetStatsUtils::GetStartTimestamp(iter->second.second->beginDate);
    NETMGR_LOG_I("Enter currentTime:%{public}d, currentDayStartTime:%{public}d, lastMonNotifyTime: %{public}d",
        currentTime, currentStartTime, iter->second.second->lastMonNotifyTime);
    if (iter->second.second->lastMonNotifyTime < currentStartTime) {
        return true;
    }
    return false;
}
 
bool NetStatsService::GetDayNotifyStatus(int32_t simId)
{
    NETMGR_LOG_I("Enter GetDayNotifyStatus.");
    auto iter = settingsTrafficMap_.find(simId);
    if (iter == settingsTrafficMap_.end() || iter->second.second == nullptr) {
        NETMGR_LOG_I("iter is nullptr.");
        return false;
    }
    if (iter->second.second->isCanNotifyDailyMark) {
        iter->second.second->isCanNotifyDailyMark = false;
        return true;
    }
    int32_t currentDayStartTime = NetStatsUtils::GetTodayStartTimestamp();
    NETMGR_LOG_I("Enter currentDayStartTime:%{public}d, lastDayNotifyTime: %{public}d",
        currentDayStartTime, iter->second.second->lastDayNotifyTime);
    if (iter->second.second->lastDayNotifyTime < currentDayStartTime) {
        return true;
    }
    return false;
}
 
bool NetStatsService::GetMonAlertStatus(int32_t simId)
{
    NETMGR_LOG_I("Enter GetMonAlertStatus.");
    auto iter = settingsTrafficMap_.find(simId);
    if (iter == settingsTrafficMap_.end() || iter->second.second == nullptr) {
        NETMGR_LOG_I("iter is nullptr.");
        return false;
    }
    if (iter->second.second->isCanNotifyMonthlyLimit) {
        NETMGR_LOG_I("isCanNotify true : states changed caused.");
        iter->second.second->isCanNotifyMonthlyLimit = false;
        iter->second.second->isCanNotifyMonthlyMark = false;
        iter->second.second->isCanNotifyDailyMark = false;
        return true;
    }
 
    int currentTime = NetStatsUtils::GetNowTimestamp();
    int currentStartTime = NetStatsUtils::GetStartTimestamp(iter->second.second->beginDate);
    NETMGR_LOG_I("Enter currentTime:%{public}d, currentDayStartTime:%{public}d, lastMonAlertTime: %{public}d",
        currentTime, currentStartTime, iter->second.second->lastMonAlertTime);
    if (iter->second.second->lastMonAlertTime < currentStartTime) {
        return true;
    }
    return false;
}

void NetStatsService::DealNotificaiton(int32_t simId, uint8_t flag)
{
    NETMGR_LOG_I("Enter DealDayNotification.");
    int simNum = NetStatsUtils::IsDualCardEnabled();
    bool isDualCard = false;
    if (simNum == 0) {
        return;
    } else if (simNum == DUAL_CARD) {
        isDualCard = true;
    }
 
    switch (flag) {
        case NET_STATS_MONTHLY_LIMIT:
            return DealMonAlert(simId, isDualCard);
        case NET_STATS_MONTHLY_MARK:
            return DealMonNotification(simId, isDualCard);
        case NET_STATS_DAILY_MARK:
            return DealDayNotification(simId, isDualCard);
        default:
            NETMGR_LOG_I("unknown notificationdeal type");
    }
}

void NetStatsService::DealDayNotification(int32_t simId, bool isDualCard)
{
    NETMGR_LOG_I("Enter DealDayNotification.");
    auto iter = settingsTrafficMap_.find(simId);
    if (iter == settingsTrafficMap_.end() || iter->second.second == nullptr) {
        NETMGR_LOG_I("iter is nullptr.");
        return;
    }
    NetMgrNetStatsLimitNotification::GetInstance().PublishNetStatsLimitNotification(NETMGR_STATS_LIMIT_DAY,
                                                                                    simId, isDualCard);
    iter->second.second->lastDayNotifyTime = NetStatsUtils::GetNowTimestamp();
    UpdateTrafficLimitDate(simId);
    NETMGR_LOG_I("update DayNotification time:%{public}d", iter->second.second->lastDayNotifyTime);
}
 
void NetStatsService::DealMonNotification(int32_t simId, bool isDualCard)
{
    NETMGR_LOG_I("Enter DealMonNotification.");
    auto iter = settingsTrafficMap_.find(simId);
    if (iter == settingsTrafficMap_.end() || iter->second.second == nullptr) {
        NETMGR_LOG_I("iter is nullptr.");
        return;
    }
    NetMgrNetStatsLimitNotification::GetInstance().PublishNetStatsLimitNotification(NETMGR_STATS_LIMIT_MONTH,
                                                                                    simId, isDualCard);
    iter->second.second->lastMonNotifyTime = NetStatsUtils::GetNowTimestamp();
    UpdateTrafficLimitDate(simId);
    NETMGR_LOG_I("update MonNotification time:%{public}d", iter->second.second->lastMonNotifyTime);
}

void NetStatsService::DealMonAlert(int32_t simId, bool isDualCard)
{
    NETMGR_LOG_I("Enter DealMonAlert.");
    if (dialog_ == nullptr) {
        dialog_ = std::make_shared<TrafficLimitDialog>();
    }

    auto iter = settingsTrafficMap_.find(simId);
    if (iter == settingsTrafficMap_.end() || iter->second.second == nullptr) {
        NETMGR_LOG_I("iter is nullptr.");
        return;
    }

    NetMgrNetStatsLimitNotification::GetInstance().PublishNetStatsLimitNotification(NETMGR_STATS_ALERT_MONTH,
                                                                                    simId, isDualCard);
    if (iter->second.second->monthlyLimitdNotifyType) {
        dialog_->PopUpTrafficLimitDialog(simId);
    }
    iter->second.second->lastMonAlertTime = NetStatsUtils::GetNowTimestamp();
    UpdateTrafficLimitDate(simId);
    NETMGR_LOG_I("update MonAlert time:%{public}d", iter->second.second->lastMonAlertTime);
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
    statsData.dayNoticeDate = info.second->lastDayNotifyTime;
    statsData.monNoticeDate = info.second->lastMonNotifyTime;
    statsData.monWarningState = info.second->isCanNotifyMonthlyLimit;
    statsData.dayNoticeState = info.second->isCanNotifyDailyMark;
    statsData.monNoticeState = info.second->isCanNotifyMonthlyMark;

    netStats.InsertData(statsData);
}

bool NetStatsService::GetMonthlyLimitBySimId(int32_t simId, uint64_t &monthlyLimit)
{
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        return false;
    }
    monthlyLimit = settingsTrafficMap_[simId].second->monthlyLimit;
    return true;
}

bool NetStatsService::GetMonthlyMarkBySimId(int32_t simId, uint16_t &monthlyMark)
{
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        return false;
    }
    monthlyMark = settingsTrafficMap_[simId].second->monthlyMark;
    return true;
}

bool NetStatsService::GetdailyMarkBySimId(int32_t simId, uint16_t &dailyMark)
{
    if (settingsTrafficMap_.find(simId) == settingsTrafficMap_.end()) {
        return false;
    }
    dailyMark = settingsTrafficMap_[simId].second->dailyMark;
    return true;
}
#endif //SUPPORT_TRAFFIC_STATISTIC
} // namespace NetManagerStandard
} // namespace OHOS
