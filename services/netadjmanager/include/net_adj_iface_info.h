/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

//
// Created by root on 22-11-22.
//

#ifndef COMMUNICATION_NET_ADJ_IFACE_INFO_H
#define COMMUNICATION_NET_ADJ_IFACE_INFO_H

#include <list>

#include "parcel.h"
#include "inet_addr.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
struct NetAdjIfaceInfo : public Parcelable {
    enum {
        // Wi-Fi Station and AP.
        TYPE_WIFI = 0,
        // Wi-Fi direct.
        TYPE_WIFI_P2P = 1,
        // Bluetooth.
        TYPE_BLUETOOTH = 2,
        // Bluetooth low energy.
        TYPE_BLE = 3,
        // USB.
        TYPE_USB = 4,
        // Ethernet.
        TYPE_ETHERNET = 5,
        // Spark link.
        TYPE_SPARK_LINK = 6,
        // Remote connection.
        TYPE_REMOTE = 7,
    };

    uint32_t type_;
    std::string ifaceName_;
    std::list<INetAddr> netAddrList_;
    std::list<Route> routeList_;
    bool Marshalling(Parcel &parcel) const override;
    static sptr<NetAdjIfaceInfo> Unmarshalling(Parcel &parcel);
    bool operator==(const NetAdjIfaceInfo &rhs) const;
    bool operator!=(const NetAdjIfaceInfo &rhs) const;
};
}
}

#endif // COMMUNICATION_NET_ADJ_IFACE_INFO_H
