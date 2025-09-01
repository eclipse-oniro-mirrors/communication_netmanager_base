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
#include "net_manager_constants.h"
#include "wrapper.rs.h"

namespace OHOS {
namespace NetManagerAni {

rust::String GetErrorCodeAndMessage(int32_t &errorCode)
{
    NetManagerStandard::NetBaseErrorCodeConvertor convertor;
    return rust::string(convertor.ConvertErrorCode(errorCode));
}

NetManagerStandard::NetStatsClient &GetNetStatsClient(int32_t &nouse)
{
    return NetManagerStandard::NetStatsClient::GetInstance();
}

StatisEventCallback::StatisEventCallback(rust::Box<StatisticsCallback> &&callback) : callback_(std::move(callback)) {}

int32_t StatisEventCallback::NetIfaceStatsChanged(const std::string &iface)
{
    NetStatsChangeInfo info{
        .iface = rust::string(iface),
    };
    return callback_->net_iface_stats_changed(info);
}

int32_t StatisEventCallback::NetUidStatsChanged(const std::string &iface, uint32_t uid)
{
    NetStatsChangeInfo info{
        .iface = rust::string(iface),
        .uid = uid,
    };
    return callback_->net_uid_stats_changed(info);
}

std::unique_ptr<StatisCallbackUnregister> RegisterStatisCallback(rust::Box<StatisticsCallback> callback, int32_t &ret)
{
    auto eventCallback = sptr<StatisEventCallback>::MakeSptr(std::move(callback));
    ret = NetManagerStandard::NetStatsClient::GetInstance().RegisterNetStatsCallback(eventCallback);
    return std::make_unique<StatisCallbackUnregister>(eventCallback);
}

StatisCallbackUnregister::StatisCallbackUnregister(sptr<StatisEventCallback> eventCallback)
    : eventCallback_(eventCallback)
{
}

int32_t StatisCallbackUnregister::Unregister() const
{
    auto ret = NetManagerStandard::NetStatsClient::GetInstance().UnregisterNetStatsCallback(eventCallback_);
    return ret;
}

NetStatsInfoInner GetTrafficStatsByIface(IfaceInfo &info, int32_t &ret)
{
    NetStatsInfo netStatsInfo;
    ret = NetManagerStandard::NetStatsClient::GetInstance().GetIfaceStatsDetail(
        std::string(info.iface), info.start_time, info.end_time, netStatsInfo);
    if (ret != 0) {
        return NetStatsInfoInner{};
    }
    return NetStatsInfoInner{.rx_bytes = netStatsInfo.rxBytes_,
                             .tx_bytes = netStatsInfo.txBytes_,
                             .rx_packets = netStatsInfo.rxPackets_,
                             .tx_packets = netStatsInfo.rxPackets_};
}

NetStatsInfoInner GetTrafficStatsByUid(UidInfo &info, int32_t &ret)
{
    NetStatsInfo netStatsInfo;
    ret = NetManagerStandard::NetStatsClient::GetInstance().GetUidStatsDetail(std::string(info.iface_info.iface),
                                                                              info.uid, info.iface_info.start_time,
                                                                              info.iface_info.end_time, netStatsInfo);
    if (ret != 0) {
        return NetStatsInfoInner{};
    }
    return NetStatsInfoInner{.rx_bytes = netStatsInfo.rxBytes_,
                             .tx_bytes = netStatsInfo.txBytes_,
                             .rx_packets = netStatsInfo.rxPackets_,
                             .tx_packets = netStatsInfo.rxPackets_};
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
    if (ret != 0) {
        return ret;
    }
    for (auto &item : map_infos) {
        netStatsInfos.push_back(OHOS::NetManagerAni::AniUidNetStatsInfoPair{
            .uid = static_cast<int32_t>(item.first),
            .net_stats_info = NetStatsInfoInner{.rx_bytes = item.second.rxBytes_,
                                                .tx_bytes = item.second.txBytes_,
                                                .rx_packets = item.second.rxPackets_,
                                                .tx_packets = item.second.rxPackets_}});
    }

    return 0;
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
    if (ret != 0) {
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
    return 0;
}
} // namespace NetManagerAni
} // namespace OHOS
