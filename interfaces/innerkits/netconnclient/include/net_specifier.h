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

#ifndef NET_SPECIFIER_H
#define NET_SPECIFIER_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
typedef enum {
    NET_CAPABILITIES_NONE,
    NET_CAPABILITIES_INTERNET = 1 << 0,
    NET_CAPABILITIES_MMS = 1 << 1,
    NET_CAPABILITIES_VPN = 1 << 2,
} NetCapabilities;
const uint64_t NET_CAPABILITIES_MAX = (NET_CAPABILITIES_INTERNET | NET_CAPABILITIES_MMS);

typedef enum {
    NET_TYPE_UNKNOWN,
    NET_TYPE_CELLULAR,
    NET_TYPE_ETHERNET,
    NET_TYPE_WIFI,
    NET_TYPE_VPN,
    NET_TYPE_MAX
} NetworkType;

struct NetSpecifier : public Parcelable {
    NetSpecifier(const std::string& ident, uint32_t netType, uint64_t netCapabilities);
    NetSpecifier() {};
    std::string ident_;
    uint32_t netType_ = NET_TYPE_UNKNOWN;
    uint64_t netCapabilities_ = NET_CAPABILITIES_NONE;
    bool IfValid();

    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetSpecifier> Unmarshalling(Parcel &parcel);
    static bool Marshalling(Parcel &parcel, const sptr<NetSpecifier> &object);
    std::string ToString(const std::string &tab) const;

    bool operator< (const NetSpecifier &spec) const
    {
        return (netType_ < spec.netType_) || (netType_ == spec.netType_ && netCapabilities_ < spec.netCapabilities_);
    }
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_SPECIFIER_H