/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "net_stats_cached.h"

#include <initializer_list>
#include <list>
#include <pthread.h>

#include "net_conn_client.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"
#include "net_stats_data_handler.h"
#include "net_stats_database_defines.h"
#include "net_stats_database_helper.h"
#include "netsys_controller.h"
#include "bpf_stats.h"
#include "ffrt_inner.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace NetStatsDatabaseDefines;
namespace {
constexpr const char *IFACE_LO = "lo";
} // namespace

int32_t NetStatsCached::StartCached()
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    auto ret = helper->CreateTable(UID_TABLE, UID_TABLE_CREATE_PARAM);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Create uid table failed");
        return STATS_ERR_CREATE_TABLE_FAIL;
    }
    ret = helper->CreateTable(IFACE_TABLE, IFACE_TABLE_CREATE_PARAM);
    if (ret != 0) {
        NETMGR_LOG_E("Create iface table failed");
        return STATS_ERR_CREATE_TABLE_FAIL;
    }
    ret = helper->CreateTable(UID_SIM_TABLE, UID_SIM_TABLE_CREATE_PARAM);
    if (ret != 0) {
        NETMGR_LOG_E("Create uid_sim table failed");
        return STATS_ERR_CREATE_TABLE_FAIL;
    }
    cacheTimer_ = std::make_unique<FfrtTimer>();
    writeTimer_ = std::make_unique<FfrtTimer>();
    cacheTimer_->Start(cycleThreshold_, [this]() { CacheStats(); });
    writeTimer_->Start(STATS_PACKET_CYCLE_MS, [this]() { WriteStats(); });
    return ret;
}

void NetStatsCached::GetUidStatsCached(std::vector<NetStatsInfo> &uidStatsInfo)
{
    uidStatsInfo.insert(uidStatsInfo.end(), stats_.GetUidStatsInfo().begin(), stats_.GetUidStatsInfo().end());
}

void NetStatsCached::GetUidSimStatsCached(std::vector<NetStatsInfo> &uidSimStatsInfo)
{
    uidSimStatsInfo.insert(uidSimStatsInfo.end(), stats_.GetUidSimStatsInfo().begin(),
                           stats_.GetUidSimStatsInfo().end());
}

void NetStatsCached::GetUidPushStatsCached(std::vector<NetStatsInfo> &uidPushStatsInfo)
{
    uidPushStatsInfo.insert(uidPushStatsInfo.end(), uidPushStatsInfo_.begin(), uidPushStatsInfo_.end());
}

void NetStatsCached::GetIfaceStatsCached(std::vector<NetStatsInfo> &ifaceStatsInfo)
{
    ifaceStatsInfo.insert(ifaceStatsInfo.end(), stats_.GetIfaceStatsInfo().begin(), stats_.GetIfaceStatsInfo().end());
}

void NetStatsCached::SetAppStats(const PushStatsInfo &info)
{
    std::lock_guard<ffrt::mutex> lock(pushLock_);
    NetStatsInfo stats;
    stats.uid_ = info.uid_;
    stats.iface_ = info.iface_;
    stats.date_ = info.endTime_;
    stats.rxBytes_ = info.rxBytes_;
    stats.txBytes_ = info.txBytes_;
    stats.rxPackets_ = 1;
    stats.txPackets_ = 1;
    stats.ident_ = std::to_string(info.simId_);
    NETMGR_LOG_D("SetAppStats info=%{public}s", stats.UidData().c_str());
    uidPushStatsInfo_.push_back(std::move(stats));
}

void NetStatsCached::GetKernelStats(std::vector<NetStatsInfo> &statsInfo)
{
    std::vector<NetStatsInfo> allInfos;
    NetsysController::GetInstance().GetAllStatsInfo(allInfos);
    std::vector<NetStatsInfo> containerInfos;
    NetsysController::GetInstance().GetAllContainerStatsInfo(containerInfos);
    allInfos.insert(allInfos.end(), containerInfos.begin(), containerInfos.end());

    LoadIfaceNameIdentMaps();
    std::for_each(allInfos.begin(), allInfos.end(), [this, &statsInfo](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        info.ident_ = ifaceNameIdentMap_[info.iface_];
        NetStatsInfo tmp = GetIncreasedStats(info);
        if (tmp.HasNoData()) {
            return;
        }
        tmp.date_ = CommonUtils::GetCurrentSecond();
        statsInfo.push_back(std::move(tmp));
    });
}

NetStatsInfo NetStatsCached::GetIncreasedStats(const NetStatsInfo &info)
{
    auto findRet = std::find_if(lastUidStatsInfo_.begin(), lastUidStatsInfo_.end(),
                                [&info](const NetStatsInfo &lastInfo) { return info.Equals(lastInfo); });
    if (findRet == lastUidStatsInfo_.end()) {
        return info;
    }
    auto currentStats = info - *findRet;
    return currentStats;
}

void NetStatsCached::CacheUidStats()
{
    std::vector<NetStatsInfo> statsInfos;
    NetsysController::GetInstance().GetAllStatsInfo(statsInfos);
    if (statsInfos.empty()) {
        NETMGR_LOG_W("No stats need to save");
        return;
    }

    LoadIfaceNameIdentMaps();

    std::for_each(statsInfos.begin(), statsInfos.end(), [this](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        info.ident_ = ifaceNameIdentMap_[info.iface_];
        std::for_each(uidPushStatsInfo_.begin(), uidPushStatsInfo_.end(), [&info](const NetStatsInfo &pushInfo) {
            if (pushInfo.Equals(info)) {
                info += pushInfo;
            }
        });
        auto findRet = std::find_if(lastUidStatsInfo_.begin(), lastUidStatsInfo_.end(),
                                    [this, &info](const NetStatsInfo &lastInfo) { return info.Equals(lastInfo); });
        if (findRet == lastUidStatsInfo_.end()) {
            stats_.PushUidStats(info);
            return;
        }
        auto currentStats = info - *findRet;
        stats_.PushUidStats(currentStats);
    });
    lastUidStatsInfo_.clear();
    lastUidStatsInfo_ = std::move(statsInfos);
    uidPushStatsInfo_.clear();
}

void NetStatsCached::CacheUidSimStats()
{
    std::vector<NetStatsInfo> statsInfos;
    NetsysController::GetInstance().GetAllContainerStatsInfo(statsInfos);
    if (statsInfos.empty()) {
        NETMGR_LOG_W("No stats need to save");
        return;
    }

    LoadIfaceNameIdentMaps();

    std::for_each(statsInfos.begin(), statsInfos.end(), [this](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        info.ident_ = ifaceNameIdentMap_[info.iface_];
        auto findRet = std::find_if(lastUidSimStatsInfo_.begin(), lastUidSimStatsInfo_.end(),
                                    [this, &info](const NetStatsInfo &lastInfo) { return info.Equals(lastInfo); });
        if (findRet == lastUidSimStatsInfo_.end()) {
            stats_.PushUidStats(info);
            return;
        }
        auto currentStats = info - *findRet;
        stats_.PushUidSimStats(currentStats);
    });
    lastUidSimStatsInfo_.clear();
    lastUidSimStatsInfo_ = std::move(statsInfos);
}

void NetStatsCached::CacheIfaceStats()
{
    std::vector<std::string> ifNameList = NetsysController::GetInstance().InterfaceGetList();
    std::for_each(ifNameList.begin(), ifNameList.end(), [this](const auto &ifName) {
        if (ifName == IFACE_LO) {
            return;
        }
        NetStatsInfo statsInfo;
        statsInfo.iface_ = ifName;
        NetsysController::GetInstance().GetIfaceStats(statsInfo.rxBytes_,
                                                      static_cast<uint32_t>(StatsType::STATS_TYPE_RX_BYTES), ifName);
        NetsysController::GetInstance().GetIfaceStats(statsInfo.rxPackets_,
                                                      static_cast<uint32_t>(StatsType::STATS_TYPE_RX_PACKETS), ifName);
        NetsysController::GetInstance().GetIfaceStats(statsInfo.txBytes_,
                                                      static_cast<uint32_t>(StatsType::STATS_TYPE_TX_BYTES), ifName);
        NetsysController::GetInstance().GetIfaceStats(statsInfo.txPackets_,
                                                      static_cast<uint32_t>(StatsType::STATS_TYPE_TX_PACKETS), ifName);
        auto findRet = lastIfaceStatsMap_.find(ifName);
        if (findRet == lastIfaceStatsMap_.end()) {
            stats_.PushIfaceStats(statsInfo);
            lastIfaceStatsMap_[ifName] = statsInfo;
            return;
        }
        auto currentStats = statsInfo - findRet->second;
        stats_.PushIfaceStats(currentStats);
        lastIfaceStatsMap_[ifName] = statsInfo;
    });
}

void NetStatsCached::CacheStats()
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    CacheUidStats();
    CacheUidSimStats();
    CacheIfaceStats();
}

void NetStatsCached::WriteStats()
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    WriteUidStats();
    WriteUidSimStats();
    WriteIfaceStats();
}
void NetStatsCached::WriteIfaceStats()
{
    if (!(CheckIfaceStor() || isForce_)) {
        return;
    }
    auto handler = std::make_unique<NetStatsDataHandler>();
    handler->WriteStatsData(stats_.GetIfaceStatsInfo(), NetStatsDatabaseDefines::IFACE_TABLE);
    handler->DeleteByDate(NetStatsDatabaseDefines::IFACE_TABLE, 0, CommonUtils::GetCurrentSecond() - dateCycle_);
    stats_.ResetIfaceStats();
}

void NetStatsCached::WriteUidStats()
{
    if (!(CheckUidStor() || isForce_)) {
        return;
    }
    auto handler = std::make_unique<NetStatsDataHandler>();
    handler->WriteStatsData(stats_.GetUidStatsInfo(), NetStatsDatabaseDefines::UID_TABLE);
    handler->DeleteByDate(NetStatsDatabaseDefines::UID_TABLE, 0, CommonUtils::GetCurrentSecond() - dateCycle_);
    stats_.ResetUidStats();
}

void NetStatsCached::WriteUidSimStats()
{
    if (!(CheckUidStor() || isForce_)) {
        return;
    }
    auto handler = std::make_unique<NetStatsDataHandler>();
    handler->WriteStatsData(stats_.GetUidSimStatsInfo(), NetStatsDatabaseDefines::UID_SIM_TABLE);
    handler->DeleteByDate(NetStatsDatabaseDefines::UID_SIM_TABLE, 0, CommonUtils::GetCurrentSecond() - dateCycle_);
    stats_.ResetUidSimStats();
}

void NetStatsCached::LoadIfaceNameIdentMaps()
{
    if (isIfaceNameIdentMapLoaded_.load()) {
        NETMGR_LOG_D("ifaceNameIdentMaps has been loaded from netConnClient.");
        return;
    }
    int32_t ret = NetConnClient::GetInstance().GetIfaceNameIdentMaps(NetBearType::BEARER_CELLULAR, ifaceNameIdentMap_);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("GetIfaceNameIdentMaps error. ret=%{public}d", ret);
        return;
    }
    isIfaceNameIdentMapLoaded_.store(true);
}

void NetStatsCached::SetCycleThreshold(uint32_t threshold)
{
    NETMGR_LOG_D("Current cycle threshold has changed current is : %{public}d", threshold);
    cycleThreshold_ = threshold;
    cacheTimer_ = std::make_unique<FfrtTimer>();
    cacheTimer_->Start(cycleThreshold_, [this]() { CacheStats(); });
}

void NetStatsCached::ForceUpdateStats()
{
    if (isIfaceNameIdentMapLoaded_.load()) {
        NETMGR_LOG_D("ifaceNameIdentMaps need to reload from netConnClient.");
        isIfaceNameIdentMapLoaded_.store(false);
    }
    isForce_ = true;
    std::function<void()> netCachedStats = [this] () {
        CacheStats();
        WriteStats();
        isForce_ = false;
    };
    ffrt::submit(std::move(netCachedStats), {}, {}, ffrt::task_attr().name("NetCachedStats"));
}

void NetStatsCached::Reset() {}

} // namespace NetManagerStandard
} // namespace OHOS
