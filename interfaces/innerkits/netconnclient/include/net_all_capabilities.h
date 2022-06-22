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

#ifndef NET_ALL_CAPABILITIES_H
#define NET_ALL_CAPABILITIES_H

#include <set>

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
enum NetCap {
    NET_CAPABILITY_MMS,
    NET_CAPABILITY_SUPL,
    NET_CAPABILITY_DUN,
    NET_CAPABILITY_FOTA,
    NET_CAPABILITY_IMS,
    NET_CAPABILITY_CBS,
    NET_CAPABILITY_WIFI_P2P,
    NET_CAPABILITY_IA,
    NET_CAPABILITY_RCS,
    NET_CAPABILITY_XCAP,
    NET_CAPABILITY_EIMS,
    NET_CAPABILITY_NOT_METERED,
    NET_CAPABILITY_INTERNET,
    NET_CAPABILITY_NOT_RESTRICTED,
    NET_CAPABILITY_TRUSTED,
    NET_CAPABILITY_NOT_VPN,
    NET_CAPABILITY_VALIDATED,
    NET_CAPABILITY_CAPTIVE_PORTAL,
    NET_CAPABILITY_NOT_ROAMING,
    NET_CAPABILITY_FOREGROUND,
    NET_CAPABILITY_NOT_CONGESTED,
    NET_CAPABILITY_NOT_SUSPENDED,
    NET_CAPABILITY_OEM_PAID,
    NET_CAPABILITY_MCX,
    NET_CAPABILITY_PARTIAL_CONNECTIVITY,
    NET_CAPABILITY_INTERNAL_DEFAULT
};

enum NetBearType {
    BEARER_CELLULAR,
    BEARER_WIFI,
    BEARER_BLUETOOTH,
    BEARER_ETHERNET,
    BEARER_VPN,
    BEARER_WIFI_AWARE,
    BEARER_LOWPAN,
    BEARER_DEFAULT
};

struct NetAllCapabilities : public Parcelable {
    uint32_t linkUpBandwidthKbps_ = 0;
    uint32_t linkDownBandwidthKbps_ = 0;
    std::set<NetCap> netCaps_;
    std::set<NetBearType> bearerTypes_;

    bool operator==(const NetAllCapabilities &other) const;
    bool operator!=(const NetAllCapabilities &other) const;
    
    bool CapsIsValid() const;
    bool CapsIsNull() const;
    virtual bool Marshalling(Parcel &parcel) const override;
    bool Unmarshalling(Parcel &parcel);
    std::string ToString(const std::string &tab) const;

private:
    void ToStrNetCaps(const std::set<NetCap> &netCaps, std::string &str) const;
    void ToStrNetBearTypes(const std::set<NetBearType> &bearerTypes, std::string &str) const;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_ALL_CAPABILITIES_H
