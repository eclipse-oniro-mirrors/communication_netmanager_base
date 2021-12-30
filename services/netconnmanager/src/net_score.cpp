/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "net_score.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t WIFI_SIGNAL_BAR   = 5;
constexpr int32_t WIFI_BASE_SCORE   = static_cast<int32_t>(NetTypeScoreValue::WIFI_VALUE) - NET_TYPE_SCORE_INTERVAL +
    NET_TYPE_SCORE_INTERVAL / WIFI_SIGNAL_BAR;
constexpr int32_t WIFI_MIN_RSSI     = -100;
constexpr int32_t WIFI_MAX_RSSI     = -55;

NetScore::NetScore() {}

NetScore::~NetScore() {}

int32_t NetScore::CalculateScoreForWifi(sptr<NetSupplier> &supplier)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("the input supplier is nullptr");
        return 0;
    }
    int32_t rssi = static_cast<int32_t>(supplier->GetStrength());
    int32_t signalScore = GetWifiSignalBar(rssi, WIFI_SIGNAL_BAR);
    int32_t wifiScore = WIFI_BASE_SCORE + signalScore * (NET_TYPE_SCORE_INTERVAL / WIFI_SIGNAL_BAR);
    if (wifiScore > static_cast<int32_t>(NetTypeScoreValue::WIFI_VALUE)) {
        wifiScore = static_cast<int32_t>(NetTypeScoreValue::WIFI_VALUE);
    }
    return wifiScore;
}

int32_t NetScore::GetWifiSignalBar(int32_t rssi, int32_t signalBars)
{
    if (rssi <= WIFI_MIN_RSSI) {
        return 0;
    } else if (rssi >= WIFI_MAX_RSSI) {
        return signalBars - 1;
    } else {
        int32_t inputRange = WIFI_MAX_RSSI - WIFI_MIN_RSSI;
        int32_t outputRange = signalBars - 1;
        return static_cast<int32_t>((rssi - WIFI_MIN_RSSI) * outputRange / inputRange);
    }
}

bool NetScore::GetServiceScore(sptr<NetSupplier> &supplier)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("the input supplier is nullptr");
        return false;
    }

    NetworkType netType = supplier->GetNetSupplierType();
    int32_t netScore = 0;
    if (netType == NET_TYPE_WIFI) {
        netScore = CalculateScoreForWifi(supplier);
    } else {
        NetTypeScore::iterator iter = netTypeScore_.find(netType);
        if (iter == netTypeScore_.end()) {
            NETMGR_LOG_E("can not find net type for this net service");
            return false;
        }
        NETMGR_LOG_I("the net type[%{public}d], score[%{public}d]",
            static_cast<int32_t>(iter->first), static_cast<int32_t>(iter->second));
        netScore = static_cast<int32_t>(iter->second);
    }

    supplier->SetNetScore(netScore);
    if (!(supplier->IfNetValid())) {
        netScore -= NET_VALID_SCORE;
    }
    supplier->SetRealScore(netScore);
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
