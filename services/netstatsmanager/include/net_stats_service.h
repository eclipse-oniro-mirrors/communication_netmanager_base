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

#ifndef NET_STATS_SERVICE_H
#define NET_STATS_SERVICE_H

#include "singleton.h"
#include "system_ability.h"

#include "net_push_stats_info.h"
#include "net_stats_cached.h"
#include "net_stats_callback.h"
#include "net_stats_history.h"
#include "net_stats_info_sequence.h"
#include "net_stats_listener.h"
#include "net_stats_network.h"
#include "net_stats_service_stub.h"
#include "netlink_manager.h"
#include "net_bundle.h"
#ifdef SUPPORT_TRAFFIC_STATISTIC
#include "ffrt_timer.h"
#include "net_stats_settings_observer.h"
#include "netsys_traffic_callback_stub.h"
#include "net_info_observer.h"
#include "netsys_controller_callback.h"
#include "net_stats_trafficLimit_dialog.h"
#endif // SUPPORT_TRAFFIC_STATISTIC
#include "network_sharing.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef SUPPORT_TRAFFIC_STATISTIC
using ObserverPtr = std::shared_ptr<TrafficDataObserver>;
using SettingsInfoPtr = std::shared_ptr<TrafficSettingsInfo>;
class TrafficObserver;
class NetsysControllerObserver;
#endif // SUPPORT_TRAFFIC_STATISTIC
class NetStatsService : public SystemAbility,
                        public NetStatsServiceStub,
                        public std::enable_shared_from_this<NetStatsService> {
    DECLARE_DELAYED_SINGLETON(NetStatsService)
    DECLARE_SYSTEM_ABILITY(NetStatsService)

public:
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;
    int32_t GetIfaceRxBytes(uint64_t &stats, const std::string &interfaceName) override;
    int32_t GetIfaceTxBytes(uint64_t &stats, const std::string &interfaceName) override;
    int32_t GetCellularRxBytes(uint64_t &stats) override;
    int32_t GetCellularTxBytes(uint64_t &stats) override;
    int32_t GetAllRxBytes(uint64_t &stats) override;
    int32_t GetAllTxBytes(uint64_t &stats) override;
    int32_t GetUidRxBytes(uint64_t &stats, uint32_t uid) override;
    int32_t GetUidTxBytes(uint64_t &stats, uint32_t uid) override;
    int32_t GetAllStatsInfo(std::vector<NetStatsInfo> &infos) override;
    int32_t GetAllSimStatsInfo(std::vector<NetStatsInfo> &infos) override;
    int32_t GetTrafficStatsByNetwork(std::unordered_map<uint32_t, NetStatsInfo> &infos,
                                     const NetStatsNetwork &networkIpc) override;
    int32_t GetTrafficStatsByUidNetwork(std::vector<NetStatsInfoSequence> &infos, uint32_t uid,
                                        const NetStatsNetwork &networkIpc) override;
    int32_t SetAppStats(const PushStatsInfo &info) override;
    int32_t RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override;
    int32_t UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback) override;
    int32_t GetIfaceStatsDetail(const std::string &iface, uint64_t start, uint64_t end,
                                NetStatsInfo &statsInfo) override;
    int32_t GetUidStatsDetail(const std::string &iface, uint32_t uid, uint64_t start, uint64_t end,
                              NetStatsInfo &statsInfo) override;
    int32_t UpdateIfacesStats(const std::string &iface, uint64_t start, uint64_t end,
                              const NetStatsInfo &stats) override;
    int32_t UpdateStatsData() override;
    int32_t ResetFactory() override;
    int32_t GetCookieRxBytes(uint64_t &stats, uint64_t cookie) override;
    int32_t GetCookieTxBytes(uint64_t &stats, uint64_t cookie) override;
    int32_t SaveSharingTraffic(const NetStatsInfo &infos) override;

#ifdef SUPPORT_TRAFFIC_STATISTIC
    void UpdataSettingsdata(int32_t simId, uint8_t flag, uint64_t value);
    bool ProcessNetConnectionPropertiesChange(int32_t simId, uint64_t ifIndex);
    int32_t GetCurActiviteSimId();
    std::map<int32_t, std::pair<ObserverPtr, SettingsInfoPtr>> GetSettingsObserverMap();
    int32_t NotifyTrafficAlert(uint8_t flag);
#endif // SUPPORT_TRAFFIC_STATISTIC

private:
    bool Init();
    void GetDumpMessage(std::string &message);
    void MergeTrafficStats(std::vector<NetStatsInfoSequence> &statsInfoSequences, const NetStatsInfo &info,
                           uint32_t currentTime);
    bool GetIfaceNamesFromManager(std::list<std::string> &ifaceNames);
    std::unordered_map<uint32_t, SampleBundleInfo> GetSampleBundleInfosForActiveUser();
    SampleBundleInfo GetSampleBundleInfoForUid(uint32_t uid);
    void RefreshUidStatsFlag(uint64_t delay);
    void RegisterCommonEvent();
    bool CommonEventPackageAdded(uint32_t uid);
    bool CommonEventPackageRemoved(uint32_t uid);
    void FilterTrafficStatsByNetwork(std::vector<NetStatsInfo> &allInfo,
        std::unordered_map<uint32_t, NetStatsInfo> &infos,
        const std::string ident, uint32_t startTime, uint32_t endTime);
    void FilterTrafficStatsByUidNetwork(std::vector<NetStatsInfo> &allInfo, std::vector<NetStatsInfoSequence> &infos,
        const uint32_t uid, const std::string ident, uint32_t startTime, uint32_t endTime);
#ifdef SUPPORT_NETWORK_SHARE
    bool IsSharingOn();
    void GetSharingStats(std::vector<NetStatsInfo> &sharingStats, uint32_t endtime);
#endif
#ifdef SUPPORT_TRAFFIC_STATISTIC
    bool CommonEventSimStateChanged(int32_t slotId, int32_t simState);
    bool CommonEventCellularDataStateChanged(int32_t slotId, int32_t dataState);
    int32_t GetAllUsedTrafficStatsByNetwork(const sptr<NetStatsNetwork> &network, uint64_t &allUsedTraffic);
    void UpdateBpfMap();
    void UpdateBeginDate(int32_t simId, int32_t beginDate);
    void UpdateUnLimitedDataEnable(int32_t simId, int8_t unLimitedDataEnable);
    void UpdateMonthlyLimitdNotifyType(int32_t simId, int8_t monthlyLimitdNotifyType);
    void UpdateMonthlyLimit(int32_t simId, uint64_t monthlyLimit);
    void UpdateMonthlyMark(int32_t simId, uint16_t monthlyMark);
    void UpdateDailyMark(int32_t simId, uint16_t dailyMark);
    void SetTrafficMapMaxValue();
    void StartNetObserver();
    void StartTrafficOvserver();
    void StopTrafficOvserver();
    bool GetNotifyStats(uint8_t flag);
    bool GetMonAlertStatus();
    bool GetMonNotifyStatus();
    bool GetDayNotifyStatus();
    void DealMonAlert(bool isDaulCard);
    void DealMonNotification(bool isDaulCard);
    void DealDayNotification(bool isDaulCard);
    void DealNotificaiton(uint8_t flag);
    bool IsMobileDataEnabled();
    void UpdateTrafficLimitDate(int32_t simId);
    void UpdateNetStatsToMapFromDB(int32_t simId);
    void UpdateCurActiviteSimChanged();
    bool CalculateTrafficAvailable(int32_t simId, uint64_t &monthlyAvailable,
        uint64_t &monthlyMarkAvailable, uint64_t &dailyMarkAvailable);
#endif // SUPPORT_TRAFFIC_STATISTIC

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    bool registerToService_;
    ServiceRunningState state_;
    std::shared_ptr<NetStatsCallback> netStatsCallback_ = nullptr;
    std::shared_ptr<NetStatsListener> subscriber_ = nullptr;
    std::unique_ptr<NetStatsCached> netStatsCached_ = nullptr;
#ifdef SUPPORT_TRAFFIC_STATISTIC
    int32_t curActiviteSimId_ = -1;
    uint64_t curIfIndex_ = UINT64_MAX;
    std::atomic_bool isWifiConnected_ = false;
    std::map<int32_t, std::pair<ObserverPtr, SettingsInfoPtr>> settingsTrafficMap_;
    std::unique_ptr<FfrtTimer> trafficTimer_ = nullptr;
    sptr<NetInfoObserver> netconnCallback_ = nullptr;
    sptr<TrafficObserver> trafficObserver_ = nullptr;
    sptr<NetsysControllerCallback> netsysControllerObserver_ = nullptr;
    SafeMap<std::string, std::string> ifaceNameIdentMap_;
    std::shared_ptr<TrafficLimitDialog> dialog_ = nullptr;
#endif // SUPPORT_TRAFFIC_STATISTIC
};

#ifdef SUPPORT_TRAFFIC_STATISTIC
class TrafficObserver : public NetsysNative::NetsysTrafficCallbackStub {
public:
    TrafficObserver();
    ~TrafficObserver() override;
    int32_t OnExceedTrafficLimits(int8_t &flag) override;
};
#endif // SUPPORT_TRAFFIC_STATISTIC
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_SERVICE_H
