/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef NETMANAGER_BASE_ANI_BRIDGE_H
#define NETMANAGER_BASE_ANI_BRIDGE_H

#include <cstdint>
#include <optional>
#include <vector>

enum NetCap {
    NET_CAPABILITY_MMS = 0,

    NET_CAPABILITY_NOT_METERED = 11,

    NET_CAPABILITY_INTERNET = 12,

    NET_CAPABILITY_NOT_VPN = 15,

    NET_CAPABILITY_VALIDATED = 16,
    NET_CAPABILITY_PORTAL = 17,

    NET_CAPABILITY_CHECKING_CONNECTIVITY = 31
};

enum NetBearType {
    BEARER_CELLULAR = 0,

    BEARER_WIFI = 1,

    BEARER_BLUETOOTH = 2,

    BEARER_ETHERNET = 3,

    BEARER_VPN = 4,
};

struct NetCapabilities {
    int64_t linkUpBandwidthKbps;

    int64_t linkDownBandwidthKbps;

    std::optional<std::vector<NetCap>> networkCap;

    std::vector<NetBearType> bearerTypes;
};

#endif /* NETMANAGER_BASE_ANI_BRIDGE_H */