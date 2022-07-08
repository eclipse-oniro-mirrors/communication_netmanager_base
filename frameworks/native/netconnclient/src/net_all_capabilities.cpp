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

#include "net_all_capabilities.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetAllCapabilities::CapsIsValid() const
{
    for (auto it = netCaps_.begin(); it != netCaps_.end(); it++) {
        if ((*it < NET_CAPABILITY_MMS) || (*it >= NET_CAPABILITY_INTERNAL_DEFAULT)) {
            return false;
        }
    }
    for (auto it = bearerTypes_.begin(); it != bearerTypes_.end(); it++) {
        if ((*it < BEARER_CELLULAR) || (*it >= BEARER_DEFAULT)) {
            return false;
        }
    }
    return true;
}

bool NetAllCapabilities::CapsIsNull() const
{
    if ((linkUpBandwidthKbps_ == 0) && (linkDownBandwidthKbps_ == 0) && (netCaps_.size() == 0) &&
        (bearerTypes_.size() == 0)) {
        return true;
    }
    return false;
}

bool NetAllCapabilities::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(linkUpBandwidthKbps_) || !parcel.WriteUint32(linkDownBandwidthKbps_)) {
        return false;
    }
    if (!parcel.WriteUint32(netCaps_.size())) {
        return false;
    }
    for (auto it = netCaps_.begin(); it != netCaps_.end(); it++) {
        if (!parcel.WriteUint32(static_cast<uint32_t>(*it))) {
            return false;
        }
    }
    if (!parcel.WriteUint32(bearerTypes_.size())) {
        return false;
    }
    for (auto it = bearerTypes_.begin(); it != bearerTypes_.end(); it++) {
        if (!parcel.WriteUint32(static_cast<uint32_t>(*it))) {
            return false;
        }
    }
    return true;
}

bool NetAllCapabilities::Unmarshalling(Parcel &parcel)
{
    if (!parcel.ReadUint32(linkUpBandwidthKbps_)) {
        return false;
    }
    if (!parcel.ReadUint32(linkDownBandwidthKbps_)) {
        return false;
    }
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        return false;
    }
    uint32_t cap = 0;
    for (uint32_t i = 0; i < size; i++) {
        if (!parcel.ReadUint32(cap)) {
            return false;
        }
        if (cap >= NET_CAPABILITY_INTERNAL_DEFAULT) {
            continue;
        }
        netCaps_.insert(static_cast<NetCap>(cap));
    }
    if (!parcel.ReadUint32(size)) {
        return false;
    }
    uint32_t type = 0;
    for (uint32_t i = 0; i < size; i++) {
        if (!parcel.ReadUint32(type)) {
            return false;
        }
        if (type >= BEARER_DEFAULT) {
            continue;
        }
        bearerTypes_.insert(static_cast<NetBearType>(type));
    }
    return true;
}

std::string NetAllCapabilities::ToString(const std::string &tab) const
{
    std::string str;
    str.append(tab);
    str.append("[NetAllCapabilities]");

    str.append(tab);
    str.append("linkUpBandwidthKbps_ = ");
    str.append(std::to_string(linkUpBandwidthKbps_));

    str.append(tab);
    str.append("linkDownBandwidthKbps_ = ");
    str.append(std::to_string(linkDownBandwidthKbps_));

    str.append(tab);
    ToStrNetCaps(netCaps_, str);

    str.append(tab);
    ToStrNetBearTypes(bearerTypes_, str);

    return str;
}

void NetAllCapabilities::ToStrNetCaps(const std::set<NetCap> &netCaps, std::string &str) const
{
    str.append("netCaps_ =");
    for (auto netCap : netCaps) {
        str.append(" ");
        switch (netCap) {
            case NET_CAPABILITY_MMS:
                str.append("NET_CAPABILITY_MMS");
                break;
            case NET_CAPABILITY_NOT_METERED:
                str.append("NET_CAPABILITY_NOT_METERED");
                break;
            case NET_CAPABILITY_INTERNET:
                str.append("NET_CAPABILITY_INTERNET");
                break;
            case NET_CAPABILITY_NOT_VPN:
                str.append("NET_CAPABILITY_NOT_VPN");
                break;
            case NET_CAPABILITY_VALIDATED:
                str.append("NET_CAPABILITY_VALIDATED");
                break;
            case NET_CAPABILITY_CAPTIVE_PORTAL:
                str.append("NET_CAPABILITY_CAPTIVE_PORTAL");
                break;
            default:
                str.append("unknown NetCap");
                break;
        }
    }
}

void NetAllCapabilities::ToStrNetBearTypes(const std::set<NetBearType> &bearerTypes, std::string &str) const
{
    str.append("NetBearType =");
    for (auto bearerType : bearerTypes) {
        str.append(" ");
        switch (bearerType) {
            case BEARER_CELLULAR:
                str.append("BEARER_CELLULAR");
                break;
            case BEARER_WIFI:
                str.append("BEARER_WIFI");
                break;
            case BEARER_ETHERNET:
                str.append("BEARER_ETHERNET");
                break;
            case BEARER_VPN:
                str.append("BEARER_VPN");
                break;
            case BEARER_WIFI_AWARE:
                str.append("BEARER_WIFI_AWARE");
                break;
            default:
                str.append("unknown NetBearType");
                break;
        }
    }
}
} // namespace NetManagerStandard
} // namespace OHOS