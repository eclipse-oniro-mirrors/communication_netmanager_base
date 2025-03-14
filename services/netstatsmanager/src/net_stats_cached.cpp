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
constexpr const char *INSTALL_SOURCE_DEFAULT = "default";
const std::string CELLULAR_IFACE_NAME = "rmnet";
} // namespace
const int8_t RETRY_TIME = 3;

NetStatsCached::NetStatsCached()
{
    isDisplayTrafficAncoList = CommonUtils::IsNeedDisplayTrafficAncoList();
}

int32_t NetStatsCached::StartCached()
{
    auto ret = CreatNetStatsTables();
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("CreatNetStatsTables error. ret: %{public}d", ret);
        return ret;
    }
#ifndef UNITTEST_FORBID_FFRT
    cacheTimer_ = std::make_unique<FfrtTimer>();
    writeTimer_ = std::make_unique<FfrtTimer>();
    cacheTimer_->StartPro(cycleThreshold_, this, [](void *netStatsCachedPtr) -> void {
        if (netStatsCachedPtr != nullptr) {
            NetStatsCached *netStatsCached = reinterpret_cast<NetStatsCached *>(netStatsCachedPtr);
            netStatsCached->CacheStats();
        } else {
            NETMGR_LOG_E("not NetStatsCached obj");
        }
    });
    writeTimer_->StartPro(STATS_PACKET_CYCLE_MS, this, [](void *netStatsCachedPtr) -> void {
        if (netStatsCachedPtr != nullptr) {
            NetStatsCached *netStatsCached = reinterpret_cast<NetStatsCached *>(netStatsCachedPtr);
            netStatsCached->WriteStats();
        } else {
            NETMGR_LOG_E("not NetStatsCached obj");
        }
    });
#endif
    return ret;
}

int32_t NetStatsCached::CreatNetStatsTables()
{
    auto helper = std::make_unique<NetStatsDatabaseHelper>(NET_STATS_DATABASE_PATH);
    int8_t curRetryTimes = 0;
    int32_t ret = -1;
    while (curRetryTimes < RETRY_TIME) {
        NETMGR_LOG_I("Create table times: %{public}d", curRetryTimes + 1);
        ret = helper->CreateTable(VERSION_TABLE, VERSION_TABLE_CREATE_PARAM);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Create version table failed");
            curRetryTimes++;
            continue;
        }
        ret = helper->CreateTable(UID_TABLE, UID_TABLE_CREATE_PARAM);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Create uid table failed");
            curRetryTimes++;
            continue;
        }
        ret = helper->CreateTable(IFACE_TABLE, IFACE_TABLE_CREATE_PARAM);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Create iface table failed");
            curRetryTimes++;
            continue;
        }
        ret = helper->CreateTable(UID_SIM_TABLE, UID_SIM_TABLE_CREATE_PARAM);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Create uid_sim table failed");
            curRetryTimes++;
            continue;
        }
        break;
    }
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Create table failed");
        return STATS_ERR_CREATE_TABLE_FAIL;
    }
    helper->Upgrade();
    return NETMANAGER_SUCCESS;
}

void NetStatsCached::GetUidStatsCached(std::vector<NetStatsInfo> &uidStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    uidStatsInfo.insert(uidStatsInfo.end(), stats_.GetUidStatsInfo().begin(), stats_.GetUidStatsInfo().end());
}

void NetStatsCached::GetUidSimStatsCached(std::vector<NetStatsInfo> &uidSimStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    std::vector<NetStatsInfo> tmpList;
    std::transform(stats_.GetUidSimStatsInfo().begin(), stats_.GetUidSimStatsInfo().end(), std::back_inserter(tmpList),
                   [](NetStatsInfo &info) {
                       NetStatsInfo tmpInfo = info;
                       return tmpInfo;
                   });
    tmpList.erase(std::remove_if(tmpList.begin(), tmpList.end(), [](const auto &item) {
                      return item.flag_ <= STATS_DATA_FLAG_DEFAULT || item.flag_ >= STATS_DATA_FLAG_LIMIT;
                  }), tmpList.end());
    std::transform(tmpList.begin(), tmpList.end(), std::back_inserter(uidSimStatsInfo), [](NetStatsInfo &info) {
        if (!isDisplayTrafficAncoList) {
            if (info.flag_ == STATS_DATA_FLAG_SIM2) {
                info.uid_ = SIM2_UID;
            } else if (info.flag_ == STATS_DATA_FLAG_SIM) {
                info.uid_ = Sim_UID;
            }
        } else {
            if (info.flag_ == STATS_DATA_FLAG_SIM_BASIC) {
                info.uid_ = Sim_UID;
            } else if (info.flag_ == STATS_DATA_FLAG_SIM2_BASIC) {
                info.uid_ = SIM2_UID;
            }
        }
        return info;
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
    GetKernelUidStats(statsInfo);
    GetKernelUidSimStats(statsInfo);
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
    uidStatsFlagMap_.Iterate([&statsInfos](const uint32_t &k, const NetStatsDataFlag &v) {
        std::for_each(statsInfos.begin(), statsInfos.end(), [&k, &v](NetStatsInfo &item) {
            if (item.uid_ == k) {
                item.flag_ = v;
            }
        });
    });

    std::for_each(statsInfos.begin(), statsInfos.end(), [this](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        if (info.flag_ <= STATS_DATA_FLAG_DEFAULT || info.flag_ >= STATS_DATA_FLAG_LIMIT) {
            info.flag_ = GetUidStatsFlag(info.uid_);
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
#ifdef SUPPORT_NETWORK_SHARE
    CacheIptablesStats();
#endif
}

void NetStatsCached::WriteStats()
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    WriteUidStats();
    WriteUidSimStats();
    WriteIfaceStats();
#ifdef SUPPORT_NETWORK_SHARE
    WriteIptablesStats();
    writeDate_ = CommonUtils::GetCurrentSecond();
#endif
}

#ifdef SUPPORT_NETWORK_SHARE
uint64_t NetStatsCached::GetWriteDateTime()
{
    return writeDate_;
}
#endif

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
    stats_.ResetUidSimStats();
}

#ifdef SUPPORT_NETWORK_SHARE
void NetStatsCached::WriteIptablesStats()
{
    if (!(CheckIptablesStor() || isForce_)) {
        return;
    }
    auto handler = std::make_unique<NetStatsDataHandler>();
    handler->WriteStatsData(stats_.GetIptablesStatsInfo(), NetStatsDatabaseDefines::UID_TABLE);
    handler->DeleteByDate(NetStatsDatabaseDefines::UID_TABLE, 0, CommonUtils::GetCurrentSecond() - dateCycle_);
    stats_.ResetIptablesStats();
}
#endif

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
#ifndef UNITTEST_FORBID_FFRT
    cacheTimer_ = std::make_unique<FfrtTimer>();
    cacheTimer_->Start(cycleThreshold_, [this]() { CacheStats(); });
#endif
}

void NetStatsCached::ForceUpdateStats()
{
    isForce_ = true;
    std::function<void()> netCachedStats = [this] () {
        isExec_ = true;
        CacheStats();
        WriteStats();
        isForce_ = false;
        LoadIfaceNameIdentMaps();
        isExec_ = false;
    };
    if (!isExec_) {
        ffrt::submit(std::move(netCachedStats), {}, {}, ffrt::task_attr().name("NetCachedStats"));
    }
}

ffrt::task_handle NetStatsCached::ForceArchiveStats(uint32_t uid)
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
        DeleteUidStats(uid);
        DeleteUidSimStats(uid);
        DeleteUidStatsFlag(uid);
        DeleteUidSimSampleBundle(uid);
        if (GetUidSimSampleBundlesSize() == 0) {
            uidStatsFlagMap_.Clear();
        }
    };
    return ffrt::submit_h(std::move(netCachedStats), {}, {}, ffrt::task_attr().name("NetForceArchiveStats"));
}

void NetStatsCached::Reset() {}

void NetStatsCached::ForceCachedStats()
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    CacheUidSimStats();
    WriteUidSimStats();
}

void NetStatsCached::SetUidSimSampleBundle(uint32_t uid, const SampleBundleInfo &info)
{
    if (!info.Valid()) {
        NETMGR_LOG_W("SetUidSimSampleBundle invalid. info[%{public}u, %{public}s]", uid, info.ToString().c_str());
        return;
    }
    uidSimSampleBundleMap_.EnsureInsert(uid, info);
}

void NetStatsCached::DeleteUidSimSampleBundle(uint32_t uid)
{
    uidSimSampleBundleMap_.Erase(uid);
}

std::optional<SampleBundleInfo> NetStatsCached::GetUidSimSampleBundle(uint32_t uid)
{
    SampleBundleInfo info;
    if (uidSimSampleBundleMap_.Find(uid, info)) {
        return info;
    }
    return std::nullopt;
}

uint32_t NetStatsCached::GetUidSimSampleBundlesSize()
{
    return static_cast<uint32_t>(uidSimSampleBundleMap_.Size());
}

void NetStatsCached::SetUidStatsFlag(std::unordered_map<uint32_t, SampleBundleInfo> &sampleBundleMap)
{
    if (sampleBundleMap.empty()) {
        NETMGR_LOG_W("SetUidStatsFlag sampleBundleMap is empty");
        return;
    }
    bool isExistSim = false;
    bool isExistSim2 = false;
    IsExistInUidSimSampleBundleMap(isExistSim, isExistSim2);
    std::optional<SampleBundleInfo> earlySampleBundleOpt = GetEarlySampleBundleInfo();
    for (auto iter = sampleBundleMap.begin(); iter != sampleBundleMap.end(); ++iter) {
        if (!iter->second.Valid()) {
            NETMGR_LOG_W("SetUidStatsFlag sampleBundleInfo is invalid. [%{public}s]", iter->second.ToString().c_str());
            continue;
        }
        if (isDisplayTrafficAncoList) {
            if (CommonUtils::IsSim(iter->second.bundleName_) || CommonUtils::IsSimAnco(iter->second.bundleName_)) {
                uidStatsFlagMap_.EnsureInsert(iter->first, STATS_DATA_FLAG_SIM_BASIC);
                continue;
            } else if (CommonUtils::IsSim2(iter->second.bundleName_) ||
                CommonUtils::IsSim2Anco(iter->second.bundleName_)) {
                uidStatsFlagMap_.EnsureInsert(iter->first, STATS_DATA_FLAG_SIM2_BASIC);
                continue;
            }
        }
        if (CommonUtils::IsInstallSourceFromSim2(iter->second.installSource_)) {
            uidStatsFlagMap_.EnsureInsert(iter->first,
                                          isExistSim2 ? STATS_DATA_FLAG_SIM2 : STATS_DATA_FLAG_DEFAULT);
        } else if (CommonUtils::IsInstallSourceFromSim(iter->second.installSource_)) {
            uidStatsFlagMap_.EnsureInsert(iter->first, isExistSim ? STATS_DATA_FLAG_SIM : STATS_DATA_FLAG_DEFAULT);
        } else if (iter->second.installSource_ == INSTALL_SOURCE_DEFAULT) {
            if (!isExistSim && !isExistSim2) {
                uidStatsFlagMap_.EnsureInsert(iter->first, STATS_DATA_FLAG_DEFAULT);
                continue;
            }
            if (earlySampleBundleOpt.has_value() &&
                CommonUtils::IsSim2(earlySampleBundleOpt.value().bundleName_) && isExistSim2) {
                uidStatsFlagMap_.EnsureInsert(iter->first,
                    isDisplayTrafficAncoList ? STATS_DATA_FLAG_SIM2_BASIC : STATS_DATA_FLAG_SIM2);
            } else if (isExistSim) {
                uidStatsFlagMap_.EnsureInsert(iter->first,
                    isDisplayTrafficAncoList ? STATS_DATA_FLAG_SIM_BASIC : STATS_DATA_FLAG_SIM);
            } else {
                uidStatsFlagMap_.EnsureInsert(iter->first, STATS_DATA_FLAG_DEFAULT);
            }
        }
    }
}

void NetStatsCached::DeleteUidStatsFlag(uint32_t uid)
{
    uidStatsFlagMap_.Erase(uid);
}

void NetStatsCached::ClearUidStatsFlag()
{
    ForceCachedStats();
    uidStatsFlagMap_.Clear();
}

NetStatsDataFlag NetStatsCached::GetUidStatsFlag(uint32_t uid)
{
    NetStatsDataFlag flag = STATS_DATA_FLAG_DEFAULT;
    if (uidStatsFlagMap_.Find(uid, flag)) {
        return flag;
    }
    if (uidSimSampleBundleMap_.Size() < 1) {
        uidStatsFlagMap_.EnsureInsert(uid, flag);
        return flag;
    }
    bool isExistSim = false;
    uidSimSampleBundleMap_.Iterate([&isExistSim](const uint32_t &k, const SampleBundleInfo &v) {
        if (CommonUtils::IsSim(v.bundleName_)) {
            isExistSim = true;
        }
    });
    flag = isExistSim ? (isDisplayTrafficAncoList ? STATS_DATA_FLAG_SIM_BASIC : STATS_DATA_FLAG_SIM) :
        STATS_DATA_FLAG_DEFAULT;
    uidStatsFlagMap_.EnsureInsert(uid, flag);
    return flag;
}

std::optional<SampleBundleInfo> NetStatsCached::GetEarlySampleBundleInfo()
{
    std::map<uint32_t, SampleBundleInfo> tmp;
    uidSimSampleBundleMap_.Iterate([&tmp](uint32_t uid, const SampleBundleInfo &info) { tmp.emplace(uid, info); });
    auto earlySampleBundle = std::max_element(
        tmp.begin(), tmp.end(), [](std::pair<uint32_t, SampleBundleInfo> l, std::pair<uint32_t, SampleBundleInfo> r) {
            return l.second.installTime_ > r.second.installTime_;
        });
    if (earlySampleBundle != tmp.end()) {
        return earlySampleBundle->second;
    } else {
        NETMGR_LOG_W("SetUidStatsFlag earlySampleBundle is not exist.");
        return std::nullopt;
    }
}

void NetStatsCached::IsExistInUidSimSampleBundleMap(bool &isExistSim, bool &isExistSim2)
{
    isExistSim = false;
    isExistSim2 = false;
    uidSimSampleBundleMap_.Iterate([&isExistSim2, &isExistSim](uint32_t uid, const SampleBundleInfo &info) {
        if (CommonUtils::IsSim(info.bundleName_)) {
            isExistSim = true;
        }
        if (CommonUtils::IsSim2(info.bundleName_)) {
            isExistSim2 = true;
        }
    });
}

void NetStatsCached::DeleteUidStats(uint32_t uid)
{
    auto ret = NetsysController::GetInstance().DeleteStatsInfo(uid);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("DeleteUidStats statsInfo failed. ret is %{public}d", ret);
    }
    std::lock_guard<ffrt::mutex> lock(lock_);
    stats_.ResetUidStats(uid);
    lastUidStatsInfo_.erase(std::remove_if(lastUidStatsInfo_.begin(), lastUidStatsInfo_.end(),
                                           [uid](const auto &item) { return item.uid_ == uid; }),
                            lastUidStatsInfo_.end());
    uidPushStatsInfo_.erase(std::remove_if(uidPushStatsInfo_.begin(), uidPushStatsInfo_.end(),
                                           [uid](const auto &item) { return item.uid_ == uid; }),
                            uidPushStatsInfo_.end());
}

void NetStatsCached::DeleteUidSimStats(uint32_t uid)
{
    std::optional<SampleBundleInfo> sampleBundleInfoOpt = GetUidSimSampleBundle(uid);
    DeleteUidSimSampleBundle(uid);
    if (!sampleBundleInfoOpt.has_value()) {
        auto ret = NetsysController::GetInstance().DeleteSimStatsInfo(uid);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("DeleteUidSimStats SimStatsInfo Failed. ret is %{public}d", ret);
        }
        std::lock_guard<ffrt::mutex> lock(lock_);
        stats_.ResetUidSimStats(uid);
        lastUidSimStatsInfo_.erase(std::remove_if(lastUidSimStatsInfo_.begin(), lastUidSimStatsInfo_.end(),
                                                  [uid](const auto &item) { return item.uid_ == uid; }),
                                   lastUidSimStatsInfo_.end());
        return;
    }
    auto sampleBundleInfo = sampleBundleInfoOpt.value();
    if (!sampleBundleInfo.Valid()) {
        NETMGR_LOG_W("DeleteUidSimStats invalid info[%{public}s]", sampleBundleInfo.ToString().c_str());
        return;
    }
    if (CommonUtils::IsSim(sampleBundleInfo.bundleName_) ||
        CommonUtils::IsSim2(sampleBundleInfo.bundleName_)) {
        if (!isDisplayTrafficAncoList) {
            auto flag = CommonUtils::IsSim(sampleBundleInfo.bundleName_) ? STATS_DATA_FLAG_SIM : STATS_DATA_FLAG_SIM2;
            DeleteUidSimStatsWithFlag(uid, flag);
        } else {
            auto flagBasic = CommonUtils::IsSim(sampleBundleInfo.bundleName_) ?
                STATS_DATA_FLAG_SIM_BASIC : STATS_DATA_FLAG_SIM2_BASIC;
            auto flagHap = (flagBasic == STATS_DATA_FLAG_SIM_BASIC) ? STATS_DATA_FLAG_SIM : STATS_DATA_FLAG_SIM2;
            DeleteUidSimStatsWithFlag(uid, flagBasic);
            DeleteUidSimStatsWithFlag(uid, flagHap);
        }
    }
}

void NetStatsCached::DeleteUidSimStatsWithFlag(uint32_t uid, uint32_t flag)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    if (handler == nullptr || handler->UpdateSimDataFlag(flag, STATS_DATA_FLAG_UNINSTALLED) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("DeleteUidSimStats updateFlag failed. uid:[%{public}d], flag[%{public}u]", uid, flag);
    }
    std::lock_guard<ffrt::mutex> lock(lock_);
    lastUidSimStatsInfo_.erase(std::remove_if(lastUidSimStatsInfo_.begin(), lastUidSimStatsInfo_.end(),
                                              [flag](const auto &item) { return item.flag_ == flag; }),
                               lastUidSimStatsInfo_.end());
    std::vector<uint32_t> uidList{uid};
    uidStatsFlagMap_.Iterate([flag, &uidList](const uint32_t &k, const NetStatsDataFlag &v) {
        if (flag == v) {
            uidList.push_back(k);
        }
    });
    std::for_each(uidList.begin(), uidList.end(), [this](const uint32_t uid) {
        uidStatsFlagMap_.EnsureInsert(uid, STATS_DATA_FLAG_DEFAULT);
        auto ret = NetsysController::GetInstance().DeleteSimStatsInfo(uid);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("DeleteUidSimStats SimStatsInfo Failed. ret[%{public}d], uid[%{public}u]", ret, uid);
        }
    });
}

#ifdef SUPPORT_NETWORK_SHARE
void NetStatsCached::DeleteIptablesStats()
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    stats_.ResetIptablesStats();
    lastIptablesStatsInfo_.clear();
}
#endif

void NetStatsCached::GetKernelUidStats(std::vector<NetStatsInfo> &statsInfo)
{
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
}

void NetStatsCached::GetKernelUidSimStats(std::vector<NetStatsInfo> &statsInfo)
{
    std::vector<NetStatsInfo> SimInfos;
    NetsysController::GetInstance().GetAllSimStatsInfo(SimInfos);
    ifaceNameIdentMap_.Iterate([&SimInfos](const std::string &k, const std::string &v) {
        std::for_each(SimInfos.begin(), SimInfos.end(), [&k, &v](NetStatsInfo &item) {
            if (item.iface_ == k) {
                item.ident_ = v;
            }
        });
    });
    uidStatsFlagMap_.Iterate([&SimInfos](const uint32_t &k, const NetStatsDataFlag &v) {
        std::for_each(SimInfos.begin(), SimInfos.end(), [&k, &v](NetStatsInfo &item) {
            if (item.uid_ == k) {
                item.flag_ = v;
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
        if (tmp.flag_ <= STATS_DATA_FLAG_DEFAULT || tmp.flag_ >= STATS_DATA_FLAG_LIMIT) {
            tmp.flag_ = GetUidStatsFlag(tmp.uid_);
        }

        if (!isDisplayTrafficAncoList) {
            if (tmp.flag_ == STATS_DATA_FLAG_SIM2) {
                tmp.uid_ = SIM2_UID;
            } else if (tmp.flag_ == STATS_DATA_FLAG_SIM) {
                tmp.uid_ = Sim_UID;
            } else {
                return;
            }
        } else {
            if (tmp.flag_ == STATS_DATA_FLAG_SIM_BASIC) {
                tmp.uid_ = Sim_UID;
            } else if (tmp.flag_ == STATS_DATA_FLAG_SIM2_BASIC) {
                tmp.uid_ = SIM2_UID;
            } else if (tmp.flag_ != STATS_DATA_FLAG_SIM && tmp.flag_ != STATS_DATA_FLAG_SIM2) {
                return;
            }
        }
        statsInfo.push_back(std::move(tmp));
    });
}

#ifdef SUPPORT_NETWORK_SHARE
void NetStatsCached::GetIptablesStatsCached(std::vector<NetStatsInfo> &iptablesStatsInfo)
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    iptablesStatsInfo.insert(iptablesStatsInfo.end(),
        stats_.GetIptablesStatsInfo().begin(), stats_.GetIptablesStatsInfo().end());
    GetIptablesStatsIncrease(iptablesStatsInfo);
}

void NetStatsCached::CacheIptablesStats()
{
    std::string ifaceName;
    nmd::NetworkSharingTraffic traffic;

    int32_t ret = NetsysController::GetInstance().GetNetworkCellularSharingTraffic(traffic, ifaceName);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("GetTrafficBytes err, ret[%{public}d]", ret);
        return;
    }
    CacheIptablesStatsService(traffic, ifaceName);
}

void NetStatsCached::CacheIptablesStatsService(nmd::NetworkSharingTraffic &traffic, std::string &ifaceName)
{
    NetStatsInfo statsInfos;
    statsInfos.uid_ = IPTABLES_UID;
    statsInfos.iface_ = ifaceName;
    statsInfos.rxBytes_ = static_cast<uint64_t>(traffic.receive);
    statsInfos.txBytes_ = static_cast<uint64_t>(traffic.send);
    statsInfos.flag_ = STATS_DATA_FLAG_DEFAULT;
    statsInfos.rxPackets_ = statsInfos.rxBytes_ > 0 ? 1 : 0;
    statsInfos.txPackets_ = statsInfos.txBytes_ > 0 ? 1 : 0;
    std::vector<NetStatsInfo> statsInfosVec;
    statsInfosVec.push_back(std::move(statsInfos));

    ifaceNameIdentMap_.Iterate([&statsInfosVec](const std::string &k, const std::string &v) {
        std::for_each(statsInfosVec.begin(), statsInfosVec.end(), [&k, &v](NetStatsInfo &item) {
            if (item.iface_ == k) {
                item.ident_ = v;
            }
        });
    });

    std::for_each(statsInfosVec.begin(), statsInfosVec.end(), [this](NetStatsInfo &info) {
        if (info.iface_ == IFACE_LO) {
            return;
        }
        auto findRet = std::find_if(lastIptablesStatsInfo_.begin(), lastIptablesStatsInfo_.end(),
            [this, &info](const NetStatsInfo &lastInfo) {return info.Equals(lastInfo); });
        if (findRet == lastIptablesStatsInfo_.end()) {
            stats_.PushIptablesStats(info);
            return;
        }
        auto currentStats = info - *findRet;
        stats_.PushIptablesStats(currentStats);
    });
    NETMGR_LOG_D("CacheIptablesStatsService info success");
    lastIptablesStatsInfo_.swap(statsInfosVec);
}

void NetStatsCached::GetIptablesStatsIncrease(std::vector<NetStatsInfo> &infosVec)
{
    std::string ifaceName;
    nmd::NetworkSharingTraffic traffic;
    int32_t ret = NetsysController::GetInstance().GetNetworkCellularSharingTraffic(traffic, ifaceName);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("GetTrafficBytes err, ret[%{public}d]", ret);
        return;
    }
    NetStatsInfo statsInfos;
    statsInfos.uid_ = IPTABLES_UID;
    statsInfos.iface_ = ifaceName;
    statsInfos.rxBytes_ = static_cast<uint64_t>(traffic.receive);
    statsInfos.txBytes_ = static_cast<uint64_t>(traffic.send);
    statsInfos.flag_ = STATS_DATA_FLAG_DEFAULT;
    statsInfos.rxPackets_ = statsInfos.rxBytes_ > 0 ? 1 : 0;
    statsInfos.txPackets_ = statsInfos.txBytes_ > 0 ? 1 : 0;
    statsInfos.date_ = CommonUtils::GetCurrentSecond();

    std::vector<NetStatsInfo> statsInfosVec;
    statsInfosVec.push_back(std::move(statsInfos));

    ifaceNameIdentMap_.Iterate([&statsInfosVec](const std::string &k, const std::string &v) {
        std::for_each(statsInfosVec.begin(), statsInfosVec.end(), [&k, &v](NetStatsInfo &item) {
            if (item.iface_ == k) {
                item.ident_ = v;
            }
        });
    });
    std::vector<NetStatsInfo> tmpInfosVec;
    if (!lastIptablesStatsInfo_.empty()) {
        std::for_each(statsInfosVec.begin(), statsInfosVec.end(), [this, &tmpInfosVec](NetStatsInfo &info) {
            if (info.iface_ == IFACE_LO) {
                return;
            }
            auto findRet = std::find_if(lastIptablesStatsInfo_.begin(), lastIptablesStatsInfo_.end(),
                [this, &info](const NetStatsInfo &lastInfo) {return info.Equals(lastInfo); });
            if (findRet == lastIptablesStatsInfo_.end()) {
                tmpInfosVec.push_back(std::move(info));
            } else {
                tmpInfosVec.push_back(info - *findRet);
            }
        });
    } else {
        tmpInfosVec = statsInfosVec;
    }
    infosVec.insert(infosVec.end(), tmpInfosVec.begin(), tmpInfosVec.end());
}
#endif

void NetStatsCached::SaveSharingTraffic(const NetStatsInfo &infos)
{
    NETMGR_LOG_I("SaveSharingTraffic enter");
#ifdef SUPPORT_NETWORK_SHARE
    std::lock_guard<ffrt::mutex> lock(lock_);
    if (infos.iface_ == "" || infos.iface_.find(CELLULAR_IFACE_NAME) == std::string::npos) {
        NETMGR_LOG_D("ifaceName not cellular [%{public}s]", infos.iface_.c_str());
        return;
    }
    nmd::NetworkSharingTraffic traffic;
    traffic.receive = infos.rxBytes_;
    traffic.send = infos.txBytes_;
    std::string ifaceName = infos.iface_;
    CacheIptablesStatsService(traffic, ifaceName);
    WriteIptablesStats();
    lastIptablesStatsInfo_.clear();
#endif
}
} // namespace NetManagerStandard
} // namespace OHOS
