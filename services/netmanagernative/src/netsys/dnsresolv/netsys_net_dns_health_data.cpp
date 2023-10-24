/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "netnative_log_wrapper.h"
#include "netsys_net_dns_health_data.h"

namespace OHOS {
namespace NetsysNative {

bool NetDnsHealthReport::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(netid_)) {
        return false;
    }
    if (!parcel.WriteUint32(uid_)) {
        return false;
    }
    if (!parcel.WriteUint32(appid_)) {
        return false;
    }
    if (!parcel.WriteString(host_)) {
        return false;
    }
    if (!parcel.WriteUint16(type_)) {
        return false;
    }
    if (!parcel.WriteBool(result_)) {
        return false;
    }
    return true;
}


bool NetDnsHealthReport::Unmarshalling(Parcel &parcel, NetDnsHealthReport &healthReport)
{
    if (!parcel.ReadUint32(healthReport.netid_) || !parcel.ReadUint32(healthReport.uid_) ||
        !parcel.ReadUint32(healthReport.appid_)) {
        return false;
    }

    if (!parcel.ReadString(healthReport.host_)) {
        return false;
    }

    if (!parcel.ReadUint16(healthReport.type_)) {
        return false;
    }

    if (!parcel.ReadBool(healthReport.result_)) {
        return false;
    }

    return true;
}
} // namespace NetsysNative
} // namespace OHOS
