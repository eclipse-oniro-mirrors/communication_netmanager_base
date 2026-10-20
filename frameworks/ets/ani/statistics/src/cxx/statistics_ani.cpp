/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "statistics_ani.h"
#include "errorcode_convertor.h"
#include "ipc_skeleton.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

sptr<StatisEventCallbackObserverAni> g_statisEventCallbackObserverAni =
    sptr<StatisEventCallbackObserverAni>(new (std::nothrow) StatisEventCallbackObserverAni());

std::atomic_bool g_isStatisObserverRegistered = false;

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::string(convertor.ConvertErrorCode(errorCode));
}

NetManagerStandard::NetStatsClient &GetNetStatsClient(int32_t &nouse)
{
    return NetManagerStandard::NetStatsClient::GetInstance();
}

int32_t StatisEventCallbackObserverAni::NetIfaceStatsChanged(const std::string &iface)
{
    NetStatsChangeInfo info{
        .iface = rust::string(iface),
    };
    execute_net_iface_stats_changed(info);
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t StatisEventCallbackObserverAni::NetUidStatsChanged(const std::string &iface, uint32_t uid)
{
    NetStatsChangeInfo info{
        .iface = rust::string(iface),
        .uid = uid,
    };
    execute_net_uid_stats_changed(info);
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t RegisterNetStatisObserver()
{
    if (g_isStatisObserverRegistered) {
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    if (g_statisEventCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }

    int32_t ret =
        NetManagerStandard::NetStatsClient::GetInstance().RegisterNetStatsCallback(g_statisEventCallbackObserverAni);
    if (ret == NetManagerStandard::NETMANAGER_SUCCESS) {
        g_isStatisObserverRegistered = true;
    }
    return ret;
}

int32_t UnRegisterNetStatisObserver()
{
    if (g_statisEventCallbackObserverAni == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_PARAMETER_ERROR;
    }
    auto ret =
        NetManagerStandard::NetStatsClient::GetInstance().UnregisterNetStatsCallback(g_statisEventCallbackObserverAni);
    if (ret == NetManagerStandard::NETMANAGER_SUCCESS) {
        g_isStatisObserverRegistered = false;
    }
    return ret;
}

NetStatsInfoInner GetTrafficStatsByIface(IfaceInfo &info, int32_t &ret)
{
    NetStatsInfo netStatsInfo;
    ret = NetManagerStandard::NetStatsClient::GetInstance().GetIfaceStatsDetail(
        std::string(info.iface), info.start_time, info.end_time, netStatsInfo);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return NetStatsInfoInner{};
    }
    return NetStatsInfoInner{.rx_bytes = netStatsInfo.rxBytes_,
                             .tx_bytes = netStatsInfo.txBytes_,
                             .rx_packets = netStatsInfo.rxPackets_,
                             .tx_packets = netStatsInfo.txPackets_};
}

NetStatsInfoInner GetTrafficStatsByUid(UidInfo &info, int32_t &ret)
{
    NetStatsInfo netStatsInfo;
    ret = NetManagerStandard::NetStatsClient::GetInstance().GetUidStatsDetail(std::string(info.iface_info.iface),
                                                                              info.uid, info.iface_info.start_time,
                                                                              info.iface_info.end_time, netStatsInfo);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return NetStatsInfoInner{};
    }
    return NetStatsInfoInner{.rx_bytes = netStatsInfo.rxBytes_,
                             .tx_bytes = netStatsInfo.txBytes_,
                             .rx_packets = netStatsInfo.rxPackets_,
                             .tx_packets = netStatsInfo.txPackets_};
}

int32_t GetTrafficStatsByNetworkVec(AniNetworkInfo &networkInfo, rust::Vec<AniUidNetStatsInfoPair> &netStatsInfos)
{
    std::unordered_map<uint32_t, NetManagerStandard::NetStatsInfo> map_infos;
    sptr<NetManagerStandard::NetStatsNetwork> networkPtr = new NetManagerStandard::NetStatsNetwork();
    networkPtr->type_ = static_cast<uint32_t>(networkInfo.type_);
    networkPtr->startTime_ = static_cast<uint64_t>(networkInfo.start_time);
    networkPtr->endTime_ = static_cast<uint64_t>(networkInfo.end_time);
    networkPtr->simId_ = static_cast<uint32_t>(networkInfo.sim_id);
    int32_t ret = DelayedSingleton<NetManagerStandard::NetStatsClient>::GetInstance()->GetTrafficStatsByNetwork(
        map_infos, networkPtr);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ret;
    }
    for (auto &item : map_infos) {
        netStatsInfos.push_back(OHOS::NetManagerAni::AniUidNetStatsInfoPair{
            .uid = static_cast<int32_t>(item.first),
            .net_stats_info = NetStatsInfoInner{.rx_bytes = item.second.rxBytes_,
                                                .tx_bytes = item.second.txBytes_,
                                                 .rx_packets = item.second.rxPackets_,
                                                .tx_packets = item.second.txPackets_}});
    }

    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t GetTrafficStatsByUidNetworkVec(rust::Vec<AniNetStatsInfoSequenceItem> &netStatsInfosSequence, uint32_t uid,
                                       AniNetworkInfo &networkInfo)
{
    std::vector<NetManagerStandard::NetStatsInfoSequence> netStatsInfosSequenceVec;
    sptr<NetManagerStandard::NetStatsNetwork> networkPtr = new NetManagerStandard::NetStatsNetwork();
    networkPtr->type_ = static_cast<uint32_t>(networkInfo.type_);
    networkPtr->startTime_ = static_cast<uint64_t>(networkInfo.start_time);
    networkPtr->endTime_ = static_cast<uint64_t>(networkInfo.end_time);
    networkPtr->simId_ = static_cast<uint32_t>(networkInfo.sim_id);
    int32_t ret = DelayedSingleton<NetManagerStandard::NetStatsClient>::GetInstance()->GetTrafficStatsByUidNetwork(
        netStatsInfosSequenceVec, uid, networkPtr);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ret;
    }

    for (auto &item : netStatsInfosSequenceVec) {
        netStatsInfosSequence.push_back(
            AniNetStatsInfoSequenceItem{.start_time = static_cast<int64_t>(item.startTime_),
                                        .end_time = static_cast<int64_t>(item.endTime_),
                                        .info = NetStatsInfoInner{.rx_bytes = item.info_.rxBytes_,
                                                                  .tx_bytes = item.info_.txBytes_,
                                                                  .rx_packets = item.info_.rxPackets_,
                                                                  .tx_packets = item.info_.txPackets_}});
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t UpdateIfacesStats(const std::string &iface, uint32_t start, uint32_t end, const NetStatsInfoInner &stats)
{
    NetManagerStandard::NetStatsInfo info;
    info.rxBytes_ = stats.rx_bytes;
    info.txBytes_ = stats.tx_bytes;
    info.rxPackets_ = stats.rx_packets;
    info.txPackets_ = stats.tx_packets;
    return NetManagerStandard::NetStatsClient::GetInstance().UpdateIfacesStats(iface, start, end, info);
}

int32_t SetCalibrationTraffic(uint32_t simId, int64_t remainingData, int64_t totalData, bool hasTotalData)
{
    uint64_t totalMonthlyData = UINT64_MAX;
    if (hasTotalData) {
        if (totalData < 0) {
            return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        }
        totalMonthlyData = static_cast<uint64_t>(totalData);
        if (totalMonthlyData == UINT64_MAX) {
            return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
        }
    }
    return NetManagerStandard::NetStatsClient::GetInstance().SetCalibrationTraffic(
        simId, remainingData, totalMonthlyData);
}

int32_t GetSelfTrafficStats(const AniNetworkInfo &networkInfo, bool hasSimId, NetStatsInfoInner &result)
{
    if (static_cast<int32_t>(networkInfo.type_) < 0 || networkInfo.start_time < 0 ||
        networkInfo.end_time < 0 || (hasSimId && networkInfo.sim_id < 0)) {
        return NetManagerStandard::NETMANAGER_ERR_INVALID_PARAMETER;
    }
    sptr<NetManagerStandard::NetStatsNetwork> network = new (std::nothrow) NetManagerStandard::NetStatsNetwork();
    if (network == nullptr) {
        return NetManagerStandard::NETMANAGER_ERR_INTERNAL;
    }
    network->type_ = static_cast<uint32_t>(networkInfo.type_);
    network->startTime_ = static_cast<uint64_t>(networkInfo.start_time);
    network->endTime_ = static_cast<uint64_t>(networkInfo.end_time);
    network->simId_ = hasSimId ? static_cast<uint32_t>(networkInfo.sim_id) : UINT32_MAX;
    std::vector<NetManagerStandard::NetStatsInfoSequence> sequence;
    int32_t ret = NetManagerStandard::NetStatsClient::GetInstance().GetTrafficStatsByUidNetwork(
        sequence, static_cast<uint32_t>(IPCSkeleton::GetCallingUid()), network);
    if (ret != NetManagerStandard::NETMANAGER_SUCCESS) {
        return ret;
    }
    result.rx_bytes = 0;
    result.tx_bytes = 0;
    result.rx_packets = 0;
    result.tx_packets = 0;
    for (const auto &item : sequence) {
        result.rx_bytes += item.info_.rxBytes_;
        result.tx_bytes += item.info_.txBytes_;
        result.rx_packets += item.info_.rxPackets_;
        result.tx_packets += item.info_.txPackets_;
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}
} // namespace NetManagerAni
} // namespace OHOS
