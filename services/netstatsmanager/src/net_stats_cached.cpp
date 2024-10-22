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
    auto ret = helper->CreateTable(VERSION_TABLE, VERSION_TABLE_CREATE_PARAM);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Create version table failed");
        return STATS_ERR_CREATE_TABLE_FAIL;
    }
    ret = helper->CreateTable(UID_TABLE, UID_TABLE_CREATE_PARAM);
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
    helper->Upgrade();
    cacheTimer_ = std::make_unique<FfrtTimer>();
    writeTimer_ = std::make_unique<FfrtTimer>();
    cacheTimer_->Start(cycleThreshold_, [this]() { CacheStats(); });
    writeTimer_->Start(STATS_PACKET_CYCLE_MS, [this]() { WriteStats(); });
    return ret;
}

void NetStatsCached::GetUidStatsCached(std::vector<NetStatsInfo> &uidStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    uidStatsInfo.insert(uidStatsInfo.end(), stats_.GetUidStatsInfo().begin(), stats_.GetUidStatsInfo().end());
}

void NetStatsCached::GetUidSimStatsCached(std::vector<NetStatsInfo> &uidSimStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    std::transform(stats_.GetUidSimStatsInfo().begin(), stats_.GetUidSimStatsInfo().end(),
                   std::back_inserter(uidSimStatsInfo), [](NetStatsInfo &info) {
                       NetStatsInfo tmpInfo = info;
                       tmpInfo.uid_ = Sim_UID;
                       return tmpInfo;
                   });
}

void NetStatsCached::GetUidPushStatsCached(std::vector<NetStatsInfo> &uidPushStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    uidPushStatsInfo.insert(uidPushStatsInfo.end(), uidPushStatsInfo_.begin(), uidPushStatsInfo_.end());
}

void NetStatsCached::GetAllPushStatsCached(std::vector<NetStatsInfo> &uidPushStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    uidPushStatsInfo.insert(uidPushStatsInfo.end(), allPushStatsInfo_.begin(), allPushStatsInfo_.end());
}

void NetStatsCached::GetIfaceStatsCached(std::vector<NetStatsInfo> &ifaceStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    ifaceStatsInfo.insert(ifaceStatsInfo.end(), stats_.GetIfaceStatsInfo().begin(), stats_.GetIfaceStatsInfo().end());
}

void NetStatsCached::SetAppStats(const PushStatsInfo &info)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    NetStatsInfo stats;
    stats.uid_ = info.uid_;
    stats.iface_ = info.iface_;
    stats.date_ = info.endTime_;
    stats.rxBytes_ = info.rxBytes_;
    stats.txBytes_ = info.txBytes_;
    stats.rxPackets_ = info.rxBytes_ > 0 ? 1 : 0;
    stats.txPackets_ = info.txBytes_ > 0 ? 1 : 0;
    if (info.netBearType_ == BEARER_CELLULAR) {
        stats.ident_ = std::to_string(info.simId_);
    }
    NETMGR_LOG_D("SetAppStats info=%{public}s", stats.UidData().c_str());
    uidPushStatsInfo_.push_back(std::move(stats));
}

void NetStatsCached::GetKernelStats(std::vector<NetStatsInfo> &statsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    std::vector<NetStatsInfo> allInfos;
    NetsysController::GetInstance().GetAllStatsInfo(allInfos);
    ifaceNameIdentMap_.Iterate([&allInfos](const std::string &k, const std::string &v) {
        std::for_each(allInfos.begin(), allInfos.end(), [&k, &v](NetStatsInfo &item) {
            if (item.iface_ == k) {
                item.ident_ = v;
            }
        });
    });
    std::for_each(allInfos.begin(), allInfos.end(), [this, &statsInfo](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        NetStatsInfo tmp = GetIncreasedStats(info);
        if (tmp.HasNoData()) {
            return;
        }
        tmp.date_ = CommonUtils::GetCurrentSecond();
        statsInfo.push_back(std::move(tmp));
    });
    std::vector<NetStatsInfo> SimInfos;
    NetsysController::GetInstance().GetAllSimStatsInfo(SimInfos);
    ifaceNameIdentMap_.Iterate([&SimInfos](const std::string &k, const std::string &v) {
        std::for_each(SimInfos.begin(), SimInfos.end(), [&k, &v](NetStatsInfo &item) {
            if (item.iface_ == k) {
                item.ident_ = v;
            }
        });
    });
    std::for_each(SimInfos.begin(), SimInfos.end(), [this, &statsInfo](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        NetStatsInfo tmp = GetIncreasedSimStats(info);
        if (tmp.HasNoData()) {
            return;
        }
        tmp.date_ = CommonUtils::GetCurrentSecond();
        tmp.uid_ = Sim_UID;
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
    return info - *findRet;
}

NetStatsInfo NetStatsCached::GetIncreasedSimStats(const NetStatsInfo &info)
{
    auto findRet = std::find_if(lastUidSimStatsInfo_.begin(), lastUidSimStatsInfo_.end(),
                                [&info](const NetStatsInfo &lastInfo) { return info.Equals(lastInfo); });
    if (findRet == lastUidSimStatsInfo_.end()) {
        return info;
    }
    return info - *findRet;
}

void NetStatsCached::CacheUidStats()
{
    std::vector<NetStatsInfo> statsInfos;
    NetsysController::GetInstance().GetAllStatsInfo(statsInfos);
    if (statsInfos.empty()) {
        NETMGR_LOG_W("No stats need to save");
        return;
    }

    ifaceNameIdentMap_.Iterate([&statsInfos](const std::string &k, const std::string &v) {
        std::for_each(statsInfos.begin(), statsInfos.end(), [&k, &v](NetStatsInfo &item) {
            if (item.iface_ == k) {
                item.ident_ = v;
            }
        });
    });

    std::for_each(statsInfos.begin(), statsInfos.end(), [this](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        auto findRet = std::find_if(lastUidStatsInfo_.begin(), lastUidStatsInfo_.end(),
                                    [this, &info](const NetStatsInfo &lastInfo) { return info.Equals(lastInfo); });
        if (findRet == lastUidStatsInfo_.end()) {
            stats_.PushUidStats(info);
            return;
        }
        auto currentStats = info - *findRet;
        stats_.PushUidStats(currentStats);
    });
    lastUidStatsInfo_.swap(statsInfos);
}

void NetStatsCached::CacheAppStats()
{
    std::vector<NetStatsInfo> pushInfos;
    std::for_each(uidPushStatsInfo_.begin(), uidPushStatsInfo_.end(), [&pushInfos](NetStatsInfo &info) {
        auto findRet = std::find_if(pushInfos.begin(), pushInfos.end(),
                                    [&info](const NetStatsInfo &item) { return info.Equals(item); });
        if (findRet == pushInfos.end()) {
            pushInfos.push_back(info);
            return;
        }
        *findRet += info;
    });
    std::for_each(pushInfos.begin(), pushInfos.end(), [this](auto &item) {
        stats_.PushUidStats(item);
        auto findRet = std::find_if(allPushStatsInfo_.begin(), allPushStatsInfo_.end(),
                                    [&item](const NetStatsInfo &info) {
                                        return info.Equals(item) && info.ident_ == item.ident_;
                                    });
        if (findRet == allPushStatsInfo_.end()) {
            allPushStatsInfo_.push_back(item);
            return;
        }
        *findRet += item;
    });
    uidPushStatsInfo_.clear();
}

void NetStatsCached::CacheUidSimStats()
{
    std::vector<NetStatsInfo> statsInfos;
    NetsysController::GetInstance().GetAllSimStatsInfo(statsInfos);
    if (statsInfos.empty()) {
        NETMGR_LOG_W("No stats need to save");
        return;
    }

    ifaceNameIdentMap_.Iterate([&statsInfos](const std::string &k, const std::string &v) {
        std::for_each(statsInfos.begin(), statsInfos.end(), [&k, &v](NetStatsInfo &item) {
            if (item.iface_ == k) {
                item.ident_ = v;
            }
        });
    });

    std::for_each(statsInfos.begin(), statsInfos.end(), [this](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        auto findRet = std::find_if(lastUidSimStatsInfo_.begin(), lastUidSimStatsInfo_.end(),
                                    [this, &info](const NetStatsInfo &lastInfo) { return info.Equals(lastInfo); });
        if (findRet == lastUidSimStatsInfo_.end()) {
            stats_.PushUidSimStats(info);
            return;
        }
        auto currentStats = info - *findRet;
        stats_.PushUidSimStats(currentStats);
    });
    lastUidSimStatsInfo_.swap(statsInfos);
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
    CacheAppStats();
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
    std::for_each(stats_.GetUidStatsInfo().begin(), stats_.GetUidStatsInfo().end(), [this](NetStatsInfo &info) {
        if (info.uid_ == uninstalledUid_) {
            info.flag_ = STATS_DATA_FLAG_UNINSTALLED;
        }
    });
    auto handler = std::make_unique<NetStatsDataHandler>();
    handler->WriteStatsData(stats_.GetUidStatsInfo(), NetStatsDatabaseDefines::UID_TABLE);
    handler->DeleteByDate(NetStatsDatabaseDefines::UID_TABLE, 0, CommonUtils::GetCurrentSecond() - dateCycle_);
    stats_.ResetUidStats();
}

void NetStatsCached::WriteUidSimStats()
{
    if (!(CheckUidSimStor() || isForce_)) {
        return;
    }
    std::for_each(stats_.GetUidSimStatsInfo().begin(), stats_.GetUidSimStatsInfo().end(), [this](NetStatsInfo &info) {
        if (info.uid_ == uninstalledUid_) {
            info.flag_ = STATS_DATA_FLAG_UNINSTALLED;
        }
    });
    auto handler = std::make_unique<NetStatsDataHandler>();
    handler->WriteStatsData(stats_.GetUidSimStatsInfo(), NetStatsDatabaseDefines::UID_SIM_TABLE);
    handler->DeleteByDate(NetStatsDatabaseDefines::UID_SIM_TABLE, 0, CommonUtils::GetCurrentSecond() - dateCycle_);
    handler->DeleteSimStatsByUid(Sim_UID);
    stats_.ResetUidSimStats();
}

void NetStatsCached::LoadIfaceNameIdentMaps()
{
    int32_t ret = NetConnClient::GetInstance().GetIfaceNameIdentMaps(NetBearType::BEARER_CELLULAR, ifaceNameIdentMap_);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("GetIfaceNameIdentMaps error. ret=%{public}d", ret);
    }
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
    isForce_ = true;
    std::function<void()> netCachedStats = [this] () {
        CacheStats();
        WriteStats();
        isForce_ = false;
        LoadIfaceNameIdentMaps();
    };
    ffrt::submit(std::move(netCachedStats), {}, {}, ffrt::task_attr().name("NetCachedStats"));
}

void NetStatsCached::ForceDeleteStats(uint32_t uid)
{
    NETMGR_LOG_I("ForceDeleteStats Enter uid[%{public}u]", uid);
    auto ret = NetsysController::GetInstance().DeleteStatsInfo(uid);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("ForceDeleteStats DeleteStatsInfo failed. ret is %{public}d", ret);
    }
    ret = NetsysController::GetInstance().DeleteSimStatsInfo(uid);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("ForceDeleteStats DeleteSimStatsInfo failed. ret is %{public}d", ret);
    }

    std::lock_guard<ffrt::mutex> lock(lock_);
    stats_.ResetUidStats(uid);
    stats_.ResetUidSimStats(uid);
    lastUidStatsInfo_.erase(std::remove_if(lastUidStatsInfo_.begin(), lastUidStatsInfo_.end(),
                                           [uid](const auto &item) { return item.uid_ == uid; }),
                            lastUidStatsInfo_.end());
    lastUidSimStatsInfo_.erase(std::remove_if(lastUidSimStatsInfo_.begin(), lastUidSimStatsInfo_.end(),
                                              [uid](const auto &item) { return item.uid_ == uid; }),
                               lastUidSimStatsInfo_.end());
    uidPushStatsInfo_.erase(std::remove_if(uidPushStatsInfo_.begin(), uidPushStatsInfo_.end(),
                                           [uid](const auto &item) { return item.uid_ == uid; }),
                            uidPushStatsInfo_.end());
}

void NetStatsCached::ForceArchiveStats(uint32_t uid)
{
    std::function<void()> netCachedStats = [this, uid]() {
        CacheStats();
        {
            std::lock_guard<ffrt::mutex> lock(lock_);
            isForce_ = true;
            uninstalledUid_ = uid;
            WriteUidStats();
            WriteUidSimStats();
            uninstalledUid_ = -1;
            isForce_ = false;
        }
        ForceDeleteStats(uid);
    };
    ffrt::submit(std::move(netCachedStats), {}, {}, ffrt::task_attr().name("NetForceArchiveStats"));
}

void NetStatsCached::Reset() {}

} // namespace NetManagerStandard
} // namespace OHOS
