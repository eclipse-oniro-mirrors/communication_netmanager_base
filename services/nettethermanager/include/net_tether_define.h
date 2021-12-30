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

#ifndef NET_TETHER_DEFINE_H
#define NET_TETHER_DEFINE_H

#include <string>
#include <list>
#include <functional>

#include "net_tether_constants.h"

namespace OHOS {
namespace NetManagerStandard {
struct RequestNetworkCallback {
    std::function<void(int32_t netId)> NetLost;
};

struct NetdResponseCallback {
    std::function<void(const std::string& iface)> NetdResponseInterfaceAdd;
    std::function<void(const std::string& iface)> NetdResponseInterfaceRemoved;
};

const std::string NETTETHERWORKTYPE_CONF_STR[] = {
    "1127,4352,4353",
};

const std::string DEFAULT_NEXT_HOP = "0.0.0.0";

enum TETHER_ERROR_CODE {
    TETHER_ERROR_NO_ERROR = 0,
    TETHER_ERROR_UNKNOWN_IFACE = 1,
    TETHER_ERROR_SERVICE_UNAVAIL = 2,
    TETHER_ERROR_UNSUPPORTED = 3,
    TETHER_ERROR_UNAVAIL_IFACE = 4,
    TETHER_ERROR_INTERNAL_ERROR = 5,
    TETHER_ERROR_TETHER_IFACE_ERROR = 6,
    TETHER_ERROR_UNTETHER_IFACE_ERROR = 7,
    TETHER_ERROR_ENABLE_FORWARDING_ERROR = 8,
    TETHER_ERROR_DISABLE_FORWARDING_ERROR = 9,
    TETHER_ERROR_IFACE_CFG_ERROR = 10,
    TETHER_ERROR_PROVISIONING_FAILED = 11,
    TETHER_ERROR_DHCPSERVER_ERROR = 12,
    TETHER_ERROR_ENTITLEMENT_UNKNOWN = 13,
    TETHER_ERROR_NO_CHANGE_TETHERING_PERMISSION = 14,
    TETHER_ERROR_NO_ACCESS_TETHERING_PERMISSION = 15,
    TETHER_ERROR_UNKNOWN_TYPE = 16,
    TETHER_ERROR_INTERFACE_ADD_ADDRESS = 17
};

enum IFACE_STATE {
    STATE_UNAVAILABLE = 0,
    STATE_AVAILABLE = 1,
    STATE_TETHERED = 2
};

const std::string BLUETOOTH_IFACE_ADDR = "192.168.44.1";
const std::string DEFAULT_IFACE_ADDR = "192.168.32.1";
constexpr uint32_t BULETOOTH_PREFIX_LEN = 24;
constexpr int32_t TETHER_PREFIX_LEN = 24;
constexpr int32_t DHCP_TIMEOUT = 60;
const std::string TETHER_AP_IFACE = "wlan1";
const std::string AP_EVENT = "usual.event.wifi.HOTSPOT_STATE";
constexpr uint32_t WIFI_SA_ID = 1127;

struct IfaceMgrCallback {
    std::function<void(const std::string& iface, int32_t state)> OnIfaceStateChange;
    std::function<void(TetheringType type, bool enable)> OnRequestTethering;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif // NET_TETHER_DEFINE_H
