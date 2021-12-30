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

#ifndef NET_SCORE_H
#define NET_SCORE_H

#include <vector>
#include <unordered_map>
#include <singleton.h>
#include "net_supplier.h"

namespace OHOS {
namespace NetManagerStandard {
using NetTypeScore = std::unordered_map<NetworkType, int32_t>;
constexpr int32_t NET_TYPE_SCORE_INTERVAL = 10;
constexpr int32_t NET_VALID_SCORE = 4 * NET_TYPE_SCORE_INTERVAL;
enum class NetTypeScoreValue : int32_t {
    USB_VALUE               = 4 * NET_TYPE_SCORE_INTERVAL,
    BLUETOOTH_VALUE         = 5 * NET_TYPE_SCORE_INTERVAL,
    CELLULAR_VALUE          = 6 * NET_TYPE_SCORE_INTERVAL,
    WIFI_VALUE              = 7 * NET_TYPE_SCORE_INTERVAL,
    ETHERNET_VALUE          = 8 * NET_TYPE_SCORE_INTERVAL,
    VPN_VALUE               = 9 * NET_TYPE_SCORE_INTERVAL
};

class NetScore {
public:
    NetScore();
    ~NetScore();
    bool GetServiceScore(sptr<NetSupplier> &supplier);

private:
    int32_t CalculateScoreForWifi(sptr<NetSupplier> &supplier);
    int32_t GetWifiSignalBar(int32_t rssi, int32_t signalBars);
private:
    NetTypeScore netTypeScore_ = {
        {NET_TYPE_CELLULAR, static_cast<int32_t>(NetTypeScoreValue::CELLULAR_VALUE)},
        {NET_TYPE_ETHERNET, static_cast<int32_t>(NetTypeScoreValue::ETHERNET_VALUE)}};
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_SCORE_H